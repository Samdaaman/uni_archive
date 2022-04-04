PERIPHERALS = pit pwm adc 
DRIVERS = rf pacer button led
TARGET = racer.bin

SRC = racer.c motors.c bumper.c
# OPT = -O2 # aparently this is a bad option (from Josh)

# ifneq ($(BOARD), racer)
# $(error BOARD should be set to racer!)
# endif

include ../core/core.mk
include ../../boards/board.mk
