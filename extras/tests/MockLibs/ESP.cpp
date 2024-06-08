#include "ESP.h"

ESPClass ESP;

void
ESPClass::restart()
{
  MOCK_FUNC_V0
}
void
ESPClass::reset()
{
  MOCK_FUNC_V0
}
void
ESPClass::eraseConfig()
{
  MOCK_FUNC_V0
}
int
ESPClass::getChipId()
{
  MOCK_FUNC_R0(int) return 0;
}
int
ESPClass::getFreeHeap()
{
  MOCK_FUNC_R0(int) return 0;
}
int
ESPClass::getHeapFragmentation()
{
  MOCK_FUNC_R0(int) return 0;
}
