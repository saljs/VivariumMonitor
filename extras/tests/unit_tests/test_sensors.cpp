#include <Hardware.h>
#include <MockLib.h>
#include <cassert>

void
test_humidity_sensor_read()
{
  Hardware testHarness = Hardware();
  VivariumMonitorConfig config = {
    .has_sht_sensor = true,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };

  // Get mock global Wire obj
  MockLib* MockWireLib = GetMock("Wire");
  assert(MockWireLib != NULL);
  MockWireLib->Reset();

  // Initialize hardware, make sure wire gets set up
  testHarness.init(&config);
  assert(MockWireLib->Called("begin") == 1);
  MockWireLib->Reset();

  // Set up sensor response
  int zero = 0;
  MockWireLib->Returns("endTransmission", 1, &zero);
  int six = 6;
  MockWireLib->Returns("requestFrom", 1, &six);
  uint8_t bytes[6];
  bytes[5] = 0x5F;
  bytes[4] = 0x15;
  bytes[3] = 0x49;
  bytes[2] = 0x72;
  bytes[1] = 0xAF;
  bytes[0] = 0xB1;
  MockWireLib->Returns(
    "read", 6, bytes, bytes + 1, bytes + 2, bytes + 3, bytes + 4, bytes + 5);

  // Check sent command
  int cmd = 0xFD;
  MockWireLib->Expects("write.arg_1", 1, &cmd);

  // Call read_sensors
  SensorData results = testHarness.read_sensors(20);
  assert(MockWireLib->Called("write") == 1);

  // Check our results
  assert(results.timestamp == 20);
  assert(results.air_temp.has_error == false);
  assert(results.air_temp.value < 20.5 && results.air_temp.value > 19.5);
  assert(results.humidity.has_error == false);
  assert(results.humidity.value < 50.5 && results.humidity.value > 49.5);
  assert(results.high_temp.has_error == true);
  assert(results.low_temp.has_error == true);
}

void
test_therm_sensors_read()
{
  Hardware testHarness = Hardware();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 3,
    .sample_interval = 1,
  };

  MockLib* MockTherm = GetMock("DallasTemperature");
  assert(MockTherm != NULL);
  MockTherm->Reset();

  // Initialize the hardware, make sure thermometers are set up
  int three = 3;
  MockTherm->Returns("getDeviceCount", 1, &three);
  bool boolt = true;
  MockTherm->Returns("getAddress", 3, &boolt, &boolt, &boolt);

  testHarness.init(&config);
  assert(MockTherm->Called("begin") == 1);
  assert(MockTherm->Called("setResolution") == 3);

  // Set up sensor response
  int zero = 0, one = 1, two = 2;
  MockTherm->Expects("getTempCByIndex.arg_1", 3, &two, &one, &zero);
  float t1 = 11.0, t2 = 14.0, t3 = 18.0;
  MockTherm->Returns("getTempCByIndex", 3, &t2, &t1, &t3);

  // Call read_sensors
  SensorData results = testHarness.read_sensors(20);

  // Check our results
  assert(results.timestamp == 20);
  assert(results.high_temp.has_error == false);
  assert(results.high_temp.value < 18.5 && results.high_temp.value > 17.5);
  assert(results.low_temp.has_error == false);
  assert(results.low_temp.value < 11.5 && results.low_temp.value > 10.5);
  assert(results.air_temp.has_error == true);
  assert(results.humidity.has_error == true);
}

void
test_i2c_bus_error_handling()
{
  Hardware testHarness = Hardware();
  VivariumMonitorConfig config = {
    .has_sht_sensor = true,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };

  // Get mock global Wire obj
  MockLib* MockWireLib = GetMock("Wire");
  assert(MockWireLib != NULL);
  MockWireLib->Reset();

  // Initialize hardware
  testHarness.init(&config);

  // Set up sensor response
  int four = 4;
  MockWireLib->Returns("endTransmission", 1, &four);

  // Set up low-level i2c stuff
  MockLib* Arduino = GetMock("MockArduino");
  assert(Arduino != NULL);
  Arduino->Reset();

  // Call read_sensors
  SensorData results = testHarness.read_sensors(20);

  // Make sure i2c bus is reset
  assert(Arduino->Called("pinMode") >= 2);

  // Check our results
  assert(results.air_temp.has_error == true);
  assert(results.humidity.has_error == true);

  // Test incomplete read fails
  MockWireLib->Returns("requestFrom", 1, &four);
  results = testHarness.read_sensors(21);

  assert(results.timestamp == 0);
  assert(results.air_temp.has_error == true);
  assert(results.humidity.has_error == true);
}

void
test_sht40_crc_fails()
{
  Hardware testHarness = Hardware();
  VivariumMonitorConfig config = {
    .has_sht_sensor = true,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };

  // Get mock global Wire obj
  MockLib* MockWireLib = GetMock("Wire");
  assert(MockWireLib != NULL);
  MockWireLib->Reset();

  // Initialize hardware
  testHarness.init(&config);

  // Set up sensor response
  int zero = 0;
  MockWireLib->Returns("endTransmission", 1, &zero);
  int six = 6;
  MockWireLib->Returns("requestFrom", 1, &six);
  uint8_t bytes[6];
  bytes[5] = 0x5F;
  bytes[4] = 0x15;
  bytes[3] = 0x49;
  bytes[2] = 0x73; // This crc is off by a bit
  bytes[1] = 0xAF;
  bytes[0] = 0xB1;
  MockWireLib->Returns(
    "read", 6, bytes, bytes + 1, bytes + 2, bytes + 3, bytes + 4, bytes + 5);

  // Call read_sensors
  SensorData results = testHarness.read_sensors(20);

  // Check our results
  assert(results.timestamp == 20);
  assert(results.air_temp.has_error == false);
  assert(results.air_temp.value < 20.5 && results.air_temp.value > 19.5);
  assert(results.humidity.has_error == true);

  // Now try with both failing
  bytes[5] = 0x5E;
  MockWireLib->Returns(
    "read", 6, bytes, bytes + 1, bytes + 2, bytes + 3, bytes + 4, bytes + 5);
  MockWireLib->Returns("requestFrom", 1, &six);
  results = testHarness.read_sensors(21);
  assert(results.timestamp == 20);
  assert(results.air_temp.has_error == true);
  assert(results.humidity.has_error == true);
}

void
test_temp_sensor_bad_value()
{
  Hardware testHarness = Hardware();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 3,
    .sample_interval = 1,
  };

  MockLib* MockTherm = GetMock("DallasTemperature");
  assert(MockTherm != NULL);
  MockTherm->Reset();

  // Initialize the hardware
  int two = 2;
  MockTherm->Returns("getDeviceCount", 1, &two);
  bool boolt = true;
  MockTherm->Returns("getAddress", 2, &boolt, &boolt);
  testHarness.init(&config);

  // Set up sensor response
  float t1 = 11.0, t2 = -180.0;
  MockTherm->Returns("getTempCByIndex", 2, &t2, &t1);

  // Call read_sensors
  SensorData results = testHarness.read_sensors(20);

  // Check our results
  assert(results.timestamp == 0);
  assert(results.high_temp.has_error == true);
  assert(results.low_temp.has_error == true);
}

void
test_sht40_does_heat()
{
  Hardware testHarness = Hardware();
  VivariumMonitorConfig config = {
    .has_sht_sensor = true,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };

  // Get mock global Wire obj
  MockLib* MockWireLib = GetMock("Wire");
  assert(MockWireLib != NULL);

  // Initialize hardware, make sure wire gets set up
  testHarness.init(&config);
  MockWireLib->Reset();

  // Set up sensor response
  int zero = 0;
  MockWireLib->Returns("endTransmission", 1, &zero);
  int six = 6;
  MockWireLib->Returns("requestFrom", 1, &six);
  uint8_t bytes[6];
  bytes[5] = 0x5F;
  bytes[4] = 0x15;
  bytes[3] = 0x49;
  bytes[2] = 0x72;
  bytes[1] = 0xAF;
  bytes[0] = 0xB1;
  MockWireLib->Returns(
    "read", 6, bytes, bytes + 1, bytes + 2, bytes + 3, bytes + 4, bytes + 5);

  // Check sent command
  int cmd = 0xFD;
  MockWireLib->Expects("write.arg_1", 1, &cmd);

  // Call read_sensors
  SensorData results = testHarness.read_sensors(20);
  assert(MockWireLib->Called("write") == 1);

  // Check our results
  assert(results.timestamp == 20);
  assert(results.air_temp.has_error == false);
  assert(results.air_temp.value < 20.5 && results.air_temp.value > 19.5);
  assert(results.humidity.has_error == false);
  assert(results.humidity.value < 50.5 && results.humidity.value > 49.5);

  // Now fast-forward in time and engage heater
  cmd = 0x2F;
  MockWireLib->Expects("write.arg_1", 1, &cmd);

  // Set up new sensor response
  MockWireLib->Returns("endTransmission", 1, &zero);
  MockWireLib->Returns("requestFrom", 1, &six);
  bytes[5] = 0x5F;
  bytes[4] = 0x15;
  bytes[3] = 0x49;
  bytes[2] = 0x2E;
  bytes[1] = 0x15;
  bytes[0] = 0x15;
  MockWireLib->Returns(
    "read", 6, bytes, bytes + 1, bytes + 2, bytes + 3, bytes + 4, bytes + 5);

  SensorData cached = testHarness.read_sensors(500);
  assert(MockWireLib->Called("write") == 2);

  // Check that we get a cached result
  assert(cached.air_temp.has_error == false);
  assert(cached.air_temp.value < 20.5 && cached.air_temp.value > 19.5);
  assert(cached.humidity.has_error == false);
  assert(cached.humidity.value < 50.5 && cached.humidity.value > 49.5);
}

void
test_no_sensors_available()
{
  Hardware testHarness = Hardware();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };

  // Get mock global Wire obj
  MockLib* MockWireLib = GetMock("Wire");
  assert(MockWireLib != NULL);
  MockWireLib->Reset();

  // Get mock therm sensors
  MockLib* MockTherm = GetMock("DallasTemperature");
  assert(MockTherm != NULL);
  MockTherm->Reset();

  // Initialize hardware
  testHarness.init(&config);

  // Call read_sensors
  SensorData results = testHarness.read_sensors(20);

  // Check our results
  assert(results.timestamp == 0);
  assert(results.high_temp.has_error == true);
  assert(results.low_temp.has_error == true);
  assert(results.air_temp.has_error == true);
  assert(results.humidity.has_error == true);
}

void
test_sensors_respect_sample_interval()
{
  Hardware testHarness = Hardware();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 1,
    .sample_interval = 5,
  };

  MockLib* MockTherm = GetMock("DallasTemperature");
  assert(MockTherm != NULL);
  MockTherm->Reset();

  // Initialize the hardware, make sure thermometers are set up
  int one = 1;
  MockTherm->Returns("getDeviceCount", 1, &one);
  bool boolt = true;
  MockTherm->Returns("getAddress", 1, &boolt);

  testHarness.init(&config);
  float t = 20.0;
  MockTherm->Returns("getTempCByIndex", 1, &t);

  // Call read_sensors
  SensorData results = testHarness.read_sensors(20);

  // Check our results
  assert(results.timestamp == 20);
  assert(results.high_temp.has_error == false);
  assert(results.high_temp.value < 20.5 && results.high_temp.value > 19.5);
  assert(results.low_temp.has_error == false);
  assert(results.low_temp.value < 20.5 && results.low_temp.value > 19.5);

  // Call read_sensors again, make sure we don't hit hardware
  SensorData cached = testHarness.read_sensors(21);
  assert(cached.timestamp == 20);
  assert(MockTherm->Called("getTempCByIndex") < 2);
}

void
test_wrong_number_of_sensors()
{
  Hardware testHarness = Hardware();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 5,
    .sample_interval = 1,
  };

  MockLib* MockTherm = GetMock("DallasTemperature");
  assert(MockTherm != NULL);
  MockTherm->Reset();

  // Initialize the hardware, make sure thermometers are set up
  int two = 2;
  MockTherm->Returns("getDeviceCount", 1, &two);
  bool boolt = true;
  MockTherm->Returns("getAddress", 2, &boolt, &boolt);

  testHarness.init(&config);
  // An error message is printed and the number of sensors is updated
  assert(config.num_therm_sensors == 2);
  assert(MockTherm->Called("setResolution") == 2);
}

int
main(void)
{
  test_humidity_sensor_read();
  test_therm_sensors_read();
  test_i2c_bus_error_handling();
  test_sht40_crc_fails();
  test_temp_sensor_bad_value();
  test_sht40_does_heat();
  test_no_sensors_available();
  test_sensors_respect_sample_interval();
  test_wrong_number_of_sensors();
  return 0;
}
