#! /bin/sh


function main() {
    BOOTMODE=`getprop bootdevice`
    if [ $BOOTMODE = "nand" ];then
	echo "nand - umount data4write"
        umount /data4write
	echo "fsck data4write"
        fsck.ext4 /dev/block/mtkd16
	echo "mount data4write"
        mount -t ext4 /dev/block/mtkd16 /data4write
        echo 3 > /proc/sys/vm/drop_caches
    else
	echo "mmc - umount data4write"
	umount /data4write
	echo "fsck data4write"
	fsck.ext4 -p /dev/block/mmcblk0p2 > /mnt/sdcard/fsck.log 2>&1
	echo "mount data4write"
	mount -t ext4 /dev/block/mmcblk0p2 /data4write
	echo 3 > /proc/sys/vm/drop_caches
    fi
}

main $@
    
