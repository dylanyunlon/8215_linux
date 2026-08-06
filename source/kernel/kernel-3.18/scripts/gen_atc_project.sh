#!/bin/sh

##
##
## param:
##   $1  defconfig file path
##   $2  output path
##   $3  project name
##   $4  platform name
##   $5  os
##   $6  os version
##   $7  customer name
##   $8  device
##   $9  board

function help()
{
	echo -e "Param introduction:"
	echo -e "\t"
	echo -e "\t 1  defconfig file path"
	echo -e "\t 2  outdir path"
	echo -e "\t 3  project name"
	echo -e "\t 4  platform name"
	echo -e "\t 5  OS name"
	echo -e "\t 6  OS Version"
	echo -e "\t 7  customer name"
	echo -e "\t 8  device name(chip)"
	echo -e "\t 9  board name"
	echo -e "\t"
}

result=
function checkscope()
{
    result=$(echo $2 | grep "$1")    
}

if [ $# != 9 ]; then
    echo "you must check atc project config. $#"
    help
    exit 1
fi

CONFIGFILE=$1
OUTPATH=$2
PROJECTNAME=$3
PLATFORMNAME=$4
OSNAME=$5
OSVERSION=$6
CUSTOMER=$7
DEVICENAME=$8
BOARDNAME=$9

result=
checkscope $OSNAME "android linux"
if [ "$result" = "" ]; then
    echo "param check fail1."
    exit 1
fi

result=
checkscope $PLATFORMNAME "ac83xx ac823x"
if [ "$result" = "" ]; then
    echo "param check fail2."
    exit 1
fi

echo " Now, we compile kernel with config as follow: "
echo "  defconfig : $CONFIGFILE"
echo "  outpath   : $OUTPATH"
echo "  project   : $PROJECTNAME"
echo "  platform  : $PLATFORMNAME"
echo "  device    : $DEVICENAME"
echo "  customer  : $CUSTOMER"
echo "  OS        : $OSNAME"
echo "  os ver    : $OSVERSION"

if [ ! -d $OUTPATH/include/generated ]; then
    mkdir -p $OUTPATH/include/generated
fi
HEADFILE=$OUTPATH/include/generated/atc_project.h



cat<<EOF>$HEADFILE
/*Note, this header file was generated automaticlly, so don't revise it by yourself.
#
#  Define project related macro for compile
#  CONFIG_ATC_OS: define which os kernel for. android or linux
#  CONFIG_ATC_OS_VER: define the version of os. e.p m
#  CONFIG_ATC_PLATFORM: define atc platform name, example ac83xx or ac823x
#  CONFIG_ATC_BOARD:    define atc board, example evb or demo board
#  CONFIG_ATC_CUSTOM:   define which custom that kernel compile for, default is atc */

#ifndef __ATC_PROJECT_H__
#define __ATC_PROJECT_H__



#define CONFIG_ATC_OS_$OSNAME
#define CONFIG_ATC_OS_VER_$OSVERSION
#define CONFIG_ATC_PLATFORM_$PLATFORMNAME
#define CONFIG_ATC_PRJ_$PROJECTNAME
#define CONFIG_ATC_BOARD_$BOARDNAME
#define CONFIG_ATC_CUSTOM_$CUSTOMER
#define CONFIG_ATC_ARCH_CHIP_$DEVICENAME

#endif // __ATC_PROJECT_H__

EOF



