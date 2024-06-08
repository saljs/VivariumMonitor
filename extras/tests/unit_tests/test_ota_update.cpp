#include <ESP8266WiFi.h>
#include <MockLib.h>
#include <Network.h>
#include <StreamUtils.h>
#include <cassert>

void
test_ota_has_update_good()
{
  Network testHarness = Network();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };
  Url update_url = {
    .host = "example.org",
    .path = "/test",
    .port = 8000,
    .set = true,
  };

  // Init the library
  testHarness.init(&config, update_url);

  // Mock the "server" response
  MockLib *MockESP = GetMock("ESP"), *MockUpdate = GetMock("Update");
  assert(MockESP != NULL);
  assert(MockUpdate != NULL);
  MockESP->Reset();
  MockUpdate->Reset();
  size_t len = 512;
  bool boolt = true;
  MockUpdate->Expects("begin.arg_1", 1, &len);
  MockUpdate->Returns("begin", 1, &boolt);
  MockUpdate->Returns("writeStream", 1, &len);
  MockUpdate->Returns("end", 1, &boolt);
  SetGlobalInputStream("HTTP/1.2 200 OK\r\n"
                       "Connection: close\r\n"
                       "Content-Length: 512\r\n"
                       "Content-Type: application/octet\r\n\r\nbody");

  ClearGlobalNetLog();
  testHarness.update_firmware(20000);
  assert(LogHasText("GET /test HTTP/1.0\r\n"));
  assert(LogHasText("Host: example.org:8000"));
  assert(LogHasText("X-FWVER: unittest"));
  assert(MockUpdate->Called("begin") == 1);
  assert(MockUpdate->Called("writeStream") == 1);
  assert(MockUpdate->Called("end") == 1);
  assert(MockESP->Called("restart") == 1);

  // Make sure we wait to check again
  ClearGlobalNetLog();
  testHarness.update_firmware(20010);
  assert(!LogHasText("GET"));
  assert(MockUpdate->Called("begin") == 1);
  assert(MockUpdate->Called("end") == 1);
}

void
test_ota_update_not_available()
{
  Network testHarness = Network();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };
  Url update_url = {
    .host = "example.org",
    .path = "/test",
    .port = 8000,
    .set = true,
  };

  // Init the library
  testHarness.init(&config, update_url);

  // Mock the "server" response
  MockLib *MockESP = GetMock("ESP"), *MockUpdate = GetMock("Update");
  assert(MockESP != NULL);
  assert(MockUpdate != NULL);
  MockESP->Reset();
  MockUpdate->Reset();
  SetGlobalInputStream(
    "HTTP/1.2 304 Not Modified\r\nConnection: close\r\n\r\n");

  ClearGlobalNetLog();
  testHarness.update_firmware(20000);
  assert(LogHasText("GET /test HTTP/1.0\r\n"));
  assert(LogHasText("Host: example.org:8000"));
  assert(LogHasText("X-FWVER: unittest"));
  assert(MockUpdate->Called("begin") == 0);
  assert(MockESP->Called("restart") == 0);

  // Make sure we wait to check again
  ClearGlobalNetLog();
  testHarness.update_firmware(20010);
  assert(!LogHasText("GET"));
  assert(MockUpdate->Called("begin") == 0);
  assert(MockUpdate->Called("end") == 0);
}

void
test_ota_update_fails_to_start()
{
  Network testHarness = Network();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };
  Url update_url = {
    .host = "example.org",
    .path = "/test",
    .port = 8000,
    .set = true,
  };

  // Init the library
  testHarness.init(&config, update_url);

  // Mock the "server" response
  MockLib *MockESP = GetMock("ESP"), *MockUpdate = GetMock("Update");
  assert(MockESP != NULL);
  assert(MockUpdate != NULL);
  MockESP->Reset();
  MockUpdate->Reset();
  bool boolf = false;
  MockUpdate->Returns("begin", 1, &boolf);
  SetGlobalInputStream("HTTP/1.2 200 OK\r\n"
                       "Connection: close\r\n"
                       "Content-Length: 512\r\n"
                       "Content-Type: application/octet\r\n\r\nbody");

  ClearGlobalNetLog();
  testHarness.update_firmware(20000);
  assert(MockUpdate->Called("begin") == 1);
  assert(MockUpdate->Called("writeStream") == 0);
  assert(MockUpdate->Called("end") == 0);
  assert(MockESP->Called("restart") == 0);
}

void
test_ota_update_fails_no_space()
{
  Network testHarness = Network();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };
  Url update_url = {
    .host = "example.org",
    .path = "/test",
    .port = 8000,
    .set = true,
  };

  // Init the library
  testHarness.init(&config, update_url);

  // Mock the "server" response
  MockLib *MockESP = GetMock("ESP"), *MockUpdate = GetMock("Update");
  assert(MockESP != NULL);
  assert(MockUpdate != NULL);
  MockESP->Reset();
  MockUpdate->Reset();
  bool boolt = true;
  size_t len = 434;
  MockUpdate->Returns("begin", 1, &boolt);
  MockUpdate->Returns("writeStream", 1, &len);
  SetGlobalInputStream("HTTP/1.2 200 OK\r\n"
                       "Connection: close\r\n"
                       "Content-Length: 512\r\n"
                       "Content-Type: application/octet\r\n\r\nbody");

  ClearGlobalNetLog();
  testHarness.update_firmware(20000);
  assert(MockUpdate->Called("begin") == 1);
  assert(MockUpdate->Called("writeStream") == 1);
  assert(MockUpdate->Called("end") == 0);
  assert(MockESP->Called("restart") == 0);
}

void
test_ota_update_fails_to_finalize()
{
  Network testHarness = Network();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };
  Url update_url = {
    .host = "example.org",
    .path = "/test",
    .port = 8000,
    .set = true,
  };

  // Init the library
  testHarness.init(&config, update_url);

  // Mock the "server" response
  MockLib *MockESP = GetMock("ESP"), *MockUpdate = GetMock("Update");
  assert(MockESP != NULL);
  assert(MockUpdate != NULL);
  MockESP->Reset();
  MockUpdate->Reset();
  bool boolt = true, boolf = false;
  size_t len = 512;
  MockUpdate->Returns("begin", 1, &boolt);
  MockUpdate->Returns("writeStream", 1, &len);
  MockUpdate->Returns("end", 1, &boolf);
  SetGlobalInputStream("HTTP/1.2 200 OK\r\n"
                       "Connection: close\r\n"
                       "Content-Length: 512\r\n"
                       "Content-Type: application/octet\r\n\r\nbody");

  ClearGlobalNetLog();
  testHarness.update_firmware(20000);
  assert(MockUpdate->Called("begin") == 1);
  assert(MockUpdate->Called("writeStream") == 1);
  assert(MockUpdate->Called("end") == 1);
  assert(MockESP->Called("restart") == 0);
}

int
main(void)
{
  test_ota_has_update_good();
  test_ota_update_not_available();
  test_ota_update_fails_to_start();
  test_ota_update_fails_no_space();
  test_ota_update_fails_to_finalize();
  return 0;
}
