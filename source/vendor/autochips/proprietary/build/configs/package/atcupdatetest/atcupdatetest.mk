################################################################################
#
# car libcluster
#
################################################################################

ATCUPDATETEST_VERSION = 1.0
ATCUPDATETEST_SITE = $(TOPDIR)/../source/packages/ab_update/atcupdatetest/1.0
ATCUPDATETEST_SITE_METHOD = local
ATCUPDATETEST_ALWAYS_BUILD = YES
ATCUPDATETEST_DEPENDENCIES += libupdate

ATCUPDATETEST_INSTALL_STAGING = YES

ATCUPDATE_SERVICE_DIR = $(TOPDIR)/../source/packages/ab_update/
ATCUPDATETEST_TARGET := atcupdatetest.bin
ATCUPDATETEST_MAKE_ARGS += SHARED_LIB=

define ATCUPDATETEST_BUILD_CMDS
         $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define ATCUPDATETEST_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/$(ATCUPDATETEST_TARGET) $(TARGET_DIR)/usr/lib
endef



$(eval $(generic-package))
