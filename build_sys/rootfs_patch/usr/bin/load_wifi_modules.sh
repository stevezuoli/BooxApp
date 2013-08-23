#!/bin/sh

# Not a good solution. I don't know why it does not use the system env.
# Without this, the dbus-send can not send message to system bus.
export DBUS_SYSTEM_BUS_ADDRESS="unix:path=/var/run/dbus/system_bus_socket"
export PATH=$PATH:/opt/onyx/arm/bin

modprobe libertas_sdio
sleep 1
ifconfig eth0 up
