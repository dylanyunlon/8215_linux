################################################################################
#
# blueserver
#
################################################################################
ifneq ($(ATC_BT_CHIP),)
    BLUESERVER_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/bluetooth/server
    CONFIG_SRC_PATH = ${TOPDIR}/../source/packages/connectivity/bluetooth/configs
    BLUESERVER_PREBUILD_PATH = $(TOPDIR)/../prebuild/connectivity/bluetooth/server
    BLUESERVER_SITE = $(shell if [ -d $(BLUESERVER_CODE_PATH) ]; then echo $(BLUESERVER_CODE_PATH); else echo $(BLUESERVER_PREBUILD_PATH); fi)

    PKGDIR := $(pkgdir)
    PKGNAME := $(call UPPERCASE,$(pkgname))

    BLUESERVER_SITE_METHOD = local
    BLUESERVER_ALWAYS_BUILD = YES
    BLUESERVER_INSTALL_STAGING = YES
    BLUESERVER_DEPENDENCIES += -lpthread universal_utils tinyxml2 libnl
    $(PKGNAME)_MAKE_ENV := \
        $(TARGET_CONFIGURE_OPTS) \
        ATC_BUILD_DIR=$(PKGDIR)/mak \
        RECURSION_MAK=$(PKGDIR)/mak/recursion_build.mak \
        BUILD_EXECUTABLE=$(PKGDIR)/mak/build_exe.mak \
        BUILD_SHARED_LIBRARY=$(PKGDIR)/mak/build_share_lib.mak \
        BUILD_STATIC_LIBRARY=$(PKGDIR)/mak/build_static_lib.mak \
        TARGET_VENDOR=$(TARGET_VENDOR) \
        DA_TOP=$(TOPDIR)/../source \
        DA_SYSROOT=$(STAGING_DIR) \

    $(PKGNAME)_MAKE_OPTS = \
        CROSS_COMPILE=$(TARGET_CROSS)

    define BLUESERVER_BUILD_CMDS
            if [ -d $(BLUESERVER_CODE_PATH) ]; then \
            echo "build BLUESERVER"; \
            $($(PKG)_MAKE_ENV) TARGET_TOP=$($(PKG)_DIR) \
                $(MAKE) $($(PKG)_MAKE_OPTS) -C $($(PKG)_SITE);\
            else \
            echo "BLUESERVER Prebuild"; \
            fi
    endef


    define BLUESERVER_INSTALL_STAGING_CMDS
        $(INSTALL) -m 0755 -D $(@D)/bin/bluetoothmanagerservice $(STAGING_DIR)/usr/bin
        $(INSTALL) -m 0755 -D $(@D)/bin/bluetoothservice $(STAGING_DIR)/usr/bin
        $(INSTALL) -D -m 0644 ${CONFIG_SRC_PATH}/atcprotocol.xml $(TARGET_DIR)/../data/misc/bluetooth/atcprotocol.xml
        $(INSTALL) -D -m 0644 ${CONFIG_SRC_PATH}/bluetoothlocaldevice.xml $(TARGET_DIR)/../data/misc/bluetooth/bluetoothlocaldevice.xml
        $(INSTALL) -D -m 0644 ${CONFIG_SRC_PATH}/bluetooth_snoop_log.sh $(TARGET_DIR)/../data/misc/bluetooth//bluetooth_snoop_log.sh
        $(INSTALL) -D -m 0644 ${CONFIG_SRC_PATH}/bt_stack.conf $(TARGET_DIR)/etc/bluetooth/bt_stack.conf
        $(INSTALL) -D -m 0644 ${CONFIG_SRC_PATH}/bt_did.conf $(TARGET_DIR)/etc/bluetooth/bt_did.conf
        $(INSTALL) -D -m 0644 ${CONFIG_SRC_PATH}/defaultCoverArt.jpeg $(TARGET_DIR)/etc/bluetooth/defaultCoverArt.jpeg
        $(INSTALL) -D -m 0755 $($(PKG)_PKGDIR)/bluetoothmanagerservice.sh $(TARGET_DIR)/etc/init.d/S83bluetoothmanagerservice
        ln -sf /etc/init.d/S83bluetoothmanagerservice $(TARGET_DIR)/usr/bin/bluetoothmanagerservice.sh
    endef

    define BLUESERVER_INSTALL_TARGET_CMDS
        $(INSTALL) -m 0755 -D $(@D)/bin/bluetoothmanagerservice $(TARGET_DIR)/usr/bin
        $(INSTALL) -m 0755 -D $(@D)/bin/bluetoothservice $(TARGET_DIR)/usr/bin
    endef

    $(eval $(generic-package))
endif
