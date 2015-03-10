#!/bin/bash

# This is for cconvineince of my dev...
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

mkdir -p /opt/cannybots/nfs
chown root.users /opt/cannybots/nfs
chmod g+w /opt/cannybots/nfs

echo 'slimstation.local:/volume1/rpi/cannybots /opt/cannybots/nfs nfs nouser,atime,auto,rw,dev,exec,suid,nolock,auto 0 0' >> /etc/fstab
mount -a

