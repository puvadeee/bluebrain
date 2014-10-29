import sys
import serial
import socket
import threading
import time
import glob


import signal,sys
def catch_ctrl_C(sig,frame):
    sys.exit()
signal.signal(signal.SIGINT, catch_ctrl_C)


PORT = ''
BAUD = 9600

UDP_IP = "127.0.0.1"
UDP_PORT = 3141


def serial_ports():
    """Lists serial ports

    :raises EnvironmentError:
        On unsupported or unknown platforms
    :returns:
        A list of available serial ports
    """
    if sys.platform.startswith('win'):
        ports = ['COM' + str(i + 1) for i in range(256)]

    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this is to exclude your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')

    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')

    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result


class serialserver(threading.Thread):

    def __init__ (self, udp):
        threading.Thread.__init__(self)
        self.ser = serial.Serial(PORT, BAUD, timeout=3)
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
		if msg:
			print "message:", msg
			self.sock.sendto(msg, (UDP_IP, UDP_PORT))
	#time.sleep(1)



if __name__ == '__main__':
	ports = serial_ports()
	print(ports)
	PORT=ports[0]
	udp = udpserver()
	print "1"
	udp.start()
	print "2"
	serial = serialserver(udp)
	print "3"
	serial.start()
	print "4"
