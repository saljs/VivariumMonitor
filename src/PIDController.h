#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include "VivariumMonitor.h"

class PIDController {
  public:
    PIDController(float target, float Kp, float Ki, float Kd, float dropoff);
    byte add_reading(SensorReading reading);
  private:
    float _target = 0;
    float _dropoff = 0;
    float _Kp = 0;
    float _Ki = 0;
    float _Kd = 0;
    float _prev_error = 0;
    float _integral = 0;
};

#endif
