
DRIVERS += usb_serial rf ledtape 
INCLUDES += -I../core
VPATH += ../core

SRC += core.c comms.c led_tape.c low_voltage.c
