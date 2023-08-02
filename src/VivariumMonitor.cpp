/*
 * VivariumMonitor.cpp
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#include "VivariumMonitor.h"

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <StreamUtils.h>
#include <LittleFS.h>
#include <Updater.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

/**********************************************************
   Global vars
 **********************************************************/
struct {
  byte analog;
  byte digital_1;
  byte digital_2;
  bool dirty;
} Outputs;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature thermometers(&oneWire);

#if DEBUG_USE_TELNET
ESPTelnet telnet;
#endif

/************************************************************
 * URL update Callback
 ************************************************************/
bool updateUrls = false;
void saveConfigCallback()
{
  updateUrls = true;
}


/**********************************************************
   Public functions
 **********************************************************/
void VivariumMonitor::setDigitalOneHandler(byte (*func)(SensorData)) {
  digital_1_func = func;
}

void VivariumMonitor::setDigitalTwoHandler(byte (*func)(SensorData)) {
  digital_2_func = func;
}

void VivariumMonitor::setAnalogHandler(byte (*func)(SensorData) ) {
  analog_func = func;
}

void VivariumMonitor::init(VivariumMonitorConfig config) {
  WiFiManager wifiManager;
  char rules_port_tmp[6] = "80";

  monitor_config = config;

#if DEBUG_USE_SERIAL
  Serial.begin(9600);
#endif
  
  // start i2c interface
  Wire.begin();

  // set up OneWire interface
  thermometers.begin();
  int numTherms = thermometers.getDeviceCount();
  for (int i = 0; i < numTherms; i++) {
    DeviceAddress temp;
    if (thermometers.getAddress(temp, i)) {
      thermometers.setResolution(temp, RESOLUTION);
    }
  }
  DEBUG_MSG("Found %d temp sensor(s).\n", numTherms);

  // set up initial outputs
  Outputs.digital_1 = 0;
  Outputs.digital_2 = 0;
  Outputs.analog = 0;
  Outputs.dirty = true;

  // set up networking
  configTime(monitor_config.ntp_zone, monitor_config.ntp_server);

  // Connect to WiFi
  WiFiManagerParameter update_host("update_host", "Hostname of configuration server.", update_url.host, CONFIG_STR_LEN);
  wifiManager.addParameter(&update_host);
  WiFiManagerParameter update_port("update_port", "Port to connect on configuration server.", rules_port_tmp, 6);
  wifiManager.addParameter(&update_port);
  WiFiManagerParameter update_path("update_path", "Path of configuration file on server.", update_url.path, CONFIG_STR_LEN);
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
    if (updateUrls && strlen(update_url.host) > 0)
    {
      File urlFile = LittleFS.open(FW_URL_FILE, "w");
      update_url.set = true;
      urlFile.write((char*)(&update_url), sizeof(update_url));
      urlFile.close();
      DEBUG_MSG("Updating update URL file to: http://%s:%d%s\n", update_url.host, update_url.port, update_url.path);
      ESP.restart();
    }
    else if (LittleFS.exists(FW_URL_FILE))
    {
      File urlFile = LittleFS.open(FW_URL_FILE, "r");
      urlFile.readBytes((char*)(&update_url), sizeof(update_url));
      urlFile.close();
      DEBUG_MSG("Update URL: http://%s:%d%s\n", update_url.host, update_url.port, update_url.path);
    }
    else
    {
      update_url.set = false;
    }
    LittleFS.end();
  }
  else 
  {
    update_url.set = false;
    DEBUG_MSG("Error, cannot mount FS! Proceeding without it.\n");
  }

  // Give NTP time to sync 
  //  Not garunteed to do so in three seconds, but in most cases will keep
  //  relays from flickering on startup.
  yield();
  delay(3000);
}

void VivariumMonitor::handle_events() {
  time_t now;
  time(&now);
  if (now > last_read)
  {
    SensorData data;
    last_read = now;

    // Update sensors
    data.timestamp = now;
    if (monitor_config.has_sht_sensor) {
      readSHTsensor(&data);
    }
    else {
      data.air_temp.value = 0;
      data.air_temp.has_error = false;
      data.humidity.value = 0;
      data.humidity.has_error = false;
    }
    if (monitor_config.num_therm_sensors > 0) {
      readTempSensors(&data);
    }
    else {
      data.high_temp.value = 0;
      data.high_temp.has_error = false;
      data.low_temp.value = 0;
      data.low_temp.has_error = false;
    }

    // Update outputs
    set_analog(analog_func(data));
    set_digital_1(digital_1_func(data));
    set_digital_2(digital_2_func(data));
    write_outputs();

    // Check post stats timer
    if (now - last_post_stats >= monitor_config.stats_interval && monitor_config.stats_url.set)
    {
      post_stats(&data);
      last_post_stats = now;
    }

    // Check firmware update timer
    if (now - last_fw_check >= FIRMWARE_CHECK_SECONDS && update_url.set)
    {
      update_firmware();
      last_fw_check = now;
    }
  }
#if DEBUG_USE_TELNET
  telnet.loop();
#endif  
}


/**********************************************************
   Helper functions
 **********************************************************/
int getHttpResult(WiFiClient& wifi, void (*callback)(Stream&, size_t) = NULL)
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
      DEBUG_MSG("Content lenght: %d\n", ret);
    }

    for (byte i = 0; i < 13; i++) {
      buf[i] = buf[i + 1];
    }
    buf[13] = bufferedWifi.read();
    if (buf[13] == ':') {
      isheader = true;
    }
    else if (isheader && buf[13] == '\n') {
      isheader = false;
    }
    else if (!isheader && buf[13] == '\n') {
      break;
    }
  }

  if (ret == 200 && callback) {
    // Run callback function with the buffered stream
    callback(bufferedWifi, len);
  }
  return ret;
}

void do_fw_upgrade(Stream& wifi, size_t len)
{
  size_t written = 0;
  DEBUG_MSG("Beginning firmware upgrade...\n");
  if (!Update.begin(len))
  {
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

bool crc8_check(int value, byte check)
{
  byte crc = 0xFF;
  for (byte bit = 16; bit > 0; bit--)
  {
    if (((value & 0x8000) >> 8) == (crc & 0x80))
      crc = (crc << 1);
    else
      crc = (crc << 1) ^ 0x31;
    value = (value << 1) & 0xFFFF;
  }
  return crc == check;
}

void reset_i2c_bus()
{
  /*
   * Code in this function adapted from:
   *   https://www.forward.com.au/pfod/ArduinoProgramming/I2C_ClearBus/index.html
   *   Written by Matthew Ford, released into open domain.
   */
  int counter, clockCount = 20;

  DEBUG_MSG("Resetting i2c bus...\n");
  // Manually control I2C pins
  pinMode(SDA, INPUT_PULLUP);
  pinMode(SCL, INPUT_PULLUP);

  // Check SCL is not held low
  if (digitalRead(SCL) == LOW)
  {
    DEBUG_MSG("I2C bus error. Could not clear sclPin clock line held low\n");
    return;
  }

  // Reset the bus
  while (digitalRead(SDA) == LOW && clockCount > 0)
  {
    clockCount--;
    // Note: I2C bus is open collector so do NOT drive sclPin or sdaPin high.
    pinMode(SCL, INPUT); // release sclPin pullup so that when made output it will be LOW
    pinMode(SCL, OUTPUT); // then clock sclPin Low
    delayMicroseconds(10); //  for >5us
    pinMode(SCL, INPUT); // release sclPin LOW
    pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5u so that even the slowest I2C devices are handled.
    //  loop waiting for sclPin to become High only wait 2sec.
    counter = 20;
    while (digitalRead(SCL) == LOW && counter > 0) {
      counter--;
      delay(100);
    }
    if (digitalRead(SCL) == LOW) { // still low after 2 sec error
      DEBUG_MSG("I2C bus error. Could not clear. sclPin clock line held low by slave clock stretch for >2sec\n");
      return;
    }
  }
  if (digitalRead(SDA) == LOW)
  {
      DEBUG_MSG("I2C bus error. Could not clear. sdaPin data line held low\n");
      return;
  }
  // else pull sdaPin line low for Start or Repeated Start
  pinMode(SDA, INPUT); // remove pullup.
  pinMode(SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  // A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5us
  pinMode(SDA, INPUT); // remove output low
  pinMode(SDA, INPUT_PULLUP); // and make sdaPin high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5us
  pinMode(SDA, INPUT_PULLUP); // Make sdaPin (data) and sclPin (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

  // Start Wire interface
  Wire.begin();
}

/**********************************************************
   Private functions
 **********************************************************/
void VivariumMonitor::readSHTsensor(SensorData* output)
{
  int bus_status;
  bool use_cache;
  byte cmd = SHT40_READ_CMD;
  DEBUG_MSG("Reading SHT40 sensor...\n");
  if (millis() - last_heated >= HEAT_INTERVAL) {
    //send heater command
    last_heated = millis();
    cmd = SHT40_HEATER_CMD;
    DEBUG_MSG("Activating heater\n");
  }

  // Give the device 15 sec to cool down after turning on heater
  use_cache = !(cmd == SHT40_HEATER_CMD) && millis() - last_heated < 15000 && last_heated > 15000;
  
  if (!use_cache)
  {
    // send command
    Wire.beginTransmission(SHT40_ADDRESS);
    Wire.write(cmd);
    bus_status = Wire.endTransmission();
    if (bus_status > 0)
    {
      output->humidity.has_error = true;
      output->humidity.value = 0;
      output->air_temp.has_error = true;
      output->air_temp.value = 0;
      DEBUG_MSG("Error requesting data from SHT40! I2c bus error %d.\n", bus_status);
      if (bus_status == 4)
      {
        // status 4 indicates error with bus
        reset_i2c_bus();
      }
      return;
    }
  }
  
  if(cmd == SHT40_HEATER_CMD || use_cache)
  {
    DEBUG_MSG("Using cached temp/humidity value.\n");
    output->humidity.has_error = false;
    output->humidity.value = last_humidity.value;
    output->air_temp.has_error = false;
    output->air_temp.value = last_air_temp.value;
    return;
  }

  //attempt to read from sensor
  delay(300);
  byte len = Wire.requestFrom(SHT40_ADDRESS, 6);
  if (len != 6)
  {
    output->humidity.has_error = true;
    output->humidity.value = 0;
    output->air_temp.has_error = true;
    output->air_temp.value = 0;
    DEBUG_MSG("Error: SHT40 returned %d bytes, not 6.\n", len);
    return;
  }

  // read response into a buffer
  int buff[6];
  for (byte i = 0; i < 6; i++)
  {
    buff[i] = Wire.read();
  }

  // check tempurature crc
  int t_ticks = (buff[0] << 8) + buff[1];
  byte checksum_t = buff[2];
  if (!crc8_check(t_ticks, checksum_t))
  {
    output->air_temp.has_error = true;
    output->air_temp.value = 0;
    DEBUG_MSG("SHT40 tempurature checksum verification failed!\n");
  }
  else
  {
    output->air_temp.has_error = false;
    output->air_temp.value = -45.0 + 175.0 * (float)t_ticks / 65535.0;
  }

  // check humidity crc
  int rh_ticks = (buff[3] << 8) + buff[4];
  byte checksum_rh = buff[5];
  if (!crc8_check(rh_ticks, checksum_rh))
  {
    output->humidity.has_error = true;
    output->humidity.value = 0;
    DEBUG_MSG("SHT40 humidity checksum verification failed!\n");
  }
  else
  {
    output->humidity.has_error = false;
    output->humidity.value = -6.0 + 125.0 * (float)rh_ticks / 65535.0;
    if (output->humidity.value > 100) output->humidity.value = 100;
    else if (output->humidity.value < 0)  output->humidity.value = 0;
  }

  // Update cached values
  if (!output->humidity.has_error) {
    last_humidity.value = output->humidity.value;
  }
  if (!output->air_temp.has_error) {
    last_air_temp.value = output->air_temp.value;
  }
}

/*
 * Read low and high temps from DS18B20 sensors.
 */
void VivariumMonitor::readTempSensors(SensorData* output)
{
  DEBUG_MSG("Reading temp sensors...\n");

  //request temps
  thermometers.requestTemperatures();
  output->high_temp.value = -55;
  output->low_temp.value = 125;

  //loop through the devices on the bus
  for (int i = 0; i < monitor_config.num_therm_sensors; i++) {
    float t = thermometers.getTempCByIndex(i);
    if (t < -120) {
      // Large negative values indicate error conditions
      DEBUG_MSG("Error: Temp sensor %d returned error: %0.f\n", i, t);
      output->high_temp.has_error = true;
      output->high_temp.value = 0;
      output->low_temp.has_error = true;
      output->low_temp.value = 0;
      return;
    }
    if (t > output->high_temp.value) {
      output->high_temp.value = t;
    }
    if (t < output->low_temp.value) {
      output->low_temp.value = t;
    }
  }
  output->low_temp.has_error = false;
  output->high_temp.has_error = false;
}

/*
   Checks if an updated firmware is available, and upgrades if so
*/
void VivariumMonitor::update_firmware()
{
  WiFiClient wifi;
  int status_code;
  DEBUG_MSG("Checking for updates...\n");
  if (! wifi.connect(update_url.host, update_url.port))
  {
    DEBUG_MSG("Unable to connect to server.\n");
    return;
  }
  // Send HTTP request
  if (wifi.connected()) {
    wifi.printf("GET %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: VivMonitor1.0\r\nConnection: close\r\nContent-Length: 0\r\nX-FWVER: " FIRMWARE_VERSION "\r\n\r\n",
                        update_url.path, update_url.host, update_url.port);
  }
  else {
    DEBUG_MSG("Connection failed before a request could be made.\n");
  }

  status_code = getHttpResult(wifi, do_fw_upgrade);
  if (status_code == 304)
  {
    DEBUG_MSG("No new firmware version.\n");
  }
  else if (status_code != 200)
  {
    DEBUG_MSG("Server returned error: %d\n", status_code);
  }
  wifi.stop();
}

/*
   Posts stats to an endpoint.
*/
void VivariumMonitor::post_stats(SensorData* readings)
{
  WiFiClient wifi;
  time_t timestamp = readings->timestamp;
  struct tm* timeinfo = localtime(&timestamp);
  char json_buffer[164];
  size_t json_size;
  DEBUG_MSG("Sending stats to http://%s:%d%s\n", monitor_config.stats_url.host, monitor_config.stats_url.port, monitor_config.stats_url.path);

  // populate json buffer
  json_size = sprintf(json_buffer, "{\"id\":%d,\"timestamp\":\"", ESP.getChipId());
  json_size += strftime(json_buffer + json_size, 20, "%Y-%m-%dT%H:%M:%S", timeinfo);
  json_size += sprintf(json_buffer + json_size, "\",\"high_temp\":%.2f,\"low_temp\":%.2f,\"air_temp\":%.2f,\"humidity\":%.2f,\"digital_1\":%d,\"digital_2\":%d,\"analog\":%d}",
                       readings->high_temp.value, readings->low_temp.value, readings->air_temp.value,
                       readings->humidity.value, Outputs.digital_1, Outputs.digital_2, Outputs.analog);

  if (! wifi.connect(monitor_config.stats_url.host, monitor_config.stats_url.port))
  {
    DEBUG_MSG("Unable to connect to server.\n");
    return;
  }
  // Send HTTP request
  if (wifi.connected()) {
    WriteBufferingStream bufferedWifi(wifi, 64);
    bufferedWifi.printf("POST %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: VivMonitor1.0\r\nConnection: close\r\nContent-type: application/json\r\nContent-Length: %d\r\n\r\n",
                        monitor_config.stats_url.path, monitor_config.stats_url.host, monitor_config.stats_url.port, json_size);
    bufferedWifi.write(json_buffer, json_size);
    bufferedWifi.flush();
  }
  else {
    DEBUG_MSG("Connection failed before a request could be made.\n");
  }
  wifi.stop();
}

void VivariumMonitor::set_analog(byte value)
{
  if (value != Outputs.analog)
  {
    Outputs.analog = value;
    Outputs.dirty = true;
  }
}

void VivariumMonitor::set_digital_1(byte value)
{
  if (value != Outputs.digital_1)
  {
    Outputs.digital_1 = value ? 1 : 0;
    Outputs.dirty = true;
  }
}

void VivariumMonitor::set_digital_2(byte value)
{
  if (value != Outputs.digital_2)
  {
    Outputs.digital_2 = value ? 1 : 0;
    Outputs.dirty = true;
  }
}

void VivariumMonitor::write_outputs()
{
  if (Outputs.dirty)
  {
    int ret;
    byte payload = Outputs.digital_1 | (Outputs.digital_2 << 1);
    byte cksum = (Outputs.analog & 0x0F) ^ ((Outputs.analog & 0xF0) >> 4) ^ (payload & 0x0F);
    payload = payload | (cksum << 4);

    DEBUG_MSG("Writing outputs:\n  ANALOG: %d\n  DIGITAL 1: %d\n  DIGITAL 2: %d\n", Outputs.analog, Outputs.digital_1, Outputs.digital_2);

    // Write out payload
    Wire.beginTransmission(I2C_SLAVE_ADDRESS);
    Wire.write(Outputs.analog);
    Wire.write(payload);
    ret = Wire.endTransmission();

    if (ret == 0)
    {
      Outputs.dirty = false;
    }
    else
    {
      DEBUG_MSG("Error updating output controller! I2c bus error %d.\n", ret);
      if (ret == 4)
      {
          // status 4 indicates error with bus
          reset_i2c_bus();
      }
    }
  }
}
