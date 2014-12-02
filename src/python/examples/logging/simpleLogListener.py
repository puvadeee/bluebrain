
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
	time.sleep(1)

