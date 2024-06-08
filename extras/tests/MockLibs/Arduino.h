#ifndef ARDUINO_H
#define ARDUINO_H

/*
 * Some dummy defininitions, they do not line up to
 * actual values
 */
#define SDA 4
#define SCL 5
#define INPUT_PULLUP 2
#define INPUT 1
#define OUTPUT 0
#define LOW 0
#define HIGH 1

#include "ESP.h"
#include "MockLib.h"
#include "Print.h"
#include "Stream.h"
#include "WString.h"
#include <cstdint>

#define F(x) x

class MockArduino : public MockLib
{
public:
  std::string GetName() override { return "MockArduino"; }
  void yield();
  void delay(int arg_1);
  void delayMicroseconds(int arg_1);
  void pinMode(uint8_t arg_1, uint8_t arg_2);
  uint8_t digitalRead(uint8_t arg_1);
  unsigned long millis();
  // Not an Arduino function, but it's easiest to put here
  void configTime(const char* zone, const char* server);
};

void
yield();
void
delay(int arg_1);
void
delayMicroseconds(int arg_1);
void
pinMode(uint8_t arg_1, uint8_t arg_2);
uint8_t
digitalRead(uint8_t arg_1);
unsigned long
millis();
void
configTime(const char* zone, const char* server);

#endif
