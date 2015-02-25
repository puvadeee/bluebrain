#!/bin/bash

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

apt-get update
apt-get upgrade

# python deps

apt-get install python-pexpect

# BlueZ building and runtime
apt-get install automake build-essential libtool glib2.0 libdbus-1-dev libudev-dev libical-dev libreadline-dev

# SDL
apt-get install libsdl-dev -y


# XBox controller

apt-get install xboxdrv jstest-gtk -y


# deamon tools

apt-get install daemontools daemontools-run


# mjpgstreamer for Camera

apt-get install libv4l-dev libjpeg8-dev imagemagick cmake

