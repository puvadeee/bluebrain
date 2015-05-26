import sys
import os
import csv
from time import sleep

from cannybots.clients.wsclient import CannybotClient
from cannybots.clients.joypad import JoypadClient


def dataReceived(message):
    print "Received: " + message

cannybot = CannybotClient(botId)             #  Connects to the first available Cannybot
joypad   = JoypadClient(cannybot)

cannybot.registerReceiveCallback(dataReceived)

sleep(2)
joypad.requestStatus()

#for speed in range(-255 , 255):
#    print "Speed: " + str(speed)
#    joypad.update(speed, speed, 0, 0)
#    sleep(0.25)

speed1 =200
speed2 =    speed1
for i in range(0,2):
    joypad.update(speed1, speed2, 0, 0)
    sleep(1)


joypad.stop()

