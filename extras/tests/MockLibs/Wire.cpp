#include "Wire.h"

void
WireClass::begin()
{
  MOCK_FUNC_V0
}
void
WireClass::beginTransmission(uint8_t arg_1){
  MOCK_FUNC_V1(uint8_t)
} uint8_t WireClass::endTransmission()
{
  MOCK_FUNC_R0(uint8_t) return 0;
}
uint8_t
WireClass::requestFrom(uint8_t arg_1, uint8_t arg_2)
{
  MOCK_FUNC_R2(uint8_t, uint8_t, uint8_t)
  return 0;
}
uint8_t
WireClass::read()
{
  MOCK_FUNC_R0(uint8_t) return 0;
}
uint8_t
WireClass::write(uint8_t arg_1)
{
  MOCK_FUNC_R1(uint8_t, uint8_t) return 0;
}

WireClass Wire;
