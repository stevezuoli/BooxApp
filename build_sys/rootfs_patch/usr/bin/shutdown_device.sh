#!/bin/sh

killall -9 udevd
killall -9 watchdog.sh
killall -9 wpa_supplicant

# Don't clear screen now.
echo "0" > /sys/onyx/clear_at_poweroff

poweroff

