#ifndef UPDATER_H
#define UPDATER_H

#include "MockLib.h"
#include "Print.h"
#include "Stream.h"

class UpdaterClass : public MockLib
{
public:
  std::string GetName() override { return "Update"; }
  bool begin(size_t arg_1);
  bool end();
  size_t writeStream(Stream& arg_1);
  void printError(Print& arg_1);
};

extern UpdaterClass Update;

#endif
