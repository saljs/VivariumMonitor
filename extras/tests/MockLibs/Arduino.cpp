#include "Arduino.h"
#include <cstring>

void
MockArduino::yield()
{
  MOCK_FUNC_V0
}
void
MockArduino::delay(int arg_1)
{
  MOCK_FUNC_V1(int)
}
void
MockArduino::delayMicroseconds(int arg_1)
{
  MOCK_FUNC_V1(int)
}
void
MockArduino::pinMode(uint8_t arg_1, uint8_t arg_2){
  MOCK_FUNC_V2(uint8_t, uint8_t)
} uint8_t MockArduino::digitalRead(uint8_t arg_1)
{
  MOCK_FUNC_R1(uint8_t, uint8_t) return 0;
}
unsigned long
MockArduino::millis()
{
  MOCK_FUNC_R0(unsigned long) return 0;
}
void
MockArduino::configTime(const char* zone, const char* server)
{
  if (expects_map.count("configTime.zone")) {
    char* exp = (char*)expects_map["configTime.zone"].top();
    expects_map["configTime.zone"].pop();
    if (strcmp(exp, zone) != 0) {
      std::cout << "ERROR configTime got unexpected zone:" << zone << "\n";
    }
  }
  if (expects_map.count("configTime.server")) {
    char* exp = (char*)expects_map["configTime.server"].top();
    expects_map["configTime.server"].pop();
    if (strcmp(exp, server) != 0) {
      std::cout << "ERROR configTime got unexpected server:" << server << "\n";
    }
  }
}

MockArduino GlobalArduino = MockArduino();

void
yield()
{
  GlobalArduino.yield();
}
void
delay(int arg_1)
{
  GlobalArduino.delay(arg_1);
}
void
delayMicroseconds(int arg_1)
{
  GlobalArduino.delayMicroseconds(arg_1);
}
void
pinMode(uint8_t arg_1, uint8_t arg_2)
{
  GlobalArduino.pinMode(arg_1, arg_2);
}
uint8_t
digitalRead(uint8_t arg_1)
{
  return GlobalArduino.digitalRead(arg_1);
}
unsigned long
millis()
{
  return GlobalArduino.millis();
}
void
configTime(const char* zone, const char* server)
{
  GlobalArduino.configTime(zone, server);
}
