#!/usr/bin/python
#
# Cannybots Scratch integration
# By Wayne Keenan 02/12/2014
# www.cannybots.com 
import sys
import os
from threading import Thread
import json
import base64

#sys.path.insert(0, "../../modules")

import websocket
from scratra import *

keepRunning = True
import signal
def signal_handler(signal, frame):
        print('Exit due to Ctrl+C')
        keepRunning=False
        sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)
        
        
ws = None
wsConnected = False
scratchInterface = None


turtleDistance = 0
turtleAngle    = 0

def send2turtle(cmd, p1):
    global ws
    global wsConnected

    #payload = {}
    #payload["rawBytes"] = [ord('f'), 0, 10]
    #msg = json.dumps(payload)

    #msg = "{'rawBytes': [ " + str(ord('f')) + ", " + str(0) + ", "  + str(10) + "]}"
    #[0,0,cmd.charCodeAt(0), p1>>8, p1 &0xFF,rChar];

    msg = '{"rawBytes":[0,0,' + str(ord('f')) + "," + str(0) + ","  + str(10) + "]}"
    #msg = base64.b64encode(msg)

    #msg =  chr(0) +  chr(0) + 'f' + chr(0) + chr(10) + '\r'
    print "WS send " + str(msg)
    try:
        if wsConnected:
            ws.send(msg)
        else:
            print "WARN: WS not yet connected"
    except Exception as e:
        print "ERROR: ws send failed: " + str(e)

def  send2scratch(msg):
    global scratchInterface
    if scratchInterface:
        print "sending to scratch: " + str(msg)
        scratchInterface.broadcast(msg)
    else:
        print "WARN: Scratch not yet connected"



@start
def whenstart(scratch):
    global scratchInterface
    scratchInterface = scratch
    print 'Scratch connection started.'


@end
def whenend(scratch):
    print 'Scratch connection ended'



# Tutle commands


@update('turtleDistance')
def turtleDistance(scratch, value):
    global turtleDistance
    print 'turtleDistance:' + str(value)
    turtleDistance = value


@update('turtleAngle')
def turtleAngle(scratch, value):
    global turtleAngle
    print 'turtleAngle:' + str(value)
    turtleAngle = value




@broadcast('Turtle_Forward')
def forward(scratch):
    send2turtle('f', turtleDistance)

@broadcast('Turtle_Backward')
def forward(scratch):
    send2turtle('b', turtleDistance)


@broadcast('Turtle_Right')
def forward(scratch):
    send2turtle('r', turtleAngle)

@broadcast('Turtle_Left')
def forward(scratch):
    send2turtle('l', turtleAngle)


# Mazing

def send2maze(cmd):
    global ws
    global wsConnected

    msg = '{"rawBytes":["'+ cmd + '"]}'
    print "WS send " + str(msg)
    #msg = base64.b64encode(msg)

    try:
        if wsConnected:
            ws.send(msg)
        else:
            print "WARN: WS not yet connected"
    except Exception as e:
        print "ERROR: ws send failed: " + str(e)

@broadcast('MoveForward')
def maze_forward(scratch):
    send2maze('f')

@broadcast('TurnRight')
def maze_right(scratch):
    send2maze('r')

@broadcast('TurnLeft')
def maze_left(scratch):
    send2maze('l')

@broadcast('Spin')
def maze_spin(scratch):
    send2maze('p')

@broadcast('Stop')
def maze_stop(scratch):
    send2maze('s')



# Web Socket

def on_message(ws, message):
    message = base64.b64decode(message)
    print "WS recv: " + str(message)
    #jsonObj = json.loads(message)
    #print jsonObj
    #if 'payload' in jsonObj:
    #    line = jsonObj.get('payload').rstrip("\r\n") 
    send2scratch(message)


def on_error(ws, error):
    print "ERROR: ws, " + str(error)
    fatalError()

def on_close(ws):
    print "### closed ###"
    fatalError()

def on_open(ws):
    global wsConnected
    print "### opened ###"
    wsConnected = True

def fatalError():
    os.kill(os.getpid(), signal.SIGINT)
    #thread.interrupt_main() 

if __name__ == "__main__":
    #websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://localhost:3141/api/ws/cannybots",
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)
    ws.on_open = on_open

    def runWs(*args):
        global ws
        ws.run_forever()
        ws.close()
        print "thread terminating..."
        keepRunning = False
        fatalError()

    #thread.start_new_thread(runWs, ())
    wst = Thread(target=runWs)
    wst.daemon = True
    wst.name = "WS Thread"
    wst.start()

    run(console=False)
    while keepRunning:           
        signal.pause()    
