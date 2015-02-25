#!/bin/bash 

#install X11 VNC
sudo apt-get install x11vnc -y
x11vnc -storepasswd
touch ~/.xsessionrc
echo "x11vnc -bg -nevershared -forever -tightfilexfer -usepw -display :0" >> ~/.xsessionrc
chmod 775 ~/.xsessionrc


# Bonjour for simpler network access
sudo apt-get install  netatalk -y
sudo cp -v rootfs/etc/avahi/services/afpd.service /etc/avahi/services/afpd.service
sudo update-rc.d avahi-daemon defaults

