#include"usbfntypedef.h"
#include"common.h"
#include <nand.h>
#include <mmc.h>

#include <partition.h>

#define WRITE_SEGMENT_SIZE		(8*1024*1024)				// For nand page size align
static unsigned g_write_segment_size = WRITE_SEGMENT_SIZE;	// For nand page size align

static BOOL  g_fgBoot = FALSE;
static DWORD g_dwTotalRead = 0;
#define SUPPORT_UBI 			0 
unsigned rx_addr = BASE_ADDR, rx_length = 0;

static unsigned long long part_write_offset = 0;			// Relative offset to the partition start offset
static char current_write_part_name[32];		
static char current_write_part_type[32];
#ifdef NEW_PARTITION_DESIGN
static u32 current_part_flag = 0;
#endif
static unsigned long long current_write_part_offset = 0;	// Relative offset to internal storage address ox00000000

#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
u32 curr_wp_offset = 0;
u32 curr_wp_size = 0;
#endif

#define MAX_USB_DWONLOAD_SIZE	(300*1024*1024)		// 300M

static UINT32 g_u4ImageMaxSize = MAX_USB_DWONLOAD_SIZE;


#define REPLICATION_NUMBER  8
static  char _szBLID1[12] = "BOOTLOADER!";
static char _szBLNFIID2[8] = "NFIINFO";
static char _szBLMSDCID2[8] = "AC83XX";

typedef struct _Fb_PartInfo
{
   char   	szPartName[16];   
   char	  	szPartType[16];
   UINT32   u4PartStartAddr;   
   unsigned long long u8PartStartAddr;
   UINT32   u4PartSize;
   unsigned long long u8PartSize;
   struct _Fb_PartInfo* pNextPart;
#ifdef NEW_PARTITION_DESIGN
   UINT32 	u4Flag;
#endif
} FB_PART_INFO, *LP_FB_PART_INFO;

static LP_FB_PART_INFO g_pPartInfoHeader = NULL;

partitionhead parthead;
partitionread *pCurpart= NULL,*pPrepart= NULL;
int partcnt = 0;


typedef struct _NFIType
{
   UINT16   pageSize;   
   UINT16   spareSize;
   UINT16   addressCycle;   
   UINT16   pageShift;
} NFI_MENU;

typedef struct _BOOTLHeader_
{
   char ID1[12];
   char version[4];
   UINT32 length;
   UINT32 startAddr;
   UINT32 checksum;
   char ID2[8];
   NFI_MENU  NFIinfo;
   UINT16 pagesPerBlock;   
   UINT16  totalBlocks;
   UINT16  blockShift;
   UINT16  linkAddr[6];   
   UINT16  lastBlock;
} BOOTL_HEADER;


extern void EdbgOutputDebugString (LPCSTR sz, ...);

BOOL  USBFNWriteData(LPVOID lpBuffer,  DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten);

BOOL  USBFNReadData(LPVOID lpBuffer, DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead);


#define VERSION "0.5"
#define PRODUCTNAME "AC83XX Car-kit"
static char serialno[] = "123456789";

static char bl_command_line[300];
static int   bl_cmmand_line_length = 0;
static nand_info_t *nand = &nand_info[0];

static int bUpgradePartinfo = 0;
#ifdef NEW_PARTITION_DESIGN
static int bFastbootUpgrade = 1;
#endif


//======================================================================================
//  
//======================================================================================

// Function 

//======================================================================================
//  
//======================================================================================

static UINT32  Log2(UINT32 value)
{
	UINT32 rc = 0;

	while ( 0 != value )
	{
		value >>= 1;
		rc++;
	}
	rc--;
	return rc;
}

static BOOL  MemCompare(UINT32 *pDest,UINT32 *pSrc,UINT32 u4Length)
{
    //UINT32 *pBuff;
    //int i;

    while(u4Length >0)
    {
     	 if(*pDest != *pSrc)
     	 {
             printf("Compare Address Src:0x%x Des:0x%x\r\n",pSrc,pDest);
     	     return FALSE;
     	 }
         pDest++;
         pSrc++;
         u4Length-=4;
    }

    return TRUE;
}

static int isCurrentPartExt4(char* partname)
{
    if (!strcmp(partname, "system") || !strcmp(partname, "system_a") || !strcmp(partname, "system_b"))
        return 1;

    if (!strcmp(partname, "usrdata") || !strcmp(partname, "data"))
        return 1;

    return 0;
}

#ifndef CONFIG_BOOT_MMC
static partitionread *get_part_info_by_name(char* partname)
{
    partitionread *part;

    part = readpartitioninfofromnand();
    while(part) {
        //dumppartitionread(part);
        if (!strcmp("datazone", partname) || !strcmp("datazone_bk", partname)) {
            printf("Do not upgrade datazone through fastboot cmd !!!\n");
            return NULL;
        }
        if (!strcmp(partname, part->szPartName)) {
            printf("find partiton[%s] in partition table\n", partname);
            return part;
        }
        part = part->nextpartition;
    }

    printf("can not find partiton[%s]\n", partname);
    return NULL;
}
#else
static partitionread *get_part_info_by_name(char* partname)
{
    partitionread *part;

    part = readpartitioninfofromflash();
    while(part) {
        if (!strcmp("datazone", partname) || !strcmp("datazone_bk", partname)) {
            printf("Do not upgrade datazone through fastboot cmd !!!\n");
            return NULL;
        }
        if (!strcmp(partname, part->szPartName)) {
            printf("find partition[%s] in partition table\n", partname);
            return part;
        }
        part = part->nextpartition;
    }

    printf("can not find partition[%s]\n", partname);
    return NULL;
}
#endif

// 0 -- success, 1 -- parameter error, 2 -- new failed
static partitioninfo * add_partition_info (int argc, char *argv[])
{
	char* endptr = NULL;
	LP_FB_PART_INFO prNewPartInfo = NULL;
	partitioninfo *ppart;

	// If it is the first part info
	if (!g_pPartInfoHeader)
	{
		printf("add_partition_info 1111\r\n");
		g_pPartInfoHeader = (LP_FB_PART_INFO)malloc(sizeof(FB_PART_INFO));
		if (!g_pPartInfoHeader)
		{
			printf("malloc failed.\r\n");
			return 0;
		}
		g_pPartInfoHeader->pNextPart = NULL;
		strcpy(g_pPartInfoHeader->szPartName, argv[0]);
		strcpy(g_pPartInfoHeader->szPartType, argv[1]);
		//g_pPartInfoHeader->u4PartStartAddr = simple_strtoul(argv[2], endptr, 16);
		//g_pPartInfoHeader->u4PartSize = simple_strtoul(argv[3], endptr, 16);
		//g_pPartInfoHeader->u8PartStartAddr = simple_strtoull(argv[2], endptr, 16);
		simple_strtou64(argv[2], endptr, 16,&(g_pPartInfoHeader->u8PartStartAddr));
		simple_strtou64(argv[3], endptr, 16,&(g_pPartInfoHeader->u8PartSize));
#ifdef NEW_PARTITION_DESIGN
		simple_strtou64(argv[4], endptr, 16,&(g_pPartInfoHeader->u4Flag));
#endif
		//g_pPartInfoHeader->u8PartSize = simple_strtoull(argv[3], endptr, 16);
		//g_pPartInfoHeader->u8PartSize = 0x123456789ABCD;

		printf("add_partition_info artStartAddr:0x%s,PartSize:0x%s\r\n",argv[2],argv[3]);
		printf("---> add_partition_info partition_size: 0x%08X, 0x%08X <---\n", (u32)(g_pPartInfoHeader->u8PartSize >> 32), (u32)(g_pPartInfoHeader->u8PartSize & 0xFFFFFFFF));

		if(bUpgradePartinfo == 1)
		{
			ppart = (partitioninfo *)malloc(sizeof(partitioninfo));
			if (!ppart)
			{
				printf("ppart malloc failed.\r\n");
				return 0;
			}
			strcpy(ppart->szPartName,g_pPartInfoHeader->szPartName);
			strcpy(ppart->szType,g_pPartInfoHeader->szPartType);
			ppart->u8PartitionStartAddr = g_pPartInfoHeader->u8PartStartAddr;//nedd modify wj
			ppart->u8PartitionSize = g_pPartInfoHeader->u8PartSize;
#ifdef NEW_PARTITION_DESIGN
			ppart->u4Flag = g_pPartInfoHeader->u4Flag;
#endif
			return ppart;
		}
		printf("add_partition_info 222\r\n");
		return 1;
	}

	LP_FB_PART_INFO prPartInfo = g_pPartInfoHeader;
	while(prPartInfo)
	{
		if (strcmp(prPartInfo->szPartName, argv[0]) == 0)
		{
			//printf("partition info already exist,update it.\r\n");
			// update already exist item
			strcpy(prPartInfo->szPartType, argv[1]);
			//prPartInfo->u4PartStartAddr = simple_strtoul(argv[2], endptr, 16);
			//prPartInfo->u4PartSize = simple_strtoul(argv[3], endptr, 16);
			//prPartInfo->u8PartStartAddr = simple_strtoull(argv[2], endptr, 16);
			simple_strtou64(argv[2], endptr, 16,&(prPartInfo->u8PartStartAddr));
			//prPartInfo->u8PartSize = simple_strtoull(argv[3], endptr, 16);
			simple_strtou64(argv[3], endptr, 16,&(prPartInfo->u8PartSize));
#ifdef NEW_PARTITION_DESIGN
			simple_strtou64(argv[4], endptr, 16,&(prPartInfo->u4Flag));
#endif
			return 1;
		}

		// find next item
		if (prPartInfo->pNextPart)
		{
			prPartInfo = prPartInfo->pNextPart;
		}
		else
		{
			break;
		}
	}

	// not find item in exist items, create it
	printf("malloc partition info...\r\n");
	prNewPartInfo = (LP_FB_PART_INFO)malloc(sizeof(FB_PART_INFO));
	if (!prNewPartInfo)
	{
		printf("malloc failed.\r\n");
		return 0;
	}
	
	printf("malloc partition info success.\r\n");
	// Add to partition info chain
	prPartInfo->pNextPart = prNewPartInfo;
	prNewPartInfo->pNextPart = NULL;
	strcpy(prNewPartInfo->szPartName, argv[0]);
	strcpy(prNewPartInfo->szPartType, argv[1]);
	//prNewPartInfo->u4PartStartAddr = simple_strtoul(argv[2], endptr, 16);
	//prNewPartInfo->u4PartSize = simple_strtoul(argv[3], endptr, 16);
	//prNewPartInfo->u8PartStartAddr = simple_strtoull(argv[2], endptr, 16);
	simple_strtou64(argv[2], endptr, 16,&(prNewPartInfo->u8PartStartAddr));
	//prNewPartInfo->u8PartSize = simple_strtoull(argv[3], endptr, 16);
	simple_strtou64(argv[3], endptr, 16,&(prNewPartInfo->u8PartSize));
#ifdef NEW_PARTITION_DESIGN
	simple_strtou64(argv[4], endptr, 16,&(prNewPartInfo->u4Flag));
#endif

    if(bUpgradePartinfo == 1)
    {
        ppart = (partitioninfo *)malloc(sizeof(partitioninfo));
		if (!ppart)
		{
			printf("ppart malloc failed.\r\n");
			return 0;
		}
	    strcpy(ppart->szPartName,prNewPartInfo->szPartName);
	    strcpy(ppart->szType,prNewPartInfo->szPartType);
		ppart->u8PartitionStartAddr = prNewPartInfo->u8PartStartAddr;//nedd modify wj
		ppart->u8PartitionSize = prNewPartInfo->u8PartSize;
#ifdef NEW_PARTITION_DESIGN
		ppart->u4Flag = prNewPartInfo->u4Flag;
#endif

		return ppart;
		
    }

	return 1;

}

static void dump_partition_info()
{
	if (!g_pPartInfoHeader)
	{
		return;
	}
	
	LP_FB_PART_INFO prPartInfo = g_pPartInfoHeader;
	while(prPartInfo)
	{
		printf("============================================\r\n");
		printf("partition name: %s\r\n", prPartInfo->szPartName);
    	printf("partition type: %s\r\n", prPartInfo->szPartType);
    	printf("partition Start address: 0x%x\r\n", prPartInfo->u4PartStartAddr);
   		printf("partition Size: 0x%x\r\n", prPartInfo->u4PartSize);
#ifdef NEW_PARTITION_DESIGN
		printf("partition Flag:0x%x\r\n", prPartInfo->u4Flag);
#endif
		printf("============================================\r\n");		

		// Next
		if (prPartInfo->pNextPart)
		{
			prPartInfo = prPartInfo->pNextPart;
		}
		else
		{
			break;
		}
	}
	
}

static int find_partition_info(char* partition_name, LP_FB_PART_INFO* prRetPartInfo)
{
	if ((!g_pPartInfoHeader) || (!partition_name) )
	{
		*prRetPartInfo = NULL;
		return 1;
	}

	LP_FB_PART_INFO prPartInfo = g_pPartInfoHeader;
	while(prPartInfo)
	{
		if (strcmp(prPartInfo->szPartName, partition_name) == 0)
		{
			// return already exist partition item
			*prRetPartInfo = prPartInfo;
			return 0;
		}

		// try to find next partition item
		if (prPartInfo->pNextPart)
		{
			prPartInfo = prPartInfo->pNextPart;
		}
		else
		{
			break;
		}
	}

	// Not find match partition.
	*prRetPartInfo = NULL;
	return 2;
}

static BOOL  fastboot_memory_compare(UINT32 *pDest,UINT32 *pSrc,UINT32 u4Length)
{
    while (u4Length >0)
    {
        if (*pDest != *pSrc)
        {
            printf("Compare Address Src:0x%x Des:0x%x\r\n", pSrc, pDest);
            return FALSE;
        }
        pDest++;
        pSrc++;
        u4Length-=4;
    }

    return TRUE;
}

#ifndef CONFIG_BOOT_MMC
static void fastboot_create_bootloader_header(char *pBLHeader, char* blbuf, UINT32 u4Imagesize)
{
	BOOTL_HEADER BLHeader;
	UINT32 u4chksum,temp32;
	unsigned char *pu4Buf = (unsigned char *)blbuf;
	UINT32 i;
	nand_info_t *nand = &nand_info[0];
	struct nand_chip *chip = nand->priv;
	

	memset(&BLHeader,0x0,sizeof(BOOTL_HEADER));
	memcpy(BLHeader.ID1,_szBLID1,12);
	memcpy(BLHeader.ID2,_szBLNFIID2,8);
	BLHeader.startAddr = 0x40000000;
	BLHeader.length = u4Imagesize;

	BLHeader.NFIinfo.pageSize = (UINT16)nand->writesize;
  
    BLHeader.pagesPerBlock = 256;
    BLHeader.totalBlocks = (UINT16)0x800;

    BLHeader.NFIinfo.spareSize =  nand->oobsize;

    printf("oob size[%d]",BLHeader.NFIinfo.spareSize);
    BLHeader.NFIinfo.addressCycle =(UINT16)0x5;

    if(BLHeader.NFIinfo.pageSize > 512)
       BLHeader.NFIinfo.pageShift = (UINT16)0x10;
     else 
       BLHeader.NFIinfo.pageShift = (UINT16)0x8;


    BLHeader.blockShift = (UINT16)(Log2(BLHeader.pagesPerBlock) + BLHeader.NFIinfo.pageShift);
   u4chksum = 0;
   for(i = 0; i< u4Imagesize ; i+=4)
   {

	   memcpy(&temp32,((unsigned char *)pu4Buf + i),4);
       u4chksum^=temp32;
   }

   BLHeader.checksum = u4chksum;
   for(i = 0; i < REPLICATION_NUMBER;i++)
   {
       memcpy(pBLHeader,&BLHeader,sizeof(BOOTL_HEADER));
	   pBLHeader = (unsigned char *)pBLHeader + sizeof(BOOTL_HEADER);
   }

      
   return;
}

#else

#define MAX_PRELOADER_SIZE		(16*1024)	//16KB
static void fastboot_create_bootloader_header(char *pBLHeader, char* blbuf, UINT32 u4Imagesize)
{
	UINT32 u4chksum, temp32;

    BYTE *pu4Buf = (BYTE *)blbuf;
    UINT32 i = 0;

    BOOTL_HEADER BLHeader;
    memset(&BLHeader, 0xCC, sizeof(BOOTL_HEADER));

    memcpy(BLHeader.ID1, _szBLID1, 12);

    memcpy(BLHeader.ID2, _szBLMSDCID2, 8);

	//  Check Max Size
	if (u4Imagesize > MAX_PRELOADER_SIZE)
	{
		u4Imagesize = MAX_PRELOADER_SIZE;
	}
    
    BLHeader.length = u4Imagesize;

    u4chksum = 0;
    for (i = 0; i < u4Imagesize; i += 4)
    {
        memcpy(&temp32, ((BYTE *)blbuf + i), 4);
        u4chksum ^= temp32;
    }
    BLHeader.pagesPerBlock = 0;
    BLHeader.totalBlocks = 0;
    BLHeader.checksum = u4chksum;

    for (i = 0; i < REPLICATION_NUMBER; i++)
    {
        memcpy(pBLHeader, &BLHeader, sizeof(BOOTL_HEADER));
        pBLHeader = (BYTE *)pBLHeader + sizeof(BOOTL_HEADER);
    }

    return TRUE;
}

#endif
static int fastboot_read_nand_image(u32 base_addr, char *partitionName, unsigned size)
{
    char buf1[10] = {0};
    char buf2[10] = {0};
    char *argv[6] = {"nand", "read"};

    sprintf(buf1, "%x", base_addr);
    argv[2] = buf1;
    argv[3] = partitionName;
    sprintf(buf2, "%x", size);
    argv[4] = buf2;
    // do_nand, Parameter 'argv' : (2013-11-08)
    // 0 - nand
    // 1 - write
    // 2 - Write Buffer Pointer
    // 3 - Partition Name
    // 4 - Partition Offset (Optional)
    // 5 - Partition Size, Real Write Buffer size (Optional)
    return do_nand(NULL, 0, 4, argv);
}

static int fastboot_raw_image_write(partitionread* prPartInfo, UINT32 u4ImageSize)
{
    //nand_info_t *nand = &nand_info[nand_curr_device];

    UINT32 u4WriteSize = ALIGN(u4ImageSize, nand->writesize);

    printf("fastboot_raw_image_write size:0x%x\r\n", u4WriteSize);
    if (0 == strcmp(prPartInfo->szPartName, "preloader") || 0 == strcmp(prPartInfo->szPartName, "preloader_bk"))
    {
        fastboot_create_bootloader_header((char *)(NAND_WRITE_BASE_ADDR - 512), NAND_WRITE_BASE_ADDR, 0x7000);
        u4WriteSize = ALIGN((u4WriteSize + 512), nand->writesize);
	printf("preloader write size:0x%x\r\n", u4WriteSize);
        if (write_nand_ex((uchar*)(NAND_WRITE_BASE_ADDR - 512), u4WriteSize, prPartInfo->szPartName, 0, prPartInfo->szType,0))
        {
            printf("write nand failed\r\n");
            return 1;
        }
#if 0
        fastboot_read_nand_image(0x100000, prPartInfo->szPartName, u4WriteSize);//0x200000

        if (fastboot_memory_compare(0x100000, (NAND_WRITE_BASE_ADDR - 512), u4WriteSize))
        {
            printf("memory compare success, 0x%X\r\n", u4WriteSize);
        }
        else
        {
            printf("memory compare failed, 0x%X\r\n", u4WriteSize);
	    return 1;
        }
#endif
    }
    else
    {
        if (write_nand_ex((uchar*)NAND_WRITE_BASE_ADDR, u4WriteSize, prPartInfo->szPartName, 0, prPartInfo->szType,0))
        {
            printf("write nand failed\r\n");
            return 1;
        }
    }

    return 0;
}

static int  fastboot_ext4_image_write(partitionread* prPartInfo, UINT32 u4ImageSize)
{
    struct mtd_device *dev;
    struct part_info *mtd_part;
    u8 pnum;

    if ((find_dev_and_part(prPartInfo->szPartName, &dev, &pnum, &mtd_part) != 0))
    {
    	return 1;
    }

    if (write_nand_ex((uchar*)NAND_WRITE_BASE_ADDR, u4ImageSize, prPartInfo->szPartName, 0, prPartInfo->szType,0))
    {
        printf("write nand failed\r\n");
        return 1;
    }

    return 0;
}

typedef struct sparse_header {
  __le32	magic;		/* 0xed26ff3a */
  __le16	major_version;	/* (0x1) - reject images with higher major versions */
  __le16	minor_version;	/* (0x0) - allow images with higer minor versions */
  __le16	file_hdr_sz;	/* 28 bytes for first revision of the file format */
  __le16	chunk_hdr_sz;	/* 12 bytes for first revision of the file format */
  __le32	blk_sz;		/* block size in bytes, must be a multiple of 4 (4096) */
  __le32	total_blks;	/* total blocks in the non-sparse output image */
  __le32	total_chunks;	/* total chunks in the sparse input image */
  __le32	image_checksum; /* CRC32 checksum of the original data, counting "don't care" */
				/* as 0. Standard 802.3 polynomial, use a Public Domain */
				/* table implementation */
} sparse_header_t;

typedef struct write_result{
	unsigned long chunk_num;
	unsigned long long remain_filelen;
	unsigned long long u8WriteAddr;
}write_result_t;

#define SPARSE_HEADER_MAGIC	0xed26ff3a

#define CHUNK_TYPE_RAW		0xCAC1
#define CHUNK_TYPE_FILL		0xCAC2
#define CHUNK_TYPE_DONT_CARE	0xCAC3
#define CHUNK_TYPE_CRC32    0xCAC4

typedef struct chunk_header {
  __le16	chunk_type;	/* 0xCAC1 -> raw; 0xCAC2 -> fill; 0xCAC3 -> don't care */
  __le16	reserved1;
  __le32	chunk_sz;	/* in blocks in output image */
  __le32	total_sz;	/* in bytes of chunk input file including chunk header and data */
} chunk_header_t;

#define CHUNK_HEADER_LEN (sizeof(chunk_header_t))
#define SPARSE_HEADER_LEN (sizeof(sparse_header_t))

static int fastboot_parse_write_ext4_image(unsigned long long u8ReadAddr, unsigned long long u8WriteAddr, unsigned long long u8Size)
{
	int   ival = 0;
	uint32_t block_size = 0 ,n;
	uchar *pfileBuffer = NULL;
	int32_t filelen = (int32_t)u8Size;
	size_t datalen = 0;
	uint32_t chunk_cnt = 0;
	uint32_t dump_ext4_blockcnt = 10;
	uint32_t uLoop = 0;
	struct mmc *mmc = NULL;
	int emmc_dev_num = 0;

	sparse_header_t *ptSparseHeader = NULL;
	chunk_header_t  *ptChunkHeader = NULL;

	char *szValAddr1 = (char *)malloc(17);
	char *szValAddr2 = (char *)malloc(17);
	char* szPhyAddr  = (char *)malloc(17);

	mmc = find_mmc_device(emmc_dev_num);
	if (!mmc)
	{
		return -1;
	}
	if (mmc_init(mmc))
	{
		return -1;
	}

	ptSparseHeader = (sparse_header_t *)u8ReadAddr;
	block_size = ptSparseHeader->blk_sz;
	chunk_cnt = ptSparseHeader->total_chunks;

	printf ("fastboot_parse_write_ext4_image:ptSparseHeader->blk_sz = 0X%x\r\n", ptSparseHeader->blk_sz);
	printf ("fastboot_parse_write_ext4_image:ptSparseHeader->total_chunks = 0X%x\r\n", ptSparseHeader->total_chunks);
	printf ("fastboot_parse_write_ext4_image:ptSparseHeader->total_blks = 0X%x\r\n", ptSparseHeader->total_blks);    
	printf ("fastboot_parse_write_ext4_image:filelen = 0X%x\r\n", filelen);
	pfileBuffer = u8ReadAddr + sizeof(sparse_header_t);
	filelen -= sizeof(sparse_header_t);
	while ((chunk_cnt > 0) && (filelen > 0))
	{
		ptChunkHeader = (chunk_header_t *)pfileBuffer;
		datalen = ptChunkHeader->chunk_sz * block_size;
		
		printf ("upg_rsd_write_ext4_image:ptChunkHeader->chunk_type = %d\r\n", ptChunkHeader->chunk_type);
		printf ("upg_rsd_write_ext4_image:ptChunkHeader->chunk_sz = 0X%x\r\n", ptChunkHeader->chunk_sz);
		printf ("upg_rsd_write_ext4_image:datalen = 0X%x\r\n", datalen);   

		if (CHUNK_TYPE_RAW == ptChunkHeader->chunk_type)
		{
			pfileBuffer += CHUNK_HEADER_LEN;
#if 0
#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
			char *argv_write[6] = {"mmc","write","0", uitostr_hex(szPhyAddr,(unsigned int)pfileBuffer), uitostr_hex(szValAddr1,(unsigned int)(u8WriteAddr/512)), uitostr_hex(szValAddr2,(unsigned int)datalen/512)};						
#else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
			char *argv_write[6] = {"mmc","write","2", uitostr_hex(szPhyAddr,(unsigned int)pfileBuffer), uitostr_hex(szValAddr1,(unsigned int)(u8WriteAddr/512)), uitostr_hex(szValAddr2,(unsigned int)datalen/512)};
#endif
			if(0 != do_mmcops(NULL, 0, 6, argv_write))
			{
				printf("\nupg_rsd_write_ext4_image Failed!\n");
			}
#endif
			mmc->block_dev.block_write(0, (unsigned int)(u8WriteAddr/512), (unsigned int)(datalen/512), (void*)pfileBuffer);

			filelen -= ptChunkHeader->total_sz;
			chunk_cnt--;
			pfileBuffer+= datalen;
			u8WriteAddr += datalen;
		}
		else if (CHUNK_TYPE_DONT_CARE == ptChunkHeader->chunk_type)
		{
			u8WriteAddr += datalen;
			pfileBuffer += ptChunkHeader->total_sz;
			filelen -= ptChunkHeader->total_sz;
			chunk_cnt--;
		}
		else
		{
			printf ("\nupg_rsd_write_ext4_image:ptChunkHeader->chunk_type error\r\n");
			return -1;
		}
	}
	if (0 != chunk_cnt)
	{
	    printf ("\nupg_rsd_write_ext4_image:chunk_cnt error\r\n");
	    return -1;
	}

	//   printf("Write %x bytes\r\n", ptSparseHeader->total_blks * block_size);
	return 0;
}

static int fastboot_erase_partition(char* szPartitionName, unsigned long long u8Addr, unsigned long long u8Size)
{
#if 1
    struct  mmc* emmc_dev;
    int     emmc_dev_num = 0;
    
	emmc_dev = find_mmc_device(emmc_dev_num);
	if (!emmc_dev)
	{
		return -1;
	}
	if (mmc_init(emmc_dev))
	{
		return -1;
	}

	printf("\nBegin to erase partition: %s\n", szPartitionName);

    return mmc_erase(emmc_dev, 1, u8Addr, u8Size);
#endif
    return 0;
}

static BOOL fastboot_usb_tx_status(const char *status)
{
	DWORD nNumberOfBytesWritten = 0;
	int len = strlen(status);
   
   	USBFNWriteData((LPVOID)status,len,&nNumberOfBytesWritten);
   	return TRUE;
}

#define dprintf  
#define cprintf  

static unsigned hex2unsigned(char *x)
{
    unsigned n = 0;

    while(*x) {
        switch(*x) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            n = (n << 4) | (*x - '0');
            break;
        case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f':
            n = (n << 4) | (*x - 'a' + 10);
            break;
        case 'A': case 'B': case 'C':
        case 'D': case 'E': case 'F':
            n = (n << 4) | (*x - 'A' + 10);
            break;
        default:
            return n;
        }
        x++;
    }

    return n;
}

static void num_to_hex8(unsigned n, char *out)
{
    static char tohex[16] = "0123456789abcdef";
    int i;
    for(i = 7; i >= 0; i--) {
        out[i] = tohex[n & 15];
        n >>= 4;
    }
    out[8] = 0;
}

static void fastboot_usb_rx_data(void *pbuff, unsigned length)
{
    DWORD dwNumberOfBytesRead = 0;
	BOOL bRet = USBFNReadData(pbuff, length, &dwNumberOfBytesRead);
	printf("fastboot_usb_rx_data: 0x%x\n", dwNumberOfBytesRead);
	return;
}

/*** memmove - Copy one area of memory to another
* @dest: Where to copy to 
* @src: Where to copy from  
* @count: The size of the area.
* 
* Unlike memcpy(), memmove() copes with overlapping areas.
*/
static void *fb_memmove(void *dest, const void *src, size_t count)
{
	char *tmp;
	const char *s;
	if (dest <= src) 
	{                 
		tmp = (char*)dest;
		s = (char*)src;
		while (count--)
			*tmp++ = *s++;
	}
	else 
	{
		tmp = (char*)dest;
		tmp += count;
		s = (char*)src;
		s += count;
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}

#define MAX_DOWNLOAD_SIZE 0x10000000 // 256M
#define MAX_RECV_SIZE 0x800000 //8M
static char gCurrentPart[32] = {0};
static u32 gCurrentOff = 0;
partitionread *gpart = NULL;

#ifndef CONFIG_BOOT_MMC
void usb_rx_cmd_complete(BYTE *cmdbuf, unsigned actual, int status)
{
    char *argv[CONFIG_SYS_MAXARGS + 1];
    int argc = 0;
    int nRet = 0;
    unsigned int partition_start = 0;
    static char dl_max_str[11];
    partitionread *part = NULL;
    int end = 0;

    if(status != 0) return;

    if(actual > 4095) actual = 4095;
    cmdbuf[actual] = 0;

    //printf("\n> %s\n",cmdbuf);

    part = (partitionread *)malloc(sizeof(partitionread));
    if (!part) {
        printf("malloc partitionread buff failed\n");
        return;
    }

    // Command -- reboot
    if(memcmp(cmdbuf, "reboot", 6) == 0)
    {
        fastboot_usb_tx_status("OKAY");
        printf("fastboot reboot ...\n");
        _reset(0, NULL);
        // Never go here
        g_fgBoot = TRUE;
        return;
    }

    // Command -- getvar
    if(memcmp(cmdbuf, "getvar:", 7) == 0)
    {
        char response[64];
        strcpy(response,"OKAY");

        if(!strcmp(cmdbuf + 7, "version"))
        {
            strcpy(response + 4, VERSION);
        }
        else if(!strcmp(cmdbuf + 7, "product"))
        {
            strcpy(response + 4, PRODUCTNAME);
        }
        else if(!strcmp(cmdbuf + 7, "serialno"))
        {
            strcpy(response + 4, serialno);
        }
        else if (!strcmp(cmdbuf + 7, "max-download-size"))
        {
            sprintf(dl_max_str, "0x%X", MAX_DOWNLOAD_SIZE);
            strcpy(response + 4, dl_max_str);
        }
        else if (!strncmp(cmdbuf + 7, "has-slot:", 9))
        {
            strcpy(gCurrentPart, cmdbuf + 16);
            if (isCurrentPartExt4(gCurrentPart)) {
                printf("fastboot ready to earse ext4 partition\n");
                if (erase_nand(gCurrentPart) != 0) {
                    fastboot_usb_tx_status("FAIL");
                    printf("nand erase system part failed!\n");
                    return;
                }
                gpart = get_part_info_by_name(gCurrentPart);
                if (!gpart) {
                    printf("find partition failed. Ret=%d.\r\n", nRet);
                    fastboot_usb_tx_status("FAIL");
                    return;
                }
            }
            printf("Current download part is %s\n", gCurrentPart);
        }
        else 
        {
            printf("Not support getvar command: %s\n", cmdbuf);
        }
        fastboot_usb_tx_status(response);
        return;
    }

    // Command -- download
    if(memcmp(cmdbuf, "download:", 9) == 0)
    {
        char status[16];
        unsigned int reamin_size = 0, size = 0;
        rx_addr = NAND_WRITE_BASE_ADDR;
        rx_length = hex2unsigned(cmdbuf + 9);
        printf("recv data addr=%x size=%x\n", rx_addr, rx_length);

        if (rx_length > g_u4ImageMaxSize)
        {
            fastboot_usb_tx_status("FAILdata too large");
            return;
        }

        strcpy(status,"DATA");
        num_to_hex8(rx_length, status + 4);
        fastboot_usb_tx_status(status);
        if (isCurrentPartExt4(gCurrentPart)){
            reamin_size = rx_length;
            while (reamin_size > 0) {
                if (reamin_size > MAX_RECV_SIZE) {
                    size = MAX_RECV_SIZE;
                    end = 0;
                } else {
                    size = reamin_size;
                    end = 1;
                    printf("last nand write segement\r\n");
                }
                reamin_size -= size;
                printf("reamin_size:0x%x size:0x%x rx_length:0x%x \n", reamin_size, size, rx_length);

                fastboot_usb_rx_data((void *)NAND_WRITE_BASE_ADDR, size);
                printf("write part:%s nand offset:0x%x\n", gpart->szPartName, gCurrentOff);
                flush_cache((ulong)NAND_WRITE_BASE_ADDR, size);
                if (write_nand_ex((char*)NAND_WRITE_BASE_ADDR, size, gpart->szPartName, gCurrentOff, "ext4", end)) {
                    fastboot_usb_tx_status("FAIL");
                    printf("write nand failed\r\n");
                    return;
                }
                gCurrentOff += size;
            }
        }else {
            fastboot_usb_rx_data((void *)NAND_WRITE_BASE_ADDR, rx_length);
        }

        fastboot_usb_tx_status("OKAY");
        return;
    }

    // Command -- erase
    if(memcmp(cmdbuf, "erase:", 6) == 0)
    {
        /*erase:partition-name*/
        if (*(cmdbuf+6) == 0)
        {
            fastboot_usb_tx_status("FAIL");
            return;
        }

        if (erase_nand(cmdbuf+6) != 0)
        {
            fastboot_usb_tx_status("FAIL");
            return;
        }

        fastboot_usb_tx_status("OKAY");
        g_fgBoot = TRUE;
        return;
    }

    // Command -- flash
    if(memcmp(cmdbuf, "flash:", 6) == 0) {
        cmdbuf += 6;
        /*flash:no partition-name*/
        if (*(cmdbuf) == 0) {
            fastboot_usb_tx_status("FAIL");
            return;
        }

        if (isCurrentPartExt4(gCurrentPart)) {
            fastboot_usb_tx_status("OKAY");
            gCurrentOff = 0;
            printf("fastboot update %s okay\r\n", gCurrentPart);
            return;
        }

        printf("nand write size %d.\r\n", nand->writesize);
        rx_length = ALIGN(rx_length, nand->writesize);

        printf("find partition: %s\r\n", cmdbuf);
        part = get_part_info_by_name(cmdbuf);
        if (!part) {
            printf("find partition failed. Ret=%d.\r\n", nRet);
            fastboot_usb_tx_status("FAIL");
            return;
        }

        // Erase before write.
        if (erase_nand(cmdbuf) != 0) {
            fastboot_usb_tx_status("FAIL");
            return;
        }

        printf("partition name:%s type:%s\r\n", part->szPartName, part->szType);
        if (strcmp(part->szType, "raw") == 0) {
            nRet = fastboot_raw_image_write(part, rx_length);
        } else if (strcmp(part->szType, "ext4") == 0) {
            nRet = fastboot_ext4_image_write(part, rx_length);
        } else {
            printf("partition type error.\r\n");
            fastboot_usb_tx_status("FAIL");
            return;
        }

        if (nRet == 0) {
            fastboot_usb_tx_status("OKAY");
            g_fgBoot = TRUE;
        } else {
            fastboot_usb_tx_status("FAIL");
        }
        return;
    }

    // Command -- oem
    if(memcmp(cmdbuf, "oem", 3) == 0)
    {
	    cmdbuf += 3;
	    if (*(cmdbuf) == 0)
	    {
		    fastboot_usb_tx_status("FAIL");
		    return;
	    }
	    //skip blank.
	    cmdbuf++;
	    if(memcmp(cmdbuf, "blcmd", 5) == 0)
	    {
		    strcpy(&(bl_command_line[bl_cmmand_line_length]),cmdbuf + 5);
		    bl_cmmand_line_length += strlen(cmdbuf + 5);
		    printf("rev comand: %s\r\n",bl_command_line);
		    fastboot_usb_tx_status("OKAY");
		    return;
	    }
	    else if(memcmp(cmdbuf, "run", 3) == 0)
	    {                      
		    bl_command_line[bl_cmmand_line_length] = 0;
		    printf("run comand: %s\r\n",bl_command_line);
		    bl_cmmand_line_length = 0;

		    if (oem_parse(bl_command_line) != 0) 
		    {
			    fastboot_usb_tx_status("FAIL");
			    return;
		    }	
		    fastboot_usb_tx_status("OKAY");
		    return;	
	    }
	    else if(memcmp(cmdbuf, "partinfo", 8) == 0)
	    {
		    printf("rev comand: partinfo\r\n");
		    argc = fb_parse_cmdline((cmdbuf + 8), argv);
#ifdef NEW_PARTITION_DESIGN
		    if (argc != 5)
#else
			    if (argc != 4)
#endif
			    {
				    fastboot_usb_tx_status("FAILparameter number error.");
				    return;
			    }
		    nRet = add_partition_info(argc, argv);
		    if (nRet)
		    {
			    printf("partinfo failed: %d\r\n", nRet);
			    fastboot_usb_tx_status("FAIL");
		    }
		    else
		    {
			    fastboot_usb_tx_status("OKAY");
		    }
		    return;
	    }
	    else if(memcmp(cmdbuf, "dumppart", 8) == 0)
	    {
		    printf("rev comand: dumppart\r\n");
		    dump_partition_info();
		    fastboot_usb_tx_status("OKAY");
		    return;
	    }
	    else if(memcmp(cmdbuf, "setmaximagesize", 15) == 0)
	    {
		    printf("rev comand: setmaximagesize\r\n");
		    char *endptr = NULL;
		    g_u4ImageMaxSize = simple_strtoul(cmdbuf+15, endptr, 16);
		    fastboot_usb_tx_status("OKAY");
		    return;
	    }
	    // ====================   oformat_usrdata ok?  =======================
	    else if(memcmp(cmdbuf, "oformat_usrdata", 15) == 0)
	    {
		    printf("rev comand: oformat\r\n");

		    // skip 'oformat_usrdata '
		    cmdbuf += 16;
		    argc = fb_parse_cmdline(cmdbuf, argv);

		    partition_start = simple_strtoul(argv[0], NULL, 16);

		    fb_format_usrdata_partition("usrdata", partition_start);

		    fastboot_usb_tx_status("OKAY");
		    return;
	    }
	    // ====================   oflash command ok?  =======================
	    else if(memcmp(cmdbuf, "oflash", 6) == 0)
	    {
		    printf("rev comand: %s\r\n", cmdbuf);

		    //skip 'oflash '
		    cmdbuf += 7;
		    // --------  handle 'oflash start part_name part_type segment_size(NNNNNNNN .HEX) partition_offset(NNNNNNNN .HEX)'  ----------
		    if(memcmp(cmdbuf, "start", 5) == 0)
		    {
			    cmdbuf += 6;
			    argc = fb_parse_cmdline(cmdbuf, argv);
			    // parse partition name
			    if (argc == 0)
			    {
				    fastboot_usb_tx_status("FAILoflash start need partition name.");
				    return;
			    }
			    strcpy(current_write_part_name, argv[0]);
			    strcpy(current_write_part_type, argv[1]);
			    printf("---> write partition name: %s <---\r\n", current_write_part_name);
			    printf("---> write partition type: %s <---\r\n", current_write_part_type);
			    if (argc > 2)
			    {
				    g_write_segment_size = simple_strtoul(argv[2], NULL, 16);
				    printf("---> write segment size: %d <---\r\n", g_write_segment_size);
			    }
			    current_write_part_offset = 0;
			    if (argc > 3)
			    {
				    current_write_part_offset = simple_strtoul(argv[3], NULL, 16);
			    }

			    if (current_write_part_offset == 0)
			    {
                                    part = get_part_info_by_name(current_write_part_name);
				    current_write_part_offset = part->u8PartitionStartAddr;
			    }
			    printf("---> write partition offset: 0x%X <---\r\n", current_write_part_offset);

			    // Erase before write.
			    if (erase_nand(current_write_part_name) != 0)
			    {
				    fastboot_usb_tx_status("FAILNand ersse failed..");
				    return;
			    }
#ifdef NEW_PARTITION_DESIGN
			    u32 flag;
			    getpartitionbypartitionname(&flag, current_write_part_name);
			    if(bUpgradePartinfo != 1 && isEnable(FASTBOOT_ENABLE, flag) == DISABLE)
			    {
				    printf("Partition[%s] donot be allowed fastboot upgraded!!!\n", current_write_part_name);
				    bFastbootUpgrade = 0;
				    char szShowString[60] = {0};
				    strcat(szShowString,"   ");
				    strcat(szShowString,current_write_part_name);
#if CONFIG_SUPPORT_CHAR == 1
				    strcat(szShowString,"��������������!!!\n");
#else
				    strcat(szShowString," donot be allowed fastboot upgraded!!!\n");
#endif
				    LCD_WriteString_FixedCharNum80(szShowString);
			    }
			    else
			    {
				    printf("Partition[%s] will be fastboot upgraded\n", current_write_part_name);
			    }
#endif
			    // Init global values
			    g_dwTotalRead = 0;
			    part_write_offset = 0;
		    }

		    // --------  handle 'oflash segment NNNN part_name'  (HEX)--------
		    else if (memcmp(cmdbuf, "segment", 7) == 0)
		    {
#ifdef NEW_PARTITION_DESIGN
			    if(bFastbootUpgrade == 0 )
			    {
				    fastboot_usb_tx_status("OKAY");
				    return;
			    }
#endif
			    unsigned snum = simple_strtoul((cmdbuf + 8), NULL, 16);
			    //printf("---> segment sum: %d <---\r\n", snum);

			    if (g_dwTotalRead > g_write_segment_size)
			    {
				    // write to storage
				    fb_write_internal_storage(current_write_part_name,
						    current_write_part_type,
						    part_write_offset,
						    BASE_ADDR,
						    g_write_segment_size,
						    0);
				    part_write_offset += g_write_segment_size;

				    g_dwTotalRead -= g_write_segment_size;
				    fb_memmove(BASE_ADDR, (BASE_ADDR + g_write_segment_size), g_dwTotalRead);
			    }
		    }

		    // -------  handle 'oflash skip NNNNNNNN', (HEX)  -------
		    else if (memcmp(cmdbuf, "skip", 4) == 0)
		    {
#ifdef NEW_PARTITION_DESIGN
			    if(bFastbootUpgrade == 0 )
			    {
				    fastboot_usb_tx_status("OKAY");
				    return;
			    }
#endif
			    unsigned skip_sz = simple_strtoul((cmdbuf + 5), NULL, 16);
			    printf("---> skip size: ox%08X <---\r\n", skip_sz);

			    if ((g_dwTotalRead + skip_sz) > g_write_segment_size)
			    {
				    // First segment
				    memset((BASE_ADDR + g_dwTotalRead), 0x00, (g_write_segment_size - g_dwTotalRead));
				    // write to storage
				    fb_write_internal_storage(current_write_part_name,
						    current_write_part_type,
						    part_write_offset,
						    BASE_ADDR,
						    g_write_segment_size,
						    0);
				    part_write_offset += g_write_segment_size;

				    skip_sz -= (g_write_segment_size - g_dwTotalRead);
				    g_dwTotalRead = 0;
				    while (skip_sz >= g_write_segment_size)
				    {
					    //memset(BASE_ADDR, 0x00, g_write_segment_size);
					    // write fake data to storage
					    //fb_write_internal_storage(current_write_part_name, 
					    //						  current_write_part_type, 
					    //						  part_write_offset, 
					    //						  BASE_ADDR, 
					    //						  g_write_segment_size,
					    //						  0);
					    part_write_offset += g_write_segment_size;
					    skip_sz -= g_write_segment_size;
				    }

				    // the rest data
				    memset(BASE_ADDR, 0x00, skip_sz);
				    g_dwTotalRead = skip_sz;				
			    }
			    else
			    {
				    memset((BASE_ADDR + g_dwTotalRead), 0x00, skip_sz);
				    g_dwTotalRead += skip_sz;
			    }
		    }
		    // -------  handle 'oflash end' -------
		    else if (memcmp(cmdbuf, "end", 3) == 0)
		    {
#ifdef NEW_PARTITION_DESIGN
			    if(bFastbootUpgrade == 0 )
			    {
				    bFastbootUpgrade = 1;
				    fastboot_usb_tx_status("OKAY");
				    return;
			    }
#endif
			    // flush the rest last segment data
			    UINT32 u4WriteSize = 0;
			    UINT32 u4WriteBufPtr = BASE_ADDR;
			    // handle 'boot' partition
			    if (0 == strcmp(current_write_part_name, "boot"))
			    {
				    fastboot_create_bootloader_header((char *)(BASE_ADDR - 512), BASE_ADDR, 0x7000);
				    g_dwTotalRead += 512;
				    u4WriteBufPtr = BASE_ADDR - 512;
			    }
			    // write to storage
			    u4WriteSize = ALIGN(g_dwTotalRead, nand->writesize);
			    fb_write_internal_storage(current_write_part_name,
					    current_write_part_type,
					    part_write_offset,
					    u4WriteBufPtr,
					    u4WriteSize,
					    1);
			    g_dwTotalRead = 0;
			    part_write_offset = 0;
			    memset(current_write_part_name, 0x00, 32);
		    }

		    fastboot_usb_tx_status("OKAY");
		    return;
	    }
	    // ====================   oflash command   =======================
	    else
	    {
		    fastboot_usb_tx_status("FAILunkonw oem command");
		    return;
	    }

    }

    fastboot_usb_tx_status("FAILunknow command"); // if get this please check command response 'return'
}

#else // =====>>>  Below code is for eMMC  <<<=====

void usb_rx_cmd_complete(BYTE *cmdbuf, unsigned actual, int status)
{
	char *argv[CONFIG_SYS_MAXARGS + 1];
	int argc = 0;
	int nRet = 0;
	unsigned long long partition_start = 0;
	static char dl_max_str[11];
	LP_FB_PART_INFO prPartInfo = NULL;
	partitioninfo *ppart = NULL;

#ifdef READ_IMAGE_USE_REAL_SIZE
	extern partitionhead *g_partitionhead;
	extern partitionread *g_partitionread;
	extern unsigned long long g_u8RealDataSize;
#endif
	if(status != 0) return;

	if(actual > 4095) actual = 4095;
	cmdbuf[actual] = 0;

	//printf("\n> %s\n",cmdbuf);

	// Command -- reboot
	if(memcmp(cmdbuf, "reboot", 6) == 0) {
		printf("rev comand: reboot\r\n");
		fastboot_usb_tx_status("OKAY");
		//do_reset(NULL, 0, 0, NULL);
		_reset(0, NULL);

		// Never go here
		g_fgBoot = TRUE;
		return;
	}

	// Command -- getvar
	if(memcmp(cmdbuf, "getvar:", 7) == 0) {
		char response[64];
		strcpy(response,"OKAY");

		if(!strcmp(cmdbuf + 7, "version")) {
			strcpy(response + 4, VERSION);
		} else if(!strcmp(cmdbuf + 7, "product")) {
			strcpy(response + 4, PRODUCTNAME);
		} else if(!strcmp(cmdbuf + 7, "serialno")) {
			strcpy(response + 4, serialno);
		} else if (!strcmp(cmdbuf + 7, "max-download-size")) {
			sprintf(dl_max_str, "0x%X", MAX_DOWNLOAD_SIZE);
			strcpy(response + 4, dl_max_str);
		} else if (!strncmp(cmdbuf + 7, "has-slot:", 9)) {
			strcpy(gCurrentPart, cmdbuf + 16);
			if (isCurrentPartExt4(gCurrentPart)) {
				gpart = get_part_info_by_name(gCurrentPart);
				if (!gpart) {
					printf("find partition failed. Ret=%d.\r\n", nRet);
					fastboot_usb_tx_status("FAIL");
					return;
				}
			}
			printf("Current download part is %s\n", gCurrentPart);
		} else {
			printf("Not support getvar command: %s\n", cmdbuf);
		}
		fastboot_usb_tx_status(response);
		return;
	}

	// Command -- download
	if(memcmp(cmdbuf, "download:", 9) == 0) {
		char status[16];
		unsigned int reamin_size = 0, size = 0;
		rx_addr = BASE_ADDR;
		rx_length = hex2unsigned(cmdbuf + 9);
		printf("recv data addr=%x size=%x\n", rx_addr, rx_length);

		if (rx_length > g_u4ImageMaxSize) {
			fastboot_usb_tx_status("FAILdata too large");
			return;
		}

		strcpy(status,"DATA");
		num_to_hex8(rx_length, status + 4);
		fastboot_usb_tx_status(status);
		if (isCurrentPartExt4(gCurrentPart)){
			gpart = get_part_info_by_name(gCurrentPart);
			if (!gpart) {
				fastboot_usb_tx_status("FAILpartition not found");
				printf("partition not found: %s\n", gCurrentPart);
				return;
			}

			reamin_size = rx_length;
			while (reamin_size > 0) {
				unsigned int copy_size;
				if (reamin_size > MAX_RECV_SIZE) {
					copy_size = MAX_RECV_SIZE;
				} else {
					copy_size = reamin_size;
					printf("last emmc write segement\r\n");
				}
				reamin_size -= copy_size;
				printf("reamin_size:0x%x size:0x%x rx_length:0x%x \n", reamin_size, copy_size, rx_length);

				fastboot_usb_rx_data((void *)BASE_ADDR, copy_size);
				printf("write part:%s partaddr:0x%x offset:0x%x\n", gpart->szPartName, gpart->u8PartitionStartAddr, gCurrentOff);

				flush_cache((ulong)BASE_ADDR , size);
				if (fb_write_emmc_storage(gpart->u8PartitionStartAddr + gCurrentOff, BASE_ADDR, copy_size)!= 0) {
					printf("write partition[%s] failed\r\n", gpart->szPartName);
					fastboot_usb_tx_status("FAILwrite emmc storage failed");
					return;
				}
				gCurrentOff += copy_size;
			}
		} else {
			fastboot_usb_rx_data((void *)BASE_ADDR, rx_length);
		}

		fastboot_usb_tx_status("OKAY");
#ifdef READ_IMAGE_USE_REAL_SIZE
		g_u8RealDataSize += rx_length;
#endif
		return;
	}

	// Command -- erase
	if(memcmp(cmdbuf, "erase:", 6) == 0) {
		partitionread *ppartitionread,*p;

		cmdbuf += 6;
		argc = fb_parse_cmdline(cmdbuf, argv);
		printf("flash argc:%d, argv 1: %s, argv 2: %s\n", argc, argv[0], argv[1]);
		ppartitionread = readpartitioninfofromflash();
		p = ppartitionread;

		while (p)
		{
			if (strcmp(p->szPartName, argv[0]) == 0)
			{
#ifdef NEW_PARTITION_DESIGN
				if(bUpgradePartinfo != 1 && isEnable(ERASE_ENABLE, p->u4Flag) == DISABLE)
				{
					printf("Partition[%s] do not allowed to be erased\n", p->szPartName);
				}
				else
				{
					printf("Partition[%s] will be erased\n", p->szPartName);
					fastboot_erase_partition(p->szPartName, p->u8PartitionStartAddr, p->u8PartitionSize);
				}
#else
				fastboot_erase_partition(p->szPartName, p->u8PartitionStartAddr, p->u8PartitionSize);
#endif
				break;
			}
			p = p->nextpartition;
		}

		fastboot_usb_tx_status("OKAY");
		return;
	}

	// Command -- flash
	if(memcmp(cmdbuf, "flash:", 6) == 0) {
		partitionread *ppartitionread,*p;

		cmdbuf += 6;
		argc = fb_parse_cmdline(cmdbuf, argv);
		printf("flash argc:%d, argv 1: %s, argv 2: %s\n", argc, argv[0], argv[1]);

		if (isCurrentPartExt4(gCurrentPart)) {
			fastboot_usb_tx_status("OKAY");
			gCurrentOff = 0;
			printf("fastboot update %s okay\r\n", gCurrentPart);
			return;
		}
#ifdef READ_IMAGE_USE_REAL_SIZE
		int error = readpartitioninfofromflash_ext();
		if (error == -1) {
			char szShowString[60]={0};
			printf("flash: get partition info fail\r\n");
			memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
			strcat(szShowString,"	...���ܴ�emmc�л�ȡ������Ϣ!!!");
#else
			strcat(szShowString,"   ...can't get partition info from emmc!!!");
#endif
			strcat(szShowString,"...\n");
			//LCD_CleanScreen();
			//LCD_WriteString(szShowString);
			LCD_WriteString_FixedCharNum80(szShowString);
			LCD_FreeStringBuf();
			return -1;
		}
		p = g_partitionread;
#else //use partition size for image size
		printf("update image use partition size\r\n");
		ppartitionread = readpartitioninfofromflash();
		p = ppartitionread;
#endif
		while (p)
		{
		#ifdef READ_IMAGE_USE_REAL_SIZE
			g_u8RealDataSize = 0;
		#endif

			if (strcmp(p->szPartName, argv[0]) == 0) {
#ifdef NEW_PARTITION_DESIGN
				if(bUpgradePartinfo != 1 && isEnable(FASTBOOT_ENABLE, p->u4Flag) == DISABLE) {
					printf("Partition[%s] do not allowed to be fastboot upgraded\n", p->szPartName);
					break;
				} else {
					printf("Partition[%s] will be fastboot upgraded\n", p->szPartName);
				}
#endif
				if (0 == strcmp(p->szType,"raw")) {
					printf("Writing image: %s\n", p->szPartName);
					fb_write_internal_storage(p->szPartName,
								p->szType,
								p->u8PartitionStartAddr,
								BASE_ADDR,
								rx_length, //p->u8PartitionSize,
								1);
				#ifdef READ_IMAGE_USE_REAL_SIZE
					if (strcmp(p->szPartName, "preloader") == 0)
						g_u8RealDataSize = rx_length + 512;  //add preloader header:512
					else
						g_u8RealDataSize = rx_length;

					g_u8RealDataSize = (g_u8RealDataSize % 512) == 0 ? g_u8RealDataSize : (g_u8RealDataSize + 512 - g_u8RealDataSize%512);
					p->u8RealDataSize = g_u8RealDataSize;
					printf("%s partition image real size: 0x%x", p->szPartName, p->u8RealDataSize);
					writepartitioninfotoflash(g_partitionhead);
				#endif
				}

				if (0 == strcmp(p->szType,"ext4")) {
					printf("Writing image: %s, length: 0x%x\n", p->szPartName, rx_length);
				#if 0
					fb_write_internal_storage(p->szPartName,
										       p->szType,
										       p->u8PartitionStartAddr,
											BASE_ADDR,
											rx_length, //p->u8PartitionSize,
											1);
				#endif
					fastboot_parse_write_ext4_image(BASE_ADDR, p->u8PartitionStartAddr, p->u8PartitionSize);//rx_length);
					//here we use compressed ext4 image,so rx_length is not real ext4 size
				}
			}
			p = p->nextpartition;
		}

		fastboot_usb_tx_status("OKAY");
		return;
	}

	// Command -- oem
	if(memcmp(cmdbuf, "oem", 3) == 0)
	{
		cmdbuf += 3;
		if (*(cmdbuf) == 0)
		{
			fastboot_usb_tx_status("FAIL");
			return;
		}
		//skip blank.
		cmdbuf++;
		if(memcmp(cmdbuf, "blcmd", 5) == 0)
		{
			strcpy(&(bl_command_line[bl_cmmand_line_length]),cmdbuf + 5);
			bl_cmmand_line_length += strlen(cmdbuf + 5);
			printf("rev comand: %s\r\n",bl_command_line);
			fastboot_usb_tx_status("OKAY");
			return;
		}
		else if(memcmp(cmdbuf, "run", 3) == 0)
		{
			bl_command_line[bl_cmmand_line_length] = 0;
			printf("run comand: %s\r\n",bl_command_line);
			bl_cmmand_line_length = 0;

			if (oem_parse(bl_command_line) != 0)
			{
				fastboot_usb_tx_status("FAIL");
				return;
			}
			fastboot_usb_tx_status("OKAY");
			return;
		}
		else if(memcmp(cmdbuf, "updatepartstart", 15) == 0)
		{
			printf("rev comand: updatepartinfo start\r\n");

			bUpgradePartinfo = 1;
			parthead.nextpartition = NULL;
			fastboot_usb_tx_status("OKAY");
			return;
		}
		else if(memcmp(cmdbuf, "updatepartend", 14) == 0)
		{
			printf("rev comand: updatepartinfo end\r\n");

			bUpgradePartinfo = 0;
			parthead.blockcnt = (partcnt * sizeof(partitionread)) % 512 ? ((partcnt * sizeof(partitionread) + 512)/512) :(partcnt * sizeof(partitionread))/512;
			printf("write partinfo into flash ,need %d blocks\r\n",parthead.blockcnt);
			writepartitioninfotoflash(&parthead);
			memset(&parthead,0,sizeof(partitionhead));
			partcnt = 0;
			pCurpart = NULL;
			memset(&g_pPartInfoHeader,0,sizeof(LP_FB_PART_INFO));

			fastboot_usb_tx_status("OKAY");
			return;
		}
		else if(memcmp(cmdbuf, "partinfo", 8) == 0)
		{
			printf("rev comand: partinfo\r\n");
			//LCD_CleanScreen();
			//LCD_MallocStringBuf();
			argc = fb_parse_cmdline((cmdbuf + 8), argv);
#ifdef NEW_PARTITION_DESIGN
			if (argc != 5)
#else
				if (argc != 4)
#endif
				{
					fastboot_usb_tx_status("FAILparameter number error.");
					printf("FAILparameter number error. %d\n", argc);
					return;
				}
			ppart = add_partition_info(argc, argv);
			if (ppart == NULL)
			{
				printf("partinfo failed: %d\r\n", nRet);
				fastboot_usb_tx_status("FAIL");
			}
			else
			{
				printf("partinfo success\r\n");
				printf("update all partition: %d\r\n", bUpgradePartinfo);

				if (bUpgradePartinfo == 1)
				{
					if (pCurpart != NULL)
						pPrepart = pCurpart;
					printf("partinfo merge partition info start\r\n");
					pCurpart = mergepartitioninfo(ppart);
					printf("partinfo merge partition info end\r\n");

					if (pCurpart == NULL)
					{
						printf("do_sdagent write new part fail\r\n");
						return -1;
					}
					if(parthead.nextpartition == NULL)
					{
						parthead.nextpartition = pCurpart;
					}
					else
					{
						pPrepart->nextpartition = pCurpart;
						pPrepart->u4LastPartition = 0;
					}
					pCurpart->nextpartition = NULL;
					pCurpart->u4LastPartition = 1;
					partcnt++;
					//free(ppart); //may cause uboot crash when do all partition update use fastboot
				}
				printf("partinfo end 1\r\n");
				fastboot_usb_tx_status("OKAY");
			}
			printf("partinfo end 2\r\n");
			return;
		}
		else if(memcmp(cmdbuf, "dumppart", 8) == 0)
		{
			printf("rev comand: dumppart\r\n");
			dump_partition_info();
			fastboot_usb_tx_status("OKAY");
			return;
		}
		else if(memcmp(cmdbuf, "setmaximagesize", 15) == 0)
		{
			printf("rev comand: setmaximagesize\r\n");
			char *endptr = NULL;
			g_u4ImageMaxSize = simple_strtoul(cmdbuf+15, endptr, 16);
			fastboot_usb_tx_status("OKAY");
			return;	
		}
		// ====================   oformat_usrdata   =======================
		else if(memcmp(cmdbuf, "oformat_usrdata", 15) == 0)
		{
			printf("rev comand: oformat\r\n");

			// skip 'oformat_usrdata '
			cmdbuf += 16;
			argc = fb_parse_cmdline(cmdbuf, argv);

			//partition_start = simple_strtoul(argv[0], NULL, 16);
			simple_strtou64(argv[0], (char **)NULL, 16,&partition_start);

			fb_format_usrdata_partition("usrdata", partition_start);

			fastboot_usb_tx_status("OKAY");
			return;
		}
		else if(memcmp(cmdbuf, "format", 6) == 0)
		{
			cmdbuf += 7;
			argc = fb_parse_cmdline(cmdbuf, argv);
			// parse partition name
			if (argc == 0)
			{
				fastboot_usb_tx_status("FAILoflash start need partition name.");
				return;
			}
			strcpy(current_write_part_name, argv[0]);
			strcpy(current_write_part_type, argv[1]);
			printf("---> write partition name: %s <---\r\n", current_write_part_name);
			printf("---> write partition type: %s <---\r\n", current_write_part_type);

			current_write_part_offset = 0;
			if (argc > 3)
			{
				current_write_part_offset = simple_strtoul(argv[3], NULL, 16);
			}

			if (current_write_part_offset == 0)
			{
				nRet = find_partition_info(current_write_part_name, &prPartInfo);
				current_write_part_offset = prPartInfo->u8PartStartAddr;
			}
			printf("---> write partition offset: 0x%X <---\r\n", current_write_part_offset);


			//printf("---> usb_rx_cmd partition_size: 0x%08X, 0x%08X <---\n", (u32)(prPartInfo->u8PartSize >> 32), (u32)(prPartInfo->u8PartSize & 0xFFFFFFFF));

			fb_format_partition(prPartInfo->szPartName,prPartInfo->u8PartStartAddr,prPartInfo->u8PartSize);
			fastboot_usb_tx_status("OKAY");
			return;

		}
		// ====================   oflash command   =======================
		else if(memcmp(cmdbuf, "oflash", 6) == 0)
		{
			printf("rev comand: %s\r\n", cmdbuf);

			//skip 'oflash '
			cmdbuf += 7;
			// --------  handle 'oflash start part_name part_type segment_size(NNNNNNNN .HEX) partition_offset(NNNNNNNN .HEX)'  ----------
			if(memcmp(cmdbuf, "start", 5) == 0)
			{
				cmdbuf += 6;
				argc = fb_parse_cmdline(cmdbuf, argv);
				printf("args num: %d\r\n", argc);
				// parse partition name
				if (argc == 0)
				{
					fastboot_usb_tx_status("FAILoflash start need partition name.");
					return;
				}

#ifdef READ_IMAGE_USE_REAL_SIZE

				printf("update all partition: %d\r\n", bUpgradePartinfo);
				if (bUpgradePartinfo == 1)  //update all partition, use partition info from aut
				{
					g_partitionhead = &parthead;
					g_partitionread = parthead.nextpartition;
				}
				else
				{
					int error = readpartitioninfofromflash_ext();
					if (error == -1)
					{
						char szShowString[60]={0};
						printf("get partition info fail\r\n");
						memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
						strcat(szShowString,"	...���ܴ�emmc�л�ȡ������Ϣ!!!");
#else
						strcat(szShowString,"   ...can't get partition info from emmc!!!");
#endif
						strcat(szShowString,"...\n");
						//LCD_CleanScreen();
						//LCD_WriteString(szShowString);
						LCD_WriteString_FixedCharNum80(szShowString);
						LCD_FreeStringBuf();
						return -1;
					}
				}

#endif
				strcpy(current_write_part_name, argv[0]);
				strcpy(current_write_part_type, argv[1]);
				printf("---> write partition name: %s <---\r\n", current_write_part_name);
				printf("---> write partition type: %s <---\r\n", current_write_part_type);
				if (argc > 2)
				{
					g_write_segment_size = simple_strtoul(argv[2], NULL, 16);
					printf("---> write segment size: 0x%08X <---\r\n", g_write_segment_size);
				}
				current_write_part_offset = 0;
				if (argc > 3)
				{
					current_write_part_offset = simple_strtoul(argv[3], NULL, 16);
				}

				if (current_write_part_offset == 0)
				{
					nRet = find_partition_info(current_write_part_name, &prPartInfo);
					if (!nRet)
					{
						//current_write_part_offset = prPartInfo->u4PartStartAddr;
						//printf("find match partition\r\n");
						current_write_part_offset = prPartInfo->u8PartStartAddr;
					}
					else
						printf("can not find match partition\r\n");
				}
				printf("---> write partition offset: 0x%X <---\r\n", current_write_part_offset);
				char szShowString[60]={0};
#if CONFIG_SUPPORT_CHAR == 1
				strcat(szShowString,"   ������������д�뾵��: ");
#else
				strcat(szShowString,"   writing image to flash: ");
#endif
				strcat(szShowString,current_write_part_name);
				strcat(szShowString,"...\n");
				//LCD_CleanScreen_part();
				//LCD_WriteString(szShowString);
				LCD_WriteString_FixedCharNum80(szShowString);
				// Init global values
				g_dwTotalRead = 0;
				part_write_offset = 0;

#ifdef NEW_PARTITION_DESIGN
				getpartitionbypartitionname(&current_part_flag, current_write_part_name);
				if(bUpgradePartinfo != 1 && isEnable(FASTBOOT_ENABLE, current_part_flag) == DISABLE)
				{
					printf("Partition[%s] donot be allowed fastboot upgraded!!!\n", current_write_part_name);
					bFastbootUpgrade = 0;
					memset(szShowString,0,60);
					strcat(szShowString,"   ");
					strcat(szShowString,current_write_part_name);
#if CONFIG_SUPPORT_CHAR == 1
					strcat(szShowString,"��������������!!!\n");
#else
					strcat(szShowString," donot be allowed fastboot upgraded!!!\n");
#endif
					LCD_WriteString_FixedCharNum80(szShowString);
				}
				else
				{
					printf("Partition[%s] will be fastboot upgraded\n", current_write_part_name);
				}
#endif
#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
				if(bFastbootUpgrade) {
					//printf("[WP] part:%s, flags:0x%x\n", current_write_part_name, current_part_flag);
					curr_wp_offset = prPartInfo->u8PartStartAddr/512;
					curr_wp_size = prPartInfo->u8PartSize/512;
					printf("[WP] clear wp on part(%s: 0x%x, 0x%x)\n", current_write_part_name, curr_wp_offset, curr_wp_size);
					if(emmc_set_user_wp(WP_DISABLE, curr_wp_offset, curr_wp_size, 1)) {
						printf("[WP] clear wp part(%s: 0x%x ~ 0x%x) fail, cancel upgrade\n", current_write_part_name,
								curr_wp_offset, curr_wp_size);

						bFastbootUpgrade = 0;
					}
				}
#endif


#ifdef READ_IMAGE_USE_REAL_SIZE
				g_u8RealDataSize = 0;  //store image real size
#endif
			}

			// --------  handle 'oflash segment NNNN part_name'  (HEX)--------
			else if (memcmp(cmdbuf, "segment", 7) == 0)
			{
#ifdef NEW_PARTITION_DESIGN
				if(bFastbootUpgrade == 0 )
				{
					fastboot_usb_tx_status("OKAY");
					return;
				}
#endif
				unsigned snum = simple_strtoul((cmdbuf + 8), NULL, 16);
				//printf("---> segment sum: %d <---\r\n", snum);

				if (g_dwTotalRead > g_write_segment_size)
				{
					// write to storage
					fb_write_internal_storage(current_write_part_name,
							current_write_part_type,
							(current_write_part_offset + part_write_offset),
							BASE_ADDR,
							g_write_segment_size,
							1);
					part_write_offset += g_write_segment_size;

					g_dwTotalRead -= g_write_segment_size;
					fb_memmove(BASE_ADDR, (BASE_ADDR + g_write_segment_size), g_dwTotalRead);
				}
			}

			// -------  handle 'oflash skip NNNNNNNN', (HEX)  -------
			else if (memcmp(cmdbuf, "skip", 4) == 0)
			{
#ifdef NEW_PARTITION_DESIGN
				if(bFastbootUpgrade == 0 )
				{
					fastboot_usb_tx_status("OKAY");
					return;
				}
#endif
				unsigned skip_sz = simple_strtoul((cmdbuf + 5), NULL, 16);
				printf("---> skip size: ox%08X <---\r\n", skip_sz);

				if ((g_dwTotalRead + skip_sz) > g_write_segment_size)
				{
					// First segment
					memset((BASE_ADDR + g_dwTotalRead), 0x00, (g_write_segment_size - g_dwTotalRead));
					// write to storage
					fb_write_internal_storage(current_write_part_name,
							current_write_part_type,
							(current_write_part_offset + part_write_offset),
							BASE_ADDR,
							g_write_segment_size,
							1);
					part_write_offset += g_write_segment_size;

					skip_sz -= (g_write_segment_size - g_dwTotalRead);
					g_dwTotalRead = 0;
					while (skip_sz >= g_write_segment_size)
					{
						part_write_offset += g_write_segment_size;
						skip_sz -= g_write_segment_size;
					}

					// the rest data
					memset(BASE_ADDR, 0x00, skip_sz);
					g_dwTotalRead = skip_sz;				
				}
				else
				{
					memset((BASE_ADDR + g_dwTotalRead), 0x00, skip_sz);
					g_dwTotalRead += skip_sz;
				}
			}
			// -------  handle 'oflash end' -------
			else if (memcmp(cmdbuf, "end", 3) == 0)
			{
				// flush the rest last segment data
				UINT32 u4WriteSize = 0;
				UINT32 u4WriteBufPtr = BASE_ADDR;
#ifdef NEW_PARTITION_DESIGN
				if(bFastbootUpgrade == 0 )
				{
					bFastbootUpgrade = 1;
					fastboot_usb_tx_status("OKAY");
					return;
				}
#endif
				// handle 'boot' partition
				//if (0 == strcmp(current_write_part_name, "boot"))
				if (0 == strcmp(current_write_part_name, "preloader"))
				{
					//fastboot_create_bootloader_header((char *)(BASE_ADDR - 512), BASE_ADDR, g_dwTotalRead);
#if 1
					struct mmc *emmc_dev = NULL;
					int emmc_dev_num = 0;
					int err = 0;


					emmc_dev = find_mmc_device(emmc_dev_num);
					if ( NULL == emmc_dev){
						printf("can't find emmc device\n");
						return;
					}
					err = mmc_init(emmc_dev);
					if(err){
						printf("init emmc device fail\n");
						return;
					}

					create_bootloader_header((char * )(BASE_ADDR -512), BASE_ADDR,PRELOADER_SIZE, 1);/*msdc boot*/

					// Write to boot partition
					//writeblknum = 129; //PRELOADER_MAX_SIZE(include dramk) + header size = 64KB   + 512Bytes = 129 Blocks
					emmc_write_bootpart(emmc_dev,(char *)(BASE_ADDR - 512), 129);
#endif
					g_dwTotalRead += 512;
#ifdef READ_IMAGE_USE_REAL_SIZE
					g_u8RealDataSize += 512;
#endif
					u4WriteBufPtr = BASE_ADDR - 512;
				}
				// write to storage
				u4WriteSize = ALIGN(g_dwTotalRead, 512);
				fb_write_internal_storage(current_write_part_name,
						current_write_part_type,
						(current_write_part_offset + part_write_offset),
						u4WriteBufPtr,
						u4WriteSize,
						1);
#ifdef READ_IMAGE_USE_REAL_SIZE
				if (strcmp(current_write_part_type, "raw") == 0)
				{
					g_u8RealDataSize = (g_u8RealDataSize % 512) == 0 ? g_u8RealDataSize : (g_u8RealDataSize + 512 - g_u8RealDataSize%512); //512Byte align
					extern int update_image_size_in_partition_table_by_partition_name(char *part_name, unsigned long long u8RealDataSize);
					update_image_size_in_partition_table_by_partition_name(current_write_part_name,g_u8RealDataSize);
					if (bUpgradePartinfo == 0) //partitial upgrade, need write partition info for each raw image
					{
						writepartitioninfotoflash(g_partitionhead);
						printf("%s partition image real data size: 0x%x\r\n", current_write_part_name, g_u8RealDataSize);
					}
					else  //update all image
					{
						printf("full upgrade,we will write partition info back after downloading all image\r\n");
					}
				}
				g_u8RealDataSize = 0;
#endif

#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
				if(bFastbootUpgrade && isEnable(WRITE_PROTECT_ENABLE, current_part_flag)) {
					printf("[WP] set wp on part(%s: 0x%x, 0x%x)\n", current_write_part_name, curr_wp_offset, curr_wp_size);
					if(emmc_set_user_wp(WP_ENABLE, curr_wp_offset, curr_wp_size, 1)) {
						printf("[WP] set wp %s(0x%x ~ 0x%x) fail, cancel upgrade\n", current_write_part_name,
								curr_wp_offset, curr_wp_size);

						bFastbootUpgrade = 0;
					}
				}
#endif
				g_dwTotalRead = 0;
				part_write_offset = 0;
				memset(current_write_part_name, 0x00, 32);
			}
			/*
			   else if(memcmp(cmdbuf, "format", 6) == 0)
			   {
			   cmdbuf += 7;
			   argc = fb_parse_cmdline(cmdbuf, argv);
			// parse partition name
			if (argc == 0)
			{
			fastboot_usb_tx_status("FAILoflash start need partition name.");
			return;
			}
			strcpy(current_write_part_name, argv[0]);
			strcpy(current_write_part_type, argv[1]);
			printf("---> write partition name: %s <---\r\n", current_write_part_name);
			printf("---> write partition type: %s <---\r\n", current_write_part_type);

			current_write_part_offset = 0;
			if (argc > 3)
			{
			current_write_part_offset = simple_strtoul(argv[3], NULL, 16);
			}

			if (current_write_part_offset == 0)
			{					
			nRet = find_partition_info(current_write_part_name, &prPartInfo);
			current_write_part_offset = prPartInfo->u4PartStartAddr;
			}
			printf("---> write partition offset: 0x%X <---\r\n", current_write_part_offset);

			fb_format_partition(prPartInfo->szPartName,prPartInfo->u4PartStartAddr,prPartInfo->u4PartSize);

			}
			 */
			fastboot_usb_tx_status("OKAY");
			return;
		}
		// ====================   oflash command   =======================
		else
		{
			fastboot_usb_tx_status("FAILunkonw oem command");
			return;
		}

	}

	fastboot_usb_tx_status("FAILunknow command"); // if get this please check command response 'return'
}

#endif

void FastBoot()
{
	BYTE  cmd_buf[64];
	DWORD nNumberOfBytesRead;
	USBInit();
	SetUpgradeMode(1);
	LCD_CleanScreen();
	LCD_MallocStringBuf();
	char szShowString[60] = {0};

#ifdef CONFIG_BOOT_MMC
	if (emmc_clear_all_wp()) {
		printf("[WP] clear all wp fail, cancel upgrade\n");
		return -1;
	} else {
		printf("[WP] clear all wp success\n");
	}
#endif

	strcat(szShowString, "Enter Fastboot Mode\n");
	LCD_WriteString_FixedCharNum80(szShowString);

	while(1)
	{
	   USBFNReadData(cmd_buf, 64, &nNumberOfBytesRead);
	   usb_rx_cmd_complete(cmd_buf, nNumberOfBytesRead, 0);
	}

}

