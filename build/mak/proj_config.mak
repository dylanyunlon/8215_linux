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
# $RCSfile: Makefile,v $
# $Revision: #2 $
# $Date: 2015/07/03 $
# $Author: jianghong.lin $
# $MD5HEX: 1476db419289e71bcc068156dea7ae10 $
#
# Description:
#         Makefile to build a  ac8317 linux demo board build. The following
#         targets are supported:
#
#             all:           Compiles middleware, custom, target and this
#                            directory and creates a final image.
#             uboot:         Compiles the uboot.
#             kernel:        Compiles the custom.
#             driver:        Compiles the kernel module driver.
#             oss:           Compiles this directory and creates a final
#                            image.
#             arm2:          Build arm2
#             mw:            Compiles middleware
#             app:           Compiles application
#
#             clean:         Cleans the middleware, custom, target and this
#                            directory and removes the run image.
#             clean_kernel:  Cleans the kernel.
#             clean_oss:     Cleans the custom.
#             clean_driver:  Cleans all kennel module driver.
#             clean_force:   Forcefully removes all libray and object
#                            directories as well as the run image. A fast way
#                            to perform cleanup.
#
#         The following commands are supported:
#
#             BUILD_CFG=debug
#                 Builds the specified target with symbolic debug info and the
#                 define file "mt5372_demo.def" is read. The run image is named
#                 is named "mt5372_demo_dbg".If BUILD_CFG is not specified then
#                 a non-debug build is created, which does not contain any
#                 symbolic debug information.
#############################################################################


# Set initial values.
#
ifeq ("$(origin BUILD_CFG)", "command line")
  export BUILD_CFG := $(BUILD_CFG)
endif
ifndef BUILD_CFG
  export BUILD_CFG := rel
endif

ifndef TOOL_CHAIN
  #export TOOL_CHAIN = 4.5.1
  export TOOL_CHAIN = 4.8.2
endif

export ENABLE_CA9 = true

ifndef ENABLE_CA9
  ifndef ENABLE_VFP
    export ENABLE_VFP=true
  endif
endif

export CONFIG_REBUILD_OSS := false

export LINUX_SOLUTION := true
export KERNEL_VER     := linux-3.18
export ROOTFS_NAME    := root.tar.gz
export ROOTFS_VENDOR  := 
export TARGET_IC      := ac8317
export UBOOT_VERSION  := 2009
export CUST_KERNEL_VER := $(KERNEL_VER)
export CUST_LOADER_VER := $(UBOOT_VERSION)


ifeq "$(RAMDISK)" "true"
  export RAMDISK_ROOTFS      := true
endif

ifeq "$(AC83XX_BOOT_DEVICE)" ""
  export AC83XX_BOOT_DEVICE  := mmc
endif

ifeq "$(AC83XX_BOOT_DEVICE)" "mmc"
  export SYS_IMG_FS          := ext4
else
  ifeq "$(AC83XX_BOOT_DEVICE)" "nand"
    export SYS_IMG_FS        := ubifs
    export UBIFS             := true
  else
    export SYS_IMG_FS          := ext4
  endif
endif

#
#  3rd party
#  

ifndef 3RD_BUILD_EXIST
  export 3RD_BUILD_EXIST :=
endif

#
#  Audio Description
#

#default project is atc, or else please add customer name
ifndef CUSTOMER
  export CUSTOMER   := $(PROJECT)
endif

ifndef MODEL_NAME
  export MODEL_NAME :=
endif

ifeq "$(PROJECT)" "atc"
  export MODEL_NAME := ac8317
endif


THIS_ROOT  := $(shell pwd)

#HW config 


ifndef OPTIMIZE_LVL
  export OPTIMIZE_LVL = -O2
endif


ifneq "$(DA_TOP)" ""
  DA_TOP := $(word 1, $(subst /$(SRC_ROOT_DIR)/,/$(SRC_ROOT_DIR) /, $(THIS_ROOT)))
endif

#
# Include main make file
#
include $(DA_TOP)/$(TOP_MAKE)/main.mak