# AudioStick

Quality audio carrier board for the Raspberry Pi Zero. A self facilitating media node.

## Description and Getting Started

This board is designed to be used with a Raspberry Pi Zero (W) to turn it into small high quality streaming audio device. It incorporates a soft power switch, which makes it easier to use in normal settings (e.g., don't need to SSH in to shutdown) and prolong the life of the SD card. The AudioStick connects directly to the standard Raspberry Pi 40pin GPIO header.

All design files and source code are under the [_Don't Be A Dick_][dbad-github] license.

### Installation on the Raspberry Pi

From a clean installation of Raspbian, log in to your Raspberry Pi and run the following 3 lines:

```shell
> git clone https://github.com/awjlogan/audiostick.git
> cd ./audiostick
> sudo ./audiostick_setup.sh
```

The `audiostick_setup.sh` script will:
 - Install the PCM5102's configuration in the boot configuration
 - Add the `asound.conf` configuration file
 - Add the power control service

When it is complete, you will need to shutdown and then remove the power before powering on again.

### Using the AudioStick

 - When the AudioStick is off, there is no power supplied to the Raspberry Pi, so power consumption is minimised. The power LED will pulse slowly.
 - Pressing the button will power on the Raspberry Pi. The power LED will flash until the Raspberry Pi has successfully booted, at which point it will stay on.
 - To turn the AudioStick OFF, press and hold the button for at least one second. The power LED will flash until the Raspberry Pi has safely shutdown. Once the LED returns to pulsing, the Raspberry Pi is completely off and it is safe to remove the power if you wish.

### Raspberry Pi Setup

The AudioStick is intended to be used with something like MPD to allow you to connect up to any speaker (with amplifier) and get access to your local music or streaming services controlled by your phone. There are a lot of good tutorials (REVISIT add some links) available, so I won't reiterate the points here, but here are a few additional things that might be useful:

 - Change the Raspberry Pi's hostname to a meaningful name, for example `livingroompi`. This will also allow you to have multiple AudioSticks on the same network.
 - Configure your NAS (or whatever storage) to mount at boot time in `/etc/fstab`. Be aware that the system might attempt to mount the device before the network is available.
 - Disable the swap file to increase the SD card's life (`sudo apt remove dphys-swapfile`)

Setting up audiostick

Setup for PCM510x:
Configure overlay: /boot/config.txt
uncomment: dtparam=i2s=on
comment: #dtparam=audio=on
add: dtoverlay=hifiberry-dac


## Hardware

### PCB

The PCB was designed using [EAGLE][eagle-web] v9. The *.brd* and *.sch* files are provided, along with a small library of parts (*AudioStick.lbr*). A full bill of materials is provided as an *.ods* file.

### Firmware and flashing the ATtiny13A

The source for the power control microcontroller is provided, and can be compiled with the supplied Makefile. You will need to alter `Makefile` with the programmer that you are using. To compile the firmware:

`> /firmware/make`

The header **J4** on the PCB is a standard Microchip \[Atmel\] 6 pin programming header. Using your compiled firmware (or the precompiled *.hex* file), you can flash the microcontroller with e.g. an Atmel ICE. To flash the microcontroller, run:

`> /firmware/make flash`

## Change Notes

### Version 2.2

 - [x] -ELEC- Change from pogo pads to pin header for AVR ISP.

### Version 2.1

 - [x] -MECH- Mirror RasPi so connectors face Audiostick board.
 - [x] -ELEC- Move LED to PWM output pin for more interesting effects (chicken transistor in case programmer can't drive pin with LED).
 - [x] -ELEC- Remove MOSFET power switch for RasPi. Use inrush inhibitor instead.
 - [x] -ELEC- Downsize RasPi power capacitor.

### Version 2.0

Finalised release PCB, firmware, and RasPi software.

## Datasheets and useful links

- [PCM5102 DAC][pcm5102-datasheet]
- [ATtiny13A][attiny13-datasheet]

[dbad-github]: https://github.com/philsturgeon/dbad
[eagle-web]: https://www.autodesk.com/products/eagle/overview
[pcm5102-datasheet]: http://www.ti.com/product/PCM5102
[attiny13-datasheet]: https://www.microchip.com/wwwproducts/en/ATTINY13A
