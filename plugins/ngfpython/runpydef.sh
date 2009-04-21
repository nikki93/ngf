#!/bin/sh

for file in pydef/*.pydef
do
    newfile=`echo $file | sed 's/\.pydef/_py.h/'`
    cat $file | ngfpydef | gperf > $newfile
done
