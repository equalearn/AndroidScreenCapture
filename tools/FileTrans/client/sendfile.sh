#! /bin/bash

#echo $1
if [ "$#" -ne 3 ]; then
    echo "Usage: ./sendfile file srvaddr srvport "
    exit 1
fi

fullname=$1
srvaddr=$2
srvport=$3
filename=`basename $fullname`
#echo $filename
md5=`md5sum $fullname | awk '{print $1}'`
echo $fullname
echo $srvaddr
echo $srvport
echo $md5
#wget http://127.0.0.1:8888/dump --post-file=try.py

wget -O /dev/null "http://${srvaddr}:${srvport}/dump" "--post-file=${fullname}" "--header=Filename: $filename" "--header=Md5: $md5"
