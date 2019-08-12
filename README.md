# AudioStick

Quality audio carrier board for the Raspberry Pi Zero

## Description and Getting Started

This board is designed to be used with a Raspberry Pi Zero (W) to turn it into small high quality streaming audio device.

### DAC

The DAC used is a PCM5102, which has:
 * An internal PLL to deal with XXX.
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

*All PCB design files are provided under the [CERN OSHW license][1]*

The PCB (v1.0) was designed in Eagle v9.2.0. The *.pcb* and *.sch* files are provided, along with a small library of parts (*AudioStick.lbr*). A full bill of materials (for each board revision) is provided as an *.ods* file.

## Firmware

*All firmware is provided under the [MIT License][2]*

The source for the power control microcontroller is provided, and can be compiled with *avr-gcc*. No additional libraries are required.

### Flashing the ATtiny13A

The header **J1** on the PCB is a standard Microchip \[Atmel\] 6 pin programming header. Using your compiled firmware (or the precompiled *.elf* file), you can flash the microcontroller with e.g. an AVR Dragon.

- [ ] Check whether LED1 and R15 should be left unpopulated for flashing. _Revise for v1.1 if so_.


[1]: https://ohwr.org/cernohl
[2]: https://opensource.org/licenses/MIT