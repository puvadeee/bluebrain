#!/bin/bash

cd vendor/bluez/
tar xvfJ bluez-5.28.tar.xz
cd bluez-5.28

patch -p0 < ../joystick.patch


export CFLAGS=`sdl-config --cflags`
export LDFLAGS="`sdl-config --libs` -lrt"


./configure \
  --prefix=/usr \
  --mandir=/usr/share/man \
  --sysconfdir=/etc \
  --localstatedir=/var \
  --libexecdir=/lib \
  --enable-experimental \
  --enable-library \
  --disable-systemd $*

make

