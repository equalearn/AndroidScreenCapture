#!/bin/bash 

pkill screencap
sleep 1
#if stop
if [ "$1" == "stop" ]; then
    echo stop
    exit
fi

w=1920
h=1080
rw=$(($w/4))
rh=$(($h/4))

while true
do
    ./screencap -r 10000 2>log | ./ffmpeg -f rawvideo -vcodec rawvideo -pix_fmt rgba -s "$w"x"$h" -framerate 5.5 -i - -preset ultrafast -tune zerolatency -y -vf scale="$rw":"$rh",format=yuv420p -vcodec libx264 -force_key_frames "expr:gte(t,n_forced*5)" -f flv $1
done
