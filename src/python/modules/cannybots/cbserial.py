#!/usr/bin/python
#
# Cannybots Serial Python Library
#
# Authors:  Wayne Keenan
#
# License:  http://opensource.org/licenses/MIT
#
# Version:   1.0  -  19.01.2015  -  Inital Version  (wayne@cannybots.com)
#

import glob
from threading import Thread

import serial
from time import sleep

def cbserial_deafultCallback(line):
	print("SERIAL_RX:"+line)

class CBSerial:

	def getSerialPorts(self):
		return glob.glob('/sys/class/tty/*/device/driver')
	
	def __init__(self, port, rxDelegate=cbserial_deafultCallback):
		self.serialConnected=False;
		self.rxDelegate=rxDelegate;
		try:
			self.ser = serial.Serial(port, 9600, timeout=1)
			self.serialConnected=True;
			self.t = Thread(target=self.readPoller)
			self.t.daemon = True
			self.t.start()

		except Exception as se:
			print("Serial exception" + str(se))

	def send(self, data):
		if (self.serialConnected):
			self.ser.write(data)
			self.ser.flush()

    # may want to look at http://stackoverflow.com/questions/10222788/line-buffered-serial-input
	def read(self):
		if (self.serialConnected):
			line=self.ser.readline()
		else:
			line="SERIAL_NOT_CONNECTED"
		line= str(line.rstrip("\n\r"))	
		self.rxDelegate(line)
		return line


	def readPoller(self):
		while self.serialConnected:
			readLine = self.read()            

def myRXDelegate(line):
	print("mines: " + line)

if __name__ == "__main__":
	mySerial = CBSerial('/dev/ttyAMA0', rxDelegate=myRXDelegate)
	print mySerial.getSerialPorts()
	while 1:
		mySerial.send("00m00\r")
		#sleep(1)
