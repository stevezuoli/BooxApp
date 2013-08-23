#!/bin/sh

# Not a good solution. I don't know why it does not use the system env.
# Without this, the dbus-send can not send message to system bus.
export DBUS_SYSTEM_BUS_ADDRESS="unix:path=/var/run/dbus/system_bus_socket"
export PATH=$PATH:/opt/onyx/arm/bin

if [ $1 = "ip-up" ]; then
    cp /etc/ppp/resolv.conf /etc/
    dbus-send --system --print-reply --type=method_call --dest=com.onyx.service.system_manager /com/onyx/object/system_manager com.onyx.interface.system_manager.modemNotify string:"ip-up" string:"" string:""
fi

if [ $1 = "pppd-exit" ]; then
    dbus-send --system --print-reply --type=method_call --dest=com.onyx.service.system_manager /com/onyx/object/system_manager com.onyx.interface.system_manager.modemNotify string:"pppd-exit" string:"$2" string:""
fi
