# Cannybots - Stayin Alive!

# Move a Cannybot forward to the beat!


# Websocket Client used to send messages to the the Cannybows WebSocket API
# TODO: move this to a Cannybots Ruby GEM
require 'json'
require 'websocket-client-simple'
ws = WebSocket::Client::Simple.connect 'ws://localhost:3141/api/ws/cannybots'


beatCount = 0

steps = 51.5   # tweak this to match the funky beat, BeeGee's have a good drummer :)

define :move_bot do |bot, cmd|
  ws.send JSON.generate({:botId => bot, :rawBytes => cmd})
end

dancePattern = Array[
{'0' => '<', '1' => '', '2' => '', '3' => '' },
{'0' => '>', '1' => '', '2' => '', '3' => '' },
{'0' => 'f', '1' => '', '2' => '', '3' => '' },
{'0' => 'b', '1' => '', '2' => '', '3' => '' },
{'0' => '>', '1' => '', '2' => '', '3' => '' },
{'0' => '<', '1' => '', '2' => '', '3' => '' },
{'0' => 'f', '1' => '', '2' => '', '3' => '' },
{'0' => 'b', '1' => '', '2' => '', '3' => '' },
]

define :perform_dance_step do
  movement = beatCount % dancePattern.length
  
  danceSteps = dancePattern[movement]
  
  danceSteps.each do |bot, cmd|
    if cmd != ''
      puts bot
      move_bot(bot, cmd)
    end
  end

end

define :play_sample_portion do |samp, s ,e|
  sample samp, start: s, finish: e
end


  use_sample_pack "/home/pi/cannybots/bluebrain/src/sonicpi/StayinAlive/samples"
  samp=:StayinAlive

  sampleLen = sample_duration samp
  sampleStepDelay  = sampleLen/steps
  stepSize = 1.0/steps

  pos = 0.0
  (steps-1).times do
    print pos
    perform_dance_step()
    play_sample_portion(samp, pos, pos+stepSize)
    sleep sampleStepDelay
    pos = pos + stepSize
    beatCount = beatCount+1
  end
