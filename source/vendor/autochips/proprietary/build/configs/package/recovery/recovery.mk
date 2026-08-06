################################################################################
#
# recovery
#
################################################################################
RECOVERY_VERSION = 1.0
RECOVERY_SITE = $(TOPDIR)/../source/packages/recovery
RECOVERY_SITE_METHOD = local
RECOVERY_ALWAYS_BUILD = YES
RECOVERY_INSTALL_STAGING = YES
RECOVERY_UPDATE_TARGET := recovery-update.bin


RECOVERY_DEPLOY_DIR = $(BINARIES_DIR)
define RECOVERY_SET_ENV_VARS
    recovery_top=$(@D) \
    recovery_build=$(@D)/build \
    recovery_os=$(@D)/os \
    recovery_out=$(@D)/out \
    recovery_lib=$(@D)/lib \
    recovery_update=$(@D)/recovery-update \
    recovery_sysroot=$(@D)/out/sysroot
endef

define RECOVERY_BUILD_CMDS
	cd $(@D) && \
    $(TARGET_MAKE_ENV) \
    KO_DIR=$(TOPDIR)/../out/target/ac83xx/lib/modules/3.18.49/kernel/drivers/ \
	$(MAKE) os 

	cd $(@D) && \
    $(TARGET_MAKE_ENV) \
    $(RECOVERY_SET_ENV_VARS) \
    CUST_PROD=$(if $(BR2_NAND_BOOT_DEVICE),NAND_PROJ,) \
    $(MAKE) -C $(@D) $(if $(BR2_NAND_BOOT_DEVICE),CUSTPROJ=NAND_PROJ ,) update

	cd $(@D) && \
    $(TARGET_MAKE_ENV) \
    recovery_top=$(@D) \
    recovery_build=$(@D)/build \
    $(MAKE) -C $(@D) rootfs
endef


define RECOVERY_DEPLOY_IMAGES
    $(INSTALL) -d -m 0755 $(RECOVERY_DEPLOY_DIR)

    if [ -f $(@D)/recovery.gz ]; then \
        $(INSTALL) -m 0644 $(@D)/recovery.gz $(RECOVERY_DEPLOY_DIR)/; \
        echo "Recovery image deployed to: $(RECOVERY_DEPLOY_DIR)/recovery.gz"; \
    else \
        echo "Warning: recovery.gz not found in $(@D)/"; \
    fi
endef





define RECOVERY_DEPLOY
    $(call RECOVERY_DEPLOY_IMAGES)
endef

RECOVERY_POST_INSTALL_TARGET_HOOKS += RECOVERY_DEPLOY
define RECOVERY_CLEAN_CMDS
    $(MAKE) -C $(@D) clean
endef

$(eval $(generic-package))