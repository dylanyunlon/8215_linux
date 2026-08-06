################################################################################
#
# mali-t82x
#
################################################################################

MALI_T82X_VERSION = r21p0-01rel0
IS_SRC_EXIST = $(shell if [ -f $(TOPDIR)/../vendor/autochips/proprietary/hardware/mali_midgard/driver/product/bldsys/sconstruct ]; then echo "true"; else echo "false"; fi;)
ifeq ($(IS_SRC_EXIST), true)
MALI_T82X_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/mali_midgard/driver/product
else
MALI_T82X_SITE = $(TOPDIR)/../prebuilt/mali-t82x
endif
MALI_T82X_SITE_METHOD = local

MALI_T82X_INSTALL_STAGING = YES

MALI_T82X_DEPENDENCIES = host-scons host-pkgconf libdrm libdinfox


ENC_TOOLS = $(MALI_T82X_SITE)/dinfo/tools/enc64.py
PUB_KEY = $(MALI_T82X_SITE)/dinfo/tools/pubkey.pem

MALI_WINSYS = gbm
MALI_T82X_LDFLAGS = $(TARGET_LDFLAGS)
MALI_T82X_CFLAGS = $(TARGET_CFLAGS)

MALI_T82X_SCONS_ENV = $(TARGET_CONFIGURE_OPTS)

MALI_T82X_SCONS_OPTS = \
        -f bldsys/sconstruct profile=tx011-release-64 \
        gpu=t82x hwver=r1p0 gles32=0 kernel_modules=0 winsys=$(MALI_WINSYS) ump=0 wayland_server=0 drm_allocator=1 \
        allocator=drm_dumb simd=1 toolchain_prefix_target=aarch64-buildroot-linux-gnu- libs_install=libs

MALI_T82X_PROVIDES = libegl libgles


ifeq ($(IS_SRC_EXIST), true)
define MALI_T82X_BUILD_CMDS
        (cd $(@D); \
                $(MALI_T82X_SCONS_ENV) \
                $(SCONS) \
                $(MALI_T82X_SCONS_OPTS))
endef
endif

define MALI_LINK_LIB
	ln -nfs libmali.so libmali.so.0; \
	ln -nfs libmali.so libEGL.so; \
	ln -nfs libmali.so libEGL.so.1; \
	ln -nfs libmali.so libEGL.so.1.0.0; \
	ln -nfs libmali.so libEGL.so.1.4; \
	ln -nfs libmali.so libgbm.so; \
	ln -nfs libmali.so libgbm.so.1; \
	ln -nfs libmali.so libgbm.so.1.0.0; \
	ln -nfs libmali.so libGLESv1_CM.so; \
	ln -nfs libmali.so libGLESv1_CM.so.1; \
	ln -nfs libmali.so libGLESv1_CM.so.1.1; \
	ln -nfs libmali.so libGLESv1_CM.so.1.1.0; \
	ln -nfs libmali.so libGLESv2.so; \
	ln -nfs libmali.so libGLESv2.so.2; \
	ln -nfs libmali.so libGLESv2.so.2.0.0; \
	ln -nfs libmali.so libwayland-egl.so; \
	ln -nfs libmali.so libwayland-egl.so.1; \
	ln -nfs libmali.so libwayland-egl.so.1.0.0
endef

define MALI_T82X_INSTALL_TARGET_CMDS
        (cd $(@D); \
		export includedir="/usr/include"; \
		make install_lib DESTDIR=$(TARGET_DIR) WINSYS=$(MALI_WINSYS); \
		cd $(TARGET_DIR)/usr/lib64; \
		$(MALI_LINK_LIB))
endef

define MALI_T82X_INSTALL_STAGING_CMDS
        (cd $(@D); \
		export includedir="/usr/include"; \
		make install DESTDIR=$(STAGING_DIR) WINSYS=$(MALI_WINSYS); \
		cd $(STAGING_DIR)/usr/lib64; \
		$(MALI_LINK_LIB))
endef

$(eval $(generic-package))
