#include <LittleFS.h>
#include <MockLib.h>
#include <VivariumMonitor.h>
#include <WiFiManager.h>
#include <cassert>

void
test_startup_first_boot()
{
  // On the first boot, the device should boot into a mode that
  // allows you to set the network details and the update url.

  VivariumMonitor underTest;
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };

  // Get mock libraries
  MockLib* MockWiFi = GetMock("WiFiManagerGlobal");
  assert(MockWiFi != NULL);
  MockWiFi->Reset();

  MockLib* MockESP = GetMock("ESP");
  assert(MockESP != NULL);
  MockESP->Reset();

  // Instructs autoConnect to call save callback
  MockWiFi->Returns("autoConnect", 1, NULL);

  // Set config page parameters
  SetWiFiManagerParamValue("update_host", "example.org");
  SetWiFiManagerParamValue("update_port", "8000");
  SetWiFiManagerParamValue("update_path", "/test");

  // Set up mock filesystem
  MockLib* MockFS = GetMock("LittleFS");
  assert(MockFS != NULL);
  MockFS->Reset();

  bool boolt = true;
  File testFile;
  std::vector<std::string> fileContents;
  testFile.AddOutputBuffer(&fileContents);
  MockFS->Returns("begin", 1, &boolt);
  MockFS->Returns("open", 1, &testFile);

  // Run startup
  underTest.init(config);

  assert(MockFS->Called("begin") == 1);
  assert(MockFS->Called("end") == 1);
  assert(MockESP->Called("restart") == 1);
  assert(fileContents[0] == "example.org");
}

void
test_startup_normal_boot()
{
  VivariumMonitor underTest;
  VivariumMonitorConfig config = {
    .has_sht_sensor = false,
    .num_therm_sensors = 0,
    .sample_interval = 1,
  };

  // Get mock libraries
  MockLib* MockWiFi = GetMock("WiFiManagerGlobal");
  assert(MockWiFi != NULL);
  MockWiFi->Reset();

  MockLib* MockESP = GetMock("ESP");
  assert(MockESP != NULL);
  MockESP->Reset();

  // Set up mock filesystem
  MockLib* MockFS = GetMock("LittleFS");
  assert(MockFS != NULL);
  MockFS->Reset();

  bool boolt = true;
  size_t len = sizeof(Url);
  File testFile;
  testFile.Expects("readBytes.arg_1", 1, &len);
  MockFS->Returns("begin", 1, &boolt);
  MockFS->Returns("exists", 1, &boolt);
  MockFS->Returns("open", 1, &testFile);

  // Run startup
  underTest.init(config);

  assert(MockFS->Called("begin") == 1);
  assert(MockFS->Called("exists") == 1);
  assert(MockFS->Called("end") == 1);
  assert(MockESP->Called("restart") == 0);
}

int
main(void)
{
  test_startup_first_boot();
  test_startup_normal_boot();
  return 0;
}
