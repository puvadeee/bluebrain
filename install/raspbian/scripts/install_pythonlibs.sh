#!/bin/bash 
cd vendor/cannybots
INSTALL_SRC_DIR=$PWD/../../src/python/modules
cd $INSTALL_SRC_DIR
#rm -fr /usr/local/lib/python2.7/dist-packages/cannybots/
./install.sh
