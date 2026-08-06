#
#  For ac8x kernel build
#
KERNEL_SRC := $(TOP_DIR)kernel
KERNEL_OUT := $(TARGET_OUT_DIR)/KERNEL_OBJ

PLATFORM := $(platform)

CONFIGFILE=ac8x-emu_defconfig
KARCH=arm64
ifeq "$(PLATFORM)" "s1"
CONFIGFILE=ac8x-emu-s1_defconfig
DTSNAME=autochips/ac8x-emu-s1
else ifeq "$(PLATFORM)" "z1"
CONFIGFILE=ac8x-emu-z1_defconfig
DTSNAME=autochips/ac8x-emu-z1
else
CONFIGFILE=ac8x-emu_defconfig
DTSNAME=autochips/ac8x-emu
endif

$(warning $(DTSNAME))

export CROSS_COMPILE := $(abspath $(TOP))/$(TARGET_TOOLCHAIN_ROOT)

JOBS := $(shell grep processor /proc/cpuinfo |wc -l)

INSTALLED_RAMDISK_TARGET := $(TARGET_OUT_DIR)/kernel.bin
$(INSTALLED_KERNEL_TARGET): kernel

.PHONY: kernel kernel-config kernel-clean
kernel: kernel-config
	make -C $(KERNEL_SRC) ARCH=$(KARCH) O=$(KERNEL_OUT) -j$(JOBS) -j24
	make -C $(KERNEL_SRC) ARCH=$(KARCH) O=$(KERNEL_OUT) $(DTSNAME).dtb -j24
	cp $(KERNEL_OUT)/arch/arm64/boot/Image $(TARGET_OUT_DIR)/kernel.bin
	cp $(KERNEL_OUT)/arch/arm64/boot/dts/autochips/$(notdir $(DTSNAME)).dtb $(TARGET_OUT_DIR)
	@echo "Default Kernel build done ->:)"

kernel-z1: kernel-clean kernel-config
	make -C $(KERNEL_SRC) ARCH=$(KARCH) O=$(KERNEL_OUT) ac8x-emu-z1_defconfig -j24
	make -C $(KERNEL_SRC) ARCH=$(KARCH) O=$(KERNEL_OUT) -j$(JOBS) -j24
	make -C $(KERNEL_SRC) ARCH=$(KARCH) O=$(KERNEL_OUT) autochips/ac8x-emu-z1.dtb -j24
	cp $(KERNEL_OUT)/arch/arm64/boot/Image $(TARGET_OUT_DIR)/kernel-z1.bin
	cp $(KERNEL_OUT)/arch/arm64/boot/dts/autochips/ac8x-emu-z1.dtb $(TARGET_OUT_DIR)
	@echo "Z1 Kernel build done ->:)"

kernel-s1: kernel-clean kernel-config
	make -C $(KERNEL_SRC) ARCH=$(KARCH) O=$(KERNEL_OUT) ac8x-emu-s1_defconfig -j24
	make -C $(KERNEL_SRC) ARCH=$(KARCH) O=$(KERNEL_OUT) -j$(JOBS) -j24
	make -C $(KERNEL_SRC) ARCH=$(KARCH) O=$(KERNEL_OUT) autochips/ac8x-emu-s1.dtb -j24
	cp $(KERNEL_OUT)/arch/arm64/boot/Image $(TARGET_OUT_DIR)/kernel-s1.bin
	cp $(KERNEL_OUT)/arch/arm64/boot/dts/autochips/ac8x-emu-s1.dtb $(TARGET_OUT_DIR)
	@echo "S1 Kernel build done ->:)"

kernel-config:
	cd $(KERNEL_SRC)
	mkdir -p $(KERNEL_OUT)
	make -C $(KERNEL_SRC) ARCH=$(KARCH) O=$(KERNEL_OUT) $(CONFIGFILE) -j24

kernel-clean:
	rm -rf $(KERNEL_OUT)
