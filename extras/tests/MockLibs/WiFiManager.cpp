#include "WiFiManager.h"

// Global map of param ID -> string value
std::map<std::string, const char*> wifi_param_map;

WiFiManagerParameter::WiFiManagerParameter(const char* id,
                                           const char* label,
                                           const char* defaultVal,
                                           int length)
{
  paramId = std::string(id);
}
const char*
WiFiManagerParameter::getValue()
{
  MOCK_FUNC_R0(const char*)
  if (wifi_param_map.count(paramId) > 0) {
    return wifi_param_map[paramId];
  }
  return "static";
}

void
SetWiFiManagerParamValue(std::string param_id, const char* value)
{
  wifi_param_map[param_id] = value;
}

void
WiFiManagerGlobal::addParameter(WiFiManagerParameter* param)
{
  MOCK_FUNC_V0
}
void
WiFiManagerGlobal::autoConnect(const char* ap)
{
  MOCK_FUNC_V0
  if (saveconfig_callback && returns_map.count("autoConnect") &&
      !returns_map["autoConnect"].empty()) {
    // Call the save config callback function
    saveconfig_callback();
    returns_map["autoConnect"].pop();
  }
}
void
WiFiManagerGlobal::setConfigPortalTimeout(int arg_1)
{
  MOCK_FUNC_V1(int)
}
void
WiFiManagerGlobal::setDebugOutput(bool arg_1)
{
  MOCK_FUNC_V1(bool)
}
void
WiFiManagerGlobal::setSaveConfigCallback(std::function<void()> func)
{
  MOCK_FUNC_V0
  saveconfig_callback = func;
}
void
WiFiManagerGlobal::setCustomHeadElement(const char* el){ MOCK_FUNC_V0 }

// We take advantage of the fact that there is only ever one of these and proxy
// it to a global object
WiFiManagerGlobal GlobalWiFiMan;
void
WiFiManager::addParameter(WiFiManagerParameter* param)
{
  GlobalWiFiMan.addParameter(param);
}
void
WiFiManager::autoConnect(const char* ap)
{
  GlobalWiFiMan.autoConnect(ap);
}
void
WiFiManager::setConfigPortalTimeout(int arg_1)
{
  GlobalWiFiMan.setConfigPortalTimeout(arg_1);
}
void
WiFiManager::setDebugOutput(bool arg_1)
{
  GlobalWiFiMan.setDebugOutput(arg_1);
}
void
WiFiManager::setSaveConfigCallback(std::function<void()> func)
{
  GlobalWiFiMan.setSaveConfigCallback(func);
}
void
WiFiManager::setCustomHeadElement(const char* el)
{
  GlobalWiFiMan.setCustomHeadElement(el);
}
