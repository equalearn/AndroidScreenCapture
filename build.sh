#!/bin/bash

while getopts 'n:' OPT; do
    case $OPT in
        n)
            ndk_path="$OPTARG";;
        ?)
            echo "Usage: `basename $0`"
    esac
done

shift $((OPTIND-1))

if [ -z "$ndk_path" ]; then
    echo haha
    exit
fi

#echo argument number $#
api_lv=23
#abi=armeabi-v7a
abi=arm64-v8a
#echo $ndk_path
#exit

export NDK_PROJECT_PATH=minicap
#ndk-build clean
#todo specify abi
ndk-build

if [ "$?" != 0 ]; then
    echo build failed
    exit
fi

mkdir -p bin
rm bin/*
cp minicap/jni/minicap-shared/aosp/libs/android-$api_lv/$abi/minicap.so bin
cp minicap/libs/$abi/minicap bin

#################
#todo
cp minicap_adaptor/cmake-build-debug-android/minicap_adaptor bin

###############
#todo
cp prebuilt-binaries/armeabi-v7a/ffmpeg bin

cp prebuilt-binaries/cap1.sh bin

