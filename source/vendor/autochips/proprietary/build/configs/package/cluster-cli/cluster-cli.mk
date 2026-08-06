################################################################################
#
# cluster-cli
#
################################################################################

CLUSTER_CLI_VERSION = 1.0.0
CLUSTER_CLI_CODE_PATH = $(TOPDIR)/../source/packages/cluster/cli
CLUSTER_CLI_SITE_METHOD = local
CLUSTER_CLI_ALWAYS_BUILD = YES
CLUSTER_CLI_DEPENDENCIES += glibc
CLUSTER_CLI_PREBUILD_PATH = $(TOPDIR)/cli
CLUSTER_CLI_SITE = $(shell if [ -d $(CLUSTER_CLI_CODE_PATH) ]; then echo $(CLUSTER_CLI_CODE_PATH); else echo $(CLUSTER_CLI_PREBUILD_PATH); fi)
define CLUSTER_CLI_BUILD_CMDS
	@if [ -d $(CLUSTER_CLI_CODE_PATH) ]; then \
		echo "build cluster-cli"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D); \
	else \
		echo "cluster-cli Prebuild"; \
	fi
endef

define CLUSTER_CLI_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/cluster $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

