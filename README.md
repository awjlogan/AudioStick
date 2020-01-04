# AudioStick

Quality audio carrier board for the Raspberry Pi Zero.

## Description and Getting Started

This board is designed to be used with a Raspberry Pi Zero (W) to turn it into small high quality streaming audio device.

All design files and source code are under the [_Don't be a dick_][dbad-github] license.

### DAC

The DAC used is a PCM5102, which has:
 * An internal PLL to deal with the SPI clocking scheme.
 * A built in charge pump supply to ground reference the output, which removes the need for output coupling capacitors.

### Power controller

There is a power controller on the board to safely turn on/off the Raspberry Pi, which will reduce the likelyhood of corrupting the SD card. The 4-phase asynchronous handshake used between the two components ensures that power is applied or removed correctly. The handshake is shown below:

Turning ON:
<pre>
           ______________
S_REQ   __/              \___________
                  ____________
RP_ACK  _________/            \______
           __________________________
PWR     __/
</pre>

Turning OFF:
<pre>
           ____________
S_REQ   __/            \_____________
                 _____________
RP_ACK  ________/             \______
        _________________________
PWR                              \___
</pre>

The Raspberry Pi can also request a shutdown by raising the **RP_REQ** signal, which will trigger the same 4-phase handshake from the power controller.

## Raspberry Pi Setup

TODO

## PCB Files

The PCB was designed using [EAGLE][eagle-web] v9.2.0. The *.pcb* and *.sch* files are provided, along with a small library of parts (*AudioStick.lbr*). A full bill of materials (for each board revision) is provided as an *.ods* file.

## Firmware

The source for the power control microcontroller is provided, and can be compiled on Linux (and possibly macOS, untested) with the supplied Makefile. You will need to alter `Makefile` with the programmer that you are using. To compile the firmware:

`> /firmware/make all`

### Flashing the ATtiny13A

The header **J1** on the PCB is a standard Microchip \[Atmel\] 6 pin programming header. Using your compiled firmware (or the precompiled *.hex* file), you can flash the microcontroller with e.g. an Atmel ICE. To flash the microcontroller, run:

`> /firmware/make flash`

## Datasheets and useful links

- [PCM5102 DAC datasheet][pcm5102-datasheet]
- [ATtiny13A datasheet][attiny13-datasheet]

## Change Notes

- [x] ELE: MOSI/MISO on ATtiny13 are swapped
- [x] ELE: Rearrange pins on micro, remove RP_REQ
- [x] ELE: ESD protection on ports
- [x] DFM: Reduce BOM count by consolidating components.
- [x] MCH: microSD card can't be removed if the RPi0 is soldered down
- [ ] ~~MSC: Move to KiCad. (1.x -> 2.0)~~
- [x] DFM: Place all components on single side.

[dbad-github]: https://github.com/philsturgeon/dbad
[eagle-web]: https://www.autodesk.com/products/eagle/overview
[pcm5102-datasheet]: http://www.ti.com/product/PCM5102
[attiny13-datasheet]: https://www.microchip.com/wwwproducts/en/ATTINY13A
