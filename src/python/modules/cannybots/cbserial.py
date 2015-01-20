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
import signal
import sys
import glob
import time; 
from time import sleep
from threading import Thread
import serial
from utils import arduino_map, arduino_constrain


def cbserial_deafultCallback(line):
	#print("SERIAL_RX:"+line)
	return

class CBSerial:

	def getSerialPorts(self):
		return glob.glob('/sys/class/tty/*/device/driver')
	
	def __init__(self, port, baud, rxDelegate=cbserial_deafultCallback):
		self.serialConnected=False;
		self.rxDelegate=rxDelegate;
		try:
			self.ser = serial.Serial(port, baud, timeout=1)
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
			
	def registerRXDelegate(self, delegate):
		self.rxDelegate=delegate
		
SEND_MOTOR_UPDATE_INTERVAL=0.02
DEFAULT_CRUISE_SPEED = 20
IR_WHITE_THRESHOLD = 780

IR1_BIAS=80
class LineFollowing:
	
	def __init__(self, connection):
		signal.signal(signal.SIGINT, self.signal_handler)
		self.cruiseSpeed=DEFAULT_CRUISE_SPEED
		self.irVals=[0,0,0]
		self.connection = connection
		self.connection.registerRXDelegate(self.myRXDelegate)
		print self.connection.getSerialPorts()

	def myRXDelegate(self, line):
		#print("myRXDelegate: " + line)
		parts = line.split(':')
		if len(parts)>1:
			(cmd, params) = parts
			#print ("cmd: " + cmd + "\tparams: " + params)
			if cmd == "IR":
				irVals = params.split(',')
				if len(irVals)==3:
					self.irVals[0]=int(irVals[0])+IR1_BIAS
					self.irVals[1]=int(irVals[1])
					self.irVals[2]=int(irVals[2])


	def run(self):

		nextSend = time.time() + SEND_MOTOR_UPDATE_INTERVAL
		Kp = 20
		Ki = 0
		Kd = 0
		P_error = 0
		I_sum = 0
		I_error = 0
		D_error = 0
		error = 0
		error_last = 0
		correction = 0.0
		speedA=0
		speedB=0
		
		while 1:
			#print(str(nextSend) + "\t" + str(time.time()))
			
			if time.time() > nextSend:
	 
				if self.irVals[1]>IR_WHITE_THRESHOLD:
					error_last = error
					error = self.irVals[0] - self.irVals[2]
					P_error = error * Kp / 100.0 # calculate proportional term
					I_sum = arduino_constrain ((I_sum + error), -1000, 1000) # integral term
					I_error = (I_sum * Ki) / 100.0
					D_error = (error - error_last) * Kd / 100.0          #calculate differential term
					correction = P_error + D_error + I_error
					
					speedA = int(self.cruiseSpeed + correction)
					speedB = int(self.cruiseSpeed - correction)

				else:
					speedA=0
					speedB=0
				
				speedA = arduino_constrain(speedA, -255, 255)
				speedB = arduino_constrain(speedB, -255, 255)
				speedA = arduino_map(speedA, -255, 255, 0, 255)
				speedB = arduino_map(speedB, -255, 255, 0, 255)
				#print("IR:",self.irVals[0], self.irVals[1], self.irVals[2], "A,B",speedA, speedB)
				#http://stackoverflow.com/questions/7380460/byte-array-in-python
				byteStr = ''.join(chr(x) for x in [0x00, 0x00, ord('m'), speedA, speedB,ord('\r')])
				self.connection.send(byteStr)
				nextSend=time.time()  + SEND_MOTOR_UPDATE_INTERVAL



	def halt(self):
		byteStr = ''.join(chr(x) for x in [0x00, 0x00, ord('s'), 0, 0,ord('\r')])
		self.connection.send(byteStr)
		print("HALT!")


	def signal_handler(self, signal, frame):
		self.halt()
		sys.exit(0)






if __name__ == "__main__":
	ser= CBSerial('/dev/ttyAMA0', 115200) 
	lf = LineFollowing(ser)
	lf.run()
