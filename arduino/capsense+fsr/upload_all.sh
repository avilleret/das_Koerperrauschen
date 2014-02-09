#!/bin/bash

for arduino in /dev/ttyACM* ; 
do 
ino upload -m micro -p $arduino
echo $?
echo wait a little bit...
sleep 1
done
