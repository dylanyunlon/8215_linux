#include "upgrade_rsd.h"
#include "x_typedef.h"
#include "bl_LcdShow.h"

#include <serial.h>
#include <nand.h>
#include <partition.h>

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

#define REPLICATION_NUMBER  8
static  char _szBLID1[12] = "BOOTLOADER!";
static char _szBLNFIID2[8] = "NFIINFO";
static char _szBLMSDCID2[8] = "MT3360A";	// Fixed Header String. Don't change it to "AC8317"
static int copyupgrade_imgcheck_flag = 0;

#define CHUNK_HEADER_LEN (sizeof(chunk_header_t))
#define SPARSE_HEADER_LEN (sizeof(sparse_header_t))

extern unsigned long long _totalsize ;
extern unsigned long long _currentsize ;


static char *uitostr_hex(char *str,unsigned int u4)
{
   sprintf(str,"%x",u4);
   return str;
}

static char *uitostr(char *str,unsigned int u4)
{
   sprintf(str,"%u",u4);
   return str;
}

static  int switch_bootmode(struct mmc *emmc_dev)
{
	int err = 0;
	err = mmc_boot_enter_bootmode(emmc_dev->host_id);
	return err;
}

static int switch_normalmode(struct mmc *emmc_dev)
{
	int err = 0;
	err = mmc_boot_exit_bootmode(emmc_dev->host_id);
	return err;
}

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

static void create_bootloader_header(char *pBLHeader,  char* blbuf,UINT32 u4Imagesize,int msdc_boot)
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

static int write_bootpart(struct mmc *emmc_dev, char * buffer,int writeblknum)
{
	int err = 0;
	int boot_part_num = 0;

	err = switch_bootmode(emmc_dev);
	if (err)
	{
		printf("emmc switch to boot mode failed, err = %d\r\n", err);
		return -1;
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
	}
	printf("------------------->>> End to write boot partition <<<---------------------\r\n");

	err = switch_normalmode(emmc_dev);
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

#define IMAGE_NUMBER 32
#define NAND_IMAGE_NUMBER 32
typedef struct image_upgrade_header{
	char partition_name[32];
	char image_name[40];
	int image_exist_flag;
}image_upgrade_header_t;

#ifdef ATC_AB_PARTITION_SUPPORT
char *partition_name[] = {"preloader","uboot_a","trustzone_a","arm2_a","kernel_a","dtb_a",
			"recovery_a", "uboot_b","trustzone_b","arm2_b","kernel_b","dtb_b","recovery_b","logo","boot_misc","vba",
			"system_a","system_b","data","metazone","nvstore", "usrdata"};

char *image_name[] = {"83XX_Preloader_realchip_sd.bin","u-boot.bin","tz.bin","arm2.bin","Image.bin","ac83xx.dtb.bin",
			"recovery.gz","u-boot.bin","tz.bin","arm2.bin","Image.bin","ac83xx.dtb.bin","recovery.gz","cluster_res.img","boot_misc.bin","vba.bin",
			"system.img.ext4","system.img.ext4","data.img.ext4","metazone.bin","nvstore.bin","usrdata.img.ext4"};

char *nandpartition_name[] = {"preloader","preloader_bk","uboot_a","uboot_b",
				"trustzone_a","trustzone_b","arm2_a","arm2_b","dtb_a","dtb_b",
				"logo","boot_misc","vba","metazone","kernel_a","kernel_b","system_a","system_b","usrdata"};

char *nandimage_name[] = {"83XX_Preloader_realchip_nand.bin","83XX_Preloader_realchip_nand.bin",
			"u-boot.bin","u-boot.bin","tz.bin","tz.bin","arm2.bin","arm2.bin","ac83xx.dtb.bin","ac83xx.dtb.bin",
			"cluster_res.img","boot_misc.bin","vba.bin","metazone.bin","Image.bin","Image.bin",
			"system.img.ext4","system.img.ext4","data.img.ext4"};
#else
char *partition_name[] = {"preloader","uboot","trustzone","arm2","kernel","dtb","recovery","uboot_bk",
		"trustzone_bk","arm2_bk","kernel_bk","dtb_bk","recovery_bk","logo","boot_misc","vba","system","systembk","data","metazone","nvstore","usrdata"};
char *image_name[] = {"83XX_Preloader_realchip_sd.bin","u-boot.bin","tz.bin","arm2.bin","Image.bin",
		"ac83xx.dtb.bin","recovery.gz","u-boot.bin","tz.bin","arm2.bin","Image.bin","ac83xx.dtb.bin","recovery.gz","cluster_res.img",
		"boot_misc.bin","vba.bin","system.img.ext4","system.img.ext4","data.img.ext4","metazone.bin", "nvstore.bin", "usrdata.img.ext4"};

char *nandpartition_name[] = {"preloader","preloader_bk","uboot","trustzone","arm2","dtb","logo",
                              "boot_misc","vba","metazone","kernel","system","usrdata"};
char *nandimage_name[] = {"83XX_Preloader_realchip_nand.bin","83XX_Preloader_realchip_nand.bin",
                          "u-boot.bin","tz.bin","arm2.bin","ac83xx.dtb.bin","cluster_res.img","boot_misc.bin","vba.bin","metazone.bin",
                          "Image.bin","system.img.ext4","data.img.ext4"};
#endif
image_upgrade_header_t file_exist_flag[IMAGE_NUMBER] = {0};
image_upgrade_header_t nandfile_exist_flag[NAND_IMAGE_NUMBER]= {0};

static int check_file_exist()
{
	int ret = -1;
	int i = 0;
	int cnt = sizeof(partition_name) / sizeof(partition_name[0]);
	for(i = 0; i < cnt; i++){
		strcpy(file_exist_flag[i].partition_name, partition_name[i]);
		strcpy(file_exist_flag[i].image_name, image_name[i]);
		file_exist_flag[i].image_exist_flag = 0;
	}
	for(i = 1; i < cnt; i++){
		if (fat_exists(file_exist_flag[i].image_name) == 1)	{
			file_exist_flag[i].image_exist_flag = 1;
			ret = 0;
		}
	}

	return ret;
}

static int check_nandfile_exist(void)
{
	int ret = -1;
	int i = 0;
	int cnt = sizeof(nandpartition_name) / sizeof(nandpartition_name[0]);
	for(i = 0; i < cnt; i++){
		strcpy(nandfile_exist_flag[i].partition_name, nandpartition_name[i]);
		strcpy(nandfile_exist_flag[i].image_name, nandimage_name[i]);
		nandfile_exist_flag[i].image_exist_flag = 0;
	}
	for(i = 0; i < cnt; i++){
		if (fat_exists(nandfile_exist_flag[i].image_name) == 1)	{
			nandfile_exist_flag[i].image_exist_flag = 1;
			ret = 0;
		}
        printf("[%d]:  partname(%s)===imagename(%s)===existflag(%d)\r\n",
               i,
               nandfile_exist_flag[i].partition_name,
               nandfile_exist_flag[i].image_name,
               nandfile_exist_flag[i].image_exist_flag);
	}

	return ret;
}

extern int do_usb(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#ifdef NAND_UPG_RDBACK_CHECK
static unsigned int checksum32(unsigned int chksum, char *buf,
	    unsigned int len)
{
	char *end;

	for (end = buf + len; buf < end; ++buf)
		chksum += *buf;

	return chksum;
}

unsigned int calc_checksum_before_upg(char *buf,
	    unsigned long long real_size)
{
	return checksum32(0, buf, real_size);
}

unsigned int calc_checksum_after_upg(unsigned long long offset,
	    unsigned long long real_size, unsigned long long part_size,
	    int mode)
{
/*FILE_RW_SIZE must be n * nand_block */
#define FILE_RW_SIZE (8 * 1024 * 1024)
	unsigned long long sizer = 0;
	char *buffer = NULL;
	unsigned int read_len = 0;
	unsigned int chksum = 0;
	unsigned long long (*nand_read_fn)(void *, unsigned long long, unsigned long long) = NULL;
	int ret = 0;
	int result = 0;

	if (real_size <= 0) {
		printf("realdata_size is 0.\n");
		return 0;
	}

	if (mode)
		nand_read_fn = nand_read_ext4_image;
	else
		nand_read_fn = nand_read_raw_image;

	buffer = (char *)NAND_WRITE_BASE_ADDR;

	ret = nand_rw_start();
	if (ret < 0) {
		printf("nand_rw_start fail.\n");
		return 0;
	}

	while (real_size > 0) {
		if (real_size >= FILE_RW_SIZE) {
			sizer = nand_read_fn(buffer, offset, FILE_RW_SIZE);
			read_len = FILE_RW_SIZE;
		} else {
			sizer = nand_read_fn(buffer, offset, real_size);
			read_len = real_size;
		}

		if (sizer != read_len) {
			printf("nand read fail.\n");
			result = -1;
			break;
		}

		chksum = checksum32(chksum, buffer, read_len);
		real_size -= read_len;
		offset += read_len;
	}

	ret = nand_rw_end();
	if (ret < 0) {
		printf("nand_rw_end fail.\n");
		return 0;
	}

	if (result < 0)
		return result;

	printf("succeeded, chksum(0x%x)!\n", chksum);

	return chksum;
}

#endif /* NAND_UPG_RDBACK_CHECK */

int upg_rsd_erase_partition(char* szPartitionName, unsigned long long u8Addr, unsigned long long u8Size)
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
int upg_rsd_check_udisk_available()
{
	char *argv[5] = {"usb","start"};
	int b_is_udisk_available = 0;

	if (do_usb(NULL, 0, 5, argv) == 0)
		b_is_udisk_available = 1;

	printf("\ncheck_udisk_available: %d\n", b_is_udisk_available);

	return b_is_udisk_available;
}

int upg_rsd_check_udisk_exist()
{
	char *argv[5] = {"usb","exist"};
	int b_is_udisk_exist = 0;

	if (do_usb(NULL, 0, 5, argv) == 0)
		b_is_udisk_exist = 1;

	printf("\ncheck_udisk_exist: %d\n", b_is_udisk_exist);

	return b_is_udisk_exist;
}

int upg_rsd_check_ext_sdcard_available(int* pdev_num)
{
	int b_is_ext_sdcard_available = 0;
	int dev_num;
	struct mmc* dev;
    block_dev_desc_t *stor_dev;

	for(dev_num = 2;dev_num > 0 ; dev_num--)
	{
		dev = find_mmc_device(dev_num);
		mmc_init(dev);

		if (dev->card_type != 0)
		{
			b_is_ext_sdcard_available = 1;
			break;
		}
	}

    if (b_is_ext_sdcard_available == 1)
    {
        stor_dev = get_dev("mmc", dev_num);
        if (!stor_dev)
        {
            printf("unknow device type!\r\n");
            b_is_ext_sdcard_available = 0;
        }
        else
        {
            if (fat_register_device(stor_dev, 0) != 0)
            {
                printf("Unable to use mmc %d:%d for fatls\r\n");
                b_is_ext_sdcard_available = 0;
            }
        }

    }

	*pdev_num = dev_num;

	printf("\ncheck_ext_sdcard_available: %d\n", b_is_ext_sdcard_available);

	return b_is_ext_sdcard_available;
}

int upg_rsd_read_raw_image(char* filename, char* dev_type, int dev_part, unsigned long long u8Addr, unsigned long long u8Size)
{
	int ret = 0;
	char *szDevPart = (char *)malloc(32);
	char *szValAddr = (char *)malloc(32);
	char* szValSize = (char *)malloc(32);

	char *argv[6] = {"fatload",dev_type, uitostr_hex(szDevPart, dev_part), uitostr_hex(szValAddr,(unsigned int)u8Addr), filename, uitostr_hex(szValSize,(unsigned int)u8Size) };

	if (do_fat_fsload(NULL, 0, 6, argv) == 0)
	{
		printf("\nread_raw_image_from_rsd %s successfully!\n", filename);
		ret = 0;
	}
	else
	{
		printf("\nread_raw_image_from_rsd %s failed!\n", filename);
		ret = -1;
	}

	free(szDevPart);
	free(szValAddr);
	free(szValSize);
	return ret;
}

int upg_rsd_write_raw_nandimage(char* partname, unsigned long long u8ReadAddr, unsigned long long u8WriteAddr, unsigned long long u8Size)
{
	nand_info_t *nand = &nand_info[nand_curr_device];
	int i = 0;
#ifdef NAND_UPG_RDBACK_CHECK
	unsigned int chksum_before = 0, chksum_after = 0;
	unsigned long long total_sz = 0;
#endif

	if ((0 == strcmp(partname, "preloader")) ||
        (0 == strcmp(partname, "preloader_bk")))
	{
		create_bootloader_header((char *)(BASE_ADDR-512),BASE_ADDR,0x7000,0); //nand boot

		#ifdef NAND_UPG_RDBACK_CHECK
		if(copyupgrade_imgcheck_flag) {
			total_sz = u8Size > 64 * 1024 ? 64 * 1024 : u8Size;
			chksum_before = calc_checksum_before_upg((char *)(BASE_ADDR - 512), total_sz);
		}
		#endif /* NAND_UPG_RDBACK_CHECK */

		u8Size = ALIGN((u8Size+512), nand->writesize);
		if(write_nand_ex((uchar*)(BASE_ADDR-512), (ulong)u8Size, partname, 0, "raw", 0))
			return -1;

		_currentsize += u8Size;
        LCD_DrawProcessNumber((int)(_currentsize*100/_totalsize));

		#ifdef NAND_UPG_RDBACK_CHECK
		if(copyupgrade_imgcheck_flag) {
			chksum_after = calc_checksum_after_upg(u8WriteAddr, total_sz, total_sz, 0);

			if (chksum_before != chksum_after) {
				printf("Error: %s chksum compare fail, chksum_before(0x%x), chksum_after(0x%x).\n",
					    partname, chksum_before, chksum_after);
			} else if (chksum_before == 0 && chksum_after == 0) {
				printf("Error: %s chksum compare fail, chksum are both 0.\n", partname);
			} else {
				printf(" %s chksum compare success, chksum_before(0x%x), chksum_after(0x%x).\n",
					    partname, chksum_before, chksum_after);
			}
		}
		#endif /* NAND_UPG_RDBACK_CHECK */
	}
	else
	{
		if (0 == strcmp(partname, "uboot"))
		{
			printf("\nUSB Write Partition will ignore last ten blocks!\n");
			u8Size = u8Size - 512*10;
		}

        u8Size = ALIGN(u8Size, nand->writesize);
        flush_cache((ulong)BASE_ADDR, u8Size);
        if(write_nand_ex((uchar *)BASE_ADDR, (ulong)u8Size, partname, 0,"raw",0))
            return -1;

		_currentsize += u8Size;
	}

	LCD_DrawProcessNumber((int)(_currentsize*100/_totalsize));

	return 0;
}
int upg_rsd_write_raw_image(char* filename, unsigned long long u8ReadAddr, unsigned long long u8WriteAddr, unsigned long long u8Size)
{
	int ret = 0;
	int blknum = 0;
	int n = 0, i = 0;
	int writeblknum = 0;
	struct mmc *emmc_dev = NULL;
	int emmc_dev_num = 0;
	int end = 0;

	char *szReadAddr = (char *)malloc(32);
	char *szWriteAddr = (char *)malloc(32);
	char* szValSize = (char *)malloc(32);

	emmc_dev = find_mmc_device(emmc_dev_num);
	n = mmc_init(emmc_dev);

	if (strcmp(filename, "83XX_Preloader_realchip_sd.bin") == 0)
	{
		int preloadersize = PRELOADER_SIZE;
		struct mmc *emmc_dev = NULL;
		int emmc_dev_num = 0;

		emmc_dev = find_mmc_device(emmc_dev_num);
		if (mmc_init(emmc_dev))
		{
			free(szReadAddr);
			free(szWriteAddr);
			free(szValSize);
			return -1;
		}

		create_bootloader_header((char * )(u8ReadAddr-512), u8ReadAddr,PRELOADER_SIZE, 1);/*msdc boot*/

		// Write to boot partition
		//writeblknum = 129; //PRELOADER_MAX_SIZE(include dramk) + header size = 64KB   + 512Bytes = 129 Blocks
		write_bootpart(emmc_dev, (char *)(u8ReadAddr - 512), 129);

		// Write to user partition
		emmc_dev->block_dev.block_write(emmc_dev_num, 0, (unsigned int)(u8Size/512), (char *)(u8ReadAddr - 512));
		_currentsize += u8Size;
		ret = 0;
	}
	else
	{
		if (strcmp(filename, "u-boot.bin") == 0)
		{
			printf("\nUSB Write Partition will ignore last ten blocks!\n");
			u8Size = u8Size - 512*10;
		}
		#if 0
		char *argv_write[6] = {"mmc","write","0", uitostr_hex(szReadAddr,u8ReadAddr), uitostr_hex(szWriteAddr, (unsigned int)(u8WriteAddr/512)), uitostr_hex(szValSize,(unsigned int)(u8Size/512))};

		if(0 != do_mmcops(NULL, 0, 6, argv_write))
		{
			printf("\nUSB Write Partition Failed!\n");
		}
		else
		{
			printf("\nUSB Write Partition Successfully!\n");
		}
		#endif
		emmc_dev->block_dev.block_write(0, (unsigned int)(u8WriteAddr/512), (unsigned int)(u8Size/512), (void*)u8ReadAddr);
		_currentsize += u8Size;
	}
    printf("\nupg_rsd_write_raw_image %s DONE!\n", filename);

	LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));

	free(szReadAddr);
	free(szWriteAddr);
	free(szValSize);

	printf("\nupg_rsd_write_raw_image %s successfully!\n", filename);

	return ret;
}

int upg_rsd_read_ext4_image(char* filename, char* dev_type, int dev_part, unsigned long long u8Addr, unsigned long long u8Size)
{
	int ret = 0;
	char *szDevPart = (char *)malloc(32);
	char *szValAddr = (char *)malloc(32);
	char* szValSize = (char *)malloc(32);

	char *argv[6] = {"fatload",dev_type, uitostr_hex(szDevPart, dev_part), uitostr_hex(szValAddr,(unsigned int)u8Addr), filename, uitostr_hex(szValSize,(unsigned int)u8Size) };

	if (do_fat_fsload(NULL, 0, 6, argv) == 0)
	{
		printf("\nupg_rsd_read_ext4_image successfully!\n");
		ret = 0;
	}
	else
	{
		printf("\nupg_rsd_read_ext4_image failed!\n");
		ret = -1;
	}

	free(szDevPart);
	free(szValAddr);
	free(szValSize);
	return ret;
}

int upg_rsd_write_ext4_image(unsigned long long u8ReadAddr, unsigned long long u8WriteAddr, unsigned long long u8Size)
{
	int   ret = 0;
	uint32_t block_size = 0 ,n;
	uchar *pfileBuffer = NULL;
	int64_t filelen = u8Size;
	size_t datalen = 0;
	uint32_t chunk_cnt = 0;
	uint32_t dump_ext4_blockcnt = 10;
	uint32_t uLoop = 0;
	struct mmc *mmc = NULL;
	int emmc_dev_num = 0;

	sparse_header_t *ptSparseHeader = NULL;
	chunk_header_t  *ptChunkHeader = NULL;

	char *szValAddr1 = (char *)malloc(32);
	char *szValAddr2 = (char *)malloc(32);
	char* szPhyAddr  = (char *)malloc(32);

	mmc = find_mmc_device(emmc_dev_num);
	if (!mmc)
	{
		ret = -1;
		goto cleanup;
	}
	if (mmc_init(mmc))
	{
		ret = -1;
		goto cleanup;
	}

	ptSparseHeader = (sparse_header_t *)u8ReadAddr;
	block_size = ptSparseHeader->blk_sz;
	chunk_cnt = ptSparseHeader->total_chunks;

	printf ("upg_rsd_write_ext4_image:ptSparseHeader->blk_sz = 0X%x\r\n", ptSparseHeader->blk_sz);
	printf ("upg_rsd_write_ext4_image:ptSparseHeader->total_chunks = 0X%x\r\n", ptSparseHeader->total_chunks);
	printf ("upg_rsd_write_ext4_image:ptSparseHeader->total_blks = 0X%x\r\n", ptSparseHeader->total_blks);
	printf ("upg_rsd_write_ext4_image:filelen = 0X%x\r\n", filelen);
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

			_currentsize += datalen;
			LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));

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
			ret = -1;
		}
	}
	if (0 != chunk_cnt)
	{
	    printf ("\nupg_rsd_write_ext4_image:chunk_cnt error\r\n");
	    ret = -1;
	}

	//   printf("Write %x bytes\r\n", ptSparseHeader->total_blks * block_size);
cleanup:
	free(szValAddr1);
	free(szValAddr2);
	free(szPhyAddr);
	return ret;
}

int upg_rsd_read_partial_ext4_image(char* filename, char* dev_type, int dev_part, unsigned long long u8Addr, unsigned long long u8Pos, unsigned long long u8Size)
{
	int ret = 0;
	char *szDevPart = (char *)malloc(32);
	char *szValAddr = (char *)malloc(32);
	char* szValSize = (char *)malloc(32);
	char* szValPos  = (char *)malloc(32);

	char *argv[7] = {"fatloadat",dev_type, uitostr_hex(szDevPart, dev_part), uitostr_hex(szValAddr,(unsigned int)u8Addr), filename,
					uitostr_hex(szValPos,(unsigned int)u8Pos), uitostr_hex(szValSize,(unsigned int)u8Size) };

	if (do_fat_fsload_at(NULL, 0, 7, argv) == 0)
	{
		//printf("\nupg_rsd_read_partial_ext4_image successfully!\n");
		ret = 0;
	}
	else
	{
		printf("\nupg_rsd_read_partial_ext4_image failed!\n");
		ret = -1;
	}

	free(szDevPart);
	free(szValAddr);
	free(szValSize);
	free(szValPos);
	return ret;
}

int upg_rsd_write_partial_ext4_image(unsigned long long u8ReadAddr, unsigned long long u8WriteAddr, unsigned long long u8Size, uint32_t block_size, int chunk_cnt)
{
	int   ret = 0;
	uint32_t n;
	uchar *pfileBuffer = NULL;
	int64_t filelen = u8Size;
	size_t datalen = 0;
	uint32_t dump_ext4_blockcnt = 10;
	uint32_t uLoop = 0;
	struct mmc *mmc = NULL;
	int emmc_dev_num = 0;

	//sparse_header_t *ptSparseHeader = NULL;
	chunk_header_t	*ptChunkHeader = NULL;

	char *szValAddr1 = (char *)malloc(32);
	char *szValAddr2 = (char *)malloc(32);
	char* szPhyAddr  = (char *)malloc(32);

	mmc = find_mmc_device(emmc_dev_num);
	if (!mmc)
	{
		ret = -1;
		goto cleanup;
	}
	if (mmc_init(mmc))
	{
		ret = -1;
		goto cleanup;
	}

	//ptSparseHeader = (sparse_header_t *)u8ReadAddr;
	//block_size = ptSparseHeader->blk_sz;
	//chunk_cnt = ptSparseHeader->total_chunks;

	//printf ("upg_rsd_write_ext4_image:ptSparseHeader->blk_sz = 0X%x\r\n", ptSparseHeader->blk_sz);
	//printf ("upg_rsd_write_ext4_image:ptSparseHeader->total_chunks = 0X%x\r\n", ptSparseHeader->total_chunks);
	//printf ("upg_rsd_write_ext4_image:ptSparseHeader->total_blks = 0X%x\r\n", ptSparseHeader->total_blks);
	//printf ("upg_rsd_write_ext4_image:filelen = 0X%x\r\n", filelen);

	pfileBuffer = u8ReadAddr;

	while (chunk_cnt > 0)
	{
		ptChunkHeader = (chunk_header_t *)pfileBuffer;
		datalen = ptChunkHeader->chunk_sz * block_size;
		sprintf(szValAddr1,"%x",u8WriteAddr);
		//printf ("upg_rsd_write_ext4_image:ptChunkHeader->chunk_type = %d\r\n", ptChunkHeader->chunk_type);
		//printf ("upg_rsd_write_ext4_image:ptChunkHeader->chunk_sz = 0X%x\r\n", ptChunkHeader->chunk_sz);
		//printf ("upg_rsd_write_ext4_image:u8WriteAddr = 0x%s\r\n", szValAddr1);
		//printf ("upg_rsd_write_ext4_image:datalen = 0X%x\r\n", datalen);

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

			_currentsize += datalen;
			LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
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
			ret = -1;
		}
	}
	if (0 != chunk_cnt)
	{
		printf ("\nupg_rsd_write_ext4_image:chunk_cnt error\r\n");
		ret = -1;
	}

	//	 printf("Write %x bytes\r\n", ptSparseHeader->total_blks * block_size);

cleanup:
	free(szValAddr1);
	free(szValAddr2);
	free(szPhyAddr);
	return 0;
}

int upg_rsd_write_partial_ext4_image_in_chunk(unsigned long long u8ReadAddr, unsigned int* u8pWriteAddr, uint32_t block_size, int chunk_cnt)
{
	uint32_t n;
	uchar *pfileBuffer = NULL;
	size_t datalen = 0;
	uint32_t dump_ext4_blockcnt = 10;
	uint32_t uLoop = 0;
	static struct mmc *mmc = NULL;
	int emmc_dev_num = 0;
	chunk_header_t	*ptChunkHeader = NULL;

	if (NULL == mmc)
	{
		mmc = find_mmc_device(emmc_dev_num);
		if (!mmc)
			return -1;
		if (mmc_init(mmc))
			return -1;
		printf("mmc had inited.\n");
	}

	pfileBuffer = u8ReadAddr;
	while (chunk_cnt > 0)
	{
		ptChunkHeader = (chunk_header_t *)pfileBuffer;
		datalen = ptChunkHeader->chunk_sz * block_size;

		//printf("upg_rsd_write_ext4_image:chunk_cnt = %d\n", chunk_cnt);
		//printf ("upg_rsd_write_ext4_image:ptChunkHeader->chunk_type = %d\r\n", ptChunkHeader->chunk_type);
		//printf ("upg_rsd_write_ext4_image:ptChunkHeader->chunk_sz = 0X%x\r\n", ptChunkHeader->chunk_sz);
		//printf ("upg_rsd_write_ext4_image:u8WriteAddr = 0x%s\r\n", szValAddr1);
		//printf ("upg_rsd_write_ext4_image:datalen = 0X%x\r\n", datalen);

		if (CHUNK_TYPE_RAW == ptChunkHeader->chunk_type)
		{
			pfileBuffer += CHUNK_HEADER_LEN;
			mmc->block_dev.block_write(0, (unsigned int)(*u8pWriteAddr/512), (unsigned int)(datalen/512), (void*)pfileBuffer);
			_currentsize += datalen;
			LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
			chunk_cnt--;
			pfileBuffer += datalen;
			*u8pWriteAddr += datalen;
		}
		else if (CHUNK_TYPE_DONT_CARE == ptChunkHeader->chunk_type)
		{
			*u8pWriteAddr += datalen;
			pfileBuffer += ptChunkHeader->total_sz;
			chunk_cnt--;
		}
		else
		{
			printf ("<%s>:ptChunkHeader->chunk_type error\n", __FUNCTION__);
			return -1;
		}
	}

	return 0;
}


unsigned long long g_u8RealDataSize = 0;

void calc_total_size(partitionread *p)
{
	int i = 0;
	int cnt = sizeof(partition_name) / sizeof(partition_name[0]);
	_totalsize = 0;
	//preloader,uboot,datazone,trustzone,arm2,logo,kernel,dtb,recovery,metazone,dvp,system,app,appbk,systembk
	while(p){
		if(strcmp(p->szPartName, "preloader") == 0){
#if SUPPORT_BOOTLOADER_UPGRADE
			if (file_exist_flag[0].image_exist_flag == 1)
			{
				_totalsize += p->u8PartitionSize;
			}
#endif
			p = p->nextpartition;
			continue;
		}

		for(i = 1; i < cnt; i++){
			if(strcmp(p->szPartName, file_exist_flag[i].partition_name) == 0){
				if (file_exist_flag[i].image_exist_flag == 1)
				{
					_totalsize += p->u8PartitionSize;
				}
				break;
			}
		}
		p = p->nextpartition;
	}
	if(_totalsize > 0){
        _totalsize = _totalsize + (_totalsize * 2 / 100);    }

    printf("_totalsize = 0x%x %x\r\n", (u32)(_totalsize >> 32), (u32)_totalsize);
}

void calc_total_nandsize(partitionread *p)
{
	int i = 0;
	int cnt = sizeof(nandpartition_name) / sizeof(nandpartition_name[0]);
	_totalsize = 0;
	while(p)
    {
        if((strcmp(p->szPartName, "datazone") == 0) || (strcmp(p->szPartName, "datazone_bk") == 0)
            /*|| (strcmp(p->szPartName, "metazone") == 0)*/)
        {
            p = p->nextpartition;
            continue;
        }

		for(i = 0; i < cnt; i++)
        {
			if(strcmp(p->szPartName, nandfile_exist_flag[i].partition_name) == 0)
            {
				if (nandfile_exist_flag[i].image_exist_flag == 1)
				{
					_totalsize += p->u8PartitionSize;
				}
				break;
			}
		}
		p = p->nextpartition;
	}

    printf("NAND: Exist file totalsize = 0x%x %x\r\n", (u32)(_totalsize >> 32), (u32)_totalsize);

    return;
}

int upg_rsd_upgrade_raw_nandimage(partitionread *p, char *dev_type, int dev_num, int index)
{
	nand_info_t *nand = &nand_info[nand_curr_device];
	unsigned int tmp = _currentsize;
	char szShowString[60]={0};
	int iRet = 0;
	unsigned int realSize = 0;
#ifdef NAND_UPG_RDBACK_CHECK
	unsigned int chksum_before = 0, chksum_after = 0;
#endif

#ifdef READ_IMAGE_USE_REAL_SIZE
	g_u8RealDataSize = 0;     //use to store real image data size
#endif

	if (0 == upg_rsd_read_raw_image(nandfile_exist_flag[index].image_name,
				dev_type,
				dev_num,
				(unsigned long long)BASE_ADDR,
				p->u8PartitionSize))
	{
		memset(szShowString, 0, 60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   ÕýÔÚÏòÉÁ´æÖÐÐ´Èë¾µÏñ: ");
#else
		strcat(szShowString,"   writing image to flash: ");
#endif
		strcat(szShowString, p->szPartName);
		strcat(szShowString,"...\n");
		LCD_WriteString_FixedCharNum80(szShowString);

#ifdef NAND_UPG_RDBACK_CHECK
		if(copyupgrade_imgcheck_flag) {
			if (strcmp(p->szPartName, "preloader") && strcmp(p->szPartName, "preloader_bk")) {

				chksum_before = calc_checksum_before_upg((char *)BASE_ADDR, g_u8RealDataSize);
			}
		}
#endif /* NAND_UPG_RDBACK_CHECK */

		erase_nand(p->szPartName);

		if (!strcmp(p->szPartName, "metazone")) {
			realSize = ALIGN(p->u8RealDataSize, nand->writesize);
			printf("#### metazone write size is 0x%x ####\n", realSize);
		} else {
			realSize = p->u8PartitionSize;
		}

		iRet = upg_rsd_write_raw_nandimage(p->szPartName,
				(unsigned long long)BASE_ADDR,
				p->u8PartitionStartAddr,
				realSize);

#ifdef NAND_UPG_RDBACK_CHECK
		if(copyupgrade_imgcheck_flag) {
			if (strcmp(p->szPartName, "preloader") && strcmp(p->szPartName, "preloader_bk")) {

				chksum_after = calc_checksum_after_upg(p->u8PartitionStartAddr,
						g_u8RealDataSize, realSize, 0);

				if (chksum_before != chksum_after) {
					printf("Error: %s chksum compare fail, chksum_before(0x%x), chksum_after(0x%x).\n",
							p->szPartName, chksum_before, chksum_after);
				} else if (chksum_before == 0 && chksum_after == 0) {
					printf("Error: %s chksum compare fail, chksum are both 0.\n", p->szPartName);
				} else {
					printf(" %s chksum compare success, chksum_before(0x%x), chksum_after(0x%x).\n",
							p->szPartName, chksum_before, chksum_after);
				}
			}
		}
#endif /* NAND_UPG_RDBACK_CHECK */

#ifdef READ_IMAGE_USE_REAL_SIZE
		if((0 == strcmp(p->szPartName, "preloader")) || (0 == strcmp(p->szPartName, "preloader_bk")))
		{
			g_u8RealDataSize += 512;     //add header:512
		}
		//g_u8RealDataSize = g_u8RealDataSize%512==0 ? g_u8RealDataSize : (g_u8RealDataSize + 512 - g_u8RealDataSize%512);
		g_u8RealDataSize = ALIGN(g_u8RealDataSize, nand->writesize);

		p->u8RealDataSize = g_u8RealDataSize;
#endif
		_currentsize = tmp + p->u8PartitionSize;
	}
	else
	{
		printf("NAND: <%s>: Read image file from SD/USB failed.\r\n", __func__);
		no_sd_or_udisk_find_handle(dev_type, dev_num);
	}

	printf("ProcessNumber = %d\n", (signed int)((double)_currentsize*100/_totalsize));
	printf("NAND RAW: Upgrade <%s> partition end!\r\n", p->szPartName);
	LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));

	return iRet;
}
#if 1
int upg_rsd_upgrade_raw_image(partitionread *p, char *dev_type, int dev_num, int index)
{
	unsigned int tmp = _currentsize;
	char szShowString[60] = {0};
	unsigned long long filesize = 0;
	unsigned long long remaining_size = 0;
	unsigned long long offset_in_file = 0;
	unsigned long long offset_in_partition = 0;
	unsigned long long segment_size = 0;
	unsigned long long aligned_size = 0;
	unsigned long long write_addr = 0;
	int segment_index = 0;
	struct mmc *emmc_dev = find_mmc_device(0);
#define SEGMENT_SIZE (0x800000) // 8MB

#ifdef READ_IMAGE_USE_REAL_SIZE
	g_u8RealDataSize = 0;     //use to store real image data size
#endif

	if (fat_size(file_exist_flag[index].image_name, &filesize)) {
		printf("get image %s size failed\r\n", file_exist_flag[index].image_name);
		return -1;
	}

	if (filesize > p->u8PartitionSize) {
		printf("imagesize(0x%x%08x) larger than partition(%s) size(0x%x%08x) :%s\n", filesize, p->szPartName, p->u8PartitionSize);
		return -1;
	}

	remaining_size = filesize;
	printf("partition:%s filesize:0x%x%08x\n", p->szPartName, (u32)(filesize >> 32), (u32)filesize);

	memset(szShowString, 0, 60);
#if CONFIG_SUPPORT_CHAR == 1
	strcat(szShowString,"   ÕýÔÚÏòÉÁ´æÖÐÐ´Èë¾µÏñ: ");
#else
	strcat(szShowString,"   writing image to flash: ");
#endif
	strcat(szShowString, p->szPartName);
	strcat(szShowString,"...\n");
	LCD_WriteString_FixedCharNum80(szShowString);

	char *szDevPart = (char *)malloc(32);
	char *szValAddr = (char *)malloc(32);
	char *szValSize = (char *)malloc(32);
	char *szValPos = (char *)malloc(32);

	if (!szDevPart || !szValAddr || !szValSize || !szValPos) {
		printf("Memory allocation failed\n");
		if (szDevPart) free(szDevPart);
		if (szValAddr) free(szValAddr);
		if (szValSize) free(szValSize);
		if (szValPos) free(szValPos);
		return -1;
	}

	while (remaining_size > 0) {
		segment_size = (remaining_size > SEGMENT_SIZE) ? SEGMENT_SIZE : remaining_size;

		//printf("Reading segment %d: offset 0x%x%08x, size 0x%x%08x\n",
		//segment_index, (u32)(offset_in_file >> 32), (u32)offset_in_file, (u32)(segment_size >> 32), (u32)segment_size);

		printf("offset_in_partition is 0x%x%08x\n", (u32)(offset_in_partition >> 32), (u32)offset_in_partition);

		write_addr = p->u8PartitionStartAddr + offset_in_partition;
		aligned_size = ALIGN(segment_size, 512);
		memset((ulong)BASE_ADDR, 0x0, aligned_size);

		char *argv[7] = {"fatloadat", dev_type, uitostr_hex(szDevPart, dev_num),
			uitostr_hex(szValAddr, (unsigned int)BASE_ADDR),
			file_exist_flag[index].image_name,
			uitostr_hex(szValPos, (unsigned int)offset_in_file),
			uitostr_hex(szValSize, (unsigned int)segment_size)};

		if (do_fat_fsload_at(NULL, 0, 7, argv) != 0) {
			printf("\nread_raw_image_from_rsd %s failed at segment %d!\n",
					file_exist_flag[index].image_name, segment_index);
			free(szDevPart);
			free(szValAddr);
			free(szValSize);
			free(szValPos);
			return -1;
		}
		flush_cache((ulong)BASE_ADDR, (ulong)segment_size);

		//printf("remaining_size is 0x%x%08x\n", (u32)(remaining_size >> 32), (u32)remaining_size);
		printf("emmc write addr: 0x%x%08x, size: 0x%x%08x\n", (u32)(write_addr >> 32), (u32)write_addr,
				(u32)(aligned_size >> 32), (u32)aligned_size);

		if (strcmp(file_exist_flag[index].image_name, "83XX_Preloader_realchip_sd.bin") == 0) {
			if (segment_index == 0) {
				create_bootloader_header((char *)(BASE_ADDR-512), BASE_ADDR, PRELOADER_SIZE, 1);
				//write to emmc boot region
				write_bootpart(emmc_dev, (char *)(BASE_ADDR - 512), 129);
				//write to user region
				emmc_dev->block_dev.block_write(0, 0, (unsigned int)(aligned_size/512), (char *)(BASE_ADDR - 512));
			} else {
				//write to emmc user region
				emmc_dev->block_dev.block_write(0, (unsigned int)(write_addr/512),
						(unsigned int)(aligned_size/512), (void*)BASE_ADDR);
			}
		} else {
			emmc_dev->block_dev.block_write(0, (unsigned int)(write_addr/512),
					(unsigned int)(aligned_size/512), (void*)BASE_ADDR);
		}

		remaining_size -= segment_size;
		offset_in_file += segment_size;
		offset_in_partition += aligned_size;
		segment_index++;

		_currentsize += segment_size;
		LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
	}

	free(szDevPart);
	free(szValAddr);
	free(szValSize);
	free(szValPos);

#ifdef READ_IMAGE_USE_REAL_SIZE
	if(strcmp(p->szPartName, "preloader") == 0){
		g_u8RealDataSize += segment_size;
	}
	g_u8RealDataSize = g_u8RealDataSize%512==0 ? g_u8RealDataSize : (g_u8RealDataSize + 512 - g_u8RealDataSize%512);
	p->u8RealDataSize = g_u8RealDataSize;
#endif
	printf("\nupg_rsd_write_raw_image %s DONE!\n", file_exist_flag[index].image_name);
	_currentsize = tmp + p->u8PartitionSize;
	LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));

	return 0;
}
#else
int upg_rsd_upgrade_raw_image(partitionread *p, char *dev_type, int dev_num, int index)
{
	unsigned int tmp = _currentsize;
	char szShowString[60]={0};
#ifdef READ_IMAGE_USE_REAL_SIZE
	g_u8RealDataSize = 0;     //use to store real image data size
#endif
	if (upg_rsd_read_raw_image(file_exist_flag[index].image_name, dev_type, dev_num, BASE_ADDR, p->u8PartitionSize) == 0)
	{
		memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   ÕýÔÚÏòÉÁ´æÖÐÐ´Èë¾µÏñ: ");
#else
		strcat(szShowString,"   writing image to flash: ");
#endif
		strcat(szShowString, p->szPartName);
		strcat(szShowString,"...\n");
		LCD_WriteString_FixedCharNum80(szShowString);
		/*if(strcmp(p->szPartName, "uboot") != 0){
			upg_rsd_erase_partition(p->szPartName, p->u8PartitionStartAddr, p->u8PartitionSize);
		}*/
		upg_rsd_write_raw_image(file_exist_flag[index].image_name, BASE_ADDR, (unsigned int)p->u8PartitionStartAddr, (unsigned int)p->u8PartitionSize);

#ifdef READ_IMAGE_USE_REAL_SIZE
		if(strcmp(p->szPartName, "preloader") == 0){
			g_u8RealDataSize += 512;     //add header:512
		}
		g_u8RealDataSize = g_u8RealDataSize%512==0 ? g_u8RealDataSize : (g_u8RealDataSize + 512 - g_u8RealDataSize%512);
		p->u8RealDataSize = g_u8RealDataSize;
#endif
	}
	else
	{
		printf("<%s>: read partition[%s] image fail. @%s:line%d\n", __FUNCTION__, file_exist_flag[index].image_name, __FILE__, __LINE__);
		no_sd_or_udisk_find_handle(dev_type,dev_num);
		return -1;
	}
	_currentsize = tmp + p->u8PartitionSize;
	LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
	return 0;
}
#endif

int upg_rsd_upgrade_ext4_nandimage(partitionread *p, char *dev_type, int dev_num, int index)
{

    nand_info_t *nand = &nand_info[nand_curr_device];
    unsigned long long tmp = _currentsize;
    unsigned long long offset = 0;
    unsigned long long readlen = 0;
    unsigned long long filesize = 0;
    char szShowString[60] = {0};
    ulong entiresize = 0x800000;
    ulong size = 0;
    int i = 0;
    int iRet = 0;
    int end = 0;
    char partname[32] = {0};
#ifdef NAND_UPG_RDBACK_CHECK
    unsigned int chksum_before = 0, chksum_after = 0;
    unsigned long long total_sz = 0;
#endif

    memcpy(partname, p->szPartName, sizeof(p->szPartName));
    memset(szShowString, 0, 60);
#if CONFIG_SUPPORT_CHAR == 1
    strcat(szShowString,"   ÕýÔÚÏòÉÁ´æÖÐÐ´Èë¾µÏñ: ");
#else
    strcat(szShowString,"   writing image to flash: ");
#endif
    strcat(szShowString, partname);
    strcat(szShowString,"...\n");
    LCD_WriteString_FixedCharNum80(szShowString);

    if (fat_size(nandfile_exist_flag[index].image_name, &filesize)) {
        printf("get image %s size failed\r\n", nandfile_exist_flag[index].image_name);
        return -1;
    }
    printf("nand upgrade %s size is 0x%llx\r\n", nandfile_exist_flag[index].image_name, filesize);

    if (erase_nand(partname) != 0) {
        printf("nand erase partition %s failed.\r\n", partname);
        return -1;
    }

    readlen = ALIGN(filesize, nand->writesize);
    g_u8RealDataSize = readlen;
    for(; readlen>0; i++)
    {
        if(readlen > entiresize)
        {
            size = entiresize;
            end = 0;
        }
        else
        {
            size = readlen;
            end = 1;
        }

        if(0 != upg_rsd_read_partial_ext4_image(nandfile_exist_flag[index].image_name,
                                                dev_type,
                                                dev_num,
                                                (unsigned long long)NAND_WRITE_BASE_ADDR,
                                                offset,
                                                size))
        {
            iRet = -1;
            printf("NAND: Read partitial ext4 image failed!\r\n");
            no_sd_or_udisk_find_handle(dev_type, dev_num);
            break;
        }

	#ifdef NAND_UPG_RDBACK_CHECK
	if(copyupgrade_imgcheck_flag) {
		chksum_before += calc_checksum_before_upg((char *)NAND_WRITE_BASE_ADDR, size);
		total_sz += size;
	}
	#endif /* NAND_UPG_RDBACK_CHECK */

        flush_cache((ulong)NAND_WRITE_BASE_ADDR, ALIGN(size, nand->writesize));
        if(write_nand_ex((char *)NAND_WRITE_BASE_ADDR, size, partname, (ulong)offset, "ext4", end))
        {
            printf("NAND: Upgrade ext4 <write_nand_ex> failed.\r\n");
            return -1;
        }

        readlen-=size;
        tmp += size;
        offset += size;

        printf("ProcessNumber = %d\n", (int)(tmp*100/_totalsize));
        LCD_DrawProcessNumber((int)(tmp*100/_totalsize));
	}

    _currentsize = (iRet != 0)?tmp:(_currentsize + p->u8PartitionSize);

    printf("ProcessNumber = %d\n", (int)(_currentsize*100/_totalsize));
    LCD_DrawProcessNumber((int)(_currentsize*100/_totalsize));

    printf("NAND EXT4: Upgrade <%s> partition end!\r\n", partname);
#ifdef NAND_UPG_RDBACK_CHECK
	if(copyupgrade_imgcheck_flag) {
		//chksum_after = calc_checksum_after_upg(p->u8PartitionStartAddr,
			//g_u8RealDataSize, p->u8PartitionSize, 1);
		chksum_after = calc_checksum_after_upg(p->u8PartitionStartAddr,
			    total_sz, p->u8PartitionSize, 1);
		if (chksum_before != chksum_after) {
			printf("Error: %s chksum compare fail, chksum_before(0x%x), chksum_after(0x%x).\n",
				    partname, chksum_before, chksum_after);
		} else if (chksum_before == 0 && chksum_after == 0) {
			printf("Error: %s chksum compare fail, chksum are both 0.\n", partname);
		} else {
			printf(" %s chksum compare success, chksum_before(0x%x), chksum_after(0x%x).\n",
				    partname, chksum_before, chksum_after);
		}
	}
#endif /* NAND_UPG_RDBACK_CHECK */

    return iRet;
}
void upg_rsd_upgrade_ext4_image(partitionread *p, char *dev_type, int dev_num, int index){
	unsigned int tmp = _currentsize;
	char szShowString[60]={0};
	if (upg_rsd_read_ext4_image(file_exist_flag[index].image_name,
                                dev_type,
                                dev_num,
                                BASE_ADDR,
                                p->u8PartitionSize) == 0)
	{
		memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   ÕýÔÚÏòÉÁ´æÖÐÐ´Èë¾µÏñ: ");
#else
		strcat(szShowString,"   writing image to flash: ");
#endif
		strcat(szShowString, p->szPartName);
		strcat(szShowString,"...\n");
		LCD_WriteString_FixedCharNum80(szShowString);
		upg_rsd_erase_partition(p->szPartName, p->u8PartitionStartAddr, p->u8PartitionSize);
		upg_rsd_write_ext4_image(BASE_ADDR, (unsigned int)p->u8PartitionStartAddr, (unsigned int)p->u8PartitionSize);
	}
	_currentsize = tmp + p->u8PartitionSize;
	LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
}

void upg_rsd_upgrade_large_ext4_image(partitionread *p, char *dev_type, int dev_num, int index)
{
	unsigned int tmp = _currentsize;
	sparse_header_t *ptSparseHeader = NULL;
	chunk_header_t  *ptChunkHeader = NULL;
	write_result_t tWriteRes = {0};
	uint32_t block_size = 0;
	int32_t chunk_cnt = 0;
	size_t datalen = 0;
	uint32_t chunk_total_sz = 0;
	unsigned long long u8Pos = 0;
	unsigned long long u8WriteAddr = p->u8PartitionStartAddr;
	unsigned long long u8ReadAddr = 0;
	unsigned int chunk_type;
	int64_t filelen = (int64_t)p->u8PartitionSize;

	char *szValAddr1 = (char *)malloc(17);
	char *szValAddr2 = (char *)malloc(17);
	char* szPhyAddr  = (char *)malloc(17);

	if (upg_rsd_read_ext4_image(file_exist_flag[index].image_name, dev_type, dev_num, BASE_ADDR, (unsigned long)SPARSE_HEADER_LEN) == 0)
	{
		char szShowString[60]={0};
		uint32_t u8PartialSize =0;
		unsigned long long  u8PartialPos = 0;
		uint32_t u8PartialData = 0;
		int chunk_num = 0;
		int count_chunk = 100;

		ptSparseHeader = (sparse_header_t *)BASE_ADDR;
		block_size = ptSparseHeader->blk_sz;
		chunk_cnt = ptSparseHeader->total_chunks;
		u8Pos = u8Pos+ sizeof(sparse_header_t);

		memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   ÕýÔÚÏòÉÁ´æÖÐÐ´Èë¾µÏñ: ");
#else
		strcat(szShowString,"   writing image to flash: ");
#endif
		strcat(szShowString,p->szPartName);
		strcat(szShowString,"...\n");
		LCD_WriteString_FixedCharNum80(szShowString);
		upg_rsd_erase_partition(p->szPartName, p->u8PartitionStartAddr, p->u8PartitionSize);

    	if (chunk_cnt < count_chunk)
			goto rest;

		do
		{
			u8PartialPos = u8Pos;
			u8PartialData = 0;
			u8PartialSize= 0;
			for (chunk_num =0; chunk_num <count_chunk; chunk_num++)
			{
				upg_rsd_read_partial_ext4_image(file_exist_flag[index].image_name, dev_type, dev_num, BASE_ADDR, u8PartialPos,  (unsigned long)CHUNK_HEADER_LEN);
				ptChunkHeader = (chunk_header_t *)BASE_ADDR;
				u8PartialData = u8PartialData + block_size * ptChunkHeader->chunk_sz;
				u8PartialPos = u8PartialPos + ptChunkHeader->total_sz;
				u8PartialSize = u8PartialSize + ptChunkHeader->total_sz;
			}
			upg_rsd_read_partial_ext4_image(file_exist_flag[index].image_name, dev_type, dev_num, BASE_ADDR, u8Pos,  (unsigned long)u8PartialSize);
			u8Pos = u8Pos + u8PartialSize;
			upg_rsd_write_partial_ext4_image(BASE_ADDR, (unsigned int)u8WriteAddr, (unsigned int)u8PartialSize, block_size, count_chunk);

			chunk_cnt = chunk_cnt-count_chunk;
			u8WriteAddr = u8WriteAddr + u8PartialData;

			printf("\ndo_udisk_upgrade read chunk count %d!\n", chunk_cnt);
		}while (chunk_cnt > count_chunk);
rest:
		//read rest chunks
		u8PartialPos = u8Pos;
		u8PartialData = 0;
		u8PartialSize= 0;
		for (chunk_num =0; chunk_num <chunk_cnt; chunk_num++)
		{
			upg_rsd_read_partial_ext4_image(file_exist_flag[index].image_name, dev_type, dev_num, BASE_ADDR, u8PartialPos,  (unsigned long)CHUNK_HEADER_LEN);
			ptChunkHeader = (chunk_header_t *)BASE_ADDR;
			u8PartialData = ptChunkHeader->chunk_sz * block_size;
			u8PartialPos = u8PartialPos + ptChunkHeader->total_sz;
			u8PartialSize = u8PartialSize + ptChunkHeader->total_sz;
		}
		upg_rsd_read_partial_ext4_image(file_exist_flag[index].image_name, dev_type, dev_num, BASE_ADDR, u8Pos,  (unsigned long)u8PartialSize);
		upg_rsd_write_partial_ext4_image(BASE_ADDR, (unsigned int)u8WriteAddr, (unsigned int)u8PartialSize, block_size, chunk_cnt);
	}
	_currentsize = tmp + p->u8PartitionSize;
	LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
}

int upg_rsd_upgrade_ext4_image_in_partial (partitionread *p, char *dev_type, int dev_num, int index)
{
	#define EXT4_PARTIAL_READ_LENGTH 0x6400000
	unsigned long long u8PartialPos = 0;
	unsigned long long u8PartitionSize = p->u8PartitionSize;
	unsigned long long u8WriteAddrTemp = p->u8PartitionStartAddr;
	unsigned long long u8Pos = 0;
	sparse_header_t *ptSparseHeader = NULL;
	uint32_t block_size = 0;
	int32_t remainChunks = 0;
	int32_t totalChunks = 0;
	char szShowString[60]={0};

	if (0 == upg_rsd_read_ext4_image(file_exist_flag[index].image_name, dev_type, dev_num, BASE_ADDR, (unsigned long long)SPARSE_HEADER_LEN))
	{
		ptSparseHeader = (sparse_header_t *)BASE_ADDR;
		block_size = ptSparseHeader->blk_sz;
		totalChunks = ptSparseHeader->total_chunks;
		remainChunks = ptSparseHeader->total_chunks;
		u8PartialPos = u8Pos + sizeof(sparse_header_t);

		printf("%s's total_chunks=%d, block_size = 0x%x\n", file_exist_flag[index].image_name, ptSparseHeader->total_chunks, block_size);
		printf("the read offset is 0x%x%08x\n", (u32)(u8PartialPos >> 32), (u32)(u8PartialPos & 0xffffffff));
		printf("the remain upgrade chunk count is %d\n", remainChunks);
		//printf("u8PartialPos = 0x%x%08x, u8PartitionSize = 0x%x%08x\n", (u32)(u8PartialPos >> 32), (u32)(u8PartialPos & 0xffffffff), (u32)(u8PartitionSize >> 32), (u32)(u8PartitionSize & 0xffffffff));
		//printf("blk_sz=0x%x, total_blks=%d, chunk_hdr_sz=0x%x, file_hdr_sz=0x%x\n", ptSparseHeader->blk_sz, ptSparseHeader->total_blks, ptSparseHeader->chunk_hdr_sz, ptSparseHeader->file_hdr_sz);

		memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"	  ÕýÔÚÏòÉÁ´æÖÐÐ´Èë¾µÏñ: ");
#else
		strcat(szShowString,"	  writing image to flash: ");
#endif
		strcat(szShowString,p->szPartName);
		strcat(szShowString,"\n");
		LCD_WriteString_FixedCharNum80(szShowString);
		upg_rsd_erase_partition(p->szPartName, p->u8PartitionStartAddr, p->u8PartitionSize);

		while (remainChunks > 0)
		{
			if (0 == upg_rsd_read_partial_ext4_image(file_exist_flag[index].image_name, dev_type, dev_num, BASE_ADDR, u8PartialPos, EXT4_PARTIAL_READ_LENGTH))
			{
				int count_chunk = 0;
				chunk_header_t *chunk_header_previous = NULL;
				chunk_header_t *chunk_header_current = (chunk_header_t *)BASE_ADDR;
				unsigned long long lastChunkPosInMem = (unsigned long long)BASE_ADDR;
				unsigned long long partialLimit = (unsigned long long)(BASE_ADDR + EXT4_PARTIAL_READ_LENGTH);
				unsigned long long current_chunk_total_sz = chunk_header_current->total_sz;

				printf ("chunk_header_current->chunk_type = %d\r\n", chunk_header_current->chunk_type);
				printf("current_chunk_total_sz = 0x%x%08x\n", (u32)(current_chunk_total_sz >> 32), (u32)(current_chunk_total_sz & 0xffffffff));

				if (current_chunk_total_sz > EXT4_PARTIAL_READ_LENGTH)//if the chunk datalen is larger than EXT4_PARTIAL_READ_LENGTH
				{
					printf("current_chunk_total_sz(0x%x%08x) is larger than 0x%x%08x\n", (u32)(current_chunk_total_sz >> 32), (u32)(current_chunk_total_sz & 0xffffffff), (u32)(EXT4_PARTIAL_READ_LENGTH >> 32), (u32)(EXT4_PARTIAL_READ_LENGTH & 0xffffffff));
					unsigned long long remain_chunk_datalen = 0;
					static struct mmc *mmc = NULL;

					if (NULL == mmc)
					{
						mmc = find_mmc_device(dev_num);
						if (!mmc)
							return -1;
						if (mmc_init(mmc))
							return -1;
					}
					mmc->block_dev.block_write(0, (u8WriteAddrTemp/512), ((EXT4_PARTIAL_READ_LENGTH-512)/512), (void*)(BASE_ADDR+12));
					u8WriteAddrTemp = u8WriteAddrTemp + EXT4_PARTIAL_READ_LENGTH -512;
					u8PartialPos = u8PartialPos + sizeof(chunk_header_t) + EXT4_PARTIAL_READ_LENGTH - 512;
					remain_chunk_datalen = current_chunk_total_sz - sizeof(chunk_header_t) - (EXT4_PARTIAL_READ_LENGTH - 512);
					_currentsize = _currentsize + EXT4_PARTIAL_READ_LENGTH - 512;
					LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
					while (remain_chunk_datalen > EXT4_PARTIAL_READ_LENGTH)
					{
						if (0 == upg_rsd_read_partial_ext4_image(file_exist_flag[index].image_name, dev_type, dev_num, BASE_ADDR, u8PartialPos, EXT4_PARTIAL_READ_LENGTH))
						{
							mmc->block_dev.block_write(0, (u8WriteAddrTemp/512), (EXT4_PARTIAL_READ_LENGTH/512), (void*)BASE_ADDR);
							u8WriteAddrTemp += EXT4_PARTIAL_READ_LENGTH;
							u8PartialPos += EXT4_PARTIAL_READ_LENGTH;
							remain_chunk_datalen -= EXT4_PARTIAL_READ_LENGTH;
							_currentsize += EXT4_PARTIAL_READ_LENGTH;
							LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
						}
						else
						{
							printf("<%s>: read partition[%s] fail. @%s:line%d\n", __FUNCTION__, file_exist_flag[index].image_name, __FILE__, __LINE__);
							no_sd_or_udisk_find_handle(dev_type,dev_num);
							return -1;
						}
					}
					if (0 == upg_rsd_read_partial_ext4_image(file_exist_flag[index].image_name, dev_type, dev_num, BASE_ADDR, u8PartialPos, remain_chunk_datalen))
					{
						mmc->block_dev.block_write(0, (u8WriteAddrTemp/512), (remain_chunk_datalen/512), (void*)BASE_ADDR);
						u8WriteAddrTemp += remain_chunk_datalen;
						u8PartialPos += remain_chunk_datalen;
						remain_chunk_datalen -= remain_chunk_datalen;
						_currentsize += remain_chunk_datalen;
						LCD_DrawProcessNumber((signed int)((double)_currentsize*100/_totalsize));
						printf("remain_chunk_datalen = 0x%x%08x\n", (u32)(remain_chunk_datalen >> 32), (u32)(remain_chunk_datalen & 0xffffffff));
					}
					else
					{
						printf("<%s>: read partition[%s] fail. @%s:line%d\n", __FUNCTION__, file_exist_flag[index].image_name, __FILE__, __LINE__);
						no_sd_or_udisk_find_handle(dev_type,dev_num);
						return -1;
					}
					remainChunks--;
				}
				else//if the chunk datalen is less than EXT4_PARTIAL_READ_LENGTH
				{
					while(lastChunkPosInMem < partialLimit)
					{
						chunk_header_previous = chunk_header_current;
						lastChunkPosInMem = lastChunkPosInMem + chunk_header_current->total_sz;
						chunk_header_current = (chunk_header_t *)lastChunkPosInMem;
						count_chunk++;
						if (count_chunk == remainChunks)
							break;
					}
					if (lastChunkPosInMem > partialLimit)
						count_chunk--;

					unsigned long long tempStartAddr = (unsigned long long)BASE_ADDR;
					unsigned long long tempEndAddr = (unsigned long long)chunk_header_previous;
					unsigned long long tempWriteLen = tempEndAddr - tempStartAddr;

					if (0 == upg_rsd_write_partial_ext4_image_in_chunk(BASE_ADDR, &u8WriteAddrTemp, block_size, count_chunk))
					{
						printf("write partial ext4 image success!\n");
					}
					else
					{
						printf("<%s>: write partition[%s] partial fail. @%s:line%d\n", __FUNCTION__, file_exist_flag[index].image_name, __FILE__, __LINE__);
						return -1;
					}

					u8PartialPos = u8PartialPos + tempWriteLen;
					remainChunks = remainChunks - count_chunk;
					printf("the remain upgrade chunk count is %d\n", remainChunks);
				}
			}
			else
			{
				printf("<%s>: read partition[%s] partial fail. @%s:line%d\n", __FUNCTION__, file_exist_flag[index].image_name, __FILE__, __LINE__);
				no_sd_or_udisk_find_handle(dev_type,dev_num);
				return -1;
			}
		}
	}
	else
	{
		printf("<%s>: read partition[%s] header fail. @%s:line%d\n", __FUNCTION__, file_exist_flag[index].image_name, __FILE__, __LINE__);
		no_sd_or_udisk_find_handle(dev_type,dev_num);
		return -1;
	}

	return 0;
}

static int update_partition_info(partitionhead *phead)
{
	struct mmc *emmc_dev = NULL;
	int res;
	int partition_info_is_write_protect = 0;
	extern partitionhead *g_partitionhead;

	emmc_dev = find_mmc_device(0);
	if ( NULL == emmc_dev){
		printf("update_partition_info:can't find emmc device\n");
		return -1;
	}
	res = mmc_init(emmc_dev);
	if (res) {
		printf("update_partition_info:init emmc device fail\n");
		return -1;
	}

	/*
	* write partition info main
	*/
#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
	if (DATAZONE_END/512 > emmc_dev->wp_size) {
		printf("update_partition_info:[WP] warning: partition table is not in the first write protect group\n");
	}

	if(1 == emmc_wpg_type(DATAZONE_END/512/emmc_dev->wp_size)) {
		partition_info_is_write_protect = 1;
		if(emmc_set_user_wp(WP_DISABLE, 0, DATAZONE_END/512, 1))
			printf("update_partition_info:[WP] clear wp fail in %s\n", __func__);
		else
			printf("update_partition_info:[WP] clear wp success in %s\n", __func__);
	}
#endif

	res = writepartitioninfotoflash2(g_partitionhead, PARTITION_INFO_MAIN_OFFSET_FROM_MMCBLK);
	if (res < 0) {
		printf("update_partition_info:write main partition info fail.\n");
	}

	res = writepartitioninfotoflash2(g_partitionhead, PARTITION_INFO_BK_OFFSET_FROM_MMCBLK);
	if (res < 0) {
		printf("update_partition_info:write bk partition info fail.\n");
	}

#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
	if(partition_info_is_write_protect) {
		if(emmc_set_user_wp(WP_ENABLE, 0,  DATAZONE_END/512, 1))
			printf("update_partition_info:[WP] restore wp fail in %s\n", __func__);
		else {
			partition_info_is_write_protect = 0;
			printf("update_partition_info:[WP] restore wp success in %s\n", __func__);
		}
	}
#endif

	printf("update_partition_info: update partiotion info successfully.\n");

	return 0;
}

static int do_extsdcard_or_udisk_upgrade(char *dev_type, int dev_num)
{
	char matchfilename[256]= {0};
	char szShowString[60]={0};
	int i = 0;
	partitionread *ppartitionread,*p;
	int res;
	int cnt = sizeof(partition_name) / sizeof(partition_name[0]);

#ifdef READ_IMAGE_USE_REAL_SIZE

	int error = readpartitioninfofromflash_ext();

	if (error == -1)
	{
		printf("get partition info fail\r\n");
		memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   ...²»ÄÜ´ÓemmcÖÐ»ñÈ¡·ÖÇøÐÅÏ¢!!!");
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

	extern partitionread *g_partitionread;
	p = g_partitionread;

#else //use partition size for image size

	ppartitionread = readpartitioninfofromflash();
	p = ppartitionread;

#endif

	if (check_file_exist() == -1)
	{
		memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
		strcat(szShowString,"   ...Éè±¸ÖÐÃ»ÓÐ¾µÏñ!!!");
#else
		strcat(szShowString,"   ...can't find image from device!!!");
#endif
		strcat(szShowString,"...\n");
	    LCD_WriteString_FixedCharNum80(szShowString);
		LCD_FreeStringBuf();
		return -1;
	}

	LCD_DrawProcess();
	LCD_DrawProcessNumber(0);

	calc_total_size(p);

	_currentsize = 0;

#ifdef READ_IMAGE_USE_REAL_SIZE
	p = g_partitionread;
#else
	p = ppartitionread;
#endif

	while(p)
	{
		if(strcmp(p->szPartName, "preloader") == 0){
			p = p->nextpartition;
			continue;
		}
		for(i = 1; i < cnt; i++){
			if(strcmp(p->szPartName, file_exist_flag[i].partition_name) == 0)
				break;
		}
		if(file_exist_flag[i].image_exist_flag != 1){
			p = p->nextpartition;
			continue;
		}
#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
		if((strcmp(p->szType, "raw") == 0) || (strcmp(p->szType, "ext4") == 0)){
			if(isEnable(WRITE_PROTECT_ENABLE, p->u4Flag)) {
				if(emmc_set_user_wp(WP_DISABLE, p->u8PartitionStartAddr/512, p->u8PartitionSize/512, 1))
					printf("[WP]clear write protect fail(part: %s)\n", p->szPartName);
				else
					printf("[WP]clear write protect success(part: %s)\n", p->szPartName);
			}
		}

#endif

#ifdef NEW_PARTITION_DESIGN
		u32 flag;
		getpartitionbypartitionname(&flag, p->szPartName);
		if(isEnable(COPY_UPGRADE_EABLE, flag) == DISABLE)
		{
			printf("Partition[%s] donot be allowed copy upgraded!!!\n", p->szPartName);
			memset(szShowString,0,60);
			strcat(szShowString,"   ");
			strcat(szShowString, p->szPartName);
#if CONFIG_SUPPORT_CHAR == 1
			strcat(szShowString,"·ÖÇø²»ÔÊÐíÉý¼¶!!!\n");
#else
			strcat(szShowString," donot be allowed copy upgraded!!!\n");
#endif
			LCD_WriteString_FixedCharNum80(szShowString);
			p = p->nextpartition;
			continue;
		}else{
			printf("Partition[%s] will be copy upgraded\n", p->szPartName);
		}
#endif
		if(strcmp(p->szType, "raw") == 0){
			if (0 != upg_rsd_upgrade_raw_image(p, dev_type, dev_num, i))
				return -1;
		}else if(strcmp(p->szType, "ext4") == 0){
			if (0 != upg_rsd_upgrade_ext4_image_in_partial(p, dev_type, dev_num, i))
				return -1;
		}else{
			printf("Uboot upgrade donot support the format of this image %s\n", file_exist_flag[i].image_name);
		}

#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
		if((strcmp(p->szType, "raw") == 0) || (strcmp(p->szType, "ext4") == 0)){if(isEnable(WRITE_PROTECT_ENABLE, p->u4Flag)) {
				if(emmc_set_user_wp(WP_ENABLE, p->u8PartitionStartAddr/512, p->u8PartitionSize/512, 1))
					printf("[WP]set write protect fail(part: %s)\n", p->szPartName);
				else
					printf("[WP]set write protect success(part: %s)\n", p->szPartName);
			}
		}
#endif
		p = p->nextpartition;
	}

#ifndef CONFIG_BOOT_MMC
#ifdef READ_IMAGE_USE_REAL_SIZE
		extern partitionhead *g_partitionhead;
		update_partition_info(g_partitionhead);
#endif
#endif

	memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
	strcat(szShowString,"   ...ÏµÍ³Éý¼¶³É¹¦");
#else
	strcat(szShowString,"   ...upgrade Linux Image Success");
#endif
	strcat(szShowString,"...\n");
	//LCD_CleanScreen_part();
	//LCD_WriteString(szShowString);
	LCD_WriteString_FixedCharNum80(szShowString);
	LCD_DrawProcessNumber(100);
	show_string_on_LCD("                ","            ");
	show_string_on_LCD("                ","            ");
	LCD_FreeStringBuf();

	return 0;
}


void show_string_on_LCD(char* showChStr, char* showEnStr)
{
	char str[60] = {0};

	memset(str,0,60);
#if CONFIG_SUPPORT_CHAR == 1
	strcat(str,showChStr);
#else
	strcat(str,showEnStr);
#endif
	LCD_WriteString_FixedCharNum80(str);
}

#ifndef EMMC_UPGRADE
int nand_rsd_check_partition(partitionread *part)
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

static int do_extsdcard_or_udisk_nand_upgrade(char *dev_type, int dev_num)
{
	extern partitionread *g_partitionread;
	extern partitionhead *g_partitionhead;
	partitionread *ppartitionread,*p;
	char szShowString[60]={0};
    int  iRet = 0;
	int  i = 0;
	int cnt = sizeof(nandpartition_name) / sizeof(nandpartition_name[0]);

#ifndef CONFIG_BOOT_MMC
#define TEST_FILE "copyupgrade_imgcheck_test.txt"
	int ret = fat_exists(TEST_FILE);
	if (!ret) {
		printf("<%s> %s is not exist.\n", __func__, TEST_FILE);
	} else {
		printf("<%s> %s exist.\n", __func__, TEST_FILE);
		copyupgrade_imgcheck_flag = 1;
	}

#endif


#ifdef READ_IMAGE_USE_REAL_SIZE

	int error = readpartitioninfofromnand_ext();

	if (error == -1)
	{
		printf("<%s >get partition info fail\r\n", __func__);
		show_string_on_LCD("   ...²»ÄÜ´ÓemmcÖÐ»ñÈ¡·ÖÇøÐÅÏ¢!!!\n",
		                   "   ...can't get partition info from emmc!!!\n");
		return -1;
	}

	ppartitionread = g_partitionread;

#else //use partition size for image size
	ppartitionread = readpartitioninfofromnand();
#endif
    p = ppartitionread;

	if (check_nandfile_exist() == -1)
	{
        printf("%s cannot find upgrade file.\r\n", __func__);
		show_string_on_LCD("   ...Éè±¸ÖÐÃ»ÓÐ¾µÏñ!!!\n",
		                   "   ...can't find image from device!!!\n");
		return -1;
	}

	LCD_DrawProcess();
	LCD_DrawProcessNumber(0);

	calc_total_nandsize(p);

	p = ppartitionread;

    while(p && !iRet)
	{
        if((strcmp(p->szPartName, "datazone") == 0) || (strcmp(p->szPartName, "datazone_bk") == 0)
           /*|| (strcmp(p->szPartName, "metazone") == 0)*/)
        {
            /*iRet = upg_rsd_upgrade_nand_dtz(p, dev_type, dev_num, i);*/
            printf("NAND: Do not need to upgrade %s.\r\n", p->szPartName);
            p = p->nextpartition;
            continue;
        }

		for(i = 0; i < cnt; i++){
			if(strcmp(p->szPartName, nandfile_exist_flag[i].partition_name) == 0)
				break;
		}
		if(nandfile_exist_flag[i].image_exist_flag != 1){
			p = p->nextpartition;
			continue;
		}

		dumppartitionread(p);
		if(nand_rsd_check_partition(p)){
			return -1;
		}

		if(strcmp(p->szType, "raw") == 0){
			iRet = upg_rsd_upgrade_raw_nandimage(p, dev_type, dev_num, i);
		}else if(strcmp(p->szType, "ext4") == 0){
			iRet = upg_rsd_upgrade_ext4_nandimage(p, dev_type, dev_num, i);
		}else{
			printf("Uboot upgrade donot support the format of this image %s\n", nandfile_exist_flag[i].image_name);
		}
		if (iRet < 0) {
			printf("NAND upgrade %s failed!\r\n", p->szPartName);
			return -1;
		}

		p = p->nextpartition;
	}

	printf("#### NAND: upgrade finished ####\r\n");
#ifdef READ_IMAGE_USE_REAL_SIZE
	iRet += writepartitioninfotonand(g_partitionhead);
#endif

	memset(szShowString,0,60);
#if CONFIG_SUPPORT_CHAR == 1
	if(iRet)
	{
		strcat(szShowString,"   ...ÏµÍ³Éý¼¶Ê§°Ü");
	}
	else
	{
		strcat(szShowString,"   ...ÏµÍ³Éý¼¶³É¹¦");
	}
#else
	if(iRet)
	{
		strcat(szShowString,"   ...upgrade Linux Image Failed");
	}
	else
	{
		strcat(szShowString,"   ...upgrade Linux Image Success");
	}
#endif
	strcat(szShowString,"...\n");
	LCD_WriteString_FixedCharNum80(szShowString);
	LCD_DrawProcessNumber(100);
	show_string_on_LCD("                \n","            \n");
	show_string_on_LCD("                \n","            \n");
	LCD_FreeStringBuf();
	return 0;
}

int do_rsd_upgrade()
{
	char szShowString[60]={0};
    int  flagUdiskAvail = 0;
    int  errUdiskUpg = 0;
    int  dev_num = 0;
	Sleep(100);
	LCD_CleanScreen();
	LCD_MallocStringBuf();

	printf("looking for SD card or U disk\r\n");
	show_string_on_LCD("   ...ÕýÔÚÕÒÉý¼¶USBÉè±¸...\n",
	                   "   ...looking for U disk!!!\n");

	if(upg_rsd_check_udisk_available() == 1)
	{
		printf("<%s> find USB device\n", __func__);
        flagUdiskAvail = 1;
#ifdef CONFIG_BOOT_MMC
        errUdiskUpg = do_extsdcard_or_udisk_upgrade("usb", 0);
#else
        errUdiskUpg = do_extsdcard_or_udisk_nand_upgrade("usb", 0);
#endif
        if(0 == errUdiskUpg)
        {
            return 0;
        }
		else
		{
			show_string_on_LCD("   ...ÏµÍ³Éý¼¶Ê§°Ü...","   ...Upgrade Linux Image Fail...");
			return -1;
		}
    }

	show_string_on_LCD("   ...ÕýÔÚÕÒÉý¼¶Éè±¸SD¿¨...\n",
	                   "   ...looking for SD card!!!\n");

    if(upg_rsd_check_ext_sdcard_available(&dev_num) == 1)
    {
        printf("<%s> find ext sdcard device, device num = %d\n", __func__, dev_num);
#ifdef CONFIG_BOOT_MMC
        if (dev_num != 0)
        {
            return do_extsdcard_or_udisk_upgrade("mmc", dev_num);
        }
#else
        return do_extsdcard_or_udisk_nand_upgrade("mmc", dev_num);
#endif
    }

    if(0 == flagUdiskAvail)
    {
        printf("can not find SD card or U disk device\r\n");
		show_string_on_LCD("   ...ÕÒ²»µ½¿ÉÓÃµÄÉý¼¶Éè±¸!!!\n",
		                   "   ...canot find SD card or U disk device!!!\n");
        LCD_FreeStringBuf();
        return -1;
    }

    return 0;
}

/***************************************************/
U_BOOT_CMD(
	copy_upgrade,	1,	1,	do_rsd_upgrade,
	"enter uboot udisk/extcard copy upgrade",
);
