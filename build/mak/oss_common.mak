#-----------------------------------------------------------------------------
# Copyright (c) 2015, Autochips Inc.
# All rights reserved.
#
# Unauthorized use, practice, perform, copy, distribution, reproduction,
# or disclosure of this information in whole or in part is prohibited.
#-----------------------------------------------------------------------------
# $RCSfile:  $
# $Revision:
# $Date:
# $Author: yuanfeng.hu $
# $CCRevision:  $
# $SWAuthor:  $
# $MD5HEX:  $
#
# Description:
#        Makefile for busybox
#---------------------------------------------------------------------------*/



THIS_ROOT :=$(shell pwd)
ifneq "$(DA_TOP)" ""
  OSS_ROOT :=$(DA_TOP)/oss
  BUILD_CORE := $(DA_TOP)/$(TOP_MAKE)
else
  OSS_ROOT :=$(word 1, $(subst /oss/,/oss /,$(THIS_ROOT)))
  BUILD_CORE := $(OSS_ROOT)/../$(TOP_MAKE)
endif 

MAKE := make
include $(PROJECT_DIR)/oss_version.mak

local_target := $(notdir $(THIS_ROOT))

include $(BUILD_CORE)/path.mak
include $(BUILD_CORE)/host.mak
include $(OSS_ROOT)/source/mak/target.mak

PACKAGE_NAME := $(filter $(local_target)-%,$(OSS_PACKAGES))
#target_version := $(word $(words $(subst -, ,$(PACKAGE_NAME)), $(subst -, ,$(PACKAGE_NAME)))
target_version := $(word $(words $(subst -, ,$(PACKAGE_NAME))), $(subst -, ,$(PACKAGE_NAME)))
SURDIR := $(target_version)/$(PACKAGE_NAME)
DEST_DIR=$(OSS_LIB_ROOT)/$(local_target)/$(target_version)

DEBUG := 0

.PHONY: all clean $(local_target)
all:$(local_target)

$(local_target) : $(local_target)_prepare
	@cd $(SURDIR) && $(config_cmd)
	@cd $(SURDIR) && $(make_cmd) 
	@cd $(SURDIR) && $(install_cmd)
	@ echo -e "\033[44;32m $(local_target) build end \033[0m"
	
$(local_target)_prepare	:
	@ echo -e "\033[44;32m $(local_target) build start \033[0m"
	@if [ -d $(DEST_DIR) ]; then \
		cd $(DEST_DIR) && $(RM) $(RM_FLAG); \
	else \
		$(MKDIR) $(MKDIR_FLAG) $(DEST_DIR); \
	fi

clean distclean:
	@cd $(SURDIR) &&  if [ -e Makefile ]; then \
	$(MAKE) $@; \
	fi
