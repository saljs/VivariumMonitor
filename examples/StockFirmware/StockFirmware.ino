/*
 * Stock firmware for Vivarium Monitors
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#include <VivariumMonitor.h>
#include <ESP8266WiFi.h>

VivariumMonitor monitor;

void setup() {
  DEBUG_MSG("Vivarium Monitor firmware " FIRMWARE_VERSION);
  Url stats_url = {
    {.host = "io.adafruit.com"},      
    {.path = "/api/v2/webhooks/feed/{key}/raw"},
    .port = 80,
    .set = true,
  };

  VivariumMonitorConfig config = {
    .has_sht_sensor = true,
    .num_therm_sensors = 0,
    .ntp_zone = "UTC0",
    .ntp_server = "pool.ntp.org",
    .stats_url = stats_url,
    .stats_interval = 600,
  };

  // Set hostname
  WiFi.hostname(F("viv-monitor-stock"));
 
  // Set output handlers
  monitor.setDigitalOneHandler(digital_1_handler);
  monitor.setDigitalTwoHandler(digital_2_handler);
  monitor.setAnalogHandler(analog_handler);

  // Initialize hardware
  monitor.init(config);
}

void loop() {
  monitor.handle_events();
}

byte digital_1_handler(SensorData reading)
{
  return 0;
} 

byte digital_2_handler(SensorData reading)
{
  return 0;
} 

byte analog_handler(SensorData reading)
{
  return 0;
}
