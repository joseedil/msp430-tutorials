OBJECTS = scheduler.o

TI_TOOLS = ~/2-development/ti/tools/gcc
MSPDEBUG_DIR = ~/2-development/ti/tools/mspdebug

GCC_DIR =  $(TI_TOOLS)/bin
SUPPORT_FILE_DIRECTORY = $(TI_TOOLS)/include

# Please set your device here
DEVICE   = msp430f5529
CC       = $(GCC_DIR)/msp430-elf-gcc
GDB      = $(GCC_DIR)/msp430-elf-gdb
MSPDEBUG = $(MSPDEBUG_DIR)/mspdebug

CFLAGS = -I $(SUPPORT_FILE_DIRECTORY) -mmcu=$(DEVICE) -O0 -g3 -ggdb -gdwarf-2
LFLAGS = -L $(SUPPORT_FILE_DIRECTORY) -T $(DEVICE).ld

MSPDEBUG_FLAGS = tilib --allow-fw-update

all: ${OBJECTS}
	$(CC) $(CFLAGS) $(LFLAGS) $? -o $(DEVICE).out

debug: all
	$(GDB) $(DEVICE).out

mspdebug: all
	$(MSPDEBUG) $(MSPDEBUG_FLAGS)
