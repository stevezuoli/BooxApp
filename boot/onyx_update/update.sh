#!/bin/sh

check_version()
{
    # Return 0 if $2 doesn't exist
    [ -f $2 ] || return 0

    # Return 0 if $1 is newer or equal than $2, otherwise 1
    major1=`awk -F. '{print $1}' $1`
    minor1=`awk -F. '{print $2}' $1`
    major2=`awk -F. '{print $1}' $2`
    minor2=`awk -F. '{print $2}' $2`

    if [ "$major1" -gt "$major2" ]; then
      return 0
    elif [ "$major1" -lt "$major2" ]; then
      return 1
    elif [ "$minor1" -gt "$minor2" ]; then
      return 0
    elif [ "$minor1" -lt "$minor2" ]; then
      return 1
    else
      return 0
    fi
}

mount -t yaffs2 /dev/mtdblock5 /root
if [ -f /mnt/mmc/language ]; then
    if [ ! -d /root/Settings ]; then
        mkdir -p /root/Settings
    fi
    echo "cp /mnt/mmc/language /root/Settings/"
    cp /mnt/mmc/language /root/Settings/
fi

if [ $? = 0 ]; then
    # Compare existing software version and package version
    if check_version ./version /root/version; then
        if [ -d /mnt/mmc/notes_template ]; then
            mkdir -p /root/notes_template
            echo "Copying notes template..."
            cp /mnt/mmc/notes_template/* /root/notes_template/
        fi
        cp ./version /root/version
        umount /root
        echo "Update is in progress..."
    else
        umount /root
        exit 1
    fi
else
    # Can't mount /dev/mtdblock5, erase that partition
    bin/flash_eraseall /dev/mtd5
fi

# Update redboot if there is a redboot update
#if [ -f "images/redboot.bin" ]; then
#    bin/flash_erase /dev/mtd0 0 2
#    cat images/redboot.bin | bin/nandwrite -m -p /dev/mtd0 || echo "Nandwrite problem with redboot"
#fi

# Update boot splash if there is a boot splash in update package
if [ -f "images/boot-splash" ]; then
    bin/flash_erase /dev/mtd1 0 4
    cat images/boot-splash | bin/nandwrite -m -p /dev/mtd1 || echo "Nandwrite problem with boot splash"
fi

# Update zImage if there is a zImage in update package
if [ -f "images/zImage" ]; then
    bin/flash_eraseall /dev/mtd2
    cat images/zImage | bin/nandwrite -m -p /dev/mtd2 || echo "Nandwrite problem with zImage"
fi

# Update initramfs if there is a initramfs in update package
if [ -f "images/zImage-initramfs" ]; then
    bin/flash_eraseall /dev/mtd3
    cat images/zImage-initramfs | bin/nandwrite -m -p /dev/mtd3 || echo "Nandwrite problem with zImage-initramfs"
fi

# Fix kernel/initramfs size and crc stored in FIS.
bin/fix-kernel-crc images/zImage images/zImage-initramfs

# Update rootfs if there is a rootfs update.
if [ -f "images/rootfs.tar.gz" ]; then
    bin/flash_eraseall /dev/mtd4
    tar -Ozxf images/rootfs.tar.gz | bin/nandwrite -m -a -o /dev/mtd4 || echo "Nandwrite problem with rootfs.yaffs2"
fi

# Mount real rootfs
mount -t yaffs2 /dev/mtdblock4 /newroot

if [ -d /mnt/mmc/manual ]; then
    echo "Copying manual..."
    cp /mnt/mmc/manual/* /newroot/usr/share/explorer/manual
fi

if [ -d /mnt/mmc/dicts ]; then
    echo "Copying dictionary..."
fi

if [ -d /mnt/mmc/fonts ]; then
    echo "Copying fonts..."
    for i in /mnt/mmc/fonts/*.ttf /mnt/mmc/fonts/*.ttc
    do
        if [ ! -f $i ]; then
            continue
        fi

        # Get file size
        file_size=`ls -l $i | awk '{print $5}'`
        space_left=`df | grep /dev/mtdblock4 | awk '{print $4}'`
        if [ `expr $space_left \* 1024 - 5242880` -gt $file_size ]; then
            echo "cp $i /newroot/opt/onyx/arm/lib/fonts/"
            cp $i /newroot/opt/onyx/arm/lib/fonts/
        fi
    done
fi

if [ -d /mnt/mmc/handwriting ]; then
    echo "Copying handwriting files..."
    mkdir -p /newroot/usr/share/handwriting/
    for i in  /mnt/mmc/handwriting/*
    do
        if [ ! -f $i ]; then
            continue
        fi

        # Get file size
        file_size=`ls -l $i | awk '{print $5}'`
        space_left=`df | grep /dev/mtdblock4 | awk '{print $4}'`
        if [ `expr $space_left \* 1024 - 5242880` -gt $file_size ]; then
             echo "cp $i /newroot/usr/share/handwriting/"
             cp $i /newroot/usr/share/handwriting/
        fi
    done
fi


if [ -d /mnt/mmc/tts ]; then
     echo "Copying tts files..."
     mkdir -p /newroot/usr/share/tts/
     for i in  /mnt/mmc/tts/*
     do
         if [ ! -f $i ]; then
            continue
         fi

         # Get file size
         file_size=`ls -l $i | awk '{print $5}'`
         space_left=`df | grep /dev/mtdblock4 | awk '{print $4}'`
         if [ `expr $space_left \* 1024 - 5242880` -gt $file_size ]; then
              echo "cp $i /newroot/usr/share/tts/"
              cp $i /newroot/usr/share/tts/
         fi
    done
fi


sync
umount /newroot

echo "Update complete."
