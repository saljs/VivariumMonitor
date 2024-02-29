/*
 * PIDController.cpp
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#include "PIDController.h"

PIDController::PIDController(float target,
                             float Kp,
                             float Ki,
                             float Kd, 
                             float dropoff
) {
  _dropoff = dropoff;
  _target = target;
  _Kp = Kp;
  _Ki = Ki;
  _Kd = Kd;
}

byte PIDController::add_reading(SensorReading reading) {
  float error, derivative;
  if (reading.has_error) {
    // In the case of a sensor error, return the last value
    error = _prev_error;
  }
  else { 
    error = _target - reading.value;
  }
  // We assume dt is 1, since we sample every second
  derivative = error - _prev_error;
  _integral = (_integral * _dropoff) + error;
  _prev_error = error;

  // Clamp the output to the range of a byte
  int output = (_Kp * error) + (_Ki * _integral) + (_Kd * derivative);
  if (output < -127) output = -127;
  if (output > 128) output = 128;

  return 127 + output;
}
