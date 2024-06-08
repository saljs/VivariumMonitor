/*
 * Hardware.h
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#include "types.h"
#include <time.h>

#define I2C_SLAVE_ADDRESS 42
#define SHT40_ADDRESS 0x44
#define SHT40_READ_CMD 0xFD
#define SHT40_HEATER_CMD 0x2F
#define HEAT_INTERVAL 300

#define RESOLUTION 9
#define ONE_WIRE_BUS 2 // D4

/*
 * Interfaces to the sensors, as well as the output controller.
 */
class Hardware
{
public:
  void init(VivariumMonitorConfig* config);
  void set_analog(byte value);
  void set_digital_1(byte value);
  void set_digital_2(byte value);
  void write_outputs();
  SensorData read_sensors(time_t now);

private:
  VivariumMonitorConfig* monitor_config = NULL;
  time_t last_heated = 0;
  SensorData reading;
  bool readSHTsensor(SensorData& output, time_t now);
  bool readTempSensors(SensorData& output);
};

#endif
