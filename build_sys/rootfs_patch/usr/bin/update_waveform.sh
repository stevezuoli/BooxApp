#!/bin/sh
if [ $# -ne 1 ]; then
    echo "Usage: `basename $0` <type>"
    exit 1
fi

rm -rf /root/Settings/waveform
if [ $1 = "8" ]; then
    bs_sfmrw write 0 2182 /usr/share/waveform/8.bin
    bs_sfmrw write 2182 51839  /usr/share/waveform/8.wbf
    echo "export WAVEFORM=3" > /root/Settings/waveform
elif [ $1 = "16" ]; then
    bs_sfmrw write 0 2182 /usr/share/waveform/16.bin
    bs_sfmrw write 2182 46597 /usr/share/waveform/16.wbf
    echo "export WAVEFORM=2" > /root/Settings/waveform
fi
