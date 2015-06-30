import sys
import os
import csv
from time import sleep
from sys import stdin


from cannybots.clients.wsclient import CannybotClient


def dataReceived(message):
    print "Received: " + message

cannybot = CannybotClient()             #  Connects to the first available Cannybot

cannybot.registerReceiveCallback(dataReceived)

sleep(2)  # wait a bit for connection setup

while 1:
    userinput = stdin.readline()
    if len(userinput)>0:
        cannybot.send(format(ord(userinput[0]),'02X'))

