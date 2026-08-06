################################################################################
#
# bluestack
#
################################################################################

ifneq ($(ATC_BT_CHIP),)
BLUESTACK_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/bluetooth/bluestack
BLUESTACK_PREBUILD_PATH = $(TOPDIR)/../prebuild/connectivity/bluetooth/bluestack
BLUESTACK_SITE = $(shell if [ -d $(BLUESTACK_CODE_PATH) ]; then echo $(BLUESTACK_CODE_PATH); else echo $(BLUESTACK_PREBUILD_PATH); fi)

PKGDIR := $(pkgdir)
PKGNAME := $(call UPPERCASE,$(pkgname))

BLUESTACK_SITE_METHOD = local
BLUESTACK_ALWAYS_BUILD = YES
BLUESTACK_INSTALL_STAGING = YES
BLUESTACK_DEPENDENCIES += libmetazone -lpthread -lz -lrt -ldl
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

    define BLUESTACK_BUILD_CMDS
            if [ -d $(BLUESTACK_CODE_PATH) ]; then \
                echo "Build Bluestack"; \
                $($(PKG)_MAKE_ENV) TARGET_TOP=$($(PKG)_DIR) \
                $(MAKE) $($(PKG)_MAKE_OPTS) -C $($(PKG)_SITE); \
            else \
                echo "Prebuild Bluestack"; \
            fi
    endef

    define BLUESTACK_INSTALL_STAGING_CMDS
        $(INSTALL) -m 0755 -D $(@D)/lib/bluetooth.default.so $(STAGING_DIR)/usr/lib
        $(INSTALL) -m 0755 -D $(@D)/lib/libbt-vendor.so $(STAGING_DIR)/usr/lib
        $(INSTALL) -d $(TARGET_DIR)/../data/misc/bluedroid
    endef

    define BLUESTACK_INSTALL_TARGET_CMDS
        $(INSTALL) -m 0755 -D $(@D)/lib/bluetooth.default.so $(TARGET_DIR)/usr/lib
        $(INSTALL) -m 0755 -D $(@D)/lib/libbt-vendor.so $(TARGET_DIR)/usr/lib
    endef

$(eval $(generic-package))
endif
