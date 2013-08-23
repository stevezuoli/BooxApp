#!/bin/bash

set -e

LTIB_DIR=/opt/freescale/ltib
ROOTFS_DIR=$LTIB_DIR/rootfs
WWWROOT=/opt/www
MERGE_ROOT=$LTIB_DIR/config/platform/imx31_3stack/merge
ROOTFS_IMAGE=$LTIB_DIR/rootfs.yaffs2

# Packages can't be handled by ltib
LIBUSB_DIR=/opt/onyx/pkgs/libusb-0.1.12
ZLIB_DIR=/opt/onyx/pkgs/zlib-1.2.3
EXPAT_DIR=/opt/onyx/pkgs/expat-2.0.1
FREETYPE_DIR=/opt/onyx/pkgs/freetype-2.3.7
DBUS_DIR=/opt/onyx/pkgs/dbus-1.2.12
TSLIB_DIR=/opt/onyx/pkgs/tslib
ICONV_DIR=/opt/onyx/pkgs/libiconv-1.13.1
CURL_DIR=/opt/onyx/pkgs/curl-7.19.7
SQLITE_DIR=/opt/onyx/pkgs/sqlite-3.5.7
OPENSSL_DIR=/opt/onyx/pkgs/openssl-0.9.7m
QT_DIR=/opt/onyx/pkgs/qt-embedded-linux-opensource-src-4.5.2
LIBDJVU_DIR=/opt/onyx/pkgs/djvulibre-3.5.22

SCRIPT_PATH=`readlink -f $0`
SCRIPT_DIR=`dirname "${SCRIPT_PATH}"`
REPO_DIR=`dirname "${SCRIPT_DIR}"`
echo "Repository path: ${REPO_DIR}"
BUILD_DIR="$REPO_DIR/build/arm"
export KERNEL_DIR=/opt/onyx/kernel/linux-2.6.26
KERNEL_IMAGE="$KERNEL_DIR/arch/arm/boot/zImage"
REDBOOT_IMAGE="$HOME/redboot.bin"
# Configure options.
export ONYX_SDK_ROOT=/opt/onyx/arm
export PATH=$PATH:$ONYX_SDK_ROOT/bin:/opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin
export PKG_CONFIG_PATH=$ONYX_SDK_ROOT/lib/pkgconfig
HOST=arm-linux
BUILD=i686

GUESSED_BRANCH=`cd "${REPO_DIR}" && git branch | grep '^\*' | cut -d ' ' -f 2`
GIT_BRANCH=${GIT_BRANCH-master}
BRANCH=${GIT_BRANCH-${GUESSED_BRANCH}}
echo "Building branch: $BRANCH"

without_music_player=0

build_libusb()
{
    arm-linux-strip $ONYX_SDK_ROOT/lib/libusb-0.1.so.4.4.4
    cp $ONYX_SDK_ROOT/lib/libusb-0.1.so.4.4.4 $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    cd $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    ln -sf libusb-0.1.so.4.4.4 libusb.so
    ln -sf libusb-0.1.so.4.4.4 libusb-0.1.so.4
}

build_zlib()
{
    arm-linux-strip $ONYX_SDK_ROOT/lib/libz.so.1.2.3
    cp $ONYX_SDK_ROOT/lib/libz.so.1.2.3 $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    cd $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    ln -sf libz.so.1.2.3 libz.so
    ln -sf libz.so.1.2.3 libz.so.1
}

build_expat()
{
    arm-linux-strip $ONYX_SDK_ROOT/lib/libexpat.so.1.5.2
    cp $ONYX_SDK_ROOT/lib/libexpat.so.1.5.2 $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    cd $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    ln -sf libexpat.so.1.5.2 libexpat.so
    ln -sf libexpat.so.1.5.2 libexpat.so.1
}

build_freetype()
{
    arm-linux-strip $ONYX_SDK_ROOT/lib/libfreetype.so.6.3.18
    cp $ONYX_SDK_ROOT/lib/libfreetype.so.6.3.18 $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    cd $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    ln -sf libfreetype.so.6.3.18 libfreetype.so
    ln -sf libfreetype.so.6.3.18 libfreetype.so.6
}

build_dbus()
{
    arm-linux-strip $ONYX_SDK_ROOT/lib/libdbus-1.so.3.4.0
    arm-linux-strip $ONYX_SDK_ROOT/bin/dbus-*
    cp $ONYX_SDK_ROOT/lib/libdbus-1.so.3.4.0 $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    cp $ONYX_SDK_ROOT/bin/dbus-* $MERGE_ROOT/$ONYX_SDK_ROOT/bin
    cp -r $ONYX_SDK_ROOT/etc/dbus-1 $MERGE_ROOT/$ONYX_SDK_ROOT/etc/

    cd $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    ln -sf libdbus-1.so.3.4.0 libdbus-1.so
    ln -sf libdbus-1.so.3.4.0 libdbus-1.so.3
}

build_tslib()
{
    arm-linux-strip $ONYX_SDK_ROOT/lib/libts-1.0.so.0.0.0
    arm-linux-strip $ONYX_SDK_ROOT/lib/ts/*.so
    cp $ONYX_SDK_ROOT/lib/libts-1.0.so.0.0.0 $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    cp $ONYX_SDK_ROOT/lib/ts/*.so $MERGE_ROOT/$ONYX_SDK_ROOT/lib/ts
    cp $ONYX_SDK_ROOT/etc/ts.conf $MERGE_ROOT/etc/

    cd $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    ln -sf libts-1.0.so.0.0.0 libts.so
    ln -sf libts-1.0.so.0.0.0 libts-1.0.so.0
}

build_iconv()
{
    arm-linux-strip $ONYX_SDK_ROOT/lib/libiconv.so.2.5.0
    arm-linux-strip $ONYX_SDK_ROOT/lib/libcharset.so.1.0.0
    cp $ONYX_SDK_ROOT/lib/libiconv.so.2.5.0 $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    cp $ONYX_SDK_ROOT/lib/libcharset.so.1.0.0 $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    cd $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    ln -sf libiconv.so.2.5.0 libiconv.so
    ln -sf libiconv.so.2.5.0 libiconv.so.2
    ln -sf libcharset.so.1.0.0 libcharset.so
    ln -sf libcharset.so.1.0.0 libcharset.so.1
}

build_curl()
{
    arm-linux-strip $ONYX_SDK_ROOT/lib/libcurl.so.4.1.1
    cp $ONYX_SDK_ROOT/lib/libcurl.so.4.1.1 $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    cd $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    ln -sf libcurl.so.4.1.1 libcurl.so
    ln -sf libcurl.so.4.1.1 libcurl.so.4
}

build_sqlite()
{
    arm-linux-strip $ONYX_SDK_ROOT/lib/libsqlite3.so.0.8.6
    cp $ONYX_SDK_ROOT/lib/libsqlite3.so.0.8.6 $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    cd $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    ln -sf libsqlite3.so.0.8.6 libsqlite3.so
    ln -sf libsqlite3.so.0.8.6 libsqlite3.so.0
}

build_openssl()
{
    arm-linux-strip $ONYX_SDK_ROOT/lib/libssl.so.0.9.7
    arm-linux-strip $ONYX_SDK_ROOT/lib/libcrypto.so.0.9.7
    cp $ONYX_SDK_ROOT/lib/libssl.so.0.9.7 $MERGE_ROOT$ONYX_SDK_ROOT/lib
    cp $ONYX_SDK_ROOT/lib/libcrypto.so.0.9.7 $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    cd $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    ln -sf libssl.so.0.9.7 libssl.so
    ln -sf libssl.so.0.9.7 libssl.so.0
    ln -sf libcrypto.so.0.9.7 libcrypto.so
    ln -sf libcrypto.so.0.9.7 libcrypto.so.0
}

build_qt()
{
    cd $ONYX_SDK_ROOT/lib
    arm-linux-strip libQt*.so.4.5.2
    cp libQt*.so.4.5.2 $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    cp fonts/*.ttf $MERGE_ROOT/$ONYX_SDK_ROOT/lib/fonts

    cd $ONYX_SDK_ROOT
    cp -r plugins/* $MERGE_ROOT/$ONYX_SDK_ROOT/plugins/

    cd $MERGE_ROOT/$ONYX_SDK_ROOT/lib
    for i in `ls libQt*.so.4.5.2`
    do
      ln -sf $i ${i/.so.4.5.2/.so}
      ln -sf $i ${i/.so.4.5.2/.so.4}
      ln -sf $i ${i/.so.4.5.2/.so.4.5}
    done
}

build_naboo()
{
    cd "$REPO_DIR"
    rake build:arm:all
    rake lrelease
    find "${BUILD_DIR}/bin" -type f -exec arm-linux-strip {} \;
    find "${BUILD_DIR}/bin" -type f -exec chmod +x {} \;
    cp -r "${BUILD_DIR}"/bin/* $MERGE_ROOT/$ONYX_SDK_ROOT/bin/
    if [ $with_music_player = 0 ]; then
      rm $MERGE_ROOT/$ONYX_SDK_ROOT/bin/music_player
    fi
    if [ $with_splash_update = 0 ]; then
      rm $MERGE_ROOT/$ONYX_SDK_ROOT/bin/update_splash
    fi
    arm-linux-strip "${BUILD_DIR}"/libs/libnaboo*.so
    arm-linux-strip "${BUILD_DIR}/libs/libpppd_onyx_plugin.so"
    arm-linux-strip "${BUILD_DIR}/libs/libonyx_ui.so"
    arm-linux-strip "${BUILD_DIR}/libs/libtts_ej.so"
    cp "${BUILD_DIR}/libs/libnabooscreen.so" \
        $MERGE_ROOT/$ONYX_SDK_ROOT/plugins/gfxdrivers/
    mkdir -p $MERGE_ROOT/$ONYX_SDK_ROOT/plugins/kbddrivers
    cp "${BUILD_DIR}/libs/libnabookeyboard.so" \
        $MERGE_ROOT/$ONYX_SDK_ROOT/plugins/kbddrivers/
    cp "${BUILD_DIR}/libs/libonyx_ui.so" \
        $MERGE_ROOT/$ONYX_SDK_ROOT/lib/
    cp "${BUILD_DIR}/libs/libpppd_onyx_plugin.so" \
        $MERGE_ROOT/usr/lib/pppd/2.4.4/
    cp "${BUILD_DIR}/libs/libtts_ej.so" \
        $MERGE_ROOT/usr/share/tts/plugins
    cp "${BUILD_DIR}/handwriting/plugins/libjhwr_plugin.so" \
        $MERGE_ROOT/usr/share/handwriting/plugins/
    find "$REPO_DIR" -name "*.qm" -exec cp -t $MERGE_ROOT/usr/share/translations {} \;
    unset QMAKESPEC
}

LOCK_FILE=/tmp/.lock

(set -C; : > $LOCK_FILE) 2> /dev/null
if [ $? != "0" ]; then
    echo "Lock File exists - exiting"
    exit 1
fi

trap 'rm $LOCK_FILE' EXIT

if [ ! -d "$REPO_DIR" ]; then
    echo "$REPO_DIR does not exist."
    exit 1
fi

source build-env.sh

# Create necessary directories.
rm -rf $MERGE_ROOT/*
mkdir -p $MERGE_ROOT/app
mkdir -p $MERGE_ROOT/$ONYX_SDK_ROOT/etc
mkdir -p $MERGE_ROOT/$ONYX_SDK_ROOT/bin
mkdir -p $MERGE_ROOT/$ONYX_SDK_ROOT/lib/fonts
mkdir -p $MERGE_ROOT/$ONYX_SDK_ROOT/lib/ts
mkdir -p $MERGE_ROOT/$ONYX_SDK_ROOT/plugins
mkdir -p $MERGE_ROOT/usr/lib/pppd/2.4.4
mkdir -p $MERGE_ROOT/usr/share/translations
mkdir -p $MERGE_ROOT/usr/share/handwriting/plugins
mkdir -p $MERGE_ROOT/usr/share/tts
mkdir -p $MERGE_ROOT/usr/share/tts/plugins
mkdir -p $MERGE_ROOT/etc
mkdir -p $MERGE_ROOT/media/sd
mkdir -p $MERGE_ROOT/media/usb
mkdir -p $MERGE_ROOT/media/mtd6
mkdir -p $MERGE_ROOT/media/flash

build_libusb
build_zlib
build_expat
build_freetype
build_dbus
build_tslib
build_iconv
build_curl
build_sqlite
build_openssl
build_qt
build_naboo

# Build rootfs
cd $LTIB_DIR
sudo rm -rf "$ROOTFS_DIR"
./ltib -m clean
./ltib

sudo cp -r "$REPO_DIR"/kernel_artifacts/rootfs_patch/* $ROOTFS_DIR/

cd "$REPO_DIR"
echo "export VERSION=\"1.5.1 `date +%Y%m%d`\"" > "$REPO_DIR/build_sys/rootfs_patch/etc/version"
GIT_COMMIT=`git log --oneline | head -1 | cut -d ' ' -f 1`
echo "export GIT_COMMIT=${GIT_COMMIT}" >> "$REPO_DIR/build_sys/rootfs_patch/etc/version"

# Patch rootfs
sudo cp -r "$REPO_DIR"/build_sys/rootfs_patch/* $ROOTFS_DIR/

# Delete unnecessary directories
sudo rm -rf "$ROOTFS_DIR/boot" "$ROOTFS_DIR/usr/include" "$ROOTFS_DIR/usr/local" "$ROOTFS_DIR/usr/share/man" "$ROOTFS_DIR"/var/* "$ROOTFS_DIR/usr/lib/fonts"
sudo rm -f "$ROOTFS_IMAGE"
# find $ROOTFS_DIR -type d -name "\.svn" | xargs sudo rm -rf

if [ "$PROFILE" != "" ] && [ "$PROFILE" != "none" ]
then
    PROFILE_ROOT="$REPO_DIR/build_sys/profiles/$PROFILE"
    PATCH_ROOTFS_HOOK="${PROFILE_ROOT}/patch_rootfs.sh"
    if [ -e "$PATCH_ROOTFS_HOOK" ]; then
        echo "Running $PATCH_ROOTFS_HOOK ..."
        source "$PATCH_ROOTFS_HOOK"
    else
        echo "$PATCH_ROOTFS_HOOK does not exist. Skipping."
    fi
fi

# Create yaffs2 image
sudo mkyaffs2image $ROOTFS_DIR $ROOTFS_IMAGE
sudo chmod 0644 $ROOTFS_IMAGE

dir="$REPO_DIR/artifacts"

if [ -d $dir ]; then
  rm -rf $dir
fi
mkdir -p $dir
tar -zcf $dir/rootfs.tar.gz $ROOTFS_IMAGE

cp "$REPO_DIR"/kernel_artifacts/zImage $dir
cp "$REPO_DIR"/kernel_artifacts/zImage-initramfs $dir

# Create update package
cp $dir/* "$REPO_DIR/boot/onyx_update/images"
cd "$REPO_DIR/boot"

tar -z -c --file=$dir/onyx_update.tgz --exclude="\.svn" onyx_update/*
aescrypt -e -p a8wZ49?b -o $dir/onyx_update.upd $dir/onyx_update.tgz
rm $dir/onyx_update.tgz

# Create final zip
BUILD_LABEL="${SOURCE_BRANCH}-${KERNEL_FLAVOR}-${PROFILE}-${BUILD_ID}"
cd $dir
cp -r "$REPO_DIR"/build_sys/packages/* .
zip -r "onyx_update-${BUILD_LABEL}.zip" onyx_update.upd dicts fonts manual language notes_template handwriting tts
md5sum -b "onyx_update-${BUILD_LABEL}.zip" > \
    "onyx_update-${BUILD_LABEL}.zip.md5"
rm -rf onyx_update.upd dicts fonts manual language notes_template handwriting tts zImage zImage-initramfs rootfs.tar.gz

# Create factory image
sudo cp "${REPO_DIR}/build_sys/firstboot.sh" $ROOTFS_DIR/etc/firstboot.sh
sudo cp -r "${REPO_DIR}/build_sys/packages/notes_template" $ROOTFS_DIR/etc/
sudo cp "${REPO_DIR}/boot/post-install" $ROOTFS_DIR/usr/bin/
sudo cp "${REPO_DIR}"/build_sys/packages/tts/* $ROOTFS_DIR/usr/share/tts/ 
sudo cp "${REPO_DIR}"/build_sys/packages/fonts/* $ROOTFS_DIR/opt/onyx/arm/lib/fonts/
sudo cp "${REPO_DIR}"/build_sys/packages/handwriting/* $ROOTFS_DIR/usr/share/handwriting/

if [ "$PROFILE" != "" ] && [ "$PROFILE" != "none" ]
then
    PROFILE_ROOT="$REPO_DIR/build_sys/profiles/$PROFILE"
    PATCH_FACTORY_ROOTFS_HOOK="${PROFILE_ROOT}/patch_factory_rootfs.sh"
    if [ -e "$PATCH_FACTORY_ROOTFS_HOOK" ]; then
        echo "Running $PATCH_FACTORY_ROOTFS_HOOK ..."
        source "$PATCH_FACTORY_ROOTFS_HOOK"
    else
        echo "$PATCH_FACTORY_ROOTFS_HOOK does not exist. Skipping."
    fi
fi

sudo mkyaffs2image $ROOTFS_DIR $ROOTFS_IMAGE
cd "$REPO_DIR/boot/onyx_update/images"
create-factory-image $REDBOOT_IMAGE boot-splash zImage zImage-initramfs $ROOTFS_IMAGE "$dir/all_in_one-${BUILD_LABEL}.bin"

echo "Done."
