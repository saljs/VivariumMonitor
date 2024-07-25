/*
 * Hardware.cpp
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#include "Hardware.h"
#include "debug.h"

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>

/**********************************************************
   Global vars
 **********************************************************/
struct
{
  byte analog;
  byte digital_1;
  byte digital_2;
  int attempts;
} Outputs;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature thermometers(&oneWire);

void
Hardware::init(VivariumMonitorConfig* config)
{
  monitor_config = config;

  // start i2c interface
  Wire.begin();

  // set up OneWire interface
  thermometers.begin();
  int numTherms = thermometers.getDeviceCount();
  if (numTherms != config->num_therm_sensors) {
    DEBUG_MSG(
      "!!! Expected %d temp sensors, only found %d. Continuing without them\n",
      config->num_therm_sensors,
      numTherms);
  }
  for (int i = 0; i < numTherms; i++) {
    DeviceAddress temp;
    if (thermometers.getAddress(temp, i)) {
      thermometers.setResolution(temp, RESOLUTION);
    }
  }
  DEBUG_MSG("Found %d temp sensor(s).\n", numTherms);

  // Set reading to initial value
  reading.timestamp = 0;
  reading.humidity.has_error = true;
  reading.air_temp.has_error = true;
  reading.high_temp.has_error = true;
  reading.low_temp.has_error = true;

  // set up initial outputs
  Outputs.digital_1 = 0;
  Outputs.digital_2 = 0;
  Outputs.analog = 0;
  Outputs.attempts = 0;
  write_outputs();
}

/**********************************************************
   Helper functions
 **********************************************************/

bool
crc8_check(int value, byte check)
{
  byte crc = 0xFF;
  for (byte bit = 16; bit > 0; bit--) {
    if (((value & 0x8000) >> 8) == (crc & 0x80))
      crc = (crc << 1);
    else
      crc = (crc << 1) ^ 0x31;
    value = (value << 1) & 0xFFFF;
  }
  return crc == check;
}

void
reset_i2c_bus()
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
  if (digitalRead(SCL) == LOW) {
    DEBUG_MSG("I2C bus error. Could not clear sclPin clock line held low\n");
    return;
  }

  // Reset the bus
  while (digitalRead(SDA) == LOW && clockCount > 0) {
    clockCount--;
    // Note: I2C bus is open collector so do NOT drive sclPin or sdaPin high.
    pinMode(
      SCL,
      INPUT); // release sclPin pullup so that when made output it will be LOW
    pinMode(SCL, OUTPUT);       // then clock sclPin Low
    delayMicroseconds(10);      //  for >5us
    pinMode(SCL, INPUT);        // release sclPin LOW
    pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(
      10); //  for >5u so that even the slowest I2C devices are handled.
    //  loop waiting for sclPin to become High only wait 2sec.
    counter = 20;
    while (digitalRead(SCL) == LOW && counter > 0) {
      counter--;
      delay(100);
    }
    if (digitalRead(SCL) == LOW) { // still low after 2 sec error
      DEBUG_MSG("I2C bus error. Could not clear. sclPin clock line held low by "
                "slave clock stretch for >2sec\n");
      return;
    }
  }
  if (digitalRead(SDA) == LOW) {
    DEBUG_MSG("I2C bus error. Could not clear. sdaPin data line held low\n");
    return;
  }
  // else pull sdaPin line low for Start or Repeated Start
  pinMode(SDA, INPUT);  // remove pullup.
  pinMode(SDA, OUTPUT); // and then make it LOW i.e. send an I2C Start or
                        // Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same
  // function as a Stop and clears the bus. A Repeat Start is a Start occurring
  // after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5us
  pinMode(SDA, INPUT);   // remove output low
  pinMode(SDA,
          INPUT_PULLUP); // and make sdaPin high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5us
  pinMode(SDA, INPUT_PULLUP); // Make sdaPin (data) and sclPin (clock) pins
                              // Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

  // Start Wire interface
  Wire.begin();
}

/**********************************************************
   Private functions
 **********************************************************/
bool
Hardware::readSHTsensor(SensorData& output, time_t now)
{
  int bus_status;
  bool use_cache;
  byte cmd = SHT40_READ_CMD;

  if (!monitor_config->has_sht_sensor) {
    output.air_temp.has_error = true;
    output.humidity.has_error = true;
    return false;
  }

  DEBUG_MSG("Reading SHT40 sensor...\n");
  if (now - last_heated >= HEAT_INTERVAL) {
    // send heater command
    last_heated = now;
    cmd = SHT40_HEATER_CMD;
    DEBUG_MSG("Activating heater\n");
  }

  // Give the device 15 sec to cool down after turning on heater
  use_cache = !(cmd == SHT40_HEATER_CMD) && now - last_heated < 15;

  if (!use_cache) {
    // send command
    Wire.beginTransmission(SHT40_ADDRESS);
    Wire.write(cmd);
    bus_status = Wire.endTransmission();
    if (bus_status > 0) {
      DEBUG_MSG("Error requesting data from SHT40! I2c bus error %d.\n",
                bus_status);
      if (bus_status == 4) {
        // status 4 indicates error with bus
        reset_i2c_bus();
      }
      output.air_temp.has_error = true;
      output.humidity.has_error = true;
      return false;
    }
  }

  if (cmd == SHT40_HEATER_CMD || use_cache) {
    DEBUG_MSG("Using cached temp/humidity value.\n");
    return true;
  }

  // attempt to read from sensor
  delay(300);
  byte len = Wire.requestFrom(SHT40_ADDRESS, 6);
  if (len != 6) {
    DEBUG_MSG("Error: SHT40 returned %d bytes, not 6.\n", len);
    output.air_temp.has_error = true;
    output.humidity.has_error = true;
    return false;
  }

  bool hasGoodValue = false;
  // read response into a buffer
  int buff[6];
  for (byte i = 0; i < 6; i++) {
    buff[i] = Wire.read();
  }

  // check tempurature crc
  int t_ticks = (buff[0] << 8) + buff[1];
  byte checksum_t = buff[2];
  if (!crc8_check(t_ticks, checksum_t)) {
    output.air_temp.has_error = true;
    DEBUG_MSG("SHT40 tempurature checksum verification failed!\n");
  } else {
    output.air_temp.has_error = false;
    output.air_temp.value = -45.0 + 175.0 * (float)t_ticks / 65535.0;
    hasGoodValue = true;
  }

  // check humidity crc
  int rh_ticks = (buff[3] << 8) + buff[4];
  byte checksum_rh = buff[5];
  if (!crc8_check(rh_ticks, checksum_rh)) {
    output.humidity.has_error = true;
    DEBUG_MSG("SHT40 humidity checksum verification failed!\n");
  } else {
    output.humidity.has_error = false;
    output.humidity.value = -6.0 + 125.0 * (float)rh_ticks / 65535.0;
    if (output.humidity.value > 100)
      output.humidity.value = 100;
    else if (output.humidity.value < 0)
      output.humidity.value = 0;
    hasGoodValue = true;
  }

  return hasGoodValue;
}

/*
 * Read low and high temps from DS18B20 sensors.
 */
bool
Hardware::readTempSensors(SensorData& output)
{
  DEBUG_MSG("Reading temp sensors...\n");

  // request temps
  thermometers.requestTemperatures();
  delay(300);
  output.high_temp.value = -55;
  output.low_temp.value = 125;
  output.low_temp.has_error = true;
  output.high_temp.has_error = true;

  // loop through the devices on the bus
  int numTherms = thermometers.getDeviceCount();
  for (int i = 0; i < numTherms; i++) {
    float t = thermometers.getTempCByIndex(i);
    if (t < -120) {
      // Large negative values indicate error conditions
      DEBUG_MSG("Error: Temp sensor %d returned error: %0.f\n", i, t);
      return false;
    }
    if (t > output.high_temp.value) {
      output.high_temp.value = t;
    }
    if (t < output.low_temp.value) {
      output.low_temp.value = t;
    }
  }

  if (monitor_config->num_therm_sensors > 0 &&
      monitor_config->num_therm_sensors == numTherms) {
    output.low_temp.has_error = false;
    output.high_temp.has_error = false;
    return true;
  }
  return false;
}

/**********************************************************
   Public functions
 **********************************************************/

void
Hardware::set_analog(byte value)
{
  if (value != Outputs.analog) {
    Outputs.analog = value;
    Outputs.attempts = NUM_SEND_ATTEMPTS;
  }
}

void
Hardware::set_digital_1(byte value)
{
  if (value != Outputs.digital_1) {
    Outputs.digital_1 = value ? 1 : 0;
    Outputs.attempts = NUM_SEND_ATTEMPTS;
  }
}

void
Hardware::set_digital_2(byte value)
{
  if (value != Outputs.digital_2) {
    Outputs.digital_2 = value ? 1 : 0;
    Outputs.attempts = NUM_SEND_ATTEMPTS;
  }
}

void
Hardware::write_outputs()
{
  if (Outputs.attempts > 0) {
    int ret;
    byte payload = Outputs.digital_1 | (Outputs.digital_2 << 1);
    byte cksum = (Outputs.analog & 0x0F) ^ ((Outputs.analog & 0xF0) >> 4) ^
                 (payload & 0x0F);
    payload = payload | (cksum << 4);

    DEBUG_MSG(
      "Writing outputs:\n  ANALOG: %d\n  DIGITAL 1: %d\n  DIGITAL 2: %d\n",
      Outputs.analog,
      Outputs.digital_1,
      Outputs.digital_2);

    // Write out payload
    Wire.beginTransmission(I2C_SLAVE_ADDRESS);
    Wire.write(Outputs.analog);
    Wire.write(payload);
    ret = Wire.endTransmission();

    if (ret == 0) {
      Outputs.attempts--;
    } else {
      DEBUG_MSG("Error updating output controller! I2c bus error %d.\n", ret);
      if (ret == 4) {
        // status 4 indicates error with bus
        reset_i2c_bus();
      }
    }
  }
}

SensorData
Hardware::read_sensors(time_t now)
{
  if (now - reading.timestamp >= monitor_config->sample_interval) {
    bool update_timestamp = readSHTsensor(reading, now);
    update_timestamp |= readTempSensors(reading);
    if (update_timestamp) {
      // If either source updated, update the timestamp
      reading.timestamp = now;
    }
  }
  return reading;
}
