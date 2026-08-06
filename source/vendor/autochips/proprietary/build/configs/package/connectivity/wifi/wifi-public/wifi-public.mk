################################################################################
#
# wifi-public
#
################################################################################

PKGNAME = $(call UPPERCASE,$(pkgname))
$(PKGNAME)_SITE = $(TOPDIR)/../source/packages/connectivity/wifi/public
$(PKGNAME)_SITE_METHOD = local
$(PKGNAME)_INSTALL_STAGING = YES
$(PKGNAME)_DEPENDENCIES = wifi-private
WIFI_CONFIG_FILE_PATH = /data/misc/wifi

define $(PKGNAME)_INSTALL_STAGING_CMDS
    rsync -au $($(PKG)_SITE)/include/wifi $(STAGING_DIR)/usr/include
    $(INSTALL) -D -m 0644 $($(PKG)_PKGDIR)/wifi_dbus.conf \
        $(TARGET_DIR)/etc/dbus-1/system.d/wifi_dbus.conf
    $(INSTALL) -d -m 0755 $(TARGET_DIR)/etc/default/
    echo "WIFI_CONFIG_FILE_PATH=$(WIFI_CONFIG_FILE_PATH)" \
        > $(TARGET_DIR)/etc/default/wifi_env.conf
    $(INSTALL) -D -m 0644 $($(PKG)_PKGDIR)/config.xml \
        $(TARGET_DIR)/etc/wifi/config.xml
    $(INSTALL) -D -m 0644 $($(PKG)_PKGDIR)/userconfig/aic8800/usrconfig.xml \
        $(TARGET_DIR)/etc/wifi/usrconfig.xml
endef

$(eval $(generic-package))
