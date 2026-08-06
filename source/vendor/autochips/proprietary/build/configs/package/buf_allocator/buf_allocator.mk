################################################################################
#
# buf_allocator
#
################################################################################

BUF_ALLOCATOR_VERSION = 1.0.0
BUF_ALLOCATOR_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/videoin/libs/utils/buf_allocator
BUF_ALLOCATOR_SITE_METHOD = local
BUF_ALLOCATOR_ALWAYS_BUILD = YES
BUF_ALLOCATOR_INSTALL_STAGING = YES
BUF_ALLOCATOR_DEPENDENCIES += libatcsurface

define BUF_ALLOCATOR_BUILD_CMDS
	 $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define BUF_ALLOCATOR_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/libbuf_allocator.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

