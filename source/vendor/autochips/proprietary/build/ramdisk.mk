# ==========================================================================
# Build system
# ==========================================================================

# Maybe set in project config.
RAMDISK_SRC := $(TOP_DIR)ramdisk
export ROOT_OUT := $(TARGET_OUT_DIR)/root
export ROOT_OUT_BIN := $(TARGET_OUT_DIR)/root/bin
export ROOT_OUT_LIB := $(TARGET_OUT_DIR)/root/usr/lib
INSTALLED_RAMDISK_TARGET := $(TARGET_OUT_DIR)/initrd.img
$(INSTALLED_RAMDISK_TARGET): rootfs
.PHONY: rootfs rootfs-clean app connectivity
rootfs:
	cd $(RAMDISK_SRC)
	rm -rf $(ROOT_OUT)
	mkdir -p $(ROOT_OUT)
	mkdir -p $(ROOT_OUT_BIN)
	mkdir -p $(ROOT_OUT_LIB)
	mkdir -p $(ROOT_OUT)/lib/
	mkdir -p $(ROOT_OUT)/lib/firmware/
	mkdir -p $(ROOT_OUT)/etc/
	mkdir -p $(ROOT_OUT)/drivers/
#	make -C $(RAMDISK_SRC)/app
	make -C $(RAMDISK_SRC)/busybox
	cp -rf $(RAMDISK_SRC)/busybox/sysconfig/* $(ROOT_OUT)/etc/
	cp -rf $(RAMDISK_SRC)/busybox/lib/firmware/* $(ROOT_OUT)/lib/firmware
	for m in `find $(TARGET_OUT_DIR)/KERNEL_OBJ/ -name *.ko`; do  \
		cp -f $$m $(ROOT_OUT)/drivers/ ; \
	done
	$(CROSS_COMPILE)strip --strip-debug $(ROOT_OUT)/drivers/*.ko
	cd $(ROOT_OUT) && ln -s /bin/busybox init && find ./ | cpio -o -H newc > ../initrd.img

rootfs-clean:
	rm -rf $(ROOT_OUT)

app: ramdisk
	make -C $(RAMDISK_SRC)/app
	cp -rfP $(RAMDISK_SRC)/libs/* $(ROOT_OUT)/lib/
	cd $(ROOT_OUT) && find ./ | cpio -o -H newc > ../initrd.img

connectivity: kernel ramdisk app

