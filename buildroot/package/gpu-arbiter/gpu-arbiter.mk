################################################################################
#
# gpu-arbiter
#
################################################################################

GPU_ARBITER_VERSION = 1.0
IS_SRC_EXIST = $(shell if [ -d $(TOPDIR)/../mali_midgard/gpu_arbiter ]; then echo "true"; else echo "false"; fi;)
ifeq ($(IS_SRC_EXIST), true)
GPU_ARBITER_SITE = $(TOPDIR)/../mali_midgard/gpu_arbiter
else
GPU_ARBITER_SITE = $(TOPDIR)/../prebuilt/gpu-arbiter
endif
GPU_ARBITER_SITE_METHOD = local

GPU_ARBITER_INSTALL_STAGING = YES

GPU_ARBITER_LDFLAGS = $(TARGET_LDFLAGS)
GPU_ARBITER_CFLAGS = $(TARGET_CFLAGS)


ifeq ($(IS_SRC_EXIST), true)
define GPU_ARBITER_BUILD_CMDS
        (cd $(@D); \
                make CC="$(TARGET_CC)" CFLAGS="$(GPU_ARBITER_CFLAGS)")
endef
endif

define GPU_ARBITER_INSTALL_CMDS
	(cd $(@D))
endef

define GPU_ARBITER_INSTALL_STAGING_CMDS
        (cd $(@D); \
                make install DESTDIR=$(STAGING_DIR))
endef

$(eval $(generic-package))
