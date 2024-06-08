#ifndef WIRE_H
#define WIRE_H

#include "MockLib.h"

class WireClass : public MockLib
{
public:
  std::string GetName() override { return "Wire"; }
  void begin();
  void beginTransmission(uint8_t arg_1);
  uint8_t endTransmission();
  uint8_t requestFrom(uint8_t arg_1, uint8_t arg_2);
  uint8_t read();
  uint8_t write(uint8_t arg_1);
};

extern WireClass Wire;

#endif
