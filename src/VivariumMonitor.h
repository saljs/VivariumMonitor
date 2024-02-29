/*
 * VivariumMonitor.h
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#ifndef VIVARIUMMONITOR_H
#define VIVARIUMMONITOR_H

#include <Arduino.h>
#include <time.h>
#include "types.h"

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

#define I2C_SLAVE_ADDRESS 42
#define SHT40_ADDRESS 0x44
#define SHT40_READ_CMD 0xFD
#define SHT40_HEATER_CMD 0x2F
#define HEAT_INTERVAL 300000

#define RESOLUTION 9
#define ONE_WIRE_BUS 2   //D4

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
    void serve_web_interface();
    void post_stats(SensorData* readings);
    void set_analog(byte value);
    void set_digital_1(byte value);
    void set_digital_2(byte value);
    void write_outputs();
};

/*
 * Web server strings
 */
#define HTTP_404_RESPONSE "\
HTTP/1.1 404 NOT FOUND\r\n\
Content-type:text/plain\r\n\
Content-Length:12\r\n\
Connection:close\r\n\r\n\
Not found.\r\n"

#define HTTP_WEB_ROOT_HEAD "\
HTTP/1.1 200 OK\r\n\
Content-type:text/html\r\n\
Connection:close\r\n\r\n\
<!DOCTYPE html>\r\n\
<html>\r\n\
    <head>\r\n\
        <title>Vivarium Monitor Web Interface</title>\r\n\
    </head>\r\n\
    <body>\r\n\
        <h3>Vivarium Monitor Web Interface</h3>\r\n\
        <p>Node information:</p>\r\n\
        <ul>\r\n\
            <li><b>Firmare version:</b> " FIRMWARE_VERSION "</li>\r\n"

#define HTTP_WEB_ROOT_FOOTER "\
        </ul>\r\n\
        <a href=\"/rs\">\r\n\
            <button>Reset device</button>\r\n\
        </a>\r\n\
    </body>\r\n\
</html>\r\n"

#define HTTP_WEB_RS "\
HTTP/1.1 200 OK\r\n\
Content-type:text/html\r\n\
Content-Length:250\r\n\
Connection:close\r\n\r\n\
<!DOCTYPE html>\r\n\
<html>\r\n\
    <head>\r\n\
        <title>Vivarium Monitor Web Interface</title>\r\n\
    </head>\r\n\
    <body>\r\n\
        <h3>Vivarium Monitor Web Interface</h3>\r\n\
        <p>Node has been reset to default configuration.</p>\r\n\
    </body>\r\n\
</html>\r\n"

#endif
