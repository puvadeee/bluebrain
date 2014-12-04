
from cannybots.utils import arduino_map


class SimpleJoypadClient:
    
    def __init__(self, bot):
        self.bot=bot
        
    
    def  updateJoypad(self, x,y,b):
		print x,y,b
		x = arduino_map(x, -255, 255, 0,255)
		y = arduino_map(-y, -255, 255, 0,255)
		msg = format(x,'02X') + format(y,'02X') + format(b, '02X') + format(128, '02X') 
		self.bot.sendHexString(msg) 

    def  updateJoypadWithZ(self, x,y,z,b):
		print x,y,z,b
		x = arduino_map(x, -255, 255, 0,255)
		y = arduino_map(-y, -255, 255, 0,255)
		z = arduino_map(z, 0, 255, 0,255)
		msg = format(x,'02X') + format(y,'02X') + format(b, '02X') + format(z, '02X') 
		self.bot.sendHexString(msg) 
