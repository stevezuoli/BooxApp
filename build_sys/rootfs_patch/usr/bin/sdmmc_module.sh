#!/bin/sh

# script for loading or unloading sdmmc modules.
if [ $# -ne 1 ]; then
    echo "Usage: `basename $0` <load|unload|reload>"
    exit 1
fi

if [ $1 = "load" ]; then
    modprobe mxc_mmc
    modprobe mmc_block
elif [ $1 = "unload" ]; then
    modprobe -r mmc_block
    modprobe -r mxc_mmc
elif [ $1 = "reload" ]; then
    modprobe -r mmc_block
    modprobe -r mxc_mmc
    modprobe mxc_mmc
    modprobe mmc_block
fi

