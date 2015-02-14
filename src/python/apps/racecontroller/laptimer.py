#!/usr/bin/python

DEFAULT_CRUISE_SPEED = 60
SPEED_DELTA = 5
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
roboticaMediumFont= pygame.font.Font(basedir+'/fonts/Robotica.ttf', 48)
roboticaTitleFont= pygame.font.Font(basedir+'/fonts/Robotica.ttf', 70)
roboticaLargeFont = pygame.font.Font(basedir+'/fonts/Robotica.ttf', 200)


def formatTime(lapTime):
    return str("%06.3f" % lapTime)
def formatLap(lap):
    return str("%-3d  " % lap)


class PlayerTimes(gui.Table):
    def __init__(self, playerNum, **params):
        gui.Table.__init__(self, height=640,  **params) #hpadding=10, vpadding=10,
        fg = (128, 0, 0)
        labelFg = (200, 200, 200)
        self.playerNum = playerNum
        self.lapCountLabel       = gui.Label("       ", color=fg, style={'font': roboticaLargeFont})
        self.bestLapTimeLabel    = gui.Label("", color=fg, style={'font': roboticaMediumFont})
        self.currentLapTimeLabel = gui.Label("", color=fg, style={'font': roboticaMediumFont})

        # last lap time
        self.tr()
        self.td(gui.Label("Laps:", align=-1, color=labelFg, style={'font': roboticaMediumFont}))
        self.td(self.lapCountLabel, align=0)

        self.tr()
        self.td(gui.Label("Best:", align=-1, color=(0, 200,0), style={'font': roboticaMediumFont}))
        self.td(self.bestLapTimeLabel,  align=-1)
        self.tr()
        self.td(gui.Label("Last:", align=-1, color=(0,0, 200), style={'font': roboticaMediumFont}))
        self.td(self.currentLapTimeLabel,  align=-1)

        self.reset()



    def reset(self):
        self.setBestLapTime(0)
        self.setCurrentLapTime(0)
        self.setLapCount(0)
        self.timerStarted=False
        self.startTime=time.time()
        self.currentSpeed = DEFAULT_CRUISE_SPEED

    def setCurrentLapTime(self, lapTime):
        self.currentLapTime = lapTime
        self.currentLapTimeLabel.value = formatTime(lapTime)

    def setBestLapTime(self, lapTime):
        self.bestLapTime = lapTime
        self.bestLapTimeLabel.value = formatTime(lapTime)

    def setLapCount(self, lapCount):
        self.lapCount = lapCount
        self.lapCountLabel.value = formatLap(lapCount)

    def lapEvent(self):
        if self.timerStarted:
            lapTime = time.time() - self.startTime
            self.setCurrentLapTime(lapTime)
            self.setLapCount(self.lapCount+1)
            if lapTime < self.bestLapTime or self.bestLapTime == 0:
                self.setBestLapTime(lapTime)
            self.timerStarted=True
            self.startTime=time.time()
        else:
            self.timerStarted=True


class LapTimer():

    def __init__(self):
        self.player1Stats = 0
        self.player2Stats = 0
        self.mainScoreBoard = 0

        self.bleInit()
        self.lastUpdateTime = time.time()
        self.running = True
        self.display = Display(windowed=1, windowWidth=1776, windowHeight=952)
        self.screen = self.display.screen
        self.clock = pygame.time.Clock()

        self.player1Stats = PlayerTimes(1, width=self.display.screen.get_width()/2)
        self.player2Stats = PlayerTimes(2, width=self.display.screen.get_width()/2)

        self.appInit()


    def appInit(self):
        self.app = gui.App()
        c = gui.Container(align=0, valign=0)
        self.table = self.setupTable()
        c.add(self.table, 0, 0)
        self.app.init(c)

    def decreasePlayerSpeed(self, playerStats, playerBle):
        print "decreasePlayerSpeed:%d" % playerStats.playerNum
        playerStats.currentSpeed = playerStats.currentSpeed - SPEED_DELTA
        playerBle.sendBytes([0x00, 0x00, 0x00, playerStats.currentSpeed])

    def increasePlayerSpeed(self, playerStats, playerBle):
        print "increasePlayerSpeed:%d" % playerStats.playerNum
        playerStats.currentSpeed = playerStats.currentSpeed + SPEED_DELTA
        playerBle.sendBytes([0x00, 0x00, 0x00, playerStats.currentSpeed])

    def resetBoard(self, playerStats):
        playerStats.reset()


    def setupTable(self):

        fg = (0, 0, 0)

        reset1Btn = gui.Button("Reset", style={'font': roboticaSmallFont})
        reset1Btn.connect(gui.CLICK,  self.resetBoard, self.player1Stats)

        reset2Btn = gui.Button("Reset", style={'font': roboticaSmallFont})
        reset2Btn.connect(gui.CLICK,  self.resetBoard, self.player2Stats)

        table = gui.Table(width=self.screen.get_width(), height=self.screen.get_height())

        table.tr()
        table.td(gui.Image(basedir + "/images/cannybots_logo.png"), colspan=2, rowspan=1, align=0, valign=0)
        table.td(gui.Label(""))

        table.tr()
        table.td(gui.Label("Racer One", color=fg, style={'font': roboticaTitleFont}), align=0, valign=0)
        table.td(gui.Label("Racer Two", color=fg, style={'font': roboticaTitleFont}), align=0, valign=0)

        table.tr()
        table.td(self.player1Stats, valign=0)
        table.td(self.player2Stats, valign=0)

        table.tr()
        table.td(reset1Btn)
        table.td(reset2Btn)

        return table

    def handleEvents(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = 0
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    self.running = False
                elif event.key == pygame.K_z:
                    self.decreasePlayerSpeed(self.player1Stats, self.ble1)
                elif event.key == pygame.K_x:
                    self.increasePlayerSpeed(self.player1Stats, self.ble1)
                elif event.key == pygame.K_n:
                    self.decreasePlayerSpeed(self.player2Stats, self.ble2)
                elif event.key == pygame.K_m:
                    self.increasePlayerSpeed(self.player2Stats, self.ble2)
                elif event.key == pygame.K_r:
                    self.player1Stats.reset()
                    self.player2Stats.reset()
                elif event.key == pygame.K_1:
                    self.bleRacer1ReceviedData(0, "LAP_START")
                elif event.key == pygame.K_2:
                    self.bleRacer2ReceviedData(0, "LAP_START")
                elif event.key == pygame.K_t:
                    pygame.display.toggle_fullscreen()

            else:
                self.app.event(event)


    def draw(self):
        self.screen.fill((255, 255, 255))
        self.app.paint()
        pygame.display.flip()

    def run(self):
        while self.running:
            self.handleEvents()
            self.draw()
            self.clock.tick(30)
        pygame.quit()



    def bleRacer1ReceviedData(self, bleuart, message):
        print "p1: " + str (message)
        if message.startswith("LAP_START") and self.player1Stats:
            self.player1Stats.lapEvent()


    def bleRacer2ReceviedData(self, bleuart, message):
        print "p2: " + str (message)
        if message.startswith("LAP_START") and self.player2Stats:
            self.player2Stats.lapEvent()

    def bleInit(self):
        if sys.platform == 'darwin':
            return

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
    rc = LapTimer()
    rc.run()
