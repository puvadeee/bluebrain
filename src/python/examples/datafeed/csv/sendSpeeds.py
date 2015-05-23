import csv
from time import sleep

from cannybots.clients.wsclient import CannybotClient
from cannybots.clients.joypad import JoypadClient

SPEED_DURATION = 2                      # Number of seconds to hold a speed for

cannybot = CannybotClient()             # Connects to the first available Cannybot (using a local WebSocket API)
joypad   = JoypadClient(cannybot)       # Creates a Joystick helper that can create and send joystick messages


# Open a csv files that has rows of Speeds in
# This uses the first row in the CSV for the column names (see the speeds.csv)
input_file = csv.DictReader(open("speeds.csv"))


for row in input_file:

    # Read the speed from column which has the title of 'Speed' in the current row
    inputDataSpeed = float(row["Speed"])

    # scale the input speed to between 0 (stop) and 255 (full speed)
    # these values were chosen by hand after inspecting the input data
    cannybotSpeed = 50 + inputDataSpeed * 10

    print "Input Data speed: {}  => Cannybot speed: {}".format(inputDataSpeed, cannybotSpeed)

    # send the forward speed
    joypad.update(0, cannybotSpeed, 0, 0)

    # hold the speed for a number of seconds
    sleep(SPEED_DURATION)


joypad.stop()       # Stop the Cannybot


