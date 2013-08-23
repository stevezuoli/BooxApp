#!/bin/sh

RETRY_TIMES=3

killall -9 udhcpc

if [ $1 = "acquire" ]; then
    ifdown -f eth0
    loop=0
    while [ $loop -lt $RETRY_TIMES ]
    do
        ifup -f eth0
        ifconfig eth0 | grep "inet addr:"
        if [ $? = 0 ]; then
            exit 0
        fi
        loop=`expr $loop + 1`
    done

    exit 1
else
    ifdown -f eth0
fi
