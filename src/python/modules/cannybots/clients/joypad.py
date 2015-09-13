
from cannybots.utils import arduino_map


class SimpleJoypadClient:

    def __init__(self, bot):
        self.bot = bot

    def __removed__updateJoypad(self, x, y, b):
        #print x,y,b
        x = arduino_map(x, -255, 255, 0, 255)
        y = arduino_map(-y, -255, 255, 0, 255)
        msg = format(x,'02X') + format(y,'02X') + format(b, '02X') + format(127, '02X')
        self.bot.sendHexString(msg)

    def __removed_updateJoypadWithZ(self, x,y,z,b):
        print x, y, z, b
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
        z = arduino_map(-z, -255, 255, 0,255)
        return format(x,'02X') + format(y,'02X') + format(b, '02X') + format(z, '02X')

    @staticmethod
    def createGetStatusMessage():
        return format(ord('i'),'02X')




class JoypadClient:

    def __init__(self, ws):
        self.ws = ws

    # axis values are in range -255 .. 255
    # button is a 8bit bitmask for 8 buttons
    def update(self, xAxis, yAxis, zAxis, buttons):
        msg = SimpleJoypadClient.createJoypadMessage(int(xAxis), int(yAxis), int(zAxis), int(buttons))
        self.ws.send(msg)
    
    def stop(self):
        self.update(0, 0, 0, 0)

    
    # Set speed 0..255  (slow..fast)
    
    def setSpeed(self, speed):
        if speed < 0:
            speed=0
        if speed > 255:
            speed = 255
        speed = (speed * 2) - 256 
        # conver speed from 0-255 to -255 to 255
        self.update(0, 0, speed,0)
            
        
    def requestStatus(self):
        msg = SimpleJoypadClient.createGetStatusMessage()
        self.ws.send(msg)
