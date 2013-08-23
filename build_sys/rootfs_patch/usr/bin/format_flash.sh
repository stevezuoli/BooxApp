#!/bin/sh

# Get capacity of /dev/mtdblock6
hex_cap=`cat /proc/mtd | grep mtd6 | awk '{print $2}'`

# Convert capacity from hex to decimal
dec_cap=0
counter=1
len=`expr length $hex_cap`
while [ $counter -le $len ];
do
    c=`expr substr $hex_cap $counter 1`
    case $c in
      a|A) c="10"
        ;;
      b|B) c="11"
        ;;
      c|C) c="12"
        ;;
      d|D) c="13"
        ;;
      e|E) c="14"
        ;;
      f|F) c="15"
        ;;
      *)
        ;;
    esac

    dec_cap=`expr $dec_cap \* 16 + $c`
    counter=`expr $counter + 1`
done

# Convert dec_cap to MB
dec_cap=`expr $dec_cap / 1024 / 1024 - 2`

# Backup DRM info
if [ -f /media/flash/.adobe-digital-editions/activation.xml ]; then
    cp /media/flash/.adobe-digital-editions/activation.xml /tmp
fi

# First, umount FAT32 over yaffs2
umount /media/flash

# Then umount yaffs2
umount /media/mtd6

# Erase whole partition
flash_eraseall /dev/mtd6

# Mount mtdblock6 as yaffs2
mount -t yaffs2 /dev/mtdblock6 /media/mtd6

# Create a big image
touch /media/mtd6/vfat.bin
gen_huge_file /media/mtd6/vfat.bin $dec_cap

# Format that image with FAT32
mkdosfs -v -F 32 -n "boox" /media/mtd6/vfat.bin

# Mount that image as FAT32 partition
mount -t vfat -o loop /media/mtd6/vfat.bin /media/flash

# Create fingerprint on FAT32 fs

naboo_reader drm fingerprint

# Restore DRM info
if [ -f /tmp/activation.xml ]; then
    cp /tmp/activation.xml /media/flash/.adobe-digital-editions/
    rm -f /tmp/activation.xml
fi

