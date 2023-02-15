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
  // [1] Filter pitch measurement
  float fpitch;
  // [1] Filter seat position measurement
  float fpos;
  // [1] Filter seat position measurement
  float fcurr;
  // [mA] 
  int curr;   
  // [%]
  float hyst;
  // [°] Point 1 - pitch angle for 0 % position reference
  float p1pitch;
  // [°] Point 2 - pitch angle for 100% position reference
  float p2pitch;
};

Parameters::Parameters()
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
  if (s.startsWith("par_fpitch"))
    fpitch = value.toFloat();
  if (s.startsWith("par_fpos"))
    fpos = value.toFloat();
  if (s.startsWith("par_fcurr"))
    fcurr = value.toFloat();  
  if (s.startsWith("par_curr"))
    curr = value.toInt();
  if (s.startsWith("par_hyst"))
    hyst = value.toFloat();
  if (s.startsWith("par_p1pitch"))
    p1pitch = value.toFloat();
  if (s.startsWith("par_p2pitch"))
    p2pitch = value.toFloat();
}

String Parameters::UpdateParameters()
{
  String par = "par;";
  par += String(speed) + ";";   // 1
  par += String(top) + ";";     // 2
  par += String(bottom) + ";";  // 3
  par += String(fpitch) + ";";  // 4
  par += String(fpos) + ";";    // 5
  par += String(fcurr) + ";";   // 6
  par += String(curr) + ";";    // 7
  par += String(hyst) + ";";    // 8
  par += String(p1pitch) + ";"; // 9
  par += String(p2pitch) + ";"; // 10
  return par;
}

void Parameters::InitParameters()
{
  speed = preferences.getInt("p_speed", 0);
  top = preferences.getInt("p_top", 0);
  bottom = preferences.getInt("p_bottom", 0);
  fpitch = preferences.getFloat("p_fpitch", 0);
  fpos = preferences.getFloat("p_fpos", 0);
  fcurr = preferences.getFloat("p_fcurr", 0);
  curr = preferences.getInt("p_curr", 0);
  hyst = preferences.getFloat("p_hyst", 0);
  p1pitch = preferences.getFloat("p_p1pitch", 0);
  p2pitch = preferences.getFloat("p_p2pitch", 0);  
}

void Parameters::SaveParameters()
{
  preferences.putInt("p_speed", speed);
  preferences.putInt("p_top", top);
  preferences.putInt("p_bottom", bottom);
  preferences.putFloat("p_fpitch", fpitch);
  preferences.putFloat("p_fpos", fpos);
  preferences.putFloat("p_fcurr", fcurr);
  preferences.putInt("p_curr", curr);
  preferences.putFloat("p_hyst", hyst);
  preferences.putFloat("p_p1pitch", p1pitch);
  preferences.putFloat("p_p2pitch", p2pitch);  
}
