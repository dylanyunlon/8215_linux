#!/bin/bash

#	Setup sysroot by pre-build oss package
#	Wrote by Ke.Xu@autochips.com

#	If sysroot dir existed, exit.

if [ -d $DA_SYSROOT ]; then
	echo "DA_SYSROOT HAS EXIST"
	exit 0
fi


mkdir -p $DA_SYSROOT

#	First step, put tool chain sysroot to ourself sysroot
#
cp -rf /mtkoss/gnuarm/vfp_4.8.2_2.6.35_cortex-a9-ubuntu/i686/sysroot/* $DA_SYSROOT/

#	Second step, put oss to sysroot
#
for i in $OSS_PACKAGES
do
	pkg_version=`echo $i | awk -F- '{print $NF}'`
	pkg_name=${i%-*}
	if [ -z "$pkg_name" -o -z "$pkg_version" ]; then
		continue
	fi
	if [ ! -d $OSS_LIB_TOP/gnuarm-4.8.2_vfp/$pkg_name/$pkg_version ]; then
		echo "Look out! $pkg_name/$pkg_version folden not exist"
		continue
	fi
	echo -e "\t install "$pkg_name "-"$pkg_version " to sysroot"
	cp -rf $OSS_LIB_TOP/gnuarm-4.8.2_vfp/$pkg_name/$pkg_version/* $DA_SYSROOT/
done

#	Third step, put native target to sysroot
#

echo -e "\t install native target"
cp -rf $DA_TOP/lib/* $DA_SYSROOT/usr/

