#!/bin/sh

# This script will set up the development environment on a clean
# installation of Ubuntu linux.

set -o nounset
set -o errexit

# Install Ubuntu packages.
sudo apt-get -y install build-essential autoconf automake libqt4-dev libqt4-sql-sqlite libfribidi-dev liblinebreak-dev cmake libtool libboost-dev

# Install newer version of libgoogle-perftools-dev
[ ! -f /usr/lib/libtcmalloc.a ] && \
    cd /tmp && \
    wget http://google-perftools.googlecode.com/files/libgoogle-perftools0_0.99.1-1_i386.deb && \
    sudo dpkg -i libgoogle-perftools0_0.99.1-1_i386.deb && \
    wget http://google-perftools.googlecode.com/files/libgoogle-perftools-dev_0.99.1-1_i386.deb && \
    sudo dpkg -i libgoogle-perftools-dev_0.99.1-1_i386.deb

# Install Google's C++ testing framework.
[ ! -f /usr/local/include/gtest/gtest.h ] && \
svn co http://googletest.googlecode.com/svn/tags/release-1.1.0/ /tmp/googletest && \
cd /tmp/googletest && \
autoscan && autoheader && aclocal && libtoolize --automake && autoconf && \
automake -ac --add-missing --foreign && ./configure && make && sudo make install

