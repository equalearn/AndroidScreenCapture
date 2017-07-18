#!/bin/bash

export LD_LIBRARY_PATH=.
#./minicap -h
#full
#./minicap -s -P 1080x1920@1080x1920/0 > 1.jpg 
#partial
#./minicap -s -P 540x960@1080x1920/0 > 1.jpg 

#get it
#./minicap -s -P 1080x1920@540x960/0 > 1.jpg 

pkill minicap

sleep 1

#if stop
if [ "$1" == "stop" ]; then
    echo stop
    exit
fi

w=$2
h=$3
rw=$(($2/4))
rh=$(($3/4))

./minicap -P "$w"x"$h"@"$rw"x"$rh"/0 &

sleep 3

fr_in=30
#fr_out=$((fr_in*2))
fr_out=$((fr_in+1))

#rtmp://120.76.76.162/live/mzya &
./minicap_adaptor $fr_in | ./ffmpeg -f image2pipe -vcodec mjpeg -framerate $fr_out -i - -preset ultrafast -tune zerolatency -y -vcodec libx264 -force_key_frames "expr:gte(t,n_forced*5)" -vf scale="$rw":"$rh",format=yuv420p -f flv $1 &

