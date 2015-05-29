# Cannybots - Stayin Alive!

# Move a Cannybot forward to the beat!


# Websocket Client used to send messages to the the Cannybows WebSocket API
# TODO: move this to a Cannybots Ruby GEM
require 'json'      
require 'websocket-client-simple'
ws = WebSocket::Client::Simple.connect 'ws://localhost:3141/api/ws/cannybots'



steps = 51.5   # tweak this to match the funky beat, BeeGee's have a good drummer :)

define :play_sample_portion do |samp, s ,e|
  sample samp, start: s, finish: e
  # Send the movement commands... pretty basic for now...
  # this is gonna get way more interesting using loopkup tables and a counter 
  # for multiple Cannybot dancers more robo moves on the dancefloor :)
  ws.send JSON.generate({:botId => '0', :rawBytes => "f"})   
end


use_sample_pack "/home/pi/cannybots/bluebrain/src/sonicpi/StayinAlive/samples"
samp=:StayinAlive

sampleLen = sample_duration samp
sampleStepDelay  = sampleLen/steps
stepSize = 1.0/steps

pos = 0.0
(steps-1).times do
  print pos
  play_sample_portion(samp, pos, pos+stepSize)
  sleep sampleStepDelay
  pos = pos + stepSize
end


