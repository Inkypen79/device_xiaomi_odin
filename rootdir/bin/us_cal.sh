# set -x
# $1: device for output
#     us: ultrasound

# tinyplay file.wav [-D card] [-d device] [-p period_size] [-n n_periods]
# sample usage: playback.sh spk
# rcv.wav:-4.5dbfs   spk: -4.8dbfs  ultra: -4.5dbfs  spk_hp:-1.8dbfs

case "$1" in
    "far" )
        playfilename=/vendor/etc/mi_us_sweep.wav
        recfilename=/sdcard/us_far.wav
        ;;
    "near" )
        playfilename=/vendor/etc/mi_us_sweep.wav
        recfilename=/sdcard/us_near.wav
        ;;
    "full" )
        playfilename=/vendor/etc/mi_us_whitenoise.wav
        recfilename=/sdcard/us_cal.wav
        ;;
    "far2near" )
        playfilename=/vendor/etc/mi_us_whitenoise.wav
        recfilename=/data/data/us_far2near.wav
        ;;
    "near2far" )
        playfilename=/vendor/etc/mi_us_whitenoise.wav
        recfilename=/data/data/us_near2far.wav
        ;;
    "cal_wn" )
        /vendor/bin/mi_ultrasound_test -near /data/data/us_near2far.wav -far /data/data/us_far2near.wav
        if [ -f /sdcard/cal.txt ]; then
            echo "ultrasound calibration done"
            sed -n '1p' /sdcard/cal.txt | tee /mnt/vendor/persist/audio/mi_us_cal.txt
        else
            echo "failed to create cal.txt"
        fi
        if [ -f /sdcard/mius_cal.bin ]; then
            cp /sdcard/mius_cal.bin /mnt/vendor/persist/audio/mius_cal.bin
        else
            echo "failed to create mius_cal.bin"
        fi
        exit 0
        ;;
    "cal" )
        /vendor/bin/mi_ultrasound_test
        if [ -f /sdcard/cal.txt ]; then
            echo "ultrasound calibration done"
        else
            echo "failed to create cal.txt"
        fi
        exit 0
        ;;
    *)
        echo "Usage: us-cal.sh far"
        exit 0
        ;;
esac

audio-factory-test -f enable_ultrasound_mic

# start recording
echo "start recording"
tinycap $recfilename -r 96000 -b 16 -c 1 -t 10 &

ret=$?
if [ $ret -ne 0 ]; then
    echo "tinycap done, return $ret"
fi
audio-factory-test -f enable_ultrasound

while [ ! -f $recfilename ];
do
    echo "sleep 0.5"
    sleep 0.5
done
echo "start playing"
tinyplay $playfilename

killall tinycap

audio-factory-test -f disable_ultrasound
audio-factory-test -f disable_ultrasound_mic

exit 0

