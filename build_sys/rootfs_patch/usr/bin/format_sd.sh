#!/bin/sh

# First, umount FAT32
umount /media/sd

# Format that image with FAT32
mkdosfs -v -F 32 -n "boox" /media/sdmmc
