import sys
import os
import csv
from time import sleep
basedir = os.path.abspath(os.path.dirname(__file__))
sys.path.insert(0, basedir+"/../../../modules")

from cannybots.clients.joypad import JoypadClient


SPEED_DURATION   = 2              # Number of seconds to hold a speed for

joypad = JoypadClient()           # Connects to the first available Cannybot

input_file = csv.DictReader(open("speeds.csv"))



for row in input_file:
    inputDataSpeed = float(row["Speed"])

    cannybotSpeed  = 50 + inputDataSpeed*10         # scale the input speed to between 0 (stop) and 255 (fullspeed)

    print "Input Data speed: {}  => Cannybot speed: {}".format(inputDataSpeed, cannybotSpeed)

    joypad.update(0, cannybotSpeed, 0, 0)           # send the forward speed
    sleep(SPEED_DURATION)                           # hold the speed for a number of seconds


joypad.stop()       # Stop the Cannybot


