#!/bin/bash 
cd ../../src/python/modules
python setup.py sdist
cp -v dist/*.tar.gz ../../../install/raspbian/vendor/cannybots/

