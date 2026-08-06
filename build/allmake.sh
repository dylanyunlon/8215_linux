#!/bin/bash

TOPDIR=$PWD
OUTPUTDIR=$TOPDIR/release_images
ISOPACKAGE_DIR=$OUTPUTDIR/iso_package
BUILDROOT_DIR=$TOPDIR/buildroot
OUT_DIR=$TOPDIR/out
LOCAL_TARGET_DIR=$OUT_DIR/target/ac83xx
UNITTEST_FILE=$OUT_DIR/build/ac83xx/test-1.0.0/atc/test_suite.tar
PREBUILT_DIR=$TOPDIR/source/vendor/autochips/proprietary/tools/Bins
INCLUDE_PRELOADER=false
#### FUNCTION ######
### color print
cprint() {
    while (( $# != 0 ))
    do
        case $1 in
            -B)
                echo -ne "\033[30m";
            ;;
            -r)
                echo -ne "\033[31m";
            ;;
            -g)
                echo -ne "\033[32m";
            ;;
            -y)
                echo -ne "\033[33m";
            ;;
            -b)
                echo -ne "\033[34m";
            ;;
            -p)
                echo -ne "\033[35m";
            ;;
            -c)
                echo -ne "\033[36m";
            ;;
            -w)
                echo -ne "\033[37m";
            ;;
            -h|-help|--help)
                echo "Usage: cprint -color string";
            ;;
            *)
                echo -e "$1\033[0m"
            ;;
        esac
        shift
    done
}

#### debug is printed in blue
#### information is printed in green
#### error is printed in read

_info() {
    STR="$@"
    cprint -g "`echo $STR`"
}

_debug() {
    STR="$@"
    cprint -b "`echo $STR`"
}

_error() {
    STR="$@"
    cprint -r "`echo $STR`"
}

_setup() {
    _info "$FUNCNAME $# $@ START"
    source ./setup $@
    _info "$FUNCNAME $# $@ END"
}

_release() {
    rm -rf $OUTPUTDIR
    mkdir -p $OUTPUTDIR
    cp $TOPDIR/build/tools/ATCUpgradeTool  -arf $OUTPUTDIR
    cp $TOPDIR/build/tools/ATCGammaTool  -arf $OUTPUTDIR
    if [ "${AC83XX_BOOT_DEVICE}" = "nand" ]; then
        cp $TOPDIR/build/tools/fastboot_nand.bat -f $OUTPUTDIR
        cp $TOPDIR/build/tools/fastboot_nand_ab.bat -f $OUTPUTDIR
        _info "copy nand fastboot bat to release"
    fi
    cp $TOPDIR/build/tools/AutoUpgrade.py -f $OUTPUTDIR
    cp $TOPDIR/build/project/evb_8317/target/* -arf $OUTPUTDIR
    cp $TOPDIR/out/images/* -arf  $OUTPUTDIR
    cp $TOPDIR/out/config/boot_misc.bin -arf  $OUTPUTDIR
    test -f ${TOPDIR}/out/build/ac83xx/linux-local/vmlinux && cp $TOPDIR/out/build/ac83xx/linux-local/vmlinux -arf  $OUTPUTDIR
    if [ "${BUILD_TEST}" = "true" ]; then
        cp $UNITTEST_FILE -f $OUTPUTDIR
    fi
}

_generateota() {
    rm -rf $ISOPACKAGE_DIR
    mkdir -p $ISOPACKAGE_DIR

    if [ "${AC83XX_BOOT_DEVICE}" = "emmc" ]; then
        cp $OUTPUTDIR/scatter.mmcboot.ext4.xml $ISOPACKAGE_DIR
    else
        cp $OUTPUTDIR/scatter.nand.ext4.xml $ISOPACKAGE_DIR
    fi
    cp $OUTPUTDIR/u-boot.bin $ISOPACKAGE_DIR
    cp $OUTPUTDIR/tz.bin $ISOPACKAGE_DIR
    cp $OUTPUTDIR/arm2.bin $ISOPACKAGE_DIR
    cp $OUTPUTDIR/Image.bin $ISOPACKAGE_DIR
    cp $OUTPUTDIR/ac83xx.dtb.bin $ISOPACKAGE_DIR
    cp $OUTPUTDIR/system.img.ext4 $ISOPACKAGE_DIR
    cp $OUTPUTDIR/cluster_res.img $ISOPACKAGE_DIR
    cp $OUTPUTDIR/vba.bin $ISOPACKAGE_DIR
    cp $LOCAL_TARGET_DIR/etc/version $ISOPACKAGE_DIR

    if [ "$INCLUDE_PRELOADER" = "true" ]; then
        _info "Copying preloader images to ISO package directory"
        cp $OUTPUTDIR/83XX_Preloader_realchip_nand.bin $ISOPACKAGE_DIR
    else
        _info "Skipping preloader images (use -pl parameter to include them)"
    fi

    cd $ISOPACKAGE_DIR
    for FILE in `ls`
    do
        md5sum $FILE >> $ISOPACKAGE_DIR/image.md5
    done
    cd -

    ISO_CONTENT_DIR=$OUTPUTDIR/iso_content
    rm -rf $ISO_CONTENT_DIR
    mkdir -p $ISO_CONTENT_DIR

    cd $ISOPACKAGE_DIR
    # generate update.zip
    zip -r $ISO_CONTENT_DIR/update.zip .
    cd -

    if [ "${AC83XX_BOOT_DEVICE}" = "emmc" ]; then
        cp $OUTPUTDIR/scatter.mmcboot.ext4.xml $ISO_CONTENT_DIR/
    else
        cp $OUTPUTDIR/scatter.nand.ext4.xml $ISO_CONTENT_DIR/
    fi

    new_date=$(cat $ISOPACKAGE_DIR/version)
    echo "version:" $new_date

    mkisofs -o linux_"$new_date".iso -J -R -A -V -v $ISO_CONTENT_DIR
    mv linux_"$new_date".iso $OUTPUTDIR/

 ###package image files to iso for otadiff, and generate md5.
    echo "generate otadiff"
    OTA_BASE1=$OUTPUTDIR/otabase1
    OTA_BASE2=$OUTPUTDIR/otabase2

    mkdir -p $OTA_BASE1
    mkdir -p $OTA_BASE2

    cp -r "$ISOPACKAGE_DIR"/* "$OTA_BASE1"
    cp -r "$ISOPACKAGE_DIR"/* "$OTA_BASE2"

    OTADIFF_ISOPACKAGE=$OUTPUTDIR/linux_diff
    rm -rf $OTADIFF_ISOPACKAGE
    mkdir -p $OTADIFF_ISOPACKAGE

    chmod 777 $PWD/source/packages/ab_update/update/1.0/src/genota/bsdiff
    python3  $PWD/source/packages/ab_update/update/1.0/src/genota/8317_diff.py $OTA_BASE1 $OTA_BASE2 ${OUTPUTDIR}/linux-diff

    mv -vf  $PWD/linux_diff.iso ${OTADIFF_ISOPACKAGE}

    # clean file after generate ota
    rm -rf $OTA_BASE1
    rm -rf $OTA_BASE2
    rm -rf ${OUTPUTDIR}/linux-diff
    rm -rf $ISOPACKAGE_DIR
    rm -rf $ISO_CONTENT_DIR
}

_upload() {
    if [ -d $OUTPUTDIR ]; then
        _debug "$FUNCNAME upload images START"
        mtkbuild -d -o $OUTPUTDIR
        _debug "$FUNCNAME upload images END"
    fi
}

_build() {
	_info "Build the images ......"

	### generate atc_storage_patitiron to kernel include dir
	if [ "$AC83XX_BOOT_DEVICE" == "nand" ]; then
		if [ "$ATC_AB_PARTITION_SUPPORT" == "true" ]; then
			if [ "$AC83XX_BOOT_DEVICE_SIZE" == "128" ]; then
				perl ${TOPDIR}/build/tools/PartitionUtility/ParsePartition2Header.pl ${TOPDIR}/build/tools/Linux_Partition_Table_AC8317.xls  ${TOPDIR}/source/kernel/kernel-3.18/include/linux/mmc/atc_storage_partition.h nand_ext4_ab_128M
			elif [ "$AC83XX_BOOT_DEVICE_SIZE" == "256" ]; then
				perl ${TOPDIR}/build/tools/PartitionUtility/ParsePartition2Header.pl ${TOPDIR}/build/tools/Linux_Partition_Table_AC8317.xls  ${TOPDIR}/source/kernel/kernel-3.18/include/linux/mmc/atc_storage_partition.h nand_ext4_ab_256M
			else
				perl ${TOPDIR}/build/tools/PartitionUtility/ParsePartition2Header.pl ${TOPDIR}/build/tools/Linux_Partition_Table_AC8317.xls  ${TOPDIR}/source/kernel/kernel-3.18/include/linux/mmc/atc_storage_partition.h nand_ext4_ab_512M
			fi
		else
			if [ "$AC83XX_BOOT_DEVICE_SIZE" == "128" ]; then
				perl ${TOPDIR}/build/tools/PartitionUtility/ParsePartition2Header.pl ${TOPDIR}/build/tools/Linux_Partition_Table_AC8317.xls  ${TOPDIR}/source/kernel/kernel-3.18/include/linux/mmc/atc_storage_partition.h nand_ext4_128M
			else
				perl ${TOPDIR}/build/tools/PartitionUtility/ParsePartition2Header.pl ${TOPDIR}/build/tools/Linux_Partition_Table_AC8317.xls  ${TOPDIR}/source/kernel/kernel-3.18/include/linux/mmc/atc_storage_partition.h nand_ext4_256M
			fi
		fi
	else
		if [ "$ATC_AB_PARTITION_SUPPORT" == "true" ]; then
			perl ${TOPDIR}/build/tools/PartitionUtility/ParsePartition2Header.pl ${TOPDIR}/build/tools/Linux_Partition_Table_AC8317.xls  ${TOPDIR}/source/kernel/kernel-3.18/include/linux/mmc/atc_storage_partition.h emmc_ab
		else
			perl ${TOPDIR}/build/tools/PartitionUtility/ParsePartition2Header.pl ${TOPDIR}/build/tools/Linux_Partition_Table_AC8317.xls  ${TOPDIR}/source/kernel/kernel-3.18/include/linux/mmc/atc_storage_partition.h emmc
		fi
	fi
	make $OPT_DEF DEVICE=${PRODUCT_DEVICE} VARIANT=${ATC_DEBUG_TYPE} -j24 ;

	if [ $? != 0 ]; then
			_error "Build error, please check the log"
			exit 1
	fi
	cd $TOPDIR
	_release
	_info "been built done"
	_generateota
}

_start_info() {
    _info "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    _info "USER_ROOT=$USER_ROOT"
    _info "VARIANT=$ATC_DEBUG_TYPE"
    _info "PRODUCT_DEVICE=$PRODUCT_DEVICE"
    _info "MOUDLES=$OPT_DEF"
    _info "INCLUDE_PRELOADER=$INCLUDE_PRELOADER"
    _info "++++++++++++++++++++Build Started++++++++++++++++++++++++++++++++++++++"
}

_remove_file() {
    test -f ${LOCAL_TARGET_DIR}/etc/ld.so.cache && rm -rf ${LOCAL_TARGET_DIR}/etc/ld.so.cache
    test -f ${LOCAL_TARGET_DIR}/etc/ld.so.conf && rm -rf ${LOCAL_TARGET_DIR}/etc/ld.so.conf
    test -d ${LOCAL_TARGET_DIR}/etc/ldconfig && rm -rf ${LOCAL_TARGET_DIR}/etc/ldconfig
}

check_raw_partition_reserve() {
    local xml_file="$1"
    local img_dir="$2"
    local reserve_kb=1024   # 1MB

    echo "[INFO] Checking RAW partitions reserve..."
    echo "$xml_file : $img_dir"

    grep '<partition' "$xml_file" | grep 'type="raw"' | grep -Ev 'name="preloader(_bk)?"' | while read -r line
    do
        part_name=$(echo "$line" | grep -o 'name="[^"]*"' | head -n1 | cut -d'"' -f2)
        part_size_hex=$(echo "$line" | sed -n 's/.*size="\([^"]*\)".*/\1/p')
        image_name=$(echo "$line" | sed -n 's/.*imagename="\([^"]*\)".*/\1/p')

        # hex -> KB
        part_size_bytes=$((part_size_hex))
        part_size_kb=$((part_size_bytes / 1024))

        img_path="${img_dir}/${image_name}"
        if [ ! -f "$img_path" ]; then
            echo "[WARN] $part_name: image not found -> $img_path"
            continue
        fi

        img_size_kb=$(du -k "$img_path" | awk '{print $1}')
        limit_kb=$((part_size_kb - reserve_kb))

        echo "[CHECK] $part_name ($image_name): img=${img_size_kb}KB limit=${limit_kb}KB"

        if [ "$img_size_kb" -gt "$limit_kb" ]; then
            echo "[ERROR] $part_name overflow! need reserve 1MB"
            exit 1
        fi
    done
}

_main() {
    _info "$@ $#"
    for arg in "$@"; do
        if [ "$arg" = "pl" ]; then
            INCLUDE_PRELOADER=true
            _info "Including preloader images in ISO package"
        fi
    done

    if [ x"$1" = x"upload" ]; then
        _upload
        exit
    fi

    if [ x"$1" = x"release" ]; then
        _release
        exit
    fi
    local setup_args=()
    for arg in "$@"; do
        if [ "$arg" != "-pl" ]; then
            setup_args+=("$arg")
        fi
    done
    _setup $@
    _info "PRODUCT_DEVICE=${PRODUCT_DEVICE}"
    _start_info
    _remove_file
    _build $@ 2>&1 | tee build_${PRODUCT_DEVICE}.log
    if [ "${AC83XX_BOOT_DEVICE}" = "nand" ] && [ "$AC83XX_BOOT_DEVICE_SIZE" != "128" ]; then
        check_raw_partition_reserve $OUTPUTDIR/scatter.nand.ext4.xml $OUTPUTDIR
    fi
}
_main $@
#### TODO ####
