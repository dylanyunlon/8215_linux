################################################################################
#
# aic8800
#
################################################################################

ifeq ($(ATC_WIFI_CHIP),AIC8800)

PKGDIR := $(pkgdir)
PKGNAME := $(call UPPERCASE,$(pkgname))

$(PKGNAME)_SITE = $(TOPDIR)/../source/packages/connectivity/wifi/hardware/aic8800
$(PKGNAME)_SITE_METHOD = local

define $(PKGNAME)_BUILD_CMDS
endef

define $(PKGNAME)_INSTALL_TARGET_CMDS
    $(INSTALL) -d -m 0755 $(TARGET_DIR)/etc/firmware/
    $(INSTALL) -D -m 0644 \
        $($(PKG)_SITE)/firmware/aic8800D80/fw_adid_8800d80_u02.bin \
        $($(PKG)_SITE)/firmware/aic8800D80/fw_patch_8800d80_u02.bin \
        $($(PKG)_SITE)/firmware/aic8800D80/fw_patch_8800d80_u02_ext0.bin \
        $($(PKG)_SITE)/firmware/aic8800D80/fw_patch_table_8800d80_u02.bin \
        $($(PKG)_SITE)/firmware/aic8800D80/fmacfw_8800d80_u02.bin \
        $($(PKG)_SITE)/firmware/aic8800D80/lmacfw_rf_8800d80_u02.bin \
        $($(PKG)_SITE)/firmware/aic8800D80/aic_userconfig_8800d80.txt \
        $(TARGET_DIR)/etc/firmware/
    #$(INSTALL) -D -m 0755 $($(PKG)_PKGDIR)/wlan_driver.sh \
    #    $(TARGET_DIR)/etc/init.d/S80wlan_driver
    #ln -sf /etc/init.d/S80wlan_driver $(TARGET_DIR)/usr/bin/wlan_driver.sh
    $(INSTALL) -D -m 0755 $($(PKG)_PKGDIR)/wlan_driver.sh $(TARGET_DIR)/usr/bin/aic8800_wlan_driver.sh
endef

$(eval $(generic-package))

endif # ATC_WIFI_CHIP=AIC8800
