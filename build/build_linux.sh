#!/bin/bash


#	Optional Build Linux System

BOOT_DEVICES="emmc nand"
DISPLAY_MODES="wayland eglfs"
FILE_SYSTEMS="ext4 ubi"
BUILD_TARGETS="all \
		kernel \
		application \
		uboot \
		arm2 \
		avin \
		misc \
		graphics \
		multimedia \
		connectivity \
		rootfs \
		bsp \
		clean_bsp \
		clean_kernel \
		clean_avin \
		clean_misc \
		clean_multimedia"

export BOOT_DEVICE=
export DISPLAY_MODE=
export FILE_SYSTEM=
export BUILD_TARGET=
export STRIP_FLAG=false
check_result=
function check_boot_device()
{
	p=$1
	if [ -z $p ]; then
		BOOT_DEVICE='emmc'; check_result=true; return
	fi	
	for i in $BOOT_DEVICES
	do	
		if [ x"$p" = "x$i" ]; then
		check_result=true; return
		fi
	done	
	check_result=false; return
}

function check_display_mode()
{
	p=$1
	if [ -z $p ]; then
		DISPLAY_MODE='eglfs'; check_result=true; return
	fi	
	for i in $DISPLAY_MODES
	do	
		if [ x"$p" = "x$i" ]; then
		check_result=true; return
		fi
	done	
	check_result=false; return
}

function check_file_system()
{
	p=$1
	if [ -z $p ]; then
		FILE_SYSTEM='ext4'; check_result=true; return
	fi	
	for i in $FILE_SYSTEMS
	do	
		if [ x"$p" = "x$i" ]; then
		check_result=true; return
		fi
	done	
	check_result=false; return
}

function check_build_target()
{
	p=$1
	if [ -z $p ]; then
		BUILD_TARGET='all'; check_result=true; return
	fi
	for i in $BUILD_TARGETS
	do	
		if [ x"$p" = "x$i" ]; then
		check_result=true; return
		fi
	done	
	check_result=false; return
}

#	-h: help
#	-b: bootdevice  NAND EMMC, default is EMMC
#	-D: Display from   wayland eglfs, default is eglfs
#	-f: File system (For uboot) ubi or ext4, default is ext4
HELP_INFO=
function help()
{
	echo -e "Usage:"
	echo -e "build_linux.sh [OPTION...]"
	echo -e "\t"
	echo -e "Help Options:"
	echo -e "\t-h, --help\t show help options"
	echo -e "\t"
	echo "Application Options:"
	echo -e "\t-b: bootdevice, from nand or emmc"
	echo -e "\t\t emmc                 ------- boot from emmc [default]"
	echo -e "\t\t nand                 ------- boot from nand"
	echo -e "\t"
	echo -e "\t-D: display mode: "
	echo -e "\t\t wayland              ------- wayland path [defalut]"
	echo -e "\t\t eglfs                ------- eglfs path"
	echo -e "\t"
	echo -e "\t-f: file system, just for build uboot and kernel at nand boot."
	echo -e "\t\t ext4                 ------- ext4 [default]"
	echo -e "\t\t ubi                  ------- ubi"
	echo -e "\t"
	echo -e "\t-t: build target, all target as follow: "
	echo -e "\t\t all                  ------- build all targets[default]"
	echo -e "\t\t kernel               ------- build kernel"
	echo -e "\t\t uboot                ------- build uboot"
	echo -e "\t\t arm2                 ------- build arm2"
	echo -e "\t\t avin                 ------- build avin module"
	echo -e "\t\t multimedia           ------- build multimedia module"
	echo -e "\t\t misc                 ------- build misc module"
	echo -e "\t\t connecttivity        ------- build connectivity module"
	echo -e "\t\t bsp                  ------- build include kernel uboot avin graphics multimedia misc..."
	echo -e "\t\t clean_kernel         ------- clean kernel"
	echo -e "\t\t clean_avin           ------- clean avin module"
	echo -e "\t\t clean_multimedia     ------- clean multimedia module"
	echo -e "\t\t clean_misc           ------- clean misc module"
	echo -e "\t\t clean_graphics       ------- clean graphics module"
	echo -e "\t\t clean_connectivity   ------- clean connectivity module"
}

while getopts b:D:f:hst: opt
do
	case "$opt" in
	h) HELP_INFO=true ;;
	b) BOOT_DEVICE=$OPTARG ;;
	D) DISPLAY_MODE=$OPTARG ;;
	f) FILE_SYSTEM=$OPTARG ;;
	t) BUILD_TARGET=$OPTARG ;;
	s) STRIP_FLAG=true ;;
	*) echo "Unknown option"; exit 1 ;;
	esac
done

if [ x"$HELP_INFO" = x"true" ]; then
	help
	exit 0
fi

check_boot_device  $BOOT_DEVICE
if test "$check_result" = "true"; then
	echo "check boot device pass"
else
	echo "please check -b param"
	exit 1
fi

check_display_mode  $DISPLAY_MODE
if test "$check_result" = "true"; then
	echo "check display mode pass"
else
	echo "please check -D param"
	exit 1
fi

check_file_system  $FILE_SYSTEM
if test "$check_result" = "true"; then
	echo "check file system pass"
else
	echo "please check -f param"
	exit 1
fi

check_build_target  $BUILD_TARGET
if test "$check_result" = "true"; then
	echo "check build target pass"
else
	echo "please check -t param"
	exit 1
fi


echo -e "build option as follow: "
echo -e "*******************************************"
echo -e "start to build: "
echo -e "\t boot target:"$BUILD_TARGET 
echo -e "\t display mode:"$DISPLAY_MODE
echo -e	"\t file system:"$FILE_SYSTEM 
echo -e "\t boot device:"$BOOT_DEVICE
echo -e "*******************************************"
if read -t 5 -p "Confirm? [y/n]:" confirm
then
	if [ "$confirm" = "n" ]; then
		exit 0
	fi
fi

LOGFILE=$DA_TOP/build/log/$BUILD_TARGET"-"$DISPLAY_MODE".log"


make $BUILD_TARGET ATC_BOOT_DEVICE=$BOOT_DEVICE ATC_DISPLAY_MODE=$DISPLAY_MODE ATC_FILE_SYSTEM=$FILE_SYSTEM STRIPFLAG=$STRIP_FLAG \
		2>&1 | tee $LOGFILE










