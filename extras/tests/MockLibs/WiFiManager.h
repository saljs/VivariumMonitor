#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include "MockLib.h"
#include <functional>
#include <string>

class WiFiManagerParameter : public MockLib
{
public:
  WiFiManagerParameter(const char* id,
                       const char* label,
                       const char* defaultVal,
                       int length);
  std::string GetName() override { return "WiFiManagerParameter"; }
  const char* getValue();

private:
  std::string paramId;
};

void
SetWiFiManagerParamValue(std::string param_id, const char* value);

class WiFiManager
{
public:
  void addParameter(WiFiManagerParameter* param);
  void autoConnect(const char* ap);
  void setConfigPortalTimeout(int arg_1);
  void setDebugOutput(bool arg_1);
  void setSaveConfigCallback(std::function<void()> func);
};

class WiFiManagerGlobal : public MockLib
{
public:
  std::string GetName() override { return "WiFiManagerGlobal"; }
  void addParameter(WiFiManagerParameter* param);
  void autoConnect(const char* ap);
  void setConfigPortalTimeout(int arg_1);
  void setDebugOutput(bool arg_1);
  void setSaveConfigCallback(std::function<void()> func);

private:
  std::function<void()> saveconfig_callback = NULL;
};

#endif
