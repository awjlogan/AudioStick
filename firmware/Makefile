MCU=attiny13a
# F_CPU=1200000
CC=avr-gcc
OBJCOPY=avr-objcopy
SIZE=avr-size
CFLAGS=-std=c99 -Wall -g -Werror=switch -pedantic -Os --short-enums -mmcu=${MCU} -I.
TARGET=audiostick
SRCS=audiostick.c

all:
	${CC} ${CFLAGS} -o ${TARGET}.elf ${SRCS}
	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.elf ${TARGET}.hex
	${SIZE} --format=avr --mcu=${MCU} ${TARGET}.elf

clean:
	rm -fv *.elf *.hex
