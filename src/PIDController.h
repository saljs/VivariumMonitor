/*
 * PIDController.h
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include "types.h"

class PIDController
{
public:
  PIDController(float target, float Kp, float Ki, float Kd, float dropoff);
  byte add_reading(SensorReading reading, time_t timestamp);

private:
  float _target = 0;
  float _dropoff = 0;
  float _Kp = 0;
  float _Ki = 0;
  float _Kd = 0;
  float _prev_error = 0;
  float _integral = 0;
  time_t last_sampled = 0;
};

#endif
