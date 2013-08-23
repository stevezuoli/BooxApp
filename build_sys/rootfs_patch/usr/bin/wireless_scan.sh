#!/bin/sh
modprobe libertas_sdio
ifconfig eth0 up
iwlist eth0 scan


