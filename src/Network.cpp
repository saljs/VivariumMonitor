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
    // ReadBufferingStream bufferedRequest(client, 64);
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
        client.print(HTTP_WEB_ROOT_HEAD);
        client.print("<li><b>Device ID:</b> ");
        client.print(ESP.getChipId());
        client.println("</li>");

        client.print("<li><b>Update server:</b> ");
        client.print(update_url.host);
        client.println("</li>");
        client.print("<li><b>Update file path:</b> ");
        client.print(update_url.path);
        client.println("</li>");

        client.print("<li><b>Free heap:</b> ");
        client.print(ESP.getFreeHeap());
        client.println(" b</li>");
        client.print("<li><b>Heap fragmentation:</b> ");
        client.print(ESP.getHeapFragmentation());
        client.println("</li>");
        client.print(HTTP_WEB_ROOT_FOOTER);

      } else if (strcmp("/rs", pathbuf) == 0) {
        // Perfrom a reset
        client.print(HTTP_WEB_RS);
        client.flush();
        client.stop();
        if (!LittleFS.format()) {
          DEBUG_MSG("Formatting filesystem failed!\n");
        } else {
          DEBUG_MSG("Resetting.\n");
          ESP.eraseConfig();
          ESP.reset();
        }
      } else {
        client.print(HTTP_404_RESPONSE);
      }
    } else {
      // throw out the rest of the content
      while (client.available()) {
        client.read();
      }
      client.print(HTTP_404_RESPONSE);
    }
    client.flush();
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
  time_t timestamp = readings.timestamp;
  struct tm* timeinfo = localtime(&timestamp);
  char json_buffer[JSONBUF_SIZE];
  size_t json_size;

  // If there are no errors, collect this sample
  if ((!monitor_config->has_sht_sensor ||
       !(readings.humidity.has_error || readings.air_temp.has_error)) &&
      (monitor_config->num_therm_sensors == 0 ||
       !(readings.high_temp.has_error || readings.low_temp.has_error))) {
    last_collected = readings;
  }

  if (timestamp - last_sent < monitor_config->stats_interval ||
      !monitor_config->stats_url.set) {
    return;
  }
  DEBUG_MSG("Sending stats to http://%s:%d%s\n",
            monitor_config->stats_url.host,
            monitor_config->stats_url.port,
            monitor_config->stats_url.path);

  // populate json buffer
  json_size = snprintf(
    json_buffer, JSONBUF_SIZE, "{\"id\":%d,\"timestamp\":\"", ESP.getChipId());
  json_size +=
    strftime(json_buffer + json_size, 20, "%Y-%m-%dT%H:%M:%S", timeinfo);

  // Deciede what value to send
  SensorData* toSend = &last_collected;
  if (last_collected.timestamp <= last_sent) {
    // If we haven't collected any good values, use the most recent
    // bad one
    toSend = &readings;
  }

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
