/*
 * VivariumMonitor.cpp
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#include "VivariumMonitor.h"

#include <DNSServer.h>
#include <LittleFS.h>
#include <WiFiManager.h>

/**********************************************************
   Global vars
 **********************************************************/
#if DEBUG_USE_TELNET
ESPTelnet telnet;
#endif

/************************************************************
 * URL update Callback
 ************************************************************/
bool updateUrls = false;
void
saveConfigCallback()
{
  updateUrls = true;
}

/**********************************************************
   Public functions
 **********************************************************/
void
VivariumMonitor::setDigitalOneHandler(byte (*func)(SensorData, time_t))
{
  digital_1_func = func;
}

void
VivariumMonitor::setDigitalTwoHandler(byte (*func)(SensorData, time_t))
{
  digital_2_func = func;
}

void
VivariumMonitor::setAnalogHandler(byte (*func)(SensorData, time_t))
{
  analog_func = func;
}

void
VivariumMonitor::init(VivariumMonitorConfig config)
{
  WiFiManager wifiManager;
  Url update_url;
  char rules_port_tmp[6] = "80";

  monitor_config = config;
  update_url.host[0] = '\0';
  update_url.path[0] = '\0';

#if DEBUG_USE_SERIAL
  Serial.begin(9600);
#endif

  // set up networking
  configTime(monitor_config.ntp_zone, monitor_config.ntp_server);

  // Connect to WiFi
  WiFiManagerParameter update_host("update_host",
                                   "Hostname of configuration server.",
                                   update_url.host,
                                   CONFIG_STR_LEN);
  wifiManager.addParameter(&update_host);
  WiFiManagerParameter update_port("update_port",
                                   "Port to connect on configuration server.",
                                   rules_port_tmp,
                                   6);
  wifiManager.addParameter(&update_port);
  WiFiManagerParameter update_path("update_path",
                                   "Path of configuration file on server.",
                                   update_url.path,
                                   CONFIG_STR_LEN);
  wifiManager.addParameter(&update_path);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
#ifndef DEBUG_ESP_CORE
  wifiManager.setDebugOutput(false);
#endif
  wifiManager.setConfigPortalTimeout(CONFIG_TIMEOUT);
  wifiManager.autoConnect("VivController-setup");
  strlcpy(update_url.host, update_host.getValue(), sizeof(update_url.host));
  strlcpy(update_url.path, update_path.getValue(), sizeof(update_url.path));
  update_url.port = atoi(update_port.getValue());

#if DEBUG_USE_TELNET
  telnet.begin();
#endif

  if (LittleFS.begin()) {
    if (updateUrls && strlen(update_url.host) > 0) {
      File urlFile = LittleFS.open(FW_URL_FILE, "w");
      update_url.set = true;
      urlFile.write((char*)(&update_url), sizeof(update_url));
      urlFile.close();
      DEBUG_MSG("Updating update URL file to: http://%s:%d%s\n",
                update_url.host,
                update_url.port,
                update_url.path);
    } else if (LittleFS.exists(FW_URL_FILE)) {
      File urlFile = LittleFS.open(FW_URL_FILE, "r");
      urlFile.readBytes((char*)(&update_url), sizeof(update_url));
      urlFile.close();
      DEBUG_MSG("Update URL: http://%s:%d%s\n",
                update_url.host,
                update_url.port,
                update_url.path);
    } else {
      update_url.set = false;
    }
    LittleFS.end();
  } else {
    update_url.set = false;
    DEBUG_MSG("Error, cannot mount FS! Proceeding without it.\n");
  }

  // If we've entered the config portal, reset to clear out heap and read in new
  // config.
  if (updateUrls) {
    ESP.restart();
  }
  updateUrls = false;

  // Initialize submodules
  net_interface.init(&monitor_config, update_url);
  hardware_interface.init(&monitor_config);

  // Give NTP time to sync
  //  Not garunteed to do so in three seconds, but in most cases will keep
  //  relays from flickering on startup.
  yield();
  delay(3000);
}

void
VivariumMonitor::handle_events()
{
  time_t now;
  byte analog_out = 0, digital_1_out = 0, digital_2_out = 0;
  time(&now);
  SensorData data = hardware_interface.read_sensors(now);

  // Update outputs
  if (analog_func) {
    analog_out = analog_func(data, now);
    hardware_interface.set_analog(analog_out);
  }
  if (digital_1_func) {
    digital_1_out = digital_1_func(data, now);
    hardware_interface.set_digital_1(digital_1_out);
  }
  if (digital_2_func) {
    digital_2_out = digital_2_func(data, now);
    hardware_interface.set_digital_2(digital_2_out);
  }

  hardware_interface.write_outputs();
  net_interface.post_stats(data, digital_1_out, digital_2_out, analog_out);

  // Check firmware update timer
  net_interface.update_firmware(now);
  // Serve web requests
  net_interface.serve_web_interface();
#if DEBUG_USE_TELNET
  telnet.loop();
#endif
}
