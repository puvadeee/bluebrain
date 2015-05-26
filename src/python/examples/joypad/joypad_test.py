import sys
import os
import csv
from time import sleep

from cannybots.clients.wsclient import CannybotClient
from cannybots.clients.joypad import JoypadClient


def dataReceived(message):
    print "Received: " + message

cannybot = CannybotClient()             #  Connects to the first available Cannybot
joypad   = JoypadClient(cannybot)
cannybot.registerReceiveCallback(dataReceived)

sleep(2)  # wait a bit for connection setup

joypad.requestStatus()

#for speed in range(-255 , 255):
#    print "Speed: " + str(speed)
#    joypad.update(speed, speed, 0, 0)
#    sleep(0.25)

for speed in range(1,5):
    motorASpeed = speed*50
    motorBSpeed = speed*50
    joypad.update(motorASpeed, motorBSpeed, 0, 0)
    sleep(1)


joypad.stop()

