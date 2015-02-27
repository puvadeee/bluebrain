#!/bin/bash

# Make sure only root can run our script
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi
cd vendor/bluez/bluez-5.28
make install && ln -svf ../libexec/bluetooth/bluetoothd /usr/sbin
install -v -dm755 /etc/bluetooth && install -v -m644 src/main.conf /etc/bluetooth/main.conf
install -v -dm755 /usr/share/doc/bluez-5.28 && install -v -m644 doc/*.txt /usr/share/doc/bluez-5.28
install -c attrib/gatttool /usr/local/bin/gatttool
#cp -v attrib/gatttool /usr/local/bin/
