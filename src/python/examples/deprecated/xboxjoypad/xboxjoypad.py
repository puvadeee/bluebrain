import time

import pygame

from time import sleep
from cannybots.radio import BLE
from cannybots.clients.joypad  import SimpleJoypadClient

pygame.init()
 
 
def main(): 
    xAxis=0
    yAxis=0
    lastUpdateTime = time.time()
    joysticks = []
    clock = pygame.time.Clock()
    keepPlaying = True
    ble   = BLE()
    myBot = ble.findNearest()
    joypadClient = SimpleJoypadClient(myBot)
 
    # for al the connected joysticks
    for i in range(0, pygame.joystick.get_count()):
        # create an Joystick object in our list
        joysticks.append(pygame.joystick.Joystick(i))
        # initialize them all (-1 means loop forever)
        joysticks[-1].init()
        # print a statement telling what the name of the controller is
        print "Detected joystick '",joysticks[-1].get_name(),"'"
    while keepPlaying:
        if (time.time() - lastUpdateTime) > 0.05:
                joypadClient.updateJoypadWithZ(int(xAxis*255), int(yAxis*255), 0,0) 
		lastUpdateTime=time.time() 
        clock.tick(60)
        for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    print "Received event 'Quit', exiting."
                    keepPlaying = False
                elif event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE:
                    print "Escape key pressed, exiting."
                    keepPlaying = False
                elif event.type == pygame.JOYAXISMOTION:
                    #print "Joystick '",joysticks[event.joy].get_name(),"' axis",event.axis,"motion."
                    if event.axis==0:
                        xAxis=joysticks[-1].get_axis(0)
                    if event.axis==1:
                        yAxis=joysticks[-1].get_axis(1)
                elif event.type == pygame.JOYBUTTONDOWN:
                    print "Joystick '",joysticks[event.joy].get_name(),"' button",event.button,"down."
                elif event.type == pygame.JOYBUTTONUP:
                    print "Joystick '",joysticks[event.joy].get_name(),"' button",event.button,"up."
                elif event.type == pygame.JOYHATMOTION:
                    print "Joystick '",joysticks[event.joy].get_name(),"' hat",event.hat," moved."
                     
 
main()
pygame.quit()

