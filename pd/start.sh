#!/bin/bash
_DIR=$(dirname $(readlink -f $0))
# sleep 10 
xterm -title "Sensor Processing" -e "pd -nomidi -noaudio $_DIR/Main_sensor-processing.pd" &
sleep 10
xterm -title "Audio Processing" -e "pd -r 48000 -blocksize 256 -nomidi -noadc -rt $_DIR/Main_dsp.pd" &
# sleep 10
# xterm -title "Data Plotting" -e "pd -nomidi -noaudio $_DIR/Main_dislpay.pd" &

