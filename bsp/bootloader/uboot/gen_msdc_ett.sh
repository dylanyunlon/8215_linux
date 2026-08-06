#!/bin/sh
#


################################################################
#
#  usage :
#
# author: mtk68019 Qingqi.Xia
# date :10/17/2014
#################################################################
export CMD_SDAGENT=false
export CMD_MSDC_ETT=true
echo start build msdc ett bootloader...
cd uboot-83xx
make clean
chmod +x mkconfig
./build.sh

if [ $? != "0" ]; then
    exit 1
fi

####################### merge preloader and u-boot ##############:
cp ./u-boot.bin ../tools/msdc_ett.bin
# cd ../tools
# chmod +x multibinmerge
# ./multibinmerge  ./   config-msdc-ett.ini ac83xx_bootloader_msdc_ett.bin