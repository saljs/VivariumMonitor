/*
 * VivariumMonitor.h
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#ifndef VIVARIUMMONITOR_H
#define VIVARIUMMONITOR_H

#include "debug.h"
#include "types.h"
#include <Arduino.h>
#include <time.h>

#define CONFIG_TIMEOUT 300
#define FW_URL_FILE F("/fw_url")

/*
 * Interfaces to the sensors, as well as the output controller.
 */
class VivariumMonitor
{
public:
  void init(VivariumMonitorConfig config);
  void setDigitalOneHandler(byte (*)(SensorData));
  void setDigitalTwoHandler(byte (*)(SensorData));
  void setAnalogHandler(byte (*)(SensorData));
  void handle_events();

private:
  VivariumMonitorConfig monitor_config;
  byte (*digital_1_func)(SensorData) = NULL;
  byte (*digital_2_func)(SensorData) = NULL;
  byte (*analog_func)(SensorData) = NULL;
};

#endif
