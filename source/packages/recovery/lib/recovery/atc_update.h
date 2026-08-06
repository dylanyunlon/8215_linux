/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RECOVERY_ATC_UPDATE_H
#define RECOVERY_ATC_UPDATE_H
#include "device.h"

#define ATC_PTBL_SIGN ('P'| ('T' << 8)|('B'<<16)|('L' << 24))
#define ATC_PARTITION_VER 0x00010000

#define IMG_FULL_NAME_MAX 80

#define ATC_PART_TBL_ADDR 0x400000 // 4MB

#define PTBL_BLOCK_SIZE 512
#define ATC_DEVNAME    "/dev/mmcblk0"
#define BLOCK_SIZE 512


#define NEW_PARTITION_DESIGN

#define CONFIG_USRDATA_EXT4

typedef unsigned long long u64;

typedef  struct partitioninfo {

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
    char  szImageFileName[48];    //40 --> 48
#else
    char  szImageFileName[40];
#endif
    unsigned long long u8RealDataSize;

#ifdef NEW_PARTITION_DESIGN
    unsigned int u4Flag;
#endif

struct partitionread *nextpartition;

}partitionread;

typedef  struct partitionhead{

    unsigned int blockcnt;
    unsigned int u4Version;
    unsigned int u4Signature;
    struct partitionread *nextpartition;
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


typedef unsigned short u16;
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

int update_image_for_auto(void);
partitionread * readpartitioninfofromflash(void);
void updata_partition_len(partitionread *ptbl ,const char *dev_name ,unsigned long long len);
int writepartitioninfotoflash(partitionread *ptbl);
int format_userdata_partition_fat32(u64 u8PartOffset, u64 u8PartSize);
void dumpallpartitioninfo(partitionread *part);
void freetblmemory(partitionread *ptbl);
void dumpwriteprotectregion(int fdwp);
void dumppartitioninfo(partitionread *part);
int read_partition_table_from_file(char *file);
#ifdef NEW_PARTITION_DESIGN
unsigned int isEnable(enum Part_Attr attribute, unsigned int flag);
#endif

extern partitionread *xmlptbl;
extern partitionread *newtblhead;
extern partitionread *newtblcur;
extern const char *devname;
#define UPDATE_FLAG_REQUIRE 0x01
#define UPDATE_FLAG_DONE    0x02



#endif  // RECOVERY_ATC_UPDATE_H
