#¡/bin/bash

INSTALL_DIR=/opt/cannybots/runtime/nodered
INSTALL_SRC_DIR=$PWD/../../src
INSTALL_BIN_FILES=$PWD/vendor/nodered


mkdir -p $INSTALL_DIR
cd $INSTALL_DIR

sudo dpkg -i $INSTALL_BIN_FILES/node_latest_armhf.deb
cp -rv $INSTALL_SRC_DIR/nodejs/cannybox  /opt/cannybots/runtime/nodered/

cd $INSTALL_DIR/cannybox
tar zxf $INSTALL_BIN_FILES/node_modules.tar.gz
cp -rv $INSTALL_SRC_DIR/www .
cp nodered/flows_default.json nodered/flows_`hostname`.json

chown pi $INSTALL_DIR
chmod g+w $INSTALL_DIR
