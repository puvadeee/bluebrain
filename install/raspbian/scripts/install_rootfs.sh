#!/bin/bash 

#cp -rv  rootfs/* /
#cp -rv  ../../src/python/apps/*  /opt/cannybots/runtime/apps/
#chown -R pi  /opt/cannybots/runtime

SCRATCH_PRJ_DIR=/home/pi/Documents/Scratch\ Projects/Cannybots
mkdir "$SCRATCH_PRJ_DIR"
cp -v   ../../src/scratch/* "$SCRATCH_PRJ_DIR"/
chown -R pi.pi "$SCRATCH_PRJ_DIR"
 
#update-rc.d xboxdrv defaults
#update-rc.d xboxdrv start

#svc -du /etc/service/*
