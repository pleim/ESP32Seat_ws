// This example code is in the Public Domain (or CC0 licensed, at your option.)
// By Evandro Copercini - 2018
//
// https://randomnerdtutorials.com/stepper-motor-esp32-websocket/#more-105630
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

#include "parameters.h"
#include "calc.h"
#include "lowpass_filter.h"

const char *ssid = "ipfire";         // Replace with your SSID
const char *password = "joseffranz"; // Repalce with your password

bool ledstate = 0;
String message = "";

// Parameters
Parameters p;

// Accelerometer
MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

// Filters and signal adaption
LowPassFilter LPFpitch = LowPassFilter(1.0);      // param
LowPassFilter LPFpositon = LowPassFilter(0.05);
LowPassFilter LPFcurrent = LowPassFilter(0.05);   // param
LowPassFilter LPFvoltage = LowPassFilter(0.1);

// Create Webserver
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Pins
const int pin_wlanmode = 13;  // at startup 0 = Client mode, 1 (or float) = AP mode
const int pin_position = 35;     // ADC1_CH6

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
int positionraw;    // seat position, raw signal of hall sensor
int currentraw;     // Raw value current sense
float pitch;        // pitch angle filtered [°]
float positionref;  // reference for seat positon [%]
float position;     // seat position, adapted to range, filtered [%]
float current;      // motor current scaled and filtered [mA]
float speed;        // motor speed [%]
float voltage;      // battery voltage [mV]


int m_auto = 0;   // Mode (0 off, 1 active)
int m_axis = 0;   // Axis select ()

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

void updateState()
{
  String stat = "state;";
  stat += String(pitch) + ";";      // 1
  stat += String(position) + ";";   // 2
  stat += String(speed) + ";";      // 3
  stat += String(current) + ";";    // 4
  stat += String(voltage) + ";";    // 5
  stat += String(ax) + " " + String(ay) + " " + String(az) + ";"; // 6
  stat += String(positionraw) + ";";  // 7
  stat += String(positionref) + ";";  // 8

  stat += (m_auto == 1) ? "1;" : "0;";  // 9
  stat += (m_axis == 1) ? "1;" : "0;";  // 10
  stat += (speed > 0) ? "1;" : "0;";    // 11
  stat += (speed < 0) ? "1" : "0";      // 12

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
    p.Parse(message);

    if (message == "save")
    {
      p.SaveParameters();
      Serial.println("Parameters saved");
      //
      // File spiffsLogFile = SPIFFS.open("/log.txt", FILE_APPEND);
      // String logstring = "Arduino\r\n";
      // byte logbuffer[logstring.length() + 1];
      // logstring.getBytes(logbuffer, logstring.length() + 1);
      // spiffsLogFile.write((uint8_t*) logbuffer, sizeof(logbuffer));
      // spiffsLogFile.flush();
      // spiffsLogFile.close();
      //
    }
    if (message == "mode_auto=false") m_auto = 0;
    if (message == "mode_auto=true")  m_auto = 1;

    if (message == "mode_axis=false") m_axis = 0;
    if (message == "mode_axis=true")  m_axis = 1;

    ws.textAll(p.UpdateParameters());
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
    ws.textAll(p.UpdateParameters());    
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

// -----------------------------------  control  ------------------------------------------------

void control()
{  
  // current limit
  if(current > p.curr) m_auto = 0;

  // position control
  if(m_auto == 1)
  {
    float pos_up = positionref - p.hyst;
    float pos_down = positionref + p.hyst;

    if(position < pos_up) speed = p.speed;
    if(position > pos_down) speed = -p.speed;

    if(speed > 0 && position > positionref) speed = 0;
    if(speed < 0 && position < positionref) speed = 0;    
  }
  else
  {
    speed = 0;
  }
}

// -----------------------------------  setup  --------------------------------------------------

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
  p.InitParameters();
  LPFpitch.Tf = p.fpitch;
  LPFpositon.Tf = p.fpos;
  LPFcurrent.Tf = p.fcurr;
  
  // Prepare for log
  if(!SPIFFS.exists("/log.txt")) {
      File writeLog = SPIFFS.open("/log.txt", FILE_WRITE);
      if(!writeLog) Serial.println("Couldn't open spiffs_log.txt");
      delay(50);
      writeLog.close();
  } 

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

// -----------------------------------  loop -----------------------------------------------------

void loop()
{
  if ((millis() - lastTime) > timerDelay)
  {
    slowCount++;

    // Read analog inputs
    positionraw = ReadAnalog(pin_position);
    float positionadapted = Adapt(positionraw, p.bottom, p.top);
    position = LPFpositon(positionadapted, p.fpos);

    currentraw = ReadAnalog(pin_current);
    current = LPFcurrent(currentraw * 1.0f, p.fcurr);

    int voltageraw = ReadAnalog(pin_cellvolt);
    voltage = LPFvoltage(voltageraw * 0.579f, 1.0f);  // const 1s filter
    
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    float pitchraw = Pitch(ax, ay, az);
    pitch = LPFpitch(pitchraw, p.fpitch);

    // control
    positionref = InterpolateP(pitch, p.p1pitch, p.p2pitch);
    control();

    // update outputs
    digitalWrite(pin_motordir, (speed < 0));
    ledcWrite(pwmChannel, abs((speed*25)/10));

    if(slowCount >= slowTask)
    {
      updateState();  // update GUI
      slowCount = 0;
    }

    lastTime = millis();
  }
  ws.cleanupClients();
}