################################################################################
#
## carbit
#
#################################################################################
ifeq ($(ATC_AB_PARTITION_SUPPORT),false)
ifneq ($(AC83XX_BOOT_DEVICE_SIZE),128)

CARBIT_VERSION = 1.0
CARBIT_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/carbit
CARBIT_PREBUILD_PATH = $(TOPDIR)/../prebuild/carlink/carbit
CARBIT_SITE = $(shell if [ -d $(CARBIT_CODE_PATH) ]; then echo $(CARBIT_CODE_PATH); else echo $(CARBIT_PREBUILD_PATH); fi)
CARBIT_SITE_METHOD = local
CARBIT_ALWAYS_BUILD = YES
CARBIT_INSTALL_STAGING = YES
CARBIT_DEPENDENCIES = universal_utils

CARBIT_MAKE_ARGS := \
        SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
        RECURSION_MAK=$(TOPDIR)/../build/mak/recursion_build.mak

#define CARBIT_BUILD_CMDS
#        @if [ -d $(CARBIT_CODE_PATH) ]; then \
#	        echo "build carbit"; \
#                $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(CARBIT_MAKE_ARGS) all; \
#        else \
#                echo "prebuild carbit"; \
#        fi
#endef

define CARBIT_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0644 -D $(@D)/libECSDKFramework.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/libECSDK.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/libECusb.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/libiMuxEC.so $(STAGING_DIR)/usr/lib
endef

define CARBIT_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libECSDKFramework.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libECSDK.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libECusb.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libiMuxEC.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

endif
endif
