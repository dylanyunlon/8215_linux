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
# $Revision: #8 $
# $Date: 2015/09/06 $
# $Author: ke.xu $
# $CCRevision: $
# $MD5HEX:  $
#
# Description:
#         Makefile to build a Linux demo board build. The following
#         targets are supported:
#
#         The following commands are supported:
#
#############################################################################

include $(DA_TOP)/$(TOP_MAKE)/host.mak

.PHONY:  $(CLEAN_TARGET)

clean_kernel:
	@ echo -e "\033[44;32m kernle clean START \033[0m"
	@cd $(KERNELDIR) && $(MAKE) clean && $(MAKE) mrproper && $(MAKE) ARCH=arm ac83xx_defconfig ##&& $(MAKE) modules_prepare
	@ echo -e "\033[44;32m kernle clean END \033[0m"

clean_uboot:
	@echo "clean uboot" 
	
	
clean_bsp: $(clean-bsp-dependent) 
	@ echo "$(clean-bsp-dependent) " 
	@ echo "$(clean-sub-function)"
	@echo "clean bsp done " 	


clean_metadata:
	@echo "clean metadata" 

clean_output :
	@echo "clean output" 
	@$(FCLEAN) $(OUTPUT_ROOT) $(OUTPUT_SYMBOL_ROOT)


clean_rootfs:
	@echo "clean rootfs ... ..." 
	@$(RM) -rf $(ROOTFS_OUT)
	@$(MKDIR) $(MKDIR_FLAG) $(ROOTFS_OUT)
	@if [ ! -d  $(ROOTFS_OUT)/etc ]; then \
		$(TAR) $(UNTAR_FLAG) $(PROJECT_DIR)/$(ROOTFS_NAME) -C $(OBJ_ROOT)/ ;\
	fi
	@echo "clean rootfs END"
	
clean_oss :
	@/bin/echo -e "  RM\tOSS" 
	for i in $(OSS_LIST); do \
		if [ -f $(LINUX_ROOT)/oss/source/$$i/Makefile ]; then \
			$(MAKE) -C $(LINUX_ROOT)/oss/source/$$i  BUILD_TARGET_OBJ_ROOT=$(OBJ_ROOT)/oss/$$i clean; \
		fi; \
		$(MAKE) -C $(OSS_LIB_ROOT)/$$i clean OSS_OUTPUT=$(OBJ_ROOT)/oss --no-print-directory -s; \
	done
	@$(FCLEAN) $(OBJ_ROOT)/oss
	
clean_driver :
	@ echo "clean drv" 
	@for i in $(KO_MODULES_LIST); do \
		$(MAKE) -C $(DRIVERS_DIR)/$$i clean BUILD_TARGET_OBJ_ROOT=$(OBJ_ROOT)/ko_modules; \
	done
	@$(FCLEAN) $(OBJ_ROOT)/ko_modules
	
	
clean_log:
	@ echo "clean log "
	@$(RM) $(RM_FLAG) $(THIS_ROOT)/build.*.log							
	
clean_debug:
	@ echo "clean debug" 
	@$(RM) $(RM_FLAG)  $(DTV_LINUX_ROOT)/output/debug.cmm	

$(clean-sub-function):
	@ echo "clean $@ start "
	@clean_target=`echo $@ | awk -F_ '{print $$2}'`; \
	if [ -d $$clean_target ]; then \
          $(MAKE) -C $$clean_target clean ; fi

