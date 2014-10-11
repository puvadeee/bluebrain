#!/usr/bin/python
#
# Cannybots BLE Python Library
#
# Authors:  Wayne Keenan
#
# License:  http://opensource.org/licenses/MIT
#
# Version:   1.0  -  11.10.2014  -  Inital Version  (wayne@cannybots.com)
#


import os
import time
import array
import Queue
from threading import  Thread, Lock

import pexpect

#runs in background:  sudo hcitool lescan --duplicates --passive
#outputs:     		 FF:C3:F0:EC:C3:D9 CB_1f73956a95eb

#for each address:
# gatttool -t random --primary -b  FF:C3:F0:EC:C3:D9
#attr handle = 0x0001, end grp handle = 0x0007 uuid: 00001800-0000-1000-8000-00805f9b34fb
#attr handle = 0x0008, end grp handle = 0x000b uuid: 00001801-0000-1000-8000-00805f9b34fb
#attr handle = 0x000c, end grp handle = 0xffff uuid: 7e400001-b5a3-f393-e0a9-e50e24dcca9e



# RX:
#[CON][FF:C3:F0:EC:C3:D9][LE]> char-write-req 0x000f 0100
#[CON][FF:C3:F0:EC:C3:D9][LE]> Characteristic value was written successfully

#Notification handle = 0x000e value: 68 65 6c 6c 6f 
#[CON][FF:C3:F0:EC:C3:D9][LE]> 
#Notification handle = 0x000e value: 68 65 6c 6c 6f 


MAX_TX_QUEUE_SIZE=3

CB_DEFAULT_SERVICEUUID='7e400001-b5a3-f393-e0a9-e50e24dcca9e'
CB_RX_UUID=''
CB_TX_UUID=''

def die(child, errstr):
	print 'ERROR: '+errstr
	print child.before, child.after
	child.terminate()



class BLE_UART:
	# Background threads:   gatttool lister for TX sending and RX notifications

	def startGattTool(self):
		os.system("killall gatttool")
		cmd='/usr/bin/gatttool -b ' + self.mac +'  -t random -I'
		self.child = pexpect.spawn(cmd)
		i = self.child.expect ([pexpect.TIMEOUT, '\[LE\]>'])
		if i == 0:
			die(self.child, 'gatttool timed out. Here is what gatttool said:')
			
		self.enqueString('connect')
		# Start the RX listener	
		self.enqueString('char-write-req 0x000f 0100')

	def tx_worker(self):
		while self.keepRunning:
			item = self.q.get(True)     # get command or if none are waiting this will block
			self._send(item)
			self.q.task_done()

	def rx_worker(self):
		while self.keepRunning:
			self.queueLock.acquire()
			self.child.expect("Notification handle = 0x000e value:\s(.*)\n")	
			if self.rxListener:
				self.rxListener(self, self.child.after.split("value: ",1)[1].rstrip().replace(" ", "").decode("hex"))
			self.queueLock.release()
			


	def __init__(self, mac,name='BLE_UART'):
		self.mac=mac
		self.keepRunning = True
		
		self.queueLock = Lock()
		self.q = Queue.Queue(maxsize=MAX_TX_QUEUE_SIZE)
	
		self.rxListener=False
		self.startGattTool()
		self.tx = Thread(target=self.tx_worker)
		self.tx.daemon = True
		self.tx.name = name
		self.tx.start()
		
		self.rx = Thread(target=self.rx_worker)
		self.rx.daemon = True
		self.rx.name = name
		self.rx.start()
		
		
	def addListener(self,func):
		self.rxListener = func
		
	def sendHexString(self, hexString):
		cmd = 'char-write-cmd 0x0011 ' + hexString
		self.enqueString(cmd)

	def enqueString(self, str):
		if self.queueLock.locked():
			return
		self.queueLock.acquire()
		self.q.put(str)
		self.queueLock.release()
		

	def _send(self, msg):
		self.child.sendline (msg)
		i = self.child.expect ([pexpect.TIMEOUT,'Command failed:','\[LE\]>'])
		if i == 0:
			die(self.child, 'gatttool timed out. detail:')
		elif i == 1:
			die(self.child, 'command failed, detail:')




class BLE:
# Background threads:   lescam

	def __init__(self,name='BLEThread'):
		pass

	def findAll(self):
		deviceName = 'cannybot1'
		bleuart1= BLE_UART(mac='FF:C3:F0:EC:C3:D9', name=deviceName)
		return [bleuart1]
		

