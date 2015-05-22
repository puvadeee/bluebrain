#!/usr/bin/python

from time import sleep
from cannybots.radio import BLE
from cannybots.clients.joypad  import SimpleJoypadClient

ble   = BLE() 
myBot = ble.findNearest()   
joypadClient = SimpleJoypadClient(myBot)

joypadClient.updateJoypad(255,255,0) 
sleep(1)

joypadClient.updateJoypad(-255,-255,0)
sleep(1)

joypadClient.updateJoypad(0,0,0) 
sleep(2)
