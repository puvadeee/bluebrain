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

sleep(2)
joypad.requestStatus()

joypad.update(-255, 255, 0, 0)
sleep(1)

joypad.update(255, -255, 0, 0)
sleep(1)

joypad.stop()

