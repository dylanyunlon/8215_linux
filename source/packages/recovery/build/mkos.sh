#!/bin/sh

#############################################################
#   constructe a simple rootfs for recovery
#############################################################

CP="cp -auf"
MKDIR="mkdir -p"
RM="rm -rf"
KSRC=$1
#SKIP_STEP="S1 S3"

#####################################
# Step 1: do a empty rootfs 
#####################################
if [ -z "`echo $SKIP_STEP | grep 'S1'`" ]; then
    echo "Step 1 setup empty rootfs Start~"
    if [ -d ${recovery_out} ]; then
        $RM ${recovery_out}
    fi
    $MKDIR ${recovery_out}

    tar xf ${recovery_os}/root.tar.gz -C ${recovery_out}
    mv ${recovery_out}/root ${recovery_out}/sysroot
    echo "Step 1 Done!"
else
    echo "Step1 has skip"
fi


######################################
# Step 2: install must be meta
######################################
if [ -z "`echo $SKIP_STEP | grep 'S2'`" ]; then
    echo "Step 2 install rootfs Start~"
    RECOVERY_METAS="glibc linux-libc-headers busybox qtbase libinput gcc libgcc opengles libpng atcimage libz glib libffi libxkbcommon udev dbus wayland osal expat"

    for d in $RECOVERY_METAS
    do
        echo "install $d ... ..."
        $CP ${recovery_os}/$d/image/* ${recovery_out}/sysroot
    done
    $CP ${recovery_os}/busybox/image/init ${recovery_out}/sysroot/etc/
    $CP ${recovery_os}/busybox/image/linuxrc ${recovery_out}/sysroot/etc/
    chmod 755  ${recovery_out}/sysroot/init
    chmod 777  ${recovery_out}/sysroot/etc/init.d/rcS
    chmod 755  ${recovery_out}/sysroot/sbin/klogd
    chmod 755  ${recovery_out}/sysroot/bin/sh


    $MKDIR ${recovery_out}/sysroot/usr/drivers
    kos=`find $KSRC -name atc_bl.ko -o -name imgresz.ko -o -name mtz_drv.ko -o -name atcfb.ko -o -name vcp.ko -o -name imgcommon.ko -o -name ump.ko -o -name ybr.ko -o -name hvsi_drv.ko -o -name drvcli.ko -o -name pdec.ko -o -name jdec.ko -o -name gdec.ko -o -name gt9xx.ko -o -name input.ko -o -name i2c-dev.ko ! -name xt_sctp.ko  -type f `

    for k in $kos
    do
        cp -raf $k ${recovery_out}/sysroot/usr/drivers 
    done
    $CP ${recovery_os}/sysconfig/* ${recovery_out}/sysroot/etc
    echo "Step 2 Done!"
else
    echo "Step2 has skip"
fi

exit 0




