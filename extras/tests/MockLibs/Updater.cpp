#include "Updater.h"

UpdaterClass Update;

bool
UpdaterClass::begin(size_t arg_1)
{
  MOCK_FUNC_R1(bool, size_t) return true;
}
bool
UpdaterClass::end()
{
  MOCK_FUNC_R0(bool) return true;
}
size_t
UpdaterClass::writeStream(Stream& arg_1)
{
  MOCK_FUNC_R0(size_t) return 0;
}
void
UpdaterClass::printError(Print& arg_1)
{}
