#!/usr/bin/python

import os
import pygame
import time
import random
from time import time

from cannybots import Display, BLE
from cannybots.utils import map

class Joypad:
    """ Joypad """
    
    def __init__ (self):
        """ init """
        self.x  = 120
        self.y  = 120
        self.b  = 0
        self.running=True
        
        self.display     = Display()
        self.view        = self.display.screen
        self.logo        = pygame.image.load("images/cannybots_logo_small.png").convert_alpha()
        self.joystick    = pygame.image.load("images/joystick.png").convert()
        self.knob        = pygame.image.load("images/knob.png").convert_alpha()
        self.button      = pygame.image.load("images/button.png").convert_alpha()

        self.ble = BLE() 
        self.myBotDevice = self.ble.findNearest()   


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
        self.view.fill((255,255,255))
        self.view.blit(self.joystick,(0,0))
        self.view.blit(self.button,(240,64))
        self.view.blit(self.logo, (320-120, 240-32))
        self.view.blit(self.knob, (self.x-50, self.y-50))
        pygame.display.update()


    def updateAxisState(self, (newX,newY)):
        if newX>240:
            newX=240
        self.x = newX
        self.y = newY     
        self.send()  
        
        
    def send(self):
        x = map(self.x-120, -120, 120, 0,255)
        y = map(self.y-120, -120, 120, 0,255)
        msg = format(x,'02X') + format(y,'02X') + format(self.b, '02X') 
        self.myBotDevice.sendHexString(msg)
    
    
    def run(self):
        self.updateAxisState((self.x,self.y))
        while self.running:
            self.handleEvents()
            self.draw()
        pygame.quit()    
            
if __name__ == "__main__":
    joypad = Joypad()
    joypad.run()
