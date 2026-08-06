################################################################################
#
# MountService
#
################################################################################

FSCK_MSDOS_VERSION = 1.0.0
FSCK_MSDOS_SITE = $(TOPDIR)/../source/packages/fsck_msdos
FSCK_MSDOS_SITE_METHOD = local
FSCK_MSDOS_ALWAYS_BUILD = YES
FSCK_MSDOS_DEPENDENCIES += glibc

define FSCK_MSDOS_BUILD_CMDS
	 $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define FSCK_MSDOS_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/fsck.msdos $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

