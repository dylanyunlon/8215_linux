#!/bin/sh

CP="cp -auf"
MKDIR="mkdir -p"
RM="rm -rf"
STRIP="arm-buildroot-linux-gnueabi-strip"
#MKFS_BIN=${recovery_top}/build/make_ext4fs
GENEXT2FS=${recovery_top}/../build/tools/genext2fs


# back sysroot
echo "  Backup sysroot Start~"
$MKDIR ${recovery_out}/root
$CP ${recovery_out}/sysroot/* ${recovery_out}/root
echo "  Done!"


#  strip rootfs
du -sh ${recovery_out}/root
includedirs=`find ${recovery_out}/root -name include -type d`
for d in $includedirs
do
    echo "rm $d"
    $RM $d
done

docdirs=`find ${recovery_out}/root -name doc -type d`
for d in $docdirs
do
    echo "rm $d"
    $RM $d
done

mandirs=`find ${recovery_out}/root -name man -type d`
for d in $mandirs
do
    echo "rm $d"
    $RM $d
done

srcdirs=`find ${recovery_out}/root -name src -type d`
for d in $srcdirs
do
    echo "rm $d"
    $RM $d
done

srcdirs=`find ${recovery_out}/root -name mkspec -type d`
for d in $srcdirs
do
    echo "rm $d"
    $RM $d
done

du -sh out/root

# split to small block to aviode var to long

bigdirs="bin lib sbin usr/bin usr/lib usr/sbin"
for d in $bigdirs
do
    echo "@ $d, do strip~"

    fs=`find ${recovery_out}/root/$d -name *.a -type f`
    for f in $fs
    do
        echo "rm $f"
        $RM $f
    done
    echo "*.a done"

    fs=`find ${recovery_out}/root/$d -name *.o -type f`
    for f in $fs
    do
        echo "rm $f"
        $RM $f
    done
    echo "*.o done"

    fs=`find ${recovery_out}/root/$d`
    for f in $fs
    do 
        if [ ! -z "`file $f | grep 'not stripped'`" ]; then
            echo "strip $f"
            $STRIP $f
        fi
    done
    echo "@ $d, stip done"
done
$STRIP --strip-debug ${recovery_out}/root/usr/drivers/*.*

# make ext4 image

#$GENEXT2FS -b 163840 -i 1024 tmp.rootfs.ext2 -d ${recovery_out}/root
chmod 777  ${recovery_out}/root/etc/init.d/rcS
chmod 777  ${recovery_out}/root/usr/bin/*
chmod 777  ${recovery_out}/root/lib/udev/*
chmod 777  ${recovery_out}/root/bin/*
chmod 777  ${recovery_out}/root/lib/*
chmod 777  ${recovery_out}/root/etc/init.d/rcS
chmod 777  ${recovery_out}/root/usr/bin/*
chmod 777  ${recovery_out}/root/lib/udev/*
chmod 777  ${recovery_out}/root/bin/chmod
chmod 777  ${recovery_out}/root/bin/test
chmod 777  ${recovery_out}/root/bin/ls
chmod 777  ${recovery_out}/root/bin/recovery-update.bin
chmod 777  ${recovery_out}/root/lib/librecovery.so

MKFS_BIN=${recovery_top}/build/make_ext4fs
$MKFS_BIN -l  50M -b 1024 tmp.img.ext4 ${recovery_out}/root

if [ $? != 0 ]; then
    echo -e "\033[40;31m mkfs image fail \033[0m"
    exit 1
else
    echo -e "\033[44;32m mkfs image success \033[0m"
fi

gzip -f -9 -c tmp.img.ext4 > recovery.gz                         #gzip it
if [ $? != 0 ]; then
    echo -e "\033[40;31m gzip ramdisk fail \033[0m"
    exit 1
else
    echo -e "\033[44;32m gzip ramdisk success \033[0m"
   # rm -rf tmp.img.ext4
fi


ls -al

