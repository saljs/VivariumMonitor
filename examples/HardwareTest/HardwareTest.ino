/* 
 * The purpose of this file to to quickly sanity-check
 * a new build on actual hardware. By connecting a scope
 * to the SDA pin on an esp8266 module (D2), you can observe
 * that the outputs are being sent once a second, the sensors
 * are queried every 5 seconds (as indicated by status light
 * flashing (D4) and aditional data on the scope. In addition, 
 * data are posted to to an endpoint every 30 seconds.
 */


#include <VivariumMonitor.h>
#include <ESP8266WiFi.h>

VivariumMonitor monitor;
time_t last_reading = 0;

void setup() {
  DEBUG_MSG("Vivarium Monitor firmware " FIRMWARE_VERSION);
  Url stats_url = {
    {.host = "192.168.0.17"}, // Change to local instance IP
    {.path = "/"},
    .port = 5000,
    .set = true,
  };

  VivariumMonitorConfig config = {
    .has_sht_sensor = true,
    .num_therm_sensors = 2,
    .sample_interval = 5, // sample every 5 seconds
    .ntp_zone = "UTC0",
    .ntp_server = "pool.ntp.org",
    .stats_url = stats_url,
    .stats_interval = 30, // send to stats server every 30 seconds
  };

  // Set hostname
  WiFi.hostname(F("viv-monitor-hardwaretest"));
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

// alternate these every second
byte digital_1_handler(SensorData reading, time_t now)
{
  if (reading.timestamp > last_reading) {
    DEBUG_MSG("Sensor readings:\n"
       " timestamp: %d\n"
       "  high temp: %f\n"
       "  low temp:  %f\n"
       "  humidity:  %f\n"
       "  air temp:  %f\n", 
       reading.timestamp,
       reading.high_temp.value,
       reading.low_temp.value,
       reading.humidity.value,
       reading.air_temp.value);
    last_reading = reading.timestamp;
  }
  return now % 2 == 0 ? 1 : 0;
} 
byte digital_2_handler(SensorData reading, time_t now)
{
  return now % 2 == 0 ? 0 : 1;
} 
// increment this one every time a sensor reading changes
byte analog_handler(SensorData reading, time_t now)
{
  return reading.timestamp % 255;
}
