#ifndef DALLASTEMP_H
#define DALLASTEMP_H

#include "MockLib.h"
#include "OneWire.h"

class DallasTemperature : public MockLib
{
public:
  DallasTemperature(OneWire* bus) {}
  std::string GetName() override { return "DallasTemperature"; }
  void begin();
  int getDeviceCount();
  bool getAddress(uint8_t* arg_1, unit8_t arg_2);
  void setResolution(uint8_t arg_1);
  void requestTemperatures();
  float getTempC(const uint8_t* arg_1);
};

#endif
