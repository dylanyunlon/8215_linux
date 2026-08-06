#include <common.h>
#include <asm/io.h>
#include <asm/arch/ac83xx_basic.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <i2c.h>
#include <asm/mach-types.h>

#include <asm/arch/x_typedef.h>
#include <asm/arch/x_bim.h>
//#include <usb.h>
#include <ac83xx_gpio.h>

#include <asm/arch/ac83xx_part_tbl.h>
#include <asm/arch/ac83xx_upg_status.h>
#include <stdio_dev.h>

#include <asm/arch/args_to_uboot.h>
#include <asm/arch/args_to_arm2.h>
#include <upg_config.h>

#include <common.h>
#include <command.h>
#include <s_record.h>
#include <net.h>
#include <ata.h>
#include <part.h>
#include <fat.h>

#include <command.h>
#include <mmc.h>
#include <metazone_inter.h>

/*
 *
 * New NAND support
 *
 */
#include <linux/mtd/mtd.h>

#include <watchdog.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <jffs2/jffs2.h>
#include <nand.h>
#include <asm/arch/x_typedef.h>
#include <common.h>
#include <linux/mtd/mtd.h>

#include <watchdog.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <jffs2/jffs2.h>
#include <nand.h>
#include <asm/arch/x_typedef.h>

#include <partition.h>
#include <bootctrl.h>
#include "upgrade_rsd.h"

#ifdef CONFIG_BOOT_MMC
#define EMMC_UPGRADE //upgrade emmc
#else
#undef  EMMC_UPGRADE //upgrade nand
#endif

#ifdef CONFIG_USRDATA_EXT4
#define USRDATA_EXT4
#endif

DECLARE_GLOBAL_DATA_PTR;
//#define IMAGE_START_ADDR_IN_SDCARD  (0x400000)
//#define IMAGE_START_ADDR_IN_SDCARD  (0xB00000)
//#define MAC_START_ADDR_IN_SDCARD  (0xAFF000)

static int verify_image_flag = 0;

#define  READ_BUFFER_SIZE   (200*1024*1024)

//#ifdef CONFIG_CMD_SDAGENT
//unsigned int g_u4PartionAddress = 0;
//#endif

#define REPLICATION_NUMBER  8
static  char _szBLID1[12] = "BOOTLOADER!";
static char _szBLNFIID2[8] = "NFIINFO";
static char _szBLMSDCID2[8] = "MT3360A";	// Fixed Header String. Don't change it to "AC8317"

extern void flush_cache(unsigned int start, unsigned int size);
extern void flush_invalid_cache(unsigned int start, unsigned int size);

extern unsigned int mmc_bootup_device;

extern int mmc_erase(struct mmc * mmc,u32 type,u64 start_addr,u64 size);
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


typedef struct upgrade_header{
	char szSignature[4];
	unsigned int bFormatFlash;
	unsigned int bEraseemmc;
#ifndef CONFIG_BOOT_MMC
	unsigned int bVerifyImage;
#endif
	unsigned int bModifyPartition;
	unsigned int bAdvanceMode;
	unsigned int nSegmentSize;
	unsigned long long u8UserdataPartitionAddress;
	unsigned int nWriteproSize;
	unsigned int bEnableDump;
	unsigned int bEnableDumpOob;
	unsigned long long dumpStartAddr;
	unsigned long long dumpSize;
#ifndef CONFIG_BOOT_MMC
	unsigned int u4Reserver[512 - 64];
#else
	unsigned int u4Reserver[512 - 60];
#endif
}upgrade_header;


typedef  struct ext4chunk{

    UINT32 chunkAddress;
	UINT32 chunklength;
	unsigned int chunktype;
	//struct ext4chunk* nextchunk;
	unsigned int lastone;
}ext4chunk;

typedef  struct ext4chunkhead{

    UINT32 chunkblock;
	struct ext4chunk* chunkhead;
}ext4chunkhead;

static long  wifi_mac_addr_buf[512];
static long  bt_mac_addr_buf[512];
static unsigned int g_ImageStartAddr = 0xB00000;
static unsigned int g_MacStartAddr = 0xAFF000;
unsigned int g_logoAddrOnSD = 0x400000;
unsigned int g_arm2AddrOnSD = 0x800000;
unsigned int g_BootMiscAddrOnSD = 0xA00000;
unsigned int g_MetazoneAddrOnSD = 0xA40000;
unsigned int g_dtbAddrOnSD  = 0x200000;

extern unsigned int _logosize;
extern unsigned int _arm2size;
extern unsigned int _dtbsize;
extern unsigned int _bootmisc_size;
extern unsigned int _mtz_size;

unsigned long long _totalsize = 0;
unsigned long long _currentsize = 0;

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

static inline char hextostring(int x)
{
	if ( x <0 || x > 15){
		printf("hex reverse error\r\n");
		return '\0';
	}

	if ( x>=0 && x <=9)
		return x+'0';

	switch(x){
		case 10:
			return 'a';
		case 11:
			return 'b';
		case 12:
			return 'c';
		case 13:
			return 'd';
		case 14:
			return 'e';
		case 15:
			return 'f';
	}
}

static inline int ultohex(char *p, ulong num)
{

	int i = 0;
	ulong tmp = num;
	p[8] = '\0';
	while((tmp>>4) != 0){
		p[7-i] = hextostring(tmp - (tmp>>4) * 16);
		tmp = tmp >> 4;
		i++;
	}

	p[7-i] = hextostring(tmp);
	return i;
}

static void nand_erase_image(const char* name)
{
	int argc = 3;
	char *argv[] = {"nand", "erase", name};
	do_nand(NULL, 0, argc, argv);

}

static int read_nand_image (u32 base_addr, char *partitionName, unsigned size)
{
	char buf1[10] = {0};
	char buf2[10] = {0};
	char *argv[6] = {"nand", "read"};

	sprintf(buf1, "%x", base_addr);
	argv[2] = buf1;
	argv[3] = partitionName;
	sprintf(buf2, "%x", size);
	argv[4] = buf2;
	return do_nand(NULL, 0, 5, argv);
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


void create_bootloader_header(char *pBLHeader,  char* blbuf,UINT32 u4Imagesize,int msdc_boot)
{
	BOOTL_HEADER BLHeader;
	UINT32 u4chksum,temp32;
	unsigned char *pu4Buf = (unsigned char *)blbuf;
	UINT32 i;
	nand_info_t *nand = &nand_info[0];
	struct nand_chip *chip = nand->priv;


	memset(&BLHeader,0x0,sizeof(BOOTL_HEADER));
	memcpy(BLHeader.ID1,_szBLID1,12);
	if(msdc_boot)
		memcpy(BLHeader.ID2,_szBLMSDCID2,8);
	else
		memcpy(BLHeader.ID2,_szBLNFIID2,8);

	BLHeader.startAddr = 0x40000000;
	BLHeader.length = u4Imagesize;

	if(!msdc_boot){
		BLHeader.NFIinfo.pageSize = (UINT16)nand->writesize;
		BLHeader.pagesPerBlock = nand->erasesize / nand->writesize;
		BLHeader.totalBlocks = nand->size / nand->erasesize;

		BLHeader.NFIinfo.spareSize =  nand->oobsize;

		printf("oob size[%d]",BLHeader.NFIinfo.spareSize);
		BLHeader.NFIinfo.addressCycle =(UINT16)0x5;

		if(BLHeader.NFIinfo.pageSize > 512)
			BLHeader.NFIinfo.pageShift = (UINT16)0x10;
		else
			BLHeader.NFIinfo.pageShift = (UINT16)0x8;


		BLHeader.blockShift = (UINT16)(Log2(BLHeader.pagesPerBlock) + BLHeader.NFIinfo.pageShift);
	}
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


static int raw_image_write(struct mmc * mmc,int dev_num,partitioninfo *part)
{
	int blknum = 0;
	int n = 0;
	nand_info_t *nand = &nand_info[nand_curr_device];

	blknum =  ALIGN(part->u8RealDataSize, 512)/512;
	printf("blknum = %d\r\n",blknum);
	flush_invalid_cache(NAND_WRITE_BASE_ADDR, blknum * 512);
	n = mmc->block_dev.block_read(dev_num, part->u8OffsetData/512, blknum, (char *)NAND_WRITE_BASE_ADDR);
	if (!n)
	{
		printf("raw_image_write block_read fail result=%d\r\n",n);
		return -1;
	}
	part->u8RealDataSize = ALIGN(part->u8RealDataSize, nand->writesize);

	if(0 == strcmp(part->szPartName,"boot")){

		create_bootloader_header((char * )(NAND_WRITE_BASE_ADDR -512),NAND_WRITE_BASE_ADDR,0x7000,0); /*nand boot*/
		part->u8RealDataSize = ALIGN((part->u8RealDataSize+512), nand->writesize);
		write_nand_ex((uchar*)(NAND_WRITE_BASE_ADDR-512),part->u8RealDataSize,part->szPartName,0,part->szType,0);
		read_nand_image(0x100000,"boot",0x200000);

		if(MemCompare(0x100000,(NAND_WRITE_BASE_ADDR -512),part->u8RealDataSize))
			printf("success\r\n");
		else
			printf("failed\r\n");
	}else{

		write_nand_ex((uchar*)NAND_WRITE_BASE_ADDR,part->u8RealDataSize,part->szPartName,0,part->szType,0);
	}

	return 0;
}



extern int write_nand_ex(uchar* buf,ulong length,char * partitionName,ulong offset,char* type,int end);

static int  ext4_image_write(struct mmc * mmc,int dev_num,partitioninfo *part)
{
	nand_info_t *nand = &nand_info[nand_curr_device];
	int blknum =   ALIGN(part->u4RealDataSize, 512)/512;
	int start= part->u8OffsetData/512;
	int i,n;
	int end=0;
	ulong size,entiresize=0x4000;  //0x4000=16384, 16384*512=8M



	printf("blknum = %d\r\n",blknum);


	for(i=0;blknum>0;i++){
		if(blknum>entiresize){
			size=entiresize;
			end=0;
		}
		else{
			size=blknum;
			end=1;
		}

		flush_invalid_cache(NAND_WRITE_BASE_ADDR, size * 512);
		n = mmc->block_dev.block_read(dev_num,start+i*entiresize, size, (char *)NAND_WRITE_BASE_ADDR);
		if (!n)
		{
			printf("raw_image_write block_read fail result=%d\r\n",n);
			return -1;
		}

		write_nand_ex((char *)NAND_WRITE_BASE_ADDR,size*512,part->szPartName,i*0x800000,part->szType,end);
		blknum-=size;
	}

	return 0;
}

int emmc_switch_bootmode(struct mmc *emmc_dev)
{
	int err = 0;
	err = mmc_boot_enter_bootmode(emmc_dev->host_id);
	return err;
}

static int emmc_switch_normalmode(struct mmc *emmc_dev)
{
	int err = 0;
	err = mmc_boot_exit_bootmode(emmc_dev->host_id);
	return err;
}

int emmc_write_bootpart(struct mmc *emmc_dev, char * buffer,int writeblknum)
{
	int err = 0;
	int boot_part_num = 0;

	err = emmc_switch_bootmode(emmc_dev);
	if (err)
	{
		printf("emmc switch to boot mode failed, err = %d\r\n", err);
		return 1;
	}
	else
	{
		;//printf("------------------->>> emmc switch to boot mode successfully <<<---------------------\r\n");
	}

	printf("------------------->>> Start to write boot partition <<<---------------------\r\n");
	for(boot_part_num = 1; boot_part_num <= 2; boot_part_num++)
	{
		if (writeblknum > 256)
		{
			printf("!!!!!!!!  WARNNING: write data is too large, max size is 256 blocks (128KB) !!!!!!!!!\r\n", err);
			writeblknum = 256;
		}
		err = mmc_boot_bwrite(emmc_dev->host_id, boot_part_num, 0, writeblknum, (char *)buffer);
		if (err == 0) // 0 means that no data be written to boot partition
		{
			printf("write data to boot partition failed, err = %d\r\n", err);
			break;
		}

		/*****************************************************************************/
		//
		// The flow code is test code for verify second copy preloader in boot partition2
		// Enable below marco define will cause the first copy preloader will be useless
		//

		//#define VERIFY_PRELOADER_SECOND_COPY
		#ifdef VERIFY_PRELOADER_SECOND_COPY
		char zerostring[512];
		memset(zerostring, 0, 512);
		if (boot_part_num == 1) // erase first block data of boot partition 1
		{
			err = mmc_boot_bwrite(emmc_dev->host_id, boot_part_num, 0, 1, (char *)zerostring);
			if (err == 0) // 0 means that no data be written to boot partition
			{
				printf("earse boot partition 1 failed, err = %d\r\n", err);
			}
			else
			{
				printf("earse boot partition %d header successfully.\r\n", boot_part_num);
			}
		}
		#endif //VERIFY_PRELOADER_SECOND_COPY

		/*****************************************************************************/
	}
	printf("------------------->>> End to write boot partition <<<---------------------\r\n");

	err = emmc_switch_normalmode(emmc_dev);
	if (err)
	{
		printf("emmc switch to normal mode failed, err = %d\r\n", err);
		return 1;
	}
	else
	{
		;//printf("---------------------->>> emmc switch to normal mode successfully <<<-------------------\r\n");
	}

	return err;


}

#define MAX_BLOCK_SIZE		(0x4000) //0x4000=16384, 16384*512=8M

int emmc_write_raw_image(struct mmc * mmc, int dev_num, partitioninfo *part)
{
	int blknum = 0;
	int n = 0, i = 0;
	int writeblknum = 0;
	struct mmc *emmc_dev = NULL;
	int emmc_dev_num = 0;
	ulong size,entiresize = MAX_BLOCK_SIZE;
	int start = part->u8OffsetData/512;
	int end = 0;

	writeblknum = blknum =  ALIGN(part->u8RealDataSize, 512)/512;

	emmc_dev = find_mmc_device(emmc_dev_num);
	n = mmc_init(emmc_dev);

	if(0 == strcmp(part->szPartName, "preloader"))
	{

		int preloadersize = PRELOADER_SIZE;
		flush_invalid_cache(BASE_ADDR, blknum * 512);
		n = mmc->block_dev.block_read(dev_num, part->u8OffsetData/512, blknum, (char *)BASE_ADDR);
		if (blknum != n)
		{
			printf("<%s>:block_read fail, result=%d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
			if (1 == msdc_get_cd(dev_num))
				no_mmc_found_handle();
			return -1;
		}

		create_bootloader_header((char * )(BASE_ADDR - 512), BASE_ADDR, PRELOADER_SIZE, 1);/*msdc boot*/

		// Write to boot partition
		flush_cache(BASE_ADDR - 512, PRELOADER_SIZE);
		emmc_write_bootpart(emmc_dev,(char *)(BASE_ADDR - 512), 129);

		// Write to user partition
		writeblknum += 1; //+header size= 512
		flush_cache(BASE_ADDR - 512, writeblknum * 512);
		emmc_dev->block_dev.block_write(emmc_dev_num, 0, writeblknum, (char *)(BASE_ADDR - 512));

	    //g_u4PartionAddress = part->u4PartitionStartAddr + part->u4PartitionSize - 512;
	    //g_u4PartionAddress = 0x400000 - 512;

	}
	else
	{
		for(i=0; blknum>0; i++)
		{
			if(blknum > entiresize)
			{
				size = entiresize;
				end = 0;
			}
			else
			{
				size = blknum;
				end = 1;
			}

			//flush_invalid_cache((ulong)BASE_ADDR, size * 512);
			n = mmc->block_dev.block_read(dev_num, start+i*entiresize, size, (char *)BASE_ADDR);
			if (size != n)
			{
				printf("<%s>:block_read fail, result=%d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
				if (1 == msdc_get_cd(dev_num))
					no_mmc_found_handle();
				return -1;
			}

			n = emmc_dev->block_dev.block_write(emmc_dev_num, (unsigned long)(part->u8PartitionStartAddr/512) + i*entiresize, size, (char *)BASE_ADDR);

			if (n != size)
			{
				printf("<emmc_write_raw_image> block_write failed = %d\r\n", n);
				return -1;
			}

			blknum -= size;
			printf(".");
			_currentsize += size*512;
			LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
			if ((i%32 == 0) && (i != 0))
			{
				printf("\r\n");
			}
		}
		printf("\r\n");
	}
	return 0;
}

static int emmc_write_ext4_image(struct mmc *mmc,int dev_num,partitioninfo *part)
{
	int emmc_dev_num = 0;
	int err = 0;
	struct mmc *emmc_dev = NULL;
	int blknum =   ALIGN(part->u8RealDataSize, 512)/512;
	int start = part->u8OffsetData/512;
	int i = 0,n;
	int end = 0;
	ulong size,entiresize = MAX_BLOCK_SIZE;
	char chunkhead[512] ={0};
	unsigned int u8offset = 0;

    ext4chunk *pchunklisthead = NULL,*pchunkcurrent = NULL , *pchunknew = NULL;
	ext4chunkhead *pchunkhead = NULL;
	emmc_dev = find_mmc_device(emmc_dev_num);
	if ( NULL == emmc_dev){
		printf("can't find emmc device\n");
		return -1;
	}
	err = mmc_init(emmc_dev);
	if(err){
		printf("init emmc device fail\n");
		return -1;
	}
    //printf("emmc_write_ext4_image chunk head address:0x%x\r\n", start * 512);
	flush_invalid_cache(chunkhead, 512);
    n = mmc->block_dev.block_read(dev_num, start, 1, (char *)chunkhead);
	if (1 != n)
	{
		printf("<%s>:block_read chunk head fail, result = %d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
		if (1 == msdc_get_cd(dev_num))
			no_mmc_found_handle();
		return -1;
	}
	_currentsize += 512;
     //printf("emmc_write_ext4_image tt chunkhead0=0x%x,chunkhead1=0x%x,chunkhead2=0x%x,chunkhead3=0x%x,chunkhead4=0x%x,chunkhead5=0x%x,chunkhead6=0x%x,chunkhead7=0x%x\r\n",chunkhead[0], chunkhead[1],chunkhead[2],chunkhead[3],chunkhead[4], chunkhead[5],chunkhead[6],chunkhead[7]);
   // flush_cache((ulong)chunkhead,512);

	pchunkhead = (ext4chunkhead *)chunkhead;
    //printf("emmc_write_ext4_image tt chunkblock= %d\r\n", pchunkhead->chunkblock);


    char *p= (char *)malloc(pchunkhead->chunkblock * 512);
	flush_invalid_cache(p, pchunkhead->chunkblock * 512);
	n = mmc->block_dev.block_read(dev_num, start + 1, pchunkhead->chunkblock, (char *)p);
	if (pchunkhead->chunkblock != n)
	{
		printf("<%s>:block_read chunk list fail, result = %d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
		if (1 == msdc_get_cd(dev_num))
			no_mmc_found_handle();
		return -1;
	}
	_currentsize += pchunkhead->chunkblock * 512;
     pchunklisthead = (ext4chunk *)p;
    // printf("emmc_write_ext4_image hh chunkblock= %d\r\n", pchunkhead->chunkblock);
/*
     pchunknew = pchunklisthead;
	 do{
	 	printf("emmc_write_ext4_image chunkAddress=%d,chunklength=%d,chunktype=%d\r\n", pchunknew->chunkAddress,pchunknew->chunklength,pchunknew->chunktype);
		 //pchunknew = pchunknew->nextchunk;
		 if(pchunknew->lastone == 1)
		 	break;
		 pchunknew += 1;

	 }
	 while(1);
*/
     pchunkcurrent = pchunklisthead;

	do{


		if (pchunkcurrent->chunktype == 0)
	    {
	        if(pchunkcurrent->lastone == 1)
		 	    break;

			u8offset += pchunkcurrent->chunklength;
			_currentsize += pchunkcurrent->chunklength*512;
			pchunkcurrent += 1;
			continue;
	    }

		 //printf("emmc_write_ext4_image startblock= %d,blcknum=%d\r\n", pchunkcurrent->chunkAddress,pchunkcurrent->chunklength);
	    n = mmc->block_dev.block_read(dev_num, pchunkcurrent->chunkAddress, pchunkcurrent->chunklength, (char *)BASE_ADDR);
		if (pchunkcurrent->chunklength != n)
		{
			printf("<%s>:block_read fail, result = %d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
			if (1 == msdc_get_cd(dev_num))
				no_mmc_found_handle();
			return -1;
		}
		_currentsize += pchunkcurrent->chunklength*512;


		//flush_cache((ulong)BASE_ADDR, ALIGN(size*512, 512));

		//n = emmc_dev->block_dev.block_write(emmc_dev_num, part->u4PartitionStartAddr/512 + i*entiresize, size, (char *)BASE_ADDR);
		n = emmc_dev->block_dev.block_write(emmc_dev_num, (unsigned long)(part->u8PartitionStartAddr/512) + u8offset, pchunkcurrent->chunklength, (char *)BASE_ADDR);
		if (n != pchunkcurrent->chunklength)
		{
			printf("<emmc_write_ext4_image> block_write failed = %d\r\n", pchunkcurrent->chunklength);
			return -1;
		}

        //pchunkcurrent = pchunkcurrent->nextchunk;
        if(pchunkcurrent->lastone == 1)
		 	break;

		u8offset += pchunkcurrent->chunklength;
		pchunkcurrent += 1;

		i++;
		printf(".");
			LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
		if ((i%32 == 0) && (i != 0))
		{
			printf("\r\n");
		}

	}while(1);

			printf("part_name[%s]%u,%u\n",part->szPartName,_currentsize,(signed int)((double)_currentsize*100/_totalsize));

#if 0
	for(i=0; blknum>0; i++)
	{
		if(blknum > entiresize)
		{
			size = entiresize;
			end = 0;
		}
		else
		{
			size = blknum;
			end = 1;
		}

		n = mmc->block_dev.block_read(dev_num, start+i*entiresize, size, (char *)BASE_ADDR);
		if (!n)
		{
			printf("emmc_write_ext4_image block_read fail result = %d\r\n", n);
			return -1;
		}


		flush_cache((ulong)BASE_ADDR, ALIGN(size*512, 512));

		//n = emmc_dev->block_dev.block_write(emmc_dev_num, part->u4PartitionStartAddr/512 + i*entiresize, size, (char *)BASE_ADDR);
		n = emmc_dev->block_dev.block_write(emmc_dev_num, (unsigned long)(part->u8PartitionStartAddr/512) + i*entiresize, size, (char *)BASE_ADDR);
		if (n != size)
		{
			printf("<emmc_write_ext4_image> block_write failed = %d\r\n", size);
		}

		blknum -= size;
		printf(".");
		if ((i%32 == 0) && (i != 0))
		{
			printf("\r\n");
		}
	}
#endif

	printf("\r\n");

	return 0;

}

static int calculate_wifi_mac_address()
{
	printf ("\ncalculate_wifi_mac_address 0x%x\n", wifi_mac_addr_buf[5]);

	wifi_mac_addr_buf[6] = 0;
	if (wifi_mac_addr_buf[5]== 255)
	{
		wifi_mac_addr_buf[5] = 0;
		if (wifi_mac_addr_buf[4] == 255)
		{
			wifi_mac_addr_buf[4] = 0;
			if (wifi_mac_addr_buf[3] == 255)
			{
				wifi_mac_addr_buf[3] = 0;
				if (wifi_mac_addr_buf[2]== 255)
				{
					wifi_mac_addr_buf[2] = 0;
					if (wifi_mac_addr_buf[1] == 255)
					{
						wifi_mac_addr_buf[1] = 0;
						if (wifi_mac_addr_buf[0]== 255)
							return -1;
						else
						{
							wifi_mac_addr_buf[0] += 1;
							return 0;
						}
					}
					else
					{
						wifi_mac_addr_buf[1] += 1;
						return 0;
					}
				}
				else
				{
					wifi_mac_addr_buf[2] += 1;
					return 0;
				}
			}
			else
			{
				wifi_mac_addr_buf[3] += 1;
				return 0;
			}
		}
		else
		{
			wifi_mac_addr_buf[4] += 1;
			return 0;
		}
	}
	else
	{
		wifi_mac_addr_buf[5] += 1;
		printf ("\ncalculate_wifi_mac_address after 0x%x\n", wifi_mac_addr_buf[5]);
		return 0;
	}
}

static int calculate_bt_mac_address()
{
	bt_mac_addr_buf[6] = 0;
	if (bt_mac_addr_buf[5]== 255)
	{
		bt_mac_addr_buf[5] = 0;
		if (bt_mac_addr_buf[4] == 255)
		{
			bt_mac_addr_buf[4] = 0;
			if (bt_mac_addr_buf[3] == 255)
			{
				bt_mac_addr_buf[3] = 0;
				if (bt_mac_addr_buf[2]== 255)
				{
					bt_mac_addr_buf[2] = 0;
					if (bt_mac_addr_buf[1] == 255)
					{
						bt_mac_addr_buf[1] = 0;
						if (bt_mac_addr_buf[0]== 255)
							return -1;
						else
						{
							bt_mac_addr_buf[0] += 1;
							return 0;
						}
					}
					else
					{
						bt_mac_addr_buf[1] += 1;
						return 0;
					}
				}
				else
				{
					bt_mac_addr_buf[2] += 1;
					return 0;
				}
			}
			else
			{
				bt_mac_addr_buf[3] += 1;
				return 0;
			}
		}
		else
		{
			bt_mac_addr_buf[4] += 1;
			return 0;
		}
	}
	else
	{
		bt_mac_addr_buf[5] += 1;
		return 0;
	}
}

#ifdef NAND_UPG_RDBACK_CHECK
extern unsigned int calc_checksum_before_upg(char *buf,
	    unsigned long long real_size);

extern unsigned int calc_checksum_after_upg(unsigned long long offset,
	    unsigned long long real_size, unsigned long long part_size,
	    int mode);
#endif

static int nand_upgrade_raw_image(struct mmc * mmc,int dev_num,partitioninfo *part)
{
	int blknum = 0;
	int n = 0;
	int print_time = 2048;
	nand_info_t *nand = &nand_info[nand_curr_device];
	int max_size = 0x4000;
	int i = 0;
	int last_size = 0;
#ifdef NAND_UPG_RDBACK_CHECK
	unsigned int chksum_before = 0, chksum_after = 0;
	unsigned long long total_sz = 0;
	char szShowString[80] = {0};
#endif

	blknum =  ALIGN(part->u8RealDataSize, 512)/512;
	printf("blknum = %d\r\n",blknum);
	if((0 == strcmp(part->szPartName,"preloader")) ||
       (0 == strcmp(part->szPartName,"preloader_bk")))
	{
		flush_invalid_cache(NAND_WRITE_BASE_ADDR, blknum * 512);
		n = mmc->block_dev.block_read(dev_num, part->u8OffsetData/512, blknum, (char *)NAND_WRITE_BASE_ADDR);
		if (blknum != n)
		{
			printf("<%s>:block_read fail, result = %d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
			if (1 == msdc_get_cd(dev_num))
				no_mmc_found_handle();
			return -1;
		}
		create_bootloader_header((char * )(NAND_WRITE_BASE_ADDR -512),NAND_WRITE_BASE_ADDR,0x7000,0); /*nand boot*/
		part->u8RealDataSize = ALIGN((part->u8RealDataSize+512), nand->writesize);
		#ifdef NAND_UPG_RDBACK_CHECK
		if(verify_image_flag) {
			chksum_before += calc_checksum_before_upg((char *)(NAND_WRITE_BASE_ADDR - 512), part->u8RealDataSize);
			total_sz += part->u8RealDataSize;
		}
		#endif /* NAND_UPG_RDBACK_CHECK */
		flush_cache(NAND_WRITE_BASE_ADDR-512, part->u8RealDataSize);
		if(write_nand_ex((uchar*)(NAND_WRITE_BASE_ADDR-512),part->u8RealDataSize,part->szPartName,0,part->szType,0))
			return -1;
		_currentsize += part->u8RealDataSize;
		LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));

		#ifdef NAND_UPG_RDBACK_CHECK
		if(verify_image_flag) {
			chksum_after = calc_checksum_after_upg(part->u8PartitionStartAddr,
				    total_sz, part->u8PartitionSize, 0);

			if (chksum_before != chksum_after) {
				sprintf(szShowString, "   Error:%s chksum fail: %X != %X\n",
                        part->szPartName, chksum_before, chksum_after);
				goto chkfail;
			} else if (chksum_before == 0 && chksum_after == 0) {
				sprintf(szShowString, "   Error:%s chksum fail: =0\n",
					    part->szPartName);
				goto chkfail;
			} else {
				printf(" %s chksum compare success, chksum_before(0x%x), chksum_after(0x%x).\n",
					    part->szPartName, chksum_before, chksum_after);
			}
		}
		#endif /* NAND_UPG_RDBACK_CHECK */
	}
	else
	{
		if(blknum <= 0x4000)
		{
			last_size = ALIGN(blknum*512,nand->writesize);

			flush_invalid_cache(NAND_WRITE_BASE_ADDR, last_size);
			n = mmc->block_dev.block_read(dev_num, part->u8OffsetData/512, blknum, (char *)NAND_WRITE_BASE_ADDR);
			if (blknum != n)
			{
				printf("<%s>:block_read fail, result = %d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
				if (1 == msdc_get_cd(dev_num))
					no_mmc_found_handle();
				return -1;
			}
			#ifdef NAND_UPG_RDBACK_CHECK
			if(verify_image_flag) {
				chksum_before += calc_checksum_before_upg((char *)NAND_WRITE_BASE_ADDR, blknum * 512);
				total_sz += blknum * 512;
			}
			#endif /* NAND_UPG_RDBACK_CHECK */
			if(write_nand_ex((uchar*)NAND_WRITE_BASE_ADDR,last_size,part->szPartName,0,part->szType,0))
				return -1;
			_currentsize += blknum * 512;
			LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
		}
		else
		{
			while(blknum > 0)
			{
				if(blknum <= max_size)
				{
					last_size =  ALIGN(blknum*512,nand->writesize);

					flush_invalid_cache(NAND_WRITE_BASE_ADDR, last_size);
					n = mmc->block_dev.block_read(dev_num, part->u8OffsetData/512+i*max_size, last_size/512, (char *)NAND_WRITE_BASE_ADDR);
					if (last_size/512 != n)
					{
						printf("<%s>:block_read fail, result = %d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
						if (1 == msdc_get_cd(dev_num))
							no_mmc_found_handle();
						return -1;
					}
					#ifdef NAND_UPG_RDBACK_CHECK
					if(verify_image_flag) {
						chksum_before += calc_checksum_before_upg((char *)NAND_WRITE_BASE_ADDR, blknum * 512);
						total_sz += blknum * 512;
					}
					#endif /* NAND_UPG_RDBACK_CHECK */
					if(write_nand_ex((uchar*)NAND_WRITE_BASE_ADDR,last_size,part->szPartName,i*max_size*512,part->szType,0))
						return -1;
					_currentsize += blknum * 512;
					LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
					break;
				}
				else{
					#ifdef NAND_UPG_RDBACK_CHECK
					if(verify_image_flag) {
						flush_invalid_cache(NAND_WRITE_BASE_ADDR, max_size);
					}
					#endif
					n = mmc->block_dev.block_read(dev_num, part->u8OffsetData/512+i*max_size, max_size, (char *)NAND_WRITE_BASE_ADDR);
					if (max_size != n)
					{
						printf("<%s>:block_read fail, result = %d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
						if (1 == msdc_get_cd(dev_num))
							no_mmc_found_handle();
						return -1;
					}
					#ifdef NAND_UPG_RDBACK_CHECK
					if(verify_image_flag) {
						chksum_before += calc_checksum_before_upg((char *)NAND_WRITE_BASE_ADDR, max_size*512);
						total_sz += max_size*512;
					}
					#endif /* NAND_UPG_RDBACK_CHECK */
					if(write_nand_ex((uchar*)NAND_WRITE_BASE_ADDR,max_size*512,part->szPartName,i*max_size*512,part->szType,0))
						return -1;
					blknum -= max_size;
					_currentsize += max_size*512;
					LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
				}
				++i;
			}
		}

		#ifdef NAND_UPG_RDBACK_CHECK
		if(verify_image_flag) {
			chksum_after = calc_checksum_after_upg(part->u8PartitionStartAddr,
				    total_sz, part->u8PartitionSize, 0);

			if (chksum_before != chksum_after) {
				sprintf(szShowString, "   Error:%s chksum fail:%X != %X\n",
					    part->szPartName, chksum_before, chksum_after);
				goto chkfail;
			} else if (chksum_before == 0 && chksum_after == 0) {
				sprintf(szShowString, "   Error:%s chksum fail: 0\n",
					    part->szPartName);
				goto chkfail;
			} else {
				printf(" %s chksum compare success, chksum_before(0x%x), chksum_after(0x%x).\n",
					    part->szPartName, chksum_before, chksum_after);
			}
		}
		#endif /* NAND_UPG_RDBACK_CHECK */
	}
	return 0;
#ifdef NAND_UPG_RDBACK_CHECK
chkfail:
	printf("%s", szShowString);
	LCD_WriteString_FixedCharNum80(szShowString);
	LCD_WriteString_FixedCharNum80("   Please retry...\n");
	LCD_FreeStringBuf();
	while (1);
	return 0;
#endif
}
static int  format_nand()
{
	nand_info_t *nand = &nand_info[nand_curr_device];

	struct nand_chip *chip = nand->priv;
	struct erase_info instr;
	int ret = -1;
	int loop1 =0;

	for(loop1 = 0; loop1 < (chip->chipsize >> (chip->bbt_erase_shift)); loop1++ ){

		instr.mtd = nand;
		instr.addr = (1<<chip->bbt_erase_shift) * loop1;
		instr.len = (1<<chip->bbt_erase_shift) * 1;
		instr.callback = 0;
		ret = nand_erase_nand_ext(nand, &instr, 0);
		if (ret){

			if (nand->block_markbad(nand, NAND_MAX_PAGESIZE * loop1 * (1<<(chip->bbt_erase_shift-chip->page_shift)))) {
				printf("block 0x%08lx NOT marked "
						"as bad! ERROR %d\r\n",
						loop1, ret);
				ret = 1;
			} else {
				printf("block 0x%08lx successfully ""marked as bad\r\n", loop1);
			}
		}else{
			printf(".");
		}
	}
	chip->options |= NAND_BBT_SCANNED;
	chip->scan_bbt(nand);
}

static int  nand_upgrade_ext4_image(struct mmc * mmc,int dev_num,partitioninfo *part)
{
	nand_info_t *nand = &nand_info[nand_curr_device];
	int blknum =   ALIGN(part->u8RealDataSize, 512)/512;
	int start= part->u8OffsetData/512;
	int i,n;
	int end=0;
	ulong size,entiresize=0x4000;  //0x4000=16384, 16384*512=8M
#ifdef NAND_UPG_RDBACK_CHECK
	unsigned int chksum_before = 0, chksum_after = 0;
	unsigned long long total_sz = 0;
	char szShowString[80] = {0};
#endif
	printf("blknum = %d\r\n",blknum);

	for(i=0;blknum>0;i++)
	{
		if(blknum>entiresize)
		{
			size=entiresize;
			end=0;
		}
		else
		{
			size=blknum;
			end=1;
		}

		flush_invalid_cache(NAND_WRITE_BASE_ADDR, size * 512);
		n = mmc->block_dev.block_read(dev_num,start+i*entiresize, size, (char *)NAND_WRITE_BASE_ADDR);
		if (size != n)
		{
			printf("<%s>:block_read fail, result = %d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
			if (1 == msdc_get_cd(dev_num))
				no_mmc_found_handle();
			return -1;
		}
		//printf(".\r\n");
#ifdef NAND_UPG_RDBACK_CHECK
		if(verify_image_flag) {
			chksum_before += calc_checksum_before_upg((char *)NAND_WRITE_BASE_ADDR, size*512);
			//printf("####Before CheckSum = %08X.\n", chksum_before);
			total_sz += size*512;
		}
#endif /* NAND_UPG_RDBACK_CHECK */
		if(write_nand_ex((char *)NAND_WRITE_BASE_ADDR,size*512,part->szPartName,i*0x800000,part->szType,end))
			return -1;
		blknum-=size;
		_currentsize += size*512;
		LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
	}

#ifdef NAND_UPG_RDBACK_CHECK
	if(verify_image_flag) {
		chksum_after = calc_checksum_after_upg(part->u8PartitionStartAddr,
				total_sz, part->u8PartitionSize, 1);

		if (chksum_before != chksum_after) {
			sprintf(szShowString, "   Error:%s chksum fail: %X! = %X\n",
					part->szPartName, chksum_before, chksum_after);
			goto chkfail;
		} else if (chksum_before == 0 && chksum_after == 0) {
			sprintf(szShowString, "   Error:%s chksum fail: =0\n",
					part->szPartName);
			goto chkfail;
		} else {
			printf(" %s chksum compare success, chksum_before(0x%x), chksum_after(0x%x).\n",
					part->szPartName, chksum_before, chksum_after);
		}
	}
#endif /* NAND_UPG_RDBACK_CHECK */

	return 0;
#ifdef NAND_UPG_RDBACK_CHECK
chkfail:
	printf("%s", szShowString);
	LCD_WriteString_FixedCharNum80(szShowString);
	LCD_WriteString_FixedCharNum80("   Please retry...\n");
	LCD_FreeStringBuf();
	while (1);
	return 0;
#endif
}

#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
void print_wp_region(unsigned char id, unsigned long long start_addr, unsigned long long end_addr)
{
	unsigned int high1, low1;
	unsigned int high2, low2;

	u64_to_u32(start_addr, &high1,&low1);
	u64_to_u32(end_addr, &high2,&low2);
	printf("[WP]region[%d](0x%x%08x ~ 0x%x%08x) need write protect\n", id, high1, low1, high2, low2);
}
#endif

/*******************************************
* FunctionName:no_sd_or_udisk_find_handle
* Parameter: None
* Return: None
* Introduction: if not find external SD card or U disk,
                     then show corresponding infomation on the LCD.
********************************************/
void no_sd_or_udisk_find_handle(char *dev_type, int dev_num)
{
	char szShowString[60]={0};

	printf("dev_type = %s\n", dev_type);
	if ((0 == strcmp(dev_type, "mmc")) && (1 == msdc_get_cd(dev_num)))
	{
		memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   ...�Ҳ���SD�豸!!!...\n");
#else
		strcat(szShowString,"   ...Canot find SD device!!!\n");
#endif
		LCD_WriteString_FixedCharNum80(szShowString);
		LCD_FreeStringBuf();
		while(1)
		{
			Sleep(100);
		}
	}
	if ((0 == strcmp(dev_type, "usb")) && (0 == upg_rsd_check_udisk_exist()))
	{
		memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   ...�Ҳ���U Disk�豸!!!...\n");
#else
		strcat(szShowString,"   ...Canot find U Disk device!!!\n");
#endif
		LCD_WriteString_FixedCharNum80(szShowString);
		LCD_FreeStringBuf();
		while(1)
		{
			Sleep(100);
		}
	}
}

/*******************************************
* FunctionName:no_mmc_found_handle
* Parameter: None
* Return: None
* Introduction: if not find eMMC device or external SD card,
                     then show "can't find SD device on the LCD."
********************************************/
void no_mmc_found_handle()
{
	char szShowString[60]={0};
	printf("can't find SD device\n");
	memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
	strcat(szShowString,"   ...�Ҳ���SD�豸!!!");
#else
	strcat(szShowString,"   ...can't find SD device!!!");
#endif
	strcat(szShowString,"...\n");
	LCD_WriteString_FixedCharNum80(szShowString);
}

#ifndef EMMC_UPGRADE
int nand_check_partition(partitioninfo *part)
{
	int ret=0;
	unsigned long long off,realpartsize,realdatasize;
	int reserved_page_nums=0;
	int page_in_block;
	int reserved_block_ftl=0;
	int blocks_in_partition=0;
	nand_info_t *nand = &nand_info[nand_curr_device];
	realpartsize=part->u8PartitionSize;
	page_in_block=nand->erasesize/nand->writesize;
	blocks_in_partition=realpartsize/nand->erasesize;
	reserved_block_ftl=GET_RESERVED_BLOCK_NUM(blocks_in_partition);

	for (off = part->u8PartitionStartAddr;
		off < part->u8PartitionStartAddr+part->u8PartitionSize;off += nand->erasesize){
		if (nand_block_isbad(nand, off)){
			realpartsize-=nand->erasesize;
			printf("bad  %08lx\n", (ulong)off);
			}
		}
	if((0 == strcmp(part->szPartName,"preloader")) ||
       (0 == strcmp(part->szPartName,"preloader_bk"))){
			realdatasize=ALIGN((part->u8RealDataSize+512), nand->writesize);
		}
	else
			realdatasize=ALIGN((part->u8RealDataSize), nand->writesize);

	/*caculate the reserved page for ext4 partition*/
	if(strstr(part->szPartName, "_ext4")){
		reserved_page_nums=part->u8RealDataSize/nand->erasesize;
		reserved_page_nums+=reserved_page_nums/page_in_block;
		printf("reserved page %d \n",reserved_page_nums);
		if(blocks_in_partition>MIN_PARTION_BLOCK_NUM){
			printf("reserved block for ftl %d \n",reserved_block_ftl);
			realpartsize-=(reserved_block_ftl*nand->erasesize);
		}
	}

	realdatasize+=(reserved_page_nums*nand->writesize);

	if(realdatasize>realpartsize){
		printf("%s there are not enough partition size 0x%08x to save image 0x%08x \n",
			part->szPartName,realpartsize,realdatasize);
		return 1;
	 }
	else
		return 0;

}
#endif

#ifndef CONFIG_BOOT_MMC
extern int dump_partition(struct mmc *mmc, int dev_num, unsigned int startaddr, unsigned int size, unsigned int mode);
#endif
extern char *make_mtdparts_for_nand_upgrade(struct mmc *mmc, int dev_num, u64 start_addr, int *error);
int do_sdagent( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	struct mmc *mmc;

	int dev_num = 0,err,flashAddr = 0;
	int mac_addr = 0;
	int n = 0;
	char  buf[512] = {0};
	//long  wifi_mac_addr_buf[512];
	//long  bt_mac_addr_buf[512];
	long  wifi_mac_cnt_buf[512];
	long  bt_mac_cnt_buf[512];
	UINT8 wifi_mac_addr[6] = {0};
	UINT8 bt_mac_addr[6] = {0};
	long  wifi_mac_cnt = 0;
	long bt_mac_cnt = 0;
	UINT16  chip_buf[512];
	UINT8 wifichip = 0;
	UINT8 btchip = 0;
	UINT8 gpschip = 0;
    UINT16 fr_type_buf[512];//for front rear type
	upgrade_header  *pheader = NULL;
	int bFormatFlash = 0;
	int bEraseemmc = 0;
	boot_ctrl_t metadata;
	unsigned int bEnableDump = 0;
	unsigned int bEnableDumpOob = 0;
	unsigned long long dumpStartAddr = 0;
	unsigned long long dumpSize = 0;

#ifndef CONFIG_BOOT_MMC
	int bVerifyImage = 0;
	unsigned int chksum_sdcard = 0;
	unsigned int chksum_nand = 0;
	unsigned long long realsize = 0;

#endif

	int bModifyPartition = 0;
	int bYaffs2fs = 1;
	partitioninfo *part = NULL;
	unsigned long long u8UserdataPartitionAddress;

	partitionhead parthead;
	partitionread *pCurpart= NULL,*pPrepart=NULL;
	int partcnt = 0;
	char szShowString[60]={0};
	u64 blkcnt = 0;
	int emmc_dev_num = 0;
	struct mmc *emmc_dev = NULL;
	int error;
	char buffer[16]= {'\0'};

	long spendtime = 0L;
	spendtime = get_timer(0);
#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
	unsigned int wp_region_start = 0;
	unsigned int wp_region_end = 0;
	unsigned char wp_region_id = 0;
	unsigned char wp_region_flag = 0;
	int partition_info_is_write_protect = 0;
#endif
	printf("----------------->>>  start sdagent  <<<---------------\r\n");
	printf("----------------->>>use 64bit address<<<---------------\r\n");

#ifdef EMMC_UPGRADE
	printf("----------------->>>upgrade emmc<<<--------------------\r\n");
#else
	printf("----------------->>>upgrade nand<<<--------------------\r\n");
#endif
	metazone_init(0);
	LCD_CleanScreen();  
	LCD_MallocStringBuf();
	LCD_DrawProcess();
	LCD_DrawProcessNumber(0);
#ifdef NEW_PARTITION_DESIGN
	/* Check if ATC Upgrade Tool Match the minimum request*/
	extern int _aut_version;
	if (CONFIG_AUT_VERSION > _aut_version)
	{
		printf("ERR: AUT Tool Version[%x] is too old, Please at least use Version[%x]\r\n", _aut_version, CONFIG_AUT_VERSION);
		memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   ERR: ATC Upgrade Tool Version is Too Old!!!");
#else
		strcat(szShowString,"   ERR: ATC Upgrade Tool Version is Too Old!!!");
#endif
		strcat(szShowString,"...\n");
		LCD_WriteString_FixedCharNum80(szShowString);

		memset(szShowString,0,60);
		sprintf(szShowString, "   ERR: Please use Version%x.%x.%x.%x At Least\n",
			(CONFIG_AUT_VERSION & 0xf000) >> 12,
			(CONFIG_AUT_VERSION & 0xf00) >> 8,
			(CONFIG_AUT_VERSION & 0xf0) >> 4,
			(CONFIG_AUT_VERSION & 0xf));
		LCD_WriteString_FixedCharNum80(szShowString);

		LCD_FreeStringBuf();
		while(1);
	}
#endif

	#if 0
	for(dev_num = 2; dev_num > 0 ; dev_num--){

		mmc = find_mmc_device(dev_num);
		if(NULL != mmc){
			printf("find mmc device @%d\r\n",dev_num);
			break;
		}
	}
	#else
	dev_num = mmc_bootup_device;
	mmc = find_mmc_device(mmc_bootup_device);
	#endif
	if(mmc == NULL)
	{

		printf("can't find mmc device\n");
		memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   ...�Ҳ���mmc�豸!!!");
#else
		strcat(szShowString,"   ...can't find mmc device!!!");
#endif
		strcat(szShowString,"...\n");
		//LCD_CleanScreen();
		//LCD_WriteString(szShowString);
		LCD_WriteString_FixedCharNum80(szShowString);
		LCD_FreeStringBuf();
		return -1;

	}

	err = mmc_init(mmc);
	if (err)
	{
		printf("mmc init fail\r\n");
		return err;
	}

#ifdef EMMC_UPGRADE
	emmc_dev = find_mmc_device(emmc_dev_num);
	if ( NULL == emmc_dev)
	{
		printf("can't find emmc device\n");
		memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   ...�Ҳ���emmc�豸!!!");
#else
		strcat(szShowString,"   ...can't find emmc device!!!");
#endif
		strcat(szShowString,"...\n");
		//LCD_CleanScreen();
		//LCD_WriteString(szShowString);
		LCD_WriteString_FixedCharNum80(szShowString);
		LCD_FreeStringBuf();
		return -1;
	}

	err = mmc_init(emmc_dev);
	if(err)
	{
		printf("init emmc device fail\n");
		memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   ...��ʼ��emmc�豸ʧ��!!!");
#else
		strcat(szShowString,"   ...init emmc device fail!!!");
#endif
		strcat(szShowString,"...\n");
		//LCD_CleanScreen();
		//LCD_WriteString(szShowString);
		LCD_WriteString_FixedCharNum80(szShowString);
		LCD_FreeStringBuf();
		return -1;
	}
#endif

	/*
          // already do in logo_read
	if (g_logoAddressOnSD < g_dtbAddrOnSD + (unsigned long long)_dtbsize)
	{
		g_logoAddressOnSD = g_dtbAddrOnSD + (unsigned long long)_dtbsize;
	}

	//already do in arm2 read
	if (g_arm2AddrOnSD < g_logoAddrOnSD + (unsigned long long)_logosize)
	{
	    g_arm2AddrOnSD = g_logoAddrOnSD + (unsigned long long)_logosize;
	    //printf("Do SD Agent arm2 addr: 0x%x\n", g_arm2AddrOnSD);
	}
    */
	{
		g_BootMiscAddrOnSD = g_arm2AddrOnSD + (unsigned long long)_arm2size;
		g_MetazoneAddrOnSD = g_BootMiscAddrOnSD + (unsigned long long)_bootmisc_size;
		g_MacStartAddr = g_MetazoneAddrOnSD + (unsigned int)_mtz_size + 0xFF000;
		g_ImageStartAddr = g_MacStartAddr + 0x1000;
		//printf("Do SD Agent arm2 addr: 0x%x\n", g_MacStartAddr);
	}
	printf("Do SD Agent dtb addr: 0x%x\r\n", g_dtbAddrOnSD);
	printf("Do SD Agent logo addr: 0x%x\r\n", g_logoAddrOnSD);
	printf("Do SD Agent arm2 addr: 0x%x\r\n", g_arm2AddrOnSD);
	printf("Do SD Agent boot_misc addr: 0x%x\r\n", g_BootMiscAddrOnSD);
	printf("Do SD Agent metazone addr: 0x%x\r\n", g_MetazoneAddrOnSD);
	printf("Do SD Agent Mac addr: 0x%x\r\n", g_MacStartAddr);
	printf("Do SD Agent ImageStartAddr: 0x%x\r\n", g_ImageStartAddr);

	flashAddr = g_ImageStartAddr/512;
	blkcnt = 1;
	flush_invalid_cache(buf, blkcnt * 512);
	n = mmc->block_dev.block_read(dev_num,flashAddr -1,blkcnt,buf);
	if(blkcnt != n)
	{
		printf("<%s>:block_read fail, result = %d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
		if (1 == msdc_get_cd(dev_num))
			no_mmc_found_handle();
		return -1;
	}
	pheader  = (upgrade_header *)buf;
	bFormatFlash = pheader->bFormatFlash;   //flag for format "usrdata" partition
	bModifyPartition = pheader->bModifyPartition;
	u8UserdataPartitionAddress = pheader->u8UserdataPartitionAddress;
	printf("write-protect-size: 0x%x\r\n", pheader->nWriteproSize);
	bEraseemmc = pheader->bEraseemmc;   //flag for erase emmc or nand

	bEnableDump = pheader->bEnableDump;
	bEnableDumpOob = pheader->bEnableDumpOob;
	printf("bEnableDump = 0x%x, bEnableDumpOob = 0x%x\n", bEnableDump, bEnableDumpOob);
	dumpStartAddr = pheader->dumpStartAddr;
	dumpSize = pheader->dumpSize;
	printf("dumpStartAddr = 0x%x, dumpSize = 0x%x\n", (unsigned int)dumpStartAddr, (unsigned int)dumpSize);

#ifndef CONFIG_BOOT_MMC
	bVerifyImage= pheader->bVerifyImage;
	verify_image_flag = pheader->bVerifyImage;//usr crc check image
	printf("verify_image_flag = %d\n",verify_image_flag);

#endif
#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
	if(bModifyPartition == 1) {//should done before erase
		if(emmc_clear_all_wp()) {
			printf("[WP] clear all wp fail, cancel upgrade\n");
			return -1;
		}else{
			printf("[WP] clear all wp success\n");
		}
	}
#endif

#ifndef CONFIG_BOOT_MMC
	if (bEnableDump) {
		printf("======== Start Dump Partition ========\n");
		memset(szShowString,0,60);
		strcat(szShowString, "   Dump StartAddr:0X");
		sprintf(buffer, "%x", dumpStartAddr);
		strcat(szShowString, buffer);
		LCD_WriteString_FixedCharNum80(szShowString);
		memset(szShowString, 0, 60);
		memset(buffer,0,16);
		strcat(szShowString, "   Dump Length:0X");
		sprintf(buffer, "%x", dumpSize);
		strcat(szShowString, buffer);
		LCD_WriteString_FixedCharNum80(szShowString);
		if (!dump_partition(mmc, dev_num, (unsigned int)dumpStartAddr, (unsigned int)dumpSize, bEnableDumpOob)) {
			memset(szShowString, 0, 60);
			strcat(szShowString, "   Dump Success");
			LCD_WriteString_FixedCharNum80(szShowString);
			LCD_DrawProcessNumber(100);
		} else {
			memset(szShowString, 0, 60);
			strcat(szShowString, "   Dump failed");
			LCD_WriteString_FixedCharNum80(szShowString);
		}
		printf("======== Finish Dump Partition ========\n");
		LCD_FreeStringBuf();
		return 0;
	}
#endif

	if (bEraseemmc !=0)
	{
	#ifdef EMMC_UPGRADE
		printf("========Satrt erase eMMC========\n");
		mmc_erase(emmc_dev,8,0,1);
		printf("========Finish erase eMMC========\n");
	#else
		printf("========Satrt erase nand========\n");
		printf("TODO: add nand erase all\r\n");
		//TODO:add nand erase all?
		format_nand();
		printf("========Finish erase nand========\n");
	#endif
	}
#if 0
#ifndef EMMC_UPGRADE
	if(bFormatFlash != 0)
	{
		format_nand();
	}
#endif
#endif

	if (bModifyPartition == 1)//select all part in tool
	{
	#ifndef CONFIG_BOOT_MMC
		char *mtdpart;
		mtdpart = make_mtdparts_for_nand_upgrade(mmc, dev_num, flashAddr, &error);
		if(error == -1)
		{
			printf("can't get partition info from SD card\n");
			memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
			strcat(szShowString,"   ...���ܴ�SD���л�ȡ������Ϣ!!!");
#else
			strcat(szShowString,"   ...can't get partition info from SD card!!!");
#endif
			strcat(szShowString,"...\n");
			//LCD_CleanScreen();
		    //LCD_WriteString(szShowString);
		    LCD_WriteString_FixedCharNum80(szShowString);
			LCD_FreeStringBuf();
			return -1;
		}else if (-2 == error)
		{
			no_mmc_found_handle();
			return -1;
		}
		setenv("mtdparts", mtdpart);
	#endif

		parthead.nextpartition = NULL;
	}
	else//not upgrade all image
	{//nand upgrade need get partition info from mtdparts
	#ifndef EMMC_UPGRADE
		extern char *g_mtdparts;
		check_partition();
		setenv("mtdparts", g_mtdparts);

		error = readpartitioninfofromnand_ext();

	#else

		error = readpartitioninfofromflash_ext();

	#endif

		if (error == -1)
		{
			printf("get partition info fail\r\n");
			memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
			strcat(szShowString,"   ...��ȡ������Ϣʧ��!!!");
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

   do
   {
		blkcnt = 1;
		flush_invalid_cache((ulong)buf, blkcnt * 512);
		n = mmc->block_dev.block_read(dev_num, flashAddr, blkcnt, buf);
		if(blkcnt != n)
		{
			printf("<%s>:block_read fail, result = %d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
			if (1 == msdc_get_cd(dev_num))
				no_mmc_found_handle();
			return -1;
		}
		part = (partitioninfo *)buf;
		_totalsize += part->u8RealDataSize;
		flashAddr = part->u8OffsetNextImage/512;

	}while(part->u8OffsetNextImage != 0);
	_totalsize += _totalsize / 100;

	flashAddr = g_ImageStartAddr/512;

	do
	{
		int blknum = 0;
		u64 free_mmc_size = 0;
		blkcnt = 1;
		flush_invalid_cache((ulong)buf, blkcnt * 512);
		n = mmc->block_dev.block_read(dev_num, flashAddr, blkcnt, buf);
		if(blkcnt != n)
		{
			printf("<%s>:block_read fail, result = %d, @%s:%u\r\n", __FUNCTION__, n, __FILE__, __LINE__);
			if (1 == msdc_get_cd(dev_num))
				no_mmc_found_handle();
			return -1;
		}
		part = (partitioninfo *)buf;
		dumppartitioninfo(part);
#ifndef EMMC_UPGRADE
		/*to check bad block if there are enough space for nand upgrade because o*/
			if(nand_check_partition(part))
				return -1;
#endif
#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
		if((bModifyPartition == 0) && (isEnable(WRITE_PROTECT_ENABLE, part->u4Flag))) {
			if(emmc_set_user_wp(WP_DISABLE, part->u8PartitionStartAddr/512, part->u8PartitionSize/512, 1)) {
				printf("[WP]clear write protect fail, cancel upgrade(part: %s)\n", part->szPartName);
				return -1;
			} else
				printf("[WP]clear write protect success(part: %s)\n", part->szPartName);
		}
#endif

#ifdef NEW_PARTITION_DESIGN
		if(bModifyPartition != 1){
			u32 flag;
			getpartitionbypartitionname(&flag, part->szPartName);
			if(isEnable(UPGRADE_ENABLE, flag) == DISABLE)
			{
				printf("Partition[%s] donot be allowed upgraded!!!\n", part->szPartName);
				memset(szShowString,0,60);
				strcat(szShowString,"   ");
				strcat(szShowString, part->szPartName);
#if CONFIG_SUPPORT_CHAR == 1
				strcat(szShowString,"��������������!!!\n");
#else
				strcat(szShowString," donot be allowed upgraded!!!\n");
#endif
				LCD_WriteString_FixedCharNum80(szShowString);
				goto upgrade_next_partition;
			}
			else
			{
				printf("Partition[%s] will be upgraded\n", part->szPartName);
			}
		}
#endif

#ifndef EMMC_UPGRADE
		if (bModifyPartition == 1 && !strcmp(part->szPartName, "system_b")) {
			printf("skip erase system_b in nand\r\n");
		} else {
			if (erase_nand(part->szPartName) != 0)
			{
				printf("erase nand failed\r\n");
				memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
				strcat(szShowString,"   ...����nandʧ��!!!");
#else
				strcat(szShowString,"   ...erase nand failed fail!!!");
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
		if(0 == strcmp(part->szType,"raw"))
		{
			//char szShowString[60]={0};
			memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
			strcat(szShowString,"   ������������д�뾵��: ");
#else
			strcat(szShowString,"   writing image to flash: ");
#endif
			strcat(szShowString,part->szPartName);
			strcat(szShowString,"...\n");
			//LCD_CleanScreen();
			//LCD_WriteString(szShowString);
			LCD_WriteString_FixedCharNum80(szShowString);
#ifdef  EMMC_UPGRADE
			n = emmc_write_raw_image(mmc,dev_num,part);
#else
			if (part->u8RealDataSize != 0) //must have image,or nand_upgrade_raw_image will fail
			{
				n = nand_upgrade_raw_image(mmc,dev_num,part);
			}
			else
				n = 0;
#endif
			if (n == -1)
			{
				memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
				strcat(szShowString,"   ...ϵͳ����ʧ��!!!");
#else
				strcat(szShowString,"   ...upgrade Linux Image fail!!!");
#endif
				strcat(szShowString,"...\n");
				//LCD_CleanScreen();
				//LCD_WriteString(szShowString);
				LCD_WriteString_FixedCharNum80(szShowString);
				LCD_FreeStringBuf();
				return -1;
			}

			//if not upgrade all image, we must update real image size in partition table
			// here only update "raw" type, for "ext4" is compressed
			if (bModifyPartition != 1)
			{
				update_image_size_in_partition_table(part);
			}

		} else if(0 == strcmp(part->szType,"ext4")) {
			memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
			strcat(szShowString,"   ������������д�뾵��: ");
#else
			strcat(szShowString,"   writing image to flash: ");
#endif
			strcat(szShowString,part->szPartName);
			strcat(szShowString,"...\n");
			//LCD_CleanScreen();
			//LCD_WriteString(szShowString);
			LCD_WriteString_FixedCharNum80(szShowString);
#ifdef EMMC_UPGRADE
			n= emmc_write_ext4_image(mmc,dev_num,part);
#else
			if (bModifyPartition == 1 && !strcmp(part->szPartName, "system_b")) {
				printf("skip upgrade system_b in nand\r\n");
				blknum = ALIGN(part->u8RealDataSize, 512)/512;
				_currentsize += blknum * 512;
				LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
				n = 0;
			} else {
				n= nand_upgrade_ext4_image(mmc,dev_num,part);
			}
#endif
			if (n == -1) {
			    memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
				strcat(szShowString,"   ...ϵͳ����ʧ��!!!");
#else
				strcat(szShowString,"   ...upgrade Linux Image fail!!!");
#endif
				strcat(szShowString,"...\n");
				//LCD_CleanScreen();
				//LCD_WriteString(szShowString);
				LCD_WriteString_FixedCharNum80(szShowString);
				LCD_FreeStringBuf();
				return -1;
			}
#ifdef USRDATA_EXT4
			char * resizePartName = "usrdata";
			if (0 == strcmp(part->szPartName, resizePartName)) {
				free_mmc_size = emmc_dev->capacity - u8UserdataPartitionAddress;
				free_mmc_size -= (2 + 50) * 512;
				part->u8PartitionSize += free_mmc_size;
			}
#endif
		} else if (0 == strcmp(part->szType,"fat32")) {
			printf("format user define partition: %s......\r\n", part->szPartName);
			memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
			strcat(szShowString,"   ��ʽ������: ");
#else
			strcat(szShowString,"   format  flash: ");
#endif
			strcat(szShowString,part->szPartName);
			strcat(szShowString,"...\n");
			//LCD_CleanScreen();
			//LCD_WriteString(szShowString);
			LCD_WriteString_FixedCharNum80(szShowString);
			#ifdef EMMC_UPGRADE
				n = format_userdata_partition_u64(emmc_dev,emmc_dev_num,part->u8PartitionStartAddr,part->u8PartitionSize);
			#else
				//TODO:add nand format fat32?
				printf("TODO: add nand partition format to fat32?? \r\n");
				n = 0;
			#endif
			if (n == -1) {
				memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
				strcat(szShowString,"   ...ϵͳ����ʧ��!!!");
#else
				strcat(szShowString,"   ...upgrade Linux Image fail!!!");
#endif
				strcat(szShowString,"...\n");
				//LCD_CleanScreen();
				//LCD_WriteString(szShowString);
				LCD_WriteString_FixedCharNum80(szShowString);
				LCD_FreeStringBuf();
				return -1;
			}
		}
#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
		if(bModifyPartition == 1) {
			if(isEnable(WRITE_PROTECT_ENABLE, part->u4Flag)) {
				if(wp_region_flag == 0) {
					wp_region_start = part->u8PartitionStartAddr/512;
					wp_region_flag = 1;
				}
			} else {
				if(wp_region_flag) {
					wp_region_end = part->u8PartitionStartAddr/512;
					wp_region_flag = 0;
					wp_region_id ++;
					print_wp_region(wp_region_id, (unsigned long long)(wp_region_start)*512, (unsigned long long)(wp_region_end)*512);
					if(emmc_set_user_wp(WP_ENABLE, wp_region_start, wp_region_end - wp_region_start, 0)) {
						printf("[WP]region[%d] write protect fail, cancel upgrade\n", wp_region_id);
						return -1;
					} else
						printf("[WP]region[%d] write protect success\n", wp_region_id);

				}
			}
		} else {
			if(isEnable(WRITE_PROTECT_ENABLE, part->u4Flag)) {
				if(emmc_set_user_wp(WP_ENABLE, part->u8PartitionStartAddr/512, part->u8PartitionSize/512, 1)) {
					printf("[WP]set write protect fail(part: %s), cancel upgrade\n", part->szPartName);
					return -1;
				} else
					printf("[WP]set write protect success(part: %s)\n", part->szPartName);
			}
		}
#endif

upgrade_next_partition:
		flashAddr = part->u8OffsetNextImage/512;

#ifndef USRDATA_EXT4
        if (flashAddr == 0 && bFormatFlash == 1)
        {
            printf("format usrdata......\r\n");
			memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
			strcat(szShowString,"   ��ʽ��usrdata ");
#else
			strcat(szShowString,"   format usrdata ");
#endif
			strcat(szShowString,"...\n");
			//LCD_CleanScreen();
	        //LCD_WriteString(szShowString);
	        LCD_WriteString_FixedCharNum80(szShowString);
#ifdef EMMC_UPGRADE
            n = format_userdata_partition_u64(emmc_dev,emmc_dev_num,u8UserdataPartitionAddress,(u64)0);
#else
			//TODO:add nand format usrdata (FAT32)func ?????
			//run_command("nand erase usrdata",0);
			//run_command("nand format  usrdata",0);
			n = 0;
#endif
			if (n == -1)
			{
			    memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
				strcat(szShowString,"   ...ϵͳ����ʧ��!!!");
#else
				strcat(szShowString,"   ...upgrade Linux Image fail!!!");
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
		if (flashAddr == 0 && bFormatFlash != 1 && bModifyPartition == 1)
		{
		 	printf("Select all part ,but not froamt usrdata\r\n");
		}
		if (bModifyPartition == 1)//select all part in tool
		{
		    if (pCurpart != NULL)
				pPrepart = pCurpart;
		    pCurpart = mergepartitioninfo(part);
			if (pCurpart == NULL)
			{
			    printf("do_sdagent write new part fail\r\n");
				memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
				strcat(szShowString,"   ...ϵͳ����ʧ��!!!");
#else
				strcat(szShowString,"   ...upgrade Linux Image fail!!!");
#endif
				strcat(szShowString,"...\n");
				//LCD_CleanScreen();
			    //LCD_WriteString(szShowString);
			    LCD_WriteString_FixedCharNum80(szShowString);
				LCD_FreeStringBuf();
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
		}
		else
		{
			printf("do not write this to flash, because tool do not have all partitions info\r\n");
		}
	}while(part->u8OffsetNextImage != 0);
    //printf("readpartitioninfofromflash run\r\n");
	//readpartitioninfofromflash();

/*
#ifndef EMMC_UPGRADE
	if((bFormatFlash != 0) )
	{
		run_command("nand erase usrdata",0);
		run_command("nand format  usrdata",0);
	}
#endif
*/
    partitionread *ptbl_dz = (partitionread *)malloc(sizeof(partitionread));
    partitionread *ptbl_dz_bk = (partitionread *)malloc(sizeof(partitionread));
    partitionread *ptbl_ub_bk = (partitionread *)malloc(sizeof(partitionread));
    uint32_t chksum = 0;

    struct datazone_info dz;
    struct bootloader_message bcb;

	if (bModifyPartition == 1) //select all part in tool
	{
		parthead.blockcnt = (partcnt * sizeof(partitionread)) % 512 ? ((partcnt * sizeof(partitionread) + 512)/512) :(partcnt * sizeof(partitionread))/512;
	}
	else
	{
		extern partitionhead *g_partitionhead;
		memcpy((void *)&parthead, (void *)g_partitionhead, sizeof(partitionhead));
	}

	error = lookup_partition_by_name(parthead.nextpartition, ptbl_dz, "datazone");
    if (error == -1) {
        printf("can't lookup datzone partition.\n");
        err = -1;
        goto out_free_tbl;
    }

    error = lookup_partition_by_name(parthead.nextpartition, ptbl_dz_bk, "datazone_bk");
    if (error == -1) {
        printf("can't lookup datzone_bk partition.\n");
        err = -1;
        goto out_free_tbl;
    }
#ifdef ATC_AB_PARTITION_SUPPORT
    error  = lookup_partition_by_name(parthead.nextpartition, ptbl_ub_bk, "uboot_b");
    if (error == -1) {
        printf("can't lookup uboot_b partition.\n");
        err = -1;
        goto out_free_tbl;
    }
#else
    error  = lookup_partition_by_name(parthead.nextpartition, ptbl_ub_bk, "uboot_bk");
    if (error == -1) {
        printf("can't lookup uboot_bk partition.\n");
        err = -1;
        goto out_free_tbl;
    }
#endif

#ifdef CONFIG_BOOT_MMC
	unsigned long long dz_u8PartitionStartAddr = DATAZONE_MAIN_OFFSET_FROM_MMCBLK;
	unsigned long long dz_bk_u8PartitionStartAddr = DATAZONE_BK_OFFSET_FROM_MMCBLK;
    if (ptbl_dz->u8PartitionStartAddr != dz_u8PartitionStartAddr) {
        printf("datazone startaddress(0x%x%x) != (0x%x%x)\n", ptbl_dz->u8PartitionStartAddr, dz_u8PartitionStartAddr);
    }

    if (ptbl_dz_bk->u8PartitionStartAddr != dz_bk_u8PartitionStartAddr) {
        printf("datazone_bk startaddress(0x%x%x) != (0x%x%x)\n", ptbl_dz_bk->u8PartitionStartAddr, dz_bk_u8PartitionStartAddr);
    }

/*
* write partition info main
*/
#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
	if(DATAZONE_END/512 > emmc_dev->wp_size) {
		printf("[WP] warning: partition table is not in the first write protect group\n");
	}

	if(1 == emmc_wpg_type(DATAZONE_END/512/emmc_dev->wp_size)) {
		partition_info_is_write_protect = 1;
		if(emmc_set_user_wp(WP_DISABLE, 0, DATAZONE_END/512, 1))
			printf("[WP] clear wp fail in %s\n", __func__);
		else
			printf("[WP] clear wp success in %s\n", __func__);
	}
#endif

	err = writepartitioninfotoflash2(&parthead, (unsigned long)(ptbl_dz->u8PartitionStartAddr + PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN));
	if (err < 0) {
		printf("write main partition info fail.\n");
		err = -1;
		goto out_free_tbl;
	}

/*
* write partition info bk
*/
	err = writepartitioninfotoflash2(&parthead, (unsigned long)(ptbl_dz_bk->u8PartitionStartAddr + PARTITION_INFO_BK_OFFSET_FROM_DATAZONE_BK));
	if (err < 0) {
		printf("write bk partition info fail.\n");
		err = -1;
		goto out_free_tbl;
	}
#endif

	memset(&dz, 0, sizeof(struct datazone_info));
#ifdef CONFIG_BOOT_MMC
    err = read_datazone(&dz, ptbl_dz->u8PartitionStartAddr);
    if (err < 0) {
        printf("read main datazone fail.\n");
        err = -1;
        goto out_free_tbl;
    }
#else
	unsigned long dtzSize = BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN;
	unsigned long dtzBcbSize = dtzSize + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN;
	nand_info_t *nand = &nand_info[nand_curr_device];

	//err = read_nand_image(NAND_WRITE_BASE_ADDR, "datazone", ptbl_dz->u8PartitionSize);
	flush_invalid_cache(NAND_WRITE_BASE_ADDR, dtzSize);
	err = nand_read_skip_bad(nand, NAND_DATAZONE_MAIN_ADDR, &dtzSize, (u_char *)NAND_WRITE_BASE_ADDR);
	if (err < 0) {
		printf("read main datazone fail.\n");
		err = -1;
		goto out_free_tbl;
	}
	memcpy((void*)&dz, (void*)(unsigned long)NAND_WRITE_BASE_ADDR, sizeof(dz));
#endif

    adjust_datazone_img_desc_bk(&dz, ptbl_ub_bk);
    chksum = calc_datazone_checksum(&dz);
    printf("calc_datazone_checksum chksum = %d\r\n", chksum);
    put_datazone_checksum(&dz, chksum);
#ifdef CONFIG_BOOT_MMC
    err = write_datazone(&dz, ptbl_dz->u8PartitionStartAddr);
    if(err < 0){
        printf("write main datazone fail.\n");
        err = -1;
        goto out_free_tbl;
    }

/*
* write datazone_bk
*/
    err = write_datazone(&dz, ptbl_dz_bk->u8PartitionStartAddr);
    if(err < 0){
        printf("write bk databkzone fail.\n");
        err = -1;
        goto out_free_tbl;
    }
#endif
/*
* write bcb_main
*/
#ifdef ATC_AB_PARTITION_SUPPORT
#ifdef CONFIG_BOOT_MMC
    read_bcb(&bcb, ptbl_dz->u8PartitionStartAddr + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN);
    memcpy(&metadata, &(bcb.metadata), sizeof(bcb.metadata));
#else
	flush_invalid_cache(NAND_WRITE_BASE_ADDR, dtzBcbSize);
    err = nand_read_skip_bad(nand, NAND_DATAZONE_MAIN_ADDR, &dtzBcbSize, (u_char *)NAND_WRITE_BASE_ADDR);
	if (err < 0) {
		printf("read main datazone fail.\n");
		err = -1;
		goto out_free_tbl;
	}
    memcpy((char* )&bcb, (char* )(NAND_WRITE_BASE_ADDR + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN), sizeof(bcb));
    memcpy(&metadata, &(bcb.metadata), sizeof(bcb.metadata));
#endif
#endif

    memset(&bcb, 0, sizeof(struct bootloader_message));
    set_bcb_tags(&bcb);
    put_bcb_bootflag(&bcb, BOOTFLAG_STARTUP_A);
#if ATC_AB_PARTITION_SUPPORT
    memcpy(&bcb.metadata, &metadata, sizeof(metadata));
    if (bModifyPartition == 1) {
        printf("set_bcb_slotinfo to slotA\n");
        set_bcb_slotinfo(&bcb, BOOTFLAG_STARTUP_A);
    }
    dump_slot_metadata(bcb.metadata.slot_info);
#endif
    chksum = calc_bcb_checksum(&bcb);
    printf("bcb_checksum %x\n", chksum);
    put_bcb_checksum(&bcb, chksum);
#ifdef CONFIG_BOOT_MMC
    err = write_bcb(&bcb, ptbl_dz->u8PartitionStartAddr + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN);
    if (err < 0) {
        printf("write main bcb fail.\n");
        err = -1;
        goto out_free_tbl;
    }
    //bcb_readback_check(ptbl_dz->u8PartitionStartAddr + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN);

/*
* write bcb_bk
*/

    err = write_bcb(&bcb, ptbl_dz_bk->u8PartitionStartAddr + BCB_BK_OFFSET_FROM_DATAZONE_BK);
    if (err < 0) {
        printf("write bk bcb fail.\n");
        err = -1;
        goto out_free_tbl;
    }
#else

/*
 *  NAND: write dtz + bcb + partition to "datazone" and "datazone_bk" partition
 */
    memcpy((void*)(unsigned long)NAND_WRITE_BASE_ADDR, (void*)&dz, sizeof(dz));//update datazone
    memcpy((void*)(unsigned long)(NAND_WRITE_BASE_ADDR + BCB_BK_OFFSET_FROM_DATAZONE_BK), (void*)&bcb, sizeof(bcb));//update bcb
    err = writepartitioninfotonand_ext((char *)(unsigned long)NAND_WRITE_BASE_ADDR, &parthead);
    if(err < 0){
        printf("write datazone or datazone_bk failed.\n");
        err = -1;
        goto out_free_tbl;
    }
#endif

#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
	if(partition_info_is_write_protect) {
		if(emmc_set_user_wp(WP_ENABLE, 0,  DATAZONE_END/512, 1))
			printf("[WP] restore wp fail in %s\n", __func__);
		else {
			partition_info_is_write_protect = 0;
			printf("[WP] restore wp success in %s\n", __func__);
		}
	}
#endif

		check_partition();
		//Metazone_Read();

	memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
	strcat(szShowString,"   ������������д��WiFi MAC��ַ...\n");
#else
	strcat(szShowString,"   writing WiFi MAC address to flash...\n");
#endif
	//LCD_CleanScreen();
	//LCD_WriteString(szShowString);
	printf("MTZ start init\n");
	metazone_init(1);

	LCD_WriteString_FixedCharNum80(szShowString);

	mac_addr = g_MacStartAddr/512;

	flush_invalid_cache((ulong)wifi_mac_cnt_buf,512);
	n = mmc->block_dev.block_read(dev_num,mac_addr+2,1,wifi_mac_cnt_buf);
	wifi_mac_cnt = wifi_mac_cnt_buf[0];
	printf("========read MMC wifi mac count %d========\n", wifi_mac_cnt);

	if (wifi_mac_cnt != 0)
	{
		int ret = -1;
    	flush_invalid_cache((ulong)wifi_mac_addr_buf,512);
		n = mmc->block_dev.block_read(dev_num,mac_addr,1,wifi_mac_addr_buf);
    	//printf("========read MMC wifi mac buf %x:%x:%x:%x:%x:%x========\n",
    	//	wifi_mac_addr_buf[0], wifi_mac_addr_buf[1], wifi_mac_addr_buf[2], wifi_mac_addr_buf[3], wifi_mac_addr_buf[4], wifi_mac_addr_buf[5]);

    	wifi_mac_addr[0] = (UINT8)wifi_mac_addr_buf[0];
    	wifi_mac_addr[1] = (UINT8)wifi_mac_addr_buf[1];
    	wifi_mac_addr[2] = (UINT8)wifi_mac_addr_buf[2];
    	wifi_mac_addr[3] = (UINT8)wifi_mac_addr_buf[3];
    	wifi_mac_addr[4] = (UINT8)wifi_mac_addr_buf[4];
    	wifi_mac_addr[5] = (UINT8)wifi_mac_addr_buf[5];
    	printf("========read MMC wifi mac %x:%x:%x:%x:%x:%x========\n",
		    wifi_mac_addr[0], wifi_mac_addr[1], wifi_mac_addr[2], wifi_mac_addr[3], wifi_mac_addr[4], wifi_mac_addr[5]);

		ret = metazone_writebinary(0x10026, wifi_mac_addr,6);
		metazone_flush(TRUE);
		calculate_wifi_mac_address(wifi_mac_addr_buf);
		wifi_mac_cnt--;
		flush_cache(wifi_mac_addr_buf, 512);
		n = mmc->block_dev.block_write(dev_num, mac_addr, 1, (char *)wifi_mac_addr_buf);
		flush_cache(wifi_mac_cnt, 512);
		n = mmc->block_dev.block_write(dev_num, mac_addr+2, 1, (char *)&wifi_mac_cnt);
	}
	else
	{
		memset(szShowString,0,60);
		printf("========Use random WiFi MAC Address========\n");
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   û�п��õ�WiFi MAC��ַ,WiFi driver���ᴦ��!\n");
#else
		strcat(szShowString,"   No valid WiFi MAC addr set,WiFi driver will handle it!\n");
#endif
		//LCD_CleanScreen();
		//LCD_WriteString(szShowString);
		LCD_WriteString_FixedCharNum80(szShowString);
	}

	memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
	strcat(szShowString,"   ������������д��BT MAC��ַ...\n");
#else
	strcat(szShowString,"   writing BT MAC address to flash...\n");
#endif
	//LCD_CleanScreen();
	//LCD_WriteString(szShowString);
	LCD_WriteString_FixedCharNum80(szShowString);

	flush_invalid_cache((ulong)bt_mac_cnt_buf,512);
	n = mmc->block_dev.block_read(dev_num,mac_addr+3,1,bt_mac_cnt_buf);
	bt_mac_cnt = bt_mac_cnt_buf[0];
	printf("========read MMC bt mac count %d========\n", bt_mac_cnt);

	if (bt_mac_cnt != 0)
	{
        flush_invalid_cache((ulong)bt_mac_addr_buf,512);
        n = mmc->block_dev.block_read(dev_num,mac_addr+1,1,bt_mac_addr_buf);
        //printf("========read MMC bt mac buf %x:%x:%x:%x:%x:%x========\n",
           //     bt_mac_addr_buf[0], bt_mac_addr_buf[1], bt_mac_addr_buf[2], bt_mac_addr_buf[3], bt_mac_addr_buf[4], bt_mac_addr_buf[5]);

        bt_mac_addr[0] = (UINT8)bt_mac_addr_buf[0];
        bt_mac_addr[1] = (UINT8)bt_mac_addr_buf[1];
        bt_mac_addr[2] = (UINT8)bt_mac_addr_buf[2];
        bt_mac_addr[3] = (UINT8)bt_mac_addr_buf[3];
        bt_mac_addr[4] = (UINT8)bt_mac_addr_buf[4];
        bt_mac_addr[5] = (UINT8)bt_mac_addr_buf[5];
        printf("========read MMC bt mac %x:%x:%x:%x:%x:%x========\n",
            bt_mac_addr[0], bt_mac_addr[1], bt_mac_addr[2], bt_mac_addr[3], bt_mac_addr[4], bt_mac_addr[5]);
		metazone_writebinary(0x10027, bt_mac_addr,6);
		metazone_flush(TRUE);
		calculate_bt_mac_address(bt_mac_addr_buf);
		//printf("========read MMC bt mac %x:%x:%x:%x:%x:%x========\n",
		//	bt_mac_addr_buf[0], bt_mac_addr_buf[1], bt_mac_addr_buf[2], bt_mac_addr_buf[3], bt_mac_addr_buf[4], bt_mac_addr_buf[5]);
		bt_mac_cnt--;
		flush_cache(bt_mac_addr_buf, 512);
		n = mmc->block_dev.block_write(dev_num, mac_addr+1, 1, (char *)bt_mac_addr_buf);
		flush_cache(&bt_mac_cnt, 512);
		n = mmc->block_dev.block_write(dev_num, mac_addr+3, 1, (char *)&bt_mac_cnt);
	}
	else
	{
		memset(szShowString,0,60);
		printf("========Use random BT MAC Address========\n");
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   û�п��õ�BT MAC��ַ,BT driver���ᴦ��!\n");
#else
		strcat(szShowString,"   No valid BT MAC addr set,BT driver will handle it!\n");
#endif
		//LCD_CleanScreen();
		//LCD_WriteString(szShowString);
		LCD_WriteString_FixedCharNum80(szShowString);
	}

    flush_invalid_cache((ulong)chip_buf,512);
    n = mmc->block_dev.block_read(dev_num,mac_addr+4,1,chip_buf);

    printf("\nWiFi Chip ID: %d\n", chip_buf[0]);
    metazone_write(0x10028, chip_buf[0],1);
    metazone_flush(TRUE);

    printf("\nBT Chip ID: %d\n", chip_buf[1]);
	metazone_write(0x10029, chip_buf[2],1);
	metazone_flush(TRUE);

    printf("\nGPS Chip ID: %d\n", chip_buf[2]);
	metazone_write(0x10030, chip_buf[1],1);
	metazone_flush(TRUE);

    //for front rear type
    flush_invalid_cache(fr_type_buf,512);
    n = mmc->block_dev.block_read(dev_num,mac_addr+5,1,fr_type_buf);
    printf("\nFR Type: %d\n", fr_type_buf[0]);
	metazone_write(0x10031, fr_type_buf[0],1);
	metazone_flush(TRUE);

    memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
	strcat(szShowString,"   ...ϵͳ�����ɹ� ...\n");
#else
	strcat(szShowString,"   ...upgrade Linux Image Success...\n");
#endif
	LCD_DrawProcessNumber(100);
	//LCD_CleanScreen();
	//LCD_WriteString(szShowString);
	LCD_WriteString_FixedCharNum80(szShowString);
	LCD_FreeStringBuf();

	spendtime = get_timer(spendtime);
	printf("\nspendtime: %ld\n", spendtime);

out_free_tbl:
	if (bModifyPartition == 1) //select all part in tool
		freetblmemory(parthead.nextpartition);
	free(ptbl_dz);
	free(ptbl_dz_bk);
	free(ptbl_ub_bk);
	return 0;
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
		sdagent,	1,	1,	do_sdagent,
		"upgrade images via sd card",
		""
	  );

#ifdef  CONFIG_NAND_DEBUG_VERSION
#include <linux/mtd/atc_nfi.h>

//extern int arg_off_size(int argc, char *argv[], nand_info_t *nand, ulong *off, size_t *size);
extern int nand_check_ftl(nand_info_t *nand, uint32_t start_offset, uint32_t length);
extern int nand_check_ftl_after_update_by_sd(nand_info_t *nand, uint32_t start_offset, uint32_t length);
extern int nand_rw_test(nand_info_t *nand, uint32_t start_offset, uint32_t length, ulong cycle);
extern int atc_nand_read_raw(struct mtd_info *mtd, u32 pageaddr);
extern int atc_nand_read_raw_with_autofdm(struct mtd_info *mtd, u32 pageaddr, u32 *buf);
extern int nand_tune_timing_test(nand_info_t *nand, uint32_t start_offset, uint32_t length, uint cycle);
extern int nand_scan_read_all(void);
extern int nand_save_timing(int timing);
extern void nand_clean_timing(void);
extern  int nand_stress_test(ulong cycle);
extern int atc_nand_read_with_swecc(struct mtd_info *mtd, u32 page, u32 *buf);
int atc_nand_write_raw(struct mtd_info *mtd, u32 page, u32 *buf);

int page_ecc_nums;
int totel_ecc_bits;
int max_ecc_bits;
static inline int str2long(char *p, ulong *num)
{
        char *endptr;

        *num = simple_strtoul(p, &endptr, 16);
        return (*p != '\0' && *endptr == '\0') ? 1 : 0;
}

/* get mtd partition info from flash */
static int get_partition_position(int argc, char *argv[], ulong *off, size_t *size)
{
	extern struct list_head devices;
	struct list_head *dentry, *pentry;
	struct part_info *part;
	struct mtd_device *dev;
	int part_num;
	char *part_name = NULL;

	if(argc >= 2) {
		if (!(str2long(argv[0], (ulong *)off))) {
			printf("%s is not a number\n", argv[0]);
			goto FAIL;
		}
		if (!(str2long(argv[1], (ulong *)size))) {
			printf("%s is not a number\n", argv[1]);
			goto FAIL;
		}
	}
	else if(argc == 1) {
		part_name = argv[0];
		list_for_each(dentry, &devices) {
			dev = list_entry(dentry, struct mtd_device, link);
			/* list partitions for given device */
			list_for_each(pentry, &dev->parts) {
				part = list_entry(pentry, struct part_info, link);
				if(0 == strcmp(part->name, part_name)) {
					*off = part->offset;
					*size = part->size;
					goto SUCCESS;
				}

			}
		}
		goto FAIL;
	}
SUCCESS:
	printf("[%s]:off 0x%x, size 0x%x\r\n", part_name ? part_name : "unknown", *off, *size);
	return 0;
FAIL:
	printf("get address fail\r\n");
	return -1;
}

static int nand_cmp_single(nand_info_t *nand, ulong off)
{
	int i;
	u_char *datbuf, *oobbuf, *p;
	struct nand_chip *chip = nand->priv;
	u32 pageaddr;

	pageaddr = off;
	off <<= chip->page_shift;
	datbuf = malloc(nand->writesize + nand->oobsize);
	oobbuf = malloc(nand->oobsize);
	if (!datbuf || !oobbuf) {
		puts("No memory for page buffer\n");
		return 1;
	}
	off &= ~(nand->writesize - 1);
	loff_t addr = (loff_t) off;
	struct mtd_oob_ops ops;
	memset(&ops, 0, sizeof(ops));
	ops.datbuf = datbuf;
	ops.oobbuf = oobbuf; /* must exist, but oob data will be appended to ops.datbuf */
	ops.len = nand->writesize;
	ops.ooblen = nand->oobsize;
	ops.mode = MTD_OOB_PLACE;
	i = nand->read_oob(nand, addr, &ops);
	if (i < 0) {
		printf("Error (%d) reading page %08lx\n", i, off);
		free(datbuf);
		free(oobbuf);
		return 1;
	}
#if 0
	printf("Page %08lx dump(ECC on):\n", off);
	i = nand->writesize >> 4;
	p = datbuf;

	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x"
			     "  %02x %02x %02x %02x %02x %02x %02x %02x\n",
			     p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			     p[8], p[9], p[10], p[11], p[12], p[13], p[14],
			     p[15]);
		p += 16;
	}
	puts("OOB:\n");
	i = nand->oobsize >> 3;
	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
		       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}
#endif
	chip->select_chip(nand, 0);
	if(atc_nand_read_raw_with_autofdm(nand, pageaddr, NULL)) {
		printf("read fail\n");
		return -1;
	}
	chip->select_chip(nand, -1);
#if 0
	printf("Page %08lx dump(ECC off):\n", off);
	i = nand->writesize >> 4;
	p = chip->buffers->databuf;

	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x"
			     "  %02x %02x %02x %02x %02x %02x %02x %02x\n",
			     p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			     p[8], p[9], p[10], p[11], p[12], p[13], p[14],
			     p[15]);
		p += 16;
	}
	puts("OOB:\n");
	i = nand->oobsize >> 3;
	p = chip->oob_poi;
	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
		       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}

#endif
	for(i=0; i < nand->writesize; i++) {
		if(datbuf[i] != chip->buffers->databuf[i]) {
			printf("diff [%d]: raw:0x%x, ecc:0x%x\n", i, chip->buffers->databuf[i], datbuf[i]);
		}
	}
	for(i = 0 ; i < 9;  i++) {
		if(oobbuf[i] != chip->oob_poi[i]) {
			printf("oob diff [%d]: raw:0x%x, ecc:0x%x\n", i, chip->oob_poi[i], oobbuf[i]);
		}
	}
	printf("addr = 0x%x Done.\r\n",off);

	free(datbuf);
	free(oobbuf);

	return 0;
}

static nand_readraw_dump(nand_info_t *nand, ulong pageaddr)
{
	struct nand_chip *chip = nand->priv;
	int i;
	u_char *p;
	int line = 0;
	unsigned int chksum;

	chip->select_chip(nand, 0);
	if(atc_nand_read_raw(nand, pageaddr)) {
		printf("read fail\n");
		return -1;
	}
	chip->select_chip(nand, -1);

	printf("Page %08lx dump(ECC off AUTOFDM off):\n", pageaddr);
	i = (nand->writesize)>> 4;
	p = chip->buffers->databuf;
	chksum = checksum32(0, p, nand->writesize);
	while (i--) {
		printf("\t[0x%04x] %02x %02x %02x %02x %02x %02x %02x %02x"
				"  %02x %02x %02x %02x %02x %02x %02x %02x\n",
				line,p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
				p[8], p[9], p[10], p[11], p[12], p[13], p[14],
				p[15]);
		p += 16;
		line +=16;
	}
	printf("chksum = 0x%x\n",chksum);
	printf("OOB:\n");
	i = nand->oobsize >> 3;
	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
			   p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}
	printf("\r\n");

}

static nand_readraw_with_autofdm_dump(nand_info_t *nand, ulong pageaddr)
{
	struct nand_chip *chip = nand->priv;
	int i;
	u_char *p;
	int line = 0;
	unsigned int chksum;

	chip->select_chip(nand, 0);
#if 0
	if(atc_nand_read_raw_with_autofdm(nand, pageaddr, NULL)) {
		printf("read fail\n");
		return -1;
	}
#else
	atc_nand_read_with_swecc(nand, pageaddr, NULL);
#endif
	chip->select_chip(nand, -1);
	
	printf("Page %08lx dump(ECC off):\n", pageaddr);
	i = nand->writesize >> 4;
	p = chip->buffers->databuf;
	chksum = checksum32(0, p, nand->writesize);

	while (i--) {
		printf("\t[0x%04x]%02x %02x %02x %02x %02x %02x %02x %02x"
				 "	%02x %02x %02x %02x %02x %02x %02x %02x\n",
				 line,p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
				 p[8], p[9], p[10], p[11], p[12], p[13], p[14],
				 p[15]);
		p += 16;
		line +=16;
	}
	printf("chksum = 0x%x\n",chksum);
	printf("OOB:%d\n",nand->oobsize);
	i = nand->oobsize >> 3;
	p = chip->oob_poi;

	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
			   p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}
}

static nand_readraw_with_ecc_dump(nand_info_t *nand, ulong pageaddr)
{
	int i;
	u_char *datbuf, *oobbuf, *p;
	struct nand_chip *chip = nand->priv;
	u32 off;
	unsigned int chksum;
	int line = 0;

	off = pageaddr;
	off <<= chip->page_shift;
	datbuf = malloc(nand->writesize + nand->oobsize);
	oobbuf = malloc(nand->oobsize);
	if (!datbuf || !oobbuf) {
		puts("No memory for page buffer\n");
		return 1;
	}
	off &= ~(nand->writesize - 1);
	loff_t addr = (loff_t) off;
	struct mtd_oob_ops ops;
	memset(&ops, 0, sizeof(ops));
	ops.datbuf = datbuf;
	ops.oobbuf = oobbuf; /* must exist, but oob data will be appended to ops.datbuf */
	ops.len = nand->writesize;
	ops.ooblen = nand->oobsize;
	ops.mode = MTD_OOB_PLACE;
	i = nand->read_oob(nand, addr, &ops);
	if (i < 0) {
		printf("Error (%d) reading page %08lx\n", i, off);
		free(datbuf);
		free(oobbuf);
		return 1;
	}

	printf("Page %08lx dump(ECC on):\n", off);
	i = nand->writesize >> 4;
	p = datbuf;
	chksum = checksum32(0, p, nand->writesize);

	while (i--) {
		printf("\t[0x%04x]%02x %02x %02x %02x %02x %02x %02x %02x"
				 "	%02x %02x %02x %02x %02x %02x %02x %02x\n",
				 line,p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
				 p[8], p[9], p[10], p[11], p[12], p[13], p[14],
				 p[15]);
		p += 16;
		line +=16;
	}
	printf("chksum = 0x%x\n",chksum);
	printf("OOB:%d\n",nand->oobsize);
	i = nand->oobsize >> 3;
	p = oobbuf;
	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
			   p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}
	free(datbuf);
	free(oobbuf);

}

static nand_write_with_ecc(nand_info_t *nand, ulong pageaddr)
{
	int i;
	u_char *datbuf, *oobbuf, *p;
	struct nand_chip *chip = nand->priv;
	u32 off;
	unsigned int chksum;
	int line = 0;

	off = pageaddr;
	off <<= chip->page_shift;
	datbuf = malloc(nand->writesize + nand->oobsize);
	oobbuf = malloc(nand->oobsize);
	if (!datbuf || !oobbuf) {
		puts("No memory for page buffer\n");
		return 1;
	}
	off &= ~(nand->writesize - 1);
	loff_t addr = (loff_t) off;
	struct mtd_oob_ops ops;
	memset(&ops, 0, sizeof(ops));
	for(i=0;i <nand->writesize; i++){
		datbuf[i] = i & 0xff;
	}

	for(i=0;i <nand->oobsize; i++){
		oobbuf[i] = (i + 1) & 0xff;
	}

	ops.datbuf = datbuf;
	ops.oobbuf = oobbuf; /* must exist, but oob data will be appended to ops.datbuf */
	ops.len = nand->writesize;
	ops.ooblen = nand->oobsize;
	ops.mode = MTD_OOB_PLACE;
	i = nand->write_oob(nand, addr, &ops);
	if (i < 0) {
		printf("Error (%d) reading page %08lx\n", i, off);
		free(datbuf);
		free(oobbuf);
		return 1;
	}

	printf("Page %08lx \n", off);
	i = nand->writesize >> 4;
	p = datbuf;
	chksum = checksum32(0, p, nand->writesize);

	while (i--) {
		printf("\t[0x%04x]%02x %02x %02x %02x %02x %02x %02x %02x"
				 "	%02x %02x %02x %02x %02x %02x %02x %02x\n",
				 line,p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
				 p[8], p[9], p[10], p[11], p[12], p[13], p[14],
				 p[15]);
		p += 16;
		line += 16;
	}
	printf("chksum = 0x%x\n",chksum);
	printf("OOB:%d\n",nand->oobsize);
	i = nand->oobsize >> 3;
	p = oobbuf;
	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
			   p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}

	free(datbuf);
	free(oobbuf);
}

static nand_write_raw(nand_info_t *nand, ulong pageaddr, u8 *buf)
{
	int i;
	u_char *datbuf, *oobbuf, *p;
	struct nand_chip *chip = nand->priv;

	unsigned int chksum;
	int line = 0;
	printf("Page %08lx \n", pageaddr);

	datbuf = malloc(nand->writesize + nand->oobsize);
	memcpy(datbuf, buf, nand->writesize + nand->oobsize);
	i = nand->writesize >> 4;
	p = datbuf;
	chksum = checksum32(0, p, nand->writesize);
	datbuf[0] = 0x55;
	datbuf[0x406] = 0xaa;
	line = 0;
	while (i--) {
		printf("\t[0x%04x]%02x %02x %02x %02x %02x %02x %02x %02x"
				 "	%02x %02x %02x %02x %02x %02x %02x %02x\n",
				 line,p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
				 p[8], p[9], p[10], p[11], p[12], p[13], p[14],
				 p[15]);
		p += 16;
		line += 16;
	}
	printf("chksum = 0x%x\n",chksum);
	printf("OOB:%d\n",nand->oobsize);
	i = nand->oobsize >> 3;
	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
			   p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}

	chip->select_chip(nand, 0);
	atc_nand_write_raw(nand,pageaddr,datbuf);
	chip->select_chip(nand, -1);

	i = nand->writesize >> 4;
	p = datbuf;
	chksum = checksum32(0, p, nand->writesize);
	printf("================================\n");

	line = 0;
	while (i--) {
		printf("\t[0x%04x]%02x %02x %02x %02x %02x %02x %02x %02x"
				 "	%02x %02x %02x %02x %02x %02x %02x %02x\n",
				 line,p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
				 p[8], p[9], p[10], p[11], p[12], p[13], p[14],
				 p[15]);
		p += 16;
		line += 16;
	}
	printf("chksum = 0x%x\n",chksum);
	printf("OOB:%d\n",nand->oobsize);
	i = nand->oobsize >> 3;
	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
			   p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}


	free(datbuf);
}

extern void nanddump_register();

int do_atc(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i;
	nand_info_t *nand;
	struct nand_chip *chip;
	ulong addr, off;
	size_t size;
	char *cmd;
#if 0
	for(i=0; i<argc; i++) {
		printf("arg%d: %s\n", i, argv[i]);
	}
#endif
	if (nand_curr_device < 0 || nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE || !nand_info[nand_curr_device].name) {
		puts("\nno devices available\n");
		return 1;
	}
	nand = &nand_info[nand_curr_device];
	chip = nand->priv;

	if(argc < 2)
		goto usage;

	cmd = argv[1];
	if(0 == strncmp(cmd, "checkext4", 9)) {//check ext4 partition
		char *partition = argv[2];
		if(!partition)
			goto usage;
		if(get_partition_position(argc - 2, argv + 2, &off, &size) != 0)
			goto usage;

		if(nand_check_ftl_after_update_by_sd(nand, off, size) != 0)
			return 1;
		return 0;
	} else if(0 == strncmp(cmd, "checkftl", 8)) {//check ftl
		char *partition = argv[2];
		if(!partition)
			goto usage;
		if(get_partition_position(argc - 2, argv + 2, &off, &size) != 0)
			goto usage;
		if(nand_check_ftl(nand, off, size) != 0)
			return 1;
		return 0;
	} else if(0 == strncmp(cmd, "rwtest", 6)) {
		ulong cycle = 1;
		char *partition = argv[3];
		if(!partition)
			goto usage;

        if (!(str2long(argv[2], &cycle))) {
            printf("'%s' is not a number\n", argv[2]);
            goto usage;
        }

		if(get_partition_position(argc - 3, argv + 3, &off, &size) != 0)
			goto usage;

		if(nand_rw_test(nand, off, size, cycle))
			return 1;
		else
			return 0;
	} else if(0 == strncmp(cmd, "readraw", 7)) {
		u32 pageaddr;
		u32 type;

	    if (!(str2long(argv[2], &pageaddr))) {
		    printf("'%s' is not a number\n", argv[2]);
		    goto usage;
		}
		type = (int)simple_strtoul(argv[3], NULL, 16);

		if (type == 1)
			nand_readraw_dump(nand, pageaddr);
		else if(type == 2)
			nand_readraw_with_autofdm_dump(nand, pageaddr);
		else if(type == 3)
			nand_readraw_with_ecc_dump(nand, pageaddr);
		else
			printf("not supprot type=%d \n", type);
		return 0;
	}else if(0 == strncmp(cmd, "cmp", 3)) {
		ulong offset;
		ulong len;
		ulong pageaddr;
		if (!(str2long(argv[2], &offset))) {
		    printf("'%s' is not a number\n", argv[2]);
		    goto usage;
		}

		if (!(str2long(argv[3], &len))) {
		    printf("'%s' is not a number\n", argv[2]);
		    goto usage;
		}

		printf("offset = 0x%x len =0x%x\n", offset, len);

		if (!len)
			len = nand->size;

		for (pageaddr = offset; pageaddr < (offset + len); pageaddr += nand->writesize){
			nand_cmp_single(nand, pageaddr / nand->writesize);
		}

		return 0;
	}else if(0==strncmp(cmd, "get_timing",10) ) {
		printf("[nand] get NFI_ACCCON: 0x%08x\n", *NFI_ACCCON);
		return 0;
	}else if(0==strncmp(cmd, "set_timing",10)) {
		int val = (int)simple_strtoul(argv[2], NULL, 16);
		*NFI_ACCCON=val;
		printf("[nand] set NFI_ACCCON: 0x%08x\n", *NFI_ACCCON);
		return 0;
	}else if(0==strncmp(cmd, "tune_timing",11)) {
		int val = (int)simple_strtoul(argv[2], NULL, 16);
		*NFI_ACCCON=val;
		printf("[nand] set NFI_ACCCON: 0x%08x\n", *NFI_ACCCON);
		off=(int)simple_strtoul(argv[3], NULL, 16);
		size=(int)simple_strtoul(argv[4], NULL, 16);
		int cycle=(int)simple_strtoul(argv[5], NULL, 16);
		nand_tune_timing_test(nand, off, size, cycle);
		printf("[nand] tune NFI_ACCCON result: 0x%08x\n", *NFI_ACCCON);
		return 0;
	}else if(0==strncmp(cmd, "read_scan",9)) {
		page_ecc_nums=0;
		totel_ecc_bits=0;
		max_ecc_bits=0;
		nand_scan_read_all();
		printf("page_ecc_nums %d  totel_ecc_bits %d max_ecc_bit %d \n",
			page_ecc_nums,totel_ecc_bits,max_ecc_bits);
		return 0;
	}else if(0==strncmp(cmd, "stress_test",11)) {
		ulong cycle = (ulong)simple_strtoul(argv[2], NULL, 10);
		page_ecc_nums=0;
		totel_ecc_bits=0;
		max_ecc_bits=0;
		nand_stress_test(cycle);
		printf("page_ecc_nums %d  totel_ecc_bits %d max_ecc_bit %d \n",
			page_ecc_nums,totel_ecc_bits,max_ecc_bits);
		return 0;
	}else if(0==strncmp(cmd, "save_timing",11)) {
		int val = (int)simple_strtoul(argv[2], NULL, 16);
		printf("save timing value 0x%x \n",val);
		nand_save_timing(val);
		return 0;
	}else if(0==strncmp(cmd, "clean_timing",12)) {
		//int val = (int)simple_strtoul(argv[2], NULL, 16);
		nand_clean_timing();
		printf("clean timing done \n");
		return 0;
	} else if(0==strncmp(cmd, "write",5)) {
		u32 pageaddr;
		u32 type;

	    if (!(str2long(argv[2], &pageaddr))) {
		    printf("'%s' is not a number\n", argv[2]);
		    goto usage;
		}
		
		type = (int)simple_strtoul(argv[3], NULL, 16);
		if (type == 1)
			nand_write_with_ecc(nand,pageaddr);
		else if (type == 2){
			nand_write_raw(nand,pageaddr,chip->buffers->databuf);
		}
	} else if(0==strncmp(cmd, "dump_reg",8)) {
		nanddump_register();
	} else {
		printf("Wrong command\n");
	}

usage:
	cmd_usage(cmdtp);
	return 1;

}

U_BOOT_CMD(atc, 6, 1, do_atc,
		"ATC tool sets",
		"atc checkext4 [off] [size] - check ext4 image\n"
		"atc checkext4 [partition] - check ext4 image\n"
		"atc checkftl [off] [size]- check ftl info\n"
		"atc checkftl [partition]- check ftl info\n"
		"atc rwtest [test cycle] [off] [size] - nand rw test\n"
		"atc rwtest [test cycle] [partition] - nand rw test\n"
		"atc cmp [page] - compare read a single page with ECC on and off\n"
		"atc readraw [page] - read a single page with ECC off and FDM off\n"
	  );
#endif
