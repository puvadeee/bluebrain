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
win: [ArduinoIDE_Install_PATH]/hardware/arduino/


b) RFduino SDK patch.

Without this patch you can not have more than one bot in the room when using the physical joypad because of an address clash in non-BLE mode
You can perform this patch to an existing installation or download a pre-patched version [here](https://www.dropbox.com/s/yilauqvzbv1arfp/RFduino_2.1.2.zip?dl=0)
Download this patch to the RFduino SDK:

* https://github.com/Cannybots/cannybots-beta/raw/master/avr/patches/RFduino/RFduino_2.1.2_patch.zip

The folders in the zip need to replace sub-folders under the RFduino folder that was installed into the Arduino IDE in the previous step.
It's preferable to move, or just delete, the existing folders rather than renaming them in-place:

* .../RFduino/libraries/RFduinoGZLL
* .../RFduino/source/libRFduinoGZLL

copy these files from the 'libRFduinoGZLL' folder:

libRFduinoGZLL.h
libRFduinoGZLL.a

to: .../hardware/arduino/RFduino/variants/RFduino/

overwriting whats there already.




##4. iOS and Android Apps

The apps are available in source form in the repo and also on the respective app stores.

Apple users can find the app [here](https://itunes.apple.com/us/app/cannybots/id932910715?mt=8)

Android users can find the app [here](https://play.google.com/store/apps/details?id=com.cannybots.cannybots)


##5. Using the programmer

Install the latest drivers form the www.ftdichip.com website.




