#!/usr/bin/env python

import RPi.GPIO as GPIO
from subprocess import call
from time import sleep

if __name__ == "__main__":

    REQn = 17
    ACKn = 27

    # Use SoC GPIO numbering
    GPIO.setmode(GPIO.BCM)

    # Setup REQn as input
    GPIO.setup(REQn, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

    # Setup ACKn as output, tie to 1 on startup
    GPIO.setup(ACKn, GPIO.OUT, initial=1)

    # Wait for REQ to go LOW
    while (GPIO.input(REQn)):
        GPIO.wait_for_edge(REQn, GPIO.FALLING)
        sleep(0.1)  # Avoid glitch

    call('poweroff', shell=False)
