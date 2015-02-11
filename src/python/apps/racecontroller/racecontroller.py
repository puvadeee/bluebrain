#!/usr/bin/python

# the following line is not needed if pgu is installed
import sys
import os
basedir = os.path.abspath(os.path.dirname(__file__))

sys.path.insert(0, basedir+"/../../modules")

import os
import time
import pygame
from cannybots.video import Display
from cannybots.radio import BLE, BLE_Manager

from pgu import gui


class Obj(gui.Widget):
    def __init__(self, **params):
        gui.Widget.__init__(self, **params)


pygame.font.init()
roboticaSmallFont = pygame.font.Font(basedir+'/fonts/Robotica.ttf', 24)
roboticaMediumFont= pygame.font.Font(basedir+'/fonts/Robotica.ttf', 36)
roboticaLargeFont = pygame.font.Font(basedir+'/fonts/Robotica.ttf', 72)


def formatTime(lapTime):
    return str("%06.3f" % lapTime)


class PlayerTimes(gui.Table):
    def __init__(self, playerNum, **params):
        gui.Table.__init__(self, **params)

        self.timerStarted=0
        self.startTime = 0
        self.playerNum = playerNum
        self.currentLapTimeLabel = gui.Label("", color=(0, 0, 0), style={'font': roboticaMediumFont})
        self.setCurrentLapTime(0.0)

        self.tr()
        self.td(self.currentLapTimeLabel, colspan=1, align=0)


    def setCurrentLapTime(self, lapTime):
        self.currentLapTime = lapTime
        self.currentLapTimeLabel.value = formatTime(lapTime)
        #print self.currentLapTimeLabel.value + "\n"

    def reset(self):
        self.setCurrentLapTime(0)
        self.timerStarted=False
        self.startTime=time.time()


class LapTimeScoreBoard(gui.Table):
    def __init__(self, **params):
        gui.Table.__init__(self, width=640, height=600, hpadding=8,**params)
        fg = (0, 0, 0)
        sty = {'font': roboticaMediumFont}
        self.placeTitle = ['1st', '2nd', '3rd', '4th', '5th']
        self.placeDriver = ['-----', '-----', '-----', '-----', '-----']
        self.placeTimes = [0.0, 0.0, 0.0, 0.0, 0.0]
        self.placeTimeLabels = []
        self.placeDriverLabels = []

        for place in range(0, len(self.placeTitle)):
            self.placeTimeLabels.append(gui.Label(formatTime(self.placeTimes[place]), color=fg, style=sty))
            self.placeDriverLabels.append(gui.Label("%s" % self.placeDriver[place], color=fg, style=sty))

        self.tr()
        self.td(gui.Label("Place"), align=0)
        self.td(gui.Label("Time"), align=0)
        self.td(gui.Label("Racer"), align=0)

        self.updateLabels()


        for place in range(0, len(self.placeTitle)):
            self.tr()
            self.td(gui.Label(self.placeTitle[place], color=fg, style=sty), align=0)
            self.td(self.placeTimeLabels[place], align=1)
            self.td(self.placeDriverLabels[place], align=1)

    def updateLabels(self):
        for place in range(0, len(self.placeTitle)):
            self.placeTimeLabels[place].value=formatTime(self.placeTimes[place])
            self.placeDriverLabels[place].value=self.placeDriver[place]

    def newTime(self, driverName, time):
        if time < 1:
            return
        for place in range(0, len(self.placeTitle)):
            if self.placeTimes[place] == 0 or time < self.placeTimes[place]:
                self.placeTimes.insert(place, time)
                self.placeDriver.insert(place, driverName)
                self.placeTimes = self.placeTimes[:5]
                self.placeDriver = self.placeDriver[:5]
                self.updateLabels()
                break

    def reset(self):
        self.placeDriver = ['----', '----', '----', '----', '----']
        self.placeTimes = [0.0, 0.0, 0.0, 0.0, 0.0]
        self.updateLabels()

class RenderWidget(gui.Widget):
    def __init__(this, width=800, height=640):
        gui.Widget.__init__(this, width=width, height=height)
        this.imageBuffer = pygame.Surface((width, height))

    def paint(this, surf):
        # Paint whatever has been captured in the buffer
        img = roboticaLargeFont.render('%03d.%03d' % (99, 99), 1, (0, 0, 0))
        this.imageBuffer.fill((255, 255, 200))
        this.imageBuffer.blit(img, (0, 0))
        surf.blit(this.imageBuffer, (0, 0))

    # Call this function to take a snapshot of whatever has been rendered
    # onto the display over this widget.
    def save_background(this):
        disp = pygame.display.get_surface()
        this.imageBuffer.blit(disp, this.get_abs_rect())


class RaceController():
    """ RaceController """

    def __init__(self):
        self.player1Stats = 0
        self.player2Stats = 0
        self.mainScoreBoard = 0

        self.bleInit()
        self.lastUpdateTime = time.time()
        self.running = True
        self.display = Display(windowed=1, windowWidth=1280, windowHeight=800)
        self.screen = self.display.screen
        self.clock = pygame.time.Clock()

        self.appInit()


    def appInit(self):
        self.app = gui.App()
        c = gui.Container(align=0, valign=0)
        self.table = self.setupTable()
        c.add(self.table, 0, 0)
        self.app.init(c)

    def decreasePlayerSpeed(self, player):
        print "decreasePlayerSpeed:%d" % player

    def increasePlayerSpeed(self, player):
        print "increasePlayerSpeed:%d" % player
        if player == 1:
            self.mainScoreBoard.newTime("One", self.player1Stats.currentLapTime, )
        else:
            self.mainScoreBoard.newTime("Two",self.player2Stats.currentLapTime)

    def resetBoard(self, value):
        self.mainScoreBoard.reset()
        self.player1Stats.reset()
        self.player2Stats.reset()


    def createIcon(self, imageFile, text, callback, value):
        e = Obj()
        e.image = pygame.image.load(imageFile)
        e.rect = pygame.Rect(0, 0, e.image.get_width(), e.image.get_height())
        e.align = 0
        btn = gui.Icon(text, style={'font': roboticaSmallFont, 'image': e.image})
        btn.connect(gui.CLICK, callback, value)
        return btn

    def createImage(self, imageFile):
        e = Obj()
        e.image = pygame.image.load(imageFile)
        e.rect = pygame.Rect(0, 0, e.image.get_width(), e.image.get_height())
        e.align = 0
        e.style = {}
        print repr(e.style)
        return e


    def setupTable(self):

        fg = (0, 0, 0)

        self.player1Stats = PlayerTimes(1)
        self.player2Stats = PlayerTimes(2)
        self.mainScoreBoard = LapTimeScoreBoard()

        p1SlowBtn = self.createIcon(basedir+"/images/button.png", "slower", self.decreasePlayerSpeed, 1)
        p1FastBtn = self.createIcon(basedir+"/images/button.png", "faster", self.increasePlayerSpeed, 1)
        p2SlowBtn = self.createIcon(basedir+"/images/button.png", "slower", self.decreasePlayerSpeed, 2)
        p2FastBtn = self.createIcon(basedir+"/images/button.png", "faster", self.increasePlayerSpeed, 2)
        resetBtn = self.createIcon(basedir+"/images/button.png", "reset", self.resetBoard, 1)


        # Create the Table cells

        table = gui.Table(width=self.screen.get_width(), height=self.screen.get_height())

        table.tr()
        table.td(gui.Label(""))
        table.td(gui.Label(""))
        table.td(gui.Image(basedir + "/images/cannybots_logo.png"), colspan=1, rowspan=1, align=0, valign=0)
        table.td(gui.Label(""))
        table.td(gui.Label(""))

        table.tr()
        table.td(gui.Label("Racer One", color=fg, style={'font': roboticaMediumFont}), colspan=2, rowspan=1, align=1,
                 valign=-1)
        table.td(self.mainScoreBoard, colspan=1, rowspan=2)
        table.td(gui.Label("Racer Two", color=fg, style={'font': roboticaMediumFont}), colspan=2, rowspan=1, align=-1,
                 valign=-1)

        table.tr()
        table.td(self.player1Stats, colspan=2, valign=-1)
        table.td(gui.Label(""))
        table.td(self.player2Stats, colspan=2, valign=-1)

        table.tr()
        table.td(p1SlowBtn, rowspan=1, align=-1)
        table.td(p1FastBtn, rowspan=1, align=1)
        table.td(resetBtn)
        table.td(p2SlowBtn, rowspan=1, align=-1)
        table.td(p2FastBtn, rowspan=1, align=1)

        table.tr()

        return table

    def handleEvents(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = 0
            elif event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE:
                self.running = False
            else:
                self.app.event(event)


    def draw(self):
        self.screen.fill((255, 255, 255))

        # self.screen.set_clip(self.mainScoreBoard.get_abs_rect())
        # self.screen.set_clip()

        self.app.paint()
        pygame.display.flip()

    def update(self):
        pass
        #if self.player1Stats.timerStarted:
        #    self.player1Stats.setCurrentLapTime(time.time() - self.player1Stats.startTime)
        #if self.player2Stats.timerStarted:
        #    self.player2Stats.setCurrentLapTime(time.time() - self.player2Stats.startTime)

    def run(self):
        while self.running:
            self.handleEvents()
            self.update()
            self.draw()
            self.clock.tick(30)
        pygame.quit()

    def bleRacer1ReceviedData(self, bleuart, message):
        print "p1: " + str (message)
        if not self.player1Stats:
            return
        if message.startswith("LAP_START"):
            print "Player1 Lap!"
            if self.player1Stats.timerStarted:
                self.player1Stats.setCurrentLapTime(time.time() - self.player1Stats.startTime)
                self.mainScoreBoard.newTime("One", self.player1Stats.currentLapTime)
            self.player1Stats.currentLapTime = 0
            self.player1Stats.timerStarted=True
            self.player1Stats.startTime=time.time()

    def bleRacer2ReceviedData(self, bleuart, message):
        print "p2: " + str (message)
        if not self.player2Stats:
            return
        if message.startswith("LAP_START"):
            print "Player2 Lap Start!"
            if self.player2Stats.timerStarted:
                self.player2Stats.setCurrentLapTime(time.time() - self.player2Stats.startTime)
                self.mainScoreBoard.newTime("Two", self.player2Stats.currentLapTime)
            self.player2Stats.currentLapTime = 0
            self.player2Stats.timerStarted=True
            self.player2Stats.startTime=time.time()

    def bleInit(self):
        self.bleManager =  BLE_Manager("hci0")
        self.ble        =  BLE(bleManager=self.bleManager)

        self.bleManager.startScanning()

        self.ble1 = self.ble.findByName(sys.argv[1])
        self.ble2 = self.ble.findByName(sys.argv[2])

        self.bleManager.stopScanning()

        self.ble1.addListener(self.bleRacer1ReceviedData)
        self.ble2.addListener(self.bleRacer2ReceviedData)


if __name__ == "__main__":
    print 'Number of arguments:', len(sys.argv), 'arguments.'
    print 'Argument List:', str(sys.argv)
    rc = RaceController()
    rc.run()
