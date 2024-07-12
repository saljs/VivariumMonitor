#include <ESP8266WiFi.h>
#include <MockLib.h>
#include <Network.h>
#include <cassert>

void
test_initial_bad_sends_nulls(Network& testHarness)
{
  SensorData readings = {
    .humidity = { .has_error = false, .value = 60.0 },
    .air_temp = { .has_error = false, .value = 24.33 },
    .high_temp = { .has_error = true, .value = 255.0 },
    .low_temp = { .has_error = true, .value = -128.0 },
    .timestamp = 10,
  };

  // Call post_stats with bad first reading
  ClearGlobalNetLog();
  testHarness.post_stats(readings, 0, 1, 20);
  assert(LogHasText("POST /statsendpoint HTTP/1.0"));
  assert(LogHasText("Host: test.com:5883"));
  assert(LogHasText("00:10")); // timestamp
  assert(LogHasText("\"high_temp\":null"));
  assert(LogHasText("\"low_temp\":null"));
  assert(LogHasText("\"air_temp\":24.33"));
  assert(LogHasText("\"humidity\":60.00"));
  assert(LogHasText("\"digital_1\":0"));
  assert(LogHasText("\"digital_2\":1"));
  assert(LogHasText("\"analog\":20"));
}

void
test_post_stats_good(Network& testHarness)
{
  MockLib* MockESP = GetMock("ESP");
  assert(MockESP != NULL);

  SensorData readings = {
    .humidity = { .has_error = false, .value = 54.6 },
    .air_temp = { .has_error = false, .value = 22.34 },
    .high_temp = { .has_error = false, .value = 25.0 },
    .low_temp = { .has_error = false, .value = 20.0 },
    .timestamp = 20,
  };
  int id = 12345;
  MockESP->Returns("getChipId", 1, &id);

  // Check we get our readings back
  ClearGlobalNetLog();
  testHarness.post_stats(readings, 0, 1, 36);
  assert(LogHasText("POST"));
  assert(LogHasText("00:20")); // timestamp
  assert(LogHasText("\"id\":12345"));
  assert(LogHasText("\"high_temp\":25.00"));
  assert(LogHasText("\"low_temp\":20.00"));
  assert(LogHasText("\"air_temp\":22.34"));
  assert(LogHasText("\"humidity\":54.60"));
  assert(LogHasText("\"digital_1\":0"));
  assert(LogHasText("\"digital_2\":1"));
  assert(LogHasText("\"analog\":36"));

  // Call post_stats again, but with a short time delta
  readings.timestamp = 22;
  ClearGlobalNetLog();
  testHarness.post_stats(readings, 0, 1, 36);
  assert(!LogHasText("POST"));

  // Try a new reading with enough time
  readings.timestamp = 30;
  readings.high_temp.value = 28.0;
  readings.air_temp.value = 25.67;
  ClearGlobalNetLog();
  testHarness.post_stats(readings, 1, 0, 42);
  assert(LogHasText("00:30")); // timestamp
  assert(LogHasText("\"high_temp\":28.00"));
  assert(LogHasText("\"air_temp\":25.67"));
  assert(LogHasText("\"digital_1\":1"));
  assert(LogHasText("\"digital_2\":0"));
  assert(LogHasText("\"analog\":42"));
}

void
test_get_good_value(Network& testHarness)
{
  SensorData readings = {
    .humidity = { .has_error = true, .value = 60.0 },
    .air_temp = { .has_error = true, .value = 24.33 },
    .high_temp = { .has_error = false, .value = 25.5 },
    .low_temp = { .has_error = false, .value = 20.5 },
    .timestamp = 32,
  };

  // Call post_stats again to feed in some (bad) data
  ClearGlobalNetLog();
  testHarness.post_stats(readings, 0, 1, 100);
  assert(!LogHasText("POST"));

  ClearGlobalNetLog();
  readings.timestamp = 34;
  readings.humidity.value = 70.0;
  testHarness.post_stats(readings, 0, 1, 120);
  assert(!LogHasText("POST"));

  // Now add some good data
  ClearGlobalNetLog();
  readings.timestamp = 36;
  readings.humidity.value = 80.0;
  readings.humidity.has_error = false;
  readings.air_temp.has_error = false;
  testHarness.post_stats(readings, 0, 1, 140);
  assert(!LogHasText("POST"));

  // Finally, add some bad data with a big enough time delta to send
  ClearGlobalNetLog();
  readings.timestamp = 40;
  readings.humidity.value = 90.0;
  readings.humidity.has_error = true;
  readings.air_temp.has_error = true;
  testHarness.post_stats(readings, 1, 0, 160);
  assert(LogHasText("POST"));
  assert(LogHasText("00:36")); // timestamp
  assert(LogHasText("\"high_temp\":25.50"));
  assert(LogHasText("\"low_temp\":20.50"));
  assert(LogHasText("\"air_temp\":24.33"));
  assert(LogHasText("\"humidity\":80.00"));
  assert(LogHasText("\"digital_1\":1"));
  assert(LogHasText("\"digital_2\":0"));
  assert(LogHasText("\"analog\":160"));
}

void
test_sends_nulls(Network& testHarness)
{
  SensorData readings = {
    .humidity = { .has_error = true, .value = 60.0 },
    .air_temp = { .has_error = true, .value = 25.33 },
    .high_temp = { .has_error = false, .value = 26.5 },
    .low_temp = { .has_error = false, .value = 21.5 },
    .timestamp = 45,
  };

  // Call post_stats again to feed in some (bad) data
  ClearGlobalNetLog();
  testHarness.post_stats(readings, 0, 1, 100);
  assert(!LogHasText("POST"));

  // Finally, add some bad data with a big enough time delta to send
  ClearGlobalNetLog();
  readings.timestamp = 50;
  readings.high_temp.value = 27.0;
  testHarness.post_stats(readings, 1, 0, 160);
  assert(LogHasText("POST"));
  assert(LogHasText("00:50")); // timestamp
  assert(LogHasText("\"high_temp\":27.00"));
  assert(LogHasText("\"low_temp\":21.50"));
  assert(LogHasText("\"air_temp\":null"));
  assert(LogHasText("\"humidity\":null"));
  assert(LogHasText("\"digital_1\":1"));
  assert(LogHasText("\"digital_2\":0"));
  assert(LogHasText("\"analog\":160"));
}

void
test_no_post_if_not_configured()
{
  Network testHarness = Network();
  VivariumMonitorConfig config = {
        .has_sht_sensor = false,
        .num_therm_sensors = 0,
        .sample_interval = 1,
        .stats_url = { 
            .set = false,
        },
        .stats_interval = 10,
    };
  Url update_url = { .set = false };

  // Init the library
  testHarness.init(&config, update_url);

  SensorData readings = {
    .humidity = { .has_error = false, .value = 54.6 },
    .air_temp = { .has_error = false, .value = 22.34 },
    .high_temp = { .has_error = false, .value = 25.0 },
    .low_temp = { .has_error = false, .value = 20.0 },
    .timestamp = 20,
  };

  // Check we don't past any data
  ClearGlobalNetLog();
  testHarness.post_stats(readings, 0, 1, 36);
  assert(!LogHasText("POST"));
}

int
main(void)
{
  Network testHarness = Network();
  VivariumMonitorConfig config = {
        .has_sht_sensor = true,
        .num_therm_sensors = 2,
        .sample_interval = 1,
        .stats_url = { 
            .host = "test.com",
            .path = "/statsendpoint",
            .port = 5883,
            .set = true,
        },
        .stats_interval = 10,
    };
  Url update_url = { .set = false };

  // Init the library
  testHarness.init(&config, update_url);

  // Run sequence tests
  test_initial_bad_sends_nulls(testHarness);
  test_post_stats_good(testHarness);
  test_get_good_value(testHarness);
  test_sends_nulls(testHarness);

  // Run standalone tests
  test_no_post_if_not_configured();
  return 0;
}
