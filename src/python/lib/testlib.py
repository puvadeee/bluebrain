import cannybots
import time

ble = cannybots.BLE()   #defalts to hci0, 


myBotDevices = ble.findAll()   
myBotDevice = myBotDevices[0]


def myRecvFunc(bleuart, message):
	print message


# raw byte oritend interface
myBotDevice.addListener(myRecvFunc)
#myBotDevice.sendBytes([255,255,0])


while True:
	time.sleep(1)

# high level classes that need a BLE object to act on 
#joypad = cannybots.apps.joypad(myBotDevice)
#joypad.send(x,y,b)

