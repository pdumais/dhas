#!/bin/sh
mkdir -p $2
for file in $1/*.wav ; do
    F=`basename $file`
    sox -V $file -r 44100 -c 2 -b 16 -t wav $2/${F%.wav}-s16.wav
    sox -V $file -r 8000 -c 1 -t wav -e u-law $2/${F%.wav}-ulaw.wav
done
