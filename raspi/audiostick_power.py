#!/usr/bin/env python

import RPi.GPIO as GPIO
import subprocess

if __name__ == "__main__":

    GPIO.setmode(GPIO.BCM)

    # Setup S_REQ
    GPIO.setup(3, GPIO.IN, pull_up_down=GPIO.PUD_UP)

    # Setup RP_ACK
    GPIO.setup(4, GPIO.OUT)