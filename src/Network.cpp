/*
 * Network.cpp
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#include "Network.h"
#include "debug.h"

#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <StreamUtils.h>
#include <Updater.h>

/**********************************************************
 * Global vars
 **********************************************************/
WiFiServer web_server(80);

/************************************************************
 * Utility functions
 ************************************************************/
int
getHttpResult(WiFiClient& wifi, void (*callback)(Stream&, size_t) = NULL)
{
  ReadBufferingStream bufferedWifi(wifi, 64);
  char buf[15];
  int ret;
  bool isheader = false;
  size_t len = 0;
  buf[14] = '\0';
  wifi.setTimeout(HTTP_TIMEOUT);
  if (!bufferedWifi.find("HTTP/1.")) {
    DEBUG_MSG("Request timed out\n");
    return -1;
  }
  buf[0] = (char)bufferedWifi.read();
  DEBUG_MSG("HTTP VERSION: 1.%c\n", buf[0]);

  ret = bufferedWifi.parseInt();
  DEBUG_MSG("Got response from server: %d\n", ret);

  bufferedWifi.find("\n");
  while (bufferedWifi.available()) {
    if (strcmp(buf, "Content-Length") == 0) {
      len = bufferedWifi.parseInt();
      isheader = true;
      DEBUG_MSG("Content length: %d\n", len);
    }

    for (byte i = 0; i < 13; i++) {
      buf[i] = buf[i + 1];
    }
    buf[13] = bufferedWifi.read();
    if (buf[13] == ':') {
      isheader = true;
    } else if (isheader && buf[13] == '\n') {
      isheader = false;
    } else if (!isheader && buf[13] == '\n') {
      break;
    }
  }

  if (ret == 200 && callback) {
    // Run callback function with the buffered stream
    callback(bufferedWifi, len);
  }
  return ret;
}

void
do_fw_upgrade(Stream& wifi, size_t len)
{
  size_t written = 0;
  DEBUG_MSG("Beginning firmware upgrade...\n");
  if (!Update.begin(len)) {
    DEBUG_MSG("Error starting update!\n");
#if DEBUG_USE_SERIAL
    Update.printError(Serial);
#endif
    return;
  }

  written = Update.writeStream(wifi);
  if (written != len) {
    DEBUG_MSG("Error writing update to flash!\n");
#if DEBUG_USE_SERIAL
    Update.printError(Serial);
#endif
    return;
  }

  if (!Update.end()) {
    DEBUG_MSG("Error finalizing update!\n");
#if DEBUG_USE_SERIAL
    Update.printError(Serial);
#endif
    return;
  }

  // reset chip
  DEBUG_MSG("Update finished. Rebooting.\n");
  ESP.restart();
}

/**********************************************************
 * Public functions
 **********************************************************/
void
Network::init(VivariumMonitorConfig* config, Url update_endpoint)
{
  monitor_config = config;
  update_url = update_endpoint;
  last_collected.timestamp = 0;
  last_collected.humidity.has_error = true;
  last_collected.air_temp.has_error = true;
  last_collected.high_temp.has_error = true;
  last_collected.low_temp.has_error = true;
  web_server.begin();
}

/*
 * Checks if an updated firmware is available, and upgrades if so.
 */
void
Network::update_firmware(time_t now)
{
  WiFiClient wifi;
  int status_code;

  if (now - last_fw_check < FIRMWARE_CHECK_SECONDS || !update_url.set) {
    return;
  }
  last_fw_check = now;
  DEBUG_MSG("Checking for updates...\n");
  if (!wifi.connect(update_url.host, update_url.port)) {
    DEBUG_MSG("Unable to connect to server.\n");
    return;
  }
  // Send HTTP request
  if (wifi.connected()) {
    wifi.printf("GET %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: "
                "VivMonitor1.0\r\nConnection: close\r\nContent-Length: "
                "0\r\nX-FWVER: " FIRMWARE_VERSION "\r\n\r\n",
                update_url.path,
                update_url.host,
                update_url.port);
  } else {
    DEBUG_MSG("Connection failed before a request could be made.\n");
  }

  status_code = getHttpResult(wifi, do_fw_upgrade);
  if (status_code == 304) {
    DEBUG_MSG("No new firmware version.\n");
  } else if (status_code != 200) {
    DEBUG_MSG("Server returned error: %d\n", status_code);
  }
  wifi.stop();
}

/*
 * Handles serving web interface to client devices.
 */
void
Network::serve_web_interface()
{
  WiFiClient client = web_server.available();
  if (client) {
    char pathbuf[5];
    WriteBufferingStream client_out(client, 64);
    client.setTimeout(HTTP_TIMEOUT);
    DEBUG_MSG("Client connected to web interface.\n");
    if (client.find("GET ")) {
      int pathlen = client.readBytesUntil(' ', pathbuf, 5);
      pathbuf[pathlen] = '\0';
      DEBUG_MSG("Path requested: %s\n", pathbuf);
      // throw out the rest of the content, we only care about the path
      while (client.available()) {
        client.read();
      }
      if (strcmp("/", pathbuf) == 0) {
        // Return the status page
        client_out.print(FPSTR(http_root_header));
        client_out.printf("<li><b>Device ID:</b> %d</li>", ESP.getChipId());
        client_out.print(
          F("<li><b>Firmware version:</b> " FIRMWARE_VERSION "</li>"));
        client_out.printf(
          "<li><b>Last update check:</b> <span class=\"time\">%d</span></li>",
          last_fw_check);
        client_out.print(F("<li><b>Update URL:</b> "));
        if (update_url.set) {
          client_out.printf("http://%s:%d%s</li>",
                            update_url.host,
                            update_url.port,
                            update_url.path);
        } else {
          client_out.print(F("Not set</li>"));
        }
        client_out.print(F("<li><b>Report URL:</b> "));
        if (monitor_config->stats_url.set) {
          client_out.printf("http://%s:%d%s</li>",
                            monitor_config->stats_url.host,
                            monitor_config->stats_url.port,
                            monitor_config->stats_url.path);
        } else {
          client_out.print(F("Not set</li>"));
        }
        client_out.printf("<li><b>Tempuerature sensors:</b> %d</li>",
                          monitor_config->num_therm_sensors);
        client_out.printf("<li><b>Hygrometer: </b>%s</li>",
                          monitor_config->has_sht_sensor ? "Yes" : "No");
        client_out.print(FPSTR(http_root_footer));

      } else if (strcmp("/rb?", pathbuf) == 0) {
        // Reboot device
        client_out.print(FPSTR(http_page_rb));
        client_out.flush();
        client.stop();
        DEBUG_MSG("Restarting.\n");
        ESP.restart();
      } else if (strcmp("/rs?", pathbuf) == 0) {
        // Perfrom a reset
        client_out.print(FPSTR(http_page_rs));
        client_out.flush();
        client.stop();
        if (!LittleFS.format()) {
          DEBUG_MSG("Formatting filesystem failed!\n");
        } else {
          DEBUG_MSG("Resetting.\n");
          ESP.eraseConfig();
          ESP.reset();
        }
      } else {
        client_out.print(FPSTR(http_404_response));
      }
    } else {
      // throw out the rest of the content
      while (client.available()) {
        client.read();
      }
      client_out.print(FPSTR(http_404_response));
    }
    client_out.flush();
    client.stop();
  }
}

/*
 * Posts stats to an endpoint.
 */
void
Network::post_stats(SensorData& readings,
                    byte digital_1,
                    byte digital_2,
                    byte analog)
{
  WiFiClient wifi;
  struct tm* timeinfo;
  char json_buffer[JSONBUF_SIZE];
  size_t json_size;

  // If there are no errors, collect this sample
  if (last_collected.timestamp < readings.timestamp &&
      (!monitor_config->has_sht_sensor ||
       !(readings.humidity.has_error || readings.air_temp.has_error)) &&
      (monitor_config->num_therm_sensors == 0 ||
       !(readings.high_temp.has_error || readings.low_temp.has_error))) {
    last_collected = readings;
    DEBUG_MSG("Caching sensor data at %d:\n  humidity: %.2f\n  air T: "
              "%.2f\n  high T: %.2f\n  low T: %.2f\n",
              last_collected.timestamp,
              last_collected.humidity.value,
              last_collected.air_temp.value,
              last_collected.high_temp.value,
              last_collected.low_temp.value);
  }

  if (readings.timestamp - last_sent < monitor_config->stats_interval ||
      !monitor_config->stats_url.set) {
    return;
  }
  DEBUG_MSG("Sending stats to http://%s:%d%s\n",
            monitor_config->stats_url.host,
            monitor_config->stats_url.port,
            monitor_config->stats_url.path);

  // Deciede what value to send
  SensorData* toSend = &last_collected;
  if (last_collected.timestamp <= last_sent) {
    // If we haven't collected any good values, use the most recent
    // bad one
    toSend = &readings;
    DEBUG_MSG("Last cached value is stale! %d <= %d. Reporting most recent.\n",
              last_collected.timestamp,
              last_sent);
  }

  // populate json buffer
  timeinfo = localtime(&toSend->timestamp);
  json_size = snprintf(
    json_buffer, JSONBUF_SIZE, "{\"id\":%d,\"timestamp\":\"", ESP.getChipId());
  json_size +=
    strftime(json_buffer + json_size, 20, "%Y-%m-%dT%H:%M:%S", timeinfo);

  char high_temp[7];
  char low_temp[7];
  char air_temp[7];
  char humidity[7];
  if (toSend->high_temp.has_error)
    strcpy(high_temp, "null");
  else
    sprintf(high_temp, "%.2f", toSend->high_temp.value);
  if (toSend->low_temp.has_error)
    strcpy(low_temp, "null");
  else
    sprintf(low_temp, "%.2f", toSend->low_temp.value);
  if (toSend->air_temp.has_error)
    strcpy(air_temp, "null");
  else
    sprintf(air_temp, "%.2f", toSend->air_temp.value);
  if (toSend->humidity.has_error)
    strcpy(humidity, "null");
  else
    sprintf(humidity, "%.2f", toSend->humidity.value);
  json_size += snprintf(
    json_buffer + json_size,
    JSONBUF_SIZE - json_size,
    "\",\"high_temp\":%s,\"low_temp\":%s,\"air_temp\":%s,\"humidity\":%s,"
    "\"digital_1\":%d,\"digital_2\":%d,\"analog\":%d}",
    high_temp,
    low_temp,
    air_temp,
    humidity,
    digital_1,
    digital_2,
    analog);

  if (!wifi.connect(monitor_config->stats_url.host,
                    monitor_config->stats_url.port)) {
    DEBUG_MSG("Unable to connect to server.\n");
    return;
  }
  // Send HTTP request
  if (wifi.connected()) {
    WriteBufferingStream bufferedWifi(wifi, 64);
    bufferedWifi.printf("POST %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: "
                        "VivMonitor1.0\r\nConnection: close\r\nContent-type: "
                        "application/json\r\nContent-Length: %d\r\n\r\n",
                        monitor_config->stats_url.path,
                        monitor_config->stats_url.host,
                        monitor_config->stats_url.port,
                        json_size);
    bufferedWifi.write(json_buffer, json_size);
    bufferedWifi.flush();
  } else {
    DEBUG_MSG("Connection failed before a request could be made.\n");
  }

  wifi.stop();
  last_sent = toSend->timestamp;
}
