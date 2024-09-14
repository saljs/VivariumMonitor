#include <ESP8266WiFi.h>
#include <MockLib.h>
#include <Network.h>
#include <cassert>
#include <string>
#include <vector>

void
test_serves_correct_update_path()
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
    .port = 80,
    .set = true,
  };

  // Get mock web server
  MockLib* MockWebServer = GetMock("WiFiServer");
  assert(MockWebServer != NULL);
  MockWebServer->Reset();

  // Init the library
  testHarness.init(&config, update_url);
  assert(MockWebServer->Called("begin") == 1);

  // get mock ESP library
  MockLib* MockESP = GetMock("ESP");
  assert(MockESP != NULL);
  MockESP->Reset();

  // Make a mock (non) request
  std::vector<std::string> netOut;
  WiFiClient request = WiFiClient(&netOut);
  request.has_data = false;
  MockWebServer->Returns("available", 1, &request);
  testHarness.serve_web_interface();
  assert(netOut.empty());

  // Now add some data
  bool boolt = true;
  int one = 1;
  request.has_data = true;
  MockWebServer->Returns("available", 1, &request);
  request.Returns("find", 1, &boolt);
  request.Expects("readBytesUntil.buffer", 1, "/");
  request.Returns("readBytesUntil", 1, &one);

  // Serve webpage and check output
  testHarness.serve_web_interface();
  assert(LogHasText("HTTP/1.0 200 OK\r\n", &netOut));
  assert(LogHasText("<b>Update URL:</b> http://example.org:80/test", &netOut));

  // Check that we did NOT reset the device
  assert(MockESP->Called("eraseConfig") == 0);
  assert(MockESP->Called("reset") == 0);
}

void
test_bad_path_404()
{
  Network testHarness = Network();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };
  Url update_url = { .host = "example.org", .path = "/test", .port = 80 };

  // Get mock web server
  MockLib* MockWebServer = GetMock("WiFiServer");
  assert(MockWebServer != NULL);
  MockWebServer->Reset();

  // Init the library
  testHarness.init(&config, update_url);

  // get mock ESP library
  MockLib* MockESP = GetMock("ESP");
  assert(MockESP != NULL);
  MockESP->Reset();

  // Make a bad request
  bool boolt = true;
  int four = 4;
  std::vector<std::string> netOut;
  WiFiClient request = WiFiClient(&netOut);
  request.has_data = true;
  MockWebServer->Returns("available", 1, &request);
  request.Returns("find", 1, &boolt);
  request.Expects("readBytesUntil.buffer", 1, "/bad");
  request.Returns("readBytesUntil", 1, &four);

  // Serve webpage and check output
  testHarness.serve_web_interface();
  assert(LogHasText("HTTP/1.0 404 NOT FOUND\r\n", &netOut));

  // Check that we did NOT reset the device
  assert(MockESP->Called("eraseConfig") == 0);
  assert(MockESP->Called("reset") == 0);
}

void
test_reset()
{
  Network testHarness = Network();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };
  Url update_url = { .host = "example.org", .path = "/test", .port = 80 };

  // Get mock web server
  MockLib* MockWebServer = GetMock("WiFiServer");
  assert(MockWebServer != NULL);
  MockWebServer->Reset();

  // Init the library
  testHarness.init(&config, update_url);

  // get mock ESP library
  MockLib* MockESP = GetMock("ESP");
  assert(MockESP != NULL);
  MockESP->Reset();

  // Make a reset request
  bool boolt = true;
  int four = 4;
  std::vector<std::string> netOut;
  WiFiClient request = WiFiClient(&netOut);
  request.has_data = true;
  MockWebServer->Returns("available", 1, &request);
  request.Returns("find", 1, &boolt);
  request.Expects("readBytesUntil.buffer", 1, "/rs?");
  request.Returns("readBytesUntil", 1, &four);

  // Serve webpage and check output
  testHarness.serve_web_interface();
  assert(LogHasText("HTTP/1.0 200 OK\r\n", &netOut));
  assert(LogHasText("Node has been reset", &netOut));

  // Check that we reset the device
  assert(MockESP->Called("eraseConfig") == 1);
  assert(MockESP->Called("reset") == 1);
}

void
test_restart()
{
  Network testHarness = Network();
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };
  Url update_url = { .host = "example.org", .path = "/test", .port = 80 };

  // Get mock web server
  MockLib* MockWebServer = GetMock("WiFiServer");
  assert(MockWebServer != NULL);
  MockWebServer->Reset();

  // Init the library
  testHarness.init(&config, update_url);

  // get mock ESP library
  MockLib* MockESP = GetMock("ESP");
  assert(MockESP != NULL);
  MockESP->Reset();

  // Make a reset request
  bool boolt = true;
  int four = 4;
  std::vector<std::string> netOut;
  WiFiClient request = WiFiClient(&netOut);
  request.has_data = true;
  MockWebServer->Returns("available", 1, &request);
  request.Returns("find", 1, &boolt);
  request.Expects("readBytesUntil.buffer", 1, "/rb?");
  request.Returns("readBytesUntil", 1, &four);

  // Serve webpage and check output
  testHarness.serve_web_interface();
  assert(LogHasText("HTTP/1.0 200 OK\r\n", &netOut));
  assert(LogHasText("Node has been reboot", &netOut));

  // Check that we restart the device without a reset
  assert(MockESP->Called("eraseConfig") == 0);
  assert(MockESP->Called("reset") == 0);
  assert(MockESP->Called("restart") == 1);
}

int
main(void)
{
  test_serves_correct_update_path();
  test_bad_path_404();
  test_reset();
  test_restart();
  return 0;
}
