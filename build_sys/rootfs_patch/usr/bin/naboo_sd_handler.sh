#!/bin/sh
# SD card event handler.

# Not a good solution. I don't know why it does not use the system env.
# Without this, the dbus-send can not send message to system bus.
export DBUS_SYSTEM_BUS_ADDRESS="unix:path=/var/run/dbus/system_bus_socket"
export PATH=$PATH:/opt/onyx/arm/bin

echo $ACTION
if [ $ACTION = "add" ]
then
    dbus-send --system --print-reply --type=method_call  --dest=com.onyx.service.system_manager /com/onyx/object/system_manager com.onyx.interface.system_manager.sdCardChanged boolean:true
    mount -t vfat /dev/sdmmc /media/sd -o iocharset=utf8,atime,diratime
else
    dbus-send --system --print-reply --type=method_call  --dest=com.onyx.service.system_manager /com/onyx/object/system_manager com.onyx.interface.system_manager.sdCardChanged boolean:false
    umount /dev/sdmmc
fi

exit 0
