################################################################################
#
# audio btspeech
#
################################################################################
ifneq ($(ATC_BT_CHIP),)
    BTSPEECH_VERSION = 1.0
    BTSPEECH_CODE_PATH = $(TOPDIR)/../source/packages/audio/btspeech
    BTSPEECH_SITE = $(BTSPEECH_CODE_PATH)

    PKGDIR := $(pkgdir)
    PKGNAME := $(call UPPERCASE,$(pkgname))

    BTSPEECH_SITE_METHOD = local
    BTSPEECH_ALWAYS_BUILD = YES
    BTSPEECH_INSTALL_STAGING = YES
    BTSPEECH_DEPENDENCIES += alsa-lib -lpthread
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

    define BTSPEECH_BUILD_CMDS
            if [ -d $(BTSPEECH_CODE_PATH) ]; then \
            echo "build BTSPEECH"; \
            $($(PKG)_MAKE_ENV) TARGET_TOP=$($(PKG)_DIR) \
                $(MAKE) $($(PKG)_MAKE_OPTS) -C $($(PKG)_SITE);\
            else \
            echo "BTSPEECH Prebuild"; \
            fi
    endef


    define BTSPEECH_INSTALL_STAGING_CMDS
        $(INSTALL) -m 0755 -D $(@D)/bin/audio_bt_server $(STAGING_DIR)/usr/bin
    endef

    define BTSPEECH_INSTALL_TARGET_CMDS
        $(INSTALL) -m 0755 -D $(@D)/bin/audio_bt_server $(TARGET_DIR)/usr/bin
    endef

    $(eval $(generic-package))
endif
