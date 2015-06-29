import csv
from time import sleep

from cannybots.clients.wsclient import CannybotClient
from cannybots.clients.joypad import JoypadClient

SPEED_DURATION = 2                      # Number of seconds to hold a speed for

cannybot1 = CannybotClient(botId='2')             # Connects to the default Cannybot configured in NodeRED (using a local WebSocket API)
cannybot2 = CannybotClient(botId='3')

sleep(2)

rider1    = JoypadClient(cannybot1)       # Creates a Joystick helper that can create and send joystick messages
rider2    = JoypadClient(cannybot2)

# Open a csv files that has rows of Speeds in
# This uses the first row in the CSV for the column names (see the speeds.csv)
input_file = csv.DictReader(open("Cycling Data for Cannybots.xlsx - Cyclists.csv"))


for row in input_file:

    # Read the speed from column which has the title of 'Speed' in the current row
    inputDataRider1Speed = float(row["Rider One Speed (km/Hr)"])
    inputDataRider2Speed = float(row["Rider Two Speed (km/Hr)"])

    # scale the input speed to between 0 (stop) and 255 (full speed)
    # these values were chosen by hand after inspecting the input data
    rider1Speed = 255 - (50 + inputDataRider1Speed * 8)
    rider2Speed = 255 - (50 + inputDataRider2Speed * 8)

    print "Input Data speed: {}  => Rider 1 speed: {}".format(inputDataRider1Speed, rider1Speed)
    print "Input Data speed: {}  => Rider 2 speed: {}".format(inputDataRider2Speed, rider2Speed)

    # send the forward speed
    rider1.update(0, rider1Speed, 0, 0)
    rider2.update(0, rider2Speed, 0, 0)

    # hold the speed for a number of seconds
    sleep(SPEED_DURATION)


rider1.stop()       # Stop the Cannybot
rider2.stop()

