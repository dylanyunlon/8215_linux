################################################################################
#
# car mali-t400
#
################################################################################

# VERSION := r5p2 or r6p1
MALI_T400_VERSION = r6p1
MALI_T400_CODE_PATH = $(TOPDIR)/../source/packages/graphics/opengles
MALI_T400_PREBUILD_PATH = $(TOPDIR)/../prebuild/opengles
MALI_T400_SITE = $(shell if [ -d $(MALI_T400_CODE_PATH) ]; then echo $(MALI_T400_CODE_PATH); else echo $(MALI_T400_PREBUILD_PATH); fi)
MALI_T400_SITE_METHOD = local
MALI_T400_ALWAYS_BUILD = YES
MALI_T400_INSTALL_STAGING = YES
MALI_T400_FBDEV_BACKEND = 1
MALI_T400_WAYLAND_BACKEND = 0
MALI_T400_PROVIDES = libegl libgles

MALI_T400_MAKE_ARGS += STATIC_LIB=

MALI_T400_MAKE_OPTS = \
	VERSION=$(MALI_T400_VERSION) \
	FBDEV_BACKEND=$(MALI_T400_FBDEV_BACKEND) \
	WAYLAND_BACKEND=$(MALI_T400_WAYLAND_BACKEND) \
	DESTDIR=$(TARGET_DIR)

define MALI_T400_BUILD_CMDS
	@if [ -d $(MALI_T400_CODE_PATH) ]; then \
		echo "build OpenGLES"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(MALI_T400_MAKE_OPTS) -f Makefile; \
	else \
		echo "OpenGLES Prebuild"; \
	fi
endef

define MALI_T400_INSTALL_STAGING_CMDS
    @if [ $(MALI_T400_FBDEV_BACKEND) -eq 1 ]; then \
        echo "install fbdev egl headers"; \
        cp -a $(@D)/opengles-$(MALI_T400_VERSION)/3rdparty/include/khronos/*  $(STAGING_DIR)/usr/include; \
	      cp -a $(@D)/opengles-$(MALI_T400_VERSION)/include/EGL/fbdev_window.h $(STAGING_DIR)/usr/include/EGL; \
	      cp -a $(@D)/opengles-$(MALI_T400_VERSION)/include/EGL/platform_fbdev/EGL/eglplatform.h $(STAGING_DIR)/usr/include/EGL; \
	  fi
	  @if [ $(MALI_T400_WAYLAND_BACKEND) -eq 1 ]; then \
		    echo "install wayland egl headers"; \
	      cp -a $(@D)/opengles-$(MALI_T400_VERSION)/include/EGL/wayland_window.h $(STAGING_DIR)/usr/include/EGL; \
	      cp -a $(@D)/opengles-$(MALI_T400_VERSION)/include/EGL/platform_wayland/EGL/eglplatform.h $(STAGING_DIR)/usr/include/EGL; \
	  fi
	  cp -arf $(@D)/opengles-$(MALI_T400_VERSION)/pc $(STAGING_DIR)/usr/lib/pkgconfig; \
	  echo "install mali ddk libs" ; \
	  $(INSTALL) -m 0755 -D $(@D)/opengles-$(MALI_T400_VERSION)/lib/libMali.so $(STAGING_DIR)/usr/lib; \
	  $(INSTALL) -m 0755 -D $(@D)/opengles-$(MALI_T400_VERSION)/lib/libUMP.so $(STAGING_DIR)/usr/lib; \
    pushd $(STAGING_DIR)/usr/lib ; \
	  ln -sf libMali.so libEGL.so.1.4; \
	  ln -sf libMali.so libGLESv1_CM.so.1.1; \
	  ln -sf libMali.so libGLESv2.so.2.0; \
	  ln -sf libEGL.so.1.4 libEGL.so.1; \
	  ln -sf libEGL.so.1 libEGL.so; \
	  ln -sf libGLESv1_CM.so.1.1 libGLESv1_CM.so.1; \
	  ln -sf libGLESv1_CM.so.1 libGLESv1_CM.so; \
	  ln -sf libGLESv2.so.2.0 libGLESv2.so.2; \
	  ln -sf libGLESv2.so.2 libGLESv2.so; \
	  popd;
endef

define MALI_T400_INSTALL_TARGET_CMDS
	  echo "install mali ddk libs" ; \
	  $(INSTALL) -m 0755 -D $(@D)/opengles-$(MALI_T400_VERSION)/lib/libMali.so $(TARGET_DIR)/usr/lib; \
	  $(INSTALL) -m 0755 -D $(@D)/opengles-$(MALI_T400_VERSION)/lib/libUMP.so $(TARGET_DIR)/usr/lib; \
    pushd $(TARGET_DIR)/usr/lib ; \
	  ln -sf libMali.so libEGL.so.1.4; \
	  ln -sf libMali.so libGLESv1_CM.so.1.1; \
	  ln -sf libMali.so libGLESv2.so.2.0; \
	  ln -sf libEGL.so.1.4 libEGL.so.1; \
	  ln -sf libEGL.so.1 libEGL.so; \
	  ln -sf libGLESv1_CM.so.1.1 libGLESv1_CM.so.1; \
	  ln -sf libGLESv1_CM.so.1 libGLESv1_CM.so; \
	  ln -sf libGLESv2.so.2.0 libGLESv2.so.2; \
	  ln -sf libGLESv2.so.2 libGLESv2.so; \
	  popd;
endef

$(eval $(generic-package))

