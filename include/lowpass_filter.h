//#pragma once
#include <esp32-hal-timer.h>

/*
    https://docs.simplefoc.com/low_pass_filter
  
    Initialization:
    LowPassFilter filter = LowPassFilter(0.001); // Tf = 1ms

    Use:
    float signal_filtered = filter(signal);

    Change filter time:
    filter.Tf = 0.01; // changed to 10ms

*/
  
class LowPassFilter
{
public:
    // Initialize filter
    // @param time_constant Filter time constant in seconds
    LowPassFilter(float time_constant)
    {
        Tf = time_constant;
        y_prev = 0.0f;
        timestamp_prev = micros();
    };

    //~LowPassFilter() = default;
  
    float operator () (float x, float tc)
    {
        Tf = tc;
        unsigned long timestamp = micros();
        float dt = (timestamp - timestamp_prev)*1e-6f;

        if (dt < 0.0f || dt > 0.5f)
        dt = 1e-3f;

        float alpha = Tf/(Tf + dt);
        float y = alpha*y_prev + (1.0f - alpha)*x;

        y_prev = y;
        timestamp_prev = timestamp;
        return y;
    };

    float Tf; 
  
protected:
    unsigned long timestamp_prev;  
    float y_prev; 
};