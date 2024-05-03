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
                             float dropoff)
{
  _dropoff = dropoff;
  _target = target;
  _Kp = Kp;
  _Ki = Ki;
  _Kd = Kd;
}

byte
PIDController::add_reading(SensorReading reading, time_t timestamp)
{
  float error, derivative;
  int output, dt = timestamp - last_sampled;
  if (!timestamp) {
    // Initialize dt to sane value to prevent swing on startup.
    dt = 1;
  }
  if (reading.has_error || !dt) {
    // In the case of a sensor error, or no time passed, return last value
    error = _prev_error;
    derivative = 0;
  } else {
    last_sampled = timestamp;
    error = _target - reading.value;
    derivative = (error - _prev_error) / dt;
    _integral = (_integral * _dropoff) + (error * dt);
    _prev_error = error;
  }

  // Clamp the output to the range of a byte
  output = (_Kp * error) + (_Ki * _integral) + (_Kd * derivative);
  if (output < -127)
    output = -127;
  if (output > 128)
    output = 128;

  return 127 + output;
}
