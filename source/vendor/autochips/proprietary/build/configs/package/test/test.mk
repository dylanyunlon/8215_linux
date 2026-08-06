################################################################################
#
# aee service: Dump information for NE and KE.
#
################################################################################

TEST_VERSION = 1.0.0
TEST_SITE = $(TOPDIR)/../source/test
TEST_SITE_METHOD = local
TEST_ALWAYS_BUILD = YES
TEST_DEPENDENCIES += mmisc atcomx libatcsurface directrender

TEST_MAKE_OPTS = \
	TOPDIR=$(TOPDIR)/.. KERNEL_TOPDIR=$(TOPDIR)/../source/kernel/kernel-3.18
	
define TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(TEST_MAKE_OPTS)
endef

define TEST_INSTALL_TARGET_CMDS
endef

$(eval $(generic-package))

