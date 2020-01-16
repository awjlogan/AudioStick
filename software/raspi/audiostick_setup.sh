#!/bin bash

if [ "$(id -u)" != "0" ]; then
	echo "You need to run under sudo. Exiting."
	exit 1
fi

echo "Backing up /boot/config.txt to ~/"
cp -v /boot/config.txt ~/config.txt_back

echo -n "Patching /boot/config.txt ..."

echo "TODO"

OVERLAY="dtOVERLAY=hifiberry-dac"
if grep -Fxq /boot/config.txt $OVERLAY
then
	echo "DAC's overlay already present. Skipping."
else
	echo "" >> /boot/config.txt
	echo $OVERLAY >> /boot/config.txt
fi

ASOUND_F="/etc/asound.conf"
if [ -f $ASOUND_F ]; then
	echo "$ASOUND_F already exists. Skipping."
else
	cp -fv ./asound.conf $ASOUND_F
fi


echo "Setting power management software to run at startup."
echo "TODO"

echo "Shutting down!"
echo "When shutdown is complete, either pull the power or hold the button to reset."
read -p "Continue? (y/n) " continue

if [ "$continue" = "y" ]; then
	shutdown -h now
else
	exit 1
fi
