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
# $RCSfile: toolchain.mak,v $
# $Revision: #4 $
# $Date: 2016/02/15 $
# $Author: jianghong.lin $
# $MD5HEX:   $
#
# Description:
#        
#############################################################################
ifndef THIS_ROOT
    THIS_ROOT := $(shell bash -c pwd -L)
endif

ifeq "$(SET_TOOLCHAIN)" "0"
    unexport CROSS_COMPILE
endif

TOP_DIR := atc_linux
ifndef LINUX_ROOT
     LINUX_ROOT := $(word 1, $(subst /$(TOP_DIR)/,/$(TOP_DIR) /, $(THIS_ROOT)))
endif

ifneq "$(SET_TOOLCHAIN)" "1"
  export SET_TOOLCHAIN := 1

  #
  # Set Toolchain
  #

  #TOOL_CHAIN ?= 4.5.1
  TOOL_CHAIN ?= 4.8.2


  #
  # Set Cross Compile Path
  #

  ENABLE_VFP ?= false
  ENABLE_CA9 ?= false


  ifeq "$(ENABLE_VFP)" "true"
    ifeq "$(ENABLE_CA9)" "true"
      $(error ENABLE_CA9 and ENABLE_VFP can not be enabled together)
    endif
  endif


  ifeq "$(TOOL_CHAIN)" "4.5.1"
	  export TOOL_CHAIN_ROOT  = /mtkoss/gnuarm/vfp_4.5.1_2.6.27_cortex-a9-rhel4/i686	 
  else ifeq "$(TOOL_CHAIN)" "4.8.2"
    export TOOL_CHAIN_ROOT = /mtkoss/gnuarm/vfp_4.8.2_2.6.35_cortex-a9-ubuntu/i686
  else
	  export TOOL_CHAIN_ROOT  = /usr/local/mtk-toolchain
  endif


  VFP_SUFFIX =


  ifeq "$(TOOL_CHAIN)" "4.5.1"
    ifeq "$(ENABLE_VFP)" "true"
      export	VFP_SUFFIX = _vfp
    else ifeq "$(ENABLE_CA9)" "true"
      export VFP_SUFFIX = _vfp_ca9
    endif
  else
    export VFP_SUFFIX = _vfp
  endif


  TOOL_CHAIN_BIN_PATH				:= $(TOOL_CHAIN_ROOT)/bin

  ifneq "$(TOOL_CHAIN_ROOT)" ""
    export CROSS_TOOLCHAIN_SYSROOT := $(TOOL_CHAIN_ROOT)/sysroot
    export CROSS_TOOLCHAIN_SYSROOT_LIB := $(TOOL_CHAIN_ROOT)/sysroot/lib
  endif

  ifndef CROSS_COMPILE
    ifeq "$(TOOL_CHAIN)" "4.5.1"   
      export CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/armv7a-mediatek451_001_vfp-linux-gnueabi-
      export OSS_CC_PREFIX := armv7a-mediatek451_001_vfp-linux-gnueabi
    endif
    ifeq "$(TOOL_CHAIN)" "4.8.2"
      export CROSS_COMPILE := $(TOOL_CHAIN_BIN_PATH)/armv7a-mediatek482_001_vfp-linux-gnueabi-
      export OSS_CC_PREFIX := armv7a-mediatek482_001_vfp-linux-gnueabi
    endif
  endif  

  ifndef CROSS_COMPILE
    $(error Not set CROSS_COMPILE, Your TOOL_CHAIN set to $(TOOL_CHAIN))
  endif


  export STRIP  := $(CROSS_COMPILE)strip
  export LD     := $(CROSS_COMPILE)ld
  export AR     := $(CROSS_COMPILE)ar
  export RANLIB := $(CROSS_COMPILE)ranlib

  # CCACHE setting
  USE_CCACHE ?= 0
  ifeq "$(USE_CCACHE)" "1"
    CCACHE_BIN := ccache
    CCACHE_BIN_CHK := $(shell which $(CCACHE_BIN) 2>&1 |grep -c 'no $(CCACHE_BIN) in')

    # CCACHE_BIN_CHK == 1 as ccache not found 
    ifeq "$(CCACHE_BIN_CHK)" "1"
      USE_CCACHE := 0
    endif
  endif


  BUILD_CROSS_COMPILE := $(CROSS_COMPILE)


  export CC    := $(BUILD_CROSS_COMPILE)gcc
  export CXX   := $(BUILD_CROSS_COMPILE)g++
  export ASM   := $(CC)


 #include $(DA_TOP)/oss/source/mak/oss_version.mak
endif


CROSS_COMPILE := $(shell echo $(CROSS_COMPILE)  | sed 's/\/\//\//g')
