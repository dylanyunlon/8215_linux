################################################################################
#
# libnl
#
################################################################################

LIBNL_VERSION = 3.4.0
LIBNL_SITE = https://github.com/thom311/libnl/releases/download/libnl$(subst .,_,$(LIBNL_VERSION))
LIBNL_LICENSE = LGPL-2.1+
LIBNL_LICENSE_FILES = COPYING
LIBNL_INSTALL_STAGING = YES
LIBNL_DEPENDENCIES = host-bison host-flex host-pkgconf

ifeq ($(BR2_PACKAGE_LIBNL_TOOLS),y)
LIBNL_CONF_OPTS += --enable-cli
else
LIBNL_CONF_OPTS += --disable-cli
endif

ifeq ($(BR2_PACKAGE_CHECK),y)
LIBNL_DEPENDENCIES += check
LIBNL_CONF_OPTS += --enable-unit-tests
else
LIBNL_CONF_OPTS += --disable-unit-tests
endif

define LIBNL_REMOVE_UNUSED_LIBS
    rm -rf $(TARGET_DIR)/etc/libnl/
    rm -f $(TARGET_DIR)/usr/lib/libnl-idiag-3.so*
    rm -f $(TARGET_DIR)/usr/lib/libnl-nf-3.so*
    rm -f $(TARGET_DIR)/usr/lib/libnl-route-3.so*
    rm -f $(TARGET_DIR)/usr/lib/libnl-xfrm-3.so*
endef

LIBNL_POST_INSTALL_TARGET_HOOKS += LIBNL_REMOVE_UNUSED_LIBS

$(eval $(autotools-package))
