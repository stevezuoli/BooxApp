#!/bin/sh

set -e

export ECOS_REPOSITORY=/home/jinlei/ecos/ecos_20050912/packages
ecosconfig new mx31_3stack redboot
ecosconfig import $ECOS_REPOSITORY/hal/arm/mx31/3stack/current/misc/redboot_ROMRAM.ecm
ecosconfig tree
make

