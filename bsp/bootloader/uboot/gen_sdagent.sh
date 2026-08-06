#!/bin/sh
#


################################################################
#
#  usage :
#
# author: mtk94037
# date :Nov/07/2011
#################################################################
export CMD_SDAGENT=true
export CMD_MSDC_ETT=false
echo build boot loader
cd uboot-83xx
chmod +x mkconfig
./build.sh

if [ $? != "0" ]; then
    exit 1
fi

####################### merge preloader and u-boot ##############:
cp ./u-boot.bin ../tools/sd-agent.bin
