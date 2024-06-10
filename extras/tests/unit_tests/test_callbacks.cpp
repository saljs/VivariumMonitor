#include <LittleFS.h>
#include <MockLib.h>
#include <VivariumMonitor.h>
#include <WiFiManager.h>
#include <cassert>

time_t start_time = 0, current_time = 0;

byte
test_handler_callback(SensorData reading, time_t now)
{
  if (start_time == 0) {
    start_time = now;
  }
  current_time = now;
  // Here's where we check that the same sensor data keeps being
  // sent until it's resampled.
  assert(reading.timestamp == start_time);
  return 0;
}
void
test_event_handler_called()
{
  VivariumMonitor underTest;
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 1,
    .sample_interval = 2,
  };

  // Set our mock handler
  underTest.setAnalogHandler(test_handler_callback);

  // Set up mock sensors
  MockLib* MockTherm = GetMock("DallasTemperature");
  assert(MockTherm != NULL);
  MockTherm->Reset();

  int one = 1;
  float t = 15.0;
  bool boolt = true;
  MockTherm->Returns("getDeviceCount", 1, &one);
  MockTherm->Returns("getAddress", 1, &boolt);
  MockTherm->Returns("getTempCByIndex", 1, &t);

  // Set up mock filesystem
  MockLib* MockFS = GetMock("LittleFS");
  assert(MockFS != NULL);
  MockFS->Reset();

  File testFile;
  MockFS->Returns("begin", 1, &boolt);
  MockFS->Returns("exists", 1, &boolt);
  MockFS->Returns("open", 1, &testFile);

  // Run startup
  underTest.init(config);

  // Run handler until time changes
  while (start_time == current_time) {
    underTest.handle_events();
  }
  assert(MockTherm->Called("getTempCByIndex") == 1);
}

int
main(void)
{
  test_event_handler_called();
  return 0;
}
