# AudioStick

Quality audio carrier board for the Raspberry Pi Zero.

## Description and Getting Started

This board is designed to be used with a Raspberry Pi Zero (W) to turn it into small high quality streaming audio device.

All design files and source code are under the [_Don't Be A Dick_][dbad-github] license.

### DAC

The DAC used is a PCM5102, which has:
 * An internal PLL to deal with the SPI clocking scheme.
 * A built in charge pump supply to ground reference the output, which removes the need for output coupling capacitors.

### Power controller

There is a power controller on the board to safely turn on/off the Raspberry Pi, which will reduce the likelyhood of corrupting the SD card. The 4-phase asynchronous handshake used between the two components ensures that power is applied or removed correctly. The handshake is shown below:

## Raspberry Pi Setup

install:
mpd
avahi-daemon
mpc ?

change hostname to location (optional):
/etc/hosts (change 127.0.0.1 entry)
/etc/hostname (to match above)

make directory:
/media/nas_homes

add entry to /etc/fstab:
turnlanevault.local:/volume1/music /media/nas_music nfs auto,nofail,noatime,nolock,intr,tcp,actimeo=1800 0 0

configure mpd /etc/mpd.conf
set /media/nas_music as music directory
set bind_to_address to hostname
set restore_paused to yes
set up mixer ?

## Hardware

### PCB

The PCB was designed using [EAGLE][eagle-web] v9. The *.pcb* and *.sch* files are provided, along with a small library of parts (*AudioStick.lbr*). A full bill of materials is provided as an *.ods* file.

### Firmware and flashing the ATtiny13A

The source for the power control microcontroller is provided, and can be compiled with the supplied Makefile. You will need to alter `Makefile` with the programmer that you are using. To compile the firmware:

`> /firmware/make all`

The header **J1** on the PCB is a standard Microchip \[Atmel\] 6 pin programming header. Using your compiled firmware (or the precompiled *.hex* file), you can flash the microcontroller with e.g. an Atmel ICE. To flash the microcontroller, run:

`> /firmware/make flash`


## Datasheets and useful links

- [PCM5102 DAC datasheet][pcm5102-datasheet]
- [ATtiny13A datasheet][attiny13-datasheet]

[dbad-github]: https://github.com/philsturgeon/dbad
[eagle-web]: https://www.autodesk.com/products/eagle/overview
[pcm5102-datasheet]: http://www.ti.com/product/PCM5102
[attiny13-datasheet]: https://www.microchip.com/wwwproducts/en/ATTINY13A
