#!/usr/bin/python
#
# Cannybots Scratch integration
# By Wayne Keenan 02/12/2014
# www.cannybots.com 
import sys
import os
import signal
from threading import Thread 
from time import sleep
import websocket
from scratra import *

from cannybots.clients.wsclient import CannybotClient

keepRunning = True




def signal_handler(signal, frame):
    print('Exit due to Ctrl+C')
    ScratchAgent.exitAgent()

signal.signal(signal.SIGINT, signal_handler)


class ScratchAgent:
    
    def __init__(self):

        ScratchAgent.keepRunning = True
        ScratchAgent.scratchInterface = None
        
        ScratchAgent.cannybot = CannybotClient()             #  Connects to the default Cannybot
        ScratchAgent.cannybot.registerReceiveCallback(self.send2scratch)

        # If there's any trouble just exit, the service daemon will restart us
        ScratchAgent.cannybot.registerErrorCallback(ScratchAgent.exitAgent)
        ScratchAgent.cannybot.registerClosedCallback(ScratchAgent.exitAgent)

        run(console=False)      # calls Scratera

    def  send2scratch(self, msg):
        if self.scratchInterface:
            print "sending to scratch: " + str(msg)
            ScratchAgent.scratchInterface.broadcast(msg)
        else:
            print "WARN: Scratch not yet connected"
    
    @staticmethod
    @start
    def whenstart(scratch):
        ScratchAgent.scratchInterface = scratch
        print 'Scratch connection started.'


    @staticmethod
    @end
    def whenend(scratch):
        ScratchAgent.scratchInterface = None
        print 'Scratch connection ended'

    # Mazing
    @staticmethod
    def send2maze(cmd):
        ScratchAgent.cannybot.send(format(ord(cmd),'02X'))

    @staticmethod
    @broadcast('MoveForward')
    def maze_forward(scratch):
        ScratchAgent.send2maze('f')

    @staticmethod
    @broadcast('TurnRight')
    def maze_right(scratch):
        ScratchAgent.send2maze('r')

    @staticmethod
    @broadcast('TurnLeft')
    def maze_left(scratch):
        ScratchAgent.send2maze('l')

    @staticmethod
    @broadcast('Spin')
    def maze_spin(scratch):
        ScratchAgent.send2maze('p')

    @staticmethod
    @broadcast('Stop')
    def maze_stop(scratch):
        ScratchAgent.send2maze('s')


    @staticmethod
    def exitAgent():
        print "Cannybots Scratch Agent exiting..."
        os.kill(os.getpid(), signal.SIGKILL)
        thread.interrupt_main() 

    def connected(self):
        wsOK = ScratchAgent.cannybot.isConnected()
        scratchOK = ScratchAgent.scratchInterface!=None
        if (wsOK and scratchOK):
            return True
        else:
            if (not scratchOK):
                print "Not connected to scratch"
            if (not wsOK):
                print "Not connected to webocket API"
            return False
            

if __name__ == "__main__":
    
    agent = ScratchAgent()
    while keepRunning:
        sleep(2)
        if (not agent.connected()):          
            keepRunning=False
        #signal.pause()    
    ScratchAgent.exitAgent()
