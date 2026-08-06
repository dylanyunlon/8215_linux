################################################################################
#
# wifi-private
#
################################################################################

PKGDIR := $(pkgdir)
PKGNAME := $(call UPPERCASE,$(pkgname))

$(PKGNAME)_ATC_SRC := $(TOPDIR)/../source/packages/connectivity/wifi/private
$(PKGNAME)_PREBUILT := $(TOPDIR)/../prebuild/connectivity/wifi/private
ifneq ($(wildcard $($(PKGNAME)_ATC_SRC)),)
$(PKGNAME)_SITE := $($(PKGNAME)_ATC_SRC)
else
$(PKGNAME)_SITE := $($(PKGNAME)_PREBUILT)
endif
$(PKGNAME)_SITE_METHOD = local
$(PKGNAME)_INSTALL_STAGING = YES
$(PKGNAME)_DEPENDENCIES = universal_utils dbus-cpp libmetazone tinyxml2 libnl libcurl
$(PKGNAME)_MAKE_ENV := \
    $(TARGET_CONFIGURE_OPTS) \
    ATC_BUILD_DIR=$(PKGDIR)/../../mak \
    RECURSION_MAK=$(PKGDIR)/../../mak/recursion_build.mak \
    BUILD_EXECUTABLE=$(PKGDIR)/../../mak/build_exe.mak \
    BUILD_SHARED_LIBRARY=$(PKGDIR)/../../mak/build_share_lib.mak \
    BUILD_STATIC_LIBRARY=$(PKGDIR)/../../mak/build_static_lib.mak \
    TARGET_VENDOR=$(TARGET_VENDOR) \
    DA_TOP=$(TOPDIR)/../source \
    DA_SYSROOT=$(STAGING_DIR) \
    CONFIG_TINYXML2=1 \
    ATC_AOSP_ENHANCEMENT_KERNEL_3_18=1 \

$(PKGNAME)_MAKE_OPTS = \
    CROSS_COMPILE=$(TARGET_CROSS)

define $(PKGNAME)_BUILD_CMDS
    if [ -d $($(PKG)_ATC_SRC) ]; then \
        $($(PKG)_MAKE_ENV) TARGET_TOP=$($(PKG)_DIR) \
            $(MAKE) $($(PKG)_MAKE_OPTS) -C $($(PKG)_SITE); \
    else \
        echo "wifi-private prebuilt"; \
    fi
endef

define $(PKGNAME)_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/lib/libwificommon.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/lib/libwificlient.so $(STAGING_DIR)/usr/lib
endef

define $(PKGNAME)_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(@D)/lib/libwificommon.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/lib/libwificlient.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/lib/libwifihal.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/bin/wifiserver $(TARGET_DIR)/usr/bin
    $(INSTALL) -m 0755 -D $(@D)/bin/wificlient_cmdtest $(TARGET_DIR)/usr/bin
    # $(INSTALL) -D -m 0755 $($(PKG)_PKGDIR)/wifiserver.sh \
    #    $(TARGET_DIR)/etc/init.d/S82wifiserver
    # ln -sf /etc/init.d/S82wifiserver $(TARGET_DIR)/usr/bin/wifiserver.sh
    $(INSTALL) -D -m 0755 $($(PKG)_PKGDIR)/wifiserver.sh \
        $(TARGET_DIR)/usr/bin/wifiserver.sh
endef

$(eval $(generic-package))
