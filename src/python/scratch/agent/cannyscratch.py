#!/usr/bin/python
#
# Cannybots Scratch integration
# By Wayne Keenan 02/12/2014, rewritten 27/05//015
# www.cannybots.com 

import sys
import os
import signal
from threading import Thread 
from time import sleep
import syslog


import websocket
import scratch              #ext. dep: sudo pip install scratchpy

from cannybots.clients.wsclient import CannybotClient

syslog.openlog(logoption=syslog.LOG_PID, facility=syslog.LOG_DAEMON)

def log(msg):
    print msg
    syslog.syslog(msg)

class ScratchAgent:
    
    def __init__(self):
        log( "Cannybots Scratch Agent starting...")

        ScratchAgent.keepRunning = True
        ScratchAgent.scratchInterface = None
        ScratchAgent.cannybot = None
    
    def startWebSocketClient(self):
        ScratchAgent.cannybot = CannybotClient()             #  Connects to the default Cannybot
        ScratchAgent.cannybot.registerReceiveCallback(self.send2scratch)

        # If there's any trouble just exit, the service daemon will restart us
        ScratchAgent.cannybot.registerErrorCallback(ScratchAgent.stop)
        ScratchAgent.cannybot.registerClosedCallback(ScratchAgent.stop)

    def startScratchClient(self):
        ScratchAgent.scratchInterface = scratch.Scratch()
        ScratchAgent.scratchInterface.connect()


    def  send2scratch(self, msg):
        if self.scratchInterface:
            log("sending to scratch: " + str(msg))
            try:
                ScratchAgent.scratchInterface.broadcast(msg)                
            except scratch.ScratchConnectionError as sce:
                log( "ERROR: (ScratchConnection) " + sce.message)
                ScratchAgent.keepRunning = False
            except Exception as err:
                log( "ERROR: (General, whilst sending to Scatch) " + err.message)
                ScratchAgent.keepRunning = False

            
        else:
            print "WARN: Scratch not yet initialised"


    @staticmethod
    def send2cannybot(message):        
        ScratchAgent.cannybot.send(format(ord(message),'02X'))



    # TODO: make this configurable and general purpose
    def processMessage(self, message):
        commands = { 
            'MoveForward' : self.maze_forward,
            'TurnRight'   : self.maze_right,
            'TurnLeft'    : self.maze_left,
            'Spin'        : self.maze_spin,
            'Stop'        : self.maze_stop
        }
        
        command = message[1]
        #print "Command: " + command
        if command in commands:            
            commands[command](message);
        else:
            log( "WARN: Unrecognised message: {}".format(message))
        
        
    # Mazing
    def maze_forward(self, message):
        self.send2cannybot('f')

    def maze_right(self, message):
        ScratchAgent.send2cannybot('r')

    def maze_left(self, message):
        ScratchAgent.send2cannybot('l')

    def maze_spin(self, message):
        ScratchAgent.send2cannybot('p')

    def maze_stop(self, message):
        ScratchAgent.send2cannybot('s')


    #motor commans
    #led and other sensor commands


    def connected(self):        
        wsOK = ScratchAgent.cannybot.isConnected()
        scratchOK = ScratchAgent.scratchInterface.connected = True
        
        if (wsOK and scratchOK and ScratchAgent.keepRunning):
            return True
        else:
            if (not scratchOK):
                log( "Not connected to scratch")
            if (not wsOK):
                log( "Not connected to webocket API")
            return False
            
    
    def run(self):
        self.startScratchClient();
        self.startWebSocketClient();
        
        self.workerThread = Thread(target=self._scratch_message_worker)
        self.workerThread.daemon = True
        self.workerThread.name = 'ScratchMsgWorker'
        self.workerThread.start()
        
    def _scratch_message_worker(self):
        try:
            while ScratchAgent.keepRunning:
                for msg in self.listen():
                    if msg[0] == 'broadcast':
                        log( "From scratch (broadcast): " + str(msg))
                        self.processMessage(msg)
                    elif msg[0] == 'sensor-update':
                        log( "From scratch (sensor): " + str(msg))
        except scratch.ScratchError as se:
            log( "ERROR: (Scratch) " + se.message)
        except StopIteration as si:
            log( "ERROR: (MsgProc) " + si.message)
        except Exception as err:
            log( "ERROR: (General) " + err.message)
            
        ScratchAgent.keepRunning = False

          
    def listen(self):
        while True:
            try:
               yield ScratchAgent.scratchInterface.receive()
            except scratch.ScratchError:
               raise StopIteration            

    @staticmethod
    def start():
        try:
            agent = ScratchAgent()
            agent.run()
            while True:
                sleep(5)        
                if not agent.connected():
                    break
        except scratch.ScratchError as se:
            log ("ERROR: (Scratch) " + se.message)
        
    @staticmethod
    def stop():
        log( "Cannybots Scratch Agent exiting...")
        ScratchAgent.keepRunning=False;
        #os.kill(os.getpid(), signal.SIGKILL)
        #thread.interrupt_main() 

if __name__ == "__main__":
    ScratchAgent.start()  # will block until error
    ScratchAgent.stop()
