PERIPHERALS = pit adc
DRIVERS = rf pacer mpu button
TARGET = hat.bin

SRC = hat.c joystick.c buzzer.c imu.c
# OPT = -O2 # aparently this is a bad option (from Josh)

# ifneq ($(BOARD), hat)
# $(error BOARD should be set to hat!)
# endif

include ../core/core.mk
include ../../boards/board.mk