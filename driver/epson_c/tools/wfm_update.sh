#!/bin/sh

# updates a waveform in flash and resets environment variables
# to reflect mode structure of new waveform
#
# Usage: wfm_update  <wfm.wbf>

if [ -z "$1" ]; then
    echo "Usage: $0 <wfm.wbf>"
    exit $NOARGS
fi

if [ ! -f "$1" ]; then
    echo "File $1 not found!"
    exit $NOTFOUND
fi

filesize=`ls -l $1 | awk '{print $5}'`
echo "filesize is $filesize bytes"

# burn the waveform to flash; should probably check the file first...
/mnt/mmc/bin/bs_sfmrw write $WFM_ADDR $filesize $1

# reset the mode environment variables
/mnt/mmc/bin/wfm_setup.sh

exit