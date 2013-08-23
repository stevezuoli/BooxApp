#!/bin/sh

retry_count=0

# Format /root
flash_eraseall /dev/mtd5

# Format internal flash
format_flash.sh

# Set default language
mount -t yaffs2 /dev/mtdblock5 /root
mkdir -p /root/Settings
post-install

if [ -d /etc/notes_template ]; then
    mv /etc/notes_template /root/
fi

# Set grayscale
if [ ! -f "/root/Settings/grayscale" ]; then
    rm -rf /root/Settings/grayscale
    echo "export GRAYSCALE=2" > /root/Settings/grayscale
fi

# Set VDD
#if [ ! -f "/root/Settings/vdd" ]; then
#    rm -rf /root/Settings/vdd
#    echo "export FIXED_VDD=1" > /root/Settings/vdd
#fi

# Set hw time (UTC time)

# Set timezone

# Check WiFi
grep "NO_TOUCH" /etc/profile
if [ $? = 0 ]; then
  # Simplified version
  exit 0
fi

grep "TG" /etc/profile
if [ $? = 0 ]; then
  # 3G version
  exit 0
fi

# Standard version, with WiFi
modprobe mxc_mmc
modprobe libertas_sdio
sleep 3
ifconfig eth0 up
if [ $? != 0 ]; then
    echo "Can't bring eth0 up!"
    blink_led &
    exit 1
fi

while [ $retry_count -lt 3 ]
do
    iwlist eth0 scan | grep Cell
    if [ $? = 0 ]; then
        break;
    fi
    retry_count=`expr $retry_count + 1`
done

if [ $retry_count -eq 3 ]; then
    echo "Can't get any scan result!"
    blink_led &
    exit 1
fi

# Check audio, TODO

