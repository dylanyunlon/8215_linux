################################################################################
#
# carplayutils
#
################################################################################
CARLINKUTILS_VERSION = 1.0
CARLINKUTILS_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/utils
CARLINKUTILS_PREBUILD_PATH = $(TOPDIR)/../prebuild/carlink/utils
CARLINKUTILS_SITE = $(shell if [ -d $(CARLINKUTILS_CODE_PATH) ]; then echo $(CARLINKUTILS_CODE_PATH); else echo $(CARLINKUTILS_PREBUILD_PATH); fi)
CARLINKUTILS_SITE_METHOD = local
CARLINKUTILS_ALWAYS_BUILD = YES
CARLINKUTILS_INSTALL_STAGING = YES
CARLINKUTILS_DEPENDENCIES = universal_utils protobuf tinyxml2 zlib

CARLINKUTILS_MAKE_ARGS := \
    SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
    RECURSION_MAK=$(TOPDIR)/../build/mak/recursion_build.mak

define CARLINKUTILS_BUILD_CMDS
	@if [ -d $(CARLINKUTILS_CODE_PATH) ]; then \
		echo "build carlinkutils"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(CARLINKCONFIGS_MAKE_ARGS) all; \
	else \
		echo "prebuild carlinkutils"; \
	fi
endef

define CARLINKUTILS_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0644 -D $(@D)/libcarlinkutils.so $(STAGING_DIR)/usr/lib
endef

define CARLINKUTILS_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcarlinkutils.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

