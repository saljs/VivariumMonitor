#ifndef STREAMUTILS_H
#define STREAMUTILS_H

#include "Stream.h"

class ReadBufferingStream : public Stream
{
public:
  ReadBufferingStream(Stream& stream, int bufLen);
  int available();
  int find(const char* match);
  int read();
  int parseInt();
  size_t readBytes(char* buffer, size_t arg_1);
  size_t readBytesUntil(char arg_1, char* buffer, size_t arg_2);
  String readString();
};

class WriteBufferingStream : public Print
{
public:
  WriteBufferingStream(Stream& stream, int bufLen);
  size_t print(const char* data) { return underlay->print(data); }
  template<typename T>
  size_t print(const T& data)
  {
    return underlay->print(&data);
  }
  template<typename T>
  size_t println(const T& data)
  {
    return underlay->print(&data);
  }
  size_t printf(const char* format, ...);
  size_t write(const uint8_t* data, size_t arg_1);
  size_t write(uint8_t arg_1);
  size_t write(const char* data, size_t arg_1);

private:
  Stream* underlay = NULL;
};

void
SetGlobalInputStream(std::string text);
#endif
