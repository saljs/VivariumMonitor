CC=g++
CPPFLAGS=-std=gnu++20 -gdwarf-2 -g3 -include MockLibs/defs.h
VERSION=-DFIRMWARE_VERSION=\"unittest\"

MOCK_LIBS=build/MockLibs/Arduino.o build/MockLibs/DallasTemperature.o build/MockLibs/ESP8266WiFi.o build/MockLibs/ESP.o build/MockLibs/LittleFS.o build/MockLibs/MockLib.o build/MockLibs/OneWire.o build/MockLibs/Print.o build/MockLibs/Stream.o build/MockLibs/StreamUtils.o build/MockLibs/Updater.o build/MockLibs/WiFiManager.o build/MockLibs/Wire.o
TEST_LIBS=build/lib/Hardware.o build/lib/Network.o build/lib/VivariumMonitor.o
TESTS := $(addprefix build/,$(basename $(shell echo unit_tests/*.cpp)))

.PHONY:all unittest

all: $(TESTS)

build:
	mkdir build
build/lib: build
	mkdir build/lib
build/MockLibs: build
	mkdir -p build/MockLibs
build/unit_tests: build
	mkdir -p build/unit_tests

build/MockLibs/%.o: MockLibs/%.cpp build/MockLibs
	$(CC) $(CPPFLAGS) -c $< -o $@
build/lib/%.o: ../../src/%.cpp build/lib $(MOCK_LIBS)
	$(CC) $(CPPFLAGS) $(VERSION) -I./MockLibs -c $< -o $@
build/unit_tests/%: unit_tests/%.cpp build/unit_tests $(TEST_LIBS)
	$(CC) $(CPPFLAGS) -I./MockLibs -I../../src $< build/MockLibs/*.o build/lib/*.o -o $@

unittest: $(TESTS)
	./test_runner.sh $(TESTS)
