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
DallasTemperature::getAddress(uint8_t* addr, unit8_t arg_1)
{
  MOCK_FUNC_R1(bool, int)
  addr.addr = arg_1;
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
DallasTemperature::getTempC(const uint8_t* arg_1)
{
  MOCK_FUNC_R1(float, int) return 0;
}
