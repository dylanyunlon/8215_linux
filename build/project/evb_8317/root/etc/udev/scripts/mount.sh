#!/bin/sh
#
# Called from udev
#
# Attempt to mount any added block devices and umount any removed devices


MOUNT="/bin/mount"
PMOUNT="/usr/bin/pmount"
UMOUNT="/bin/umount"
FUSER="/usr/bin/fuser"
BLKID="/usr/sbin/blkid"
DEVPATH="$1"
KERNELNUMBER="$2"
ID_FS_TYPE=""

#judge fstype is fat or not
fstype=`$BLKID $DEVNAME -s TYPE`
echo $fstype | grep "fat"
if [ $? -eq 0 ] ;
then 
	ID_FS_TYPE="vfat"
fi
#judge fstype is exfat or not
echo $fstype | grep "exfat"
if [ $? -eq 0 ] ;
then
        ID_FS_TYPE="exfat"
fi
#judge fstype is ntfs or not
echo $fstype | grep "ntfs"
if [ $? -eq 0 ] ;
then 
	ID_FS_TYPE="ntfs"
fi

echo ">$DEVNAME"
#get the partition number
#blkid_result return blkid command result. With -s UUID,to avoid hide partition
#udisk_sd_headname return devname without kernel number,such as "sdb1" return "sdb"
#partition_number return udisk or sdcard partition number
blkid_result="`$BLKID -s UUID`"
echo "$DEVNAME" | grep "sd"
if [ $? -eq 0 ] ;
then
	udisk_sd_headname="`expr substr "$DEVNAME" "6" "3"`"
else
	udisk_sd_headname="`expr substr "$DEVNAME" "6" "7"`"
fi
partition_number="`echo $blkid_result | grep -o $udisk_sd_headname | wc -l`"

#echo "$blkid_result"
#echo "123$udisk_sd_headname"
#echo "11$partition_number"

for line in `grep -v ^# /etc/udev/mount.blacklist`
do
	if [ ` expr match "$DEVNAME" "$line" ` -gt 0 ];
	then
		logger "udev/mount.sh" "[$DEVNAME] is blacklisted, ignoring"
		exit 0
	fi
done

automount() {
	name="`basename "$DEVNAME"`"
	
	udisk_sdcard="$1"
	#mountpoint="/"
	echo "auto mount "
		  		
	mountpoint="/media/$udisk_sdcard"
	echo 
	#! test -d "/media/$udisk_sdcard" && mkdir -p "media/$udisk_sdcard" #add symlink when mkdir
	! test -d "/media/$udisk_sdcard" && mkdir -p "media/$udisk_sdcard" 
	echo
	! test -d "/$udisk_sdcard" && ln -snf "/media/$udisk_sdcard/" "/$udisk_sdcard"

	if [ "$partition_number" -gt "1" ] ;
	then
		echo 
		! test -d "/media/$udisk_sdcard/"$udisk_sdcard"_partition$KERNELNUMBER" && mkdir -p "/media/$udisk_sdcard/"$udisk_sdcard"_partition$KERNELNUMBER"
		mountpoint="/media/$udisk_sdcard/"$udisk_sdcard"_partition$KERNELNUMBER"
	fi
       
	#fstype="vfat"
	# Silent util-linux's version of mounting auto
	if [ "x`readlink $MOUNT`" = "x/bin/mount.util-linux" ] ;
	then
		MOUNT="$MOUNT -o silent"
	fi
	
	# If filesystem type is vfat, change the ownership group to 'disk', and
	# grant it with  w/r/x permissions.
	case $ID_FS_TYPE in
	vfat|fat)
		MOUNT="$MOUNT -o utf8,umask=007,nosuid,noatime,nodiratime,gid=`awk -F':' '/^disk/{print $3}' /etc/group`,errors=continue"
		;;
	# TODO
	*)
		;;
	esac
	
	#mount different type (ntfs or vfat,et)
	if [ $ID_FS_TYPE = "ntfs" ] ;
	then 
		if ! "ntfs-3g" $DEVNAME "$mountpoint"
		then 
			#logger "mount.sh/automount" "$MOUNT -t auto $DEVNAME \"/run/media/$name\" failed!"
	                rm_dir "$mountpoint" 
		        ! test -d "/media/$udisk_sdcard" && rm -rf "/$udisk_sdcard"
		else 
			#! test -d "/$udisk_sdcard" && ln -s "/media/$udisk_sdcard/" "/"  #put this action befor mount action 
			logger "mount.sh/automount" "Auto-mount of [$mountpoint/$name] successful"
        	        touch "/tmp/.automount-$name"
		fi
	else
		if ! $MOUNT -t auto $DEVNAME "$mountpoint"
	        then
        	        #logger "mount.sh/automount" "$MOUNT -t auto $DEVNAME \"/run/media/$name\" failed!"
                	rm_dir "$mountpoint"
			! test -d "/media/$udisk_sdcard" && rm -rf "/$udisk_sdcard"
        	else
			#! test -d "/$udisk_sdcard" && ln -s "/media/$udisk_sdcard/" "/"  #put this action befor mount action
                	logger "mount.sh/automount" "Auto-mount of [$mountpoint/$name] successful"
                	touch "/tmp/.automount-$name"
        	fi
	fi
}

#udisk_sdcard means to dentify rigth name(udisk or sdcard) to the rigth slot or port number
udisk_sdcard=""
select_mountpoint() {	
	name="`basename "$DEVNAME"`"
	dirname=${name:0:2}
	#set flag number to mount on different port or slot number
	flag="1"
	echo "$dirname"
	echo $DEVPATH | grep "ac_usbh"
	if [ $? -eq 0 ] ;
	then
		echo $DEVPATH | grep "1-1.2"
		if [ $? -eq 0 ] ;
		then 
			udisk_sdcard="udisk5"
			flag="2"
		fi
		
		echo $DEVPATH | grep "1-1.3"
		if [ $? -eq 0 ] ;
		then
			udisk_sdcard="udisk3"
			flag="3"
		fi

		echo $DEVPATH | grep "1-1.4"
		if [ $? -eq 0 ] ;
		then
			udisk_sdcard="udisk4"
			flag="4"
		fi
		
		if [ $flag = "1" ] ;
		then
 			udisk_sdcard="udisk1"
		fi
	else
		echo $DEVPATH | grep "1-1:"
		if [ $? -eq 0 ] ;
		then 
			udisk_sdcard="udisk1"
		fi

		echo $DEVPATH | grep "2-1:"
		if [ $? -eq 0 ] ;
		then 
			udisk_sdcard="udisk2"
		fi
	fi

	echo $DEVPATH | grep "MSDC1"
	if [ $? -eq 0 ] ;
	then 
		udisk_sdcard="ext_sdcard1"
	fi

	echo $DEVPATH | grep "MSDC2"
	if [ $? -eq 0 ] ;
	then 
		udisk_sdcard="ext_sdcard2"
	fi		
}
	
rm_dir() {
	# We do not want to rm -r populated directories
	if test "`find "$1" | wc -l | tr -d " "`" -lt 2 -a -d "$1"
	then
		! test -z "$1" && rm -r "$1"
	else
		logger "mount.sh/automount" "Not removing non-empty directory [$1]"
	fi
}

# No ID_FS_TYPE for cdrom device, yet it should be mounted
echo "----------------------DEVNAME:" $DEVNAME
echo "----------------------ACTION:" $ACTION

name="`basename "$DEVNAME"`"
[ -e /sys/block/$name/device/media ] && media_type=`cat /sys/block/$name/device/media`

#if [ "$ACTION" = "add" ] && [ -n "$DEVNAME" ] && [ -n "$ID_FS_TYPE" -o "$media_type" = "cdrom" ]; then
if [ "$ACTION" = "add" ] && [ -n "$DEVNAME" ]; then
        echo "----------------------Mount Start:"
	if [ -x "$PMOUNT" ]; then
		$PMOUNT $DEVNAME 2> /dev/null
	elif [ -x $MOUNT ]; then
    		$MOUNT $DEVNAME 2> /dev/null
	fi
	
	# If the device isn't mounted at this point, it isn't
	# configured in fstab (note the root filesystem can show up as
	# /dev/root in /proc/mounts, so check the device number too)
	if expr $MAJOR "*" 256 + $MINOR != `stat -c %d /`; then
		grep -q "^$DEVNAME " /proc/mounts || select_mountpoint && automount "$udisk_sdcard"
	fi
        echo "----------------------Mount End"
fi


if [ "$ACTION" = "remove" ] || [ "$ACTION" = "change" ] && [ -x "$UMOUNT" ] && [ -n "$DEVNAME" ]; then
        echo "----------------------Unmount Start"
	
	#get mountpoint
	# Call select_mountpoint() func to set udisk_sdcard value
	select_mountpoint
	#remove symlink of mountpoint at first
	test -d "/$udisk_sdcard" && rm -rf "/$udisk_sdcard" && echo done
	for mnt in `cat /proc/mounts | grep "$DEVNAME" | cut -f 2 -d " " `
	do
		#$UMOUNT $mnt
		#if $UMOUNT $mnt &> /dev/null; then
		#	echo "umount fail"
		#else
		#	$FUSER -km $mnt
		#	$FUSER -m $mnt
		#	$UMOUNT $mnt
		#fi
		retries=1
		mount_flag="kill"
		while [ $retries -le 10 ]
		do 
			if $UMOUNT $mnt &> /dev/null; then
				echo "umount success for $retries times"
				retries=11
				mount_flag="success"
			else 
				echo "umount fail for $retries times"
				retries=$(($retries+1))
				sleep 1
			fi
		done
		
		if [ "$mount_flag" = "kill" ]; then
			echo "try to kill process"
			$FUSER -km $mnt
			$FUSER -m $mnt
			$UMOUNT $mnt
		fi
	done
	
	# Remove empty directories from auto-mounter
	
	if [ -d "/media/$udisk_sdcard/"$udisk_sdcard"_partition$KERNELNUMBER" ] ;
	then
		test -d "/media/$udisk_sdcard/"$udisk_sdcard"_partition$KERNELNUMBER" && rm_dir "/media/$udisk_sdcard/"$udisk_sdcard"_partition$KERNELNUMBER" 		
		content="`ls $udisk_sdcard`"
		#echo "content:$content"
		if [ "$content" = "" ]; 
		then
			#echo "/$udisk_sdcard is empty,remove it"
			test -d "/media/$udisk_sdcard" &&  rm_dir "/media/$udisk_sdcard"
		fi
	else 
		test -e "/tmp/.automount-$name" && test -d "/media/$udisk_sdcard" && rm_dir "/media/$udisk_sdcard"
	fi
        echo "----------------------Unmount End"
fi
