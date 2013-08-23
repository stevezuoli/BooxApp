#!/bin/sh

wfm_addr=0x0886
wfm_modesel_offset=0x10
wfm_type_offset=0x13

let wfm_modesel_addr=wfm_addr+wfm_modesel_offset
let wfm_type_addr=wfm_addr+wfm_type_offset

echo "Finding waveform type for installed waveform..."
echo "Looking at address $wfm_type_addr"
# read serial flash to find waveform type for waveform
/mnt/mmc/bin/bs_sfmrw read $wfm_type_addr 1 /tmp/wfmtype
type=`hexdump -v -n 1 -e '"%01d"' /tmp/wfmtype`


echo "Finding mode layout for installed waveform..."
echo "Looking at address $wfm_modesel_addr"
# read serial flash to find mode map type for waveform
/mnt/mmc/bin/bs_sfmrw read $wfm_modesel_addr 1 /tmp/modenum
modenum=`hexdump -v -n 1 -e '"%01d"' /tmp/modenum`


# now strip the old mode definitions and wfm address 
# (if any) out of /etc/profile
echo "Modifying /etc/profile for waveform related environment variables"
sed -e '/MODE_INIT.*/d' \
    -e '/MODE_MU.*/d' \
    -e '/MODE_GU.*/d' \
    -e '/MODE_GC.*/d' \
    -e '/MODE_DU.*/d' \
    -e '/MODE_GC4.*/d' \
    -e '/MODE_GC16.*/d' \
    -e '/WFM_ADDR.*/d' \
    </etc/profile > profile_stripped
cp profile_stripped /etc/profile



# now concatenate new mode defs and wfm address to /etc/profile
echo "Found waveform type $type, mode layout $modenum"
# check if wfm is type TD, WD, or VD
if [ "$type" -eq 8 ] || [ "$type" -eq 15 ] || [ "$type" -eq 16 ]
    then
    if [ "$modenum" -eq 0 ]; then
	echo "export MODE_INIT=0" | cat - >> /etc/profile
	echo "export MODE_MU=1" | cat - >> /etc/profile 
	echo "export MODE_GU=2" | cat - >> /etc/profile 
	echo "export MODE_GC=3" | cat - >> /etc/profile 
	echo "export MODE_DU=4" | cat - >> /etc/profile 
	
    elif [ "$modenum" -eq 1 ]; then	
	echo "export MODE_INIT=0" | cat - >> /etc/profile
	echo "export MODE_MU=1" | cat - >> /etc/profile 
	echo "export MODE_GU=2" | cat - >> /etc/profile 
	echo "export MODE_GC=3" | cat - >> /etc/profile 
	echo "export MODE_DU=4" | cat - >> /etc/profile 

    elif [ "$modenum" -eq 2 ]; then
	echo "export MODE_INIT=0" | cat - >> /etc/profile
	echo "export MODE_MU=1" | cat - >> /etc/profile 
	echo "export MODE_DU=1" | cat - >> /etc/profile 
	echo "export MODE_GU=2" | cat - >> /etc/profile 
	echo "export MODE_GC=3" | cat - >> /etc/profile
    fi

# check if wfm is type TE, WE, or VE
elif [ "$type" -eq 11 ] || [ "$type" 14 ] || [ "$type" -eq 17 ]
    then
    if [ "$modenum" -eq 1 ]; then
	echo "export MODE_INIT=0" | cat - >> /etc/profile
	echo "export MODE_MU=1" | cat - >> /etc/profile 
	echo "export MODE_GU=2" | cat - >> /etc/profile 
	echo "export MODE_GC=2" | cat - >> /etc/profile 
	echo "export MODE_DU=1" | cat - >> /etc/profile 
	echo "export MODE_GC4=3" | cat - >> /etc/profile
	echo "export MODE_GC16=2" | cat - >> /etc/profile  
    fi

# default case for all others or unknown type
else
        echo "Not a know waveform type; using default mode numbering"
        echo "export MODE_INIT=0" | cat - >> /etc/profile
	echo "export MODE_MU=1" | cat - >> /etc/profile 
	echo "export MODE_GU=2" | cat - >> /etc/profile 
	echo "export MODE_GC=3" | cat - >> /etc/profile 
	echo "export MODE_DU=4" | cat - >> /etc/profile 
fi
    
echo "export WFM_ADDR=$wfm_addr" | cat - >> /etc/profile 

exit
	

