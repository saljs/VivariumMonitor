#include "Print.h"

#include <cstdarg>

size_t
Print::write(const uint8_t* data, size_t arg_1)
{
  AddToBuffer((const char*)data);
  MOCK_FUNC_R1(size_t, size_t)
  return 0;
}
size_t
Print::write(uint8_t arg_1)
{
  MOCK_FUNC_R1(size_t, uint8_t) return 0;
}
size_t
Print::write(const char* data, size_t arg_1)
{
  AddToBuffer(data);
  MOCK_FUNC_R1(size_t, size_t)
  return 0;
}
void
Print::flush()
{
  MOCK_FUNC_V0
}
int
Print::availableForWrite()
{
  MOCK_FUNC_R0(int) return 0;
}
size_t
Print::print(const String& data)
{
  AddToBuffer(data.c_str());
  MOCK_FUNC_R0(size_t)
  return 0;
}
size_t
Print::print(const char* data)
{
  AddToBuffer(data);
  MOCK_FUNC_R0(size_t)
  return 0;
}
size_t
Print::println(const String& data)
{
  return print(data);
}
size_t
Print::println(const char* data)
{
  return print(data);
}
size_t
Print::printf(const char* format, ...)
{
  char buf[256];
  va_list args;

  va_start(args, format);
  vsnprintf(buf, 256, format, args);
  va_end(args);
  AddToBuffer(buf);
  MOCK_FUNC_R0(size_t)
  return 0;
}

void
Print::AddOutputBuffer(std::vector<std::string>* out)
{
  print_buffer = out;
}

void
Print::AddToBuffer(const char* line)
{
  AddToBuffer(std::string(line));
}

void
Print::AddToBuffer(std::string line)
{
  if (print_buffer) {
    print_buffer->push_back(line);
  }
}
