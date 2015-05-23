#!/usr/bin/python
#
# Cannybots Scratch integration
# By Wayne Keenan 02/12/2014
# www.cannybots.com 
import sys
import os
from time import sleep
from threading import Thread
import json
import base64
import signal
from cannybots.clients.joypad  import SimpleJoypadClient

#sys.path.insert(0, "../../modules")

import websocket

keepRunning = True
def signal_handler(signal, frame):
        print('Exit due to Ctrl+C')
        keepRunning=False
        fatalError()

signal.signal(signal.SIGINT, signal_handler)
        

host = 'localhost'        
ws = None
wsConnected = False

if (len(sys.argv) > 1):
    host=sys.argv[1]


def send2cannybot(rawBytes):
    global ws
    global wsConnected

    msg = '{"hexBytes":['+ rawBytes + ']}'
    print "WS send " + str(msg)
    #msg = base64.b64encode(msg)

    try:
        if wsConnected:
            ws.send(msg)
        else:
            print "WARN: WS not yet connected"
    except Exception as e:
        print "ERROR: ws send failed: " + str(e)


# Web Socket

def on_message(ws, message):
    message = base64.b64decode(message)
    print "WS recv: " + str(message)
    #jsonObj = json.loads(message)
    #print jsonObj
    #if 'payload' in jsonObj:
    #    line = jsonObj.get('payload').rstrip("\r\n") 


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
    os.kill(os.getpid(), signal.SIGKILL)
    #thread.interrupt_main() 

if __name__ == "__main__":
    #websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://"+host+":3141/api/ws/cannybots",
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)
    ws.on_open = on_open

    def runWs(*args):
        global ws
        try:
            ws.run_forever()
        except Exception as e:
            print "ERROR: ws run failed: " + str(e)
            
        ws.close()
        print "thread terminating..."
        keepRunning = False
        fatalError()

    #thread.start_new_thread(runWs, ())
    wst = Thread(target=runWs)
    wst.daemon = True
    wst.name = "WS Thread"
    wst.start()

    while keepRunning:

        joypadMessage = SimpleJoypadClient.createJoypadMessage(255,255,0,0)
        print joypadMessage
        send2cannybot(joypadMessage)
        sleep(1)
        
        joypadMessage = SimpleJoypadClient.createJoypadMessage(0,0,0,0)
        print joypadMessage
        send2cannybot(joypadMessage)
        sleep(1)

        joypadMessage = SimpleJoypadClient.createGetStatusMessage()
        print joypadMessage
        send2cannybot(joypadMessage)
        sleep(1)



       #signal.pause()    
