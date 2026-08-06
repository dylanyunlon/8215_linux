#ifndef __PARTITION_H_
#define __PARTITION_H_

#include <config.h>
#include <bootctrl.h>

#define ATC_PTBL_SIGN ('P'| ('T' << 8)|('B'<<16)|('L' << 24))

#define ATC_PARTITION_VER 0x00010000

#define ATC_PART_TBL_ADDR 0x400000 // 4MB

#define PTBL_BLOCK_SIZE 512


typedef  struct partitioninfo{

	char      szPartName[20];
	char      szType[20];
	unsigned int u4Mount;
	unsigned int  u4PartitionStartAddr;
	unsigned long long  u8PartitionStartAddr;
	unsigned int   u4PartitionSize;
	unsigned long long  u8PartitionSize;
#ifdef NEW_PARTITION_DESIGN
	char  szImageFileName[48];        //20 --> 48
#else
	char  szImageFileName[20];
#endif
	unsigned int u4OffsetData;
	unsigned long long u8OffsetData;
	unsigned int u4SizeImage;
	unsigned int u4OffsetNextImage;
	unsigned long long u8OffsetNextImage;
	unsigned int u4SegmentSize;
	unsigned int u4RealDataSize;
	unsigned long long u8RealDataSize;
#ifdef NEW_PARTITION_DESIGN
	unsigned int u4Flag;             //add u4Flag, sync with AUT
#endif
}partitioninfo;



typedef  struct partitionread{

	char      szPartName[20];
	char      szType[20];
	unsigned int u4Mount;
	//unsigned int  u4PartitionStartAddr;
	unsigned long long  u8PartitionStartAddr;
	//unsigned int   u4PartitionSize;
	unsigned long long  u8PartitionSize;
	unsigned int  u4LastPartition;
#ifdef NEW_PARTITION_DESIGN
	char  szImageFileName[48];   //40 --> 48
#else
	char  szImageFileName[40];
#endif
	unsigned long long u8RealDataSize;
#ifdef NEW_PARTITION_DESIGN
	unsigned int u4Flag;         //add u4Flag
#endif
	struct partitionread *nextpartition;
}partitionread;


typedef  struct partitionhead{

	unsigned int blockcnt;
	unsigned int u4Version;
	unsigned int u4Signature;
	struct partitionread *nextpartition;
#ifdef CONFIG_SECURITY_UPGRADE
	char checksum[4];
#endif
}partitionhead;

#ifdef NEW_PARTITION_DESIGN
typedef  struct partitionflag{
	unsigned int upgradable:1;     //sdcard partitial upgrade, upgrade enable bit
	unsigned int eraseable:1;      //partition erase enable bit, mainly for "fastboot erase"
	unsigned int fastbootable:1;   //partition upgrade enable bit, mainly use for "fastboot flash partition image"
	unsigned int copyupgradable:1; //partition upgrade enable bit, mainly use for copy upgrade(SDCARD or UDISK with FAT32 format)
	unsigned int write_protect:1;  //partition write protect enable bit, if set, one cannot write files to this partition
	unsigned int mountable:1;      //partition mount enable bit
	unsigned int :26;
}partitionflag;

#define ENABLE  1
#define DISABLE 0

typedef enum Part_Attr  //must sync with struct partitionflag
{
	UPGRADE_ENABLE = 0,
	ERASE_ENABLE,
	FASTBOOT_ENABLE,
	COPY_UPGRADE_EABLE,
	WRITE_PROTECT_ENABLE,
	MOUNT_ENABLE
}Part_Attr;

#endif

#define NAND_DATAZONE_SIZE	0x200000
#define NAND_DATAZONE_MAIN_ADDR	0x100000
#define NAND_DATAZONE_BK_ADDR	(NAND_DATAZONE_MAIN_ADDR + NAND_DATAZONE_SIZE)

#define DATAZONE_MAIN_OFFSET_FROM_MMCBLK	(64 * 1024)
#define BCB_MAIN_OFFSET_FROM_MMCBLK (DATAZONE_MAIN_OFFSET_FROM_MMCBLK + 4 * 1024)
#define PARTITION_INFO_MAIN_OFFSET_FROM_MMCBLK (BCB_MAIN_OFFSET_FROM_MMCBLK + 4 * 1024)
#define DATAZONE_BK_OFFSET_FROM_MMCBLK	(PARTITION_INFO_MAIN_OFFSET_FROM_MMCBLK + 472 * 1024)
#define BCB_BK_OFFSET_FROM_MMCBLK (DATAZONE_BK_OFFSET_FROM_MMCBLK + 4 * 1024)
#define PARTITION_INFO_BK_OFFSET_FROM_MMCBLK (BCB_BK_OFFSET_FROM_MMCBLK + 4 * 1024)
#define DATAZONE_END	(PARTITION_INFO_BK_OFFSET_FROM_MMCBLK + 472 * 1024)

#define BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN (4 * 1024)
#define PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN (BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN + 4 * 1024)

#define BCB_BK_OFFSET_FROM_DATAZONE_BK (4 * 1024)
#define PARTITION_INFO_BK_OFFSET_FROM_DATAZONE_BK (BCB_BK_OFFSET_FROM_DATAZONE_BK + 4 * 1024)

#define DATAZONE_INFO_DZ_LEN 0x104
#define DATAZONE_INFO_RSV0 0x3C
#define DATAZONE_INFO_RSV1 (0xBC - 0x4C - 24)
#define DATAZONE_INFO_RSV2 (DATAZONE_INFO_DZ_LEN - 0xBC - 24)
#define DATAZONE_INFO_RSV3 (512 - DATAZONE_INFO_DZ_LEN - 4)

#define BOOTFLAG_STARTUP_A 0
#define BOOTFLAG_STARTUP_B 1

#define BCB_TAG "BCBHead"

struct image_desc {
	unsigned int dwLoadAddress;
	unsigned int dwLoadPhyAddr;
	unsigned int dwJumpAddress;
	unsigned int dwJumpPhyAddr;
	unsigned int dwStartAddr;
	unsigned int dwTtlLen;
};

struct datazone_info {
	char rsv0[DATAZONE_INFO_RSV0];
	char magic[16];
	struct image_desc img_desc;
	char rsv1[DATAZONE_INFO_RSV1];
	struct image_desc img_desc_bk;
	char rsv2[DATAZONE_INFO_RSV2];
	char checksum[4];
	char rsv3[DATAZONE_INFO_RSV3];
};
struct bootloader_message {
#ifdef CONFIG_SECURITY_UPGRADE
	char tags[16];
	char checksum[4];
#endif
	char command[32];
	char status[32];
#ifndef CONFIG_SECURITY_UPGRADE
	char recovery[512];
	boot_ctrl_t metadata;
#else
	char recovery[32];
	unsigned int bootflag;
	char laststatus[32];
	boot_ctrl_t metadata;
	char reserved[512];
#endif
};

#define FASTBOOT_UPG_MODE "fastboot_upg"
#define COPY_UPG_MODE   "copy_upg"

void clear_bcb_upg_mode(void);

uint32_t read_datazone_bcb(struct bootloader_message *bcb);
uint32_t set_datazone_bcb(struct bootloader_message *bcb);

#endif

