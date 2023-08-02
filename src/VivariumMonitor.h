/*
 * VivariumMonitor.h
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#ifndef VIVARIUMMON_H
#define VIVARIUMMON_H

#include <Arduino.h>
#include <time.h>

// Set up debugging if it's turned on
#ifdef DEBUG_ESP_CORE
#define DEBUG_ON 1
#define DEBUG_USE_SERIAL 1
#define DEBUG_USE_TELNET 1
#include <ESPTelnet.h>
extern ESPTelnet telnet;
#undef DEBUG_MSG
#define DEBUG_MSG(format, ...) DEBUG_TELNET.printf(PSTR(format), ##__VA_ARGS__); DEBUG_SERIAL.printf(PSTR(format), ##__VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif

#define CONFIG_TIMEOUT 300
#define HTTP_TIMEOUT 8000
#define FIRMWARE_CHECK_SECONDS 14400
#define FW_URL_FILE F("/fw_url")
#define CONFIG_STR_LEN 126

#define I2C_SLAVE_ADDRESS 42
#define SHT40_ADDRESS 0x44
#define SHT40_READ_CMD 0xFD
#define SHT40_HEATER_CMD 0x2F
#define HEAT_INTERVAL 300000

#define RESOLUTION 9
#define ONE_WIRE_BUS 2   //D4


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
  int num_therm_sensors;

  // Time setup
  const char* ntp_zone;
  const char* ntp_server;

  // Network endpoint setup
  Url stats_url;
  unsigned int stats_interval;
} ViviariumMonitorConfig;

/*
 * Interfaces to the sensors, as well as the output controller.
 */
class VivariumMonitor {
  public:
    void init(VivariumMonitorConfig config);
    void setDigitalOneHandler(byte (*)(SensorData));
    void setDigitalTwoHandler(byte (*)(SensorData));
    void setAnalogHandler(byte (*)(SensorData));
    void handle_events();
  private:
    ViviariumMonitorConfig monitor_config;
    time_t last_read = 0;
    time_t last_fw_check = 0;
    time_t last_post_stats = 0;
    unsigned long last_heated = 0;
    SensorReading last_humidity;
    SensorReading last_air_temp;
    Url update_url;
    byte (*digital_1_func)(SensorData) = NULL;
    byte (*digital_2_func)(SensorData) = NULL;
    byte (*analog_func)(SensorData) = NULL;
    void readSHTsensor(SensorData* output);
    void readTempSensors(SensorData* output);
    void update_firmware();
    void post_stats(SensorData* readings);
    void set_analog(byte value);
    void set_digital_1(byte value);
    void set_digital_2(byte value);
    void write_outputs();
};

#endif
