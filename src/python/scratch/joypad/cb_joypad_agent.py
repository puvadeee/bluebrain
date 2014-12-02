#!/usr/bin/python
#
# Cannybots Scratch integration
# By Wayne Keenan 02/12/2014
# www.cannybots.com 

from scratra import *

from time import sleep
from cannybots.radio import BLE
from cannybots.clients.joypad  import SimpleJoypadClient

ble   = BLE() 
myBot = ble.findNearest()   
joypadClient = SimpleJoypadClient(myBot)


motorASpeed = 0
motorBSpeed = 0



@start
def whenstart(scratch):
  print 'Scratch connection started.'

@end
def whenend(scratch):
  print 'Scratch connection ended'


@broadcast('CB_FORWARD')
def forward(scratch):
  print 'move forward'
  joypadClient.updateJoypad(motorASpeed,motorBSpeed,0)

  scratch.broadcast("CB_HELLO!")

@update('motorA_speed')
def motorA_speed(scratch, value):
  global motorASpeed
  print 'motorA_speed:' + str(value)
  motorASpeed = value


@update('motorB_speed')
def motorB_speed(scratch, value):
  print 'motorB_speed:' + str(value)
  motorBSpeed = value


run(console=False)

