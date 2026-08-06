#!/bin/bash


echo "Start to build the Hypervisor Project"
TOPDIR=$PWD
DOM0DIR=$TOPDIR/dom0
DOMUDIR=$TOPDIR/domU
OUTPUTDIR=$TOPDIR/images

ALL_ARG=$*
_find_string() {
 	find_o_str=0
 	find_p_str=0
	for arg in $ALL_ARG
	do
		if [ $find_o_str == "1" ];then
			param0=$arg
			break
		fi

		if [ $find_p_str == "1" ];then
			platform=$arg
			break
		fi


		if [ $arg == $1 ];then
			if [ $1 == -o ]; then
				find_o_str=1
			fi
			if [ $1 == -p ]; then
				find_p_str=1
			fi
		fi
	done
}

_find_string -o

_find_string -p

echo "param0 = $param0"
echo "platform = $platform"

domu_build=""
if [ x"$param0" = x"full" ]; then
        dom0_build="true"
        domu_build="true"
elif [ x"$param0" = x"dom0" ]; then
        dom0_build="true"
elif [ x"$param0" = x"domu" ]; then
        domu_build="true"
else
        dom0_build="true"
        domu_build="true"
fi

if [ ! -e $DOM0DIR/buildroot/output/images/system_ac8015-xen.img ]; then
        dom0_build="true"
        echo "the system_ac8015-xen.img is not exist and need to build dom0"
elif [ ! -e $DOMUDIR/release_images/system.img ]; then
        domu_build="true"
        echo "the system.img is not exist and need to build domu"
fi

if [ -e $DOM0DIR/buildroot/.config ]; then
        rm $DOM0DIR/buildroot/.config
fi
_dom0_release() {
        dom0_file="
    xen.bin \
    ac8x-xen.dtb \
    ac8x-xen_demo.dtb \
    Image_ac8015-xen.bin \
    system_ac8015-xen.img \
    system_ac8015-xen-single.img \
    ac8x_logo.mrf \
    logo_1024_600_1024_600.mrf \
    logo_768_600_1024_600.mrf \
    sfdis.bin \
    ramdisk.img \
    scatter.mmcboot.ext4.xml \
    scatter.mmcboot.ext4-demo.xml \
    scatter.mmcboot.ext4-single.xml \
    scatter.mmcboot.ext4-demo-single.xml \
    ATCUpgradeTool"
    for FILE in $dom0_file
    do
        if [ -e $DOM0DIR/buildroot/output/images/$FILE ]; then
            cp $DOM0DIR/buildroot/output/images/$FILE $OUTPUTDIR/ -rf
        else
            echo  "the file $FILE is not exist ,and need to rebuild dom0  or build full"
            #exit 1
        fi
    done
}

_build_dom0() {
        echo "Step one: Build the dom0 images ......"
        echo "$TOPDIR, $domu_build, "
        if [ x"$platform" = x"single" ]; then
            cd $DOM0DIR && make platform=D1 -j24 ;
        else
            cd $DOM0DIR && make -j24 ;
        fi
        if [ $? != 0 ]; then
                echo  "Build dom0 error, please check the log"
                exit 1
        fi
        cd $TOPDIR
        _dom0_release
        echo "dom0 been built done"
}

_domu_release() {
        domu_file="
    hl.bin \
    hsm.bin \
    viss.bin \
    viss_lpddr4_demo.bin \
    vdec.fwb \
    trustzone.bin \
    lk.bin \
    cache.img \
    Image.bin \
    vendor.img \
    system.img \
    userdata.img \
    avm.img \
    metazone.bin \
    ac8x.dtb \
    ac8x_demo_hypv.dtb "
    for FILE in $domu_file
    do
        if [ -e $DOMUDIR/release_images/$FILE ]; then
            cp $DOMUDIR/release_images/$FILE $OUTPUTDIR/  -rf
        else
            echo  "the file $FILE is not exist ,and need to rebuild domu or build full"
            #exit 1
        fi
    done
}


_build_domu() {
        echo "Step two: Build the domU images ......"
        if [ x"$platform" = x"single" ]; then
            cd $DOMUDIR && ./allmake.sh -p ac8x_car_hypv_D1
        else
            cd $DOMUDIR && ./allmake.sh -p ac8x_car_hypv
        fi
        if [ $? != 0 ]; then
                echo  "Build domU error, please check the log"
                exit 1
        fi
        cd $TOPDIR
        _domu_release
        echo "domU been built done"
}


_build(){
        mkdir $OUTPUTDIR
        if [  x"$dom0_build" = x"true" ] && [ x"$domu_build" = x"true" ]; then
                echo "build dom0 and domu "
                _build_dom0
                _build_domu
                exit 0
        fi
        if [ x"$dom0_build" = x"true" ]; then
                _build_dom0
                echo "Only build dom0 and it's success :)"
                echo "just copy the last domu images"
                _domu_release
                exit 0
        fi
        if [ x"$domu_build" = x"true" ]; then
                #echo "just copy the last dom0 images"
                _dom0_release
                _build_domu
                echo "Only build domu and it's success :)"
                exit 0
        fi
}

_build 2>&1 | tee $TOPDIR/hypervisor_build.log
