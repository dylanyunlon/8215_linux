################################################################################
#
# Xen
#
################################################################################
IS_SRC_EXIST = $(shell if [ -d $(TOPDIR)/../xen-4.11 ]; then echo "true"; else echo "false"; fi;)
ifeq ($(IS_SRC_EXIST), true)

XEN_VERSION = 4.11.1
#XEN_SITE = https://downloads.xenproject.org/release/xen/$(XEN_VERSION)
XEN_SITE = $(TOPDIR)/../xen-4.11
XEN_SITE_METHOD = local
XEN_LICENSE = GPL-2.0
XEN_LICENSE_FILES = COPYING
XEN_DEPENDENCIES = host-acpica host-python gpu-arbiter
TARGET_LDFLAGS =

# Calculate XEN_ARCH
ifeq ($(ARCH),aarch64)
XEN_ARCH = arm64
else ifeq ($(ARCH),arm)
XEN_ARCH = arm32
endif

XEN_CONF_OPTS = \
	--disable-ocamltools \
	--with-initddir=/etc/init.d

XEN_CONF_ENV = PYTHON=$(HOST_DIR)/bin/python2
XEN_MAKE_ENV = \
	XEN_TARGET_ARCH=$(XEN_ARCH) \
	CROSS_COMPILE=$(TARGET_CROSS) \
	HOST_EXTRACFLAGS="-Wno-error" \
	$(TARGET_CONFIGURE_OPTS)

ifeq ($(BR2_PACKAGE_XEN_HYPERVISOR),y)
XEN_MAKE_OPTS += dist-xen
XEN_INSTALL_IMAGES = YES
define XEN_INSTALL_IMAGES_CMDS
	cp $(@D)/xen/xen $(BINARIES_DIR)
endef
else
XEN_CONF_OPTS += --disable-xen
endif

ifeq ($(BR2_PACKAGE_XEN_TOOLS),y)
XEN_DEPENDENCIES += dtc libaio libglib2 ncurses openssl pixman util-linux yajl
ifeq ($(BR2_PACKAGE_ARGP_STANDALONE),y)
XEN_DEPENDENCIES += argp-standalone
endif
XEN_INSTALL_TARGET_OPTS += DESTDIR=$(TARGET_DIR) install-tools
XEN_MAKE_OPTS += dist-tools
XEN_CONF_OPTS += --with-extra-qemuu-configure-args="--disable-sdl --disable-opengl"

define XEN_INSTALL_INIT_SYSV
	mv $(TARGET_DIR)/etc/init.d/xencommons $(TARGET_DIR)/etc/init.d/S50xencommons
	mv $(TARGET_DIR)/etc/init.d/xen-watchdog $(TARGET_DIR)/etc/init.d/S50xen-watchdog
	mv $(TARGET_DIR)/etc/init.d/xendomains $(TARGET_DIR)/etc/init.d/S60xendomains
endef
else
XEN_INSTALL_TARGET = NO
XEN_CONF_OPTS += --disable-tools
endif

$(eval $(autotools-package))

else
XEN_VERSION = 4.11.1
XEN_SITE = $(TOPDIR)/../prebuilt/xen
XEN_SITE_METHOD = local
XEN_LICENSE = GPL-2.0
XEN_LICENSE_FILES = COPYING

define XEN_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755	 -d  $(TARGET_DIR)/etc/default
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/etc/default/*  $(TARGET_DIR)/etc/default
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/etc/init.d/*  $(TARGET_DIR)/etc/init.d
	$(INSTALL) -m 0755	 -d  $(TARGET_DIR)/etc/xen/scripts
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/etc/xen/scripts/*  $(TARGET_DIR)/etc/xen/scripts
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/etc/xen/cpupool  $(TARGET_DIR)/etc/xen
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/etc/xen/README*  $(TARGET_DIR)/etc/xen
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/etc/xen/xl*  $(TARGET_DIR)/etc/xen
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/usr/bin/*  $(TARGET_DIR)/usr/bin
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/usr/sbin/*  $(TARGET_DIR)/usr/sbin
	$(INSTALL) -m 0755	 -d  $(TARGET_DIR)/usr/lib/xen/bin
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/usr/lib/xen/bin/*  $(TARGET_DIR)/usr/lib/xen/bin
	$(INSTALL) -m 0755	 -d  $(TARGET_DIR)/usr/lib/xen/libexec
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/usr/lib/xen/libexec/*  $(TARGET_DIR)/usr/lib/xen/libexec
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/usr/lib/lib*  $(TARGET_DIR)/usr/lib
	$(INSTALL) -m 0755	 -d  $(TARGET_DIR)/usr/share/qemu-xen/qemu/keymaps
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/usr/share/qemu-xen/qemu/keymaps/*   $(TARGET_DIR)/usr/share/qemu-xen/qemu/keymaps
	$(INSTALL) -m 0755	 -D $(XEN_SITE)/usr/share/qemu-xen/qemu/*.*   $(TARGET_DIR)/usr/share/qemu-xen/qemu/
	$(INSTALL) -m 0755	 -d  $(TARGET_DIR)/var/lib/xen/dump
	$(INSTALL) -m 0755	 -d  $(TARGET_DIR)/var/lib/xenstored
	$(INSTALL) -m 0755	 -d  $(TARGET_DIR)/var/log/xen
	$(INSTALL) -m 0755	 -d  $(TARGET_DIR)/var/run/xen
	$(INSTALL) -m 0755	 -d  $(TARGET_DIR)/var/run/xenstored
	cp  $(XEN_SITE)/xen-4.11.1  $(BINARIES_DIR)/xen.bin -f
endef

define XEN_INSTALL_INIT_SYSV
	mv $(TARGET_DIR)/etc/init.d/xencommons $(TARGET_DIR)/etc/init.d/S50xencommons
	mv $(TARGET_DIR)/etc/init.d/xen-watchdog $(TARGET_DIR)/etc/init.d/S50xen-watchdog
	mv $(TARGET_DIR)/etc/init.d/xendomains $(TARGET_DIR)/etc/init.d/S60xendomains
endef

$(eval $(generic-package))

endif
