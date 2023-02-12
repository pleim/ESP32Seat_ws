#pragma once
#include <Preferences.h>

Preferences preferences;

class Parameters
{
private:
  /* data */
public:
  Parameters(/* args */);
  ~Parameters();

  void InitParameters();
  void SaveParameters();
  void Parse(String s);
  String UpdateParameters();

  // [1] Motor speed 
  int speed;
  // [1] Upper position seat (raw value)
  int top;
  // [1] Upper position seat (raw value)
  int bottom;
  // [1] Filter seat position measurement
  float fpos;
  // [1] Filter seat position measurement
  float fcurr;
  // [mA] 
  int curr;   
  // [%]
  float hyst;
  // [°] Point 1 - angle
  float p1angle;
  // [%] Point 1 - ref. position
  float p1ref;
  // [°] Point 2 - angle
  float p2angle;
  // [%] Point 2 - ref. position
  float p2ref;
};

Parameters::Parameters(/* args */)
{}

Parameters::~Parameters()
{}

void Parameters::Parse(String s)
{
  if (s.length() < 3)
    return;

  int i = s.indexOf('=');
  if (i == -1)
    return;

  String value = s.substring(i + 1);

  if (s.startsWith("par_speed"))
    speed = value.toInt();
  if (s.startsWith("par_top"))
    top = value.toInt();
  if (s.startsWith("par_bottom"))
    bottom = value.toInt();
  if (s.startsWith("par_fpos"))
    fpos = value.toFloat();
  if (s.startsWith("par_fcurr"))
    fcurr = value.toFloat();  
  if (s.startsWith("par_currmax"))
    curr = value.toInt();
  if (s.startsWith("par_hyst"))
    hyst = value.toFloat();
  if (s.startsWith("par_p1angle"))
    p1angle = value.toFloat();
  if (s.startsWith("par_p1ref"))
    p1ref = value.toFloat();
  if (s.startsWith("par_p2angle"))
    p2angle = value.toFloat();
  if (s.startsWith("par_p2ref"))
    p2ref = value.toFloat();
}

String Parameters::UpdateParameters()
{
  String par = "par;";
  par += String(speed) + ";";
  par += String(top) + ";";
  par += String(bottom) + ";";
  par += String(fpos) + ";";
  par += String(fcurr) + ";";
  par += String(curr) + ";";
  par += String(hyst) + ";";
  par += String(p1angle) + ";";
  par += String(p1ref) + ";";
  par += String(p2angle) + ";";
  par += String(p2ref);
  return par;
}

void Parameters::InitParameters()
{
  speed = preferences.getInt("p_speed", 0);
  top = preferences.getInt("p_top", 0);
  bottom = preferences.getInt("p_bottom", 0);
  fpos = preferences.getFloat("p_fpos", 0);
  fcurr = preferences.getFloat("p_fcurr", 0);
  curr = preferences.getInt("p_curr", 0);
  hyst = preferences.getFloat("p_hyst", 0);
  p1angle = preferences.getFloat("p_p1angle", 0);
  p1ref = preferences.getFloat("p_p1ref", 0);
  p2angle = preferences.getFloat("p_p2angle", 0);
  p2ref = preferences.getFloat("p_p2ref", 0);
}

void Parameters::SaveParameters()
{
  preferences.putInt("p_speed", speed);
  preferences.putInt("p_top", top);
  preferences.putInt("p_bottom", bottom);
  preferences.putFloat("p_fpos", fpos);
  preferences.putFloat("p_fcurr", fcurr);
  preferences.putInt("p_curr", curr);
  preferences.putFloat("p_hyst", hyst);
  preferences.putFloat("p_p1angle", p1angle);
  preferences.putFloat("p_p1ref", p1ref);
  preferences.putFloat("p_p2angle", p2angle);
  preferences.putFloat("p_p2ref", p2ref);
}
