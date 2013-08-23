#!/bin/sh
if [ $# -ne 1 ]; then
    echo "Usage: `basename $0` <date>"
    exit 1
fi

date -s $1
hwclock -w -u
