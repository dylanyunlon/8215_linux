################################################################################
#
# connman
#
################################################################################

CONNMAN_SITE = $(TOPDIR)/../source/packages/connectivity/wifi/connman/connman-1.37
CONNMAN_SITE_METHOD = local
CONNMAN_DEPENDENCIES = dbus libglib2 readline iptables
CONNMAN_AUTORECONF = YES
CONNMAN_PRE_CONFIGURE_HOOKS += CONNMAN_AUTORECONF_M4
WIFI_CONFIG_FILE_PATH = /data/misc/wifi

CONNMAN_CONF_OPTS = \
    --enable-debug \
    --enable-loopback \
    --enable-client \
    --enable-wifi \
    --with-tmpfilesdir=/etc/tmpfiles.d/ \
    \
    --enable-atc_aosp_enhancement \
    --disable-config_atc_8317 \
    --with-atc_wifi_chip=${ATC_WIFI_CHIP} \
    --with-atc_wlan_transmission_mode=${ATC_WLAN_TRANSMISSION_MODE} \
    --with-wifi_config_path=${WIFI_CONFIG_FILE_PATH} \
    \
    --disable-wispr \
    --disable-polkit \
    --disable-tools \
    --disable-ethernet \
    --disable-gadget \
    --disable-bluetooth \
    --disable-ofono \
    --disable-dundee \
    --disable-pacrunner \
    --disable-neard \

define CONNMAN_AUTORECONF_M4
    mkdir $(@D)/m4
endef

define CONNMAN_INSTALL_CM
    $(INSTALL) -D -m 0755 $(@D)/client/connmanctl $(TARGET_DIR)/usr/bin/connmanctl
    # $(INSTALL) -D -m 0755 $($(PKG)_PKGDIR)/connman.sh $(TARGET_DIR)/etc/init.d/S81connman
    # ln -sf /etc/init.d/S81connman $(TARGET_DIR)/usr/bin/connman.sh
    $(INSTALL) -D -m 0755 $($(PKG)_PKGDIR)/connman.sh $(TARGET_DIR)/usr/bin/connman.sh
endef

CONNMAN_POST_INSTALL_TARGET_HOOKS += CONNMAN_INSTALL_CM

$(eval $(autotools-package))
