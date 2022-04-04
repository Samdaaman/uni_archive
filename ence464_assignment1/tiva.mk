
# Specify the toolchain and compilers we are going to be using
TOOLCHAIN = arm-none-eabi

CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-gcc
# GDB = $(TOOLCHAIN)-gdb
GDB = gdb-multiarch

# Specify the path to OpenOCD
OPENOCD = openocd

# Determine paths to the scripts and driver library
SCRIPTS = $(TIVA_DIR)/scripts
DRL = $(TIVA_DIR)/SW-TM4C-2.2.0.295
FREERTOS = $(TIVA_DIR)/FreeRTOSv202104.00/FreeRTOS

INCLUDES += -I $(TIVA_DIR) \
		    -I $(FREERTOS)/Source/portable/GCC/ARM_CM4F \
			-I $(FREERTOS)/Source/include

# Add the driver library to the include and linker paths
INCLUDES += -I $(DRL)
LIBS += $(DRL)/driverlib/gcc/libdriver.a

# If a TARGET name is not given, use the current working directories name
ifndef TARGET
TARGET := build/$(shell basename `pwd`).elf
endif

# Default to -Os optimization
ifndef OPT
OPT = -Os
endif

VPATH += $(TIVA_DIR) $(DRL)/utils \
	$(FREERTOS)/Source \
	$(FREERTOS)/Source/portable/GCC/ARM_CM4F \
	$(FREERTOS)/Source/portable/MemMang

# Add the startup file to the source path
SRC += startup.c ustdlib.c \
	croutine.c \
	event_groups.c \
	list.c \
	queue.c \
	stream_buffer.c \
	tasks.c \
	timers.c \
	port.c \
	heap_1.c 

# Generate the list of object files to compile
OBJ = $(addprefix build/,$(SRC:.c=.o))

DEFINES = -DPART_TM4C123GH6PM

# Setup the default compiler flags
CFLAGS += -mcpu=cortex-m4 -g3 -Wall -W -mthumb -mfloat-abi=hard \
	$(OPT) $(DEFINES)


# Allow for quiet or verbose building
ifndef VERBOSE
Q = @
endif


# Default build rule
# Just builds the binary file, does NOT attempt to program it
all: $(TARGET)

# Generic rule for turning c files into o files
build/%.o: %.c
	@mkdir -p $(dir $@)
	$(info CC $^)
	$(Q) $(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

# Generic rule for turning o files into bin files
$(TARGET): $(OBJ)
	$(info LD $@)
	$(Q) $(LD) $(CFLAGS) -T$(SCRIPTS)/link.ld -o $@ $^ $(LIBS)

# Cleanup all the existing build files so the next build is fresh
clean:
	rm -rf $(OBJ) $(TARGET)

# Use GDB to load the code onto the device
program: $(TARGET)
	$(GDB) -batch -x $(SCRIPTS)/program.gdb $^

# Use GDB to debug the device
debug: $(TARGET)
	$(GDB) -x $(SCRIPTS)/debug.gdb $^

# Shortcut for running OpenOCD for this specific board
openocd:
	$(OPENOCD) -f board/ek-tm4c123gxl.cfg

# Opens tty session with the device
tty:
	minicom -D /dev/ttyACM0 -m -b 9600


.PHONY: all clean program debug openocd

