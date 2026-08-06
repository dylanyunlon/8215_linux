################################################################################
#
# bluecommom
#
################################################################################

ifneq ($(ATC_BT_CHIP),)
    BLUECOMMON_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/bluetooth/common
    BLUECOMMON_PREBUILD_PATH = $(TOPDIR)/../prebuild/connectivity/bluetooth/common
    BLUECOMMON_SITE = $(shell if [ -d $(BLUECOMMON_CODE_PATH) ]; then echo $(BLUECOMMON_CODE_PATH); else echo $(BLUECOMMON_PREBUILD_PATH); fi)

    BLUECOMMON_SITE_METHOD = local
    BLUECOMMON_ALWAYS_BUILD = YES
    BLUECOMMON_INSTALL_STAGING = YES
    BLUECOMMON_DEPENDENCIES += -lpthread
    BLUECOMMON_DEPENDENCIES += protobuf
    BLUECOMMON_MAKE_ARGS = \
        TOPDIR=$(TOPDIR)/.. KERNEL_TOPDIR=$(TOPDIR)/../source/kernel/kernel-3.18 \
        CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-g++ \
        SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot

    define BLUECOMMON_BUILD_CMDS
        @if [ -d $(BLUECOMMON_CODE_PATH) ]; then \
            echo "build bluecommon"; \
            $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(BLUECOMMON_MAKE_ARGS) all; \
        else \
            echo "bluecommon Prebuild"; \
        fi
    endef


    define BLUECOMMON_INSTALL_STAGING_CMDS
        $(INSTALL) -m 0755 -D $(@D)/libbluetoothcommon.so $(STAGING_DIR)/usr/lib
    endef

    define BLUECOMMON_INSTALL_TARGET_CMDS
        $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libbluetoothcommon.so $(TARGET_DIR)/usr/lib
    endef

    $(eval $(generic-package))
endif
