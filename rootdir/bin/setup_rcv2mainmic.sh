# tinyplay file.wav [-D card] [-d device] [-p period_size] [-n n_periods]
# sample usage: playback.sh 2000.wav  1
set -x

sleep 1

echo "enabling main mic"
audio-factory-test -f enable_main-mic

# start recording
tinycap /data/data/rcv2main_mic.wav -r 48000 -b 16 -T 4 && echo "capture done" &



sleep 1
echo "enabling top-spk"
audio-factory-test -f enable_rcv-spk
tinyplay /vendor/etc/top_spk_seal.wav
sleep 1


echo "disabling main mic"
audio-factory-test -f disable_main-mic

echo "disabling receiver"
audio-factory-test -f disable_rcv-spk

