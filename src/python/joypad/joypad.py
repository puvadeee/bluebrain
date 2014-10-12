#!/usr/bin/python

import pygame

from cannybots.radio import BLE
from cannybots.video import Display
from cannybots.utils import arduino_map
from cannybots.clients.joypad  import SimpleJoypadClient

class Joypad:
    """ Joypad """
    
    def __init__ (self):
        """ init """
        self.x  = 120
        self.y  = 120
        self.b  = 0
        self.running = True  
        self.display = Display()
        self.screen  = self.display.screen
        
        self.loadImages()
        self.connectBot()
    
    
    def loadImages(self):
        self.logo        = pygame.image.load("images/cannybots_logo_small.png").convert_alpha()
        self.joystick    = pygame.image.load("images/joystick.png").convert()
        self.knob        = pygame.image.load("images/knob.png").convert_alpha()
        self.button      = pygame.image.load("images/button.png").convert_alpha()
    
    
    def connectBot(self):
        self.ble   = BLE() 
        self.myBot = self.ble.findNearest()   
        self.joypadClient = SimpleJoypadClient(self.myBot)

    def handleEvents(self):
        event = pygame.event.poll()
        
        if event.type == pygame.QUIT:
            self.running = 0
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                self.running=False
        elif self.display.isX and event.type == pygame.MOUSEMOTION:            
            (b1,b2,b3) = pygame.mouse.get_pressed()
            if b1:
                self.updateAxisState(event.pos)
        elif event.type == pygame.MOUSEMOTION:
            self.updateAxisState(event.pos)
            
        elif event.type == pygame.MOUSEBUTTONUP:
                self.updateAxisState((120,120))

    def draw(self):
        self.screen.fill((255,255,255))
        self.screen.blit(self.joystick,(0,0))
        self.screen.blit(self.button,(240,64))
        self.screen.blit(self.logo, (320-120, 240-32))
        self.screen.blit(self.knob, (self.x-50, self.y-50))
        pygame.display.update()


    def updateAxisState(self, (newX,newY)):
        if newX>240:
            newX=240
        self.x = newX
        self.y = newY 
        x = arduino_map(self.x-120, -120, 120, -255,255)
        y = arduino_map(self.y-120, -120, 120, -255,255)    
        self.joypadClient.updateJoypad(x, y, self.b)  
   
    
    def run(self):
        while self.running:
            self.handleEvents()
            self.draw()
        pygame.quit()    
            
if __name__ == "__main__":
    joypad = Joypad()
    joypad.run()
