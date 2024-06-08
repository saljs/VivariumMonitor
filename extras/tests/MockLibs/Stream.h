#ifndef STREAM_H
#define STREAM_H

#include "MockLib.h"
#include "Print.h"
#include "WString.h"

struct Stream : public Print
{
public:
  std::string GetName() override { return "Stream"; }
  int available();
  int read();
  int peek();
  size_t readBytes(char* buffer, size_t arg_1);
  String readString();
  void setTimeout(unsigned long arg_1);
  bool find(const char* match);
  int parseInt();
  size_t readBytesUntil(char arg_1, char* buffer, size_t arg_2);
};

#endif
