#!/bin/sh
./flash_eraseall /dev/mtd2
cat zImage | ./nandwrite -p /dev/mtd2
./flash_eraseall /dev/mtd3
cat zImage-initramfs | ./nandwrite -p /dev/mtd3
./fix-kernel-crc zImage zImage-initramfs
reboot

