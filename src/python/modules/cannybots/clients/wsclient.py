import os
import websocket
from threading import Thread
import base64

class CannybotClient:

    def __init__(self, botId=0, host="localhost", port="3141", url="/api/ws/cannybots"):
        self.botId = str(botId)
        self.wsConnected = False
        self._initWS(host, port, url)
        self.receiveCallback= None
        self.debug = "CANNYBOTS_DEBUG_PYTHON_LIB" in os.environ


    def _initWS(self, host, port, url):

        self.ws = websocket.WebSocketApp("ws://{}:{}{}".format(host, port, url),
                                    on_message=self._on_message,
                                    on_error=self._on_error,
                                    on_close=self._on_close)
        self.ws.on_open = self._on_open

        self.wst = Thread(target=self._runWs)
        self.wst.daemon = True
        self.wst.name = "CB WS Thread"
        self.wst.start()

    def _runWs(self, *args):
        try:
            self.ws.run_forever()
        except Exception as e:
            print "ERROR: ws run failed: " + str(e)
        self.ws.close()
        self.wsConnected = False


    def _on_message(self, ws, message):
        message = base64.b64decode(message)
        if self.debug:
            print "WS recv: " + str(message)
        if self.receiveCallback:
            self.receiveCallback(message)

    def _on_error(self, ws, error):
        print "ERROR: ws, " + str(error)

    def _on_close(self, ws):
        if self.debug:
            print "### WebSocket closed ###"

    def _on_open(self, ws):
        if self.debug:
            print "### WebSocket opened ###"
        self.wsConnected = True


    def send(self, hexBytes):

        msg = '{"botId":"' + self.botId + '","hexBytes":["' + hexBytes + '"]}'
        if self.debug:
            print "WS send " + str(msg)

        try:
            if self.wsConnected:
                self.ws.send(msg)
            else:
                print "WARN: WS not yet connected"
        except Exception as e:
            print "ERROR: ws send failed: " + str(e)


    def registerReceiveCallback(self, callback):
        self.receiveCallback = callback
