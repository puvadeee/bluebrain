#!/bin/bash
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi
sed -i /etc/inittab -e "s|^.*:.*:respawn:.*ttyAMA0|#&|"
sed -i /boot/cmdline.txt -e "s/console=ttyAMA0,[0-9]\+ //"

