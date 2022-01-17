# tinyplay file.wav [-D card] [-d device] [-p period_size] [-n n_periods]
# sample usage: playback.sh 2000.wav  1
set -x

sleep 1

echo "enabling back mic"
audio-factory-test -f enable_back-mic
# start recording
tinymix "TX_DEC2 Volume" 0
tinycap /data/data/rcv2back_mic.wav -r 48000 -b 16 -T 4 && echo "capture done" &
sleep 0.3
tinymix "TX_DEC2 Volume" 84

sleep 0.7
echo "enabling top-spk"
audio-factory-test -f enable_top-spk
tinyplay /vendor/etc/rcv_seal.wav

sleep 1


echo "disabling back mic"
audio-factory-test -f disable_back-mic

echo "disabling receiver"
audio-factory-test -f disable_top-spk
