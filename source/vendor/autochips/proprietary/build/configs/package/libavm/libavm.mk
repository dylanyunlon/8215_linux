################################################################################
#
# libavm algo so
#
################################################################################

LIBAVM_VERSION = 1.0.0
LIBAVM_SITE = $(TOPDIR)/../prebuilt/libavm
LIBAVM_SITE_METHOD = local
LIBAVM_ALWAYS_BUILD = YES
LIBAVM_DEPENDENCIES += opencv3

define LIBAVM_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(LIBAVM_SITE)/libavm_algo.so $(TARGET_DIR)/usr/lib
       $(INSTALL) -m 0755 -D $(LIBAVM_SITE)/algoApi.h $(TARGET_DIR)/usr/include
endef

$(eval $(generic-package))
