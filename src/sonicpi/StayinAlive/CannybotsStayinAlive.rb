# Cannybots - Stayin Alive!

use_sample_pack "/Users/wayne//projects/SonicPiDancing/samples/"

define :play_sample_portion do |samp, s ,e|
  sample samp, start: s, finish: e
end


steps = 51.5
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


