ARDUINO_DIR=/home/xoza/src-old/arduino-1.5.8
USER_LIB_PATH=/home/xoza/Arduino/libraries
BOARD_TAG    = nano
ARDUINO_LIBS = SdFat/SdFat
MCU=atmega328p
ARCHITECTURE  = avr
AVRDUDE_OPTS = -v -V 
BOARD_SUB=atmega328

AVRDUDE_ISP_BAUDRATE=115200
include /home/xoza/src/Arduino-Makefile/Arduino.mk
CXXFLAGS=-Wno-deprecated-declarations -pedantic -Wall -std=gnu++11 -Wextra -O3
