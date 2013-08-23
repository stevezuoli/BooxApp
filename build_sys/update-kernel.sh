#!/bin/sh

flash_erase /dev/mtd0 0 2
cat ./redboot.bin | nandwrite -m -p /dev/mtd0

flash_erase /dev/mtd1 0 4
cat ./boot-splash.dat | nandwrite -m -p /dev/mtd1

flash_eraseall /dev/mtd2
cat ./zImage | nandwrite -m -p /dev/mtd2
flash_eraseall /dev/mtd3
cat ./zImage-initramfs | nandwrite -m -p /dev/mtd3
./fix-kernel-crc ./zImage ./zImage-initramfs
