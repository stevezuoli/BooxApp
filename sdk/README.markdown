

# The Onyx Boox SDK

## Introduction

The Onyx Boox SDK is used for developing applications on the Boox
ebook readers produced by Onyx International.

## Development environment

The toolchain used for cross-compiling can be obtained at
https://github.com/onyx-intl/toolchain. This repository contains toolchains for various processors (platforms). Each directory is for one platform.

### imx31
The directory `imx31` is for BOOX A(X)60/A(X)60S, M90.

### marvell166e
The directory `marvell166e` is for BOOX A(X)61/A(X)61S, BOOX M91/M91S.

### imx508
The directory `imx508` is for BOOX A62/A62S, M92/M92S and I62. The file `gcc-4.4.4-glibc-2.11.1-multilib-1.0.tar.gz` is the toolchain compiler.

## Building the SDK

Grab the source:
    git clone git@github.com:onyx-intl/booxsdk.git
    git submodule update --init

Then download some prebuilt third-party libraries:
    In each directory in toolchain repository that mentioned above, there is an archive named with sdk_xxx.tar.gz. The archive contains the prebuilt third-party libraries for the target platform. Extract it to `/opt` too.
    Note: In `/opt/onyx/arm/` directory, there can be only one platform at a time. If needed to build different platforms, rename the `arm` to different names, like `arm_imx508`, `arm_imx31`, `arm_marvell`. And link `arm` to the real directory by command:
    cd /opt/onyx/ && ln -s arm_imx508/ arm

If you want to build the third-party libraries from source, you can
get the source code at http://opensource.onyx-international.com/

To build the SDK, install CMake and Rake, then run
    rake build:arm:default

CCache and DistCC will be used if they are available. If you do not
want to use DistCC, append `DISABLE_DISTCC=1` to the build command.

The SDK is frequently updated and sometimes may break compatibility
with earlier versions. If you want your application to run no matter
which version of the libraries are installed on the device, you can
link your application against the static libraries. You can build the
static libraries by running
    rake build:arm:static

## Notes for 64-bit hosts

If you are running a 64-bit environment, please make sure you have
32-bit runtime libraries installed. (For example, on Arch Linux, it is `multilib/lib32-gcc-libs`.)


# Open source packages

Please visit http://opensource.onyx-international.com/ to download all source packages.

