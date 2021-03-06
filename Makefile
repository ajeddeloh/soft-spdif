NAME=soft-spdif
-include $(DEPS)

# Tools
PREFIX = arm-none-eabi
CC = $(PREFIX)-gcc
AS = $(PREFIX)-as
OBJCOPY = $(PREFIX)-objcopy
GDB = $(PREFIX)-gdb

# Directories
CUBE = STM32CubeL4
CMSIS = $(CUBE)/Drivers/CMSIS
CMSIS_DEV=$(CMSIS)/Device/ST/STM32L4xx

# Include path
INC += -I$(CMSIS)/Include
INC += -I$(CMSIS_DEV)/Include

# Flags
CFLAGS  = -std=c17 -Wall -Wextra -g3 -gdwarf-2 -MMD -MP -O2
CFLAGS += -mcpu=cortex-m4 -mthumb -DSTM32L476xx -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mtune=cortex-m4 -march=armv7e-m
CFLAGS += -T $(LINKER_SCRIPT) --specs=nosys.specs
LFLAGS = -lm

# CFLAGS for just our sources
EXTRA_CFLAGS = -pedantic -fanalyzer

# Startup Files
STARTUP_S = src/startup_stm32l476xx.s
SYSTEM_C = $(CMSIS_DEV)/Source/Templates/system_stm32l4xx.c
STARTUP_O = build/startup/$(notdir $(STARTUP_S:.s=.o))
SYSTEM_O = build/startup/$(notdir $(SYSTEM_C:.c=.o))
LINKER_SCRIPT = src/STM32L476VGTx_FLASH.ld

# Our files
SRCS = $(wildcard src/*.c)
OBJS = $(addprefix build/,$(SRCS:.c=.o))

DEPS = $(OBJS:.o=.d) $(SYSTEM_O:.o=.d)
# Recipes

.PHONY: all
all: build/$(NAME).bin

.PHONY: clean
clean:
	rm -f $(OBJS) \
	    $(STARTUP_O) \
	    $(SYSTEM_O) \
	    $(HAL_OBJS) \
	    $(BSP_OBS) \
	    $(DEPS) \
	    build/$(NAME).elf build/$(NAME).bin

build/src/%.o: src/%.c #build/src
	mkdir -p $(@D)
	$(CC) -c $(INC) $(CFLAGS) $(EXTRA_CFLAGS) $< -o $@

$(STARTUP_O): $(STARTUP_S) #build/startup
	mkdir -p $(@D)
	$(AS) -c $(INC) $< -o $@

$(SYSTEM_O): $(SYSTEM_C) #build/startup
	mkdir -p $(@D)
	$(CC) -c $(INC) $(CFLAGS) $< -o $@

build/$(NAME).elf: $(OBJS) $(STARTUP_O) $(SYSTEM_O)
	$(CC) -o $@ $(CFLAGS) $^ $(LFLAGS)

build/$(NAME).bin: build/$(NAME).elf
	$(OBJCOPY) -O binary $< $@

.PHONY: gdb
gdb: build/$(NAME).elf
	$(GDB) -ex 'target ext :3333' build/$(NAME).elf

.PHONY: flash
flash: build/$(NAME).elf
	$(GDB) -ex 'target ext :3333' -ex 'load' build/$(NAME).elf
