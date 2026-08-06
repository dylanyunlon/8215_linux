################################################################################
#
# cluster-daemon
#
################################################################################

CLUSTER_DAEMON_VERSION = 1.0.0
CLUSTER_DAEMON_SITE = $(TOPDIR)/../cluster/daemon
CLUSTER_DAEMON_SITE_METHOD = local
CLUSTER_DAEMON_ALWAYS_BUILD = YES
CLUSTER_DAEMON_DEPENDENCIES += glibc

define CLUSTER_DAEMON_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define CLUSTER_DAEMON_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/cluster-daemon $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

