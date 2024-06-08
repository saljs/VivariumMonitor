#include "LittleFS.h"

LittleFSClass LittleFS;

void
File::close()
{
  MOCK_FUNC_V0
}

bool
LittleFSClass::format()
{
  MOCK_FUNC_R0(bool) return true;
}
bool
LittleFSClass::begin()
{
  MOCK_FUNC_R0(bool) return true;
}
void
LittleFSClass::end()
{
  MOCK_FUNC_V0
}
bool
LittleFSClass::exists(const char* path)
{
  MOCK_FUNC_R0(bool) return false;
}
File
LittleFSClass::open(const char* path, const char* mode)
{
  MOCK_FUNC_R0(File) return File();
}
