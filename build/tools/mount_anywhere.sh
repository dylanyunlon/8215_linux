
#
# Try to mount external SD or USB Mass storage device 
# to anywhere you want
#

#
# color_print - Print string with color
# @$1: specify color
# @$2: specify string which is printed with specified color
#
# -e  		enable interpretation of backslash escapes
# \e or \033 	output escapes
# format: 	\e[background_color;forground_color;HIGHLIGHTm
# default: 	\e[0m
####################################
# black		\033[30m
# red		\033[31m
# green		\033[32m
# yellow 	\033[33m
# blue		\033[34m
# purple 	\033[35m
# cyan 		\033[36m
# white		\033[37m
# reset		\033[0m
####################################
function color_print() {
	while (( $# != 0 ))
	do
		case $1 in
			-black)
				echo -ne "\033[30m";
			;;
			-red)
				echo -ne "\033[31m";
			;;
			-green)
				echo -ne "\033[32m";
			;;
			-yellow)
				echo -ne "\033[33m";
			;;
			-blue)
				echo -ne "\033[34m";
			;;
			-purple)
				echo -ne "\033[35m";
			;;
			-cyan)
				echo -ne "\033[36m";
			;;
			-white)
				echo -ne "\033[37m";
			;;
			-h|-help|--help)
				echo "Usage: color_print -color string";
				echo "Example: color_print -red red -green green";
			;;
			*)
				echo -e "$1\033[0m"
			;;
		esac
		shift
	done
}

#
# usage - print usage of this scripts
#
function usage() {
  color_print -blue "Usage: any_mount.sh sd <mount_point>"
  color_print -blue "Usage: any_mount.sh usb <mount_point>"
  color_print -blue "Usage: any_mount.sh switch" 
}

#
# kill_process - kill all processes which access speicified dir
# $1:  dir 
#
function kill_process() {
  local DIR=$1
  color_print -red "proceses access $DIR get killed" 
  /system/bin/busybox fuser -m $DIR
  /system/bin/busybox fuser -m -k $DIR   
}

#
# make_dir - create dir for mounting device
# $1:  dir
#
function make_dir() {
 local MOUNTPOINT=$1
 mkdir -p $MOUNTPOINT
}

#
# check_mount -  check dir to see if it is used for mount point
# $1: dir
#
function check_mount() {
local RET=true
local MOUNTPOINT=$1

RET=`mount | /system/bin/busybox grep "$MOUNTPOINT"`

if [ -n $RET ];then
  color_print -blue "$MOUNTPOINT is not mounted"
  return 0
else
  color_print -blue "$MOUNTPOINT is mounted"
  return 1
fi

}

#
# umount_device - umount device on specified dir
# $1: dir
#
function umount_device() {
local RET=true
local MOUNT_POINT=$1

while($RET)
do
  check_mount "$MOUNT_POINT"
  if [ $? -eq 1 ];then
     kill_process $MOUNT_POINT
     `umount $MOUNT_POINT`
  else
     RET=false
  fi
done

}

EXTERNAL_SD=/dev/block/mmcblk0p1
EXTERNAL_USB=/dev/block/sda1
MOUNT_POINT_SD=/mnt/ext_sdcard
MOUNT_POINT_USB=/mnt/udisk
MOUNT_POINT_SYSTEM=/system
SYSTEM_BLOCK=/dev/block/mtkd7
VOLD_FSTAB=/system/etc/vold.fstab
TMP=/data/.tmp
SWITCH_MARK=/data/.switched
SWITCH_FSTAB=/system/etc/switch.fstab

function main() {
  local OBJECT=$1
  local DIRECTORY=$2

  case $OBJECT in
    sd)
      make_dir $DIRECTORY
      if [ `echo $OBJECT | /system/bin/busybox grep sd` ];then
        if [ -e $EXTERNAL_SD ];then
          color_print -blue "umount external SD" 
          umount_device $MOUNT_POINT_SD
          color_print -blue "mount external SD $DIRECTORY"
          `mount -t vfat $EXTERNAL_SD $DIRECTORY`
        else
          color_print -red "Please insert SD" 
          exit 1
        fi
      fi
    ;;
    usb)
      make_dir $DIRECTORY
      if [ `echo $OBJECT | /system/bin/busybox grep usb` ];then
        if [ -e $EXTERNAL_USB ];then
          color_print -blue "umount usb" 
          `umount_device $MOUNT_POINT_USB`
          color_print -blue "mount usb $DIRECTORY"
          `mount -t vfat $EXTERNAL_USB $DIRECTORY`
        else
          color_print -red "Please insert USB" 
          exit 2
        fi
      fi
   ;;
    switch)
      color_print -blue "remount system"
      mount -o remount $SYSTEM_BLOCK $MOUNT_POINT_SYSTEM
      /system/bin/busybox mv $VOLD_FSTAB $TMP 
      color_print -blue "move $VOLD_FSTAB $TMP"
      /system/bin/busybox mv $SWITCH_FSTAB  $VOLD_FSTAB
      color_print -blue "move $SWITCH_FSTAB  $VOLD_FSTAB"
      if [ -e $SWITCH_MARK ];then
        color_print -blue " mark existed"
        /system/bin/busybox rm $SWITCH_MARK 
      else 
        color_print -blue " mark NOT existed"
        /system/bin/busybox touch $SWITCH_MARK
      fi
       /system/bin/busybox mv $TMP $SWITCH_FSTAB
        color_print -blue "move $TMP $VOLD_FSTAB"
       /system/bin/busybox sync
        color_print -red "Reboot Device"
       reboot
    ;;
    *)
      usage
      color_print -red "Invalid parameters...!"
    ;;
  esac
}

main $@
