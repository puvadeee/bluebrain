#!/bin/bash 

cp -r  rootfs/* /
chown -R pi  /opt/cannybots/runtime

update-rc.d xboxdrv defaults
update-rc.d xboxdrv start
