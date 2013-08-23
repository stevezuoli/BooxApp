#!/bin/bash

tmp=$1
old=${tmp:2}

new=${old/arm-none-linux-gnueabi/arm-linux}
ln -sf $old $new
