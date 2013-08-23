
ifndef TOPDIR
    TOPDIR = .
endif

##########################################################
#
# ISP1362 Hardware Selection (enable one of the following)
# 
##########################################################
#PCI_KIT = y
PXA250_KIT = y


##########################################################
#
# Accelent IDP hardware revision Selection
# 
##########################################################
ifeq ($(PXA250_KIT),y)
#PXA250_REV2 = y
PXA250_REV4 = y
endif


#############################################
#  PXA250 Development Kit options
#############################################
ifeq ($(PXA250_KIT),y)

ifeq ($(PXA250_REV4),y)
KERN_MOD_FLAGS = -DCONFIG_1362_PXA250 -DCONFIG_PXA250_REV4 
else
KERN_MOD_FLAGS = -DCONFIG_1362_PXA250 -DCONFIG_PXA250_REV2
endif

CROSS_COMPILE     ?= /usr/local/arm/2.95.3/bin/arm-linux-
AS    	?= $(CROSS_COMPILE)as
LD    	?= $(CROSS_COMPILE)ld
CC    	?= $(CROSS_COMPILE)gcc 

CFLAGS = -Wno-unknown-pragmas -Wstrict-prototypes -O2 -pipe -msoft-float -mshort-load-bytes -march=armv4 -mtune=strongarm1100 -fno-strict-aliasing
KERNELDIR = $(KERNEL_PATH)

APPS_CFLAGS = -g -Wall -O $(KERN_MOD_FLAGS)

else

#############################################
#  PCI Development Kit options
#############################################

ifeq ($(PCI_KIT),y)
KERN_MOD_FLAGS = -DCONFIG_1362_PCI
endif


KERNELDIR = $(TOPDIR)/../..
CC = gcc
CFLAGS = -g -Wall -O -Wstrict-prototypes -Wno-trigraphs -O2 -fomit-frame-pointer -fno-strict-aliasing -fno-common -pipe -mpreferred-stack-boundary=2 -march=i686
APPS_CFLAGS = -g -Wall -O 


endif


################################################
# For OTG/HOST/DEVICE modes (enable one of them)
################################################
#OTG = y
HOSTANDDEVICE=y
#HOSTONLY = y
#DEVICEONLY = y


ifeq ($(OTG),y)
OTGFLAG = -DCONFIG_USB_OTG
endif

#############################################
# Selection for OTG mass storage demo
#############################################
OTG_MASS_STORAGE_DEMO = 1

#####################################
# mass storage device size (8MB/16MB)
#####################################
#MS_SIZE_FLAG = _8MB_SIZE_
MS_SIZE_FLAG = _128MB_SIZE_

##########################################################
# Debug flags    Function level(FUNC) detail level (DETAIL)
##########################################################
#DEBUG_FLAG = -DCONFIG_FUNC_DEBUG
#DEBUG_FLAG = -DCONFIG_DETAIL_DEBUG

OTG_DEBUG = 1
PDC_DEBUG = 1
PDC_CD_DEBUG = 1
HAL_DEBUG = 1
PHCI_DEBUG = 1

#################
# Directory paths
#################
INCLUDEDIR = $(KERNELDIR)/include
HAL_INCL_DIR = $(TOPDIR)/hal
OBJ_INSTALL_DIR = $(TOPDIR)/objs
HOST_DIR = $(TOPDIR)/host
DEVICE_DIR = $(TOPDIR)/device
OTG_DIR = $(TOPDIR)/otg
APPL_DIR = $(TOPDIR)/appl
KERN_VER_FILE = $(INCLUDEDIR)/linux/version.h
KERN_VERSION     = $(shell awk -F\" '/REL/ {print $$2}' $(KERN_VER_FILE))
KERN_INSTALL_DIR = $(INSTALL_MOD_PATH)/lib/modules/$(KERN_VERSION)/kernel/drivers/usb/
