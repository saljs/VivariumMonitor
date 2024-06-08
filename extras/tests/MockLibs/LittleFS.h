#ifndef LITTLEFS_H
#define LITTLEFS_H

#include "MockLib.h"
#include "Stream.h"

class File : public Stream
{
public:
  void close();
};

class LittleFSClass : public MockLib
{
public:
  std::string GetName() override { return "LittleFS"; }
  bool format();
  bool begin();
  void end();
  bool exists(const char* path);
  File open(const char* path, const char* mode);
};

extern LittleFSClass LittleFS;

#endif
