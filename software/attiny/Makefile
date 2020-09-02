MCU=attiny13a
# F_CPU=1200000
CC			= avr-gcc
OBJCOPY 	= avr-objcopy
SIZE 		= avr-size
OPT			= O2
CFLAGS 		= -std=c99 -Wall -g -Werror=switch -pedantic --short-enums -I.
TARGET 		= audiostick
SRCS 		= audiostick.c
MCU_FLASH 	= attiny13
PROGRAMMER 	= atmelice_isp

all:
	@echo ----------------------------------
	@echo   Compiling AudioStick firmware
	@echo ----------------------------------
	@echo

	${CC} ${CFLAGS} -${OPT} -mmcu=${MCU} -o ${TARGET}.elf ${SRCS}
	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.elf ${TARGET}.hex
	${SIZE} --format=avr --mcu=${MCU} ${TARGET}.elf

flash:
	@echo ----------------------------------
	@echo       Flashing ATtiny13A
	@echo ----------------------------------
	@echo
	@echo Programmer: ${PROGRAMMER}

	avrdude -p ${MCU_FLASH} -c ${PROGRAMMER} -U flash:w:${TARGET}.hex:i -P usb -v

clean:
	rm -fv *.elf *.hex
