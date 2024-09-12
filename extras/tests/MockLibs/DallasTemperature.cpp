#include "DallasTemperature.h"

void
DallasTemperature::begin()
{
  MOCK_FUNC_V0
}
int
DallasTemperature::getDeviceCount()
{
  MOCK_FUNC_R0(int) return 0;
}
bool
DallasTemperature::getAddress(uint8_t* addr, uint8_t arg_1)
{
  MOCK_FUNC_R1(bool, int)
  *addr = arg_1;
  return true;
}
void
DallasTemperature::setResolution(uint8_t arg_1)
{
  MOCK_FUNC_V1(int)
}
void
DallasTemperature::requestTemperatures()
{
  MOCK_FUNC_V0
}
float
DallasTemperature::getTempC(const uint8_t* addr)
{
  MOCK_FUNC_R0(float) return 0;
}
