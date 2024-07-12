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
  PIDController(double target, double Kp, double Ki, double Kd, double dropoff);
  byte add_reading(SensorReading reading, time_t timestamp);

private:
  double _target = 0;
  double _dropoff = 0;
  double _Kp = 0;
  double _Ki = 0;
  double _Kd = 0;
  double _prev_error = 0;
  double _integral = 0;
  time_t last_sampled = 0;
};

#endif
