/*
 * debug.h
 * Copyright Sal Skare
 * Released under GPL3 license
 */

#ifndef DEBUG_H
#define DEBUG_H

// Set up debugging if it's turned on
#ifdef DEBUG_ESP_CORE
#define DEBUG_ON 1
#define DEBUG_USE_SERIAL 1
#define DEBUG_USE_TELNET 1
#include <ESPTelnet.h>
extern ESPTelnet telnet;
#undef DEBUG_MSG
#define DEBUG_MSG(format, ...) DEBUG_TELNET.printf(PSTR(format), ##__VA_ARGS__); DEBUG_SERIAL.printf(PSTR(format), ##__VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif

#endif
