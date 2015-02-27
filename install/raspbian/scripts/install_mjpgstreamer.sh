#!/bin/bash

cd vendor/mjpgstreamer/mjpg-streamer-master/mjpg-streamer-experimental/
make all
cp -rv *.so mjpg_streamer www /opt/cannybots/runtime/camera/


