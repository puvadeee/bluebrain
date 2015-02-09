import sys

sys.path.insert(0, "modules")

import time
from cannybots.radio import BLE


def myRecvFunc(bleuart, message):
    print message


def myOnDisconnectFunc(ble, message):
    print message


ble = BLE()

myBot = ble.findByName('PingEchoBot')
myBot.addListener(myRecvFunc)
myBot.onDisconnect(myOnDisconnectFunc)

while True:
    time.sleep(3)
    msg = format(1, '02X') + format(2, '02X') + format(3, '02X') + format(4, '02X')
    print "sending: " + msg
    #myBot.sendHexString(msg)

