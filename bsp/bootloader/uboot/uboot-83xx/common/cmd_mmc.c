/*
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

 
/**********************************************************************************
mmc  boot  opcode [OP Param] [R/W buffer Pointer] [R/W Address Pointer] [R/W Size]
mmc: command, must.

boot: indicates that we opereate boot partition, must.

opcode: opreation code, now we define below operations:
                #define BOOT_OP_NONE                                                      (0)          // no operation, for init
#define BOOT_OP_ENTER_MODE0                                          (1)          // enter boot mode0, in this mode, emmc will send data to host automaticlly.
#define BOOT_OP_EXIT_MODE0                                               (2)          // exit boot mode0
#define BOOT_OP_ENTER_MODE1                                          (3)          // enter boot mode1, now for write, we enter this mode.
#define BOOT_OP_EXIT_MODE1                                               (4)          // exit boot mode1
#define BOOT_OP_WRITE_BOOT_PART0                              (5)          // write boot partition 0
#define BOOT_OP_READ_BOOT_PART0                                (6)          // read boot partition 0
#define BOOT_OP_WRITE_BOOT_PART1                              (7)          // write boot partition 1
#define BOOT_OP_READ_BOOT_PART1                                (8)          // read boot partition 1
                #define BOOT_PART_NONE                        (0)
                #define BOOT_PART_PART0                       (1)
                #define BOOT_PART_PART1                       (2)

[OP Param]: opreation parameter, optional; such as: BOOT_OP_SET_BOOTPART_FOR_BOOT needs boot parition number for set to eMMC.

[R/W Buffer Pointer]: buffer for store data read from boot partition, when opcode is READ operation, otherwise it store data for write to boot partition. Optional.

[R/W Address Pointer]: Address pointer, must be 4 Byte alignment. Uses  block as unit. optional. 

[R/W Size]: uses block as unit. Optional.
******************************************************************************************************/



#include <common.h>
#include <command.h>

//#define MSDC_PERFORMANCE_TEST
#ifdef MSDC_PERFORMANCE_TEST
#define MEM_TEST_ADDR  (0x19300000)
#define MMC_TSET_BLK_ADDR (0x200000)
static unsigned int boot_time_ms(void)
{
	volatile unsigned int time = 0;
	
	/***
	* Register F000814C, which was triggered by BootROM, 
	* start with 0xFFFFFFFF, end with 0x00000000,
	* decrease with every 27M crystal oscillation.
	*/
	time = (0xFFFFFFFF - (*((volatile uint32_t*)(0xF000814C)))) / 27000;
	return time;
}
#endif
#ifndef CONFIG_GENERIC_MMC
static int curr_device = -1;

int do_mmc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int dev;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	if (strcmp(argv[1], "init") == 0) {
		if (argc == 2) {
			if (curr_device < 0)
				dev = 1;
			else
				dev = curr_device;
		} else if (argc == 3) {
			dev = (int)simple_strtoul(argv[2], NULL, 10);
		} else {
			cmd_usage(cmdtp);
			return 1;
		}

		if (mmc_legacy_init(dev) != 0) {
			puts("No MMC card found\n");
			return 1;
		}

		curr_device = dev;
		printf("mmc%d is available\n", curr_device);
	} else if (strcmp(argv[1], "device") == 0) {
		if (argc == 2) {
			if (curr_device < 0) {
				puts("No MMC device available\n");
				return 1;
			}
		} else if (argc == 3) {
			dev = (int)simple_strtoul(argv[2], NULL, 10);

#ifdef CONFIG_SYS_MMC_SET_DEV
			if (mmc_set_dev(dev) != 0)
				return 1;
#endif
			curr_device = dev;
		} else {
			cmd_usage(cmdtp);
			return 1;
		}

		printf("mmc%d is current device\n", curr_device);
	} else {
		cmd_usage(cmdtp);
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	mmc, 3, 1, do_mmc,
	"MMC sub-system",
	"init [dev] - init MMC sub system\n"
	"mmc device [dev] - show or set current device"
);
#else /* !CONFIG_GENERIC_MMC */

#ifdef CONFIG_MTK_SD

#include <mmc.h>

//=============================== Format FAT32 Parttion ============================================
// Struct definition

/** fat boot section struct */
#pragma pack(push ,1)
typedef struct _IMAGE_BPB{
	u16 BPB_BytesPerSec;
	u8 	BPB_SecPerClus;
	u16	BPB_RsvdSecCnt;
	u8	BPB_NumFATs;
	u16 BPB_RootEntCnt;
	u16 BPB_ToSec16;
	u8 	BPB_Media;
	u16 BPB_FATSz16;
	u16 BPB_SecPerTrk;
	u16 BPB_NumHeads;
	u32	BPB_HidSec;
	u32	BPB_ToSec32;
	u32	BPB_FATSz32;
	u16 BPB_Flags;
	u16 BPB_FSVer;
	u32 BPB_RootClus;
	u16	BPB_FSInfo;
	u16 BPB_Reserved;
	u8 	reserved[12];
} IMAGE_BPB, *PIMAGE_BPB;

typedef struct _IMAGE_EXTERN_BPB{
	u8 BS_DrvNum;
	u8 BS_Reserved1;
	u8 BS_BootSig;
	u32	BS_VSN;
	u8 BS_VolumeLabel[11];
	u8 BS_SystemID[8];
} IMAGE_EXTERN_BPB, *PIMAGE_EXTERN_BPB;

typedef struct _IMAGE_DBP_SECTOR{
	u8 JmpCnd[3];
	char OSVersion[8];
	IMAGE_BPB bpb;
	IMAGE_EXTERN_BPB bpb_extern;
} IMAGE_DBP_SECTOR, *PIMAGE_DBP_SECTOR;

typedef struct _IMAGE_FSINFO{
	u32 FI_BootSig;
	u8 	FI_Reserved1[480];
	u32 FI_Signature;
	u32 FI_FreeClus;
	u32 FI_NextClus;
	u8 	FI_Reserved2[14];
	u16 FI_EndSig;
} IMAGE_FSINFO, *PIMAGE_FSINFO;

#pragma pack(pop)

typedef struct _DSKSZTOSECPERCLUS {
    u32   DiskSize;
    u8    SecPerClusVal;
}DSKSZTOSECPERCLUS;


// Number of sectors to allocate in memory for read/write buffers
#define NUM_BLOCK_SECTORS 	256
#define VFAT_BLOCK_SIZE		512
#define FAT_FSINFO_SIG		0x41615252

static DSKSZTOSECPERCLUS DskTableFAT32 [] = {
    {    66600,  0},       // disks up to 32.5 MB, the 0 value for SecPerClusVal trips an error 
    {   532480,  1},       // disks up to 260 MB,  .5k cluster 
    { 16777216,  8},       // disks up to     8 GB,    4k cluster
    { 33554432, 16},       // disks up to   16 GB,    8k cluster
    { 67108864, 32},       // disks up to   32 GB,  16k cluster
    { 0xFFFFFFFF, 64}      // disks greater than 32GB, 32k cluster
};


static void fat_fill_bpb_extern(IMAGE_EXTERN_BPB *bpb_extern)
{
	char *type = "FAT32";
	char *p;
	bpb_extern->BS_DrvNum  = 0x80;
	bpb_extern->BS_Reserved1 = 0;
	bpb_extern->BS_BootSig = 0x29;
	bpb_extern->BS_VSN = 0x94525487;
	p = (char *)bpb_extern->BS_VolumeLabel;
	p[0] = 'N';
	p[1] = 'O';
	p[2] = ' ';
	p[3] = 'N';
	p[4] = 'A';
	p[5] = 'M';
	p[6] = 'E';
	p[7] = ' '; 
	p[8] = 'A';
	p[9] = 'T';
	p[10] = 'C';
	memcpy(bpb_extern->BS_SystemID, type, 6);
}


static u32 GetSectorsPerCluster (u32 dwTotalSectors)
{
	int i;
	u32 dwSectorsPerCluster = 0;

	for (i = 0; 1; i++) 
	{
		if (dwTotalSectors <= DskTableFAT32[i].DiskSize) 
		{
			dwSectorsPerCluster = DskTableFAT32[i].SecPerClusVal;
			break;
		}
	}

	return dwSectorsPerCluster;
}

static void fat_fill_bpb(IMAGE_BPB *bpb , u32 dwTotalSectors)
{
	bpb->BPB_BytesPerSec = VFAT_BLOCK_SIZE;
	bpb->BPB_SecPerClus = GetSectorsPerCluster(dwTotalSectors); //(1024*16)/VFAT_BLOCK_SIZE;// Ref to 'DskTableFAT32'
	bpb->BPB_RsvdSecCnt = 32;
	bpb->BPB_NumFATs = 2;
	bpb->BPB_RootEntCnt = 0;
	bpb->BPB_ToSec16 = 0;
	bpb->BPB_Media = 0xF8;
	bpb->BPB_FATSz16 = 0;
	bpb->BPB_SecPerTrk = 0x3F;
	bpb->BPB_NumHeads = 0xFF;
	bpb->BPB_HidSec = 0;
	bpb->BPB_ToSec32 = 0; //
	bpb->BPB_FATSz32 = 0; //
	bpb->BPB_Flags = 0;
	bpb->BPB_FSVer = 0;
	bpb->BPB_RootClus = 2;
	bpb->BPB_FSInfo = 1;
	bpb->BPB_Reserved = 0;
	memset(bpb->reserved, 0x00, 12);
}

static void fat_fill_dbp(IMAGE_DBP_SECTOR *dbp, u32 dwTotalSectors)
{
	char *version = "MSDOS5.0";
	dbp->JmpCnd[0] = 0xEB;
	dbp->JmpCnd[1] = 0x58;
	dbp->JmpCnd[2] = 0x90;
	memcpy(dbp->OSVersion, version, 8);
	fat_fill_bpb(&(dbp->bpb), dwTotalSectors);
	fat_fill_bpb_extern(&(dbp->bpb_extern));
}

static void fat_fill_fsinfo(IMAGE_FSINFO *fsinfo)
{
	fsinfo->FI_BootSig = FAT_FSINFO_SIG;
	fsinfo->FI_Signature = 0x61417272;
	fsinfo->FI_NextClus = 2;
	fsinfo->FI_FreeClus = -1;
	fsinfo->FI_EndSig = 0x55AA;
}

static u32 caculate_fat_size(u32 total_sect, u32 resv_sect, u32 fat_num, u32 SecPerClus)
{
	u32 fat_sect = 0;
	u32 inval_sect = 0;
	u32 tmp = 0;
	while(1)
	{
		fat_sect++;
		inval_sect = total_sect - resv_sect - fat_sect*fat_num;
		tmp = (inval_sect/SecPerClus + 2)*4;
		if(tmp <= fat_sect * VFAT_BLOCK_SIZE)
		{
			return fat_sect;
		}
	}
}

int format_userdata_partition(struct mmc *mmc, int dev, u32 u4PartOffset, u32 u4PartSize)
{
	u32 i = 0;
	u32 sector_id = 0;
	IMAGE_DBP_SECTOR *block_buffer = (PIMAGE_DBP_SECTOR)malloc(VFAT_BLOCK_SIZE);
	IMAGE_BPB *bpb = &(block_buffer->bpb);
	IMAGE_FSINFO *fsinfo;
	u_char* pWriteBuf = (u_char*)malloc(VFAT_BLOCK_SIZE * NUM_BLOCK_SECTORS);
	u32 u4RemainWriteSector = 0;
	u64 u8PartitionSize = 0;

	if(block_buffer == NULL || pWriteBuf == NULL)
	{
		printf("Can not alloc memory for userdata format function\n");
		return -1;
	}

	if (u4PartSize)
	{
		u8PartitionSize = u4PartSize;
	}
	else
	{
		u8PartitionSize = mmc->capacity - u4PartOffset;
	}
	//printf("---> Format_Userdata_Partition: Addr = 0x%X, Size= 0x%X, mmc= 0x%X <---\n", u4PartOffset, u4PartSize, (u32)mmc->capacity);
	//printf("---> Format_Userdata_Partition: u8PartitionSize = 0x%X <---\n", u8PartitionSize);
	
	memset(block_buffer, 0x00, VFAT_BLOCK_SIZE);
	memset(pWriteBuf, 0x00, VFAT_BLOCK_SIZE * NUM_BLOCK_SECTORS);
	
	fat_fill_dbp((IMAGE_DBP_SECTOR *)block_buffer, (u32)(u8PartitionSize/VFAT_BLOCK_SIZE) );
	bpb->BPB_ToSec32 = (u32)(u8PartitionSize / VFAT_BLOCK_SIZE);
	bpb->BPB_FATSz32 = caculate_fat_size(bpb->BPB_ToSec32, bpb->BPB_RsvdSecCnt, bpb->BPB_NumFATs, bpb->BPB_SecPerClus);
	((u_char *)block_buffer)[510] = 0x55;
	((u_char *)block_buffer)[511] = 0xAA;

	printf("Format userdat partition: TotalSector = 0x%X, FAT Size = 0x%X, SectorsPerCluster = %d \n", bpb->BPB_ToSec32, bpb->BPB_FATSz32, bpb->BPB_SecPerClus);

	/** write boot section */
	mmc->block_dev.block_write(dev, (u4PartOffset/VFAT_BLOCK_SIZE + sector_id), 1, block_buffer);
	
	sector_id = 1;
	fsinfo = (IMAGE_FSINFO *)pWriteBuf;
	fat_fill_fsinfo(fsinfo); 
	mmc->block_dev.block_write(dev, (u4PartOffset/VFAT_BLOCK_SIZE + sector_id), 1, pWriteBuf);
	
	sector_id++;
	memset(pWriteBuf, 0x00, VFAT_BLOCK_SIZE * NUM_BLOCK_SECTORS);

	mmc->block_dev.block_write(dev, (u4PartOffset/VFAT_BLOCK_SIZE + sector_id), (bpb->BPB_RsvdSecCnt - 2), pWriteBuf);
	
	/** write FAT table */
	((u32 *)pWriteBuf)[0] = 0x0FFFFFF8;
	((u32 *)pWriteBuf)[1] = 0x0FFFFFFF;
	((u32 *)pWriteBuf)[2] = 0x0FFFFFFF;
	sector_id = bpb->BPB_RsvdSecCnt;
	mmc->block_dev.block_write(dev, (u4PartOffset/VFAT_BLOCK_SIZE + sector_id), 1, pWriteBuf);
	sector_id++; //33
	memset(pWriteBuf, 0x00, 12); // Reset all data to 0

	u4RemainWriteSector = bpb->BPB_FATSz32 - 1;
	for(i = 1; i<bpb->BPB_FATSz32; )
	{
		if (u4RemainWriteSector < NUM_BLOCK_SECTORS)
		{
			mmc->block_dev.block_write(dev, (u4PartOffset/VFAT_BLOCK_SIZE + sector_id), u4RemainWriteSector, pWriteBuf);
			sector_id += u4RemainWriteSector;
			i += u4RemainWriteSector;
			u4RemainWriteSector = 0;
		}
		else
		{
			mmc->block_dev.block_write(dev, (u4PartOffset/VFAT_BLOCK_SIZE + sector_id), NUM_BLOCK_SECTORS, pWriteBuf);
			sector_id += NUM_BLOCK_SECTORS;
			i += NUM_BLOCK_SECTORS;
			u4RemainWriteSector -= NUM_BLOCK_SECTORS;
		}
	}
	
	/** write 2th fat table */
	((u32 *)pWriteBuf)[0] = 0x0FFFFFF8;
	((u32 *)pWriteBuf)[1] = 0x0FFFFFFF;  
	((u32 *)pWriteBuf)[2] = 0x0FFFFFFF;
	mmc->block_dev.block_write(dev, (u4PartOffset/VFAT_BLOCK_SIZE + sector_id), 1, pWriteBuf);
	sector_id++;
	memset(pWriteBuf, 0x00, 12); // Reset all data to 0
	u4RemainWriteSector = bpb->BPB_FATSz32 - 1;
	for(i = 1; i<bpb->BPB_FATSz32; )
	{
		if (u4RemainWriteSector < NUM_BLOCK_SECTORS)
		{
			mmc->block_dev.block_write(dev, (u4PartOffset/VFAT_BLOCK_SIZE + sector_id), u4RemainWriteSector, pWriteBuf);
			sector_id += u4RemainWriteSector;
			i += u4RemainWriteSector;
			u4RemainWriteSector = 0;
		}
		else
		{
			mmc->block_dev.block_write(dev, (u4PartOffset/VFAT_BLOCK_SIZE + sector_id), NUM_BLOCK_SECTORS, pWriteBuf);
			sector_id += NUM_BLOCK_SECTORS;
			i += NUM_BLOCK_SECTORS;
			u4RemainWriteSector -= NUM_BLOCK_SECTORS;
		}
	}
	
	// Root Directory, Max Size is 32 Sectors, now we earse 256 Sectors
	mmc->block_dev.block_write(dev, (u4PartOffset/VFAT_BLOCK_SIZE + sector_id), NUM_BLOCK_SECTORS, pWriteBuf);

	free(block_buffer);
	free(pWriteBuf);
	
	return 0;
}

int format_userdata_partition_u64(struct mmc *mmc, int dev, u64 u8PartOffset, u64 u8PartSize)
{
   
	u32 i = 0;
	u32 sector_id = 0;
	IMAGE_DBP_SECTOR *block_buffer = (PIMAGE_DBP_SECTOR)malloc(VFAT_BLOCK_SIZE);
	IMAGE_BPB *bpb = &(block_buffer->bpb);
	IMAGE_FSINFO *fsinfo;
	u_char* pWriteBuf = (u_char*)malloc(VFAT_BLOCK_SIZE * NUM_BLOCK_SECTORS);
	u32 u4RemainWriteSector = 0;
	u64 u8PartitionSize = 0;
    printf("---> format_userdata_partition_u64 eMMC u8PartOffset: 0x%08X, 0x%08X <---\r\n", (u32)(u8PartOffset >> 32),(u32)(u8PartOffset & 0xFFFFFFFF));
	printf("---> format_userdata_partition_u64 eMMC u8PartSize: 0x%08X, 0x%08X <---\r\n", (u32)(u8PartSize >> 32),(u32)(u8PartSize & 0xFFFFFFFF));
    printf("\r\n");

	if(block_buffer == NULL || pWriteBuf == NULL)
	{
		printf("Can not alloc memory for userdata format function\n");
		return -1;
	}

	if (u8PartSize)
	{
		u8PartitionSize = u8PartSize;
	}
	else
	{
		u8PartitionSize = mmc->capacity - u8PartOffset;
	}
	//printf("---> Format_Userdata_Partition: Addr = 0x%X, Size= 0x%X, mmc= 0x%X <---\n", u4PartOffset, u4PartSize, (u32)mmc->capacity);
	//printf("---> Format_Userdata_Partition: u8PartitionSize = 0x%X <---\n", u8PartitionSize);
	printf("---> Format_Userdata_Partition eMMC capacity: 0x%08X, 0x%08X <---\r\n",(u32)(mmc->capacity >> 32), (u32)(mmc->capacity & 0xFFFFFFFF));
	
	memset(block_buffer, 0x00, VFAT_BLOCK_SIZE);
	memset(pWriteBuf, 0x00, VFAT_BLOCK_SIZE * NUM_BLOCK_SECTORS);
	
	fat_fill_dbp((IMAGE_DBP_SECTOR *)block_buffer, (u32)(u8PartitionSize/VFAT_BLOCK_SIZE) );
	bpb->BPB_ToSec32 = (u32)(u8PartitionSize / VFAT_BLOCK_SIZE);
	bpb->BPB_FATSz32 = caculate_fat_size(bpb->BPB_ToSec32, bpb->BPB_RsvdSecCnt, bpb->BPB_NumFATs, bpb->BPB_SecPerClus);
	((u_char *)block_buffer)[510] = 0x55;
	((u_char *)block_buffer)[511] = 0xAA;

	//printf("Format userdat partition: TotalSector = 0x%X, FAT Size = 0x%X, SectorsPerCluster = %d \n", bpb->BPB_ToSec32, bpb->BPB_FATSz32, bpb->BPB_SecPerClus);

	/** write boot section */
	mmc->block_dev.block_write(dev, (unsigned int)(u8PartOffset/VFAT_BLOCK_SIZE + sector_id), 1, block_buffer);
	
	sector_id = 1;
	fsinfo = (IMAGE_FSINFO *)pWriteBuf;
	fat_fill_fsinfo(fsinfo); 
	mmc->block_dev.block_write(dev, (unsigned int)(u8PartOffset/VFAT_BLOCK_SIZE + sector_id), 1, pWriteBuf);
	
	sector_id++;
	memset(pWriteBuf, 0x00, VFAT_BLOCK_SIZE * NUM_BLOCK_SECTORS);

	mmc->block_dev.block_write(dev, (unsigned int)(u8PartOffset/VFAT_BLOCK_SIZE + sector_id), (bpb->BPB_RsvdSecCnt - 2), pWriteBuf);
	
	/** write FAT table */
	((u32 *)pWriteBuf)[0] = 0x0FFFFFF8;
	((u32 *)pWriteBuf)[1] = 0x0FFFFFFF;
	((u32 *)pWriteBuf)[2] = 0x0FFFFFFF;
	sector_id = bpb->BPB_RsvdSecCnt;
	mmc->block_dev.block_write(dev, (unsigned int)(u8PartOffset/VFAT_BLOCK_SIZE + sector_id), 1, pWriteBuf);
	sector_id++; //33
	memset(pWriteBuf, 0x00, 12); // Reset all data to 0

	u4RemainWriteSector = bpb->BPB_FATSz32 - 1;
	for(i = 1; i<bpb->BPB_FATSz32; )
	{
		if (u4RemainWriteSector < NUM_BLOCK_SECTORS)
		{
			mmc->block_dev.block_write(dev, (unsigned int)(u8PartOffset/VFAT_BLOCK_SIZE + sector_id), u4RemainWriteSector, pWriteBuf);
			sector_id += u4RemainWriteSector;
			i += u4RemainWriteSector;
			u4RemainWriteSector = 0;
		}
		else
		{
			mmc->block_dev.block_write(dev, (unsigned int)(u8PartOffset/VFAT_BLOCK_SIZE + sector_id), NUM_BLOCK_SECTORS, pWriteBuf);
			sector_id += NUM_BLOCK_SECTORS;
			i += NUM_BLOCK_SECTORS;
			u4RemainWriteSector -= NUM_BLOCK_SECTORS;
		}
		
		printf(".");
		if (((i-1)% (32*256) == 0) && (i-1 != 0))
		{
			printf("\r\n");
		}
	}
	
	/** write 2th fat table */
	((u32 *)pWriteBuf)[0] = 0x0FFFFFF8;
	((u32 *)pWriteBuf)[1] = 0x0FFFFFFF;  
	((u32 *)pWriteBuf)[2] = 0x0FFFFFFF;
	mmc->block_dev.block_write(dev, (unsigned int)(u8PartOffset/VFAT_BLOCK_SIZE + sector_id), 1, pWriteBuf);
	sector_id++;
	memset(pWriteBuf, 0x00, 12); // Reset all data to 0
	u4RemainWriteSector = bpb->BPB_FATSz32 - 1;
	for(i = 1; i<bpb->BPB_FATSz32; )
	{
		if (u4RemainWriteSector < NUM_BLOCK_SECTORS)
		{
			mmc->block_dev.block_write(dev, (unsigned int)(u8PartOffset/VFAT_BLOCK_SIZE + sector_id), u4RemainWriteSector, pWriteBuf);
			sector_id += u4RemainWriteSector;
			i += u4RemainWriteSector;
			u4RemainWriteSector = 0;
		}
		else
		{
			mmc->block_dev.block_write(dev, (unsigned int)(u8PartOffset/VFAT_BLOCK_SIZE + sector_id), NUM_BLOCK_SECTORS, pWriteBuf);
			sector_id += NUM_BLOCK_SECTORS;
			i += NUM_BLOCK_SECTORS;
			u4RemainWriteSector -= NUM_BLOCK_SECTORS;
		}
		
		printf(".");
		if (((i-1)% (32*256) == 0) && (i-1 != 0))
		{
			printf("\r\n");
		}
	}
	
	// Root Directory, Max Size is 32 Sectors, now we earse 256 Sectors
	mmc->block_dev.block_write(dev, (unsigned int)(u8PartOffset/VFAT_BLOCK_SIZE + sector_id), NUM_BLOCK_SECTORS, pWriteBuf);

	free(block_buffer);
	free(pWriteBuf);
	
	printf("\r\n");
	return 0;
}
void print_mmc_usage()
{
	printf("mmc command usage:\n");
	printf("\t (01) mmc [help]                                                          ==> print mmc command usage\n");
	printf("\t (02) mmc [rescan] [sd_slot](0/1/2)                                       ==> re_init card\n");
	printf("\t (03) mmc [read]   [sd_slot](0/1/2) [address](blocks) [read_size](blocks) ==> read blocks\n");
	printf("\t (04) mmc [write]  [sd_slot](0/1/2) [address](blocks) [read_size](blocks) ==> write blocks\n");
	printf("\t (05) mmc [format] [sd_slot](0/1/2) [partition_name] [offset] [size]      ==> format usrdate partition\n");
	
	// ========================================================================
	// ETT Releated Functions 
	// ========================================================================
	#ifdef CONFIG_MSDC_ETT
	
	printf("\t (06) mmc [ett]    [sd_slot](0/1/2) [r/w]                                 ==> do msdc ett r/w ops\n");
	printf("\t (07) mmc [reset]  [sd_slot](0/1/2) [reset_type](0-sw, 1-hw)              ==> reset msdc module\n");
	printf("\t (08) mmc [pads]   [sd_slot](0/1/2) [s/g] [clk_drv] [cmd_drv] [dat_drv] [resistor] [slew_rate]\n");
	printf("\t                                                                          ==> set msdc pin pad setting\n");
	printf("\t (09) mmc [pss]    [sd_slot](0/1/2) [s/g] [r/w/rw] [0/1]                  ==> enable/disable pre_setting for read/write\n"); // pre_setting state
	printf("\t (10) mmc [sclk]   [sd_slot](0/1/2) [ddr/sdr] [clk]                       ==> set work clock\n");
	printf("\t (11) mmc [pres]   [sd_slot](0/1/2) [s/g] [r/w] [ddr/sdr] [ck_sel] [ckgen_delay] [pad_delay] [internal_delay] [sample_edge]\n"); // pre_setting params
	printf("\t\t\t [ck_sel]         range:  0 ~ 7  \n");
	printf("\t\t\t [ckgen_delay]    range:  0 ~ 31 \n");
	printf("\t\t\t [pad_delay]      range:  0 ~ 31 \n");
	printf("\t\t\t [internal_delay] range:  0 ~ 31 \n");
	printf("\t\t\t [sample_edge]    range:  0 ~ 1  \n");
	printf("\t                                                                          ==> get/set read/write pre_setting parameters\n");
	printf("\t (12) mmc [hs200]  [sd_slot](0/1/2) [s/g] [0/1]                           ==> enter hs200 mode when change to HS200\n");
	printf("\t (13) mmc [scs]    [sd_slot](0/1/2) [s/g] [clock](162/147/135)[0/1]       ==> set msdc clock source\n");
	printf("\t (14) mmc [dump]   [sd_slot](0/1/2) [dst] [offset] [start_address] [size] ==> dump data to sd card\n");
	printf("\t (15) mmc [erase]  [sd_slot](0/1/2) [type] [start_address] [size]         ==> erase/trim/discard emmc blocks\n");
	printf("\t (16) mmc [dreg]   [sd_slot](0/1/2) [type]                                ==> dump register content\n");
	printf("\t (17) mmc [tune_emmc] [r/w/rw]                                            ==> tuning emmc\n");
	printf("\t (18) mmc [hqa]    [sd_slot](0/1/2) [r/w] [times]                         ==> emmc hqa test\n");
	printf("\t (19) mmc [pattern][sd_slot](0/1/2)                                       ==> emmc pattern test\n");
	
	#if 0
	printf("\t (15) mmc [aett]   [sd_slot](0/1/2) [clock]                               ==> auto msdc ett at selected clock\n");
	#endif
	
	#endif //CONFIG_MSDC_ETT

	#ifdef MSDC_PERFORMANCE_TEST
	printf("\t (20) mmc [test_perf] [sd_slot](0/1/2) [clk source] [rw] [size(MB)]      ==> Test eMMC/SD RW Performance\n");
	#endif
}


static void print_mmc_sclk_usage()
{
	printf("mmc sclk sample:\n");
	printf("\t mmc sclk 1 sdr 50  ==> change msdc slot1 work clock to sdr 50MHz\n");
}

static void print_mmc_cmd_sample(char* argv)
{
	if (strcmp(argv[1], "sclk") == 0) 
	{
		print_mmc_sclk_usage();
	}
}

//=============================================================================
int do_mmcops(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int err = 0;
	int dev_num = -1;
	int opcode = 0;
	ulong real_blk = 0;
	void *pbuf = NULL;
	u32 cnt_blk = 0;
	u32 addr_blk = 0;
	struct mmc *mmc = NULL;
	int dst_dev = -1;
	struct mmc *dst_mmc = NULL;
	int rw = 0;
	int times = 0;
	uint16_t reg_id;
	uint8_t val;
	#ifdef MSDC_PERFORMANCE_TEST
	u32 time1, time2;
	int clock;
	u32 size_mb;
	#endif
	
	if (argc < 2)
	{
		printf("error command parameters number!!\n");
		print_mmc_usage();
		return 1;
	}	

	if (strcmp(argv[1], "rescan") == 0) 
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		if (!mmc)
		{
			return 1;
		}

		mmc_force_reinit(mmc);

		return err;
	}
	// ================   mmc read op =================== 
		// 0 - mmc 
		// 1 - read
		// 2 - 0 --> mmc slot select
		// 3 - Read Buffer Pointer
		// 4 - Read Address (in block)
		// 5 - Read Buffer Size (in block)
	if (strcmp(argv[1], "read") == 0) 
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		pbuf = (void *)simple_strtoul(argv[3], NULL, 16);
		cnt_blk = simple_strtoul(argv[5], NULL, 16);
		addr_blk = simple_strtoul(argv[4], NULL, 16);
		mmc = find_mmc_device(dev_num);
		if (!mmc)
		{
			return 1;
		}
		//printf("\nMMC read: dev # %d, block # %d, count %d ... ", dev, blk, cnt);

		err = mmc_init(mmc);
        if (err)
        {
        	return err;
        }

		real_blk = mmc->block_dev.block_read(dev_num, addr_blk, cnt_blk, pbuf);

		/* flush cache after read */
		flush_cache((ulong)pbuf, cnt_blk * 512); /* FIXME */

		//printf("%d blocks read: %s\n", n, (n==cnt) ? "OK" : "ERROR");
			
		return (real_blk == cnt_blk) ? 0 : 1;
	} 
	// ================   mmc write op =================== 
		// 0 - mmc 
		// 1 - write
		// 2 - 0 --> mmc slot select
		// 3 - Write Buffer Pointer
		// 4 - Write Address (in block)
		// 5 - Write Buffer Size (in block)
	else if (strcmp(argv[1], "write") == 0) 
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		pbuf = (void *)simple_strtoul(argv[3], NULL, 16);
		cnt_blk = simple_strtoul(argv[5], NULL, 16);
		addr_blk = simple_strtoul(argv[4], NULL, 16);
		mmc = find_mmc_device(dev_num);
		if (!mmc)
		{
			return 1;
		}
		//printf("\n MMC write: dev_num=%u, addr_blk=%u, cnt_blk=%u\n", dev_num, addr_blk, cnt_blk);

		err = mmc_init(mmc);
        if (err)
        {
        	return err;
        }
		real_blk = mmc->block_dev.block_write(dev_num, addr_blk, cnt_blk, pbuf);
		return (real_blk == cnt_blk) ? 0 : 1;
	}
	// ================   mmc format userdata partition ===================
		// Command format:
		// 0 - mmc
		// 1 - format
		// 2 - 0 --> mmc slot select
		// 3 - userdata
		// 4 - partition start address (in byte)
		// 5 - partition size (in byte)
	else if (strcmp(argv[1], "format") == 0)
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		//char* partition = (char*)argv[3];
		//u64 offset = simple_strtoull(argv[4], NULL, 16);
		u64 offset, size;
		simple_strtou64(argv[4], NULL, 16,&(offset));
		//u64 size = simple_strtoull(argv[5], NULL, 16);
		simple_strtou64(argv[5], NULL, 16,&(size));
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
		{
        	return err;
		}
		printf("---> Format_Userdata_Partition offset: 0x%08X, 0x%08X <---\n", (u32)(offset >> 32), (u32)(offset & 0xFFFFFFFF));
		printf("---> Format_Userdata_Partition size: 0x%08X, 0x%08X <---\n", (u32)(size >> 32), (u32)(size & 0xFFFFFFFF));

		printf("\n---------> format_userdata_partition: offset = 0x%X, size = 0x%X <----------\r\n", offset, size);

		err = format_userdata_partition_u64(mmc, dev_num, offset, size);
		if (err)
		{
        	return err;
		}	
		return 0;
	}
	
	//=====================  emmc boot partition op  =========================
	//mmc  boot  opcode [OP Param] [R/W buffer Pointer] [R/W Address Pointer] [R/W Size]
	//==================================================================
	else if (strcmp(argv[1], "boot") == 0) // for emmc boot partition op
	{
		dev_num = 0; // hardcode to 0, because only slot0 supports emmc boot partition.
		opcode = simple_strtoul(argv[2], NULL, 10);
		switch (opcode)
		{
		case BOOT_OP_ENTER_BOOT_MODE:
			err = mmc_boot_enter_bootmode(dev_num);
			break;
			
		case BOOT_OP_EXIT_BOOT_MODE:
			err = mmc_boot_exit_bootmode(dev_num);
			break;
			
 		case BOOT_OP_WRITE_BOOT_PART1:   // write boot partition 0
 		case BOOT_OP_READ_BOOT_PART1:    // read boot partition 0
 		case BOOT_OP_WRITE_BOOT_PART2:   // write boot partition 1
 		case BOOT_OP_READ_BOOT_PART2:    // read boot partition 1
			pbuf = (void *)simple_strtoul(argv[3], NULL, 16);
			addr_blk = simple_strtoul(argv[4], NULL, 16);
			cnt_blk = simple_strtoul(argv[5], NULL, 16);
			
			int write_op = 0;
			int boot_part_num = BOOT_PART_PART1; // default op on boot partition 1
			if ((opcode == BOOT_OP_WRITE_BOOT_PART1) || (opcode == BOOT_OP_WRITE_BOOT_PART2))
			{
				write_op = 1;
			}

			if ((opcode == BOOT_OP_WRITE_BOOT_PART2) || (opcode == BOOT_OP_READ_BOOT_PART2))
			{
				boot_part_num = BOOT_PART_PART2;
			}

			if (write_op)
			{
				real_blk = mmc_boot_bwrite(dev_num, boot_part_num, addr_blk, cnt_blk, pbuf);
			}
			else
			{
				real_blk =  mmc_boot_bread(dev_num, boot_part_num, addr_blk, cnt_blk, pbuf);
			}
			err = (real_blk == cnt_blk) ? 0 : 1;
 			break;
			
		default:
			err = 1;
			break;
				
		}

		return err;
	}
	else if (!strcmp(argv[1], "list")) 
	{
		print_mmc_devices('\n');
		return 0;
	}
	else if (!strcmp(argv[1], "help")) 
	{
		print_mmc_usage();
		return 0;
	}
	
	// ========================================================================
	// ETT Releated Functions  (Begin)
	// ========================================================================
	#ifdef CONFIG_MSDC_ETT
	
	else if (!strcmp(argv[1], "ett")) 
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		int ett_type = 0;
		if (!strcmp(argv[3], "w"))
		{
			ett_type = 1;
		}
		
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;
		
		mmc_ett(mmc, dev_num, ett_type);
		return 0;
	}
	/* Format: mmc pss <0/1/2> <s/g> <r/w> <0/1>*/
	else if (!strcmp(argv[1], "pss")) //pre_setting state
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;

		if (!strcmp(argv[3], "s"))
		{
			int enable = simple_strtoul(argv[5], NULL, 10);
			int rw = 0; // default ops is read
			if (!strcmp(argv[4], "w"))
				rw = 1;
			else if (!strcmp(argv[4], "rw"))
				rw = 2;

			mmc_set_rw_pre_setting_en(mmc, rw, enable);
		}
		else if (!strcmp(argv[3], "g"))
		{
			mmc_get_rw_pre_setting_en(mmc);
		}
		
		return 0;
	}
	else if (!strcmp(argv[1], "reset")) // sw/hw reset whole module
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;

		int reset_type = simple_strtoul(argv[3], NULL, 10);
		mmc_hw_reset_whole_module(mmc, reset_type);

		return 0;			
	}
	else if (!strcmp(argv[1], "pads")) // pin pad setting
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;

		if (!strcmp(argv[3], "s"))
		{
			u32 clk_drv 	= simple_strtoul(argv[4], NULL, 16);
			u32 cmd_drv 	= simple_strtoul(argv[5], NULL, 16);
			u32 dat_drv 	= simple_strtoul(argv[6], NULL, 16);
			u32 resistor 	= simple_strtoul(argv[7], NULL, 16);
			u32 slew_rate 	= simple_strtoul(argv[8], NULL, 16);
			
			mmc_set_pad_params(mmc, clk_drv, cmd_drv, dat_drv, resistor, slew_rate);
		}
		else
		{
			mmc_get_pad_params(mmc);
		}
		return 0;			
	}
	else if (!strcmp(argv[1], "sclk")) // change msdc work clock and reinit it
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;

		if (argc < 4)
		{
			print_mmc_sclk_usage();
			return 0;
		}
		
		int ddr = strcmp(argv[3], "sdr");
		int clk = simple_strtoul(argv[4], NULL, 10);
		mmc_set_work_clock(mmc, ddr, clk);

		return 0;			
	}
	else if (!strcmp(argv[1], "pres")) // set/get pre_setting params
	{
		int rw = 0;
		int ddr = 0;
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;

		// rw flag
		rw = 0; // default ops is read
		if (!strcmp(argv[4], "w"))
			rw = 1;

		// ddr flag
		ddr = 0; // default ops is read
		if (!strcmp(argv[5], "ddr"))
			ddr = 1;

		if (!strcmp(argv[3], "s"))
		{
			u32 ck_sel 			= simple_strtoul(argv[6], NULL, 10);
			u32 ckgen_delay 	= simple_strtoul(argv[7], NULL, 10);
			u32 pad_delay 		= simple_strtoul(argv[8], NULL, 10);
			u32 internal_delay 	= simple_strtoul(argv[9], NULL, 10);
			u32 sample_edge 	= simple_strtoul(argv[10],NULL, 10);
			
			if (rw) // Write
				msdc_set_write_pre_setting_params(mmc, ddr, ck_sel, ckgen_delay, pad_delay, internal_delay, sample_edge);
			else
				msdc_set_read_pre_setting_params(mmc, ddr, ck_sel, ckgen_delay, pad_delay, sample_edge);
		}
		else if (!strcmp(argv[3], "g"))
		{
			if (rw) // Write
				msdc_get_write_pre_setting_params(mmc, ddr);
			else
				msdc_get_read_pre_setting_params(mmc, ddr);
		}
		
		return 0;
	}
	// =============== hs200 command ===============
	// Get or Set HS200 mode for eMMC
	// =========================================
	else if (!strcmp(argv[1], "hs200"))
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;

		if (!strcmp(argv[3], "s"))
		{
			int enable = simple_strtoul(argv[4], NULL, 10);
			mmc_set_hs200_mode(mmc, enable);
		}
		else if (!strcmp(argv[3], "g"))
		{
			mmc_get_hs200_mode(mmc);
		}
		return 0;
	}
	// ================ scs command ===============
	// Set clock source for SD Card or eMMC
	// =========================================
	else if (!strcmp(argv[1], "scs"))
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;

		if (!strcmp(argv[3], "s"))
		{
			int clock = simple_strtoul(argv[4], NULL, 10);
			int reinit = simple_strtoul(argv[5], NULL, 10);
			mmc_set_clock_source(mmc, clock, reinit);
		}
		else if (!strcmp(argv[3], "g"))
		{
			mmc_get_clock_source(mmc);
		}
		return 0;
	}
	// =============== dump command ===============
	// Dump eMMC contents to external SD card
	// =========================================
	else if (!strcmp(argv[1], "dump"))
	{
		// source device
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;

		// dst device
		dst_dev = simple_strtoul(argv[3], NULL, 10);
		dst_mmc = find_mmc_device(dst_dev);
		err = mmc_init(dst_mmc);
		if (err)
        	return err;

		u64 dst_offset = 0, start_addr = 0, size = 0;
		simple_strtou64(argv[4], NULL, 16, &dst_offset);
		simple_strtou64(argv[5], NULL, 16, &start_addr);
		simple_strtou64(argv[6], NULL, 16, &size);
		
		mmc_dump_data(mmc, dst_mmc, dst_offset, start_addr, size);

		return 0;
	}
	// =============== erase command ===============
	// Erase/Trim/Discard eMMC
	// =========================================
	else if (!strcmp(argv[1], "erase"))  
	{
		// source device
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;

		u32 type = simple_strtoul(argv[3], NULL, 10);
		//u64 start_addr = 0x200000, size = 0x200000;  //2097152
		
		u64 start_addr = 0, size = 1024;
		simple_strtou64(argv[4], NULL, 16, &start_addr);
		simple_strtou64(argv[5], NULL, 16, &size);
		
		mmc_erase(mmc, type, start_addr, size);

		return 0;
	}
	// =============== dreg command ===============
	// Print out register content of SD/eMMC
	// =========================================
	else if (!strcmp(argv[1], "dreg"))
	{
		// source device
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;

		u32 type = 0;
		mmc_dump_register(mmc, type);

		return 0;
	}

	else if (!strcmp(argv[1], "tune_emmc"))
	{
		mmc = find_mmc_device(0); // eMMC default uses SD Slot0
		err = mmc_init(mmc);
		if (err)
        	return err;

		mmc_set_hs200_mode(mmc, 1);				// Enable HS200 Timing
		mmc_get_hs200_mode(mmc);				// Check HS200 Timing
		mmc_set_work_clock(mmc, 0, 200);		// Select 200MHz Clock and re-init eMMC
		mmc_set_rw_pre_setting_en(mmc, 2, 0);	// Disable Pre-Setting for RW OPs
		mmc_ett(mmc, 0, 0);						// MSDC ETT for Read
		mmc_ett(mmc, 0, 1);						// MSDC ETT for Write
		
		return 0;
	}
	else if (!strcmp(argv[1], "hqa"))
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;
		if (!strcmp(argv[3], "w"))
			rw = 1;
		times = simple_strtoul(argv[4], NULL, 10);
		mmc_set_hs200_mode(mmc, 1);				// Enable HS200 Timing
		mmc_get_hs200_mode(mmc);				// Check HS200 Timing
		mmc_set_work_clock(mmc, 0, 200);		// Select 200MHz Clock and re-init eMMC
		
		mmc_hqa_test(mmc, rw, times);
		
		return 0;
	}
	else if (!strcmp(argv[1], "pattern"))
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
        	return err;
		
		mmc_set_hs200_mode(mmc, 1);				// Enable HS200 Timing
		mmc_get_hs200_mode(mmc);				// Check HS200 Timing
		mmc_set_work_clock(mmc, 0, 200);		// Select 200MHz Clock and re-init eMMC
		
		mmc_hqa_test(mmc, rw, times);
		
		return 0;
	}
	
	#endif //CONFIG_MSDC_ETT
	// ========================================================================
	// ETT Releated Functions (End)
	// ========================================================================
	else if(strncmp(argv[1], "ecsd_set", 8) == 0) {
		mmc = find_mmc_device(0);
		reg_id = simple_strtoul(argv[2], NULL, 10);
		val = simple_strtoul(argv[3], NULL, 16);
		if(reg_id >= 512)
			return -1;

		printf("=====write ext_csd[%d] to val [%d]...\n", reg_id, val);
		if(mmc_ext_csd_set(mmc, reg_id, val))
			return -1;
		else
			return 0;
	}
	else if(strncmp(argv[1], "ecsd_get", 8) == 0) {
		mmc = find_mmc_device(0);
		reg_id = simple_strtoul(argv[2], NULL, 10);
		if(reg_id >= 512)
			return -1;

		printf("=====get ext_csd[%d] val...\n", reg_id);
		if(mmc_ext_csd_get(mmc, reg_id, &val))
			return -1;
		else {
			printf("ext_csd[%d]=0x%x\n", reg_id, val);
			return 0;
		}
	}
	else if (strcmp(argv[1], "wp_check") == 0)
	{
		mmc = find_mmc_device(0);
		if (!mmc) {
			return 1;
		}
		if(argc >= 3) {
			addr_blk = simple_strtoul(argv[2], NULL, 16);
			cnt_blk = simple_strtoul(argv[3], NULL, 16);
		}
		else {
			addr_blk = 0;
			cnt_blk = mmc->ext_csd.sectors;
		}
		return mmc_dump_wp_status(mmc, addr_blk, cnt_blk);
	}
	else if(strcmp(argv[1], "wp_set") == 0) {
		if(argc < 5) {
			return 1;
		}

		addr_blk = simple_strtoul(argv[3], NULL, 16);
		cnt_blk = simple_strtoul(argv[4], NULL, 16);
		if(strcmp(argv[2], "on") == 0) {
			err = emmc_set_user_wp(WP_ENABLE, addr_blk, cnt_blk, 1);
		}
		else if(strcmp(argv[2], "off") == 0) {
			err = emmc_set_user_wp(WP_DISABLE, addr_blk, cnt_blk, 1);
		}
		if(!err)
			printf("set wp %s success\n", argv[2]);
		return err;
	}
	else if(strcmp(argv[1], "wp_clear") == 0) {
		return emmc_clear_all_wp();
	}
#if 0
	else if(strcmp(argv[1], "wp_test") == 0) {
		wp_test();
		return 0;
	}
#endif
#ifdef MSDC_PERFORMANCE_TEST
	else if(strcmp(argv[1], "test_perf") == 0) {
		dev_num = simple_strtoul(argv[2], NULL, 10);
		clock=simple_strtoul(argv[3], NULL, 10);
		rw = simple_strtoul(argv[4], NULL, 16);
	    size_mb = simple_strtoul(argv[5], NULL, 10);
		printf("dev_num %d, clock %d,rw %d, size_mb %d \n",
			dev_num,clock,rw,size_mb);
		mmc = find_mmc_device(dev_num);
		msdc_change_clock_source(mmc, clock);
		if(mmc_force_reinit(mmc)){
			printf("mmc init fail and return \n");
			return 1;
		}
		
		cnt_blk= size_mb*1024*2;// MB

		time1 = boot_time_ms();
		if(rw)
			real_blk = mmc->block_dev.block_read(dev_num, MMC_TSET_BLK_ADDR, cnt_blk, (void *)MEM_TEST_ADDR);
		else
			real_blk = mmc->block_dev.block_write(dev_num, MMC_TSET_BLK_ADDR, cnt_blk, (void *)MEM_TEST_ADDR);
		time2=boot_time_ms();
		err=(real_blk == cnt_blk) ? 0 : 1;

		if(err)
			printf("test perf fail \n");
		else
			printf(" === %d MB Data %s cost %lld ms and speed is %d MB\\S ===\n",size_mb,
			rw? "Read":"Write",time2-time1,(1000/(time2-time1))*size_mb);
	}
#endif
	else {
		printf("error command: %s, Usage:\n%s\n", argv[1], cmdtp->usage);
		return 1;
	}

	return err;
}

U_BOOT_CMD(
	mmc, 12, 1, do_mmcops,
	"MMC sub system",
	"mmc read <device num> addr blk# cnt\n"
	"mmc write <device num> addr blk# cnt\n"
	"mmc format <device num>\n"
	"mmc list - lists available devices");

#endif  //CONFIG_MTK_SD

//================================================================================================//

#ifdef CONFIG_ATC_MSDC

#include "../drivers/mmc/atc_mmc_core.h"

int do_mmcops(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int err = 0;
	int dev_num = -1;
	int opcode = 0;
	ulong real_blk = 0;
	void *pbuf = NULL;
	u32 cnt_blk = 0;
	u32 addr_blk = 0;
	struct mmc_host *mmc = NULL;
	
	if (argc < 2)
	{
		printf("error command parameters number!!\n");
		return 1;
	}

	if (strcmp(argv[1], "rescan") == 0) 
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		mmc = find_mmc_device(dev_num);
		if (!mmc)
		{
			return 1;
		}

		//err = mmc_init(mmc);

		return err;
	}
	// ================   mmc read op =================== 
		// 0 - mmc 
		// 1 - read
		// 2 - 0 --> mmc slot select
		// 3 - Read Buffer Pointer
		// 4 - Read Address (in block)
		// 5 - Read Buffer Size (in block)
	if (strcmp(argv[1], "read") == 0) 
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		pbuf = (void *)simple_strtoul(argv[3], NULL, 16);
		cnt_blk = simple_strtoul(argv[5], NULL, 16);
		addr_blk = simple_strtoul(argv[4], NULL, 16);
		mmc = find_mmc_device(dev_num);
		if (!mmc)
		{
			return 1;
		}
		//printf("\nMMC read: dev # %d, block # %d, count %d ... ", dev, blk, cnt);

		err = mmc_init(mmc);
		if (err)
		{
			return err;
		}

		real_blk = mmc_block_read(dev_num, addr_blk, cnt_blk, pbuf);

		/* flush cache after read */
		flush_cache((ulong)pbuf, cnt_blk * 512); /* FIXME */

		//printf("%d blocks read: %s\n", n, (n==cnt) ? "OK" : "ERROR");
			
		return (real_blk == cnt_blk) ? 0 : 1;
	} 
	// ================   mmc write op =================== 
		// 0 - mmc 
		// 1 - write
		// 2 - 0 --> mmc slot select
		// 3 - Write Buffer Pointer
		// 4 - Write Address (in block)
		// 5 - Write Buffer Size (in block)
	else if (strcmp(argv[1], "write") == 0) 
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		pbuf = (void *)simple_strtoul(argv[3], NULL, 16);
		cnt_blk = simple_strtoul(argv[5], NULL, 16);
		addr_blk = simple_strtoul(argv[4], NULL, 16);
		mmc = find_mmc_device(dev_num);
		if (!mmc)
		{
			return 1;
		}
		//printf("\n MMC write: dev_num=%u, addr_blk=%u, cnt_blk=%u\n", dev_num, addr_blk, cnt_blk);

		err = mmc_init(mmc);
		if (err)
		{
			return err;
		}
		real_blk = mmc_block_write(dev_num, addr_blk, cnt_blk, pbuf);
		return (real_blk == cnt_blk) ? 0 : 1;
	}
	// ================   mmc format userdata partition ===================
		// Command format:
		// 0 - mmc
		// 1 - format
		// 2 - 0 --> mmc slot select
		// 3 - userdata
		// 4 - partition start address (in byte)
		// 5 - partition size (in byte)
	/*else if (strcmp(argv[1], "format") == 0)
	{
		dev_num = simple_strtoul(argv[2], NULL, 10);
		//char* partition = (char*)argv[3];
		u32 offset = simple_strtoul(argv[4], NULL, 16);
		u32 size = simple_strtoul(argv[5], NULL, 16);
		mmc = find_mmc_device(dev_num);
		err = mmc_init(mmc);
		if (err)
		{
			return err;
		}

		printf("\n---------> format_userdata_partition: offset = 0x%X, size = 0x%X <----------\r\n", offset, size);

		err = format_userdata_partition(mmc, dev_num, offset, size);
		if (err)
		{
			return err;
		}	
		return 0;
	}*/
	
	//=====================  emmc boot partition op  =========================
	//mmc  boot  opcode [OP Param] [R/W buffer Pointer] [R/W Address Pointer] [R/W Size]
	//==================================================================
	else if (strcmp(argv[1], "boot") == 0) // for emmc boot partition op
	{
		dev_num = 0; // hardcode to 0, because only slot0 supports emmc boot partition.
		opcode = simple_strtoul(argv[2], NULL, 10);
		switch (opcode)
		{
		case BOOT_OP_ENTER_BOOT_MODE:
			err = mmc_boot_enter_bootmode(dev_num);
			break;
			
		case BOOT_OP_EXIT_BOOT_MODE:
			err = mmc_boot_exit_bootmode(dev_num);
			break;
			
		case BOOT_OP_WRITE_BOOT_PART1:	 // write boot partition 0
		case BOOT_OP_READ_BOOT_PART1:	 // read boot partition 0
		case BOOT_OP_WRITE_BOOT_PART2:	 // write boot partition 1
		case BOOT_OP_READ_BOOT_PART2:	 // read boot partition 1
			pbuf = (void *)simple_strtoul(argv[3], NULL, 16);
			addr_blk = simple_strtoul(argv[4], NULL, 16);
			cnt_blk = simple_strtoul(argv[5], NULL, 16);
			
			int write_op = 0;
			int boot_part_num = BOOT_PART_PART1; // default op on boot partition 1
			if ((opcode == BOOT_OP_WRITE_BOOT_PART1) || (opcode == BOOT_OP_WRITE_BOOT_PART2))
			{
				write_op = 1;
			}

			if ((opcode == BOOT_OP_WRITE_BOOT_PART2) || (opcode == BOOT_OP_READ_BOOT_PART2))
			{
				boot_part_num = BOOT_PART_PART2;
			}

			if (write_op)
			{
				real_blk = mmc_boot_bwrite(dev_num, boot_part_num, addr_blk, cnt_blk, pbuf);
			}
			else
			{
				real_blk =	mmc_boot_bread(dev_num, boot_part_num, addr_blk, cnt_blk, pbuf);
			}
			err = (real_blk == cnt_blk) ? 0 : 1;
			break;
			
		default:
			err = 1;
			break;
				
		}

		return err;
	}
	else if (!strcmp(argv[1], "list")) 
	{
		print_mmc_devices('\n');
		return 0;
	}
	else if (!strcmp(argv[1], "insert")) 
	{
#if 0
		  if (sdhci_card_exist())
		  {
			  printf("Card inserted ^^\n");
		  }
		   else
		  {
			  printf("Card not inserted !!\n");
		  }
#endif
		return 0;
	}
	else
	{
		printf("error command: %s, Usage:\n%s\n", argv[1], cmdtp->usage);
		return 1;
	}

	return err;
}


int format_userdata_partition_u64(struct mmc *mmc, int dev, u64 u8PartOffset, u64 u8PartSize)
{
	return 0;
}

U_BOOT_CMD(
	mmc, 6, 1, do_mmcops,
	"MMC sub system, device_num = 0, 1, 2",
	"mmc read <device num> addr blk# cnt\n"
	"mmc write <device num> addr blk# cnt\n"
	"mmc format <device num>\n"
	"mmc init <device_num>"
	"mmc register <device_num>"
	"mmc list - lists available devices");

#endif  //CONFIG_ATC_MSDC

#endif
