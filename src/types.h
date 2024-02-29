/*
 * types.h
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#ifndef VIVARIUMONITORTYPES_H
#define VIVARIUMONITORTYPES_H

#include <time.h>
#define CONFIG_STR_LEN 126

#ifndef byte
typedef unsigned char byte;
#endif

/*
 * A monad that indicates if a sensor reading was sucessful, and it's value.
 */
typedef struct SensorReading {
  bool has_error;
  float value;
} SensorReading;

/*
 * A collection of sensor readings.
 */
typedef struct SensorData {
  SensorReading humidity;
  SensorReading air_temp;
  SensorReading high_temp;
  SensorReading low_temp;
  time_t timestamp;
} SensorData;

/*
 * Simple URL definition
 */
 typedef struct Url {
  char host[CONFIG_STR_LEN];
  char path[CONFIG_STR_LEN];
  unsigned int port;
  bool set;
} Url;
 
/*
 * Configuration data for the VivariumMonitor class.
 */
typedef struct VivariumMonitorConfig {
  // Hardware setup
  bool has_sht_sensor;
  unsigned int num_therm_sensors;
  unsigned int sample_interval;

  // Time setup
  const char* ntp_zone;
  const char* ntp_server;

  // Network endpoint setup
  Url stats_url;
  unsigned int stats_interval;
} ViviariumMonitorConfig;

# endif
