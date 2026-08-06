#!/bin/sh -e

BOARD_NAME=$(sed -n \
           's/^BR2_DEFCONFIG=".*\/\(.*\)_defconfig"$/\1/p' \
           ${BR2_CONFIG})

test ! -e ${BINARIES_DIR}/zImage || mv ${BINARIES_DIR}/zImage ${BINARIES_DIR}/Image.bin;
test ! -e ${BINARIES_DIR}/ac83xx.dtb || mv ${BINARIES_DIR}/ac83xx.dtb ${BINARIES_DIR}/ac83xx.dtb.bin;

MKFS_BIN=${TOPDIR}/../build/tools/make_ext4fs
ROOTFS_DIR=${TOPDIR}/../out/target/ac83xx
if [ "$AC83XX_BOOT_DEVICE" == "nand" ]; then
	SCATTER_FILE=${ROOTFS_DIR}/../scatter.nand.ext4.xml
else
	SCATTER_FILE=${ROOTFS_DIR}/../scatter.mmcboot.ext4.xml
fi
echo ${SCATTER_FILE}

export PERL5LIB="${TOPDIR}/../build/tools/PartitionUtility":$PERL5LIB

if test -e ${SCATTER_FILE} ; then
	rm ${SCATTER_FILE}
else
	echo " ${SCATTER_FILE} isn't exist!!"
fi

if [ "$AC83XX_BOOT_DEVICE" == "nand" ]; then
	if [ "$ATC_AB_PARTITION_SUPPORT" == "true" ]; then
		if [ "$AC83XX_BOOT_DEVICE_SIZE" == "128" ]; then
			perl ${TOPDIR}/../build/tools/PartitionUtility/PartitionUtility_Nand_Ext4.pl ${TOPDIR}/../build/tools/Linux_Partition_Table_AC8317.xls  ${ROOTFS_DIR}/../ 128 ab
		elif [ "$AC83XX_BOOT_DEVICE_SIZE" == "256" ]; then
			perl ${TOPDIR}/../build/tools/PartitionUtility/PartitionUtility_Nand_Ext4.pl ${TOPDIR}/../build/tools/Linux_Partition_Table_AC8317.xls  ${ROOTFS_DIR}/../ 256 ab
		else
			perl ${TOPDIR}/../build/tools/PartitionUtility/PartitionUtility_Nand_Ext4.pl ${TOPDIR}/../build/tools/Linux_Partition_Table_AC8317.xls  ${ROOTFS_DIR}/../ 512 ab
		fi
	else
		if [ "$AC83XX_BOOT_DEVICE_SIZE" == "128" ]; then
			perl ${TOPDIR}/../build/tools/PartitionUtility/PartitionUtility_Nand_Ext4.pl ${TOPDIR}/../build/tools/Linux_Partition_Table_AC8317.xls  ${ROOTFS_DIR}/../ 128
		else
			perl ${TOPDIR}/../build/tools/PartitionUtility/PartitionUtility_Nand_Ext4.pl ${TOPDIR}/../build/tools/Linux_Partition_Table_AC8317.xls  ${ROOTFS_DIR}/../ 256
		fi
	fi
else
	if [ "$ATC_AB_PARTITION_SUPPORT" == "true" ]; then
		perl ${TOPDIR}/../build/tools/PartitionUtility/PartitionUtility.pl ${TOPDIR}/../build/tools/Linux_Partition_Table_AC8317.xls  ${ROOTFS_DIR}/../ ab
	else
		perl ${TOPDIR}/../build/tools/PartitionUtility/PartitionUtility.pl ${TOPDIR}/../build/tools/Linux_Partition_Table_AC8317.xls  ${ROOTFS_DIR}/../
	fi
fi

if [ "$AC83XX_BOOT_DEVICE" == "emmc" ]; then
	cp ${TOPDIR}/../build/tools/ATCUpgradeTool/config_EMMC.ini ${TOPDIR}/../build/tools/ATCUpgradeTool/config.ini
else
	cp ${TOPDIR}/../build/tools/ATCUpgradeTool/config_NAND_EXT4.ini ${TOPDIR}/../build/tools/ATCUpgradeTool/config.ini
fi

generate_emmc_image() {
	PARTITION_XML_FILE=$1
	PARTITION_NAME=$2
	FILE_DIR=$3
	if test -e ${MKFS_BIN} ; then

		if [ ! -f ${PARTITION_XML_FILE} ]; then
			echo " ${PARTITION_XML_FILE}  file not exist and use the default value!!"
			exit -1
		else
			PARTITION_SIZE=$(cat ${PARTITION_XML_FILE} | grep \"${PARTITION_NAME}\" | awk '{print $6}' | cut -d '"' -f 2 | head -1)
			PARTITION_SIZE=$(printf %d ${PARTITION_SIZE})
			PARTITION_IMAGENAME=$(cat ${PARTITION_XML_FILE} | grep \"${PARTITION_NAME}\" | awk '{print $7}' | cut -d '"' -f 2 | head -1)
		fi

		echo "============ ${PARTITION_XML_FILE} ${FILE_DIR}"
		echo "${PARTITION_NAME} partition size is ${PARTITION_SIZE}"
		echo "${PARTITION_NAME} partiton image name is ${PARTITION_IMAGENAME}"
		echo "make fs tool is ${MKFS_BIN}"
		echo "============"
		if [[ "${PARTITION_NAME}" == "system" || "${PARTITION_NAME}" == "system_a" ]]; then
			${MKFS_BIN} -J -l ${PARTITION_SIZE}  ${BINARIES_DIR}"/"${PARTITION_IMAGENAME} ${FILE_DIR}
		else
			${MKFS_BIN} -l ${PARTITION_SIZE}  ${BINARIES_DIR}"/"${PARTITION_IMAGENAME} ${FILE_DIR}
		fi
	else
		echo "${MKFS_BIN} tool not exist !!"
		exit -1
	fi
}

get_usrdata_index() {
    local scatter_file="$1"

    local index=0
    local found=0

    while read line; do
        if echo "$line" | grep -q "<partition"; then
            imagename=$(echo "$line" | sed -n 's/.*imagename="\([^"]*\)".*/\1/p')
			#echo $imagename

            if [ "$imagename" = "data.img.ext4" ]; then
                echo "$index"
                return
            fi

            index=$((index + 1))
        fi
    done < "$scatter_file"
}

update_fstab_usrdata() {
    local scatter="$1"
    local rootfs="$2"

    idx=$(get_usrdata_index "$scatter")
    dev_node="/dev/mtkd${idx}"

    echo "[INFO] usrdata index=$idx -> $dev_node"

    sed -i "s|^/dev/mtkd[0-9]*[[:space:]]\+/data|${dev_node} /data|" \
        "$rootfs/etc/fstab"
}

INITTAB="${TARGET_DIR}/etc/inittab"
update_inittab() {
    local scatter="$1"

    idx=$(get_usrdata_index "$scatter")
    dev_node="/dev/mtkd${idx}"

    echo "[INFO] usrdata index=$idx -> $dev_node"

	if grep -q '/bin/mount -a' "$INITTAB"; then
		if ! grep -q 'e2fsck' "$INITTAB"; then
			sed -i "\|^si3::sysinit:\/bin\/mount -a|i\\
si2a::sysinit:/usr/bin/e2fsck.sh $dev_node" "$INITTAB"
		else
			echo $dev_node
			sed -i "s|/dev/mtkd[0-9]\+|${dev_node}|g" "$INITTAB"
		fi
	fi
}

generate_nand_image() {
	PARTITION_XML_FILE=$1
	PARTITION_NAME=$2
	FILE_DIR=$3
	if test -e ${MKFS_BIN} ; then

		if [ ! -f ${PARTITION_XML_FILE} ]; then
			echo " ${PARTITION_XML_FILE}  file not exist and use the default value!!"
			exit -1
		else
			PARTITION_SIZE=$(cat ${PARTITION_XML_FILE} | grep \"${PARTITION_NAME}\" | awk '{print $6}' | cut -d '"' -f 2 | head -1)
			PARTITION_SIZE=$(printf %d ${PARTITION_SIZE})
			PARTITION_IMAGENAME=$(cat ${PARTITION_XML_FILE} | grep \"${PARTITION_NAME}\" | awk '{print $7}' | cut -d '"' -f 2 | head -1)
		fi

		echo "============ ${PARTITION_XML_FILE} ${FILE_DIR}"
		echo "${PARTITION_NAME} partition size is ${PARTITION_SIZE}"
		echo "${PARTITION_NAME} partiton image name is ${PARTITION_IMAGENAME}"
		echo "make fs tool is ${MKFS_BIN}"
		echo "============"
		rootfs_val=$(du -sb ${FILE_DIR} | awk '{print $1}')
		echo $rootfs_val
		threshold=$((PARTITION_SIZE * 90 * 95 / 10000 - 2 * 1024 * 1024))
		echo "threshold:$threshold"
		echo "make fs partition size:$((PARTITION_SIZE * 90 / 100))"
		if [ "$rootfs_val" -gt "$threshold" ]; then
			echo "${PARTITION_NAME} image size is more than reserved block size"
			exit 1
		fi
		if [[ "${PARTITION_NAME}" == "system" || "${PARTITION_NAME}" == "system_a" ]]; then
			update_fstab_usrdata ${SCATTER_FILE} ${FILE_DIR}
			update_inittab ${SCATTER_FILE}
			${MKFS_BIN} -J -l $((PARTITION_SIZE * 90 / 100)) -b 1024 ${BINARIES_DIR}"/"${PARTITION_IMAGENAME} ${FILE_DIR}
		else
			${MKFS_BIN} -l $((PARTITION_SIZE * 90 / 100 - 2 * 1024 * 1024)) -b 1024 ${BINARIES_DIR}"/"${PARTITION_IMAGENAME} ${FILE_DIR}
		fi
	else
		echo "${MKFS_BIN} tool not exist !!"
		exit -1
	fi
}

#generate version of package into system.img
now_date=`date "+%Y-%m-%d-%H%M%S"`
echo $now_date
new_date=`echo $now_date | sed -e 's/-//g'`
echo $new_date > ${TARGET_DIR}/etc/version



if [ "$AC83XX_BOOT_DEVICE" == "nand" ]; then
	if [ "$ATC_AB_PARTITION_SUPPORT" == "true" ]; then
		generate_nand_image ${SCATTER_FILE} system_a ${ROOTFS_DIR}
	else
		generate_nand_image ${SCATTER_FILE} system ${ROOTFS_DIR}
	fi
else
	if [ "$ATC_AB_PARTITION_SUPPORT" == "true" ]; then
		generate_emmc_image ${SCATTER_FILE} system_a ${ROOTFS_DIR}
	else
		generate_emmc_image ${SCATTER_FILE} system ${ROOTFS_DIR}
	fi
fi

# build userdata partition
DATA_DIR=${TOPDIR}/../out/target/data
if test ! -e ${DATA_DIR} ; then
	echo " ${DATA_DIR} is not exist!!"
	mkdir ${DATA_DIR}
fi

if test ! -e ${DATA_DIR}/OTAupdate ; then
    echo "Creating OTAupdate directory..."
    mkdir -p ${DATA_DIR}/OTAupdate
fi
if [ "$AC83XX_BOOT_DEVICE" == "nand" ]; then
	generate_nand_image ${SCATTER_FILE} usrdata ${DATA_DIR}
else
	generate_emmc_image ${SCATTER_FILE} usrdata ${DATA_DIR}
fi

if test -e ${SCATTER_FILE} ; then
	mv ${SCATTER_FILE} ${TOPDIR}/../out/images/;
fi
