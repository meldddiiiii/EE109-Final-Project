DEVICE     = atmega328p
CLOCK      = 16000000
PROGRAMMER = -c arduino -b 115200 -P /dev/cu.usbmodem*
OBJECTS = project.o timers.o encoder.o lcd.o adc.o buzzer.o serial.o 
FUSES      = -U hfuse:w:0xde:m -U lfuse:w:0xff:m -U efuse:w:0x05:m

# Tune the lines below only if you know what you are doing:

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID main.hex

clean:
	rm -f main.hex main.elf $(OBJECTS)

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(DEVICE) main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c

timer_test: timer_test.hex
	$(AVRDUDE) -U flash:w:timer_test.hex:i
timer_test.o: lab5.c
	$(COMPILE) -DTASK3 -c lab5.c -o timer_test.o
timer_test.elf: timer_test.o lcd.o
	$(COMPILE) -o timer_test.elf timer_test.o lcd.o
timer_test.hex: timer_test.elf
	rm -f timer_test.hex
	avr-objcopy -j .text -j .data -O ihex timer_test.elf timer_test.hex
	avr-size --format=avr --mcu=$(DEVICE) timer_test.elf
