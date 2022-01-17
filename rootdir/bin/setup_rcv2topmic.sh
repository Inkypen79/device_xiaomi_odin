# tinyplay file.wav [-D card] [-d device] [-p period_size] [-n n_periods]
# sample usage: playback.sh 2000.wav  1
set -x


echo "enabling top mic"
audio-factory-test -f enable_top-mic
# start recording
tinycap /data/data/rcv2top_mic.wav -r 48000 -b 16 -T 4 && echo "capture done" &



sleep 1
echo "enabling receiver"
audio-factory-test -f enable_receiver
tinyplay /vendor/etc/rcv_seal.wav

sleep 1


echo "disabling top mic"
audio-factory-test -f disable_top-mic

echo "disabling receiver"
audio-factory-test -f disable_receiver
