require 'json'		
require 'base64'
require 'websocket-client-simple'

ws = WebSocket::Client::Simple.connect 'ws://localhost:3141/api/ws/cannybots'

ws.on :message do |msg|
  data = Base64.decode64(msg.data)
  puts "received: " + data
end

ws.on :open do
  puts "Socket opened"
end

ws.on :close do |e|
  p e
  exit 1
end

ws.on :error do |e|
  p e
end


puts "Enter some text and press return."

loop do
  text = STDIN.gets.strip
  msg = JSON.generate({:rawBytes => text})	# Cannybots JSON message 
  puts "sending: " + msg
  ws.send msg 
end

