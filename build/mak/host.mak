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
# $RCSfile: host.mak,v $
# $Revision: #8 $
# $Date: 2015/07/17 $
# $Author: jianghong.lin $
# $MD5HEX:   $
#
# Description:
#        
#############################################################################

export SHELL=/bin/bash

#define name for linux solution source code root directory, this is a configuration itme
ifeq "$(SRC_ROOT_DIR)" ""
  export SRC_ROOT_DIR := atc_linux
endif

ifneq "$(DA_TOP)" ""
  DA_TOP := $(word 1, $(subst /$(SRC_ROOT_DIR)/,/$(SRC_ROOT_DIR) /, $(shell bash -c pwd -L)))
endif

ifndef BUILD_TOOLDIR
  export BUILD_TOOLDIR := $(DA_TOP)/build/tools
  #add mkimage tool dir into system PATH environment variable
  export PATH := $(BUILD_TOOLDIR): $(PATH)
endif


#
# Set the Use Script
#

RM      = /bin/rm
RM_FLAG = -rf

RMDIR   = /bin/rmdir
RMDIR_FLAG = --ignore-fail-on-non-empty -p

LN      = /bin/ln
LN_FLAG = -sf

CP      = /bin/cp
CP_FLAG = -rf
RLS_CP_FLAG = -au


MV      = /bin/mv
MV_FLAG =

ifneq "$(BUILD_TOOLDIR)" ""
  export FCLEAN = $(BUILD_TOOLDIR)/fast_clean/fclean.sh
else
  export FCLEAN = $(DA_TOP)/build/tools/fast_clean/fclean.sh
endif
ifndef FCLEAN
  export FCLEAN = $(DA_TOP)/build/tools/fast_clean/fclean.sh
endif

empty :=
space := $(empty) $(empty)

BUILD_QUEUE :=
Q1:="$(shell which androiq)"
Q2:="$(shell which newtesq)"
ifneq "$(Q2)" ""
  BUILD_QUEUE := newtestq
endif
ifeq "$(BUILD_QUEUE)" ""
  ifneq "$(Q1)" ""
    BUILD_QUEUE := androidq
  endif
endif  
BSUB := $(shell which bsub)
ifeq ($(BSUB),$(empty))
  BUILD_QUEUE := $(empty)
endif

export BUILD_QUEUE

#
# Make command and options
#
MAKE      =  make SHELL=$(SHELL)
MAKE_FLAG =
export JOBS  ?= $(shell cat /proc/cpuinfo |grep processor|wc -l)
FMAKE     = $(MAKE) -j $(JOBS)

SUPPORT_MAKE_VERSION := 3.81
MAKE_VERSION := $(shell make -v|grep 'GNU Make'|awk '{print $$3}')

# check for broken versions of make
ifeq (0,$(shell expr $(MAKE_VERSION) \>= 3.81))
$(warning ********************************************************************************)
$(warning *  You are using version $(MAKE_VERSION) of make.)
$(warning *  You must upgrade to version $(SUPPORT_MAKE_VERSION) or greater.)
$(warning ********************************************************************************)
$(error stopping)
endif


#
# Create directory command and options
#
MKDIR      = mkdir
MKDIR_FLAG = -p

#
# tar command and options
#
TAR        = tar
TAR_FLAG   = zcf
UNTAR_FLAG = zxf
UNTAR_BZ_FLAG = jxf


#
# gzip command and options
#
GZIP        = /usr/bin/gzip


#
# find command and options
#
FIND        = /usr/bin/find

#need any customer release tool ??

#
# Date command and options
#
DATE = date
DATE_FLAG = +%Y%m%d_%H%M%S
SDATE_FLAG = +%s

#
# file/directory change mode command
#
CHMOD      = /bin/chmod

#we need lockfile ???
LOCKFILE = lockfile -1 -l 30

#
# The shell define
#
export SHELL=/bin/bash

#
# mtk tool chain
#
export GENEXT2FS       = $(BUILD_TOOLDIR)/genext2fs
export MKFSEXT4        = $(BUILD_TOOLDIR)/mkfs.ext4
export MKIMAGE         = $(BUILD_TOOLDIR)/mkimage
export MKUBIFS         = $(BUILD_TOOLDIR)/mkfs.ubi.new
export MKE2FS          = $(BUILD_TOOLDIR)/mke2fs
#need export libtool ??

export MTK_MKIMAGE := $(MKIMAGE)
  
#
# The following functions are used to check on files and execute
# a set of function.
#

if_file_notexist_w_sym_fct   = if [ -e $(1) -o -h $(1) ]; then if [ ! -e $(2) -a ! -h $(2) ]; then $(LN) $(LN_FLAG) $(1) $(2); if [ "$(SYMLINK_LOG)" != "" ]; then echo  $(2) >> $(SYMLINK_LOG); fi ; fi ; fi

#
# The following functions are used to check on files and execute
# a set of function.
#
if_file_exist_fct     = @if [ -e $(1) ]; then $(2) fi
if_file_not_exist_fct = @if [ ! -e $(1) ]; then $(2) fi


#
# The following functions are used to check on directories and execute
# a set of function.
#
if_dir_exist_fct     = @if [ -d $(1) ]; then $(2) fi
if_dir_not_exist_fct = @if [ ! -d $(1) ]; then $(2) fi


#
# The following functions are used to calculate the execute time of
# a set of function.
#
cal_exec_time_fct     = echo BUILD_$(1)_END_TIME=`$(DATE) $(DATE_FLAG)`;\
                        if [ -e .build_start_time_$(1) ]; then expr `$(DATE) $(SDATE_FLAG)` - `cat .build_start_time_$(1)` > .build_time_$(1); \
                                              echo BUILD_$(1)_EXEC_TIME:`cat  .build_time_$(1)` s; \
                                              rm -rf .build_start_time_$(1); fi
                        
                                              
set_start_time_fct    = $(DATE) $(SDATE_FLAG) > .build_start_time_$(1); \
                        echo BUILD_$(1)_START_TIME=`$(DATE) $(DATE_FLAG)`

export CHK_ERR_WARN_SCRIPT_ROOT  = $(TOOL_ROOT)/pbuild
export CHK_ERR_WARN_SCRIPT = $(CHK_ERR_WARN_SCRIPT_ROOT)/chk_warn_err.sh 
export ERROR_LOG    = $(THIS_ROOT)/build_fail.log
export BRANCH_NAME  = $(word 2,$(BUILD_NAME))
export FIND_AUTHOR ?= false
export CHECK_WARNING_BUILD ?= false

ifeq "$(RLS_CUSTOM_BUILD)" "true"
export FIND_AUTHOR = false
endif
ifeq "$(CHECK_WARNING_BUILD)" "true"
export FIND_AUTHOR ?= true
endif

export CHECK_WARNING ?= false

chk_err_fct =   sed -e '/\( [Ee]rror[ :]\|first defined here\|multiple definition of\)/!d' $(1) > $(1).tmp.txt; \
				if [ -s $(1).tmp.txt ]; then	\
					/bin/echo -e '\n\n=========================== BUILD_ERROR_MESSAGES ==========================='; \
					cat $(1).tmp.txt;	\
					rm -f $(1) $(1).tmp.txt;	\
					exit 1;	\
				fi; \
				rm -f $(1) $(1).tmp.txt
          
chk_err_warn_fct     = if [ -e $(1) ]; then /bin/bash $(CHK_ERR_WARN_SCRIPT) $(CHK_ERR_WARN_SCRIPT_ROOT) $(BRANCH_NAME) $(1) $(ERROR_LOG) $(FIND_AUTHOR) $(CHECK_WARNING);  if [ -e $(ERROR_LOG) ]; then  exit 1; else $(call chk_err_fct, $(1));	 fi ; fi
                                              
define condinc
    $(eval 
        ifeq (, $(fiter %$(notdir $1), $(MAKEFILE_LIST)))
          include $1
        endif
    )
endef

define uniq =
  $(eval seen :=)
  $(foreach _,$1,$(if $(filter $_,${seen}),,$(eval seen += $_)))
  ${seen}
endef

