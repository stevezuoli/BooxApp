#!/bin/sh

export TOOLCHAIN_PATH=/opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi
export PATH=$TOOLCHAIN_PATH/usr/bin:$TOOLCHAIN_PATH/bin:$PATH
export PKG_CONFIG_PATH=$TOOLCHAIN_PATH/usr/lib/pkgconfig
NABOO_REPO_DIR=/opt/onyx/naboo/trunk
NABOO_BUILD_DIR=/opt/onyx/naboo/build-trunk

cd $NABOO_BUILD_DIR
rm -rf ./bin/*
export QMAKESPEC=$TOOLCHAIN_PATH/usr/mkspecs/qws/linux-arm-g++
cmake -DBUILD_FOR_ARM:BOOL=ON -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON $NABOO_REPO_DIR
make

