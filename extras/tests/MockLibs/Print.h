#ifndef PRINT_H
#define PRINT_H

#include <cstring>
#include <stdint.h>
#include <string>
#include <vector>

#include "MockLib.h"
#include "WString.h"

struct Print : public MockLib
{
public:
  std::string GetName() override { return "Print"; }
  size_t write(const uint8_t* data, size_t arg_1);
  size_t write(uint8_t arg_1);
  size_t write(const char* data, size_t arg_1);
  void flush();
  int availableForWrite();
  size_t print(const String& data);
  size_t print(const char* data);
  size_t println(const char* data = "");
  size_t println(const String& data);
  size_t printf(const char* format, ...);

  template<typename T>
  size_t print(const T& data)
  {
    return print(String(data));
  }

  template<typename T>
  size_t println(const T& data)
  {
    return println(String(data));
  }

  void AddOutputBuffer(std::vector<std::string>* out);

private:
  std::vector<std::string>* print_buffer = NULL;
  void AddToBuffer(const char* line);
  void AddToBuffer(std::string line);
};

#endif
