
include $(ARM2TOPDIR)/build/config.mk

awtk_src := $(ARM2TOPDIR)/lib/libawtk/src/awtk-1.8/src
awtk_3rd := $(ARM2TOPDIR)/lib/libawtk/src/awtk-1.8/3rd

CUR_CSRC += ./src/main.c 
CUR_CSRC += ./src/application.c 
CUR_CSRC += ./src/pages/home_page.c 
CUR_CSRC += ./src/common/navigator.c 

COBJ     = $(subst .c,.o,$(CUR_CSRC))

ALLOBJS := $(COBJ) 

LOCAL_CFLAG += -DHAS_AWTK_CONFIG
LOCAL_CFLAG += -I$(ARM2TOPDIR)/include
LOCAL_CFLAG += -D_GLIBCXX_INCLUDE_NEXT_C_HEADERS
LOCAL_CFLAG += -D__FLT_EVAL_METHOD__=0

STATICLIBS += $(ARM2OBJDIR)/libawtk.a

# Note: For search metazone.h
# LOCAL_INCULE := -I$(ATC_KERNEL_DIR)/include/misc/atc/metazone
AWTK_DIR := $(awtk_src)/
AWTK_DIR += $(awtk_src)/ext_widgets/
AWTK_DIR += $(awtk_3rd)/
AWTK_DIR += $(awtk_3rd)/nanovg/
AWTK_DIR += $(awtk_3rd)/nanovg/base/
AWTK_DIR += $(awtk_3rd)/agge/
AWTK_DIR += $(awtk_3rd)/libunibreak/
AWTK_DIR += $(ARM2TOPDIR)/lib/libawtk/platform/8317/
LOCAL_INCULE += $(foreach dir, $(AWTK_DIR), -I$(dir))
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/include/media/atc/

#LDSCRIPT := arm2_awtk.lds
LDSCRIPT := $(ARM2BUILDDIR)/arm2_dram.lds

TARGETOBJ    := $(ARM2OBJDIR)/arm2_awtk.o
TARGETOBJDIR := $(ARM2OBJDIR)/src/obj

include $(ARM2BUILDDIR)/module_awtk.mk
