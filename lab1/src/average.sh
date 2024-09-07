#!/bin/bash
maxcount=0
count=0
filename=$1
readarray file < ./$filename
for item in ${file[*]}
    do
    let "count += 1"
    let "maxcount += item"
done
rez=$(bc<<<"scale=3;$maxcount/$count")
echo $rez