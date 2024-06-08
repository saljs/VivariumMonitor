#include "DallasTemperature.h"

DeviceAddress::DeviceAddress(int addr)
{
  this->addr = addr;
}
bool
DeviceAddress::operator==(const DeviceAddress& rhs) const
{
  return rhs.addr == addr;
}
bool
DeviceAddress::operator!=(const DeviceAddress& rhs) const
{
  return !operator==(rhs);
}

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
DallasTemperature::getAddress(DeviceAddress& addr, int arg_1)
{
  MOCK_FUNC_R1(bool, int)
  addr.addr = arg_1;
  return true;
}
void
DallasTemperature::setResolution(DeviceAddress addr, int arg_1)
{
  if (expects_map.count("setResolution.addr") > 0) {
    DeviceAddress* exp =
      (DeviceAddress*)expects_map["setResolution.addr"].top();
    expects_map["setResolution.addr"].pop();
    if (*exp != addr) {
      std::cout << "ERROR: setResulution got an unexpedted address\n";
      assert(false);
    }
  }
  MOCK_FUNC_V1(int)
}
void
DallasTemperature::requestTemperatures()
{
  MOCK_FUNC_V0
}
float
DallasTemperature::getTempCByIndex(int arg_1)
{
  MOCK_FUNC_R1(float, int) return 0;
}
