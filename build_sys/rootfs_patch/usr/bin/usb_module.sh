#!/bin/sh

RETRY_TIMES=5

# Script for loading USB modules.
if [ $# -ne 1 ]; then
    echo "Usage: `basename $0` <detect|connect|disconnect|serial|3G-host|cleanup>"
    exit 1
fi

if [ $1 = "detect" ]; then
    modprobe -r pcconn_mon
    modprobe pcconn_mon
elif [ $1 = "connect" ]; then
    modprobe -r pcconn_mon
    naboo_reader drm fingerprint
    umount /media/flash
    umount /media/sd
    if [ -b /dev/sdmmc ]; then
        modprobe g_file_storage file=/media/mtd6/vfat.bin,/dev/sdmmc removable=y
    else
        modprobe g_file_storage file=/media/mtd6/vfat.bin removable=y
    fi
elif [ $1 = "disconnect" ]; then
    modprobe -r pcconn_mon
    modprobe -r g_file_storage
    dbus-send --system --print-reply --type=method_call --dest=com.onyx.service.system_manager /com/onyx/object/system_manager com.onyx.interface.system_manager.reportUSBConnectionChanged boolean:false
    mount /media/flash
    naboo_reader drm fingerprint
    if [ -b /dev/sdmmc ]; then
        ACTION=add /usr/bin/naboo_sd_handler.sh
    fi
elif [ $1 = "serial" ]; then
    modprobe g_serial
elif [ $1 = "3G-host" ]; then
    lsmod | grep ehci_hcd
    if [ $? != 0 ]; then
        modprobe ehci_hcd
    fi

    # Don't use infinite loop
    loop=0
    while [ $loop -lt $RETRY_TIMES ]
    do
        lsusb | grep -v "Device 001"
        if [ $? = 0 ]; then
            echo "Found device."
            break
        fi

        if [ `value 0 22` = "0" ]; then
            echo "3G switch is powered off."
            loop=$RETRY_TIMES
            break;
        fi


        loop=`expr $loop + 1`
        echo "Retry $loop"
        sleep 1
    done

    if [ $loop -ge $RETRY_TIMES ]; then
        echo "Could not find device."
        dbus-send --system --print-reply --type=method_call --dest=com.onyx.service.system_manager /com/onyx/object/system_manager com.onyx.interface.system_manager.modemNotify string:"device-not-found" string:"" string:""
        exit 1
    fi

    vp=`lsusb | grep -v "Device 001" | awk '{print $6}'`
    vendor_id=`expr substr $vp 1 4`
    product_id=`expr substr $vp 6 4`

    if [ "$vp" = "12d1:1d09" ]; then
        lsmod | grep cdc_acm
        if [ $? != 0 ]; then
            modprobe cdc-acm
        fi
    else
        lsmod | grep usbserial
        if [ $? != 0 ]; then
            modprobe usbserial vendor=0x$vendor_id product=0x$product_id
        fi
    fi

    modprobe ppp_async

    dbus-send --system --print-reply --type=method_call --dest=com.onyx.service.system_manager /com/onyx/object/system_manager com.onyx.interface.system_manager.modemNotify string:"device-detected" string:"$vendor_id" string:"$product_id"
elif [ $1 = "cleanup" ]; then
    modprobe -r g_file_storage
    modprobe -r pcconn_mon
fi

