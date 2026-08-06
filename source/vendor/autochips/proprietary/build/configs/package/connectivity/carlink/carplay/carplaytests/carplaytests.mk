################################################################################
#
# libcarplayclienttestsclient
#
################################################################################
CARPLAYTESTS_VERSION = 1.0
CARPLAYTESTS_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/tests/carplayclienttests/client
CARPLAYTESTS_SITE = $(shell echo $(CARPLAYTESTS_CODE_PATH))
CARPLAYTESTS_SITE_METHOD = local
CARPLAYTESTS_ALWAYS_BUILD = YES
CARPLAYTESTS_INSTALL_STAGING = YES

CARPLAYTESTS_MAKE_ARGS += \
    SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
    RECURSION_MAK=$(TOPDIR)/../build/mak/recursion_build.mak

define CARPLAYTESTS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(CARLINKCONFIGS_MAKE_ARGS) all;
endef

define CARPLAYTESTS_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0644 -D $(@D)/libcarplayclienttestsclient.so $(STAGING_DIR)/usr/lib
endef

define CARPLAYTESTS_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcarplayclienttestsclient.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

