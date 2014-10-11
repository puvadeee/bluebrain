#!/usr/bin/python

import os
import pygame
import time
import random
import pexpect

btaddr='FF:C3:F0:EC:C3:D9'


 
class joypad:
    screen = None;
    isX = 0;

    def __init__(self):
        "Ininitializes a new pygame screen using the framebuffer"
        # Based on "Python GUI in Linux frame buffer"
        # http://www.karoltomala.com/blog/?p=679
        disp_no = os.getenv("DISPLAY")
        if disp_no:
            print "I'm running under X display = {0}".format(disp_no)
            self.isX=1
        
        # Check which frame buffer drivers are available
        # Start with fbcon since directfb hangs with composite output
        drivers = ['fbcon', 'directfb', 'svgalib', 'x11']
        found = False
        for driver in drivers:
            # Make sure that SDL_VIDEODRIVER is set
            if not os.getenv('SDL_VIDEODRIVER'):
                os.putenv('SDL_VIDEODRIVER', driver)
            try:
                pygame.display.init()
            except pygame.error:
                print 'Driver: {0} failed.'.format(driver)
                continue
            found = True
            break
    
        if not found:
            raise Exception('No suitable video driver found!')
        
        size = (pygame.display.Info().current_w, pygame.display.Info().current_h)
        print "Framebuffer size: %d x %d" % (size[0], size[1])
        opts = 0
        if self.isX:
            opts = pygame.RESIZABLE
            size=(320,240)
        else:
            opts = pygame.FULLSCREEN 
        self.screen = pygame.display.set_mode(size, opts)
        # Clear the screen to start
        self.screen.fill((0, 0, 0))        
        # Initialise font support
        pygame.font.init()
        # Render the screen
        pygame.display.update()
 
    def __del__(self):
        "Destructor to make sure pygame shuts down, etc."
 
    def test(self):
        red = (255, 0, 0)
	white=(255,255,255)
        self.screen.fill(white)
        pygame.display.update()
 
def arduino_map(x, in_min, in_max, out_min, out_max):
        return (x - in_min) * (out_max - out_min) // (in_max - in_min) + out_min

def die(child, errstr):
    print 'ERROR: '+errstr
    print child.before, child.after
    child.terminate()
    exit(1)

cmd='/usr/bin/gatttool -b ' + btaddr+'  -t random -I'
child = pexpect.spawn(cmd)
i = child.expect ([pexpect.TIMEOUT, '\[LE\]>'])
if i == 0:
        die(child, 'gatttool timed out. Here is what gatttool said:')

def send_cmd(child, cmd):
	child.sendline (cmd)
	i = child.expect ([pexpect.TIMEOUT,'Command failed:','\[LE\]>'])
	#print child.before
	if i == 0:
        	die(child, 'gatttool timed out. detail:')
	elif i == 1:
		die(child, 'command failed, detail:')

send_cmd(child, 'connect')

from time import time

lastSendTime =time()

def send(x,y,b):
	global lastSendTime
	if ( (time() - lastSendTime) > 0.1):
		x = arduino_map(x-120, -120,120, 0,255)
		y = arduino_map(y-120, -120,120, 0,255)
		msg = format(x,'02X') + format(y,'02X') + format(b, '02X') 
		#print msg
		send_cmd(child, 'char-write-cmd 0x0011 ' + msg)
		lastSendTime=time()

pad = joypad()
pad.test()

logo = pygame.image.load("images/cannybots_logo_small.png").convert_alpha()
joystick = pygame.image.load("images/joystick.png").convert()
knob = pygame.image.load("images/knob.png").convert_alpha()
button= pygame.image.load("images/button.png").convert_alpha()


#pygame.mouse.set_visible(0)

running=1

(x,y) = (120,120)

while running:
	pad.screen.fill((255,255,255))
	pad.screen.blit(joystick,(0,0))
	pad.screen.blit(button,(320-80,64))
	pad.screen.blit(logo, (320-120, 240-32))
	event = pygame.event.poll()
	if event.type == pygame.QUIT:
		running = 0
        elif event.type == pygame.MOUSEMOTION:
		(x,y)=event.pos
		if x>240:
			x=240 
                	#print "joy at (%d, %d)" % (x,y)
	elif event.type == pygame.MOUSEBUTTONUP:
		(x,y)=(120,120)
	(jx,jy) = (x-50, y-50)
	send(x,y,0)	
	pad.screen.blit(knob, (jx,jy))
	pygame.display.update()

        #pygame.display.flip()

