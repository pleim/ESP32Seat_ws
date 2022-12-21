// This example code is in the Public Domain (or CC0 licensed, at your option.)
// By Evandro Copercini - 2018
//
//
// https://randomnerdtutorials.com/stepper-motor-esp32-websocket/#more-105630
//
// https://randomnerdtutorials.com/esp32-websocket-server-arduino/
//

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "SPIFFS.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

#include "parameter.h"

const char *ssid = "ipfire";         // Replace with your SSID
const char *password = "joseffranz"; // Repalce with your password

bool ledstate = 0;
String message = "";



// Accelerometer
MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

// Create Webserver
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Pins
const int pin_wlanmode = 13;  // at startup 0 = Client mode, 1 (or float) = AP mode
const int pin_poti = 35;     // ADC1_CH6

const int pin_motorpwm = 18; // speed of motor
const int pin_motordir = 19; // direction of motor
const int pin_motorflt = 4;  // motor fault (inverted)
const int pin_motorslp = 5;  // motor sleep (0 = on, 1 = sleep)
const int pin_current = 34;  // motor current sense pin of DRV8876, 2500mV/A

const int pin_cellvolt = 33; // cell voltage, divider 0.5, 3.7V -> 1.85V = 2296, scaling 620 1/V

const int freq = 5000;    // pwm
const int pwmChannel = 0; // pwm
const int resolution = 8; // pwm

// Timer variables
unsigned long lastTime = 0;       // timerstamp of last cycle 
unsigned long timerDelay = 100;   // intervall
unsigned int slowCount = 0;       // counter for slow task
unsigned int slowTask = 5;        // count of main cycles for slow task

// Variables
int pitch;        // pitch angle (° x 10 ??)
int position;     // seat position [%]
int speed;        // motor speed [%]
int current;      // motor current [mA]
int voltage;      // battery voltage [mV]

parameters p;


int m_auto = 0;   // Mode (0 off, 1 active)
int m_axis = 0;   // Axis select ()
int pitch_raw;    // Signal from accelerometer
int poti_raw;     // Signal from hall sensor
int current_raw;  // Raw value current sense


int ReadAnalog(int pin)
{
  int n = 10;  // max 10
  int x = 0;
  for (size_t i = 0; i < n; i++)
  {
    x += analogRead(pin);
  }
  return x/n;
}

void control(int pitch_in)
{  
  pitch = pitch * 9 + pitch_in;      // filter 
  pitch = pitch / 10;

  position = poti_raw - 2048;        // scale ?, +/- 2048

  // current limit
  if(current > p.curr) m_auto = 0;

  // position control
  if(m_auto == 1)
  {
    int pt = pitch;
    if(pt > p.top ) pt = p.top;
    if(pt < p.bottom) pt = p.bottom;

    int pos_up = pt - p.hyst;
    int pos_down = pt + p.hyst;
    int pos_stop = pt;

    if(position < pos_up) speed = p.speed;
    if(position > pos_down) speed = -p.speed;

    if(speed > 0 && position > pos_stop) speed = 0;
    if(speed < 0 && position < pos_stop) speed = 0;    
  }
  else
  {
    speed = 0;
  }
}

void updateState()
{
  String stat = "state;";
  stat += String(pitch) + ";";
  stat += String(position) + ";";
  stat += String(speed) + ";";
  stat += String(current) + ";";
  stat += String(voltage) + ";";

  stat += (m_auto == 1) ? "1;" : "0;";
  stat += (m_axis == 1) ? "1;" : "0;";
  stat += (speed > 0) ? "1;" : "0;";
  stat += (speed < 0) ? "1" : "0";

  ws.textAll(stat);
}

// new message event
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    message = (char *)data;
    Serial.println(message);
    ParsePair(message, &p);

    if (message == "save")
    {
      saveParameters(p);
      Serial.println("Parameters saved");
    }
    if (message == "mode_auto=false") m_auto = 0;
    if (message == "mode_auto=true")  m_auto = 1;

    if (message == "mode_axis=false") m_axis = 0;
    if (message == "mode_axis=true")  m_axis = 1;

    ws.textAll(updateParameters(p));
  }
}

// Every event from websocket
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    updateState();
    ws.textAll(updateParameters(p));    
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// Initialize SPIFFS
void initSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  else
  {
    Serial.println("SPIFFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi()
{
  int pinstate = digitalRead(pin_wlanmode);  
  Serial.printf("Pin state for WLAN mode: #%u \n", pinstate);

  if(pinstate == HIGH)
  {
    WiFi.softAP("Seat controller");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP mode, IP address: ");
    Serial.println(IP);
  }
  else
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Client mode, connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print('.');
      delay(1000);
    }
  }
  Serial.println(WiFi.localIP());
}

void setup()
{
  // GPIO
  pinMode(pin_wlanmode, INPUT_PULLUP);
  pinMode(pin_motordir, OUTPUT);
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(pin_motorpwm, pwmChannel);

  //
  Serial.begin(115200);
  initWiFi();
  initWebSocket();

  // Parameters
  initSPIFFS();
  preferences.begin("seat_ws", false);
  p = initParameters();


  // IMU sensor
  Wire.begin();
  accelgyro.initialize();
  accelgyro.setDLPFMode(6);   // Filter 5Hz (?)
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  // Web Server and update OTA
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  server.serveStatic("/", SPIFFS, "/");
  AsyncElegantOTA.begin(&server);
  server.begin();
}

void loop()
{
  if ((millis() - lastTime) > timerDelay)
  {
    slowCount++;

    // Read inputs
    poti_raw = ReadAnalog(pin_poti);
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    current_raw = ReadAnalog(pin_current);
    current = current_raw;

    // control
    if(m_axis == 0)
      pitch_raw = ay;
    else
      pitch_raw = az;
    
    pitch_raw = pitch_raw * p.sensibility;
    pitch_raw = pitch_raw / 1000;
    pitch_raw += p.offset;
    control(pitch_raw);

    // update outputs
    digitalWrite(pin_motordir, (speed < 0));
    ledcWrite(pwmChannel, abs((speed*25)/10));

    if(slowCount >= slowTask)
    {
      // cell voltage
      int n = ReadAnalog(pin_cellvolt);
      n = n * 1000;
      voltage = n / 579;
 
      // update gui
      updateState();
      slowCount = 0;
    }

    lastTime = millis();
  }

  ws.cleanupClients();
}