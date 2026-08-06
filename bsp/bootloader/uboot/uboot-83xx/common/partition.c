
#include <partition.h>
#include <config.h>

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <timestamp.h>
#include <version.h>
#include <net.h>
#include <serial.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <mmc.h>
#if CONFIG_EXTERNAL_MCU
#include <ext_mcu.h>
#endif

extern char *g_mtdparts;
extern char *g_partitionstr;
extern unsigned long long g_miscPartitonAddr;
extern unsigned long long g_arm2PartitonAddr;
extern unsigned long long g_arm2PartitonSize;

extern unsigned long long g_tzPartitonAddr;
extern unsigned long long g_tzPartitonSize;

extern unsigned long long g_dtbPartitonAddr;
extern unsigned long long g_dtbPartitonSize;

extern unsigned long long g_bootmiscPartitonAddr;
extern unsigned long long g_bootmiscPartitonSize;

extern unsigned long long g_logoPartitonAddr;
extern unsigned long long g_logoPartitonSize;

extern unsigned long long g_metazonePartitonAddr;
extern unsigned long long g_metazonePartitonSize;

extern unsigned long long g_dvpPartitonAddr;
extern unsigned long long g_dvpPartitonSize;

extern unsigned int g_systemIndex;
extern char g_bootcmd[100];
extern char g_bootrecovercmd[100];
extern unsigned long long g_envpartitionoffset;
// return partition info with partition table head
// it will modify g_partitionhead and g_partitionread variable
partitionhead *g_partitionhead = NULL;
partitionread *g_partitionread = NULL;

#ifdef CONFIG_SECURITY_UPGRADE
unsigned int g_u4PartionAddress = 0x12000;
#else
unsigned int g_u4PartionAddress = 0x400000;;
#endif

extern void flush_invalid_cache(unsigned int start, unsigned int size);
extern void flush_cache(unsigned int start, unsigned int size);
char *uitostr_hex(char *str,unsigned int u4)
{
	sprintf(str,"%08x",u4);
	return str;
}

char *uitostr(char *str,unsigned int u4)
{
	sprintf(str,"%u",u4);
	return str;
}


unsigned int u64_to_u32(unsigned long long  u64,unsigned int* uhigh,unsigned int* ulow)
{
	*ulow = (unsigned int)u64;
	*uhigh = (unsigned int)(u64 >> 32);

	return 0;
}

char *uitostr_u64(char *str,unsigned long long u8)
{
	sprintf(str,"%ullX",u8);
	return str;
}

#ifdef NEW_PARTITION_DESIGN

unsigned int isEnable(enum Part_Attr attribute, unsigned int flag)
{
	unsigned int temp = 0;
	struct partitionflag *pflag = (struct partitionflag *)(&flag);
	switch(attribute)
	{
		case UPGRADE_ENABLE:
			temp = pflag->upgradable;
			break;
		case ERASE_ENABLE:
			temp = pflag->eraseable;
			break;
		case FASTBOOT_ENABLE:
			temp = pflag->fastbootable;
			break;
		case COPY_UPGRADE_EABLE:
			temp = pflag->copyupgradable;
			break;
		case WRITE_PROTECT_ENABLE:
			temp = pflag->write_protect;
			break;
		case MOUNT_ENABLE:
			temp = pflag->mountable;
			break;
		default:
			printf("ERR: Unknown Partition Attribute[%d]!!!\r\n", (unsigned int)(attribute));
			break;
	}
	if (temp)
		return ENABLE;
	else
		return DISABLE;
}

void dump_partition_flag(unsigned int flag)
{
	printf("Partition Flag: 0x%x  ", flag);
	struct partitionflag *pflag = (struct partitionflag *)(&flag);
	printf("[%d%d %d%d%d%d] ",
			pflag->mountable,
			pflag->write_protect,
			pflag->copyupgradable,
			pflag->fastbootable,
			pflag->eraseable,
			pflag->upgradable
	      );

	printf("[%d%d %d%d%d%d]\r\n",
			isEnable(MOUNT_ENABLE, flag),
			isEnable(WRITE_PROTECT_ENABLE, flag),
			isEnable(COPY_UPGRADE_EABLE, flag),
			isEnable(FASTBOOT_ENABLE, flag),
			isEnable(ERASE_ENABLE, flag),
			isEnable(UPGRADE_ENABLE, flag)
	      );

#if 0
	printf("Test error PARTITION ATTRIBUTE\r\n");
	isEnable(9, flag);
#endif
}

void dumppartitionread(partitionread *part)
{
	unsigned int vallow,valhigh;
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
	//char      szPartName[20];
	printf("partition name:%s\r\n",part->szPartName);
	//char      szType[20];
	printf("partition type:%s\r\n",part->szType);
	//unsigned int u4Mount;
	printf("partition mount:%d\r\n",part->u4Mount);
	//unsigned int  u4PartitionStartAddr;
	//unsigned long long  u8PartitionStartAddr;
	u64_to_u32(part->u8PartitionStartAddr,&valhigh,&vallow);
	printf("partition start addr: 0x%X%08X\r\n", valhigh, vallow);
	//unsigned int   u4PartitionSize;
	//unsigned long long  u8PartitionSize;
	u64_to_u32(part->u8PartitionSize,&valhigh,&vallow);
	printf("partition Size: 0x%X%08X\r\n", valhigh, vallow);
	//unsigned int  u4LastPartition;
	printf("partition last partition:%d\r\n",part->u4LastPartition);
	//char  szImageFileName[48];   //40 --> 48
	printf("partition Image Name:%s\r\n",part->szImageFileName);
	//unsigned long long u8RealDataSize;
	u64_to_u32(part->u8RealDataSize,&valhigh,&vallow);
	printf("Real Image Size: 0x%X%08X\r\n", valhigh, vallow);
	//unsigned int u4Flag;         //add u4Flag
	printf("partition flag:0x%X\r\n",part->u4Flag);
	dump_partition_flag(part->u4Flag);
	//struct partitionread *nextpartition;
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
}

#endif



void dumppartitioninfo(partitioninfo *part)
{
	unsigned int vallow,valhigh;
	printf("----------------------------------------------------------------\r\n");
	printf("partition name:%s\r\n",part->szPartName);
	printf("partition type:%s\r\n",part->szType);
	u64_to_u32(part->u8PartitionStartAddr,&valhigh,&vallow);
	printf("partition Start address:0x%X%08X\r\n",valhigh,vallow);
	//printf("partition Start address:%lld\r\n",part->u8PartitionStartAddr);
	//printf("partition Start address:%llu\r\n",part->u8PartitionStartAddr);
	//printf("partition Start address:0x%llX\r\n",part->u8PartitionStartAddr);
	//printf("partition Start address:0x%X\r\n",part->u8PartitionStartAddr);
	//printf("partition Start address:0x%X%08X\r\n",part->u8PartitionStartAddr,(unsigned int)(part->u8PartitionStartAddr));
	//printf("partition Start address hh:0x%16X\r\n",(part->u8PartitionStartAddr));
	u64_to_u32(part->u8PartitionSize,&valhigh,&vallow);
	printf("partition Size:0x%X%08X\r\n",valhigh,vallow);
#ifdef NEW_PARTITION_DESIGN
	printf("Partition Image Name: %s\r\n", part->szImageFileName);
#endif
	//printf("offset data:0x%X\r\n",part->u4OffsetData);
	u64_to_u32(part->u8OffsetData,&valhigh,&vallow);
	printf("offset data:0x%X%08X\r\n",valhigh,vallow);
	//printf("Real Data size:0x%X\r\n",part->u4RealDataSize);
	//printf("next image offset:0x%X\r\n",part->u4OffsetNextImage);
	u64_to_u32(part->u8RealDataSize,&valhigh,&vallow);
	printf("Real Data size:0x%X%08X\r\n",valhigh,vallow);
#ifdef NEW_PARTITION_DESIGN
	printf("u4Flag:0x%X\r\n",part->u4Flag);
	dump_partition_flag(part->u4Flag);
#endif
	u64_to_u32(part->u8OffsetNextImage,&valhigh,&vallow);
	printf("next image offset:0x%X%08X\r\n",valhigh,vallow);
	printf("----------------------------------------------------------------\r\n");

	return;
}


partitionread *mergepartitioninfo(partitioninfo *part)
{
	partitionread *ppartition;

	ppartition = (partitionread *)malloc(sizeof(partitionread));
	if (ppartition == NULL)
	{
		printf("<mergepartitioninfo> malloc failed\r\n");
		return NULL;
	}
	//printf("mergepartitioninfo szPartName:%s\r\n",part->szPartName);
	strcpy(ppartition->szPartName,part->szPartName);
	strcpy(ppartition->szType,part->szType);
	strcpy(ppartition->szImageFileName, part->szImageFileName);
	ppartition->u4Mount = part->u4Mount;
	ppartition->u8PartitionSize = part->u8PartitionSize;
	ppartition->u8RealDataSize = part->u8RealDataSize;
	ppartition->u8PartitionStartAddr = part->u8PartitionStartAddr;
#ifdef NEW_PARTITION_DESIGN
	ppartition->u4Flag= part->u4Flag;
#endif

	return ppartition;
}

void freetblmemory(partitionread *ptbl)
{
	partitionread *ptemp = NULL;

	while (ptbl) {
		ptemp = ptbl;
		ptbl = ptbl->nextpartition;
		free(ptemp);
	}
	ptemp = NULL;

	return;
}

#ifdef CONFIG_SECURITY_UPGRADE
uint32_t checksum32 (uint32_t chksum, char *buf, int len)
{
	char *end;

	for (end = buf + len; buf < end; ++buf)
		chksum += *buf;
	return chksum;
}

void put_checksum32(char *dest, uint32_t chksum)
{
	if (dest == NULL) {
		printf("dest is NULL.\n");
		return;
	}

	//TODO, confirm how to checksum.
	memcpy(dest, &chksum, 4);
}

/**
 * get_checksum32() -get checksum from @src
 *
 * get 4 bytes data from@src starting, translate them
 * to 32bit checksum.
 */
uint32_t get_checksum32(char *src)
{
	uint32_t chksum = 0;

	if (src == NULL) {
		printf("src is NULL.\n");
		return 0;
	}

	//TODO, confirm how to checksum.
	memcpy(&chksum, src, 4);

	return chksum;
}

unsigned int lookup_partition_by_name(partitionread *ptbl, partitionread *tmp, const char*name)
{
	partitionread * current = ptbl;
	if (current == NULL || name == NULL){
		printf("ptbl or name is NULL!\n");
		return -1;
	}

	while (current){
		if(strcmp(current->szPartName, name)==0){
			printf("found %s.\n", name);
			memcpy((void *)tmp, (void *)current, sizeof(partitionread));
			break;
		}
		else
			current=current->nextpartition;
	}

	return 0;
}

/*
 * write_partition_info() -write partition info to flash.
 * /----------------------/
 * /                                            /
 * / partitionhead(512Bytes)      /
 * /                                            /
 * /----------------------/
 * /                                            /
 * / partitionread                        /
 * /                                            /
 * /----------------------/
 */

int writepartitioninfotoflash2(partitionhead *parthead, unsigned long offset)
{

	struct mmc *emmc_dev = NULL;
	int emmc_dev_num = 0;
	char  *buf,*p;
	int n,err = 0;
	uint32_t chksum = 0;
	partitionread *pcurpart;
	printf("parthead->blockcnt:%x\n", parthead->blockcnt);
	buf = (char *)malloc(parthead->blockcnt * 512);

	parthead->u4Version = ATC_PARTITION_VER;
	parthead->u4Signature = ATC_PTBL_SIGN;


	//printf("---------------writepartitioninfotoflash2 start  addr:0x%X-----------------\r\n",g_u4PartionAddress);


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

	memset(buf, 0, parthead->blockcnt * PTBL_BLOCK_SIZE);
	pcurpart= parthead->nextpartition;
	p = buf;

	//printf("readpartitioninfofromflash block_read test parthead->blockcnt=%d\r\n",parthead->blockcnt);
	while(1)
	{
		if (pcurpart == NULL)
			break;

		//printf("<writepartitioninfotoflash> szPartName:%s,nextpartition:0x%X \r\n",pcurpart->szPartName,pcurpart->nextpartition);
		memcpy(p,pcurpart,sizeof(partitionread));
		chksum += checksum32(0, (char *)p, sizeof(partitionread));
		p += sizeof(partitionread);
		//ptmppart = pcurpart;
		pcurpart= pcurpart->nextpartition;
		//free(ptmppart);
	}

	printf("parthead->blockcnt:%d, checksum: 0x%x, start: 0x%x\n", parthead->blockcnt, chksum, offset);
	put_checksum32(parthead->checksum, chksum);

	flush_cache(buf, parthead->blockcnt * 512);
	n = emmc_dev->block_dev.block_write(emmc_dev_num, (unsigned long)(offset/512 + 1) , parthead->blockcnt, (char *)buf);
	if (n == 0)
	{
		printf("<writepartitioninfotoflash2> block_write failed \r\n");
	}

	memset(buf, 0, PTBL_BLOCK_SIZE);
	memcpy(buf,parthead,sizeof(partitionhead));

	flush_cache(buf, 512);
	n = emmc_dev->block_dev.block_write(emmc_dev_num, (unsigned long)(offset/512), 1, (char *)buf);
	if (n != 1)
	{
		printf("<writepartitioninfotoflash2> block_write part head failed = %d\r\n", n);
		return -1;
	}
	//printf("<writepartitioninfotoflash2> block_write part head success\r\n");

	printf("---------------writepartitioninfotoflash2 end-------------------\r\n");

	return 0;
}

/*
 * when read partition info from emmc, invoke this function to adjust
 * nextpartition field of partitionread.
 *
 * Note, this function should only be invoked after partion info read from emmc.
 */

uint32_t get_partition_info_checksum(struct partitionhead *phead)
{
	return get_checksum32(phead->checksum);
}

uint32_t calc_partition_info_checksum(partitionread *ptbl)
{
	uint32_t chksum = 0;

	while (ptbl->u4LastPartition == 0) {
		//TODO, comfirm how to calc partition table checksum.
		chksum += checksum32(0, (char *)ptbl, sizeof(partitionread));
		ptbl++;
	}

	chksum += checksum32(0, (char *)ptbl, sizeof(partitionread));

	return chksum;
}

/*
 * check partitioninfo crc
 * if tag and crc check pass, return 1, otherwise 0.
 */
int check_partition_info_checksum(struct partitionhead *phead, partitionread *ptbl)
{
	uint32_t chksum_calc = 0;
	uint32_t chksum_pi = 0;

	chksum_calc = calc_partition_info_checksum(ptbl);
	chksum_pi = get_partition_info_checksum(phead);
	printf("chksum_calc=%d,chksum_pi=%d.\n",chksum_calc, chksum_pi);
	if (chksum_calc != chksum_pi) {
		printf("partition info checksum check fail.\n");
		return 0;
	}

	printf("partition info checksum check pass.\n");
	return 1;
}
partitionread * readpartitioninfofromflash();

int partition_info_readback_check(unsigned long offset)
{
	struct partitionhead rb_parthead;
	partitionread *rb_ptbl = NULL;


	rb_ptbl = readpartitioninfofromflash();

	if (rb_ptbl == NULL) {
		printf("read_partition_info fail.\n");
		return 0;
	}

	if (check_partition_info_checksum(&rb_parthead, rb_ptbl) == 0) {
		printf("check_partiton_info_checksum fail.\n");
		free(rb_ptbl);
		return 0;
	}

	free(rb_ptbl);
	return 1;
}

unsigned long read_common_area(unsigned long offset, void *pbuf, int len)
{
	unsigned long ret = 0;
	if (pbuf == NULL) {
		printf("pbuf is NULL.\n");
		return -1;
	}
	struct mmc *bootdev;
	int devnum = -1;
#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
	devnum = 0;
#elif (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT2)
	devnum = 2;
#endif

	bootdev = find_mmc_device(devnum);
	if ( NULL == bootdev){
		printf("can't find emmc device\n");
		return 1;
	}
	ret = mmc_init(bootdev);
	if(ret){
		printf("init emmc device fail\n");
		return -1;
	}
	len = (len % 512 == 0) ? (len / 512) : (len / 512 + 1);
	flush_invalid_cache(pbuf, len * 512);
	ret = bootdev->block_dev.block_read(devnum, (unsigned int)(offset/512), len, (void *)pbuf);
	if (ret != len)
	{
		printf("Read Info Failed!\n");
	}

	return 0;
}

int write_common_area(unsigned long offset, void *pbuf, int len)
{
	unsigned long ret = 0;
	if (pbuf == NULL) {
		printf("pbuf is NULL.\n");
		return -1;
	}
	struct mmc *bootdev;
	int devnum = -1;
#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
	devnum = 0;
#elif (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT2)
	devnum = 2;
#endif

	bootdev = find_mmc_device(devnum);
	if ( NULL == bootdev){
		printf("can't find emmc device\n");
		return 1;
	}
	ret = mmc_init(bootdev);
	if(ret){
		printf("init emmc device fail\n");
		return -1;
	}

	len = (len % 512 == 0) ? (len / 512) : (len / 512 + 1);
	printf("write_common_area %x, %x, %x\n", offset, offset/512, len);
	flush_cache(pbuf, len * 512);
	ret = bootdev->block_dev.block_write(devnum, (unsigned int)(offset/512), len, (void *)pbuf);
	if (ret != len)
	{
		printf("Write Info Failed!\n");
	}

	return 0;
}

/*
 * read datazone from emmc
 *
 * return 0 if read success, otherwise -1.
 */
int read_datazone(struct datazone_info*pdz, unsigned long long offset)
{
	return read_common_area((unsigned long)offset, pdz, sizeof(struct datazone_info));
}

int write_datazone(struct datazone_info *pdz, unsigned long long offset)
{
	return write_common_area((unsigned long)offset, pdz, sizeof(struct datazone_info));
}

uint32_t get_datazone_checksum(struct datazone_info *pdz)
{
	return get_checksum32(pdz->checksum);
}

uint32_t calc_datazone_checksum(struct datazone_info *pdz)
{
	/*
	 * calc checksum of datazone_info, but NOT include checksum[4].
	 */
	return checksum32(0, (char *)pdz, DATAZONE_INFO_DZ_LEN);
}

void put_datazone_checksum(struct datazone_info *pdz, uint32_t chksum)
{
	put_checksum32(pdz->checksum, chksum);
}

/*
 * check datazone checksum
 * if tag and crc check pass, return 1, otherwise 0.
 */
int check_datazone_checksum(struct datazone_info *pdz)
{
	uint32_t chksum_calc = 0;
	uint32_t chksum_dz = 0;

	chksum_calc = calc_datazone_checksum(pdz);
	chksum_dz = get_datazone_checksum(pdz);

	if (chksum_calc != chksum_dz) {
		printf("datazone checksum check fail.\n");
		return 0;
	}

	printf("checksum check pass.\n");
	return 1;
}

/*
 * when datazone update done, read bcb back from emmc check
 * to make sure update is correct.
 *
 * return 1 if check pass, otherwise 0.
 */
int datazone_readback_check(unsigned long offset)
{
	struct datazone_info rb_dz;
	int res = 0;

	memset(&rb_dz, 0, sizeof(struct datazone_info));
	res = read_datazone(&rb_dz, offset);
	if (res < 0) {
		printf("read_datazone fail.\n");
		return 0;
	}

	return check_datazone_checksum(&rb_dz);
}

/**
 * adjust_datazone_img_desc_bk()
 * according to datazone image_desc field and uboot_bk table entry to adjust
 * datazone image_desc_bk field.
 */
void adjust_datazone_img_desc_bk(struct datazone_info *pdz, partitionread *ptbl_ub_bk)
{
	//TODO, confirm how to adjust img_desc_bk ???
	memcpy(&pdz->img_desc_bk, &pdz->img_desc, sizeof(struct image_desc));
	pdz->img_desc_bk.dwStartAddr = ptbl_ub_bk->u8PartitionStartAddr;
	pdz->img_desc_bk.dwTtlLen = ptbl_ub_bk->u8PartitionSize;
}

int read_bcb(struct bootloader_message *pbcb, unsigned long long offset)
{
	return read_common_area((unsigned long)offset, pbcb, sizeof(struct bootloader_message));
}

int write_bcb(struct bootloader_message *pbcb, unsigned long long offset)
{
	return write_common_area((unsigned long)offset, pbcb, sizeof(struct bootloader_message));
}

uint32_t get_bcb_checksum(struct bootloader_message *pbcb)
{
	return get_checksum32(pbcb->checksum);
}

uint32_t calc_bcb_checksum(struct bootloader_message *pbcb)
{
	/*
	 * calc checksum of bcb, but NOT include tages[16] and checksum[4].
	 */
	return checksum32(0, (char *)pbcb + 16 + 4, sizeof(struct bootloader_message) - (16 + 4));
}

void put_bcb_checksum(struct bootloader_message *pbcb, uint32_t chksum)
{
	put_checksum32(pbcb->checksum, chksum);
}

/*
 * check bcb checksum
 * if tag and checksum check pass, return 1, otherwise 0.
 */
int check_bcb_tag_checksum(struct bootloader_message *pbcb)
{
	uint32_t chksum_calc = 0;
	uint32_t chksum_bcb = 0;

	if (strncmp(pbcb->tags, BCB_TAG, strlen(BCB_TAG))) {
		printf("bcb.tags:%s\n", pbcb->tags);
		printf("bcb tag check fail.\n");
		return 0;
	}

	chksum_calc = calc_bcb_checksum(pbcb);
	chksum_bcb = get_bcb_checksum(pbcb);
	printf("chksum_calc=%d, chksum_bcb=%d\n", chksum_calc, chksum_bcb);
	if (chksum_calc != chksum_bcb) {
		printf("bcb checksum check fail. chksum_calc(0x%x), chksum_bcb(0x%x)\n", chksum_calc, chksum_bcb);
		return 0;
	}
	printf("bcb tag and checksum check pass.\n");
	return 1;
}

/*
 * when bcb update done, read bcb back from emmc check
 * to make sure update is correct.
 *
 */
int bcb_readback_check(unsigned long long offset)
{
	struct bootloader_message rb_bcb;
	int ret = 0;

	memset(&rb_bcb, 0, sizeof(struct bootloader_message));
	ret = read_bcb(&rb_bcb, offset);
	if (ret < 0) {
		printf("read_bcb fail.\n");
		return 0;
	}

	return check_bcb_tag_checksum(&rb_bcb);
}

uint32_t get_bcb_bootflag(struct bootloader_message *pbcb)
{
	return pbcb->bootflag;
}

void put_bcb_bootflag(struct bootloader_message *pbcb, uint32_t bootflag)
{
	pbcb->bootflag = bootflag;
}

void set_bcb_tags(struct bootloader_message *pbcb)
{
	strncpy(pbcb->tags, BCB_TAG, 16);
}

void print_bcb_tags(struct bootloader_message *pbcb)
{
	int i;

	printf("bcb tags:");
	for (i = 0; i < 16; i++) {
		printf("%d", pbcb->tags[i]);
	}
	printf("\n");
}

unsigned int DATAZONE_LOAD_ADDR_INT = DATAZONE_LOAD_ADDR;
enum checkpart{
	BCB = 0,
	PARTITION
} checkpartname;

u32 check_head_tag(enum checkpart checkpartname)
{
	char tags[16];
	if(checkpartname == BCB){
		memcpy(tags, (char *)DATAZONE_LOAD_ADDR, 16);
		if(memcmp(tags,"BCBHead", 7) == 0)
			return 0;
		return 1;
	}
	else if(checkpartname == PARTITION){
		memcpy(tags, (char *)(DATAZONE_LOAD_ADDR + 8), 16);
		if(memcmp(tags, "PTBL", 4) == 0)
			return 0;
		return 1;
	}
	else{
		return 1;
	}
	return 1;
}
u32 check_checksum(enum checkpart checkpartname)
{
	u32 checksum_from_calc = 0;
	u32 i = 0;
	u32 cnt;
	u8 *tmp = 0;
	partitionhead *phead;
	printf("check_checksum checkpartname = %d\n", checkpartname);
	if(checkpartname == BCB){
		tmp = (u8*)(DATAZONE_LOAD_ADDR + 20);
		for(i = 0; i < (sizeof(struct bootloader_message) - 20); i++){
			checksum_from_calc += *tmp++;
		}
		printf("check_checksum checksum_from_calc = %x,(*(u32 *)(DATAZONE_LOAD_ADDR + 0x10))= %x\n",
			checksum_from_calc,(*(u32 *)(DATAZONE_LOAD_ADDR + 0x10)));
		if(checksum_from_calc == (*(u32 *)(DATAZONE_LOAD_ADDR + 0x10)))
			return 0;
	}
	else if(checkpartname == PARTITION){
		phead = (partitionhead *)DATAZONE_LOAD_ADDR;
		cnt = phead->blockcnt;
		tmp = (u8*)(DATAZONE_LOAD_ADDR + 512);
		for(i = 0; i < cnt * 512; i++){
			checksum_from_calc += *tmp++;
		}
		//printf("check_checksum checksum_from_calc = %x,(*(u32 *)(DATAZONE_LOAD_ADDR + 0x10))= %x\n", checksum_from_calc,(*(u32 *)(DATAZONE_LOAD_ADDR + 0x10)));
		//if(checksum_from_calc == (*(u32 *)(DATAZONE_LOAD_ADDR + 0x10)))
		return 0;
	}else{
		return 1;
	}

	return 1;
}

u32 check_NAND_BCB_Valid(int *partflag)
{
	int ret;
	unsigned long DTZAddr = NAND_DATAZONE_MAIN_ADDR;
	unsigned long DTZAddr_bk = NAND_DATAZONE_BK_ADDR;
	unsigned long dtz_BCBSize = PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN;
	unsigned long BCBAddr = DTZAddr + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN;
	unsigned long BCBAddr_bk = DTZAddr_bk + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN;
	nand_info_t *nand = &nand_info[nand_curr_device];

	/* check bcb will read bcb to 5a00000 which will coverd partition table */
	g_partitionhead = NULL;
	g_partitionread = NULL;
	*partflag = -1;

	checkpartname = BCB;
	flush_invalid_cache(DATAZONE_LOAD_ADDR - BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN, dtz_BCBSize);
	ret = nand_read_skip_bad(nand, DTZAddr, &dtz_BCBSize, (unsigned char*)(unsigned long)(DATAZONE_LOAD_ADDR - BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN));
	if(ret != 0){
		printf("Read BCB Info Failed!\n");
		goto check_BCB_bk;
	}

	if (check_head_tag(checkpartname) != 0)
	{
		printf("No Valid BCB head tag Found!\n");
		goto check_BCB_bk;
	}

	if (check_checksum(checkpartname) != 0)
	{
		printf("No Valid BCB data Found!\n");
		goto check_BCB_bk;
	}
	printf("Read BCB Info Successfully, use BCB!\n");
	g_miscPartitonAddr = BCBAddr;
	//get_bcb_upg_mode();
	*partflag = 0;
	return 0;

check_BCB_bk:
	flush_invalid_cache(DATAZONE_LOAD_ADDR - BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN, dtz_BCBSize);
	ret = nand_read_skip_bad(nand, DTZAddr_bk, &dtz_BCBSize, (unsigned char*)(unsigned long)(DATAZONE_LOAD_ADDR - BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN));
	if(ret != 0)
	{
		printf("Read BCB_bk Info Failed!\n");
		return 1;
	}

	if (check_head_tag(checkpartname) != 0)
	{
		printf("No Valid BCB_bk head tag Found!\n");
		return 1;
	}

	if (check_checksum(checkpartname) != 0)
	{
		printf("No Valid BCB_bk data Found!\n");
		return 1;
	}

	printf("Read BCB_bk Info Successfully, use BCB_bk!\n");
	g_miscPartitonAddr = BCBAddr_bk;
	//get_bcb_upg_mode();
	*partflag = 1;
	return 0;
}

u32 check_BCB_Valid()
{
	u32 ret;

	u32 BCBAddr = 0x10000 + 4 * 1024;
	u32 BCBSize = 4 * 1024;
	u32 BCBAddr_bk = 0x10000 + 480 * 1024 + 4 * 1024;

	checkpartname = BCB;
#ifdef CONFIG_BOOT_MMC
	struct mmc *bootdev;
	int devnum = -1;
#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
	devnum = 0;
#elif (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT2)
	devnum = 2;
#endif

	bootdev = find_mmc_device(devnum);
	if ( NULL == bootdev){
		printf("can't find emmc device\n");
		return 1;
	}
	ret = mmc_init(bootdev);
	if(ret){
		printf("init emmc device fail\n");
		return -1;
	}
	memset((void *)DATAZONE_LOAD_ADDR, 0, sizeof(struct bootloader_message));
	/*check bcb*/
	flush_invalid_cache(DATAZONE_LOAD_ADDR, BCBSize);
	ret = bootdev->block_dev.block_read(devnum, (unsigned int)((BCBAddr)/512), BCBSize/512, (char *)(DATAZONE_LOAD_ADDR));
	if (ret == 0)
	{
		printf("Read BCB Info Failed!\n");
		goto check_BCB_bk;
	}
	if (check_head_tag(checkpartname) != 0)
	{
		printf("No Valid BCB head tag Found!\n");
		goto check_BCB_bk;
	}

	if (check_checksum(checkpartname) != 0)
	{
		printf("No Valid BCB data Found!\n");
		goto check_BCB_bk;
	}
	printf("Read BCB Info Successfully, use BCB!\n");
	g_miscPartitonAddr = BCBAddr;
	//get_bcb_upg_mode();
	return 0;

check_BCB_bk:
	/*check bcb_bk*/
	flush_invalid_cache(DATAZONE_LOAD_ADDR, BCBSize);
	ret = bootdev->block_dev.block_read(devnum, (unsigned int)((BCBAddr_bk)/512), BCBSize/512, (char *)(DATAZONE_LOAD_ADDR));
	if (ret == 0)
	{
		printf("Read BCB_bk Info Failed!\n");
		return 1;
	}

	if (check_head_tag(checkpartname) != 0)
	{
		printf("No Valid BCB_bk head tag Found!\n");
		return 1;
	}

	if (check_checksum(checkpartname) != 0)
	{
		printf("No Valid BCB_bk data Found!\n");
		return 1;
	}

	printf("Read BCB_bk Info Successfully, use BCB_bk!\n");
	g_miscPartitonAddr = BCBAddr_bk;
	//get_bcb_upg_mode();
	return 0;
#endif
	g_miscPartitonAddr = 0;
	return 1;
}

unsigned int check_nand_partition_Valid(void)
{
	int ret;

	unsigned long partitionAddr = NAND_DATAZONE_MAIN_ADDR + PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN;
	unsigned long partitionAddr_bk = NAND_DATAZONE_BK_ADDR + PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN;
	unsigned long partitionSize = PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN;
	nand_info_t *nand = &nand_info[nand_curr_device];
	checkpartname = PARTITION;

	/*check partitioninfo*/
	flush_invalid_cache(DATAZONE_LOAD_ADDR , partitionSize);
	ret = nand_read_skip_bad(nand, partitionAddr, &partitionSize, (unsigned char*)(unsigned long)DATAZONE_LOAD_ADDR);
	if (ret != 0)
	{
		printf("Read partition head Info Failed!\n");
		goto check_partition_bk;
	}

	if (check_head_tag(checkpartname) != 0)
	{
		printf("No Valid partition head tag Found!\n");
		goto check_partition_bk;
	}

	if (check_checksum(checkpartname) != 0)
	{
		printf("No Valid partition data Found!\n");
		goto check_partition_bk;
	}
	printf("Read partition Info Successfully, use partition!\n");
	return 0;

check_partition_bk:
	/*check partitioninfo_bk*/
	flush_invalid_cache(DATAZONE_LOAD_ADDR , partitionSize);
	ret = nand_read_skip_bad(nand, partitionAddr_bk, &partitionSize, (unsigned char*)(unsigned long)DATAZONE_LOAD_ADDR);
	if (ret != 0)
	{
		printf("Read partition_bk head Info Failed!\n");
		return 1;
	}

	if (check_head_tag(checkpartname) != 0)
	{
		printf("No Valid partition_bk head tag Found!\n");
		return 1;
	}

	if (check_checksum(checkpartname) != 0)
	{
		printf("No Valid partition_bk data Found!\n");
		return 1;
	}
	printf("Read partition_bk Info Successfully, use partition_bk!\n");

	return 0;
}

unsigned int check_partition_Valid()
{
	u32 ret;
	u32 partitionInfoAddr = 0x10000 + 8 * 1024;
	u32 partitionInfoSize = 472 * 1024;
	u32 partitionInfoAddr_bk = 0x10000 + 480 * 1024 + 8 * 1024;

	checkpartname = PARTITION;
#ifdef CONFIG_BOOT_MMC
	struct mmc *bootdev;
	int devnum = -1;
#if (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT0)
	devnum = 0;
#elif (CONFIG_BOOT_SD_SLOT  == MSDC_SLOT2)
	devnum = 2;
#endif

	bootdev = find_mmc_device(devnum);
	if ( NULL == bootdev){
		printf("can't find emmc device\n");
		return 1;
	}
	ret = mmc_init(bootdev);
	if(ret){
		printf("init emmc device fail\n");
		return -1;
	}

	/*check partitioninfo*/
	flush_invalid_cache(DATAZONE_LOAD_ADDR, 512);
	ret = bootdev->block_dev.block_read(devnum, (unsigned int)((partitionInfoAddr)/512), 1, (char *)DATAZONE_LOAD_ADDR);
	if (ret == 0)
	{
		printf("Read partition head Info Failed!\n");
		goto check_partition_bk;
	}
	if (check_head_tag(checkpartname) != 0)
	{
		printf("No Valid partition head tag Found!\n");
		goto check_partition_bk;
	}

	partitionhead *ppartitionhead = (partitionhead *)(DATAZONE_LOAD_ADDR);

	flush_invalid_cache(DATAZONE_LOAD_ADDR + 512, ppartitionhead->blockcnt * 512);
	ret = bootdev->block_dev.block_read(devnum, (unsigned int)((partitionInfoAddr)/512 + 1), (ppartitionhead->blockcnt), (char *)(DATAZONE_LOAD_ADDR + 512));
	if (ret == 0)
	{
		printf("Read partition Info Failed!\n");
		goto check_partition_bk;
	}

	if (check_checksum(checkpartname) != 0)
	{
		printf("No Valid partition data Found!\n");
		goto check_partition_bk;
	}
	printf("Read partition Info Successfully, use partition!\n");
	return 0;

check_partition_bk:
	/*check partitioninfo_bk*/
	flush_invalid_cache(DATAZONE_LOAD_ADDR, 512);
	ret = bootdev->block_dev.block_read(devnum, (unsigned int)((partitionInfoAddr_bk)/512), 1, (char *)DATAZONE_LOAD_ADDR);
	if (ret == 0)
	{
		printf("Read partition_bk head Info Failed!\n");
		return 1;
	}

	if (check_head_tag(checkpartname) != 0)
	{
		printf("No Valid partition_bk head tag Found!\n");
		return 1;
	}

	ppartitionhead = (partitionhead *)(DATAZONE_LOAD_ADDR);
	flush_invalid_cache(DATAZONE_LOAD_ADDR + 512, ppartitionhead->blockcnt * 512);
	ret = bootdev->block_dev.block_read(devnum, (unsigned int)((partitionInfoAddr_bk)/512 + 1), (ppartitionhead->blockcnt), (char *)(DATAZONE_LOAD_ADDR + 512));
	if (ret == 0)
	{
		printf("Read partition_bk Info Failed!\n");
		return 1;
	}

	if (check_checksum(checkpartname) != 0)
	{
		printf("No Valid partition_bk data Found!\n");
		return 1;
	}
	printf("Read partition_bk Info Successfully, use partition_bk!\n");
	return 0;
#endif

	return 1;
}

uint32_t read_datazone_bcb(struct bootloader_message *bcb)
{
    unsigned long dtz_BCBSize = PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN;
    unsigned long DTZSize = dtz_BCBSize + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN;
    unsigned long bcbSize = sizeof(struct bootloader_message);
    int err;
#ifndef CONFIG_BOOT_MMC
    nand_info_t *nand = &nand_info[nand_curr_device];
#endif

#ifdef CONFIG_BOOT_MMC
    if (check_BCB_Valid() != 0) {
        printf("check_BCB_Valid Failed!\n");
        return 1;
    }
    memcpy((char* )bcb, (char *)DATAZONE_LOAD_ADDR, bcbSize);
    return 0;
#else
	flush_invalid_cache(DATAZONE_LOAD_ADDR , DTZSize);
    err = nand_read_skip_bad(nand, NAND_DATAZONE_MAIN_ADDR, &DTZSize, (unsigned char*)(unsigned long)(DATAZONE_LOAD_ADDR));
    if(err != 0){
        printf("Read datazone info Failed!\n");
        goto read_bk;
    }
    memcpy((char* )bcb, (char *)(DATAZONE_LOAD_ADDR + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN), bcbSize);

    return 0;
read_bk:
	flush_invalid_cache(DATAZONE_LOAD_ADDR , DTZSize);
    err = nand_read_skip_bad(nand, NAND_DATAZONE_BK_ADDR, &DTZSize, (unsigned char*)(unsigned long)(DATAZONE_LOAD_ADDR));
    if(err != 0){
        printf("Read datazone info Failed!\n");
        return 1;
    }
    memcpy((char* )bcb, (char *)(DATAZONE_LOAD_ADDR + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN), bcbSize);

    return 0;
#endif
}

uint32_t set_datazone_bcb(struct bootloader_message *bcb)
{
    uint32_t checksum_from_calc = 0;
    unsigned long dtz_BCBSize = PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN;
    unsigned long DTZSize = dtz_BCBSize + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN;
    int err;

#ifdef CONFIG_NAND_BOOT
    nand_info_t *nand = &nand_info[nand_curr_device];
	flush_invalid_cache(DATAZONE_LOAD_ADDR , DTZSize);
    err = nand_read_skip_bad(nand, NAND_DATAZONE_MAIN_ADDR, &DTZSize, (unsigned char*)(unsigned long)(DATAZONE_LOAD_ADDR));
    if(err != 0){
        printf("Read datazone info Failed!\n");
        return 1;
    }
#endif
    checksum_from_calc = calc_bcb_checksum(bcb);
    printf("cal bcb checksum is 0x%x\n", checksum_from_calc);
    put_bcb_checksum(bcb, checksum_from_calc);

#ifdef CONFIG_BOOT_MMC
    err = write_bcb(bcb, BCB_MAIN_OFFSET_FROM_MMCBLK);
    if (err < 0) {
        printf("write main bcb fail.\n");
        return 1;
    }

    /*set bcb_bk*/
    err = write_bcb(bcb, BCB_BK_OFFSET_FROM_MMCBLK);
    if (err < 0) {
        printf("write bcb_bk fail.\n");
        return 1;
    }
#else
    printf("copy bcb to datazone bcb\n");
    memcpy((char*)(DATAZONE_LOAD_ADDR + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN), (char*)bcb, sizeof(struct bootloader_message));

    printf("write datazone info to nand\n");
    erase_nand("datazone");
	flush_cache(DATAZONE_LOAD_ADDR, DTZSize);
    err = write_nand_ex((uchar*)DATAZONE_LOAD_ADDR,
			DTZSize,
			"datazone",
			0,
			"raw",
			0);
	if (err < 0) {
        printf("write main bcb fail.\n");
        return 1;
    }

    printf("write datazone_bk info to nand\n");
    erase_nand("datazone_bk");
    err = write_nand_ex((uchar*)DATAZONE_LOAD_ADDR,
			DTZSize,
			"datazone_bk",
			0,
			"raw",
			0);
    if (err < 0) {
        printf("write bcb_bk fail.\n");
        return 1;
    }
#endif

    return 0;
}

uint32_t clear_datazone_bcb(void)
{
	int ret;
	uint32_t chksum;

	printf("Starting clear datazone bcb\n");

#ifdef CONFIG_BOOT_MMC
	struct bootloader_message bcb;
	ret = read_datazone_bcb(&bcb);
	if (ret) {
		printf("read datazone bcb fail.\n");
		return 1;
	}

	memset(bcb.recovery, 0, sizeof(bcb.recovery));

#ifdef CONFIG_SECURITY_UPGRADE
	// If security upgrade is enabled, calculate and set checksum
	chksum = calc_bcb_checksum(&bcb);
	printf("New bcb_checksum: 0x%x\n", chksum);
	put_bcb_checksum(&bcb, chksum);
#endif
	ret = write_bcb(&bcb, BCB_MAIN_OFFSET_FROM_MMCBLK);
	if (ret < 0) {
		printf("write main bcb fail.\n");
		return 1;
	}
#else
#define NAND_PAGE_SIZE 4096
	char addr_str[16] = {0};
	static unsigned char page_aligned_buffer[NAND_PAGE_SIZE] __attribute__((aligned(NAND_PAGE_SIZE)));
	struct bootloader_message *bcb = (struct bootloader_message *)page_aligned_buffer;

	memset(page_aligned_buffer, 0xFF, NAND_PAGE_SIZE);

	// Read BCB data from datazone partition using partition name
	printf("Reading BCB data from datazone partition...\n");
	sprintf(addr_str, "0x%lx", (unsigned long)page_aligned_buffer);
	char *argv_read[] = {"nand", "read", addr_str, "datazone", "0x1000"};
	ret = do_nand(NULL, 0, 6, argv_read);
	if(ret != 0){
		printf("Warning: Failed to read BCB info, using default BCB settings\n");
	} else {
		printf("Successfully read BCB info\n");
	}

	memset(bcb->command, 0, sizeof(bcb->command));
	memset(bcb->recovery, 0, sizeof(bcb->recovery));

#ifdef CONFIG_SECURITY_UPGRADE
	chksum = calc_bcb_checksum(bcb);
	printf("New BCB checksum: 0x%x\n", chksum);
	put_bcb_checksum(bcb, chksum);
#endif

	// Erase datazone partition
	printf("Erasing first 4KB of datazone partition...\n");
	char *argv_erase[] = {"nand", "erase", "datazone", "0x1000"};
	ret = do_nand(NULL, 0, 5, argv_erase);
	if (ret != 0) {
		printf("Error: Failed to erase datazone partition 0x1000\n");
		return 1;
	}

	// Write modified BCB data to datazone partition
	printf("Writing modified BCB data...\n");
	char *argv_write[] = {"nand", "write", addr_str, "datazone", "0x1000"};
	ret = do_nand(NULL, 0, 6, argv_write);
	if (ret != 0) {
		printf("Error: Failed to write BCB data\n");
		return 1;
	}

#endif
	printf("BCB data write to datazone success\n");
	return 0;
}

partitionread * readpartitioninfofromflash()
{
	char *bufpartinfo;
	partitionhead *pparthead;
	partitionread *ppartread,*pprepartition,*pcurpartition;

	//printf("---------------readpartitioninfofromflash start addr:0x%X-----------------\r\n",u4PartionAddress);


	if(0 != check_partition_Valid())
	{
		printf("check_partition_Valid fail\r\n");
		while(1);
	}

	ppartread = (partitionread *)(DATAZONE_LOAD_ADDR + 512);
	pcurpartition = ppartread;
	pprepartition = pcurpartition;

	while(pcurpartition != NULL)
	{
		if (pcurpartition->u4LastPartition == 1)
		{
			//printf("readpartitioninfofromflash this is last partition\r\n");
			pcurpartition->nextpartition = NULL;
			break;
		}
		else
		{

			//printf("readpartitioninfofromflash part before addree:0x%X,partitionread,size=%d\r\n",pcurpartition,sizeof(partitionread));
			pcurpartition = pcurpartition + 1;
			//printf("readpartitioninfofromflash part after addree:0x%X\r\n",pcurpartition);
			pprepartition->nextpartition = pcurpartition;
			pprepartition = pcurpartition;
		}
	}
	//printf("---------------readpartitioninfofromflash end-------------------\r\n");

	return ppartread;

}



int * readpartitioninfofromflash_ext()
{

	struct mmc *emmc_dev = NULL;
	int emmc_dev_num = 0;
	int n;
	ulong size;
	int err = 0;
	//char  buf[512];
	partitionread *ppartread,*pprepartition,*pcurpartition;

	unsigned long blknum;

	//printf("---------------readpartitioninfofromflash start addr:0x%X-----------------\r\n",u4PartionAddress);


	if(0 != check_partition_Valid())
	{
		printf("check_partition_Valid fail\r\n");
		while(1);
	}

	ppartread = (partitionread *)(DATAZONE_LOAD_ADDR + 512);


	pcurpartition = ppartread;
	pprepartition = pcurpartition;


	while(pcurpartition != NULL)
	{
		//printf("readpartitioninfofromflash partname:%s\r\n",pcurpartition->szPartName);
		//printf("readpartitioninfofromflash szType:%s\r\n",pcurpartition->szType);
		//printf("readpartitioninfofromflash u4Mount:%d\r\n",pcurpartition->u4Mount);
		//printf("readpartitioninfofromflash nextpartition:0x%X\r\n",pcurpartition->nextpartition);


		if (pcurpartition->u4LastPartition == 1)
		{
			//printf("readpartitioninfofromflash this is last partition\r\n");
			pcurpartition->nextpartition = NULL;
			break;
		}
		else
		{

			//printf("readpartitioninfofromflash part before addree:0x%X,partitionread,size=%d\r\n",pcurpartition,sizeof(partitionread));
			pcurpartition = pcurpartition + 1;
			//printf("readpartitioninfofromflash part after addree:0x%X\r\n",pcurpartition);
			pprepartition->nextpartition = pcurpartition;
			pprepartition = pcurpartition;
		}
	}


	//printf("---------------readpartitioninfofromflash end-------------------\r\n");

	//return ppartread;
	g_partitionhead = (partitionhead *)(DATAZONE_LOAD_ADDR);
	g_partitionread = ppartread;

	g_partitionhead->nextpartition = g_partitionread;

	return 0;
}
#else //below is not CONFIG_SECURITY_UPGRADE = no


partitionread * readpartitioninfofromflash()
{

	struct mmc *emmc_dev = NULL;
	int emmc_dev_num = 0;
	int n;
	ulong size;
	int err = 0;
	char  buf[512];
	char *bufpartinfo;
	partitionhead *pparthead;
	partitionread *ppartread,*pprepartition,*pcurpartition;
	unsigned long u4PartionAddress = 0x400000 - 512;

	unsigned long blknum;

	//printf("---------------readpartitioninfofromflash start addr:0x%X-----------------\r\n",u4PartionAddress);



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

	blknum = 1;
	flush_invalid_cache(buf, blknum * 512);
	n = emmc_dev->block_dev.block_read(emmc_dev_num, u4PartionAddress/512, blknum, (char *)buf);
	if (!n)
	{
		printf("readpartitioninfofromflash block_read fail result=%d\r\n",n);
		return -1;
	}
	//printf("readpartitioninfofromflash block_read success  result=%d\r\n",n);
	pparthead = (partitionhead *)buf;

	//printf("readpartitioninfofromflash block_read  blockcnt=%d\r\n",pparthead->blockcnt);

	bufpartinfo =(char *)malloc(pparthead->blockcnt*512);
	flush_invalid_cache(bufpartinfo, pparthead->blockcnt * 512);
	n = emmc_dev->block_dev.block_read(emmc_dev_num, (u4PartionAddress- pparthead->blockcnt*512)/512, pparthead->blockcnt, (char *)bufpartinfo);
	if (!n)
	{
		printf("readpartitioninfofromflash block_read fail result=%d\r\n",n);
		return -1;
	}

	ppartread = (partitionread *)bufpartinfo;
	pcurpartition = ppartread;
	pprepartition = pcurpartition;



	while(pcurpartition != NULL)
	{
		//printf("readpartitioninfofromflash partname:%s\r\n",pcurpartition->szPartName);

		//printf("readpartitioninfofromflash szType:%s\r\n",pcurpartition->szType);
		//printf("readpartitioninfofromflash u4Mount:%d\r\n",pcurpartition->u4Mount);
		//printf("readpartitioninfofromflash nextpartition:0x%X\r\n",pcurpartition->nextpartition);


		if (pcurpartition->u4LastPartition == 1)
		{
			//printf("readpartitioninfofromflash this is last partition\r\n");
			pcurpartition->nextpartition = NULL;
			break;
		}
		else
		{

			//printf("readpartitioninfofromflash part before addree:0x%X,partitionread,size=%d\r\n",pcurpartition,sizeof(partitionread));
			pcurpartition = pcurpartition + 1;
			//printf("readpartitioninfofromflash part after addree:0x%X\r\n",pcurpartition);
			pprepartition->nextpartition = pcurpartition;
			pprepartition = pcurpartition;
		}
	}


	//printf("---------------readpartitioninfofromflash end-------------------\r\n");

	return ppartread;

}

int * readpartitioninfofromflash_ext()
{

	struct mmc *emmc_dev = NULL;
	int emmc_dev_num = 0;
	int n;
	ulong size;
	int err = 0;
	//char  buf[512];
	char *buf;
	char *bufpartinfo;
	partitionhead *pparthead;
	partitionread *ppartread,*pprepartition,*pcurpartition;
	unsigned long u4PartionAddress = 0x400000 - 512;

	unsigned long blknum;

	//printf("---------------readpartitioninfofromflash start addr:0x%X-----------------\r\n",u4PartionAddress);



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

	buf = (char *)malloc(512);
	if (buf == NULL)
		return -1;

	blknum = 1;
	flush_invalid_cache(buf, blknum * 512);
	n = emmc_dev->block_dev.block_read(emmc_dev_num, u4PartionAddress/512, blknum, (char *)buf);
	if (!n)
	{
		printf("readpartitioninfofromflash block_read fail result=%d\r\n",n);
		return -1;
	}
	//printf("readpartitioninfofromflash block_read success  result=%d\r\n",n);
	pparthead = (partitionhead *)buf;


	//printf("readpartitioninfofromflash block_read  blockcnt=%d\r\n",pparthead->blockcnt);

	bufpartinfo =(char *)malloc(pparthead->blockcnt*512);
	flush_invalid_cache(bufpartinfo, pparthead->blockcnt * 512);
	n = emmc_dev->block_dev.block_read(emmc_dev_num, (u4PartionAddress- pparthead->blockcnt*512)/512, pparthead->blockcnt, (char *)bufpartinfo);
	if (!n)
	{
		printf("readpartitioninfofromflash block_read fail result=%d\r\n",n);
		return -1;
	}

	ppartread = (partitionread *)bufpartinfo;


	pcurpartition = ppartread;
	pprepartition = pcurpartition;


	while(pcurpartition != NULL)
	{
		//printf("readpartitioninfofromflash partname:%s\r\n",pcurpartition->szPartName);
		//printf("readpartitioninfofromflash szType:%s\r\n",pcurpartition->szType);
		//printf("readpartitioninfofromflash u4Mount:%d\r\n",pcurpartition->u4Mount);
		//printf("readpartitioninfofromflash nextpartition:0x%X\r\n",pcurpartition->nextpartition);

		if (pcurpartition->u4LastPartition == 1)
		{
			//printf("readpartitioninfofromflash this is last partition\r\n");
			pcurpartition->nextpartition = NULL;
			break;
		}
		else
		{

			//printf("readpartitioninfofromflash part before addree:0x%X,partitionread,size=%d\r\n",pcurpartition,sizeof(partitionread));
			pcurpartition = pcurpartition + 1;
			//printf("readpartitioninfofromflash part after addree:0x%X\r\n",pcurpartition);
			pprepartition->nextpartition = pcurpartition;
			pprepartition = pcurpartition;
		}
	}


	//printf("---------------readpartitioninfofromflash end-------------------\r\n");

	//return ppartread;
	g_partitionhead = pparthead;
	g_partitionread = ppartread;

	g_partitionhead->nextpartition = g_partitionread;

	return 0;
}
#endif

#if ATC_AB_PARTITION_SUPPORT
void set_bcb_slotinfo(struct bootloader_message *pbcb, uint32_t bootflag)
{
	boot_ctrl_t *metadata = &(pbcb->metadata);
	int slot1;

	memset(metadata, 0, sizeof(boot_ctrl_t));

	metadata->magic = BOOTCTRL_MAGIC;
	metadata->doublepart = 1;

	metadata->slot_info[bootflag].successful_boot = 1;
	metadata->slot_info[bootflag].priority = 7;
	metadata->slot_info[bootflag].retry_count = 3;
	metadata->slot_info[bootflag].normal_boot = 1;

	/* Re-set arg to another slot */
	slot1 = (bootflag == 0) ? 1 : 0;
	metadata->slot_info[slot1].successful_boot = 0;
	metadata->slot_info[slot1].priority = 6;
	metadata->slot_info[slot1].retry_count = 3;
	metadata->slot_info[slot1].normal_boot = 1;
}
#endif

//write partition info to emmc
int writepartitioninfotoflash(partitionhead *parthead)
{
#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
	int partition_info_is_write_protect = 0;
#endif
	struct mmc *emmc_dev = NULL;
	int emmc_dev_num = 0;
	char  *buf,*p;
	int n,err = 0;;
	partitionread *pcurpart;
	buf = (char *)malloc(parthead->blockcnt * 512);
	memset(buf,0,parthead->blockcnt * 512);
	parthead->u4Version = ATC_PARTITION_VER;
	parthead->u4Signature = ATC_PTBL_SIGN;
	memcpy(buf,parthead,sizeof(partitionhead));

	printf("---------------writepartitioninfotoflash start  addr:0x%X-----------------\r\n",g_u4PartionAddress);

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

#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
	//printf("[WP]wpg%d: %d\n", 0, emmc_wpg_type(0));
	//printf("[WP]wpg%d: %d\n", 1, emmc_wpg_type(1));
	//printf("[WP]wpg%d: %d\n", 2, emmc_wpg_type(2));
	//printf("[WP]wpg%d: %d\n", 3, emmc_wpg_type(3));
	//printf("[WP]wpg%d: %d\n", 4, emmc_wpg_type(4));

	if(g_u4PartionAddress/512 > emmc_dev->wp_size) {
		printf("[WP] warning: partition table is not in the first write protect group\n");
	}

	if(1 == emmc_wpg_type(g_u4PartionAddress/512/emmc_dev->wp_size)) {
		partition_info_is_write_protect = 1;
		if(emmc_set_user_wp(WP_DISABLE, 0, g_u4PartionAddress/512, 1))
			printf("[WP] clear wp fail in %s\n", __func__);
		else
			printf("[WP] clear wp success in %s\n", __func__);
	}
#endif

	flush_cache(buf, 512);
	n = emmc_dev->block_dev.block_write(emmc_dev_num, (unsigned long)((g_u4PartionAddress-512)/512) , 1, (char *)buf);
	if (n != 1)
	{
		printf("<writepartitioninfotoflash> block_write part head failed = %d\r\n", n);
		return -1;
	}
	printf("<writepartitioninfotoflash> block_write part head success\r\n");


	memset(buf,0,parthead->blockcnt * 512);
	pcurpart= parthead->nextpartition;
	p = buf;

	printf("readpartitioninfofromflash block_read test parthead->blockcnt=%d\r\n",parthead->blockcnt);

	while(1)
	{
		if (pcurpart == NULL)
			break;

		printf("<writepartitioninfotoflash> szPartName:%s,nextpartition:0x%X \r\n",pcurpart->szPartName,pcurpart->nextpartition);
		memcpy(p,pcurpart,sizeof(partitionread));
		p += sizeof(partitionread);
		pcurpart= pcurpart->nextpartition;
	}

	flush_cache(buf, parthead->blockcnt * 512);
	n = emmc_dev->block_dev.block_write(emmc_dev_num, (unsigned long)((g_u4PartionAddress - parthead->blockcnt*512 -512)/512) , parthead->blockcnt, (char *)buf);
	if (n == 0)
	{
		printf("<writepartitioninfotoflash> block_write failed \r\n");
	}

#if (CONFIG_EMMC_WRITE_PROTECT_ENABLE == 1)
	if(partition_info_is_write_protect) {
		if(emmc_set_user_wp(WP_ENABLE, 0,  g_u4PartionAddress/512, 1))
			printf("[WP] restore wp fail in %s\n", __func__);
		else {
			partition_info_is_write_protect = 0;
			printf("[WP] restore wp success in %s\n", __func__);
		}
	}
#endif
	printf("---------------writepartitioninfotoflash end-------------------\r\n");
	return 0;
}

#ifdef NEW_PARTITION_DESIGN
void getpartitionbypartitionname(u32 *flag, char *partitionname)
{
	partitionread *pcurpartition;
	pcurpartition = g_partitionread;

	while(pcurpartition != NULL)
	{
		if (strcmp(pcurpartition->szPartName, partitionname) == 0)
		{
			*flag = pcurpartition->u4Flag;
			break;
		}
		pcurpartition = pcurpartition->nextpartition;
	}

}
#endif

int writepartitioninfotonand(partitionhead *parthead)
{
	unsigned long DTZAddr = NAND_DATAZONE_MAIN_ADDR;
	unsigned long DTZAddr_bk = NAND_DATAZONE_BK_ADDR;
	unsigned long dtz_BCBSize = PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN;
	unsigned long DTZSize = NAND_DATAZONE_SIZE;
	nand_info_t *nand = &nand_info[nand_curr_device];

	char *buf,*p;
	int  err = 0;
	int partflag = -1;
	unsigned long partLen = parthead->blockcnt * 512 + 512;
	uint32_t chksum = 0;
	partitionread *pcurpart;

	buf = (char*)malloc(partLen);
	if(NULL == buf)
	{
		printf("Can not malloc memory for part table.\r\n");
		return -1;
	}

	memset(buf, 0, partLen);
	p = buf;
	buf += 512;
	pcurpart = parthead->nextpartition;
	while(pcurpart)
	{
		chksum += checksum32(0, pcurpart, sizeof(partitionread));
		memcpy(buf, pcurpart, sizeof(partitionread));
		buf += sizeof(partitionread);
		pcurpart = pcurpart->nextpartition;
	}

	put_checksum32(parthead->checksum, chksum);
	parthead->u4Version = ATC_PARTITION_VER;
	parthead->u4Signature = ATC_PTBL_SIGN;
	memcpy(p, parthead, sizeof(partitionhead));

	err = check_NAND_BCB_Valid(&partflag);
	if(0 != err)
	{
		printf("Check bcb failed.\r\n");
		free(p);
		return -1;
	}

	if(0 == partflag)
	{
		flush_invalid_cache(DATAZONE_LOAD_ADDR - PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN , dtz_BCBSize);
		err = nand_read_skip_bad(nand, DTZAddr, &dtz_BCBSize, (unsigned char*)(unsigned long)(DATAZONE_LOAD_ADDR - PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN));
		if(err != 0){
			printf("Read DTZ Info Failed!Start to read dtz_bk\n");
			partflag = -1;
		}
	}
	else
	{
		flush_invalid_cache(DATAZONE_LOAD_ADDR - PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN , dtz_BCBSize);
		err = nand_read_skip_bad(nand, DTZAddr_bk, &dtz_BCBSize, (unsigned char*)(unsigned long)(DATAZONE_LOAD_ADDR - PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN));
		if(err != 0){
			printf("Read DTZ_BK Info Failed!  Write partition info error.\n");
			partflag = -1;
		}
	}

	memset((void*)(unsigned long)(DATAZONE_LOAD_ADDR), 0, partLen);
	memcpy((void*)(unsigned long)(DATAZONE_LOAD_ADDR), p, partLen);

	(void)erase_nand("datazone");

	flush_cache(DATAZONE_LOAD_ADDR - PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN, DTZSize);
	err = write_nand_ex((uchar*)(DATAZONE_LOAD_ADDR - PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN),
			(ulong)DTZSize,
			"datazone",
			0,
			"raw",
			0);
	if (err < 0)
	{
		printf("<%s> block_write part to dtz failed = %d\r\n", __func__, err);
	}

	(void)erase_nand("datazone_bk");

	err = write_nand_ex((uchar*)(DATAZONE_LOAD_ADDR - PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN),
			(ulong)DTZSize,
			"datazone_bk",
			0,
			"raw",
			0);
	if (err < 0)
	{
		printf("<%s> block_write part to dtz_bk failed = %d\r\n", __func__, err);
	}

	free(p);
	printf("---------------writepartitioninfotoflash end-------------------\r\n");

	return 0;
}

int writepartitioninfotonand_ext(char *pDtzBuf, partitionhead *parthead)
{
	char *buf,*p;
	int  err = 0;
	uint32_t chksum = 0;
	partitionread *pcurpart;

	buf = (char *)(pDtzBuf + PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN + PTBL_BLOCK_SIZE);
	memset(buf, 0, parthead->blockcnt * PTBL_BLOCK_SIZE);
	pcurpart = parthead->nextpartition;
	p = buf;

	while(pcurpart)
	{
		memcpy(p,pcurpart,sizeof(partitionread));
		chksum += checksum32(0, p, sizeof(partitionread));
		p += sizeof(partitionread);
		pcurpart = pcurpart->nextpartition;
	}

	printf("parthead->blockcnt:%d, checksum: 0x%x\n", parthead->blockcnt, chksum);
	put_checksum32(parthead->checksum, chksum);

	buf = (char *)(pDtzBuf + PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN);

	parthead->u4Version = ATC_PARTITION_VER;
	parthead->u4Signature = ATC_PTBL_SIGN;

	memset(buf, 0, PTBL_BLOCK_SIZE);
	memcpy(buf,parthead,sizeof(partitionhead));

	(void)erase_nand("datazone");

	flush_cache(pDtzBuf, NAND_DATAZONE_SIZE);
	err = write_nand_ex( (uchar*)pDtzBuf,
			(ulong)NAND_DATAZONE_SIZE,
			"datazone",
			0,
			"raw",
			0);
	if (err < 0)
	{
		printf("<%s> block_write part to dtz failed = %d\r\n", __func__, err);
		return -1;
	}

	(void)erase_nand("datazone_bk");

	err = write_nand_ex( (uchar*)pDtzBuf,
			(ulong)NAND_DATAZONE_SIZE,
			"datazone_bk",
			0,
			"raw",
			0);
	if (err < 0)
	{
		printf("<%s> block_write part to dtz_bk failed = %d\r\n", __func__, err);
		return -1;
	}
	printf("---------------<%s> end-------------------\r\n", __func__);

	return 0;
}

/*
 *   read partition info from nand, add by mtk68080 20150916
 *   NOTE: here we need to use "env" partiton info from defautl "mtdparts",so we must make sure "env"
 *    part info in default "mtdparts" is same with real case
 */
partitionread * readpartitioninfofromnand(void)
{
	char *buf = NULL;
	char *bufpartinfo;
	int n;
	partitionhead *pparthead;
	partitionread *ppartread,*pprepartition,*pcurpartition;

	//printf("---------------readpartitioninfofromnand start ----------------\r\n");
	if(0 != check_nand_partition_Valid())
	{
		printf("check_partition_Valid fail\r\n");
		while(1);
	}

	buf = (char *)DATAZONE_LOAD_ADDR;
	pparthead = (partitionhead *)buf;

	bufpartinfo = buf + PTBL_BLOCK_SIZE;

	ppartread = (partitionread *)bufpartinfo;
	pcurpartition = ppartread;
	pprepartition = pcurpartition;


	while(pcurpartition != NULL)  //from array  to linklist
	{
		if (pcurpartition->u4LastPartition == 1)
		{
			//printf("readpartitioninfofromnand this is last partition\r\n");
			pcurpartition->nextpartition = NULL;
			break;
		}
		else
		{
			pcurpartition = pcurpartition + 1;
			pprepartition->nextpartition = pcurpartition;
			pprepartition = pcurpartition;
		}
	}

	g_partitionhead = pparthead;
	g_partitionread = ppartread;
	g_partitionhead->nextpartition = g_partitionread;

	printf("---------------readpartitioninfofromnand end-------------------\r\n");
	return ppartread;
}


/*
 *   read partition info from nand, add by mtk68080 20150916
 *   NOTE: here we need to use "env" partiton info from defautl "mtdparts",so we must make sure "env"
 *    part info in default "mtdparts" is same with real case
 */
int readpartitioninfofromnand_ext(void)
{
	char *buf = NULL;
	char *bufpartinfo;
	int n;
	partitionhead *pparthead;
	partitionread *ppartread,*pprepartition,*pcurpartition;

	//printf("---------------readpartitioninfofromnand start ----------------\r\n");
	if(0 != check_nand_partition_Valid())
	{
		printf("check_partition_Valid fail\r\n");
		while(1);
	}

	buf = (char *)DATAZONE_LOAD_ADDR;
	pparthead = (partitionhead *)buf;

	bufpartinfo = buf + PTBL_BLOCK_SIZE;

	ppartread = (partitionread *)bufpartinfo;
	pcurpartition = ppartread;
	pprepartition = pcurpartition;


	while(pcurpartition != NULL)  //from array  to linklist
	{
		if (pcurpartition->u4LastPartition == 1)
		{
			pcurpartition->nextpartition = NULL;
			break;
		}
		else
		{
			pcurpartition = pcurpartition + 1;
			pprepartition->nextpartition = pcurpartition;
			pprepartition = pcurpartition;
		}
	}

	g_partitionhead = pparthead;
	g_partitionread = ppartread;
	g_partitionhead->nextpartition = g_partitionread;

	printf("---------------readpartitioninfofromflash end-------------------\r\n");
	return 0;
}

char *check_partition()
{
	partitionread *ppartitionread,*p;
	char *propvaluetest;
	char *mtd_parts;
	char szVal[17];
	unsigned int blocknum;
	unsigned int blockoffset;
	unsigned int vallow,valhigh;
	int partflag = -1;
	char *strval;
	unsigned int index = 0;
#ifdef ATC_AB_PARTITION_SUPPORT
	slot_metadata_t slot_info[2];
	int ab_slot = 0;
#endif
	unsigned int bootfromAorB = 0;//0-->A  1-->B
#ifndef CONFIG_BOOT_MMC
	nand_info_t *nand0 = &nand_info[nand_curr_device];
#endif
	propvaluetest = (char *)malloc(1024);
	memset(propvaluetest,0,1024);
	mtd_parts= (char *)malloc(600);
	memset(mtd_parts,0,600);
	//printf("check_partition ww enter:\r\n");
#ifdef CONFIG_SECURITY_UPGRADE
#ifdef CONFIG_BOOT_MMC
	if(check_BCB_Valid() != 0){
		printf("No Valid datazone!!!\n");
		while(1);
	}
	bootfromAorB = get_bootflag_from_bcb();
#else
	if(check_NAND_BCB_Valid(&partflag) != 0){
		printf("NAND:No Valid datazone!!!\n");
		while(1);
	}
	bootfromAorB = get_bootflag_from_bcb();
#endif
#ifdef ATC_AB_PARTITION_SUPPORT
	const char* suffix = get_suffix();
	if (!suffix) {
		printf("suffix is NULL, default use slotA\n");
		ab_slot = 0;
	} else {
		if(0 == strcmp(suffix, BOOTCTRL_SUFFIX_A)) {
			ab_slot = 0;
		} else {
			ab_slot = 1;
		}
	}
	printf("ab_slot is %d\n", ab_slot);
#endif
#endif
	printf("********Boot from %s partition*********\r\n", (bootfromAorB == 0)?"Main":"Backup");
#ifdef CONFIG_BOOT_MMC
	ppartitionread = readpartitioninfofromflash();
#else //nand boot
	ppartitionread = readpartitioninfofromnand();
#endif
	p = ppartitionread;
	strcat(propvaluetest,"parts=");
#ifndef CONFIG_BOOT_MMC
	strcat(mtd_parts,"mtdparts=atcnand:");
#endif

#ifndef CONFIG_BOOT_MMC
	strcat(g_bootcmd,"nand read"); //for nand boot
	strcat(g_bootrecovercmd,"nand read");
#else
#if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT0)
	strcat(g_bootcmd,"mmc read 0");
	strcat(g_bootrecovercmd,"mmc read 0");
#else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
	strcat(g_bootcmd,"mmc read 2");
	strcat(g_bootrecovercmd,"mmc read 2");
#endif
#endif

	while(p)
	{
#ifdef NEW_PARTITION_DESIGN
		//dumppartitionread(p);
#endif

#ifndef CONFIG_BOOT_MMC
		p->u8RealDataSize = ALIGN(p->u8RealDataSize, nand0->writesize);
#else
		p->u8RealDataSize = ALIGN(p->u8RealDataSize, 512);
#endif
		printf("name:%s addr:0x%llx realsize:0x%llx\n",
				p->szPartName, p->u8PartitionStartAddr, p->u8RealDataSize);

		if (p->u8PartitionSize != 0)
		{
			//printf("check_partition u4PartitionSize=0x%X\r\n",p->u4PartitionSize);
			if (p->u8PartitionSize < 1024*1024)
			{
				vallow= (unsigned int)(p->u8PartitionSize / 1024);
				strval = uitostr(szVal,vallow);
				strcat(propvaluetest,strval);
				strcat(propvaluetest,"K@");
#ifndef CONFIG_BOOT_MMC
				strcat(mtd_parts,strval);
				strcat(mtd_parts,"K@");
#endif

			}
			else
			{
				vallow= (unsigned int)(p->u8PartitionSize / (1024*1024));
				strval = uitostr(szVal,vallow);
				strcat(propvaluetest,strval);
				strcat(propvaluetest,"M@");
#ifndef CONFIG_BOOT_MMC
				strcat(mtd_parts,strval);
				strcat(mtd_parts,"M@");
#endif
			}
		}

		strcat(propvaluetest,"0x");
#ifndef CONFIG_BOOT_MMC
		strcat(mtd_parts,"0x");
#endif

		//printf("check_partition u8PartitionStartAddr=0x%llX:\r\n",p->u4PartitionSize);
		//strval = uitostr_u64(szVal,p->u8PartitionStartAddr);
		u64_to_u32(p->u8PartitionStartAddr,&valhigh,&vallow);
		if (valhigh != 0)
		{
			strval = uitostr_hex(szVal,valhigh);
			strcat(propvaluetest,strval);
#ifndef CONFIG_BOOT_MMC
			strcat(mtd_parts,strval);
#endif
			//printf("check_partition high:%s\r\n",strval);
		}
		strval = uitostr_hex(szVal,vallow);
		if (vallow==0 && valhigh !=0)
		{
			strcat(propvaluetest,"00000000");
#ifndef CONFIG_BOOT_MMC
			strcat(mtd_parts,"00000000");
#endif
		}
		else
		{
			strcat(propvaluetest,strval);
#ifndef CONFIG_BOOT_MMC
			strcat(mtd_parts,strval);
#endif
		}
		//printf("check_partition low:%s\r\n",strval);

#ifdef ATC_AB_PARTITION_SUPPORT
		if ((ab_slot == 0 && (0 == strcmp(p->szPartName, "kernel_a")))
				|| (ab_slot == 1 && (0 == strcmp(p->szPartName, "kernel_b"))))
#else
			if ((bootfromAorB == 0 && (0 == strcmp(p->szPartName, "kernel")))
					|| (bootfromAorB == 1 && (0 == strcmp(p->szPartName, "kernel_bk"))))
#endif
			{
#ifdef CONFIG_BOOT_MMC
				//u64_to_u32(p->u8PartitionStartAddr,&valhigh,&vallow);
				blockoffset = (unsigned int)(p->u8PartitionStartAddr/512);
				strval = uitostr_hex(szVal,blockoffset);

#ifdef CONFIG_KERNEL_ZIMAGE
				sprintf(g_bootcmd + strlen(g_bootcmd), " 0x%x 0x", KERNEL_LOAD_ADDR);
#else
				strcat(g_bootcmd," 0x6000000 0x");
#endif
				strcat(g_bootcmd,strval);

#ifdef CONFIG_KERNEL_ZIMAGE
				sprintf(g_bootrecovercmd + strlen(g_bootrecovercmd), " 0x%x 0x", KERNEL_LOAD_ADDR);
#else
				strcat(g_bootrecovercmd," 0x6000000 0x");
#endif
				strcat(g_bootrecovercmd,strval);
#ifdef READ_IMAGE_USE_REAL_SIZE
				vallow= (unsigned int)(p->u8RealDataSize/512);
#else
				vallow= (unsigned int)(p->u8PartitionSize/512);
#endif
				strval = uitostr_hex(szVal,vallow);
				printf("++++ name:%s vallow:%s ++++\r\n", p->szPartName, strval);
				strcat(g_bootcmd," 0x");
				strcat(g_bootcmd,strval);
				strcat(g_bootcmd,"\;");

				strcat(g_bootrecovercmd," 0x");
				strcat(g_bootrecovercmd,strval);
				strcat(g_bootrecovercmd,"\;");
#else //for nand boot
#ifdef CONFIG_KERNEL_ZIMAGE
				sprintf(g_bootcmd + strlen(g_bootcmd), " 0x%x ", KERNEL_LOAD_ADDR);
				sprintf(g_bootrecovercmd + strlen(g_bootrecovercmd), " 0x%x ", KERNEL_LOAD_ADDR);
#else
				strcat(g_bootcmd," 0x6000000 ");
				strcat(g_bootrecovercmd," 0x6000000 ");
#endif
#ifdef ATC_AB_PARTITION_SUPPORT
				if(0 == ab_slot) {
					strcat(g_bootcmd,"kernel_a 0x");
					strcat(g_bootrecovercmd,"kernel_a 0x");
				} else {
					strcat(g_bootcmd,"kernel_b 0x");
					strcat(g_bootrecovercmd,"kernel_b 0x");
				}
#else
				if(0 == bootfromAorB) {
					strcat(g_bootcmd,"kernel 0x");
					//for nand recovery mode
					strcat(g_bootrecovercmd,"kernel 0x");
				} else {
					strcat(g_bootcmd,"kernel_bk 0x");
					//for nand recovery mode
					strcat(g_bootrecovercmd,"kernel_bk 0x");
				}
#endif

#ifdef READ_IMAGE_USE_REAL_SIZE
				vallow= (unsigned int)p->u8RealDataSize;
#else
				vallow= (unsigned int)p->u8PartitionSize;
#endif
				strval = uitostr_hex(szVal,vallow);

				strcat(g_bootcmd,strval);
				strcat(g_bootrecovercmd,strval);
				strcat(g_bootcmd,"\;");
				strcat(g_bootrecovercmd,"\;");
#endif

#ifndef CONFIG_NOT_USE_RAMDISK
#ifndef CONFIG_BOOT_MMC
				strcat(g_bootcmd,"nand read");   //for nand boot
#else
#if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT0)
				strcat(g_bootcmd,"mmc read 0");
#else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
				strcat(g_bootcmd,"mmc read 2");
#endif
#endif
#endif

#ifndef CONFIG_BOOT_MMC
				strcat(g_bootrecovercmd,"nand read");	 //for nand boot
#else
#if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT0)
				strcat(g_bootrecovercmd,"mmc read 0");
#else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
				strcat(g_bootrecovercmd,"mmc read 2");
#endif
#endif
			}

#ifndef	CONFIG_NOT_USE_RAMDISK
		if(0 == strcmp(p->szPartName, "rootfs"))
		{
#ifdef CONFIG_BOOT_MMC
			u64_to_u32(p->u8PartitionStartAddr,&valhigh,&vallow);
			blockoffset = vallow/512;
			strval = uitostr_hex(szVal,blockoffset);
			strcat(g_bootcmd," 0xC800000 0x");
			strcat(g_bootcmd,strval);

#ifdef READ_IMAGE_USE_REAL_SIZE
			vallow= (unsigned int)(p->u8RealDataSize/512);
#else
			vallow= (unsigned int)(p->u8PartitionSize/512);
#endif
			strval = uitostr_hex(szVal,vallow);
			strcat(g_bootcmd," 0x");
			strcat(g_bootcmd,strval);
#ifdef CONFIG_KERNEL_ZIMAGE
			sprintf(g_bootcmd + strlen(g_bootcmd), "\;bootz 0x%x", KERNEL_LOAD_ADDR);
#else
			strcat(g_bootcmd,"\;bootm 0x6000000");
#endif
#else //for nand boot
#ifdef CONFIG_KERNEL_ZIMAGE
			sprintf(g_bootcmd + strlen(g_bootcmd), " 0xC800000 rootfs\;bootz 0x%x", KERNEL_LOAD_ADDR);
#else
			strcat(g_bootcmd," 0xC800000 rootfs\;bootm 0x6000000");
#endif
#endif
		}
#endif

#ifdef ATC_AB_PARTITION_SUPPORT
		if ((ab_slot == 0 && (0 == strcmp(p->szPartName, "recovery_a")))
				|| (ab_slot == 1 && (0 == strcmp(p->szPartName, "recovery_b"))))
#else
			if(((bootfromAorB == 0) && (0 == strcmp(p->szPartName, "recovery")))
					|| ((bootfromAorB == 1) && (0 == strcmp(p->szPartName, "recovery_bk"))))
#endif
			{
#ifdef CONFIG_BOOT_MMC
				u64_to_u32(p->u8PartitionStartAddr,&valhigh,&vallow);
				blockoffset = vallow/512;
				strval = uitostr_hex(szVal,blockoffset);
				strcat(g_bootrecovercmd," 0xC800000 0x");
				strcat(g_bootrecovercmd,strval);

#ifdef READ_IMAGE_USE_REAL_SIZE
				vallow= (unsigned int)(p->u8RealDataSize/512);
#else
				vallow= (unsigned int)(p->u8PartitionSize/512);
#endif
				strval = uitostr_hex(szVal,vallow);
				strcat(g_bootrecovercmd," 0x");
				strcat(g_bootrecovercmd,strval);

#ifdef CONFIG_KERNEL_ZIMAGE
				sprintf(g_bootrecovercmd + strlen(g_bootrecovercmd), "\;bootz 0x%x", KERNEL_LOAD_ADDR);
#else
				strcat(g_bootrecovercmd,"\;bootm 0x6000000");
#endif

#else//for nand recovery mode
#ifdef READ_IMAGE_USE_REAL_SIZE
				vallow= (unsigned int)(p->u8RealDataSize);
#else
				vallow= (unsigned int)(p->u8PartitionSize);
#endif
				strval = uitostr_hex(szVal,vallow);

				if(0 == bootfromAorB)
				{
					strcat(g_bootrecovercmd," 0xC800000 recovery 0x");
				}
				else
				{
					strcat(g_bootrecovercmd," 0xC800000 recovery_bk 0x");
				}
				strcat(g_bootrecovercmd,strval);

#ifdef CONFIG_KERNEL_ZIMAGE
				sprintf(g_bootrecovercmd + strlen(g_bootrecovercmd), "\;bootz 0x%x", KERNEL_LOAD_ADDR);
#else
				strcat(g_bootrecovercmd,"\;bootm 0x6000000");
#endif
#endif
			}

		strcat(propvaluetest,"(");
		strcat(propvaluetest,p->szPartName);
		strcat(propvaluetest,")");

#ifndef CONFIG_BOOT_MMC
		strcat(mtd_parts,"(");
		strcat(mtd_parts,p->szPartName);
		strcat(mtd_parts,")");
#endif
#ifndef CONFIG_SECURITY_UPGRADE
		if(0 == strcmp(p->szPartName, "misc"))
		{
			g_miscPartitonAddr = p->u8PartitionStartAddr;
		}
#else
		if(0 == strcmp(p->szPartName, "datazone"))
		{
			g_miscPartitonAddr = p->u8PartitionStartAddr + 0x1000;
		}
#endif

#ifdef ATC_AB_PARTITION_SUPPORT
		if ((ab_slot == 0 && (0 == strcmp(p->szPartName, "trustzone_a")))
				|| (ab_slot == 1 && (0 == strcmp(p->szPartName, "trustzone_b"))))
#else
			if((bootfromAorB == 0 && (0 == strcmp(p->szPartName, "trustzone")))
					|| ((bootfromAorB == 1) && (0 == strcmp(p->szPartName, "trustzone_bk"))))
#endif
			{
				g_tzPartitonAddr = p->u8PartitionStartAddr;

#ifdef READ_IMAGE_USE_REAL_SIZE
				g_tzPartitonSize = p->u8RealDataSize;
#ifndef CONFIG_BOOT_MMC
				g_tzPartitonSize = ALIGN(g_tzPartitonSize, nand0->writesize);
#endif
#else
				g_tzPartitonSize = p->u8PartitionSize;
#endif
			}

#ifdef ATC_AB_PARTITION_SUPPORT
		if ((ab_slot == 0 && (0 == strcmp(p->szPartName, "dtb_a")))
				|| (ab_slot == 1 && (0 == strcmp(p->szPartName, "dtb_b"))))
#else
			if((bootfromAorB == 0 && (0 == strcmp(p->szPartName, "dtb")))
					|| ((bootfromAorB == 1) && (0 == strcmp(p->szPartName, "dtb_bk"))))
#endif
			{
				g_dtbPartitonAddr = p->u8PartitionStartAddr;

#ifdef READ_IMAGE_USE_REAL_SIZE
				g_dtbPartitonSize = p->u8RealDataSize;
#ifndef CONFIG_BOOT_MMC
				g_dtbPartitonSize = ALIGN(g_dtbPartitonSize, nand0->writesize);
#endif
#else
				g_dtbPartitonSize = p->u8PartitionSize;
#endif
			}

#ifdef ATC_AB_PARTITION_SUPPORT
		if ((ab_slot == 0 && (0 == strcmp(p->szPartName, "arm2_a")))
				|| (ab_slot == 1 && (0 == strcmp(p->szPartName, "arm2_b"))))
#else
			if(((bootfromAorB == 0) && (0 == strcmp(p->szPartName, "arm2")))
					|| (bootfromAorB == 1 && (0 == strcmp(p->szPartName, "arm2_bk"))))
#endif
			{
				g_arm2PartitonAddr = p->u8PartitionStartAddr;
#ifdef READ_IMAGE_USE_REAL_SIZE
				g_arm2PartitonSize = p->u8RealDataSize;
#ifndef CONFIG_BOOT_MMC
				g_arm2PartitonSize = ALIGN(g_arm2PartitonSize, nand0->writesize);
#endif
#else
				g_arm2PartitonSize = p->u8PartitionSize;
#endif
			}

#ifdef ATC_AB_PARTITION_SUPPORT
		if ((ab_slot == 0 && 0 == strcmp(p->szPartName, "system_a"))
			|| (ab_slot == 1 && 0 == strcmp(p->szPartName, "system_b")))
#else
		if (0 == strcmp(p->szPartName, "system"))
#endif
		{
			g_systemIndex = index;
			printf("system partition index is %u\n", g_systemIndex);
		}

		if(0 == strcmp(p->szPartName, "logo"))
		{
			g_logoPartitonAddr = p->u8PartitionStartAddr;

#ifdef READ_IMAGE_USE_REAL_SIZE
			g_logoPartitonSize = p->u8RealDataSize;
#ifndef CONFIG_BOOT_MMC
			g_logoPartitonSize = ALIGN(g_logoPartitonSize, nand0->writesize);
#endif
#else
			g_logoPartitonSize = p->u8PartitionSize;
#endif
		}

		if(0 == strcmp(p->szPartName, "boot_misc"))
		{
			g_bootmiscPartitonAddr = p->u8PartitionStartAddr;

#ifdef READ_IMAGE_USE_REAL_SIZE
			g_bootmiscPartitonSize = p->u8RealDataSize;
#ifndef CONFIG_BOOT_MMC
			g_bootmiscPartitonSize = ALIGN(g_dvpPartitonSize, nand0->writesize);
#endif
#else
			g_bootmiscPartitonSize = p->u8PartitionSize;
#endif
		}

		if(0 == strcmp(p->szPartName, "metazone"))
		{
			g_metazonePartitonAddr = p->u8PartitionStartAddr;
#ifdef READ_IMAGE_USE_REAL_SIZE
			g_metazonePartitonSize = p->u8RealDataSize;
#else
			g_metazonePartitonSize = p->u8PartitionSize;
#endif
		}

		if(0 == strcmp(p->szPartName, "dvp"))
		{
			g_dvpPartitonAddr = p->u8PartitionStartAddr;

#ifdef READ_IMAGE_USE_REAL_SIZE
			g_dvpPartitonSize = p->u8RealDataSize;
#ifndef CONFIG_BOOT_MMC
			g_dvpPartitonSize = ALIGN(g_dvpPartitonSize, nand0->writesize);
#endif
#else
			g_dvpPartitonSize = p->u8PartitionSize;
#endif
		}


		if(0 == strcmp(p->szPartName, "env"))
		{
			g_envpartitionoffset = p->u8PartitionStartAddr;
		}

		if (p->u4Mount == 0)
			strcat(propvaluetest,"0");
		else
			strcat(propvaluetest,"1");
		//mtdparts do not need mount info
#ifdef CONFIG_USRDATA_EXT4
		if(p->nextpartition) {
			strcat(propvaluetest,",");
#ifndef CONFIG_BOOT_MMC
			strcat(mtd_parts,",");
#endif
		}
#else
		strcat(propvaluetest,",");
#ifndef CONFIG_BOOT_MMC
		strcat(mtd_parts,",");
#endif

#endif
		p = p->nextpartition;
		++index;
	}

#ifdef CONFIG_NOT_USE_RAMDISK
	//both for nand and emmc
#ifdef CONFIG_KERNEL_ZIMAGE
	sprintf(g_bootcmd + strlen(g_bootcmd), "bootz 0x%x", KERNEL_LOAD_ADDR);
#else
	strcat(g_bootcmd,"bootm 0x6000000");
#endif
#endif

#ifndef CONFIG_USRDATA_EXT4
	strcat(propvaluetest,"-(usrdata)1");
#ifndef CONFIG_BOOT_MMC
	//strcat(mtd_parts,"-(usrdata)");
	mtd_parts[strlen(mtd_parts)-1] = '\0';
#endif
#endif

#ifndef CONFIG_BOOT_MMC
	g_mtdparts = mtd_parts;
	printf("check_partition mtdparts: %s\r\n",g_mtdparts);
#endif
	printf("check_partition part: %s\r\n",propvaluetest);
	printf("check_partition bootcmd: %s\r\n",g_bootcmd);
	printf("check_partition bootrecoverycmd: %s\r\n",g_bootrecovercmd);

	printf("check_partition mtdparts len: %d\r\n",strlen(g_mtdparts));
	printf("check_partition propvaluetest len: %d\r\n",strlen(propvaluetest));

	if(strlen(g_mtdparts) > 600){
		printf("Error check_partition g_mtdparts > 600. \r\n");
	}

	if(strlen(propvaluetest) > 1024){
		printf("Error check_partition propvaluetest > 1024. \r\n");
	}

	return (char *)propvaluetest; //return parts
}


//make mtdparts with part info from sd card
char *make_mtdparts_for_nand_upgrade(struct mmc *mmc, int dev_num, u64 start_addr, int *error)
{
	partitioninfo *part;
	int n;
	char szVal[60];
	char buf[512];

	char *mtdparts;
	mtdparts = (char *)malloc(600);
	memset(mtdparts, 0 ,600);

	strcat(mtdparts,"mtdparts=atcnand:");

	//printf("make mtdparts begin\r\n");
	do
	{
		flush_invalid_cache(buf, 512);
		n = mmc->block_dev.block_read(dev_num, start_addr, 1, buf);
		if (n == -1)
		{
			printf("get partition info from SD fail!\r\n");
			*error = -1;//get part info fail, update fail
			return NULL;
		}
		if(1 != n)
		{
			printf("block_read fail, result=%d\r\n", n);
			if (1 == msdc_get_cd(dev_num))
				*error = -2;//no eMMC device or SD device find
			return NULL;
		}
		part = (partitioninfo *)buf;

		memset(szVal, 0, 60);

		/*  //do not neet mount info in mtdparts
		    sprintf(szVal, "0x%x%08x@0x%x%08x(%s)%d,",
		    (unsigned int)(part->u8PartitionSize>> 32),
		    (unsigned int)(part->u8PartitionSize),
		    (unsigned int)(part->u8PartitionStartAddr >> 32),
		    (unsigned int)(part->u8PartitionStartAddr),
		    part->szPartName,
		    part->u4Mount);
		 */

		sprintf(szVal, "0x%x%08x@0x%x%08x(%s),",
				(unsigned int)(part->u8PartitionSize>> 32),
				(unsigned int)(part->u8PartitionSize),
				(unsigned int)(part->u8PartitionStartAddr >> 32),
				(unsigned int)(part->u8PartitionStartAddr),
				part->szPartName);
		strcat(mtdparts,szVal);

		start_addr = part->u8OffsetNextImage/512;

	}while(part->u8OffsetNextImage != 0);


	//strcat(mtdparts,"-(usrdata)");
	mtdparts[strlen(mtdparts)-1] = '\0';

	printf("make mtdparts end\r\nmtdparts=%s\r\n", mtdparts);
	*error = 0;
	return mtdparts;
}


/*******************************************************************************
 * read partition info for partial image upgrade/copy upgrade
 * to sync the RealImageSize in partition table
 ********************************************************************************/
//partitionhead *g_partitionhead = NULL;
//partitionread *g_partitionread = NULL;

int update_image_size_in_partition_table(partitioninfo *part)
{
	partitionread *p;
	p = g_partitionread;
	int flag = 0;
	if(p == NULL)
	{
		printf("partiton info is null\r\n");
		return -1;
	}

	while(p)
	{
		if (strcmp(part->szPartName, p->szPartName) == 0)
		{
			p->u8RealDataSize = part->u8RealDataSize;
			flag = 1;
			break;
		}

		p = p->nextpartition;
	}

	if (flag == 1)
	{
		printf("update %s partition info success\r\n", part->szPartName);
	}
	else
	{
		printf("WARNING:partition %s is not exist in partition table,this may lead to unexpected result\r\n", part->szPartName);
		return -1;
	}


	return 0;

}

//  for fastboot update real image size when flashing image
int update_image_size_in_partition_table_by_partition_name(char *part_name, unsigned long long u8RealDataSize)
{
	partitionread *p;
	p = g_partitionread;
	int flag = 0;
	if(p == NULL)
	{
		printf("partiton info is null\r\n");
		return -1;
	}

	while(p)
	{
		if (strcmp(part_name, p->szPartName) == 0)
		{
			p->u8RealDataSize = u8RealDataSize;
			flag = 1;
			break;
		}

		p = p->nextpartition;
	}

	if (flag == 1)
	{
		printf("update %s partition info success\r\n", part_name);
	}
	else
	{
		printf("WARNING:partition %s is not exist in partition table,this may lead to unexpected result\r\n", part_name);
		return -1;
	}


	return 0;

}
