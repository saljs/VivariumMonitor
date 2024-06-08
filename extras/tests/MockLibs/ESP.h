#ifndef ESP_H
#define ESP_H

#include "MockLib.h"

class ESPClass : public MockLib
{
public:
  void restart();
  void reset();
  void eraseConfig();
  int getChipId();
  int getFreeHeap();
  int getHeapFragmentation();
  std::string GetName() override { return "ESP"; }
};

extern ESPClass ESP;

#endif
