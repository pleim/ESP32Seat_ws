#define M_PI           3.14159265358979323846
#include "calc.h"
#include <math.h>

// https://stackoverflow.com/questions/3755059/3d-accelerometer-calculate-the-orientation 
// https://howtomechatronics.com/tutorials/arduino/how-to-track-orientation-with-arduino-and-adxl345-accelerometer/


// Normalize all axis to range -1..+1 (not limited)
acc Normalize(int x, int y, int z, int nx, int ny, int nz )
{
    acc a;
    a.ax = x / nx;
    a.ay = y / ny;
    a.az = z / nz;

    return a;
}

// Calc pitch 
float Pitch(acc a)
{       
    float magxy = sqrtf(a.ax*a.ax + a.ay*a.ay); 
    float p = atan2(magxy, a.az);
    return Rad2Deg(p);
}

// radiant to degree
float Rad2Deg(float rad)
{
    return rad * 180 / M_PI;
}

