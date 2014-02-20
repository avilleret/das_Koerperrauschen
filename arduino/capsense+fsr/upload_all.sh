#!/bin/bash

ino clean
echo Build the sketch...
ino build -m micro

for arduino in /dev/ttyACM* ; 
do 
echo upload to $arduino
ino upload -m micro -p $arduino
echo wait a little bit...
sleep 1
done
