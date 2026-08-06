#!/bin/sh
#


################################################################
#
#  usage :
#
# author: mtk94037
# date :Nov/07/2011
#################################################################
source ../../env.sh
export CMD_SDAGENT=false
echo build boot loader
cd uboot-83xx
make clean
chmod +x mkconfig
./build.sh

if [ $? != "0" ]; then
    exit 1
fi

####################### merge preloader and u-boot ##############:
cp ./u-boot.bin ../tools
cd ../tools
chmod +x multibinmerge
if [ "$AC83XX_BOOT_DEVICE" =  "nand" ];then
./multibinmerge  ./   config.ini ac83xx_bootloader_ddr3.bin
elif [ "$AC83XX_BOOT_DEVICE" =  "sd2" ];then
./multibinmerge  ./   config-sd2.ini ac83xx_bootloader_ddr3.bin
else
./multibinmerge  ./   config-emmc.ini ac83xx_bootloader_ddr3.bin

fi
