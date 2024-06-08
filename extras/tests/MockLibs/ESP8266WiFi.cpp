#include "ESP8266WiFi.h"

std::vector<std::string> global_net_log;

WiFiClient::WiFiClient()
{
  AddOutputBuffer(&global_net_log);
}
WiFiClient::WiFiClient(std::vector<std::string>* log)
{
  AddOutputBuffer(log);
}
bool
WiFiClient::connect(const char* host, int arg_1)
{
  MOCK_FUNC_R1(bool, int)
  return true;
}
bool
WiFiClient::connected()
{
  MOCK_FUNC_R0(bool) return true;
}
void
WiFiClient::stop(){ MOCK_FUNC_V0 }

WiFiServer::WiFiServer(int port)
{}
void
WiFiServer::begin(){ MOCK_FUNC_V0 } WiFiClient WiFiServer::available()
{
  MOCK_FUNC_R0(WiFiClient) return WiFiClient();
}

bool
LogHasText(const char* term, std::vector<std::string>* lines)
{
  if (lines == NULL) {
    lines = &global_net_log;
  }
  std::string log = "";
  for (std::string line : *lines) {
    log += line;
  }
  return log.find(term) != std::string::npos;
}

void
ClearGlobalNetLog()
{
  global_net_log.clear();
}
