#!/bin/bash 

cp -rv  rootfs/* /
cp -rv  ../../src/python/apps/*  /opt/cannybots/runtime/apps/
chown -R pi  /opt/cannybots/runtime

sudo update-rc.d avahi-daemon defaults
update-rc.d xboxdrv defaults
update-rc.d xboxdrv start

svc -du /etc/service/*
