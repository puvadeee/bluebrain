#!/usr/bin/python

botName = "PingEchoBot"

import sys
import os
basedir = os.path.abspath(os.path.dirname(__file__))
#basedir = os.path.dirname(__file__)
import time
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
        self.z  = 240-64
        self.b  = 0
        self.lastUpdateTime = time.time()
        self.running = True  
        self.display = Display()
        self.screen  = self.display.screen
        
        self.loadImages()
        self.connectBot()
        self.clock = pygame.time.Clock()
    
    
    def loadImages(self):
        self.logo        = pygame.image.load(basedir+"/images/cannybots_logo_small.png").convert_alpha()
        self.joystick    = pygame.image.load(basedir+"/images/joystick.png").convert_alpha()
        self.knob        = pygame.image.load(basedir+"/images/knob.png").convert_alpha()
        self.button      = pygame.image.load(basedir+"/images/button.png").convert_alpha()
    
    
    def connectBot(self):
        self.ble   = BLE() 
        #self.myBot = self.ble.findNearest()   
        self.myBot = self.ble.findByName('CannyBot1')   
        self.joypadClient = SimpleJoypadClient(self.myBot)

    def updateJoypad(self, (newX,newY), force):   
        if newX>320-64:
			self.z=newY-32 
   
        if newX<240:
            self.x = newX
            self.y = newY 			
            
        if force:
			self.z=240-64;
	

        if force or (time.time() - self.lastUpdateTime) > 0.1:
			x = arduino_map(self.x-120, -120, 120, -255,255)
			y = arduino_map(self.y-120, -120, 120, -255,255)    
			z = arduino_map(self.z, 0, 240-32, 255, 0)    
			self.joypadClient.updateJoypadWithZ(x, y, -z,self.b) 
			self.lastUpdateTime = time.time()

    def handleEvents(self):
        for event in pygame.event.get():
			if event.type == pygame.QUIT:
				self.running = 0
			elif event.type == pygame.KEYDOWN:
				if event.key == pygame.K_ESCAPE:
					self.running=False
			elif event.type == pygame.MOUSEMOTION:   
				(b1,b2,b3) = pygame.mouse.get_pressed()
				if b1:
					self.updateJoypad(event.pos, False)
			elif event.type == pygame.MOUSEBUTTONUP:
					self.updateJoypad((120,120), True)

    def draw(self):
        self.screen.fill((255,255,255))
        self.screen.blit(self.logo, (320-120, 0))
        self.screen.blit(self.joystick,(0,0))
        self.screen.blit(self.button,(240,self.z))
        self.screen.blit(self.knob, (self.x-50, self.y-50))
        pygame.display.update()
   
    
    def run(self):
        while self.running:
            self.handleEvents()
            self.draw()
            self.clock.tick(60)
        pygame.quit()    
            
if __name__ == "__main__":
    joypad = Joypad()
    joypad.run()
