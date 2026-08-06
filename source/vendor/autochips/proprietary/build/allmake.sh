#!/bin/bash

TOPDIR=$PWD
OUTPUTDIR=$TOPDIR/release_images
ISOPACKAGE_DIR=$OUTPUTDIR/iso_package
PRODUCT_DEVICE="ac8015-linux"
BUILDROOT_DIR=$TOPDIR/buildroot
OUT_DIR=$TOPDIR/out
LOCAL_TARGET_DIR=$OUT_DIR/target/ac8015-linux
PREBUILT_DIR=$TOPDIR/vendor/autochips/proprietary/tools/Bins

MOD_DEF=userdebug
OPT_DEF=_all

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
    if [ -e $OUT_DIR/.config ]; then
		rm $OUT_DIR/.config
	fi
    _info "$FUNCNAME $# $@ END"
}

_release() {
    _file=" \
	hl.bin \
	hl_verified.img \
	hl_efuse.bin \
	pubkey_efuse.bin \
	hsm.bin \
	viss.bin \
	viss_demo.bin \
	viss_ddr3_demo.bin \
	lk.bin \
	trustzone.bin \
	ac8x-linux.dtb \
	ac8x-linux_demo.dtb \
	Image_ac8015-linux.bin \
	system.img \
	avm.img \
	userdata.img \
	metazone.bin \
	sfdis.bin \
	ac8x_logo_1024_600_TX0.mrf \
	ac8x_logo_1024_600_TX0_Cluster.mrf \
	ac8x_logo_1920_720_TX0_Cluster.mrf \
	ac8x_logo_1920_1080_TX0_Cluster.mrf \
	scatter.mmcboot.ext4.xml \
	scatter.mmcboot.ext4-evb.xml \
	scatter.mmcboot.efuse.xml \
	ATCUpgradeTool"

    rm -rf $OUTPUTDIR
    mkdir -p $OUTPUTDIR

    for FILE in $_file
    do
        if [ -e $OUT_DIR/images/$FILE ]; then
            cp $OUT_DIR/images/$FILE $OUTPUTDIR/ -rf
        elif [ -e $PREBUILT_DIR/$FILE ]; then
            cp $PREBUILT_DIR/$FILE $OUTPUTDIR/ -rf
        else
            _error "the file $FILE is not exist ,and need to rebuild or build full"
            #exit 1
        fi
    done

    rm -rf $ISOPACKAGE_DIR
    mkdir -p $ISOPACKAGE_DIR

    cp $OUTPUTDIR/scatter.mmcboot.ext4.xml $ISOPACKAGE_DIR
    cp $OUTPUTDIR/hsm.bin $ISOPACKAGE_DIR
    cp $OUTPUTDIR/lk.bin $ISOPACKAGE_DIR
    cp $OUTPUTDIR/trustzone.bin $ISOPACKAGE_DIR
    cp $OUTPUTDIR/viss_demo.bin $ISOPACKAGE_DIR
    cp $OUTPUTDIR/Image_ac8015-linux.bin $ISOPACKAGE_DIR
    cp $OUTPUTDIR/ac8x-linux_demo.dtb $ISOPACKAGE_DIR
    cp $OUTPUTDIR/system.img $ISOPACKAGE_DIR
    cp $LOCAL_TARGET_DIR/etc/version $ISOPACKAGE_DIR

    cd $ISOPACKAGE_DIR
    for FILE in `ls`
    do
        md5sum $FILE >> $ISOPACKAGE_DIR/image.md5
    done
    cd -

    new_date=$(cat $ISOPACKAGE_DIR/version)
    echo "version:" $new_date

    mkisofs -o linux_"$new_date".iso -J -R -A -V -v $ISOPACKAGE_DIR
    mv linux_"$new_date".iso $OUTPUTDIR/
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
	_setup
	make $OPT_DEF DEVICE=${PRODUCT_DEVICE} VARIANT=${MOD_DEF} -j24 ;

	if [ $? != 0 ]; then
			_error "Build error, please check the log"
			exit 1
	fi
	cd $TOPDIR
	_release
	_info "been built done"
}

_start_info() {
    _info "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    _info "USER_ROOT=$USER_ROOT"
    _info "VARIANT=$MOD_DEF"
    _info "PRODUCT_DEVICE=$PRODUCT_DEVICE"
    _info "MOUDLES=$OPT_DEF"
    _info "++++++++++++++++++++Build Started++++++++++++++++++++++++++++++++++++++"
}

_main() {
    _info "$@ $#"
    while getopts rp: opt
    do
        case "$opt" in
        r)
            USER_ROOT=yes
            shift
        ;;
        p)
            PRODUCT_DEVICE=$OPTARG
            shift
            shift
        ;;
        *)
            _error "Unknown option";
            exit 1
        ;;
        esac
    done

    _info "PRODUCT_DEVICE=${PRODUCT_DEVICE}"
    _info "$@  $#"
    if [ x"$1" = x"upload" ]; then
        _upload
        exit
    fi

    if [ x"$1" = x"release" ]; then
        _release
        exit
    fi

    MODELIST="user userdebug eng"
    if [ $# = 1 ]; then
        if [ "`echo $MODELIST | grep $1`" ]; then
            MOD_DEF=$1
        else
            OPT_DEF=$1
        fi
    elif [ $# = 2 ]; then
        MOD_DEF=$1
        OPT_DEF=$2
    fi

    _start_info
    _build $@ 2>&1 | tee build_${PRODUCT_DEVICE}.log
}
_main $@
#### TODO ####
