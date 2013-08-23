#!/bin/sh
killall -9 usb_module.sh

RETRY_TIMES=10
pppd_pid_file=/var/run/ppp-onyx.pid

if [ ! -f $pppd_pid_file ]; then
    exit
fi

pppd_pid=`head -n 1 $pppd_pid_file`

# Send SIGTERM
kill $pppd_pid
loop=0
while [ $loop -lt $RETRY_TIMES ]
do
    if [ -f $pppd_pid_file ]; then
        sleep 1
    else
        break
    fi
    loop=`expr $loop + 1`
done

ps -ef | grep "pppd connect"
if [ $? = 0 ]; then
    killall -9 pppd
    rm -rf $pppd_pid_file
fi
