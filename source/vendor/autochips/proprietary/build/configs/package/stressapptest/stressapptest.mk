################################################################################
#
# MountService
#
################################################################################

STRESSAPPTEST_VERSION = 1.0.0
STRESSAPPTEST_SITE = $(TOPDIR)/../source/packages/common/stressapptest
STRESSAPPTEST_SITE_METHOD = local
STRESSAPPTEST_ALWAYS_BUILD = YES
STRESSAPPTEST_DEPENDENCIES += glibc

STRESSAPPTEST_AUTORECONF = YES
STRESSAPPTEST_DEPENDENCIES = libaio

define STRESSAPPTEST_BUILD_CMDS
	 $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define STRESSAPPTEST_INSTALL_TARGET_CMDS
endef

$(eval $(autotools-package))

