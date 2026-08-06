################################################################################
#
# dbus-c++
#
################################################################################

PKGNAME = $(call UPPERCASE,$(pkgname))
$(PKGNAME)_SITE = $(TOPDIR)/../source/packages/connectivity/wifi/libdbus-c++/libdbus-c++-0.9.0
$(PKGNAME)_SITE_METHOD = local
$(PKGNAME)_INSTALL_STAGING = YES
$(PKGNAME)_DEPENDENCIES = dbus
$(PKGNAME)_AUTORECONF = YES
$(PKGNAME)_CONF_OPTS = \
    --disable-ecore \
    --disable-glib \
    --disable-examples \
    --disable-tests \
    --disable-tools

$(eval $(autotools-package))
