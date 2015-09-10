# Name: Makefile
# Author: Dirkjan Krijnders
# Copyright: 2014 Dirkjan Krijnders
# License: CC BY-SA 3.0

# This is a prototype Makefile. Modify it according to your needs.
# You should at least check the settings for
# DEVICE ....... The AVR device you compile for
# CLOCK ........ Target AVR clock rate in Hertz
# OBJECTS ...... The object files created from your source files. This list is
#                usually the same as the list of source files with suffix ".o".
# PROGRAMMER ... Options to avrdude which define the hardware you use for
#                uploading to the AVR and the interface where this hardware
#                is connected. We recommend that you leave it undefined and
#                add settings like this to your ~/.avrduderc file:
#                   default_programmer = "stk500v2"
#                   default_serial = "avrdoper"
# FUSES ........ Parameters for avrdude to flash the fuses appropriately.



DEVICE     = atmega328p
# DEVICE     = attiny84
CLOCK      = 160000000

# @echo "Board: $(BOARD)"
PROGRAMMER = -c usbasp
OBJECTS    = LNS88.o S88/S88.o LocoNet/LocoNet.o LocoNet/utility/ln_sw_uart.o LocoNet/utility/ln_buf.o LocoNet/utility/utils.o
LINK_OBJECTS  = 
INCLUDE    = -ILocoNet/utility -ILocoNet/ -IS88
FUSES      = -U hfuse:w:0xdf:m -U lfuse:w:0xff:m

# ATMega8 fuse bits used above (fuse bits for other devices are different!):
# Example for 8 MHz internal oscillator
# Fuse high byte:
# 0xd9 = 1 1 0 1   1 0 0 1 <-- BOOTRST (boot reset vector at 0x0000)
#        ^ ^ ^ ^   ^ ^ ^------ BOOTSZ0
#        | | | |   | +-------- BOOTSZ1
#        | | | |   +---------- EESAVE (set to 0 to preserve EEPROM over chip erase)
#        | | | +-------------- CKOPT (clock option, depends on oscillator type)
#        | | +---------------- SPIEN (if set to 1, serial programming is disabled)
#        | +------------------ WDTON (if set to 0, watchdog is always on)
#        +-------------------- RSTDISBL (if set to 0, RESET pin is disabled)
# Fuse low byte:
# 0x24 = 0 0 1 0   0 1 0 0
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (8M internal RC)
#        | |  +--------------- SUT 1..0 (slowly rising power)
#        | +------------------ BODEN (if 0, brown-out detector is enabled)
#        +-------------------- BODLEVEL (if 0: 4V, if 1: 2.LNS88#
# For computing fuse byte values for other devices and options see
# the fuse bit calculator at http://www.engbedded.com/fusecalc/


# Tune the lines below only if you know what you are doing:

#CFLAGS += -ffunction-sections
#LDFLAGS += -Wl,-gc-sections

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall $(CFLAGS) -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) $(INCLUDE)

# symbolic targets:
all:	LNS88.hex 

#LNS88.eeprom.hex

#LNS88.eeprom.hex

%.o: %.cpp
	$(COMPILE) -c $< -o $@ -DBOARD=$(BOARD)

%.o: %.c
	$(COMPILE) -c $< -o $@ -DBOARD=$(BOARD)

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:LNS88.hex:i
#	sleep 2
#	$(AVRDUDE) -U eeprom:w:LNS88.eeprom.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

clean:
	rm -f LNS88.hex LNS88.elf $(OBJECTS)

# file targets:
LNS88.elf: $(OBJECTS)
	$(COMPILE) -o LNS88.elf $(LDFLAGS) $(OBJECTS) $(LINK_OBJECTS)

LNS88.hex: LNS88.elf
	rm -f LNS88.hex
	avr-objcopy -j .text -j .data -O ihex LNS88.elf LNS88.hex
	avr-size --format=avr --mcu=$(DEVICE) LNS88.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

LNS88.eeprom.hex: LNS88.elf
	avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

# Targets for code debugging and analysis:
disasm:	LNS88.elf
	avr-objdump -d LNS88.elf

cpp:
	$(COMPILE) -E LNS88.c
