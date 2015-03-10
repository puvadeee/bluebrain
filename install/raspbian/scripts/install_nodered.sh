#¡/bin/bash

INSTALL_DIR=/opt/cannybots/runtime/nodered
INSTALL_SRC_DIR=$PWD/../../src
INSTALL_BIN_FILES=$PWD/vendor/nodered

if [ "$(grep -c ^processor /proc/cpuinfo)" = "1" ]; then

echo Pi v1 install
sudo dpkg -i $INSTALL_BIN_FILES/node_0.10.36_armhf.deb

else
echo Pi v2 install
#from:  curl -sL https://deb.nodesource.com/setup | sudo bash -
./scripts/setup_nodered_pi2.sh 
apt-get install nodejs
sudo apt-get install -y build-essential python-dev python-rpi.gpio nodejs
fi

mkdir -p $INSTALL_DIR
cd $INSTALL_DIR

cp -rv $INSTALL_SRC_DIR/nodejs/cannybox  /opt/cannybots/runtime/nodered/

cd $INSTALL_DIR/cannybox
#tar zxf $INSTALL_BIN_FILES/node_modules.tar.gz
cp -rv $INSTALL_SRC_DIR/www .
cp nodered/flows_default.json nodered/flows_`hostname`.json

chown pi $INSTALL_DIR
chmod g+w $INSTALL_DIR
npm install
