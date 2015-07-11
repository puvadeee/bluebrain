#!/bin/bash 
cd vendor/cannybots
tar zxvf Cannybots-*.tar.gz
cd Cannybots-*
rm -fr /usr/local/lib/python2.7/dist-packages/cannybots/
sudo python setup.py install

