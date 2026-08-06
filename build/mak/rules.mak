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

include $(DA_TOP)/$(TOP_MAKE)/host.mak
include $(PROJECT_DIR)/oss_version.mak
#.PHONY: all init rootfs_prepare uboot kernel arm2 oss rootfs sys_config debug clean $(sub-function) misc sdk 
.PHONY: all init rootfs_prepare uboot kernel arm2 rootfs  debug clean $(sub-function) misc sdk 

init: output_prepare rootfs_prepare log_init
	@echo "init"
	@$(CHMOD) 755 $(TOOL_ROOT)/genext2fs
	@$(CHMOD) 755 $(TOOL_ROOT)/mkimage
	@$(CHMOD) 755 $(FCLEAN)
	@if [ ! -d $(OBJ_ROOT)/root ]; then \
		$(MKDIR) $(MKDIR_FLAG) $(OBJ_ROOT)/root ; \
	fi
	@if [ ! -d $(IMG_REL) ]; then \
		$(MKDIR) $(MKDIR_FLAG) $(IMG_REL); \
	fi

pre_sysroot_init:
	#@ time $(BUILD_QUEUE) sh $(DA_TOP)/build/tools/tasks/do_pre_sysroot.sh
	@echo " YP SDK: $(YP_SDK_VER); Local SDK : $(LOCAL_VER)"
ifneq ($(YP_SDK_VER),$(LOCAL_VER))
	@echo " update Yocto SDK "
	@rm -rf $(DA_SYSROOT) && mkdir -p $(DA_SYSROOT)
	@time rsync -rlD --exclude-from=$(DA_TOP)/build/exclude.list $(SDKTARGETSYSROOT)/* $(DA_SYSROOT)
	@echo "Timestamp: $(YP_SDK_VER)" > $(LOCALSDKVER)
endif

output_prepare:
	@echo " mkdir : $(OBJ_ROOT) OBJECT_TYPE:$(OBJECT_TYPE)"
	@$(MKDIR) $(MKDIR_FLAG) $(METADATA_OUTPUT_ROOT)
	@$(MKDIR) $(MKDIR_FLAG) $(OBJ_ROOT)
	@$(MKDIR) $(MKDIR_FLAG) $(KERNEL_OBJ_ROOT)
	@$(MKDIR) $(MKDIR_FLAG) $(OUTPUT_SYMBOL_ROOT)
	@$(MKDIR) $(MKDIR_FLAG) $(UBOOT_OBJ_OUT)
	

rootfs_prepare :
	@echo " Prepare rootfs dir"
ifneq "$(ROOTFS_VENDOR)" ""
	@if [ ! -d $(OBJ_ROOT)/root/rootfs_$(ROOTFS_VENDOR) ]; then \
		$(MKDIR) $(MKDIR_FLAG) $(OBJ_ROOT)/root/rootfs_$(ROOTFS_VENDOR) ; \
	fi
endif
	@$(MKDIR) $(MKDIR_FLAG) $(ROOTFS_OUT)
	@if [ ! -d $(ROOTFS_OUT)/etc ]; then \
	  echo " Untar $(PROJECT_DIR)/root_$(INIT_TYPE).tar.gz"; \
	  $(TAR) -zxf $(PROJECT_DIR)/root_$(INIT_TYPE).tar.gz -C $(ROOTFS_OUT)/; \
	else \
	  echo " rootfs folder existing !!!"; \
	fi

log_init:
	@$(MKDIR) $(MKDIR_FLAG) $(LOGDIR)


autotool:
#	@$(MAKE) -C $(OSS_TOP)/source/autoconf-native
#	@$(MAKE) -C $(OSS_TOP)/source/automake-native

kernel:
	@$(call set_start_time_fct,$@);
	@CROSS_COMPILE=$(CROSS_COMPILE_PREFIX) \
	$(BUILD_QUEUE) $(MAKE) -f $(DA_TOP)/$(TOP_MAKE)/kernel_uboot.mak kernel; \
	if [ $$? != 0 ]; then \
	  echo -e "\033[40;31m kernle build end \033[0m"; \
	  exit 1; \
	fi
	@$(call cal_exec_time_fct,$@)


uboot:
	@$(call set_start_time_fct,$@);
	@ if [ -f $(UBOOT_SRC)/Makefile ]; then $(BUILD_QUEUE) $(FMAKE) -C $(UBOOT_SRC) ; fi
	@$(call cal_exec_time_fct,$@);
	
driver:
	@$(call set_start_time_fct,$@);
	@echo " build driver"
	@$(call cal_exec_time_fct,$@);
		
	
sys_config:
	@echo "config setting for : " $(CUST_DIR)
	$(CP) $(RLS_FLAG) -r $(CUST_DIR)/root/etc/* $(ROOTFS_OUT)/etc/

rootfs: 
	@ echo -e "\033[44;32m gen rootfs start \033[0m"
	@ time $(BUILD_QUEUE) sh $(DA_TOP)/build/gen_rootfs.sh	
	@ echo -e "\033[44;32m gen rootfs end \033[0m"
oss : autotool
	@ echo -e "\033[44;32m oss build start \033[0m"
#	@ time $(BUILD_QUEUE) $(MAKE) -C $(OSS_TOP)/source 
	@ echo -e "\033[44;32m oss build end \033[0m"
	
show_report:
	@if [ -e $(THIS_ROOT)/make.log ]; then \
		$(CP) $(CP_FLAG) $(THIS_ROOT)/make.log $(OUTPUT_ROOT)/; \
	fi
	@$(call if_file_exist_fct, $(THIS_ROOT)/phase2_warning.result.log, /bin/echo -e "\n=========== Build Phase2 Warning Rst ===========\n";sort -u $(THIS_ROOT)/phase2_warning.result.log;/bin/echo -e "\n\n";)
	
	
warn_chk:
	@$(RM) $(RM_FLAG) $(THIS_ROOT)/warning.txt
	@echo Warn check result : $(THIS_ROOT)/warning.txt

debug:
	@echo ";build info" > $(LINUX_ROOT)/out/debug.cmm	
	@echo \&arm_smp='"$(ENABLE_SMP_IC)"' >> $(LINUX_ROOT)/out/debug.cmm
	@echo \&arm_ca9='"$(ENABLE_CA9)"' >> $(LINUX_ROOT)/out/debug.cmm
	@echo \&target_ic='"$(TARGET_IC)"' >> $(LINUX_ROOT)/out/debug.cmm

$(sub-function):
	@echo -e "build $@ start"
	@if [ -d $@ ]; then    \
	    $(MAKE) -C $@ ; fi
	@echo -e "build $@ end"

application:
	@echo "build application"
	$(MAKE) -C $@
	$(MAKE) -C $@ install
	@echo "build application end"

arm2:
	@echo "arm2 build start"
	CROSS_COMPILE="armv6z-mediatek-linux-gnueabi-" $(MAKE) -C $@
	@cp -f $@/target/arm2 $(IMG_REL)/
	@cp -f $@/target/arm2.bin $(IMG_REL)/
	@echo "arm2 build end"

sdk: $(sub-function)
	@echo -e "collect sdk start"
	@ python $(DA_TOP)/build/sdk.py
	@echo -e "collect sdk end"

#include $(DA_TOP)/build/mak/oss_task.mak	
include $(DA_TOP)/$(TOP_MAKE)/prechk_rules.mak
include $(DA_TOP)/$(TOP_MAKE)/help_rules.mak
include $(DA_TOP)/$(TOP_MAKE)/clean_rules.mak
#include $(OSS_LIB_ROOT)/mak/Makefile

ifeq "$(DEBUG_MAKE)" "true"
$(__BREAKPOINT)
endif
