#include "Stream.h"
#include <cstring>

int
Stream::available()
{
  MOCK_FUNC_R0(int) return 0;
}
int
Stream::read()
{
  MOCK_FUNC_R0(int) return 0;
}
int
Stream::peek()
{
  MOCK_FUNC_R0(int) return 0;
}
size_t
Stream::readBytes(char* buffer, size_t arg_1)
{
  MOCK_FUNC_R1(size_t, size_t);
  if (!expects_map["readBytes.buffer"].empty()) {
    const char* in = (const char*)expects_map["readBytes.buffer"].top();
    expects_map["readBytes.buffer"].pop();
    for (int i = 0; i < arg_1; i++) {
      buffer[i] = in[i];
    }
  }
  return 0;
}
String
Stream::readString()
{
  MOCK_FUNC_R0(String) return String();
}
void
Stream::setTimeout(unsigned long arg_1)
{
  MOCK_FUNC_V1(unsigned long)
}
bool
Stream::find(const char* match)
{
  MOCK_FUNC_R0(bool) return false;
}
int
Stream::parseInt()
{
  MOCK_FUNC_R0(int) return 0;
}
size_t
Stream::readBytesUntil(char arg_1, char* buffer, size_t arg_2)
{
  if (expects_map.count("readBytesUntil.buffer") > 0) {
    // Since this function writes into a buffer, we need to mock that.
    const char* text = (const char*)expects_map["readBytesUntil.buffer"].top();
    expects_map["readBytesUntil.buffer"].pop();
    strcpy(buffer, text);
  }
  MOCK_FUNC_R2(size_t, char, size_t)
  return 0;
}
