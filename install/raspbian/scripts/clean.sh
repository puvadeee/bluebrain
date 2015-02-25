find . -name '@eaDir' -exec rm -fr '{}' ';'
find . -name '.DS_Store' -exec rm -fr '{}' ';'
rm -fr vendor/pgu/pgu-0.18
rm -fr vendor/bluez/bluez-5.28
rm -fr vendor/cannybots/Cannybots-0.1dev
rm -fr vendor/mjpgstreamer/mjpg-streamer-master

