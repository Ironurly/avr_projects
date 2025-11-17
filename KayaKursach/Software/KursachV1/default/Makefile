###############################################################################
# Makefile for the project KursachV1
###############################################################################

## General Flags
PROJECT = KursachV1
MCU = atmega328p
TARGET = KursachV1.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=KursachV1.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = KursachV1.o log.o i2c_config.o ssd1306.o sms_reader.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) KursachV1.hex KursachV1.eep KursachV1.lss size

## Compile
KursachV1.o: ../KursachV1.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

log.o: ../log.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

i2c_config.o: ../display/i2c_config.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ssd1306.o: ../display/ssd1306.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

sms_reader.o: ../GSM/sms_reader.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) KursachV1.elf dep/* KursachV1.hex KursachV1.eep KursachV1.lss KursachV1.map


## Other dependencies
-include $(shell mkdir dep 2>NUL) $(wildcard dep/*)

