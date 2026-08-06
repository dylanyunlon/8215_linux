################################################################################
#
# wpa_supplicant
#
################################################################################

PKGNAME := $(call UPPERCASE,$(pkgname))

$(PKGNAME)_SITE = $(TOPDIR)/../source/packages/connectivity/wifi/wpa_supplicant/wpa_supplicant-2.10
$(PKGNAME)_SITE_METHOD = local
$(PKGNAME)_DEPENDENCIES = dbus libnl openssl
$(PKGNAME)_MAKE_ENV = $(TARGET_CONFIGURE_OPTS)

define $(PKGNAME)_CONFIGURE_CMDS
    cp $(@D)/wpa_supplicant/defconfig $(@D)/wpa_supplicant/.config
endef

define $(PKGNAME)_BUILD_CMDS
    unset CFLAGS CPPFLAGS CXXFLAGS; \
    $($(PKG)_MAKE_ENV) $(MAKE) -C $(@D)/wpa_supplicant
endef

define $(PKGNAME)_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/wpa_supplicant/wpa_supplicant $(TARGET_DIR)/usr/sbin/
    $(INSTALL) -D -m 0755 $(@D)/wpa_supplicant/wpa_cli $(TARGET_DIR)/usr/sbin/
    #$(INSTALL) -D -m 0755 $(@D)/wpa_supplicant/wpa_passphrase $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0755 $($(PKG)_PKGDIR)/wpa_supplicant.sh $(TARGET_DIR)/usr/bin/
    $(INSTALL) -D -m 0644 $($(PKG)_PKGDIR)/wpa_supplicant.conf $(TARGET_DIR)/etc/
    $(INSTALL) -D -m 0644 $($(PKG)_PKGDIR)/dbus-wpa_supplicant.conf \
        $(TARGET_DIR)/etc/dbus-1/system.d/dbus-wpa_supplicant.conf
endef

$(eval $(generic-package))
