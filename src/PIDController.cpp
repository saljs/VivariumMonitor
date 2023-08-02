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
  if (reading.has_error) {
    // In the case of a sensor error, return 0
    return 0;
  }
  
  // We assume dt is 1, since we sample every second
  float error = _target - reading.value;
  float derivative = error - _prev_error;
  _integral = (_integral * _dropoff) + error;
  _prev_error = error;

#ifdef DEBUG_ESP_CORE
  Serial.print("PID controller error: ");
  Serial.println(error);
#endif

  // Clamp the output to the range of a byte
  int output = (_Kp * error) + (_Ki * _integral) + (_Kd * derivative);
  if (output < -127) output = -127;
  if (output > 128) output = 128;

  return 127 + output;
}
