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
# $RCSfile: Makefile,v $
# $Revision: #34 $
# $Date: 2016/01/19 $
# $Author: ke.xu $
# $MD5HEX:  $
#
# Description:
#         Makefile to build a Linux demo board build. The following
#         targets are supported:
#
#         The following commands are supported:
#
#############################################################################
ifndef THIS_ROOT
	THIS_ROOT=$(shell bash -c pwd -L)
endif
TOP_DIR := atc_linux
ifeq "$(DA_TOP)" ""
	DA_TOP := $(word 1, $(subst /$(TOP_DIR)/,/$(TOP_DIR) /, $(THIS_ROOT)))
endif


.PHONY: all bsp clean 
all:

ifeq "$(TOP_MAKE)" ""
  export TOP_MAKE := build/mak
endif
ifndef
  export TOP_MAKE := build/mak
endif

#
#common include
#
include $(DA_TOP)/$(TOP_MAKE)/include.mak


ifeq "$(CHECK_WARNING_BUILD)" "true"
BUILD_STEP_N += warn_chk
endif

SAVE_LOG = 2>&1 | tee $(DA_TOP)/build/log/build_$@.log

sub-function := avin multimedia graphics connectivity app misc dvd
#trustzone
.PHONY: $(sub-function)  application
clean-sub-function := $(addprefix clean_,$(sub-function))
all: 
	$(MAKE) autotool bsp rootfs 
	PartitionUtility=`perl $(DA_TOP)/build/tools/PartitionUtility/PartitionUtility.pl`;
	PartitionUtility_Nand_Ext4=`perl $(DA_TOP)/build/tools/PartitionUtility/PartitionUtility_Nand_Ext4.pl`;
#	echo $(PartitionUtility)
	@cp -f $(PROJECT_DIR)/target/* $(IMG_REL)/
	@cp -f $(DA_TOP)/build/tools/scatter.mmcboot.ext4.xml $(IMG_REL)/
#	for cebuild to upload to p4
	@if [ -e $(DA_TOP)/trustzone/target/tz.bin ]; then cp -f $(DA_TOP)/trustzone/target/tz.bin $(DA_TOP)/project/evb_8317/target/; fi
	
#	cp tz.bin to image_release
	@if [ -e $(DA_TOP)/trustzone/target/tz.bin ]; then cp -f $(DA_TOP)/trustzone/target/tz.bin $(IMG_REL)/ ; fi

#	$(MAKE) -j $(JOBS) $(BUILD_STEP_1) 2>&1 | tee make_step1.log
#	@$(call chk_err_warn_fct, make_step1.log);			
#	$(MAKE)  $(BUILD_STEP_2) 2>&1 | tee make_step2.log
#	@$(call chk_err_warn_fct, make_step2.log);
#	$(MAKE) $(BUILD_STEP_N)

export bsp-dependent :=  kernel misc graphics  multimedia avin uboot arm2 connectivity dvd 
clean-bsp-dependent := $(addprefix clean_,$(bsp-dependent))
bsp: $(bsp-dependent)  application
	@ echo "BSP Build Done"

export CLEAN_TARGET = clean_metadata \
                      clean_init \
                      clean_uboot \
                      clean_kernel \
                      clean_driver \
                      clean_rootfs \
                      clean_oss \
                      $(clean-sub-function) \
	              clean_misc \
                      clean_sdk \
                      clean_log 

clean:  $(CLEAN_TARGET)
	@ echo -e "\033[44;32m CLEAN END \033[0m"  
ifeq "$(CLEAN_ALL_CFG_OBJ)" "true"
	@$(FCLEAN) $(OUTPUT_ROOT)
	@$(FCLEAN) $(OUTPUT_SYMBOL_ROOT)
endif


include $(DA_TOP)/$(TOP_MAKE)/rules.mak



