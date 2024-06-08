#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <MockLib.h>
#include <StreamUtils.h>
#include <VivariumMonitor.h>
#include <WiFiManager.h>
#include <cassert>
#include <chrono>
#include <thread>

void
test_sends_stats_and_update(VivariumMonitor& underTest)
{
  MockLib* MockTherm = GetMock("DallasTemperature");
  assert(MockTherm != NULL);
  MockTherm->Reset();

  // Set up sensor response
  float t1 = 11.0, t2 = 18.0;
  MockTherm->Returns("getTempCByIndex", 2, &t2, &t1);

  // Set reponse from update server
  SetGlobalInputStream(
    "HTTP/1.2 304 Not Modified\r\nConnection: close\r\n\r\n");

  // Call event handler
  ClearGlobalNetLog();
  underTest.handle_events();

  // Check we posted stats
  assert(LogHasText("POST /stats/post HTTP/1.0"));

  // Check we queried for update
  assert(LogHasText("GET /test HTTP/1.0\r\n"));
}

void
test_respects_intervals(VivariumMonitor& underTest)
{
  MockLib* MockTherm = GetMock("DallasTemperature");
  assert(MockTherm != NULL);
  MockTherm->Reset();

  // Call event handler
  ClearGlobalNetLog();
  underTest.handle_events();

  // Check that sensors were not queried, no data were sent
  assert(MockTherm->Called("getTempCByIndex") == 0);
  assert(!LogHasText("POST"));
  assert(!LogHasText("GET"));
}

void
test_sends_stats_again(VivariumMonitor& underTest)
{
  MockLib* MockTherm = GetMock("DallasTemperature");
  assert(MockTherm != NULL);
  MockTherm->Reset();

  // Set up sensor response
  float t1 = 12.0, t2 = 17.0;
  MockTherm->Returns("getTempCByIndex", 2, &t2, &t1);

  // Call event handler
  ClearGlobalNetLog();
  underTest.handle_events();

  // Check we posted stats with new reading
  assert(LogHasText("POST /stats/post HTTP/1.0"));
  assert(LogHasText("\"high_temp\":17.00"));
  assert(MockTherm->Called("getTempCByIndex") == 2);
}

int
main(void)
{
  using namespace std::this_thread;
  using namespace std::chrono_literals;
  VivariumMonitor underTest;
  VivariumMonitorConfig config = {
        .has_sht_sensor = false, // So we don't have to mock i2c
        .num_therm_sensors = 2,
        .sample_interval = 1,
        .stats_url = {
            .host = "test.com",
            .path = "/stats/post",
            .port = 8083,
            .set = true,
        },
        .stats_interval = 1,
    };
  MockLib* MockWiFi = GetMock("WiFiManagerGlobal");
  assert(MockWiFi != NULL);
  MockWiFi->Reset();

  // Set config page parameters
  SetWiFiManagerParamValue("update_host", "example.org");
  SetWiFiManagerParamValue("update_port", "8000");
  SetWiFiManagerParamValue("update_path", "/test");

  // Set up mock filesystem
  MockLib* MockFS = GetMock("LittleFS");
  assert(MockFS != NULL);
  MockFS->Reset();

  bool boolt = true;
  size_t len = sizeof(Url);
  Url update_url = {
    .host = "example.org",
    .path = "/test",
    .port = 8000,
    .set = true,
  };
  File testFile;
  testFile.Expects("readBytes.buffer", 1, &update_url);
  MockFS->Returns("begin", 1, &boolt);
  MockFS->Returns("exists", 1, &boolt);
  MockFS->Returns("open", 1, &testFile);

  // Initialize the hardware, make sure thermometers are set up
  MockLib* MockTherm = GetMock("DallasTemperature");
  assert(MockTherm != NULL);
  MockTherm->Reset();

  int two = 2;
  MockTherm->Returns("getDeviceCount", 1, &two);
  MockTherm->Returns("getAddress", 2, &boolt, &boolt);

  // Setup
  underTest.init(config);

  // Run tests
  test_sends_stats_and_update(underTest);
  test_respects_intervals(underTest);
  sleep_for(1s);
  test_sends_stats_again(underTest);

  return 0;
}
