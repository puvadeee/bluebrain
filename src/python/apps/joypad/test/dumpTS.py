import os
import pygame
  
eventX="/dev/input/touchscreen"
os.environ["SDL_FBDEV"] = "/dev/fb1"
os.environ["SDL_MOUSEDRV"] = "TSLIB"
os.environ["SDL_MOUSEDEV"] = eventX
x = y = 0
running = 1
screen = pygame.display.set_mode((320, 240))
 
while running:
	event = pygame.event.poll()
	if event.type == pygame.QUIT:
		running = 0
	elif event.type == pygame.MOUSEMOTION:
		print "mouse at (%d, %d)" % event.pos

	screen.fill((0, 0, 0))
	pygame.display.flip()

