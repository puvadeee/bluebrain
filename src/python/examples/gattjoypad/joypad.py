#!/usr/bin/python

# the following line is not needed if pgu is installed
import sys
import os
basedir = os.path.abspath(os.path.dirname(__file__))

sys.path.insert(0, basedir+"/../../modules")

import time
import pygame
from cannybots.radio import BLE, BLE_Manager


class Joypad():
    """ RaceController """

    def __init__(self):
        self.bleInit()
        self.running = True


    def run(self):
        while self.running:
			time.sleep(1)


    def bleInit(self):
        if sys.platform == 'darwin':
            return

        self.bleManager =  BLE_Manager("hci0")
        self.ble        =  BLE(bleManager=self.bleManager)

        self.bleManager.startScanning()
        gattOpts="-j "+sys.argv[2]
        self.ble1 = self.ble.findByAddress(sys.argv[1], gattOpts)

        self.bleManager.stopScanning()

        #self.ble1.addListener(self.bleRacer1ReceviedData)


if __name__ == "__main__":
    print 'Number of arguments:', len(sys.argv), 'arguments.'
    print 'Argument List:', str(sys.argv)
    joypad = Joypad()
    joypad.run()

