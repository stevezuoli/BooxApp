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
if [ ! -f "/root/Settings/vdd" ]; then
    rm -rf /root/Settings/vdd
    echo "export FIXED_VDD=1" > /root/Settings/vdd
fi
# Set hw time (UTC time)

# Set timezone

# Check audio, TODO

