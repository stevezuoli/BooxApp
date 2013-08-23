#!/bin/sh

if [ -f "/etc/firstboot.sh" ]; then
    /etc/firstboot.sh
    if [ $? != 0 ]; then
        exit 1
    else
        rm -f /etc/firstboot.sh
        poweroff
    fi
fi

# Recreate the config directories if necessary.
if [ ! -d "/root/Settings" ]; then
    rm -f /root/Settings
    mkdir -p /root/Settings
fi

if [ ! -d "/root/onyx_reader" ]; then
    rm -f /root/onyx_reader
    mkdir -p /root/onyx_reader
fi

if [ ! -d "/usr/share/adobe/resources/fonts" ]; then
    ln -s /opt/onyx/arm/lib/fonts /usr/share/adobe/resources/fonts
fi

if [ ! -f "/root/Settings/language" ]; then
    rm -rf /root/Setting/language  # just in case there's a dir with the name.
    echo "export LANG=en_US.UTF-8" > /root/Settings/language
fi

if [ ! -f "/root/Settings/pointercal" ]; then
    rm -rf /root/Settings/pointercal
    echo "-2287 1 59751 5 2280 -27771556 -34758" > /root/Settings/pointercal
fi

if [ ! -f "/root/Settings/screen" ]; then
    rm -rf /root/Settings/screen
    echo "export QWS_DISPLAY=Transformed:Rot0:OnyxScreen:/dev/mem" > /root/Settings/screen
fi

if [ ! -f "/root/Settings/waveform" ]; then
    rm -rf /root/Settings/waveform
    echo "export WAVEFORM=3" > /root/Settings/waveform
fi

[ -f /etc/profile ] && source /etc/profile

# Load mmc modules.
sdmmc_module.sh load

# Now we're using system bus.
if [ ! -d "/var/run/dbus" ]; then
    rm -f /var/run/dbus
    mkdir -p /var/run/dbus/
fi

if [ -f "/var/run/dbus/pid" ]; then
    rm -f /var/run/dbus/pid
fi
dbus-daemon --system --print-address

# Start watchdog, a safe wrapper for system_manager
/etc/watchdog.sh &
