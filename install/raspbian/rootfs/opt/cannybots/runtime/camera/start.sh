#!/bin/sh

export LD_LIBRARY_PATH="$(pwd)"
./mjpg_streamer -i "./input_raspicam.so -fps 25 -q 50 -x 640 -y 480" -o "./output_http.so -p 9000 -w ./www"



