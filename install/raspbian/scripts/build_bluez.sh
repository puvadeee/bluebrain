#!/bin/bash

cd vendor/bluez/
#tar xvfJ bluez-5.28.tar.xz
#cd bluez-5.28

tar xvfJ bluez-5.30.tar.xz
cd bluez-5.30

patch -p0 < ../joystick.patch


export CFLAGS=`sdl-config --cflags`
export LDFLAGS="`sdl-config --libs` -lrt"


./configure \
  --prefix=/usr \
  --mandir=/usr/share/man \
  --sysconfdir=/etc \
  --localstatedir=/var \
  --libexecdir=/lib \
  --enable-library \
  --disable-systemd $*

#  --enable-experimental \

make

