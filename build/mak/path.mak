################################################################################
# Copyright Statement:                                                         #
#                                                                              #
#   This software/firmware and related documentation ("Autochips Software")    #
# are protected under international and related jurisdictions'copyright laws   #
# as unpublished works. The information contained herein is confidential and   #
# proprietary to Autochips Inc. Without the prior written permission of        #
# Autochips Inc., any reproduction, modification, use or disclosure of         #
# Autochips Software, and information contained herein, in whole or in part,   #
# shall be strictly prohibited.                                                #
# Autochips Inc. Copyright (C) 2015. All rights reserved.                      #
#                                                                              #
#   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND       #
# AGREES TO THE FOLLOWING:                                                     #
#                                                                              #
#   1)Any and all intellectual property rights (including without              #
# limitation, patent, copyright, and trade secrets) in and to this             #
# Software/firmware and related documentation ("Autochips Software") shall     #
# remain the exclusive property of Autochips Inc. Any and all intellectual     #
# property rights (including without limitation, patent, copyright, and        #
# trade secrets) in and to any modifications and derivatives to Autochips      #
# Software, whoever made, shall also remain the exclusive property of          #
# Autochips Inc.  Nothing herein shall be construed as any transfer of any     #
# title to any intellectual property right in Autochips Software to Receiver.  #
#                                                                              #
#   2)This Autochips Software Receiver received from Autochips Inc. and/or its #
# representatives is provided to Receiver on an "AS IS" basis only.            #
# Autochips Inc. expressly disclaims all warranties, expressed or implied,     #
# including but not limited to any implied warranties of merchantability,      #
# non-infringement and fitness for a particular purpose and any warranties     #
# arising out of course of performance, course of dealing or usage of trade.   #
# Autochips Inc. does not provide any warranty whatsoever with respect to the  #
# software of any third party which may be used by, incorporated in, or        #
# supplied with the Autochips Software, and Receiver agrees to look only to    #
# such third parties for any warranty claim relating thereto.  Receiver        #
# expressly acknowledges that it is Receiver's sole responsibility to obtain   #
# from any third party all proper licenses contained in or delivered with      #
# Autochips Software.  Autochips is not responsible for any Autochips Software #
# releases made to Receiver's specifications or to conform to a particular     #
# standard or open forum.                                                      #
#                                                                              #
#   3)Receiver further acknowledge that Receiver may, either presently         #
# and/or in the future, instruct Autochips Inc. to assist it in the            #
# development and the implementation, in accordance with Receiver's designs,   #
# of certain softwares relating to Receiver's product(s) (the "Services").     #
# Except as may be otherwise agreed to in writing, no warranties of any        #
# kind, whether express or implied, are given by Autochips Inc. with respect   #
# to the Services provided, and the Services are provided on an "AS IS"        #
# basis. Receiver further acknowledges that the Services may contain errors    #
# that testing is important and it is solely responsible for fully testing     #
# the Services and/or derivatives thereof before they are used, sublicensed    #
# or distributed. Should there be any third party action brought against       #
# Autochips Inc. arising out of or relating to the Services, Receiver agree    #
# to fully indemnify and hold Autochips Inc. harmless.  If the parties         #
# mutually agree to enter into or continue a business relationship or other    #
# arrangement, the terms and conditions set forth herein shall remain          #
# effective and, unless explicitly stated otherwise, shall prevail in the      #
# event of a conflict in the terms in any agreements entered into between      #
# the parties.                                                                 #
#                                                                              #
#   4)Receiver's sole and exclusive remedy and Autochips Inc.'s entire and     #
# cumulative liability with respect to Autochips Software released hereunder   #
# will be, at Autochips Inc.'s sole discretion, to replace or revise the       #
# Autochips Software at issue.                                                 #
#                                                                              #
#   5)The transaction contemplated hereunder shall be construed in             #
# accordance with the laws of Singapore, excluding its conflict of laws        #
# principles.  Any disputes, controversies or claims arising thereof and       #
# related thereto shall be settled via arbitration in Singapore, under the     #
# then current rules of the International Chamber of Commerce (ICC).  The      #
# arbitration shall be conducted in English. The awards of the arbitration     #
# shall be final and binding upon both parties and shall be entered and        #
# enforceable in any court of competent jurisdiction.                          #
################################################################################
###########################################################################
# $RCSfile: path.mak,v $
# $Revision: #8 $
# $Date: 2015/07/25 $
# $Author: ke.xu $
#
# Description:
#         Root path definitions for all components. It is assumed that
#         PROJECT_ROOT(atc_linux) has alreday been exsited.
#
#############################################################################

#
#Define root
#
ifeq "$(SRC_ROOT_DIR)" ""
  export SRC_ROOT_DIR := atc_linux
endif
ifndef SRC_ROOT_DIR
  export SRC_ROOT_DIR       := atc_linux
endif

THIS_ROOT          := $(shell pwd)
export DA_ROOT     := $(word 1, $(subst /$(SRC_ROOT_DIR)/,/$(SRC_ROOT_DIR) /, $(THIS_ROOT)))
export LINUX_ROOT  := $(DA_ROOT)

TOOL_CHAIN ?= 4.8.2

export VFP_SUFFIX  = 
ifeq "$(TOOL_CHAIN)" "4.5.1"
    ifeq "$(ENABLE_VFP)" "true"
        export VFP_SUFFIX = _vfp
    else ifeq "$(ENABLE_CA9)" "true"
        export VFP_SUFFIX = _vfp_ca9
    endif
else
    export VFP_SUFFIX = _vfp
endif

ifeq "$(LINUX_MAK_ROOT)" ""
  export LINUX_MAK_ROOT  := $(DA_ROOT)/build/mak
endif
ifndef LINUX_MAK_ROOT
  export LINUX_MAK_ROOT  := $(DA_ROOT)/build/mak
endif

export TOOL_ROOT       := $(DA_ROOT)/build/tools
export LIBATC_ROOT         := $(DA_ROOT)/lib


-include $(DA_ROOT)/oss/source/mak/oss_versoin.mak
#
# Define open source software root path
#
export LIB_SUB_ROOT   = lib/gnuarm-$(TOOL_CHAIN)$(VFP_SUFFIX)
ifeq "$(OSS_ROOT)" ""
  ifndef OSS_ROOT
    export OSS_ROOT       := $(DA_ROOT)/oss
  endif
endif

export OSS_SRC_ROOT   := $(OSS_ROOT)/source
export OSS_LIB_ROOT   := $(OSS_ROOT)/$(LIB_SUB_ROOT)


############################Open src lib config start########################
 # busybox
 export BUSYBOX_LIB_PATH   := $(OSS_LIB_ROOT)/busybox/$(BUSYBOX_VERSION)/lib
 export BUSYBOX_INC_PATH   := $(OSS_LIB_ROOT)/busybox/$(BUSYBOX_VERSION)/include

 # kmod
 export KMOD_LIB_PATH   := $(OSS_LIB_ROOT)/kmod/$(KMOD_VERSION)/lib
 export KMOD_INC_PATH   := $(OSS_LIB_ROOT)/kmod/$(KMOD_VERSION)/include
 
 # libpng
 export LIBPNG_LIB_PATH := $(OSS_LIB_ROOT)/libpng/$(LIBPNG_VERSION)/lib
 export LIBPNG_INC_PATH := $(OSS_LIB_ROOT)/libpng/$(LIBPNG_VERSION)/include
 
 # ncurse
 export NCURSE_LIB_PATH := $(OSS_LIB_ROOT)/ncurse/$(NCURSE_VERSION)/lib
 export NCURSE_INC_PATH := $(OSS_LIB_ROOT)/ncurse/$(NCURSE_VERSION)/include
 
 # tslib
 export TSLIB_LIB_PATH  := $(OSS_LIB_ROOT)/tslib/$(TSLIB_VERSION)/lib
 export TSLIB_INC_PATH  := $(OSS_LIB_ROOT)/tslib/$(TSLIB_VERSION)/include
 
 # udev
 export UDEV_LIB_PATH   := $(OSS_LIB_ROOT)/udev/$(TSLIB_VERSION)/lib
 export UDEV_INC_PATH   := $(OSS_LIB_ROOT)/udev/$(TSLIB_VERSION)/include
 
 # qt-free
 export QT_LIB_PATH            := $(OSS_LIB_ROOT)/qt/$(QT_VERSION)/usr/lib
 export QT_INC_PATH            := $(OSS_LIB_ROOT)/qt/$(QT_VERSION)/usr/include
 
 # utli-linux
 export UTLILINUX_LIB_PATH     := $(OSS_LIB_ROOT)/util-linux/$(UTILLINUX_VERSION)/usr/lib
 export UTLILINUX_INC_PATH     := $(OSS_LIB_ROOT)/util-linux/$(UTILLINUX_VERSION)/usr/include 
 
 #zlib
 export ZLIB_LIB_PATH          := $(OSS_LIB_ROOT)/zlib/$(ZLIB_VERSION)/lib
 export ZLIB_INC_PATH          := $(OSS_LIB_ROOT)/zlib/$(ZLIB_VERSION)/include
 
 # alsa    
 export ALSA_INC_PATH           := $(OSS_LIB_ROOT)/alsa/$(ALSA_VERSION)/include
 export ALSA_LIB_PATH           := $(OSS_LIB_ROOT)/alsa/$(ALSA_VERSION)/lib

 # DBus
 export DBUS_LIB_PATH           := $(OSS_LIB_ROOT)/dbus/$(DBUS_VERSION)/lib
 export DBUS_INC_PATH           := $(OSS_LIB_ROOT)/dbus/$(DBUS_VERSION)/include
 export DBUS_ARCH_DEPS_INC_PATH := $(OSS_LIB_ROOT)/dbus/$(DBUS_VERSION)/lib/dbus-$(DBUS_LIB_VERSION)/include 
############################Open src lib config end##########################

#
# Define third_party root path
#
export THIRDPARTY_ROOT       := $(DA_ROOT)/third_party
export THIRDPARTY_SRC_ROOT   := $(THIRDPARTY_ROOT)/source
export THIRDPARTY_LIB_ROOT   := $(THIRDPARTY_ROOT)/$(LIB_SUB_ROOT)
export GST_ROOT              ?= $(THIRDPARTY_SRC_ROOT)/GStreamer
export GST_WORKING_DIR       ?= $(GST_ROOT)


ifndef OBJECT_TYPE
  ifndef BUILD_CFG
	  export OBJECT_TYPE := rel
  else
	  export OBJECT_TYPE := $(BUILD_CFG)
  endif
endif

#
# Define out root path
#
ifeq "$(CUSTOMER)" ""
  export TARGET_OUT := $(MODEL_NAME)
else
  export TARGET_OUT := $(CUSTOMER)/$(MODEL_NAME)
endif

#export METADATA_OUTPUT      := $(DA_ROOT)/out/$(TARGET_OUT)
#export METADATA_OUTPUT_ROOT := $(DA_ROOT)/out/$(TARGET_OUT)/$(OBJECT_TYPE)
#export OBJ_ROOT             := $(DA_ROOT)/out/$(TARGET_OUT)/$(OBJECT_TYPE)
#export ROOTFS_OUT           := $(OBJ_ROOT)/root
export UBOOT_OBJ_OUT        := $(OBJ_ROOT)/uboot
export KERNEL_OBJ_ROOT      := $(OBJ_ROOT)/$(KERNEL_VER)

ifeq "$(LOGDIR)" ""
  export LOGDIR               := $(DA_TOP)/build/log
endif


export OSS_OUTPUT := $(OBJ_ROOT)/oss

#
#Define header file dir
#


#
#Define image out root
#


export OUTPUT_SYMBOL_ROOT := $(DA_ROOT)/out_symbol/$(TARGET_OUT)/$(OBJECT_TYPE)




