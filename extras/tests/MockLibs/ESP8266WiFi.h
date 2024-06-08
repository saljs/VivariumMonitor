#ifndef ESP8266WIFI_H
#define ESP8266WIFI_H

#include "ESP.h"
#include "MockLib.h"
#include "Stream.h"
#include <string>
#include <vector>

class WiFiClient : public Stream
{
public:
  WiFiClient();
  WiFiClient(std::vector<std::string>* log);
  std::string GetName() override { return "WiFiClient"; }
  bool connect(const char* host, int arg_1);
  bool connected();
  void stop();
  bool has_data = false;
  operator bool() const { return has_data; }
};

class WiFiServer : public MockLib
{
public:
  WiFiServer(int port);
  std::string GetName() override { return "WiFiServer"; }
  void begin();
  WiFiClient available();
};

bool
LogHasText(const char* term, std::vector<std::string>* lines = NULL);
void
ClearGlobalNetLog();
#endif
