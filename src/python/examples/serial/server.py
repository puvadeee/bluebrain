import serial
import socket
import threading
import time

#PORT = '/dev/tty.PL2303-00002006'
PORT = '/dev/tty.PL2303-00001004'
BAUD = 57600
#BAUD = 115200

UDP_IP = "127.0.0.1"
UDP_PORT = 3141





class serialserver(threading.Thread):

    def __init__ (self, udp):
        threading.Thread.__init__(self)
        self.ser = serial.Serial(PORT, BAUD, timeout=3)
        #self.ser = serial.Serial('/dev/ttys000', BAUD, timeout=3)
        self.udp = udp


    def run(self):
        #while True:
        #    rcv = self.ser.read(10)
        #    print("You sent:" + repr(rcv))

        buffer = ''
        while True:
		self.udp.send(self.ser.read(self.ser.inWaiting()))
		time.sleep(0.001)



class udpserver(threading.Thread):

    def __init__ (self):
        threading.Thread.__init__(self)
        print "init"

    def run(self):
        print "UDP target IP:", UDP_IP
        print "UDP target port:", UDP_PORT
        self.sock = socket.socket(socket.AF_INET, # Internet
                                  socket.SOCK_DGRAM) # UDP

    def send(self, msg):
        print "message:", msg

        self.sock.sendto(msg, (UDP_IP, UDP_PORT))
	#time.sleep(1)



udp = udpserver()
print "1"
udp.start()
print "2"
serial = serialserver(udp)
print "3"
serial.start()
print "4"
