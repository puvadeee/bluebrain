import sys
import os
import signal
from threading import Thread 
from time import sleep

import platform

VERSION=1.2

hasSyslog = False

hasSyslog = (platform.system() == "Linux")

if hasSyslog:
    import syslog
    syslog.openlog(logoption=syslog.LOG_PID, facility=syslog.LOG_DAEMON)
else:
    print "Syslog not avaialble"

import websocket
import scratch              #external dep: sudo pip install scratchpy

from cannybots.clients.wsclient import CannybotClient
from cannybots.clients.joypad import JoypadClient

CONNECTION_CHECK_DELAY=10


def log(msg):
    global hasSyslog
    print msg
    if hasSyslog:
        syslog.syslog(msg)

class ScratchAgent:
    
    def __init__(self):
        log( "Cannybots Scratch Agent v{} starting...".format(VERSION))

        self.keepRunning = True
        self.scratchInterface = None
        self.cannybot = None
    
    def startWebSocketClient(self):
        self.cannybot = CannybotClient()             #  Connects to the default Cannybot
        self.cannybot.registerReceiveCallback(self.processCannybotEvent)

        # If there's any trouble just exit, the service daemon will restart us
        self.cannybot.registerErrorCallback(self.stop)
        self.cannybot.registerClosedCallback(self.stop)
        
        self.joypad   = JoypadClient(self.cannybot) 
        

    def startScratchClient(self):
        self.scratchInterface = scratch.Scratch()
        self.scratchInterface.connect()


    def  send2scratch(self, msg):
        if self.scratchInterface:
            log("sending to scratch: " + str(msg))
            try:
                self.scratchInterface.broadcast(msg)                
            except scratch.ScratchConnectionError as sce:
                log( "ERROR: (ScratchConnection) " + sce.message)
                self.keepRunning = False
            except Exception as err:
                log( "ERROR: (General, whilst sending to Scatch) " + err.message)
                self.keepRunning = False

            
        else:
            print "WARN: Scratch not yet initialised"


    def send2cannybot(self, message):        
        self.cannybot.send(format(ord(message),'02X'))



    # TODO: make this configurable and general purpose
    def processScratchSensorUpdate(self, message):
        log("processScratchSensorUpdate: {}".format(message))
        for sensor, value in message[1].iteritems():
            log("INFO: processScratchSensorUpdate {} = {}".format(sensor, value))
            if sensor == "CannybotSpeed":
                self.joypad.setSpeed(value)
            
        
    # TODO: make this configurable and general purpose
    def processScratchBroadcast(self, message):
        commands = { 
            'MoveForward' : self.maze_forward,
            'TurnRight'   : self.maze_right,
            'TurnLeft'    : self.maze_left,
            'Spin'        : self.maze_spin,
            'Stop'        : self.maze_stop
        }
        
        command = message[1]
        #log( "Command: " + command)
        if command in commands:            
            commands[command](message);
        else:
            log( "WARN: Unrecognised broadcast: {}".format(message))

    # TODO: make this configurable and general purpose
    def processCannybotEvent(self, message):
        log("cannybot event {}".format(message))
        eventTextMappings = { 
            'RGB:R' : "Red",
            'RGB:G' : "Green",
            'RGB:B' : "Blue",
            'RGB:Y' : "Yellow",
            'LINE:N': "OffTheLine",
            'LINE:Y': "OnTheLine",
            
        }
        
        eventText = message
        #log( "Command: " + command)
        if eventText in eventTextMappings:            
            eventText=eventTextMappings[eventText];
        
        self.send2scratch(eventText)
        
    # Mazing
    def maze_forward(self, message):
        self.send2cannybot('f')

    def maze_right(self, message):
        self.send2cannybot('r')

    def maze_left(self, message):
        self.send2cannybot('l')

    def maze_spin(self, message):
        self.send2cannybot('p')

    def maze_stop(self, message):
        self.send2cannybot('s')


    #motor commands
    #led and other sensor commands


    def isConnected(self):        
        wsOK = self.cannybot.isConnected()
        scratchOK = self.scratchInterface.connected = True
        
        if (wsOK and scratchOK and self.keepRunning):
            return True
        else:
            if (not scratchOK):
                log( "Not connected to scratch")
            if (not wsOK):
                log( "Not connected to webocket API")
            return False
            
    
    def connect(self):
        self.startScratchClient();
        self.startWebSocketClient();


    def run(self):
        self.workerThread = Thread(target=self._scratch_message_worker)
        self.workerThread.daemon = True
        self.workerThread.name = 'ScratchMsgWorker'
        self.workerThread.start()
        
    def _scratch_message_worker(self):
        try:
            while self.keepRunning:
                for msg in self.listen():
                    if msg[0] == 'broadcast':
                        log( "From scratch (broadcast): " + str(msg))
                        self.processScratchBroadcast(msg)
                    elif msg[0] == 'sensor-update':
                        log( "From scratch (sensor): " + str(msg))
                        self.processScratchSensorUpdate(msg)
        except scratch.ScratchError as se:
            log( "ERROR: MsgWorker (Scratch) " + se.message)
        except StopIteration as si:
            log( "ERROR: MsgWorker (MsgProc) " + si.message)
        except Exception as err:
            log( "ERROR: MsgWorker (General) " + err.message)
            
        self.keepRunning = False

          
    def listen(self):
        while True:
            try:
               yield self.scratchInterface.receive()
            except scratch.ScratchError:
               raise StopIteration            

    def start(self):
        try:
            self.connect()
            self.run()
            while True:
                sleep(CONNECTION_CHECK_DELAY)        
                if not self.isConnected():
                    break
        except scratch.ScratchError as se:
            log ("ERROR: Main (Scratch) " + se.message)
        except Exception as err:
            log( "ERROR: Main (General) " + err.message)
        
        

    def stop(self):
        log( "Cannybots Scratch Agent exiting...")
        self.keepRunning=False;


if __name__ == "__main__":
    agent = ScratchAgent()
    agent.start()  # will block until error (e.g. connection loss)
    agent.stop()
