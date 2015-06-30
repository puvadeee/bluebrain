# Cannybots - Stayin Alive!

# Dancing Cannybots

# Websocket Client used to send messages to the the Cannybots WebSocket API
require 'json'
require 'websocket-client-simple'
ws = WebSocket::Client::Simple.connect 'ws://localhost:3141/api/ws/cannybots'


beatCount = 0
steps = 51.5   # tweak this to match  the funky beat, BeeGee's have a good drummer :)


dancePattern = Array[
{'0' => '<', '1' => '<', '2' => '', '3' => '' },
{'0' => '>', '1' => '>', '2' => '', '3' => '' },
{'0' => 'f', '1' => 'f', '2' => '', '3' => '' },
{'0' => 'b', '1' => 'b', '2' => '', '3' => '' },
{'0' => '>', '1' => '>', '2' => '', '3' => '' },
{'0' => '<', '1' => '<', '2' => '', '3' => '' },
{'0' => 'f', '1' => 'f', '2' => '', '3' => '' },
{'0' => 'b', '1' => 'b', '2' => '', '3' => '' },
]

define :perform_dance_step do
  movement = beatCount % dancePattern.length
  
  danceSteps = dancePattern[movement]
  
  danceSteps.each do |bot, cmd|
    if cmd != ''
      ws.send JSON.generate({:botId => bot, :rawBytes => cmd})
    end
  end

end

use_sample_pack "/home/pi/cannybots/bluebrain/src/sonicpi/StayinAlive/samples"
samp=:StayinAlive

sampleLen = sample_duration samp
sampleStepDelay  = sampleLen/steps
sampleStepDelta = 1.0/steps

(steps-1).times do
  samplePos = sampleStepDelta * beatCount

  perform_dance_step()

  sample samp, start: samplePos, finish: samplePos + sampleStepDelta
  sleep sampleStepDelay

  beatCount = beatCount+1
end
