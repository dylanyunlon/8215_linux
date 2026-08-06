#!/bin/sh

#############################################################
#   constructe a simple rootfs for recovery
#############################################################

CP="cp -auf"
MKDIR="mkdir -p"
RM="rm -rf"
STRIP="arm-poky-linux-gnueabi-strip"
MKFS_BIN=${DA_TOP}/build/tools/make_ext4fs

SKIP_STEP="S1 S3"

#####################################
# Step 1: do a empty rootfs 
#####################################
if [ -z "`echo $SKIP_STEP | grep 'S1'`" ]; then
    echo "Step 1 setup empty rootfs Start~"
    if [ -d $PWD/out ]; then
        $RM $PWD/out
    fi
    $MKDIR $PWD/out

    tar xf os/root.tar.gz -C $PWD/out/

    ROOTFS_DIR=$PWD/out/root
    echo "Step 1 Done!"
else
    echo "Step1 has skip"
fi


######################################
# Step 2: install must be meta
######################################
if [ -z "`echo $SKIP_STEP | grep 'S2'`" ]; then
    echo "Step 2 install rootfs Start~"
    RECOVERY_METAS="glibc busybox qtbase libinput gcc libgcc opengles libpng atcimage libz glib wayland libffi"

    for d in $RECOVERY_METAS
    do
        echo "install $d ... ..."
        $CP os/$d/image/* out/root
    done

    $MKDIR out/root/usr/drivers
    $CP os/drivers/* out/root/usr/drivers

    $CP os/sysconfig/* out/root/etc
    echo "Step 2 Done!"
else
    echo "Step2 has skip"
fi
######################################
# Step 3: Backup a rootfs as sysroot
######################################
if [ -z "`echo $SKIP_STEP | grep 'S3'`" ]; then
    echo "Step 3 Backup sysroot Start~"
    $MKDIR out/sysroot
    $CP out/root/* out/sysroot
    echo "Step 3 Done!"
else
    echo "Step 3 has skip"
fi

######################################
# Step 4: Strip rootfs
######################################
echo "Step 4 Strip Start~"
du -sh out/root
includedirs=`find out/root -name include -type d`
for d in $includedirs
do
    echo "rm $d"
    $RM $d
done

docdirs=`find out/root -name doc -type d`
for d in $docdirs
do
    echo "rm $d"
    $RM $d
done

mandirs=`find out/root -name man -type d`
for d in $mandirs
do
    echo "rm $d"
    $RM $d
done

srcdirs=`find out/root -name src -type d`
for d in $srcdirs
do
    echo "rm $d"
    $RM $d
done

du -sh out/root

bigdirs="bin lib sbin usr/bin usr/lib usr/sbin"
for d in $bigdirs
do
    echo "@ $d, do strip~"

    fs=`find out/root/$d -name *.a -type f`
    for f in $fs
    do
        echo "rm $f"
        $RM $f
    done
    echo "*.a done"

    fs=`find out/root/$d -name *.o -type f`
    for f in $fs
    do
        echo "rm $f"
        $RM $f
    done
    echo "*.o done"

    fs=`find out/root/$d`
    for f in $fs
    do 
        if [ ! -z "`file $f | grep 'not stripped'`" ]; then
            echo "strip $f"
            $STRIP $f
        fi
    done
    echo "@ $d, stip done"
done

du -sh out/root

######################################
# Step 5: make ext4 image
######################################
$MKFS_BIN -s -l 130M recovery.img.ext4 out/root
if [ $? != 0 ]; then
    echo -e "\033[40;31m make system image fail \033[0m"
    exit 1
else
    echo -e "\033[44;32m make system image success \033[0m"
fi

ls -al

exit 0




