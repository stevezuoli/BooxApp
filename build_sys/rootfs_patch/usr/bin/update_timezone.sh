#!/bin/sh

if [ $# -ne 1 ]; then
    echo "Usage: `basename $0` <zone>"
    exit 1
fi

if [ -f /etc/localtime ]; then
    rm -f /etc/localtime
fi
ln -sf $1 /etc/localtime


