# File existed?

LOCAL_PATH := $(CURDIR)
LOCAL_TOP  := $(LOCAL_PATH)/../
export CROSS_COMPILE := arm-buildroot-linux-gnueabi-
PATH := ${PATH}:$(HOST_DIR)/bin
ifeq "$(DEVICE)" ""
DEVICE := ac83xx_evb
endif

ifeq "$(OS)" ""
OS := android
endif

$(warning $(DEVICE))
$(warning $(OS))
PLATFORM := $(word 1, $(subst _, ,$(DEVICE)))

define directory_exist
$(shell if [ -d "$1" ]; then \
           echo $1 ; \
        fi)
endef


$(warning $(DEVICE))
PLATFORM := $(word 1, $(subst _, ,$(DEVICE)))

SUBDIR := $(call directory_exist,$(LOCAL_TOP)/aud_protect) \
          $(call directory_exist,$(LOCAL_TOP)/atc_ftl/atc-ftl) \
          $(call directory_exist,$(LOCAL_TOP)/nosuchdir) 

ifeq "$(OS)" "android"
SUBDIR += $(call directory_exist,$(LOCAL_TOP)/../screen-hvdetect)
endif
$(info "HOST_DIR:$(HOST_DIR) PATH:$(PATH)")
$(warning "Platform is $(PLATFORM), OS is $(OS)")
all:
	@for d in $(SUBDIR); do make -C $$d PLATFORM=$(PLATFORM) OS=$(OS); done 
