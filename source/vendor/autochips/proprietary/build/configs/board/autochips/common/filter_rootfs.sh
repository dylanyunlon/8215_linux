#! /bin/bash -e
#
# description: filter rootfs for nand 128M

ROOTFS_DIR=${TOPDIR}/../out/target/ac83xx

echo "filter rootfs for nand 128M"
if [ "$AC83XX_BOOT_DEVICE" == "nand" ]; then
	if [ "$AC83XX_BOOT_DEVICE_SIZE" == "128" ]; then
		if test -e ${ROOTFS_DIR}/lib/modules/3.18.49/kernel/drivers/btdrv ; then
			rm -rf ${ROOTFS_DIR}/lib/modules/3.18.49/kernel/drivers/btdrv
		fi
		if test -e ${ROOTFS_DIR}/usr/lib/libyajl.so ; then
			rm -rf ${ROOTFS_DIR}/usr/lib/libyajl.so*
		fi
		if test -e ${ROOTFS_DIR}/usr/lib/libmenu.so ; then
			rm -rf ${ROOTFS_DIR}/usr/lib/libmenu.so*
			rm -rf ${ROOTFS_DIR}/usr/lib/libfreetype*
			rm -rf ${ROOTFS_DIR}/usr/lib/libform.so*
			rm -rf ${ROOTFS_DIR}/usr/lib/libpanel.so*
		fi
		if test -e ${ROOTFS_DIR}/usr/bin/pcretest ; then
			rm -rf ${ROOTFS_DIR}/usr/bin/pcretest
		fi
		if test -e ${ROOTFS_DIR}/usr/bin/pcre2test ; then
			rm -rf ${ROOTFS_DIR}/usr/bin/pcre2test
		fi

	fi
fi
