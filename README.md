#CannyBots BlueBrain 


#Installation

##1. Versions

These version /must/ be used:

* Arduino IDE:  v1.5.7
* RFduino SDK:  v2.1.2


##2. Arduino IDE RFduino Support

a) Install RFduino v2.1.2 Library for Arduino

Download and install the v2.1.2 of the RFduino library:  

* http://www.rfdigital.com/wp-content/uploads/2014/03/RFduino_2.1.2.zip

It *MUST* be  v2.1.2 because of some customisations we've made. 
You also must be using Arduino IDE 1.5.7 or higher for the RFduino library to work.

Unzip the RFduino zip under:  
mac: [ArduinoIDE_Install_PATH]/Java/hardware/arduino/
win:	[ArduinoIDE_Install_PATH]/hardware/arduino/


b) RFduino SDK patch.

Download this patch to the RFduino SDK:

* https://github.com/Cannybots/cannybots-beta/raw/master/avr/patches/RFduino/RFduino_2.1.2_patch.zip

The folders in this zip need to replace sub-folders under the RFduino folder that was installed into the Arduino IDE in the previous step.

* RFduinoGZLL			under:  	[ArduinoIDE_Install_PATH]/Java/hardware/arduino/RFduino/libraries
* libRFduinoGZLL		under:	[ArduinoIDE_Install_PATH]/Java/hardware/arduino/RFduino/source

It's preferable to move, or just delete, the existing folders rather than renaming them in-place:

* [ArduinoIDE_Install_PATH]/Java/hardware/arduino/RFduino/libraries/RFduinoGZLL
* [ArduinoIDE_Install_PATH]/Java/hardware/arduino/RFduino/libraries/libRFduinoGZLL

Without the patch you can not have more than one bot in the room when using the physical joypad (address clashes in non-BLE mode)


##3. iOS App

In Safari on on your iOS device goto http://cannybots.github.io/cannybots-beta/

You will have needed to provide us with your Device ID for this to install and run.
