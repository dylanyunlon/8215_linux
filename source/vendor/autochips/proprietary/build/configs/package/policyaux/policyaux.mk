################################################################################
#
# audio policyaux
#
################################################################################
    ifneq ($(AC83XX_BOOT_DEVICE)-$(AC83XX_BOOT_DEVICE_SIZE), nand-128)

    POLICYAUX_VERSION = 1.0
    POLICYAUX_CODE_PATH = $(TOPDIR)/../source/packages/audio/policyaux
    POLICYAUX_SITE = $(POLICYAUX_CODE_PATH)

    PKGDIR := $(pkgdir)
    PKGNAME := $(call UPPERCASE,$(pkgname))

    POLICYAUX_SITE_METHOD = local
    POLICYAUX_ALWAYS_BUILD = YES
    POLICYAUX_INSTALL_STAGING = YES
    POLICYAUX_DEPENDENCIES += -lpthread dbus -ldl -lrt
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

    define POLICYAUX_BUILD_CMDS
            if [ -d $(POLICYAUX_CODE_PATH) ]; then \
            echo "build POLICYAUX"; \
            $($(PKG)_MAKE_ENV) TARGET_TOP=$($(PKG)_DIR) \
                $(MAKE) $($(PKG)_MAKE_OPTS) -C $($(PKG)_SITE);\
            else \
            echo "POLICYAUX Prebuild"; \
            fi
    endef


    define POLICYAUX_INSTALL_STAGING_CMDS
        $(INSTALL) -m 0755 -D $(@D)/lib/libpolicyaux.so $(STAGING_DIR)/usr/lib
    endef

    define POLICYAUX_INSTALL_TARGET_CMDS
        $(INSTALL) -m 0755 -D $(@D)/lib/libpolicyaux.so $(TARGET_DIR)/usr/lib
    endef

    $(eval $(generic-package))

    endif