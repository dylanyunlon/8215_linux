################################################################################
#
# car libsurface_atc
#
################################################################################

LIBATCSURFACE_VERSION = 1.0
LIBATCSURFACE_CODE_PATH = $(TOPDIR)/../source/packages/graphics/surface/surface
LIBATCSURFACE_PREBUILD_PATH = $(TOPDIR)/../prebuild/atcsurface
LIBATCSURFACE_SITE = $(shell if [ -d $(LIBATCSURFACE_CODE_PATH) ]; then echo $(LIBATCSURFACE_CODE_PATH); else echo $(LIBATCSURFACE_PREBUILD_PATH); fi)
LIBATCSURFACE_SITE_METHOD = local
LIBATCSURFACE_ALWAYS_BUILD = YES
LIBATCSURFACE_INSTALL_STAGING = YES
# LIBATCSURFACE_DEPENDENCIES += libdrm 

LIBATCSURFACE_MAKE_ARGS += STATIC_LIB=

LIBATCSURFACE_MAKE_OPTS = \
	KENREL_HEADER_PATH=$(TOPDIR)/../source/kernel/kernel-3.18/drivers/misc/atc/inc


define LIBATCSURFACE_BUILD_CMDS
	@if [ -d $(LIBATCSURFACE_CODE_PATH) ]; then \
		echo "build libsurface"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(LIBATCSURFACE_MAKE_OPTS) -f Makefile; \
	else \
		echo "libsurface_atc Prebuild"; \
	fi
endef

define LIBATCSURFACE_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libsurface_atc.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/include/atcsurface.h $(STAGING_DIR)/usr/include/
endef

define LIBATCSURFACE_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libsurface_atc.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

