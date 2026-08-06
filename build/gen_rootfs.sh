#!/bin/bash


#
#        Generate rootfs, wrote by Ke Xu @ ATC
#
#

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${DA_TOP}/build/tools"

MKFS_BIN=${DA_TOP}/build/tools/make_ext4fs
#export MKFSEXT2=${DA_TOP}/build/tools/mkfs.ext2
MKFSEXT2=${DA_TOP}/build/tools/genext2fs
MKE2FS_CONFIG=${DA_TOP}/build/tools/etc
DEBUGFS_BIN=${DA_TOP}/build/tools/debugfs
MKFSUBI=${DA_TOP}/build/tools/mkfs.ubifs
UBINIZE=${DA_TOP}/build/tools/ubinize

export CP="cp -ra"
export MV="mv"

if [ ! -d $IMG_REL ]; then
    mkdir -p $IMG_REL
fi



#---------------------------- Update Scatter file --------------------------------
if [ "$BOOT_DEVICE" != "nand" ]; then
    perl ${DA_TOP}/build/tools/PartitionUtility/PartitionUtility.pl;
else
    perl ${DA_TOP}/build/tools/PartitionUtility/PartitionUtility_Nand_Ext4.pl;
fi

if [ "$BOOT_DEVICE" != "nand" ]; then
$CP ${DA_TOP}/build/tools/scatter.mmcboot.ext4.xml $IMG_REL/
else
$CP ${DA_TOP}/build/tools/scatter.nand.ext4.xml $IMG_REL/
fi
#
#
#
#------------------------------Collect all material to rootfs--------------------
#

echo -e "\033[44;32m  Step 3, install atc lib and app.....Skipped~~~~~\033[0m"
#$CP $DA_LIBDIR/app/* $ROOTFS_OUT/usr/app/
#mkdir -p $ROOTFS_OUT/usr/etc/
#$CP $DA_LIBDIR/etc/* $ROOTFS_OUT/usr/etc/ 

  
echo -e "\033[44;32m File collect Done, now to make rootfs......Skipped~~~~~ \033[0m"

#if [ "$STRIPFLAG" = "true" ]; then
#  python $BUILD_TOOLDIR/tasks/do_strip.py
#fi

#
#------------------------------Start do rootfs-----------------------------------
#
#
ls -al $ROOTFS_OUT

# move app to independent directory for make app image
rm -rf ${DA_OUTPUT}/app
mv -f $ROOTFS_OUT/usr/app ${DA_OUTPUT}/                          # mv app to other space
rm -rif $ROOTFS_OUT/usr/app                                      # force remove app directory in usr

#mv $ROOTFS_OUT/usr ${DA_OUTPUT}/usr                                # mv usr to other space


ls -al $ROOTFS_OUT

if [ "$ATC_FILE_SYSTEM"  == "ubi" ]; then
    echo "cp rcs_ubi ro rcS"
    cp ${DA_TOP}/build/tools/atc_ubiattach $ROOTFS_OUT/bin/
fi
#${MKFSEXT2} -F -i 4096 tmp.rootfs.ext2 -d $ROOTFS_OUT              #make ext2 rootfs image
#${MKFSEXT2} -b 4096 -i 1024 tmp.rootfs.ext2 -d $ROOTFS_OUT/         #make ext2 rootfs image

#if [ $? != 0 ]; then
#    echo -e "\033[40;31m mkfs image fail \033[0m"
#    exit 1
#else
#    echo -e "\033[44;32m mkfs image success \033[0m"
#fi

#gzip -f -9 -c tmp.rootfs.ext2 > ramdisk.gz                         #gzip it
#if [ $? != 0 ]; then
#    echo -e "\033[40;31m gzip ramdisk fail \033[0m"
#    exit 1
#else
#    echo -e "\033[44;32m gzip ramdisk success \033[0m"
#    rm -rf tmp.rootfs.ext2
#    mv ramdisk.gz $IMG_REL/
#fi


#
#---------------------------Make system image----------------------------------------
#
#

cp -rf $DEBUGFS_BIN $DA_OUTPUT/root/usr/bin
#$MV  ${DA_OUTPUT}/usr    $ROOTFS_OUT/usr                        #mv usr back

#$MKFS_BIN -s -l 280M -b 8192 system.img.ext4 $ROOTFS_OUT/usr         #make ext4 system image
if [ "$BOOT_DEVICE" != "nand" ]; then
    echo "make image system.img"
    $MKFS_BIN -s -l 1536M -b 1024 system.img.ext4 $ROOTFS_OUT/         #make ext4 system image
else
    if [ "$ATC_FILE_SYSTEM"  == "ubi" ]; then
        echo "Make system ubifs "
        cp  ${DA_TOP}/build/rcS_ubi $ROOTFS_OUT/etc/init.d/rcS
        $MKFSUBI -r $ROOTFS_OUT -m 2048 -e 126976 -c 2400 -o system.img
        $UBINIZE -o system.img.ubi -m 2048 -p 131072 -s 2048 ${DA_TOP}/build/system.cfg
        mv system.img.ubi $IMG_REL/
    else
        cp  ${DA_TOP}/build/rcS_ftl $ROOTFS_OUT/etc/init.d/rcS
        $MKFS_BIN -l 300M system.img.ext4 $ROOTFS_OUT/
    fi 
fi

if [ $? != 0 ]; then
    echo -e "\033[40;31m make system image fail \033[0m"
    exit 1
else
    echo -e "\033[44;32m make system image success \033[0m"
if [ "$ATC_FILE_SYSTEM"  != "ubi" ]; then
    mv system.img.ext4 $IMG_REL/
fi
fi

if [ "$ATC_FILE_SYSTEM"  != "ubi" ]; then
$MKFS_BIN -s -l 10M fake.img.ext4 $ROOTFS_OUT/boot         #make ext4 system image
if [ $? != 0 ]; then
    echo -e "\033[40;31m make fake image fail \033[0m"
    exit 1
else
    echo -e "\033[44;32m make fake image success \033[0m"
    mv fake.img.ext4 $IMG_REL/
fi
fi

# make app image
if [ "$BOOT_DEVICE" != "nand" ]; then
	$MKFS_BIN -s -l 50M app.img.ext4 ${DA_OUTPUT}/app         #make ext4 system image
else
if [ "$ATC_FILE_SYSTEM"  == "ubi" ]; then
    $MKFSUBI -r ${DA_OUTPUT}/app -m 2048 -e 126976 -c 480 -o app.img
    $UBINIZE -o app.img.ubi -m 2048 -p 131072 -s 2048 ${DA_TOP}/build/app.cfg
else
	$MKFS_BIN -l 50M app.img.ext4 ${DA_OUTPUT}/app  
fi  
fi

if [ $? != 0 ]; then
    echo -e "\033[40;31m make app image fail \033[0m"
    exit 1
else
if [ "$ATC_FILE_SYSTEM"  == "ubi" ]; then
    echo -e "\033[44;32m make app image success \033[0m"
    $CP app.img.ubi $IMG_REL/appbk.img.ubi
    mv app.img.ubi $IMG_REL/
else
    echo -e "\033[44;32m make app image success \033[0m"
    $CP app.img.ext4 $IMG_REL/appbk.img.ext4
    mv app.img.ext4 $IMG_REL/
fi

fi

exit 0

