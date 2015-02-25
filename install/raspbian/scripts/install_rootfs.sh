#!/bin/bash 

cp -r  rootfs/* /
cp -r  ../../src/python/apps/*  /opt/cannybots/runtime/apps/
chown -R pi  /opt/cannybots/runtime

sudo update-rc.d avahi-daemon defaults
update-rc.d xboxdrv defaults
update-rc.d xboxdrv start
