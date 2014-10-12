
class SimpleJoypadClient:
    
    def __init__(self, bot):
        self.bot=bot
        
    
    def  updateJoypad(self, x,y,b):
        x = map(x, -255, 255, 0,255)
        y = map(y, -255, 255, 0,255)
        msg = format(x,'02X') + format(y,'02X') + format(b, '02X') 
        self.myBotDevice.sendHexString(msg) 
