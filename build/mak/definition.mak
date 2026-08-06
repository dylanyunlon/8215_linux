###############################################################################
# Copyright Statement:                                                        #
#                                                                             #
#   This software/firmware and related documentation ("Autochips Software")    #
# are protected under international and related jurisdictions'copyright laws  #
# as unpublished works. The information contained herein is confidential and  #
# proprietary to Autochips Inc. Without the prior written permission of        #
# Autochips Inc., any reproduction, modification, use or disclosure of         #
# Autochips Software, and information contained herein, in whole or in part,   #
# shall be strictly prohibited.                                               #
# Autochips Inc. Copyright (C) 2015. All rights reserved.                      #
#                                                                             #
#   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND      #
# AGREES TO THE FOLLOWING:                                                    #
#                                                                             #
#   1)Any and all intellectual property rights (including without             #
# limitation, patent, copyright, and trade secrets) in and to this            #
# Software/firmware and related documentation ("Autochips Software") shall     #
# remain the exclusive property of Autochips Inc. Any and all intellectual     #
# property rights (including without limitation, patent, copyright, and       #
# trade secrets) in and to any modifications and derivatives to Autochips      #
# Software, whoever made, shall also remain the exclusive property of         #
# Autochips Inc.  Nothing herein shall be construed as any transfer of any     #
# title to any intellectual property right in Autochips Software to Receiver.  #
#                                                                             #
#   2)This Autochips Software Receiver received from Autochips Inc. and/or its  #
# representatives is provided to Receiver on an "AS IS" basis only.           #
# Autochips Inc. expressly disclaims all warranties, expressed or implied,     #
# including but not limited to any implied warranties of merchantability,     #
# non-infringement and fitness for a particular purpose and any warranties    #
# arising out of course of performance, course of dealing or usage of trade.  #
# Autochips Inc. does not provide any warranty whatsoever with respect to the  #
# software of any third party which may be used by, incorporated in, or       #
# supplied with the Autochips Software, and Receiver agrees to look only to    #
# such third parties for any warranty claim relating thereto.  Receiver       #
# expressly acknowledges that it is Receiver's sole responsibility to obtain  #
# from any third party all proper licenses contained in or delivered with     #
# Autochips Software.  Autochips is not responsible for any Autochips Software   #
# releases made to Receiver's specifications or to conform to a particular    #
# standard or open forum.                                                     #
#                                                                             #
#   3)Receiver further acknowledge that Receiver may, either presently        #
# and/or in the future, instruct Autochips Inc. to assist it in the            #
# development and the implementation, in accordance with Receiver's designs,  #
# of certain softwares relating to Receiver's product(s) (the "Services").    #
# Except as may be otherwise agreed to in writing, no warranties of any       #
# kind, whether express or implied, are given by Autochips Inc. with respect   #
# to the Services provided, and the Services are provided on an "AS IS"       #
# basis. Receiver further acknowledges that the Services may contain errors   #
# that testing is important and it is solely responsible for fully testing    #
# the Services and/or derivatives thereof before they are used, sublicensed   #
# or distributed. Should there be any third party action brought against      #
# Autochips Inc. arising out of or relating to the Services, Receiver agree    #
# to fully indemnify and hold Autochips Inc. harmless.  If the parties         #
# mutually agree to enter into or continue a business relationship or other   #
# arrangement, the terms and conditions set forth herein shall remain         #
# effective and, unless explicitly stated otherwise, shall prevail in the     #
# event of a conflict in the terms in any agreements entered into between     #
# the parties.                                                                #
#                                                                             #
#   4)Receiver's sole and exclusive remedy and Autochips Inc.'s entire and     #
# cumulative liability with respect to Autochips Software released hereunder   #
# will be, at Autochips Inc.'s sole discretion, to replace or revise the       #
# Autochips Software at issue.                                                 #
#                                                                             #
#   5)The transaction contemplated hereunder shall be construed in            #
# accordance with the laws of Singapore, excluding its conflict of laws       #
# principles.  Any disputes, controversies or claims arising thereof and      #
# related thereto shall be settled via arbitration in Singapore, under the    #
# then current rules of the International Chamber of Commerce (ICC).  The     #
# arbitration shall be conducted in English. The awards of the arbitration    #
# shall be final and binding upon both parties and shall be entered and       #
# enforceable in any court of competent jurisdiction.                         #
###############################################################################
###########################################################################
# $RCSfile: definition.mak,v $
# $Revision: #1 $
# $Date: 2015/07/02 $
# $Author: jianghong.lin $
#
# Description:
#
#############################################################################

export SHELL ?= /bin/bash

ifndef KERNEL_VER
$(error KERNEL_VER is undefined)
endif

ifndef TOOL_CHAIN
$(error TOOL_CHAIN is undefined)
endif

ifndef CUSTOMER
$(error CUSTOMER is undefined)
endif

ifndef MODEL_NAME
$(error MODEL_NAME is undefined)
endif

ifndef OSS_ROOT
  export OSS_ROOT := $(DA_TOP)/oss
endif

ifndef LINUX_SOLUTION
	export LINUX_SOLUTION = true
endif



ifdef VERSION
export UPGRADE_PKG_VER ?= $(VERSION)
endif

ifndef UBOOT_VERSION
	export UBOOT_VERSION := 1.3.4
endif

ifndef DFB_VERSION
	export DFB_VERSION := 1.5.3
endif

ifndef VERSION
export VERSION := MTK.LINUX.1.0.0
endif


ifndef REBUILD_3RD_PARTY
export REBUILD_3RD_PARTY := false
endif

ifndef REBUILD_GST_LIBS
export REBUILD_GST_LIBS := false
endif
export GST_USB_NAME ?= local
export GST_3RD_NAME ?= gst

ifndef DAILY_BUILD
export DAILY_BUILD := false
endif

ifndef CLEAN_ALL_CFG_OBJ
export CLEAN_ALL_CFG_OBJ = false
endif

ifndef BUILD_CFG
export CLEAN_ALL_CFG_OBJ := true
BUILD_CFG := rel
endif
export BUILD_CFG

ifndef RLS_CUSTOM_BUILD
export RLS_CUSTOM_BUILD := $(shell if [ -d $(VM_LINUX_ROOT)/project_x/mtk_obj ]; then echo 'true'; else echo 'false'; fi )
endif

ifndef RLS_CUSTOM_BUILD_KERNEL_ONLY
export RLS_CUSTOM_BUILD_KERNEL_ONLY := $(shell if [ -f $(VM_LINUX_ROOT)/project_x/mtk_obj/.RLS_CUSTOM_BUILD_KERNEL_ONLY ]; then echo 'true'; else echo 'false'; fi )
endif

ifndef 3RD_BUILD_EXIST
export 3RD_BUILD_EXIST := $(shell if [ -e $(VM_LINUX_ROOT)/dtv_linux/mak/3rd_build.mak ]; then echo "true"; else echo ""; fi)
endif

export INIT_DONE_LOG := $(VM_LINUX_ROOT)/output/$(CUSTOMER)/$(MODEL_NAME)/$(BUILD_CFG)/obj/.MTK_DTV_BUILD_INIT_DONE.txt
export SYMLINK_LOG := $(VM_LINUX_ROOT)/output/$(CUSTOMER)/$(MODEL_NAME)/$(BUILD_CFG)/obj/.MTK_DTV_BUILD_SYMLINK.txt


ifndef ODB
export ODB := false
endif

ifndef ROOTFS_DYNAMIC_GEN
export ROOTFS_DYNAMIC_GEN := true
endif

ifeq "$(RLS_CUSTOM_BUILD)" "true"
export ROOTFS_DYNAMIC_GEN := false
endif

ifndef MMP_SUPPORT
export MMP_SUPPORT = full
endif


ifeq "$(BUILD_CFG)" "debug"
export CUST_OBJECT_TYPE := cli
else
ifeq "$(BUILD_CFG)" "cli"
export CUST_OBJECT_TYPE := cli
else
export CUST_OBJECT_TYPE := rel
endif
endif

ifndef OBJECT_TYPE
ifndef BUILD_CFG
export OBJECT_TYPE := rel
else
export OBJECT_TYPE := $(BUILD_CFG)
endif
endif


#
# Set the configuration
#
ifeq "$(COMBINED_BUILD_CFG)" "true"
export CFG_DEF := $(MODEL_NAME).cfg
else
ifeq "$(BUILD_CFG)" "debug"
export CFG_DEF := $(MODEL_NAME)_dbg.cfg
else
ifeq "$(BUILD_CFG)" "cli"
export CFG_DEF := $(MODEL_NAME)_cli.cfg
else
export CFG_DEF := $(MODEL_NAME).cfg
endif
endif
endif

ifndef COMPANY
export COMPANY = mtk
endif

ifndef MODEL
export MODEL = mt5392bm1v1US
endif

ifndef MW_MODEL
export MW_MODEL = mt5392b_atsc_linux
endif

ifndef BRAND
export BRAND := $(COMPANY)
endif

ifndef ODB
export ODM := $(COMPANY)
endif

ifndef NEED_CLI_MODULE
ifdef STANDALONE_CLI_MODULE
export NEED_CLI_MODULE = $(STANDALONE_CLI_MODULE)
endif
endif

export BUILTIN ?= FALSE
export DYNAMIC_KERNEL_CONFIG ?= FALSE

export SCRIPT_SCR ?= script.scr

export DOWNLOAD_XML ?= download.xml

ifndef ROOTFS_NAME
export ROOTFS_NAME = 5391_free
endif

export ROOTFS_TARGET = $(OBJ_ROOT)/rootfs/rootfs_$(ROOTFS_NAME)

export USB_UPGRADE_FILE_NAME ?= upgrade.pkg
export USB_UPGRADE_FILE_LOADER_NAME ?= upgrade_loader.pkg

export KER_MODULES_DIR ?= kernel_modules

ifndef AUTORUN_SH
export AUTORUN_SH := autorun.sh
endif


ifndef REF_TARGET_IC
export REF_TARGET_IC :=$(TARGET_IC)
endif

ifndef TARGET_DRIVER
export TARGET_DRIVER := $(subst mt,,$(TARGET_IC))_driver
export DRIVER_DIR := $(TARGET_DRIVER)
endif

ifndef KEY_FROM_DFB
export KEY_FROM_DFB := true
endif

ifeq "$(KEY_FROM_DFB)" "true"
export key_from_dfb := y
else
export key_from_dfb := n
endif

export CUST_CLR_IMG ?= n

export DFBRC_FROM_DFB ?= false
export SAWMANRC_FROM_DFB ?= false


export STRIP := $(lastword $(STRIP))
export STRIP_FLAG := --strip-unneeded -R .comment

export CONFIG_NEW_WIFI_SUPPLICANT ?= true
export WIFI_DONGLE ?= RT557X

ifndef NOT_NEED_O
export NOT_NEED_O = false
endif

ifndef LOG_UIMAGE
export LOG_UIMAGE := uImage.log
endif

export VFP_SUFFIX  =

ifeq "$(TOOL_CHAIN)" "4.5.1"
ifeq "$(ENABLE_VFP)" "true"
export VFP_SUFFIX = _vfp
else ifeq "$(ENABLE_CA9)" "true"
export VFP_SUFFIX = _vfp_ca9
endif
else
ifeq "$(ENABLE_VFP)" "true"
export VFP_SUFFIX = _vfp
endif
endif

ifeq "$(ENABLE_CA9)" "true"
export CPU_TYPE = ca9
else
export CPU_TYPE = arm11
endif

export SIGGEN_KEY := $(shell if [ -e $(THIS_ROOT)/data/public_key -a -e $(THIS_ROOT)/data/private_key ]; then echo "$(THIS_ROOT)/data/public_key $(THIS_ROOT)/data/private_key"; else echo "";fi)
export NFSB_KEY   := $(shell if [ -e $(THIS_ROOT)/data/nfsb_public_key -a -e $(THIS_ROOT)/data/nfsb_private_key ]; then echo "$(THIS_ROOT)/data/nfsb_public_key $(THIS_ROOT)/data/nfsb_private_key"; else echo "";fi)

export USIG_GEN_ID ?= MTK

ifeq "$(KERNEL_VER)" "linux-2.6.27"
export MKFS_VERSION := 3.0
else ifeq "$(KERNEL_VER)" "linux-2.6.35"
export MKFS_VERSION := 4.2
export MKSQUASHFS_COMPR := -comp lzo
export MKFS_DEVTAB := $(VM_LINUX_ROOT)/chiling/rootfs/common/misc/dev4_2.txt
else
export MKFS_VERSION := 4.2
export MKSQUASHFS_COMPR := -comp lzo
export MKFS_DEVTAB := $(VM_LINUX_ROOT)/chiling/rootfs/common/misc/dev4_2.txt
endif

CHK_WARN_SCRIPT := $(VM_LINUX_ROOT)/project_x/tools/pbuild/chk_warn.sh
export BRANCH_NAME  = $(word 2,$(BUILD_NAME))

ifeq "$(CHECK_WARNING_BUILD)" "true"
export FIND_AUTHOR ?= true
endif
export FIND_AUTHOR ?= false

#
#UBOOT Configuration
#
ifndef SECURE_BOOT
export SECURE_BOOT = n
endif

ifndef BOOT_TYPE
export BOOT_TYPE := ROM2NAND
endif

ifndef BOOT
export BOOT := nand
ifeq "$(BOOT_TYPE)" "ROM2NAND"
export BOOT := nand
endif

ifeq "$(BOOT_TYPE)" "ROM2EMMC"
export BOOT = emmc
endif

ifeq "$(BOOT_TYPE)" "ROM2NOR"
export BOOT = nor
endif
endif

#
#3rd_party library
#
ifeq "$(findstring DVBCT,$(BRDCST_IF))" "DVBCT"
export 3RD_PARTIES:=LINUX
endif
ifeq "$(findstring ISDBT,$(BRDCST_IF))" "ISDBT"
export 3RD_PARTIES:=LINUX
endif

ifeq "$(findstring MN88471,$(BRDCST_IF))" "MN88471"
export 3RD_PARTIES:=LINUX
endif

ifeq "$(findstring CXD2820,$(BRDCST_IF))" "CXD2820"
export 3RD_PARTIES:=LINUX
endif

ifeq "$(3RD_RO)" "true"
    export 3RD_RW_PATH ?= 3rd_rw

    ifneq "$(3RD_RO_DBG)" "true"
        export 3RD_UPG_PART_BASE ?= true
    else
        export 3RD_UPG_PART_BASE ?= false
    endif
else
    export 3RD_RW_PATH ?= 3rd
    export 3RD_UPG_PART_BASE ?= false
endif

export 3RD_EXE_PATH ?= 3rd

ifeq "$(3RD_UPG_PART_BASE)" "true"
    export 3RD_UPG_METHOD ?= PART
    export 3RD_UPG_FILE ?= 3rd_file
    export 3RD_UPG_PATH ?= 12
else
    export 3RD_UPG_METHOD ?= FILE
    export 3RD_UPG_FILE ?= 3rd.tgz
    export 3RD_UPG_PATH ?= /3rd
endif

export 3RD_PATH ?= 3rd

#
#Kernel Config
#
KERNEL_WIFI_STR1 := $(KERNEL_VER)

ifeq "$(ENABLE_CA9)" "true"
KERNEL_WIFI_STR2 := _CA9
else 
KERNEL_WIFI_STR2 := _ARM11
endif

ifeq "$(ENABLE_SMP)" "true"
KERNEL_WIFI_STR3 := _SMP
else
KERNEL_WIFI_STR3 := _NONSMP
endif

KERNEL_CONFIG_SMP:=
ifeq "$(ENABLE_SMP_IC)" "true"
KERNEL_WIFI_STR3 := _SMP
KERNEL_CONFIG_SMP:= _smp
endif

ifeq "$(BUILD_CFG)" "debug"
KERNEL_WIFI_STR4 := _DBG
else ifeq "$(BUILD_CFG)" "cli"
KERNEL_WIFI_STR4 := _DBG
else ifeq "$(ENABLE_MET_TOOL)" "true"
KERNEL_WIFI_STR4 := _DBG
else
KERNEL_WIFI_STR4 := _REL
endif

export KERNEL_VER_FOR_3RD :=$(KERNEL_WIFI_STR1)$(KERNEL_WIFI_STR2)$(KERNEL_WIFI_STR3)$(KERNEL_WIFI_STR4)

ifndef KERNEL_CONFIG
ifeq "$(TV_DRV_VFY)" "true"
ifdef KERNEL_DEFINE
	KERNEL_CONFIG=$(REF_TARGET_IC)$(KERNEL_CONFIG_SMP)_$(KERNEL_DEFINE)_mod_vfy$(KERNEL_CONFIG_DBG)_defconfig
else
	KERNEL_CONFIG=$(REF_TARGET_IC)$(KERNEL_CONFIG_SMP)_mod_vfy$(KERNEL_CONFIG_DBG)_defconfig
endif
else
ifdef KERNEL_DEFINE
	KERNEL_CONFIG=$(REF_TARGET_IC)$(KERNEL_CONFIG_SMP)_$(KERNEL_DEFINE)_mod$(KERNEL_CONFIG_DBG)_defconfig
else
	KERNEL_CONFIG=$(REF_TARGET_IC)$(KERNEL_CONFIG_SMP)_mod$(KERNEL_CONFIG_DBG)_defconfig
endif
endif
endif
export KERNEL_CONFIG
ifeq "$(BUILD_CFG)" "debug"
MW_AP_EXT=_dbg
KERNEL_CONFIG_DBG=_dbg
export KERNEL_DEBUG=y
export DEBUG=y
else
MW_AP_EXT=
ifeq "$(ENABLE_MET_TOOL)" "true"
KERNEL_CONFIG_DBG=_dbg
else
KERNEL_CONFIG_DBG=
endif
export KERNEL_DEBUG=n
export DEBUG=n
endif
ifeq "$(BUILTIN)" "true"
KERNEL_CONFIG := $(REF_TARGET_IC)_defconfig
endif
export KERNEL_CONFIG

ifndef O
export O = $(KERNEL_OBJ_ROOT)/$(KERNEL_VER)/$(KERNEL_CONFIG)
endif

ifndef KERNEL_CONFIG_MOD_DBG
ifneq "$(subst _mod_,,$(KERNEL_CONFIG))" "$(KERNEL_CONFIG)"
	export KERNEL_CONFIG_MOD_DBG=$(KERNEL_CONFIG)
else	
	export KERNEL_CONFIG_MOD_DBG=$(REF_TARGET_IC)_mod_defconfig
endif
endif

ifdef KERNEL_CONFIG_DIFF
export KERNEL_CONFIG_DIFF_FILES := $(patsubst %,$(VM_LINUX_ROOT)/chiling/kernel/$(KERNEL_VER)/arch/arm/configs/$(REF_TARGET_IC)_%.cdiff,$(KERNEL_CONFIG_DIFF))
endif

#
#Kernel module
#

FS_KER_MOD_LIST := fat.ko   \
                   vfat.ko  \
                   msdos.ko

ifeq "$(NTFS_WRITE_SUPPORT)" "true"
FS_KER_MOD_LIST += fuse.ko
endif

FS_KER_MOD_LIST += ntfs.ko


ifeq "$(MTP_SUPPORT)" "true"
FS_KER_MOD_LIST += fuse.ko
endif

ifeq "$(EXFAT_SUPPORT)" "true"
FS_KER_MOD_LIST += fuse.ko
endif

ifeq "$(CODEPAGE_936)" "true"
FS_KER_MOD_LIST += nls_cp936.ko
endif
ETH_KER_MOD_LIST := star_mac.ko

USB_KER_MOD_LIST := usbcore.ko            \
                    mtk-hcd.ko            \
                    xhci-hcd.ko           \
                    sg.ko                 \
                    hid.ko                \
                    usbhid.ko             \
                    usb-storage.ko
                    
ifeq "$(SUPPORT_TTY_USB)" "true"
USB_KER_MOD_LIST += usbserial.ko        \
                    pl2303.ko
endif

BLK_KER_MOD_LIST := brd.ko             \
                    loop.ko            \
                    scsi_mod.ko        \
                    sd_mod.ko

ifeq "$(RAMZSWAP_SUPPORT)" "true"
BLK_KER_MOD_LIST += ramzswap.ko
endif

V4L_KER_MOD_LIST := compat_ioctl32.ko     \
                    v4l2-common.ko        \
                    v4l1-compat.ko        \
                    v4l2-int-device.ko    \
                    videodev.ko           \
                    uvcvideo.ko           \
                    snd-mtk.ko

BLUETOOTH_KER_MOD_LIST := bluetooth.ko    \
                          l2cap.ko        \
                          sco.ko          \
                          rfcomm.ko       \
                          bnep.ko         \
                          hidp.ko         \
                          btusb.ko        \
                          hci_uart.ko     \
                          uinput.ko
  
ifeq "$(SDMMC_SUPPORT)" "true"                     
MMC_KER_MOD_LIST := mmc_core.ko            \
                    mmc_block.ko            \
                    msdc_drv.ko  
endif         

export FS_KER_MOD_LIST
export ETH_KER_MOD_LIST
export USB_KER_MOD_LIST
export BLK_KER_MOD_LIST
export V4L_KER_MOD_LIST
export BLUETOOTH_KER_MOD_LIST
export MMC_KER_MOD_LIST
ifeq "$(ENABLE_MET_TOOL)" "true"
ifneq "$(ANDROID)" "true"
export AUTO_BUILD_MET_TOOL := /mtkoss/met/auto_build_met_linux
export MET_SCRIPT_DIR := 3rd/lib/linux_met
endif
endif

