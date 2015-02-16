#!/usr/bin/python
#
# Cannybots BLE Python Library
#
# Authors:  Wayne Keenan
#
# License:  http://opensource.org/licenses/MIT
#
# Version:   1.0  -  11.10.2014  -  Inital Version  (wayne@cannybots.com)
#

import sys
import os
import time
import array
import Queue
from threading import Thread, Lock
import logging
import atexit

import pexpect


# runs in background:  sudo hcitool lescan --duplicates --passive
#outputs:     		 FF:C3:F0:EC:C3:D9 CB_1f73956a95eb

#for each address:
# gatttool -t random --primary -I -b  FF:C3:F0:EC:C3:D9
#attr handle = 0x0001, end grp handle = 0x0007 uuid: 00001800-0000-1000-8000-00805f9b34fb
#attr handle = 0x0008, end grp handle = 0x000b uuid: 00001801-0000-1000-8000-00805f9b34fb
#attr handle = 0x000c, end grp handle = 0xffff uuid: 7e400001-b5a3-f393-e0a9-e50e24dcca9e



# RX:
#[CON][FF:C3:F0:EC:C3:D9][LE]> char-write-req 0x000f 0100
#[CON][FF:C3:F0:EC:C3:D9][LE]> Characteristic value was written successfully

#Notification handle = 0x000e value: 68 65 6c 6c 6f 
#[CON][FF:C3:F0:EC:C3:D9][LE]> 
#Notification handle = 0x000e value: 68 65 6c 6c 6f 


MAX_TX_QUEUE_SIZE = 3

CB_DEFAULT_SERVICEUUID = '7e400001-b5a3-f393-e0a9-e50e24dcca9e'
CB_RX_UUID = ''
CB_TX_UUID = ''


def die(child, errstr):
    print 'ERROR: ' + errstr
    print child.before, child.after
    child.terminate()
    sys.exit(errstr)


class BLE_Manager:

    def __init__(self, mac, hci='hci0'):
        self.hciDevice = hci
        self.hcitool = False

    def startHCITool(self):
        print "HCI TOOL STARTING"
        cmd = 'hcitool  lescan --duplicates --passive'
        if self.hcitool:
            self.hcitool.close()

        self.hcitool = pexpect.spawn(cmd)
        i = self.hcitool.expect([pexpect.TIMEOUT, 'LE Scan ...'])
        self.devices = {}
        if i == 0:
            die(self.hciTool, 'hci timed out. Here is what hcitool said:')

    def hci_worker(self):
        while self.keepRunning:
            try:
                if self.hcitool == 0:
                    print "HCITOOL not running skipping"
                    time.sleep(5)
                    continue
                i = self.hcitool.expect([pexpect.TIMEOUT, pexpect.EOF, "(.*)\n"])
                if i == 0:
                    print "hcitool: no devices (timeout)"
                elif i ==1 :
                    #print "HCI TOOL EOF - retry in  3..2..1.."
                    time.sleep(5)
                    continue
                else:
                    # print "hci:" + self.hcitool.after
                    timeNow = int(time.time())
                    items = self.hcitool.after.split(' ')
                    if len(items) == 2:
                        if items[0] in self.devices:
                            self.devices[items[0]]['lastseen'] = timeNow
                        else:
                            self.devices[items[0]] = {'name': items[1].strip(), 'rssi': '0', 'lastseen': timeNow}

                    # purge old entries
                    oldestTime = timeNow - 2
                    for mac in self.devices.keys():
                        if self.devices[mac]['lastseen'] < oldestTime:
                            self.devices.pop(mac, None)

                        #print self.devices
                        #time.sleep(1)
            except Exception as e:
                logging.exception('hci_worker')
                self.startHCITool()



    def startScanning(self):
        #self.stopScanning()
        self.keepRunning = True
        self.startHCITool()
        self.hcitoolThread = Thread(target=self.hci_worker)
        self.hcitoolThread.daemon = True
        self.hcitoolThread.name = 'BLE_Manager_hciworker'
        self.hcitoolThread.start()

    def stopScanning(self):
        self.keepRunning = False
        if self.hcitool:
            time.sleep(1)
            self.hcitool.close()
            print "HCI TOOL EXIT: " + str(self.hcitool.exitstatus)
            self.hcitool = 0



class BLE_UART:
    # Background threads:   gatttool lister for TX sending and RX notifications

    def startGattTool(self):
        # os.system("killall gatttool")
        cmd = 'gatttool -b ' + self.mac + '  -t random -I ' + self.gattOpts
        self.child = pexpect.spawn(cmd)
        atexit.register(self.closeGattTool)

        i = self.child.expect([pexpect.TIMEOUT, '\[LE\]>'], timeout=10)
        if i == 0:
            die(self.child, 'gatttool timed out. Here is what gatttool said:')

        #self.enqueString('connect')
        # Start the RX listener
        #time.sleep(1)
        #self.enqueString('char-write-req 0x000f 0100')

    def closeGattTool(self):
        print "Killing gatttool child"
        if self.child:
            self.keepRunning = False
            self.child.close()
            #self.child.terminate()
            print "GATTTOOL TOOL EXIT: " + str(self.child.exitstatus)

    def tx_worker(self):
        while self.keepRunning:
            item = self.q.get(True)  # get command or if none are waiting this will block
            self._send(item)
            self.q.task_done()

    def rx_worker(self):
        while self.keepRunning:
            #self.queueLock.acquire()
            i = self.child.expect([pexpect.TIMEOUT, "Notification handle = 0x000e value:\s(.*)\n"])

            if i>0 and self.rxListener:
                #print "BLE_UART recv: " + str(self.child.after)
                try:
                    self.rxListener(self, self.child.after.split("value: ", 1)[1].rstrip().replace(" ", "").decode("hex"))
                except TypeError as e:
                    print "Failed to decode notification, " + str(e)
            #self.queueLock.release()


    def __init__(self, mac, name='BLE_UART', gattOpts=""):
        self.mac = mac
        self.keepRunning = True
        self.onDisconnectDelegate = False
        self.gattOpts = gattOpts

        self.queueLock = Lock()
        self.q = Queue.Queue(maxsize=MAX_TX_QUEUE_SIZE)

        self.rxListener = False
        self.startGattTool()

        self.tx = Thread(target=self.tx_worker)
        self.tx.daemon = True
        self.tx.name = name
        #self.tx.start()

        self.rx = Thread(target=self.rx_worker)
        self.rx.daemon = True
        self.rx.name = name

        self.rx.start()


    def addListener(self, func):
        self.rxListener = func

    def sendBytes(self, bytes):
        byteStr = ''.join(chr(x) for x in bytes)
        self.sendHexString(byteStr)


    def sendHexString(self, hexString):
        cmd = 'char-write-cmd 0x0011 ' + hexString
        self.enqueString(cmd)

    def enqueString(self, str):
        #if self.queueLock.locked():
        #    return
        #self.queueLock.acquire()
        self.q.put(str)
        #self.queueLock.release()


    def _send(self, msg):
        self.child.sendline(msg)
        i = self.child.expect([pexpect.TIMEOUT, 'command failed', '\[LE\]>'])
        if i == 0:
            print "gatt timeout"
            #die(self.child, 'gatttool timed out. detail:')
        elif i == 1:
            print "Wake up, time to die"
            #self.keepRunning = False
            #die(self.child, 'command failed, detail:')
        #disconnection error:
        #ERROR: command failed, detail:
        # char-write-cmd 0x0011 89810080
        #Command failed:


    def onDisconnect(self, delegate):
        self.onDisconnectDelegate = delegate


class BLE:
    def __init__(self, hci="hci0", name='BLEThread', bleManager=False):

        if not bleManager:
            self.bleManager = BLE_Manager(hci)
            self.bleManager.startScanning()
        else:
            self.bleManager = bleManager


    def findNearest(self, gattOpts=""):
        deviceAddress = ''
        while not deviceAddress:
            devices = self.devicesInRange()
            for mac in devices.keys():
                deviceAddress = mac
            time.sleep(1)

        print  "CONNECT TO: " + deviceAddress
        #self.bleManager.stopScanning()

        bleDevice = BLE_UART(mac=deviceAddress, gattOpts=gattOpts)
        return bleDevice


    def findByName(self, name, gattOpts=""):
        deviceAddress = ''
        while not deviceAddress:
            devices = self.devicesInRange()
            for mac in devices.keys():
                print mac + " " + str(devices[mac])
                try:
                    if devices[mac]['name'] == name:
                        deviceAddress = mac
                except Exception:
                    logging.exception('findByName')
            time.sleep(1)

        print  "CONNECT TO: " + deviceAddress
        #self.bleManager.stopScanning()

        bleDevice = BLE_UART(mac=deviceAddress, gattOpts=gattOpts)
        return bleDevice


    def devicesInRange(self):
        return self.bleManager.devices

    def stopScanning(self):
        self.bleManager.stopScanning()
		




 
