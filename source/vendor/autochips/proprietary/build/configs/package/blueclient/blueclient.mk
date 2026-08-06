################################################################################
#
# blueclient
#
################################################################################
ifneq ($(ATC_BT_CHIP),)
    BLUECLIENT_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/bluetooth/client
    BLUECLIENT_PREBUILD_PATH = $(TOPDIR)/../prebuild/connectivity/bluetooth/client
    BLUECLIENT_SITE = $(shell if [ -d $(BLUECLIENT_CODE_PATH) ]; then echo $(BLUECLIENT_CODE_PATH); else echo $(BLUECLIENT_PREBUILD_PATH); fi)

    BLUECLIENT_SITE_METHOD = local
    BLUECLIENT_ALWAYS_BUILD = YES
    BLUECLIENT_INSTALL_STAGING = YES
    BLUECLIENT_DEPENDENCIES += universal_utils
    BLUECLIENT_MAKE_ARGS = \
        TOPDIR=$(TOPDIR)/.. KERNEL_TOPDIR=$(TOPDIR)/../source/kernel/kernel-3.18 \
        CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-g++ \
        SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot
    BLUECLIENT_MAKE_ARGS += STATIC_LIB=

    define BLUECLIENT_BUILD_CMDS
        @if [ -d $(BLUECLIENT_CODE_PATH) ]; then \
            echo "build blueclient"; \
            $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(BLUECLIENT_MAKE_ARGS) all; \
        else \
            echo "blueclient Prebuild"; \
        fi
    endef


    define BLUECLIENT_INSTALL_STAGING_CMDS
        $(INSTALL) -m 0755 -D $(@D)/libbluetoothclient.so $(STAGING_DIR)/usr/lib
    endef

    define BLUECLIENT_INSTALL_TARGET_CMDS
        $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libbluetoothclient.so $(TARGET_DIR)/usr/lib
    endef

    $(eval $(generic-package))
endif
