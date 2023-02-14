#pragma once

#include <math.h>

#define M_PI           3.14159265358979323846

// https://stackoverflow.com/questions/3755059/3d-accelerometer-calculate-the-orientation 
// https://howtomechatronics.com/tutorials/arduino/how-to-track-orientation-with-arduino-and-adxl345-accelerometer/


// -----------  Calc pitch  ------------------
float Pitch(int x, int y, int z)
{       
    float fx = float(x);
    float fy = float(y);
    float fz = float(z);
    float mag = sqrtf(fx*fx + fz*fz); 
    float p = atan2(fy, mag);
    return p* 180 / M_PI;
}


// -------------  Adapt  -----------------------
float Adapt(int in, int min, int max)
{
    float a = float(in - min);
    float b = float(max - min);
    return a/b * 100.0f;  
}


// ------------  Interpolation  ----------------
float Interpolate(float x, float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    float out = (x - x1)/dx * dy + y1;
    float min = fmin(y1, y2);
    float max = fmax(y1, y2);
    if(out < min) out = min;
    if(out > max) out = max;
    return out;
}

float InterpolateP(float x, float x1, float x2)
{
    float dx = x2 - x1;    
    float out = (x - x1)/dx * 100;
    if(out < 0.0) out = 0.0;
    if(out > 100.0) out = 100.0;
    return out;
}

