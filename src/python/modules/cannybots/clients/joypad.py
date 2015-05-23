
from cannybots.utils import arduino_map

from threading import Thread
import websocket
import base64
import signal

class SimpleJoypadClient:

    def __init__(self, bot):
        self.bot = bot

    def updateJoypad(self, x, y, b):
        #print x,y,b
        x = arduino_map(x, -255, 255, 0, 255)
        y = arduino_map(-y, -255, 255, 0, 255)
        msg = format(x,'02X') + format(y,'02X') + format(b, '02X') + format(127, '02X')
        self.bot.sendHexString(msg)

    def updateJoypadWithZ(self, x,y,z,b):
        #print x, y, z, b
        x = arduino_map(x, -255, 255, 0,255)
        y = arduino_map(-y, -255, 255, 0,255)
        z = arduino_map(z, -255, 255, 0,255)
        msg = format(x,'02X') + format(y,'02X') + format(b, '02X') + format(z, '02X')
        self.bot.sendHexString(msg)

    @staticmethod
    def createJoypadMessage(x,y,z,b):
        #print x,y,z,b
        x = arduino_map(x, -255, 255, 0,255)
        y = arduino_map(-y, -255, 255, 0,255)
        z = arduino_map(z, -255, 255, 0,255)
        return '"' + format(x,'02X') + format(y,'02X') + format(b, '02X') + format(z, '02X') + '"'

    @staticmethod
    def createGetStatusMessage():
        return '"' + format(ord('i'),'02X') + '"'




class JoypadClient:

    def __init__(self, botId=1, host="localhost", port="3141", url="/api/ws/cannybots"):
        self.botId = botId
        self.wsConnected = False
        self.keepRunning = True
        self.initWS(host, port, url)


    # axis values are in range -255 .. 255
    # button is a 32bit bitmask for 32 buttons
    def update(self, xAxis, yAxis, zAxis, buttons):
        msg = SimpleJoypadClient.createJoypadMessage(int(xAxis), int(yAxis), int(zAxis), int(buttons))
        self.send(msg)

    def stop(self):
        self.update(0, 0, 0, 0)


    # Web Socket
    def initWS(self, host, port, url):

        self.ws = websocket.WebSocketApp("ws://{}:{}{}".format(host, port, url),
                                    on_message=self.on_message,
                                    on_error=self.on_error,
                                    on_close=self.on_close)
        self.ws.on_open = self.on_open

        #thread.start_new_thread(runWs, ())
        self.wst = Thread(target=self.runWs)
        self.wst.daemon = True
        self.wst.name = "CB WS Thread"
        self.wst.start()

    def runWs(self, *args):
        try:
            self.ws.run_forever()
        except Exception as e:
            print "ERROR: ws run failed: " + str(e)
        self.ws.close()
        self.keepRunning = False


    @staticmethod
    def on_message(ws, message):
        message = base64.b64decode(message)
        print "WS recv: " + str(message)

    @staticmethod
    def on_error(ws, error):
        print "ERROR: ws, " + str(error)

    @staticmethod
    def on_close(ws):
        print "### closed ###"

    def on_open(self, ws):
        print "### opened ###"
        self.wsConnected = True


    def send(self, hexBytes):

        msg = '{"hexBytes":['+ hexBytes + ']}'
        print "WS send " + str(msg)

        try:
            if self.wsConnected:
                self.ws.send(msg)
            else:
                print "WARN: WS not yet connected"
        except Exception as e:
            print "ERROR: ws send failed: " + str(e)

