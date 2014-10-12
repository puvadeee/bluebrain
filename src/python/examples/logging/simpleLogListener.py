
import time
from cannybots.radio import BLE

def myRecvFunc(bleuart, message):
	print message

ble = BLE() 
myBot = ble.findNearest()   
myBot.addListener(myRecvFunc)

while True:
	time.sleep(1)

