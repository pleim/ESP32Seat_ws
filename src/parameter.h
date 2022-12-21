#ifndef _parameter_h
#define _parameter_h

#include <Preferences.h>

Preferences preferences;

typedef struct par_type
{
    int speed = 90;
    int top = 70;
    int bottom = 15;
    int curr = 320;
    int hyst = 50;
    int offset = 450;
    int sensibility = 300;
}parameters;

void ParsePair(String s, parameters *p)
{
  if (s.length() < 3)
    return;

  int i = s.indexOf('=');
  if (i == -1)
    return;

  int value = s.substring(i + 1).toInt();

  if (s.startsWith("par_speed"))
    p->speed = value;
  if (s.startsWith("par_top"))
    p->top = value;
  if (s.startsWith("par_bottom"))
    p->bottom = value;
  if (s.startsWith("par_currmax"))
    p->curr = value;
  if (s.startsWith("par_hyst"))
    p->hyst = value;
  if (s.startsWith("par_offset"))
    p->offset = value;
  if (s.startsWith("par_sens"))
    p->sensibility = value;
}

String updateParameters(parameters p)
{
  String par = "par;";
  par += String(p.speed) + ";";
  par += String(p.top) + ";";
  par += String(p.bottom) + ";";
  par += String(p.curr) + ";";
  par += String(p.hyst) + ";";
  par += String(p.offset) + ";";
  par += String(p.sensibility);
  return par;
}

parameters initParameters()
{
  parameters p;
  p.speed = preferences.getInt("p_speed", 0);
  p.top = preferences.getInt("p_top", 0);
  p.bottom = preferences.getInt("p_bottom", 0);
  p.curr = preferences.getInt("p_curr", 0);
  p.hyst = preferences.getInt("p_hyst", 0);
  p.offset = preferences.getInt("p_offset", 0);
  p.sensibility = preferences.getInt("p_sens", 0);
  return p;
}

void saveParameters(parameters p)
{
  preferences.putInt("p_speed", p.speed);
  preferences.putInt("p_top", p.top);
  preferences.putInt("p_bottom", p.bottom);
  preferences.putInt("p_curr", p.curr);
  preferences.putInt("p_hyst", p.hyst);
  preferences.putInt("p_offset", p.offset);
  preferences.putInt("p_sens", p.sensibility);
}

#endif // _parameter_h
