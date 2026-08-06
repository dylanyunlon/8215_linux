#!/bin/sh -e

echo "make domU1 zImage dtb and ramdisk to rootfs"
BOARD_NAME=$(sed -n \
           's/^BR2_DEFCONFIG=".*\/\(.*\)_defconfig"$/\1/p' \
           ${BR2_CONFIG})

COMMON_PATH=${TOPDIR}/../source/vendor/autochips/proprietary/build/configs/board/autochips/common/

if [ "$AC83XX_BOOT_DEVICE" == "nand" ]; then
	test -f ${TARGET_DIR}/etc/init.d/S01syslogd && sed -i 's|/data/atclog|/tmp/atclog|g' ${TARGET_DIR}/etc/init.d/S01syslogd
	test -f ${TARGET_DIR}/etc/syslog.conf && sed -i 's|/data/atclog|/tmp/atclog|g' ${TARGET_DIR}/etc/syslog.conf
	test -f ${TARGET_DIR}/usr/bin/strace  && rm -rf ${TARGET_DIR}/usr/bin/strace
	if [ "$ATC_AB_PARTITION_SUPPORT" == "true" ]; then
		if [ "$AC83XX_BOOT_DEVICE_SIZE" == "128" ]; then
			cp -f ${COMMON_PATH}/fstab_nand_ab_128 ${TARGET_DIR}/etc/fstab
		else
			cp -f ${COMMON_PATH}/fstab_nand_ab ${TARGET_DIR}/etc/fstab
		fi
	else
		cp -f ${COMMON_PATH}/fstab_nand ${TARGET_DIR}/etc/fstab
	fi
	cp -f ${COMMON_PATH}/e2fsck.sh ${TARGET_DIR}/usr/bin
else
	cp -f ${COMMON_PATH}/fstab ${TARGET_DIR}/etc/fstab
fi

if [ "${KERNEL_PANIC_REBOOT}" = "false" ]; then
	cp -f ${COMMON_PATH}/S38panic ${TARGET_DIR}/etc/init.d/
fi

cp -f ${COMMON_PATH}/ld.so.conf ${TARGET_DIR}/etc/
cp -f ${COMMON_PATH}/ld.so.cache ${TARGET_DIR}/etc/
cp -arf ${COMMON_PATH}/ldconfig  ${TARGET_DIR}/etc/
cp -f ${COMMON_PATH}/android-gadget-setup ${TARGET_DIR}/etc/
cp -f ${COMMON_PATH}/interfaces ${TARGET_DIR}/etc/network/interfaces
cp -f ${COMMON_PATH}/S30adb ${TARGET_DIR}/etc/init.d/
cp -f ${COMMON_PATH}/S33dbus ${TARGET_DIR}/etc/init.d/
test -f ${TARGET_DIR}/etc/init.d/S30dbus && rm -f  ${TARGET_DIR}/etc/init.d/S30dbus
test -f ${TARGET_DIR}/etc/init.d/aee_start && mv -f ${TARGET_DIR}/etc/init.d/aee_start ${TARGET_DIR}/etc/init.d/S65aee_start
test -f ${COMMON_PATH}/early_modutils.sh && cp -f ${COMMON_PATH}/early_modutils.sh ${TARGET_DIR}/etc/init.d/S00early_modutils
if [ "$ATC_AB_PARTITION_SUPPORT" == "false" ]; then
    test -f ${COMMON_PATH}/modutils.sh && cp -f ${COMMON_PATH}/modutils.sh ${TARGET_DIR}/etc/init.d/S70modutils
else
#Compatibility workaround for CL:268752.Prevent appmanager does not compile, cause gt9xx.ko not to load
    test -f ${COMMON_PATH}/modutils_ab_part_support.sh && cp -f ${COMMON_PATH}/modutils_ab_part_support.sh ${TARGET_DIR}/etc/init.d/S70modutils
fi
cp -f ${COMMON_PATH}/S60atcmount ${TARGET_DIR}/etc/init.d/
if [ "${ATC_SHOW_VBA}" = "true" ]; then
cp -f ${COMMON_PATH}/S00vba ${TARGET_DIR}/etc/init.d/
chmod 755 ${TARGET_DIR}/etc/init.d/S00vba
fi

if [[ "$ATC_AB_PARTITION_SUPPORT" == "false" && !("$AC83XX_BOOT_DEVICE" == "nand" && "$AC83XX_BOOT_DEVICE_SIZE" == "128") ]]; then
cp -f ${COMMON_PATH}/S39audiobtserver ${TARGET_DIR}/etc/init.d/
fi

if [ "$ATC_AB_PARTITION_SUPPORT" == "true" ]; then
	cp -f ${COMMON_PATH}/S40update ${TARGET_DIR}/etc/init.d/
	chmod 755 ${TARGET_DIR}/etc/init.d/S40update
fi

if [[ "$ATC_AB_PARTITION_SUPPORT" == "false" && !("$AC83XX_BOOT_DEVICE" == "nand" && "$AC83XX_BOOT_DEVICE_SIZE" == "128") ]]; then
chmod 755 ${TARGET_DIR}/etc/init.d/S39audiobtserver
fi

chmod 755 ${TARGET_DIR}/etc/init.d/S60atcmount

# CarPLay so/bin
if [[ "$AC83XX_BOOT_DEVICE" == "nand" && "$AC83XX_BOOT_DEVICE_SIZE" == "128" ]]; then
	test -f ${TARGET_DIR}/usr/lib/libconfig.so && rm -rf ${TARGET_DIR}/usr/lib/libconfig.so
	test -f ${TARGET_DIR}/usr/lib/libconfig.so.11 && rm -rf ${TARGET_DIR}/usr/lib/libconfig.so.11
	test -f ${TARGET_DIR}/usr/lib/libconfig.so.11.0.2 && rm -rf ${TARGET_DIR}/usr/lib/libconfig.so.11.0.2
	test -f ${TARGET_DIR}/usr/lib/libconfig++.so && rm -rf ${TARGET_DIR}/usr/lib/libconfig++.so
	test -f ${TARGET_DIR}/usr/lib/libconfig++.so.11 && rm -rf ${TARGET_DIR}/usr/lib/libconfig++.so.11
	test -f ${TARGET_DIR}/usr/lib/libconfig++.so.11.0.2 && rm -rf ${TARGET_DIR}/usr/lib/libconfig++.so.11.0.2

	test -f ${TARGET_DIR}/usr/lib/libprotobuf.so && rm -rf ${TARGET_DIR}/usr/lib/libprotobuf.so
	test -f ${TARGET_DIR}/usr/lib/libprotobuf.so.16 && rm -rf ${TARGET_DIR}/usr/lib/libprotobuf.so.16
	test -f ${TARGET_DIR}/usr/lib/libprotobuf.so.16.0.0 && rm -rf ${TARGET_DIR}/usr/lib/libprotobuf.so.16.0.0
	test -f ${TARGET_DIR}/usr/lib/libprotobuf-lite.so && rm -rf ${TARGET_DIR}/usr/lib/libprotobuf-lite.so
	test -f ${TARGET_DIR}/usr/lib/libprotobuf-lite.so.16 && rm -rf ${TARGET_DIR}/usr/lib/libprotobuf-lite.so.16
	test -f ${TARGET_DIR}/usr/lib/libprotobuf-lite.so.16.0.0 && rm -rf ${TARGET_DIR}/usr/lib/libprotobuf-lite.so.16.0.0
	test -f ${TARGET_DIR}/usr/lib/libprotoc.so && rm -rf ${TARGET_DIR}/usr/lib/libprotoc.so
	test -f ${TARGET_DIR}/usr/lib/libprotoc.so.16 && rm -rf ${TARGET_DIR}/usr/lib/libprotoc.so.16
	test -f ${TARGET_DIR}/usr/lib/libprotoc.so.16.0.0 && rm -rf ${TARGET_DIR}/usr/lib/libprotoc.so.16.0.0
	test -f ${TARGET_DIR}/usr/bin/protoc && rm -rf ${TARGET_DIR}/usr/bin/protoc

	test -f ${TARGET_DIR}/usr/lib/libcoreutils && rm -rf ${TARGET_DIR}/usr/lib/libcoreutils.so
	test -f ${TARGET_DIR}/usr/lib/libcarplayplugin.so && rm -rf ${TARGET_DIR}/usr/lib/libcarplayplugin.so

	test -f ${TARGET_DIR}/usr/lib/libaccessoryinfo.so && rm -rf ${TARGET_DIR}/usr/lib/libaccessoryinfo.so
	test -f ${TARGET_DIR}/usr/lib/libMFICoprocessor.so && rm -rf ${TARGET_DIR}/usr/lib/libMFICoprocessor.so
	test -d ${TARGET_DIR}/etc/carplay && rm -rf ${TARGET_DIR}/etc/carplay

	test -f ${TARGET_DIR}/usr/lib/libcarlinkclient.so && rm -rf ${TARGET_DIR}/usr/lib/libcarlinkclient.so
	test -f ${TARGET_DIR}/usr/lib/libusbg.so && rm -rf ${TARGET_DIR}/usr/lib/libusbg.so
	test -f ${TARGET_DIR}/usr/lib/libcarlinkimpl.so && rm -rf ${TARGET_DIR}/usr/lib/libcarlinkimpl.so
	test -f ${TARGET_DIR}/usr/bin/carlinkmanager && rm -rf ${TARGET_DIR}/usr/bin/carlinkmanager
	test -f ${TARGET_DIR}/usr/bin/carlink.sh && rm -rf ${TARGET_DIR}/usr/bin/carlink.sh

	test -f ${TARGET_DIR}/usr/lib/libcarlinkutils.so && rm -rf ${TARGET_DIR}/usr/lib/libcarlinkutils.so
	test -f ${TARGET_DIR}/usr/lib/libfdk-aac.so && rm -rf ${TARGET_DIR}/usr/lib/libfdk-aac.so
	test -f ${TARGET_DIR}/usr/lib/libfdk-aac.so.2 && rm -rf ${TARGET_DIR}/usr/lib/libfdk-aac.so.2
	test -f ${TARGET_DIR}/usr/lib/libfdk-aac.so.2.0.0 && rm -rf ${TARGET_DIR}/usr/lib/libfdk-aac.so.2.0.0
	test -f ${TARGET_DIR}/usr/lib/libopus.so && rm -rf ${TARGET_DIR}/usr/lib/libopus.so
	test -f ${TARGET_DIR}/usr/lib/libopus.so.0 && rm -rf ${TARGET_DIR}/usr/lib/libopus.so.0
	test -f ${TARGET_DIR}/usr/lib/libopus.so.0.7.0 && rm -rf ${TARGET_DIR}/usr/lib/libopus.so.0.7.0

	test -f ${TARGET_DIR}/usr/bin/carplaymanagerservice && rm -rf ${TARGET_DIR}/usr/bin/carplaymanagerservice
	test -f ${TARGET_DIR}/usr/bin/carplaymanager.sh && rm -rf ${TARGET_DIR}/usr/bin/carplaymanager.sh
	sed -i 's/^carm:/#carm:/' ${TARGET_DIR}/etc/inittab

	test -f ${TARGET_DIR}/usr/lib/libcarplaypluginclient.so && rm -rf ${TARGET_DIR}/usr/lib/libcarplaypluginclient.so
	test -f ${TARGET_DIR}/usr/lib/libcarplayplugincommon.so && rm -rf ${TARGET_DIR}/usr/lib/libcarplayplugincommon.so
	test -f ${TARGET_DIR}/usr/lib/libcarplayhid.so && rm -rf ${TARGET_DIR}/usr/lib/libcarplayhid.so
	test -f ${TARGET_DIR}/usr/lib/libcarplay_av_core.so && rm -rf ${TARGET_DIR}/usr/lib/libcarplay_av_core.so
	test -f ${TARGET_DIR}/usr/lib/libcarplay_plugin_impl.so && rm -rf ${TARGET_DIR}/usr/lib/libcarplay_plugin_impl.so
	test -f ${TARGET_DIR}/usr/lib/libaudioconverter.so && rm -rf ${TARGET_DIR}/usr/lib/libaudioconverter.so
	test -f ${TARGET_DIR}/usr/lib/libaudiostream.so && rm -rf ${TARGET_DIR}/usr/lib/libaudiostream.so
	test -f ${TARGET_DIR}/usr/lib/libscreenstream.so && rm -rf ${TARGET_DIR}/usr/lib/libscreenstream.so
	test -f ${TARGET_DIR}/usr/bin/carplaypluginservice && rm -rf ${TARGET_DIR}/usr/bin/carplaypluginservice
	test -f ${TARGET_DIR}/usr/bin/carplayplugin.sh && rm -rf ${TARGET_DIR}/usr/bin/carplayplugin.sh
	sed -i 's/^plug:/#plug:/' ${TARGET_DIR}/etc/inittab

	test -f ${TARGET_DIR}/usr/lib/libcarplayclient.so && rm -rf ${TARGET_DIR}/usr/lib/libcarplayclient.so
	test -f ${TARGET_DIR}/usr/lib/libcarplaycommon.so && rm -rf ${TARGET_DIR}/usr/lib/libcarplaycommon.so
	test -f ${TARGET_DIR}/usr/bin/carplayservice && rm -rf ${TARGET_DIR}/usr/bin/carplayservice

	test -f ${TARGET_DIR}/usr/lib/libcarplayclienttestsclient.so && rm -rf ${TARGET_DIR}/usr/lib/libcarplayclienttestsclient.so

	test -f ${TARGET_DIR}/usr/lib/libiapclient.so && rm -rf ${TARGET_DIR}/usr/lib/libiapclient.so
	test -f ${TARGET_DIR}/usr/lib/libiapimpl.so && rm -rf ${TARGET_DIR}/usr/lib/libiapimpl.so
	test -f ${TARGET_DIR}/usr/bin/iap && rm -rf ${TARGET_DIR}/usr/bin/iap
	test -f ${TARGET_DIR}/usr/bin/iap.sh && rm -rf ${TARGET_DIR}/usr/bin/iap.sh
	sed -i 's/^iap:/#iap:/' ${TARGET_DIR}/etc/inittab

	test -f ${TARGET_DIR}/usr/lib/libmdnssd.so && rm -rf ${TARGET_DIR}/usr/lib/libmdnssd.so
	test -f ${TARGET_DIR}/usr/bin/mdnsd && rm -rf ${TARGET_DIR}/usr/bin/mdnsd

	test -f ${TARGET_DIR}/usr/bin/iperf && rm -rf ${TARGET_DIR}/usr/bin/iperf
else
	test -f ${TARGET_DIR}/usr/lib/libconfig++.so && rm -rf ${TARGET_DIR}/usr/lib/libconfig++.so
	test -f ${TARGET_DIR}/usr/lib/libconfig++.so.11 && rm -rf ${TARGET_DIR}/usr/lib/libconfig++.so.11
	test -f ${TARGET_DIR}/usr/lib/libconfig++.so.11.0.2 && rm -rf ${TARGET_DIR}/usr/lib/libconfig++.so.11.0.2

	test -f ${TARGET_DIR}/usr/lib/libprotoc.so && rm -rf ${TARGET_DIR}/usr/lib/libprotoc.so
	test -f ${TARGET_DIR}/usr/lib/libprotoc.so.16 && rm -rf ${TARGET_DIR}/usr/lib/libprotoc.so.16
	test -f ${TARGET_DIR}/usr/lib/libprotoc.so.16.0.0 && rm -rf ${TARGET_DIR}/usr/lib/libprotoc.so.16.0.0
	test -f ${TARGET_DIR}/usr/bin/protoc && rm -rf ${TARGET_DIR}/usr/bin/protoc
fi

sh ${COMMON_PATH}/filter_rootfs.sh
test  -d ${TARGET_DIR}/etc/xdg || mkdir ${TARGET_DIR}/etc/xdg
test  -d ${TARGET_DIR}/data/ || mkdir ${TARGET_DIR}/data/
cp -f ${COMMON_PATH}/vba/* ${TARGET_DIR}/etc/xdg/
chmod 755 ${TARGET_DIR}/etc/init.d/*
echo "dbus:x:103:103:D-Bus Message Daemon:/var/run/dbus:/usr/sbin/nologin" >> ${TARGET_DIR}/etc/passwd
echo "dbus:x:103:" >> ${TARGET_DIR}/etc/group
echo "export DBUS_SESSION_BUS_ADDRESS=\"unix:abstract=/tmp/dbus-service\"" >> ${TARGET_DIR}/etc/profile

