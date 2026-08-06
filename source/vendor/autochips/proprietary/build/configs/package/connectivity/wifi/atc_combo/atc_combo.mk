################################################################################
#
# aic8800
#
################################################################################

PKGDIR := $(pkgdir)
PKGNAME := $(call UPPERCASE,$(pkgname))

$(PKGNAME)_SITE = $(TOPDIR)/../source/packages/connectivity/wifi/hardware/aic8800
$(PKGNAME)_SITE_METHOD = local

define $(PKGNAME)_BUILD_CMDS
endef

define $(PKGNAME)_INSTALL_TARGET_CMDS
    $(INSTALL) -d -m 0755 $(TARGET_DIR)/etc/default/
    echo "ATC_WIFI_CHIP=${ATC_WIFI_CHIP}" \
        > $(TARGET_DIR)/etc/default/atc_combo.conf
    echo "ATC_WLAN_TRANSMISSION_MODE=${ATC_WLAN_TRANSMISSION_MODE}" \
        >> $(TARGET_DIR)/etc/default/atc_combo.conf

    $(INSTALL) -D -m 0755 $($(PKG)_PKGDIR)/atc_combo.sh \
        $(TARGET_DIR)/etc/init.d/S80atc_combo
    ln -sf /etc/init.d/S80atc_combo $(TARGET_DIR)/usr/bin/atc_combo.sh
endef

$(eval $(generic-package))
