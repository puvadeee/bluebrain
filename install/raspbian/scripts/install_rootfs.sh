#!/bin/bash 

cp -rv  rootfs/* /
cp -rv  ../../src/python/apps/*  /opt/cannybots/runtime/apps/
chown -R pi  /opt/cannybots/runtime

cp -rv  ../../src/python/scratch/agent  /opt/cannybots/runtime/scratch/

SCRATCH_PRJ_DIR=/home/pi/Documents/Scratch\ Projects/Cannybots
mkdir "$SCRATCH_PRJ_DIR"
cp -v   ../../src/scratch/PiMazing.sb "$SCRATCH_PRJ_DIR"
chown -R pi.pi "$SCRATCH_PRJ_DIR"
 
sudo update-rc.d avahi-daemon defaults
update-rc.d xboxdrv defaults
update-rc.d xboxdrv start

svc -du /etc/service/*
