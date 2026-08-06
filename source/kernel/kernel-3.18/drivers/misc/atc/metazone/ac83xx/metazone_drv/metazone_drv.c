/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/


#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_address.h>
#include <linux/of_reserved_mem.h>
#include <linux/dma-mapping.h>
#ifdef CONFIG_MTD_NAND_ATC
#include <linux/mtd/mtd.h>
#endif

#include "x_os.h"
#include "winutil.h"
#include "windows.h"

#include "metazone_inter.h"
#include "metazone.h"
#include "metazone_ioctl.h"

#include "drv_thread.h"
#include "x_ver.h"
#include "metazone_default_table.h"

#define KERNEL_STANDARD_API
#ifdef KERNEL_STANDARD_API
#include <linux/kthread.h>
#include "x_os.h"	/* event operatioen API define header file */

static struct task_struct *mtz_thread_task;
#endif

#define MTZ_MODE_NAME		("MTZ")
#define MTZ_VER_MAJOR		(01)
#define MTZ_VER_MINOR		(01)
#define MTZ_VER_REV			(00)
#define MTZ_MISC_DVR_NAME	("mtz") // /dev/mtz
#define MTZ_MAX_SIZE		(0x10000)	// Phy storage media size for each metazone, in byte

struct resvd_mem_info {
	unsigned long phys_addr;
	unsigned long virt_addr;
	unsigned long size;
};

typedef struct mtz_dev_info {
	struct miscdevice cdev;	/* Char device structure */
	struct device *dev;
	struct resvd_mem_info reserved_mem_info;
} mtzdev_info;

struct resvd_mem_info *g_mtz_rsvmem_info = NULL;

//#ifdef NAND_BOOT
//#ifdef CONFIG_MTD_NAND_ATC
//int mtd_write_metazone(unsigned char *pbBuffer, unsigned int u4Size);
//#endif

#define ATC_KERNEL_LINUX_LICENSE     "GPL"
#define MTZ_DRV_THREAD_STACK_SIZE	(2)

#ifdef KERNEL_STANDARD_API
#define MTZ_DRV_THREAD_PRIORITY			(81)
#else
#define MTZ_DRV_THREAD_PRIORITY			PRIORITY(PRIORITY_CLASS_REALTIME, PRIORITY_LAYER_TIME_CRITICAL, 0)
#endif

bool _fgWritableZoneInited = false;
void *_pMetaZone = NULL;
TMetaZone *_pMetaHeader = NULL;
char *_pbReserve = NULL;
u32 *_pu4Value = NULL;
u32 _u4BinaryStart = 0;
static bool _fgWritableZoneDirty;

#ifdef CONFIG_MTZ_ANDROID
#define USE_RESERVED_MEMORY			(1) // 0: dynamic  1: static   [ dts reserved memory address allocate ]
#else

//#ifdef NAND_BOOT
#ifdef CONFIG_MTD_NAND_ATC
#define USE_RESERVED_MEMORY			(1)
extern int nand_get_blocksize(char *name);
u32 nandBlockSize =0;

static u32 mtz_slot_count;
static u32 mtz_slots_per_block;
static u32 mtz_current_slot_idx;
static u32 mtz_prev_slot_idx;
static u32 mtz_current_seq;
static u32 mtz_active_blocks;
static u32 mtz_block_size;
static u32 mtz_data_blocks[2];
static u32 mtz_metadata_block;
static u8 mtz_slot_map[MTZ_MAX_SLOTS];  // 0: empty, 1: occupied
static struct mtd_info *mtz_mtd;

#else
#define MTZ_PARTITION_SIZE  (0x20000)  /*256KB*/
#define USE_RESERVED_MEMORY			(0)
#endif

#endif

#ifndef KERNEL_STANDARD_API
static HANDLE_T _hMtzFlushThread;
#endif

#define METAZONE_FS_SIZE			(0x10000)	/* 64KB */
#define METAZONE_FS_BINARY_OFFSET	(10240)

static u32 _u4FsBinaryStart;
static void *_pFsMetaZone;
static bool _fgFileZoneInited;
static struct mutex _Lock;
static struct semaphore _rSema;
#define FLUSH_EVENT				("MTZ_FLUSH")

#define Lock()		mutex_lock(&_Lock)
#define Unlock()	mutex_unlock(&_Lock)

#define MTZ_PARTITION_ANDROID  "/dev/block/metazone"
#define MTZ_PARTITION_LINUX  "/dev/metazone"

extern s32 get_static_reserved_memory(const char *uname, phys_addr_t *base, phys_addr_t *size);

struct RES_MEM {
    u32 start;
    u32 size;
};

static struct RES_MEM MTZmem = {0,0};
Wflag p1;
Wflag p2;

#ifdef CONFIG_MTD_NAND_ATC
static UINT16 calculate_CRC16(const UINT8 *pbuff, UINT32 length)
{
    static const UINT16 crc16_ccitt_table[256] = {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
        0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
        0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
        0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
        0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
        0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
        0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
        0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
        0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
        0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
        0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
        0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
        0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
        0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
        0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
        0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
        0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
        0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
        0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
        0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
        0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
        0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
        0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
    };
    UINT16 crc = 0xFFFF;
    UINT32 i;

    if (!pbuff || length == 0) {
        pr_info("[MTZ] CRC input invalid\r\n");
        return (UINT16)0xFFFF;
    }

    for (i = 0; i < length; i++) {
        UINT8 idx = (UINT8)((crc >> 8) ^ pbuff[i]);
        crc = (UINT16)((crc << 8) ^ crc16_ccitt_table[idx]);
    }

    return crc;
}

static SlotHeader *mtz_get_slot_header_rsv_addr(void)
{
	SlotHeader *header;

	if (_pMetaZone == NULL) {
		pr_err("[MTZ]mtz_get_slot_header_rsv_addr: _pMetaZone is NULL\n");
		return NULL;
	}

	header = (SlotHeader *)((char *)_pMetaZone + MTZ_SLOT_SIZE - MTZ_SLOT_HEADER_SIZE);
	return header;
}

static unsigned short recount_header_crc(const TMetaZone *pMetaHeader)
{
	unsigned short reCRC;
	SlotHeader *header;

	if (pMetaHeader == NULL) {
		pr_err("[MTZ] header CRC input is invalid \n");
		return (unsigned short)-1;
	}
	reCRC = calculate_CRC16((unsigned char *)pMetaHeader, (unsigned short)sizeof(TMetaZone));
	header = mtz_get_slot_header_rsv_addr();
	if (!header)
		return -1;
	header->crc.crc_header = reCRC;
	pr_info("[MTZ] Header CRC recount (%u)\n", reCRC);
	return reCRC;
}

static int check_header_crc(const TMetaZone *pMetaHeader)
{
	SlotHeader *header;
	unsigned short reCRC;

	if (_pMetaZone == NULL)
		return -1;

	header = mtz_get_slot_header_rsv_addr();
	if (!header || header->state != 0xFF)
		return -1;

	reCRC = calculate_CRC16((unsigned char *)pMetaHeader, (unsigned short)sizeof(TMetaZone));
	if (header->crc.crc_header != reCRC) {
		pr_err("[MTZ] Check header CRC  is wrong, expected: %u, calculated: %u\n", header->crc.crc_header, reCRC);
		return -1;
	}

	return 0;
}

static int recount_CRC_value(int type)
{
	unsigned short reCRC;
	SlotHeader *header;

	if (_pMetaZone == NULL) {
		pr_err("[MTZ] CRC recount failed: metazone memory is NULL\n");
		return -1;
	}

	header = mtz_get_slot_header_rsv_addr();
	if (!header) {
		pr_err("[MTZ] CRC recount failed: slot header invalid\n");
		return -1;
	}

	Lock();
    switch(type){
	  case 1:
		 reCRC = calculate_CRC16((char *)(_pu4Value + 1) , (unsigned short)(_pMetaHeader->dwValueNum - 1)*4);       
		 header->crc.crc_dword = reCRC;
		 pr_info("[MTZ] Dword CRC recount (%u)\n", reCRC);
		 break;

      case 2:
		 reCRC = calculate_CRC16((char *)_u4BinaryStart , (unsigned short)((_pMetaHeader->dwBinaryItemSize + 4) 
		                          * _pMetaHeader->dwBinaryNum));
		 header->crc.crc_binary = reCRC;
		 pr_info("[MTZ] Binary CRC recount (%u)\n", reCRC);
		 break;

      case 3:
		 reCRC = calculate_CRC16(_pbReserve , (unsigned short)(_pMetaHeader->dwReserveSize));
		 header->crc.crc_reserved = reCRC;
		 pr_info("[MTZ] Reserved CRC recount (%u)\n", reCRC);
		 break;

	  default:
	  	 pr_err("[MTZ] CRC recount type [%d] is wrong \n", type);
		Unlock();
		 return -1;
    }

	Unlock();
	return 0;
}

static int check_dword_crc_region(const TMetaZone *pMetaHeader, const u32 *pu4Value,
	unsigned short expected_crc, const char *tag)
{
	unsigned short reCRC;

	reCRC = calculate_CRC16((unsigned char *)(pu4Value + 1),
		(unsigned short)((pMetaHeader->dwValueNum - 1)* 4));
	if (expected_crc != reCRC) {
		pr_err("[MTZ] Check dword CRC  is wrong, expected: %u, calculated: %u\n", expected_crc, reCRC);
		return -1;
	}

	return 0;
}

static int check_binary_crc_region(const TMetaZone *pMetaHeader, u32 u4BinaryStart,
	unsigned short expected_crc, const char *tag)
{
	unsigned short reCRC;

	reCRC = calculate_CRC16((unsigned char *)u4BinaryStart,
		(unsigned short)((pMetaHeader->dwBinaryItemSize + 4) * pMetaHeader->dwBinaryNum));
	if (expected_crc != reCRC) {
		pr_err("[MTZ] Check binary CRC  is wrong, expected: %u, calculated: %u\n", expected_crc, reCRC);
		return -1;
	}

	return 0;
}

static int check_reserved_crc_region(const TMetaZone *pMetaHeader, const char *pbReserve,
	unsigned short expected_crc, const char *tag)
{
	unsigned short reCRC;

	reCRC = calculate_CRC16((unsigned char *)pbReserve,
		(unsigned short)pMetaHeader->dwReserveSize);
	if (expected_crc != reCRC) {
		pr_err("[MTZ] Check reserved CRC  is wrong, expected: %u, calculated: %u\n", expected_crc, reCRC);
		return -1;
	}

	return 0;
}


static int check_CRC_flag1(void)
{
	SlotHeader *header;

	if (_pMetaZone == NULL)
		return -1;

	header = mtz_get_slot_header_rsv_addr();
	if (!header || header->state != 0xFF)
		return -1;

	if (check_dword_crc_region(_pMetaHeader, _pu4Value, header->crc.crc_dword, "dwValue1") != 0)
		return -1;

	if (check_binary_crc_region(_pMetaHeader, _u4BinaryStart, header->crc.crc_binary, "dwBinary1") != 0)
		return -1;

	if (check_reserved_crc_region(_pMetaHeader, _pbReserve, header->crc.crc_reserved, "dwReserved1") != 0)
		return -1;

	return 0;
}

static int check_CRC_flag2(void)
{
	return check_CRC_flag1();
}

#else
static unsigned short calculate_CRC16(unsigned char* pbuff, unsigned short length)
{
	unsigned short shift, data, val;
    int i;
	shift = 0xffff;
    if(pbuff ==NULL || length <=0){
        pr_err("[MTZ] calculate_CRC input is invalid \n");
        return -1;
	}
	for(i = 0; i< length; i++){
       if(0 == (i%8))
          data = (*pbuff++) << 8;
	   val = shift ^ data;
       shift = shift << 1;
	   data = data << 1;
	   if(val & 0x8000)
	      shift = shift ^ 0x1021;
	}

    return shift;
}

static int recount_CRC_value(int type)
{
	unsigned short reCRC;
	char *position;
	char *pbBuffer;
	char *pbCRCstart = (char *)((unsigned int)_pMetaZone + MTZ_PARTITION_SIZE/2-0x200 + sizeof(Wflag));

    switch(type){
	  case 1:
		 reCRC = calculate_CRC16((char *)(_pu4Value + 1) , (unsigned short)(_pMetaHeader->dwValueNum)*4);       
         position = pbCRCstart;
		 pr_info("[MTZ] Dword CRC recount (%u)\n", reCRC);
		 goto update;

      case 2:
		 reCRC = calculate_CRC16((char *)_u4BinaryStart , (unsigned short)((_pMetaHeader->dwBinaryItemSize + 4) 
		                          * _pMetaHeader->dwBinaryNum));
		 position = pbCRCstart + 2;
		 pr_info("[MTZ] Binary CRC recount (%u)\n", reCRC);
		 goto update;

      case 3:
		 reCRC = calculate_CRC16(_pbReserve , (unsigned short)(_pMetaHeader->dwReserveSize));
         position = pbCRCstart + 4;
		 pr_info("[MTZ] Reserved CRC recount (%u)\n", reCRC);
		 goto update;

	  default:
	  	 pr_err("[MTZ] CRC recount type [%d] is wrong \n", type);
		 return -1;
    }

update:
	pbBuffer = (char *)(&reCRC);
	Lock();
	memcpy(position, pbBuffer, 2);
	Unlock();

	return 0;
}


static int check_CRC_flag1(void)
{
	unsigned short reCRC;
	CRCflag CRC1;

	char *pbCRCstart = (char *)((unsigned int)_pMetaZone + MTZ_PARTITION_SIZE/2-0x200 + sizeof(Wflag));

	memcpy(&CRC1, pbCRCstart, sizeof(CRCflag));

	reCRC = calculate_CRC16((char *)(_pu4Value + 1) , (_pMetaHeader->dwValueNum)*4);
	if(CRC1.crc_dword != reCRC){
		pr_err("[MTZ] Check CRC dwValue1 is wrong \n");
		return -1;
	}

	reCRC = calculate_CRC16((char *)_u4BinaryStart , (_pMetaHeader->dwBinaryItemSize + 4)
			* _pMetaHeader->dwBinaryNum);
	if(CRC1.crc_binary != reCRC){
		pr_err("[MTZ] Check CRC dwBinary1 is wrong \n");
		return -1;
	}

	reCRC = calculate_CRC16(_pbReserve , _pMetaHeader->dwReserveSize);
	if(CRC1.crc_reserved != reCRC){
		pr_err("[MTZ] Check CRC dwReserved1 is wrong \n");
		return -1;
	}

	return 0;
}

static int check_CRC_flag2(void)
{
	unsigned short reCRC;
	CRCflag CRC2;
	char *pbCRCstart2 = (char *)((unsigned int)_pMetaZone + MTZ_PARTITION_SIZE-0x200 + sizeof(Wflag));
	TMetaZone *_pMetaHeader2 = (TMetaZone *)((char *)_pMetaHeader + MTZ_PARTITION_SIZE/2);
	u32 *_pu4Value2 = (u32 *)((char *)_pu4Value + MTZ_PARTITION_SIZE/2);
	u32 _u4BinaryStart2 = _u4BinaryStart + MTZ_PARTITION_SIZE/2;
	char *_pbReserve2 = _pbReserve + MTZ_PARTITION_SIZE/2;

	memcpy(&CRC2, pbCRCstart2, sizeof(CRCflag));

	reCRC = calculate_CRC16((char *)(_pu4Value2 + 1) , (_pMetaHeader2->dwValueNum)*4);
	if(CRC2.crc_dword != reCRC){
		pr_err("[MTZ] Check CRC dwValue2 is wrong \n");
		return -1;
	}

	reCRC= calculate_CRC16((char *)_u4BinaryStart2 , (_pMetaHeader2->dwBinaryItemSize + 4)
			* _pMetaHeader2->dwBinaryNum);
	if(CRC2.crc_binary != reCRC){
		pr_err("[MTZ] Check CRC dwBinary2 is wrong \n");
		return -1;
	}

	reCRC = calculate_CRC16(_pbReserve2 , _pMetaHeader2->dwReserveSize);
	if(CRC2.crc_reserved != reCRC){
		pr_err("[MTZ] Check CRC dwReserved2 is wrong \n");
		return -1;
	}

	return 0;
}

#endif

//#ifndef NAND_BOOT
#ifndef CONFIG_MTD_NAND_ATC
static int update_wflag(unsigned char sector,unsigned int offset)
{
	struct file *fp;
	mm_segment_t old_fs;
	loff_t pos;
	int ret;
	char *pbBuffer;
	unsigned u4Size=sizeof(Wflag);
	Lock();
	pbBuffer = (char *)kmalloc(u4Size, GFP_KERNEL);
	if (!pbBuffer) {
		pr_err("[MTZ]kmalloc pbBuffer fail \n");
	}
#ifdef CONFIG_MTZ_ANDROID
	fp = filp_open(MTZ_PARTITION_ANDROID, O_RDWR, 0644U);
	if (IS_ERR(fp)) {
		pr_err("[MTZ] open %s file error\n",MTZ_PARTITION_ANDROID);
		return -1;
	}
#else
	fp = filp_open(MTZ_PARTITION_LINUX , O_RDWR, 0644U);
	if (IS_ERR(fp)) {
		pr_err("[MTZ] open %s file error\n",MTZ_PARTITION_LINUX);
		return -1;
	}
#endif

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	if(sector==1){
		if(offset==1)
			(p1.write_before)++;
		else if(offset==2)
			(p1.write_after)++;
		memcpy(pbBuffer, &p1, sizeof(Wflag));
		pos = (MTZ_PARTITION_SIZE/2)-0x200;
		ret = vfs_write(fp, pbBuffer, u4Size, &pos);
	}
	else if(sector==2){
		if(offset==1)
			(p2.write_before)++;
		else if(offset==2)
			(p2.write_after)++;
		memcpy(pbBuffer, &p2, sizeof(Wflag));
		pos = MTZ_PARTITION_SIZE-0x200;
		ret = vfs_write(fp, pbBuffer, u4Size, &pos);
	}

	filp_close(fp, NULL);
	set_fs(old_fs);
	kfree(pbBuffer);
	Unlock();
	return 0;
}

static int update_CRC_flag(int i)
{
	struct file *fp;
	mm_segment_t old_fs;
	loff_t pos;
	int ret;
	char *pbBuffer;
	char *pbCRCstart;
	unsigned u4Size = sizeof(CRCflag);
	Lock();
	pbBuffer = (char *)kmalloc(u4Size, GFP_KERNEL);
	if (!pbBuffer) {
		pr_err("[MTZ]kmalloc pbBuffer fail \n");
	}

#ifdef CONFIG_MTZ_ANDROID
	fp = filp_open(MTZ_PARTITION_ANDROID, O_RDWR, 0644U);
	if (IS_ERR(fp)) {
		pr_err("[MTZ] open %s file error\n",MTZ_PARTITION_ANDROID);
		return -1;
	}
#else
	fp = filp_open(MTZ_PARTITION_LINUX , O_RDWR, 0644U);
	if (IS_ERR(fp)) {
		pr_err("[MTZ] open %s file error\n",MTZ_PARTITION_LINUX);
		return -1;
	}
#endif

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	if(1 == i){
		pbCRCstart = (char *)((unsigned int)_pMetaZone + MTZ_PARTITION_SIZE/2-0x200 + sizeof(Wflag));
		//memcpy(pbBuffer +sizeof(Wflag), pbCRCstart, sizeof(CRCflag));
		memcpy(pbBuffer, pbCRCstart, sizeof(CRCflag));

		pos = (MTZ_PARTITION_SIZE/2)-0x200+sizeof(Wflag);
		ret = vfs_write(fp, pbBuffer, u4Size, &pos);
	}
	else if(2 == i){
		pbCRCstart = (char *)((unsigned int)_pMetaZone + MTZ_PARTITION_SIZE - 0x200 + sizeof(Wflag));
		//memcpy(pbBuffer +sizeof(Wflag), pbCRCstart, sizeof(CRCflag));
		memcpy(pbBuffer, pbCRCstart, sizeof(CRCflag));

		pos = MTZ_PARTITION_SIZE-0x200+sizeof(Wflag);
		ret = vfs_write(fp, pbBuffer, u4Size, &pos);
	}

	filp_close(fp, NULL);
	set_fs(old_fs);
	Unlock();
	return 0;
}


static int write_metazone1(const char *pbBuffer, unsigned long u4Size)
{
	struct file *fp;
	mm_segment_t old_fs;
	loff_t pos;
	int ret;

	char mtz_path[256];
		
#ifdef CONFIG_MTZ_ANDROID
	strcpy(mtz_path,MTZ_PARTITION_ANDROID);
#else
	strcpy(mtz_path,MTZ_PARTITION_LINUX);
#endif

	fp = filp_open(mtz_path, O_RDWR, 0644U);
	if (IS_ERR(fp)) {
		pr_err("[MTZ] open %s file error\n",mtz_path);
		return -1;
	}
	//mdelay(50000);
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	pos = 0x0;
	ret = vfs_write(fp, pbBuffer, u4Size, &pos);

	filp_close(fp, NULL);
	set_fs(old_fs);

	return 0;
}

static int write_metazone2(const char *pbBuffer, unsigned long u4Size)
{
	struct file *fp;
	mm_segment_t old_fs;
	loff_t pos;
	int ret;
	char mtz_path[256];
	
#ifdef CONFIG_MTZ_ANDROID
	strcpy(mtz_path,MTZ_PARTITION_ANDROID);
#else
	strcpy(mtz_path,MTZ_PARTITION_LINUX);
#endif

	fp = filp_open(mtz_path, O_RDWR, 0644U);
	if (IS_ERR(fp)) {
		pr_err("[MTZ] open %s file error\n",mtz_path);
		return -1;
	}
	//mdelay(50000);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	pos = MTZ_PARTITION_SIZE/2;
	ret = vfs_write(fp, pbBuffer, u4Size, &pos);

	filp_close(fp, NULL);
	set_fs(old_fs);

	return 0;
}

static int mmc_write_metazone(const char *pbBuffer, unsigned long u4Size)
{
	/*before write to metazone 1*/
	update_wflag(1,1);
	
	write_metazone1(pbBuffer,u4Size);
	/*after write to metazone 1*/
	update_CRC_flag(1);

	update_wflag(1,2);
	
	pr_info("[MTZ] metazone update_wflag \n");
	/*before write to metazone 2*/
	update_wflag(2,1);
	
	write_metazone2(pbBuffer,u4Size);
	/*after write to metazone 2*/
	update_CRC_flag(2);

	update_wflag(2,2);
	
	return 0;
}


static int mmc_read_metazone(char *pbBuffer, unsigned long u4Size)
{
	struct file *fp;
	mm_segment_t old_fs;
	loff_t pos;
	int ret;
	char mtz_path[256];

#ifdef CONFIG_MTZ_ANDROID
	strcpy(mtz_path,MTZ_PARTITION_ANDROID);
#else
	strcpy(mtz_path,MTZ_PARTITION_LINUX);
#endif

	fp = filp_open(mtz_path, O_RDWR, 0644U);
	if (IS_ERR(fp)) {
		pr_err("[MTZ] open %s file error\n",mtz_path);
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	pos = 0x0;
	ret = vfs_read(fp, pbBuffer, u4Size, &pos);
	if (ret != u4Size) {
		pr_err("[MTZ] read metazone from %s fail\n",mtz_path);
		filp_close(fp, NULL);
		set_fs(old_fs);
		return -1;
	}

	filp_close(fp, NULL);
	set_fs(old_fs);

	return 0;
}
#else
extern int nand_isbad_block(char *name, int offset);
extern int nand_markbad_block(char *name, int offset);

static int mtz_find_next_good_slot(u32 *slot_idx)
{
	u32 i;
	for (i = 0; i < mtz_slot_count; i++) {
		u32 idx = (mtz_current_slot_idx + i + 1) % mtz_slot_count;
		if (mtz_slot_map[idx] == 0) {
			pr_info("index:%d is good slot\n", idx);
			*slot_idx = idx;
			return 0;
		}
	}
	return -1;
}

static inline u32 mtz_slot_data_addr(u32 slot_idx)
{
	u32 slot_in_block = slot_idx % mtz_slots_per_block;
	u32 slot_block = (slot_idx < mtz_slots_per_block) ? mtz_data_blocks[0] : mtz_data_blocks[1];
	return slot_block * mtz_block_size + slot_in_block * MTZ_SLOT_SIZE;
}

static inline u32 mtz_slot_header_addr(u32 slot_idx)
{
	size_t page_size = mtz_mtd ? mtz_mtd->writesize : 0;

	if (page_size == 0 || page_size > MTZ_SLOT_SIZE)
		return mtz_slot_data_addr(slot_idx) + MTZ_SLOT_SIZE - MTZ_SLOT_HEADER_SIZE;

	if ((mtz_slot_data_addr(slot_idx) + MTZ_SLOT_SIZE )% page_size != 0) {
		pr_err("[MTZ] slot %u data region is not aligned with page size\n", slot_idx);
		return mtz_slot_data_addr(slot_idx) + MTZ_SLOT_SIZE - MTZ_SLOT_HEADER_SIZE;
	}

	return mtz_slot_data_addr(slot_idx) + MTZ_SLOT_SIZE - page_size;
}

static int mtz_read_slot_header(u32 slot_idx, SlotHeader *header)
{
	u32 addr_header;
	size_t retlen;
	int ret;
	size_t page_size = mtz_mtd->writesize;
	u8 *page_buf;

	if (!header)
		return -1;

	if (page_size == 0 || page_size > MTZ_SLOT_SIZE)
		return -1;

	addr_header = mtz_slot_header_addr(slot_idx);
	if (addr_header % page_size != 0) {
		pr_err("[MTZ] slot header address is not aligned with page size\n");
		return -1;
	}

	page_buf = kmalloc(page_size, GFP_KERNEL);
	if (!page_buf)
		return -1;

	ret = mtz_mtd->_read(mtz_mtd, addr_header, page_size, &retlen, page_buf);
	if (ret || retlen != page_size) {
		kfree(page_buf);
		return -1;
	}

	memcpy(header, page_buf + (page_size - MTZ_SLOT_HEADER_SIZE), sizeof(SlotHeader));
	kfree(page_buf);

	return 0;
}

static int mtz_init_mtd_layout(void)
{
	mtz_mtd = get_mtd_device_nm("metazone");
	if (IS_ERR(mtz_mtd)) {
		pr_err("[MTZ] get mtd device fail\n");
		return -1;
	}
	mtz_block_size = mtz_mtd->erasesize;
	if (mtz_block_size < MTZ_SLOT_SIZE) {
		pr_err("[MTZ] invalid block size %u\n", mtz_block_size);
		return -1;
	}

	mtz_slots_per_block = mtz_block_size / MTZ_SLOT_SIZE;
	mtz_active_blocks = 0;
	mtz_slot_count = 0;

	memset(mtz_slot_map, 0, sizeof(mtz_slot_map));

	{
		u32 total_blocks = mtz_mtd->size / mtz_block_size;
		u32 found = 0;
		u32 i;

		for (i = 0; i < total_blocks && found < MTZ_ACTIVE_BLOCKS; i++) {
			if (mtz_mtd->_block_isbad(mtz_mtd, i * mtz_block_size)) {
				pr_err("[MTZ] block %d is bad block\n", i);
				continue;
			}

			mtz_data_blocks[found] = i;
			found++;
		}

		if (found == 0) {
			pr_err("[MTZ] cannot find any good block\n");
			return -1;
		}

		mtz_active_blocks = found;
		mtz_slot_count = mtz_active_blocks * mtz_slots_per_block;
		if (mtz_slot_count > MTZ_MAX_SLOTS)
			mtz_slot_count = MTZ_MAX_SLOTS;
		pr_info("[MTZ] mtd layout init complete, block size: %u, slots per block: %u, total slots: %u\n",
			mtz_block_size, mtz_slots_per_block, mtz_slot_count);
		if (mtz_active_blocks == 1)
			pr_info("[MTZ] single-block mode enabled\n");
	}
	return 0;
}

static inline bool mtz_seq_after(u32 seq_a, u32 seq_b)
{
	/*
	 * Wrap-around safe sequence comparison:
	 * returns true when seq_a is logically after seq_b even if u32 overflows.
	 * Example: seq_b=0xFFFFFFFE, seq_a=0x00000001 => seq_a is newer.
	 */
	return (s32)(seq_a - seq_b) > 0;
}

static void mtz_scan_slots_for_latest(void)
{
	u32 i;
	u32 best_seq = 0;
	u32 prev_seq = 0;
	bool best_valid = false; /* best_seq initialized by a real slot */
	bool prev_valid = false; /* prev_seq initialized by a real slot */

	mtz_prev_slot_idx = MTZ_MAX_SLOTS;
	mtz_current_slot_idx = 0;
	mtz_current_seq = 0;

	for (i = 0; i < mtz_slot_count; i++) {
		SlotHeader header;
		int ret = mtz_read_slot_header(i, &header);
		if (ret == 0 &&
		    header.magic == METAZONE_SLOT_MAGIC && header.state == 0xFF) {
			mtz_slot_map[i] = 1;
			if (!best_valid) {
				pr_info("[MTZ] found newer slot %u with seq %u\n", i, header.seq);
				best_seq = header.seq;
				mtz_current_slot_idx = i;
				best_valid = true;
				continue;
			}
			if (mtz_seq_after(header.seq, best_seq)) {
				prev_seq = best_seq;
				mtz_prev_slot_idx = mtz_current_slot_idx;
				prev_valid = true;
				best_seq = header.seq;
				mtz_current_slot_idx = i;
				pr_info("[MTZ] mtz_seq_after found newer slot %u with seq %u\n", i, header.seq);
			} else if (!prev_valid || mtz_seq_after(header.seq, prev_seq)) {
				prev_seq = header.seq;
				mtz_prev_slot_idx = i;
				prev_valid = true;
			}
		} else {
			mtz_slot_map[i] = 0;
		}
	}

	if (best_valid)
		mtz_current_seq = best_seq;
	pr_info("[MTZ] scan slots complete, current slot: %u, previous slot: %u, with seq %u\n", mtz_current_slot_idx, mtz_prev_slot_idx, mtz_current_seq);
}

static int mtz_write_slot_header_page(u32 slot_idx, const SlotHeader *header, size_t page_size)
{
	u32 page_addr;
	u8 *page_buf;
	size_t retlen;
	int ret;

	if (page_size == 0 || page_size > MTZ_SLOT_SIZE) {
		pr_err("[MTZ] invalid page size %zu\n", page_size);
		return -1;
	}

	page_addr = mtz_slot_data_addr(slot_idx) + MTZ_SLOT_SIZE - page_size;
	if ((page_addr % page_size) != 0) {
		pr_err("[MTZ] slot header page address is not aligned with page size\n");
		return -1;
	}
	page_buf = kmalloc(page_size, GFP_KERNEL);
	if (!page_buf) {
		pr_err("[MTZ] alloc page buffer fail\n");
		return -1;
	}
	pr_info("[MTZ] write slot header page, slot %u, page addr: 0x%x, page size: %zu\n",
		slot_idx, page_addr, page_size);
	memset(page_buf, 0, page_size);
	memcpy(page_buf + (page_size - MTZ_SLOT_HEADER_SIZE), header, sizeof(SlotHeader));
	ret = mtz_mtd->_write(mtz_mtd, page_addr, page_size, &retlen, page_buf);
	kfree(page_buf);
	if (ret || retlen != page_size) {
		pr_err("[MTZ] update slot header fail\n");
		return -1;
	}

	return 0;
}

static int mtz_erase_block(u32 block_idx)
{
	struct erase_info erase;
	u32 addr = block_idx * mtz_block_size;

	if (!mtz_mtd || !mtz_mtd->_erase) {
		pr_err("[MTZ] erase skipped: mtd or erase op is NULL\n");
		return -1;
	}

	if (mtz_mtd->erasesize == 0 || (addr % mtz_mtd->erasesize) != 0) {
		pr_err("[MTZ] erase addr not aligned: addr=0x%x erase=0x%x\n",
			addr, mtz_mtd->erasesize);
		return -1;
	}

	if ((addr + mtz_block_size) > mtz_mtd->size) {
		pr_err("[MTZ] erase addr out of range: addr=0x%x size=0x%x\n",
			addr, mtz_mtd->size);
		return -1;
	}

	if (!mtz_mtd->_block_isbad) {
		pr_err("[MTZ] erase skipped: block_isbad op is NULL\n");
		return -1;
	}

	if (mtz_mtd->_block_isbad(mtz_mtd, addr)) {
		pr_err("[MTZ] block %d is bad, cannot erase\n", block_idx);
		return -1;
	}
	memset(&erase, 0, sizeof(struct erase_info));
	erase.mtd = mtz_mtd;
	erase.addr = addr;
	erase.len = mtz_block_size;
	erase.time = 1000;
	erase.retries = 2;
	return mtz_mtd->_erase(mtz_mtd, &erase);
}

static int mtz_prepare_slot_block_for_write(u32 slot_idx)
{
	u32 slot_in_block;
	u32 slot_block;
	u32 base_slot;
	u32 i;
	u32 ret;

	if (mtz_slots_per_block == 0) {
		pr_err("[MTZ] invalid slots per block\n");
		return -1;
	}

	slot_in_block = slot_idx % mtz_slots_per_block;
	if (slot_in_block != 0)
		return 0;

	slot_block = (slot_idx < mtz_slots_per_block) ? mtz_data_blocks[0] : mtz_data_blocks[1];
	pr_info("[MTZ] new block write, erase block %u before slot %u\n",
		slot_block, slot_idx);

	ret = mtz_erase_block(slot_block);
	if (ret != 0)
		return ret;
	
	base_slot = (slot_block == mtz_data_blocks[0]) ? 0 : mtz_slots_per_block;
	for (i = 0; i < mtz_slots_per_block && (base_slot + i) < mtz_slot_count; i++) {
		mtz_slot_map[base_slot + i] = 0;
	}

	return 0;
}


static int mtz_write_slot(u32 slot_idx, char *data, u32 size)
{
	u32 addr_data;
	size_t retlen;
	int ret;
	SlotHeader *header;
	size_t page_size;
	size_t data_size;

	header = mtz_get_slot_header_rsv_addr();
	if (!header) {
		pr_err("[MTZ] slot header is invalid\n");
		return -1;
	}

	if (size != MTZ_SLOT_SIZE) {
		pr_err("[MTZ] slot data size must be %u, got %u\n", MTZ_SLOT_SIZE, size);
		return -1;
	}
	page_size = mtz_mtd->writesize;
	data_size = size - page_size;
	addr_data = mtz_slot_data_addr(slot_idx);
	pr_info("[MTZ] write slot %u, data addr: 0x%x, data size: %zu, page size: %zu\n",
		slot_idx, addr_data, data_size, page_size);
	if (addr_data % mtz_mtd->writesize != 0) {
		pr_err("[MTZ] slot data write not page-aligned (addr=0x%x size=%u page=%u)\n",
			addr_data, size, mtz_mtd->writesize);
		return -1;
	}

	// Set up header state
	header->magic = METAZONE_SLOT_MAGIC;
	header->seq = mtz_current_seq + 1;
	header->state = 0x01; // writin

	if (mtz_prepare_slot_block_for_write(slot_idx) != 0)
		return -1;

	// Write slot data (exclude last page)
	ret = mtz_mtd->_write(mtz_mtd, addr_data, data_size, &retlen, (u_char *)data);
	if (ret || retlen != data_size) {
		pr_err("[MTZ] write slot data fail\n");
		return -1;
	}

	// Update header state to valid
	header->state = 0xFF;
	if (mtz_write_slot_header_page(slot_idx, header, page_size) != 0)
		return -1;


	mtz_current_slot_idx = slot_idx;
	mtz_slot_map[slot_idx] = 1;
	mtz_current_seq = header->seq;
	pr_info("[MTZ] write slot %u, mtz_current_seq%d,  header->seq:%d\n", slot_idx, mtz_current_seq, header->seq);

	return 0;
}

static int mtz_read_slot(u32 slot_idx, char *data, u32 size)
{
	u32 addr_data;
	size_t retlen;
	int ret;
	SlotHeader header;
	size_t page_size = mtz_mtd->writesize;

	addr_data = mtz_slot_data_addr(slot_idx);
	pr_info("[MTZ] mtz_read_slot, slot_idx:%d, addr_data:0x%x\n", slot_idx, addr_data);
	if (addr_data % page_size != 0) {
		pr_err("[MTZ] slot data read not page-aligned (addr=0x%x size=%u page=%u)\n",
			addr_data, size, mtz_mtd->writesize);
		return -1;
	}
	// if (mtz_read_slot_header(slot_idx, &header) != 0)
	// 	return -1;

	// if (header.magic != METAZONE_SLOT_MAGIC || header.state != 0xFF) {
	// 	return -1;
	// }

	// Read data
	ret = mtz_mtd->_read(mtz_mtd, addr_data, size, &retlen, (u_char *)data);
	if (ret || retlen != size) {
		pr_info("[MTZ] metaozne read fail , ret:%d, retlen:%d, size:%d\n", ret, retlen, size);
		return -1;
	}

	return 0;
}

static void mtz_load_current_slot_to_rsvmem(void)
{
	if (!g_mtz_rsvmem_info) {
		pr_err("[MTZ] rsv memory info is NULL, skip loading current slot\n");
		return;
	}

	if (mtz_current_slot_idx == MTZ_MAX_SLOTS) {
		pr_err("[MTZ] current slot unavailable, skip loading to rsv memory\n");
		return;
	}

	{
		u32 read_size = g_mtz_rsvmem_info->size > MTZ_SLOT_SIZE ? MTZ_SLOT_SIZE : g_mtz_rsvmem_info->size;
		if ((read_size % mtz_mtd->writesize )!= 0) {
			pr_err("[MTZ] rsv memory size is not aligned with mtd page size\n");
			return;
		}

		pr_info("[MTZ] load current slot %u into rsv memory, size=%u\n",
			mtz_current_slot_idx, read_size);
		if (mtz_read_slot(mtz_current_slot_idx,
				(char *)g_mtz_rsvmem_info->virt_addr, read_size) != 0) {
			pr_err("[MTZ] load current slot %u failed\n", mtz_current_slot_idx);
		}
	}
}

static bool mtz_get_next_block_first_slot(u32 current_slot, u32 *next_slot)
{
	u32 slot_in_block;

	if (!next_slot)
		return false;

	if (mtz_slots_per_block == 0 || current_slot == MTZ_MAX_SLOTS)
		return false;

	slot_in_block = current_slot % mtz_slots_per_block;
	if (slot_in_block != (mtz_slots_per_block - 1))
		return false;

	if (mtz_active_blocks == 1)
		*next_slot = 0;
	else
		*next_slot = (current_slot < mtz_slots_per_block) ? mtz_slots_per_block : 0;
	return true;
}

static int mtd_write_metazone(unsigned char *pbBuffer, unsigned int u4Size)
{
	u32 next_slot;

	if (mtz_get_next_block_first_slot(mtz_current_slot_idx, &next_slot)) {
		pr_info("[MTZ] current block full, switch to next block slot %u\n", next_slot);
		goto write_slot;
	}

	if (mtz_find_next_good_slot(&next_slot) != 0) {
		pr_err("[MTZ] no good slot available\n");
		return -1;
	}

	write_slot:
	if (mtz_write_slot(next_slot, pbBuffer, u4Size) != 0) {
		pr_err("[MTZ] write slot %d fail\n", next_slot);
		mtz_slot_map[next_slot] = 1; // mark bad
		return -1;
	}

	mtz_current_slot_idx = next_slot;
	return 0;
}

static int mtd_read_metazone(char *pbBuffer, unsigned long u4Size)
{
	u32 best_slot = MTZ_MAX_SLOTS;
	u32 best_seq = 0;
	u32 i;
	bool best_valid = false;

	for (i = 0; i < mtz_slot_count; i++) {
		SlotHeader header;
		size_t retlen;
		int ret;

		ret = mtz_read_slot_header(i, &header);
		if (ret != 0)
			continue;

		if (header.magic != METAZONE_SLOT_MAGIC || header.state != 0xFF)
			continue;

		if (!best_valid || mtz_seq_after(header.seq, best_seq)) {
			best_seq = header.seq;
			best_slot = i;
			best_valid = true;
		}
	}
	pr_info("[MTZ] mtd_read_metazone, best_slot:%d\n", best_slot);
	if (best_slot == MTZ_MAX_SLOTS) {
		pr_err("[MTZ] no valid slot found\n");
		return -1;
	}
	return mtz_read_slot(best_slot, pbBuffer, u4Size);
}
#endif
#ifndef CONFIG_MTD_NAND_ATC
static int mmc_compare_metazone(void)
{
	struct file *fp;
	mm_segment_t old_fs;
	loff_t pos = 0;
	int ret = 0;
	char *mtz_buf1 = NULL;
	char *mtz_buf2 = NULL;
	
	fp = filp_open("/dev/metazone", O_RDWR, 0644U);
	if (IS_ERR(fp)) {
		pr_err("[MTZ] open /dev/block/metazone file error\n");
		return -1;
	}

	// Allocate buffer for metazone read
	mtz_buf1 = kzalloc(MTZ_MAX_SIZE, GFP_KERNEL);
	mtz_buf2 = kzalloc(MTZ_MAX_SIZE, GFP_KERNEL);
	if (!mtz_buf1 || !mtz_buf2) {
		pr_err("[MTZ]allocat buffer for read metazone fail\r\n");
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	pos = 0x0;
	ret = vfs_read(fp, mtz_buf1, MTZ_MAX_SIZE, &pos);

	pos = 0x10000;
	ret = vfs_read(fp, mtz_buf2, MTZ_MAX_SIZE, &pos);

	filp_close(fp, NULL);
	set_fs(old_fs);
	
	// Compare, TODO

	// If they are diff, sync it. TODO

	// Release buffer
	kfree(mtz_buf1);
	kfree(mtz_buf2);

	return 0;
}
#else

static bool mtz_restore_binary_defaults_from_table(void)
{
	u32 i;

	if ((!_pMetaHeader) || (!_u4BinaryStart)) {
		pr_err("[MTZ] restore binary defaults failed: binary area is not ready\n");
		return false;
	}
	memset((char *)_u4BinaryStart, 0, _pMetaHeader->dwBinaryItemSize * _pMetaHeader->dwBinaryNum);
	for (i = 0; i < METAZONE_BINARY_DEFAULT_COUNT; i++) {
		if (MZ_SUCCESS != MetaZone_WriteBinary(g_metazone_binary_defaults[i].index,
			(const char *)g_metazone_binary_defaults[i].data,
			g_metazone_binary_defaults[i].size)) {
			pr_err("[MTZ] restore default binary failed, index=0x%x\n",
				g_metazone_binary_defaults[i].index);
			return false;
		}
	}

	recount_CRC_value(2);
	pr_info("[MTZ] restore binary defaults from metazone_default_table completed\n");
	return true;
}

static bool mtz_restore_reserved_defaults_from_table(void)
{
	u32 i;
	u32 ret;

	if ((!_pMetaHeader) || (!_pbReserve)) {
		pr_err("[MTZ] restore reserved defaults failed: reserved area is not ready\n");
		return false;
	}
	memset(_pbReserve, 0, _pMetaHeader->dwReserveSize);
	for (i = 0; i < METAZONE_RESERVED_DEFAULT_COUNT; i++) {
		if (!g_metazone_reserved_defaults[i].data) {
			pr_err("[MTZ] restore reserved default failed: data is NULL, offset=0x%x\n",
				g_metazone_reserved_defaults[i].offset);
			return false;
		}

		if (g_metazone_reserved_defaults[i].size == 0) {
			pr_info("[MTZ] skip reserved default with zero size, offset=0x%x\n",
				g_metazone_reserved_defaults[i].offset);
			continue;
		}

		ret = MetaZone_WriteReserved_Offset((char *)g_metazone_reserved_defaults[i].data,
			g_metazone_reserved_defaults[i].offset,
			g_metazone_reserved_defaults[i].size);
		if (ret == MZ_FAILURE) {
			pr_err("[MTZ] restore reserved default failed, offset=0x%x size=0x%x\n",
				g_metazone_reserved_defaults[i].offset,
				g_metazone_reserved_defaults[i].size);
			return false;
		}
	}

	recount_CRC_value(3);
	pr_info("[MTZ] restore reserved defaults from metazone_default_table completed\n");
	return true;
}




static bool mtz_restore_dword_defaults_from_table(void)
{
	u32 i;

	if ((!_pMetaHeader) || (!_pu4Value)) {
		pr_err("[MTZ] restore dword defaults failed: dword area is not ready\n");
		return false;
	}
	memset((char *)_pu4Value, 0, _pMetaHeader->dwValueNum * 4);
	for (i = 0; i < METAZONE_DWORD_DEFAULT_COUNT; i++) {
		u32 value = 0;
		if (g_metazone_dword_defaults[i].values)
			value = g_metazone_dword_defaults[i].values[0];

		if (MZ_SUCCESS != MetaZone_Write(g_metazone_dword_defaults[i].index, value)) {
			pr_err("[MTZ] restore default dword failed, index=0x%x\n",
				g_metazone_dword_defaults[i].index);
			return false;
		}
	}

	recount_CRC_value(1);
	pr_info("[MTZ] restore dword defaults from metazone_default_table completed\n");
	return true;
}


static bool mtz_restore_defaults_from_table(void)
{
	u32 header_copy_size;

	if ((!_pMetaZone) || (!g_mtz_rsvmem_info)) {
		pr_err("[MTZ] restore defaults failed: metazone memory is not ready\n");
		return false;
	}

	memset(_pMetaZone, 0, sizeof(TMetaZone));/*memset header buffer*/
	header_copy_size = (u32)sizeof(metazone_backup_header_defaults);
	if (header_copy_size > g_mtz_rsvmem_info->size)
		header_copy_size = (u32)g_mtz_rsvmem_info->size;

	memcpy(_pMetaZone, metazone_backup_header_defaults, header_copy_size);
	_pMetaHeader = (TMetaZone *)_pMetaZone;
	_pbReserve = (char *)((unsigned int)_pMetaZone + sizeof(TMetaZone));
	_pu4Value = (u32 *)((unsigned int)_pMetaZone + _pMetaHeader->dwValueOffset);
	_u4BinaryStart = (u32)_pMetaZone + _pMetaHeader->dwBinaryOffset;
	recount_header_crc(_pMetaHeader);
	_fgWritableZoneInited = true;

	if (!mtz_restore_dword_defaults_from_table()) {
		_fgWritableZoneInited = false;
		return false;
	}

	if (!mtz_restore_binary_defaults_from_table()) {
		_fgWritableZoneInited = false;
		return false;
	}

	if (!mtz_restore_reserved_defaults_from_table()) {
		_fgWritableZoneInited = false;
		return false;
	}

	if (mtz_write_slot(0, (char *)_pMetaZone, MTZ_SLOT_SIZE) != 0) {
		pr_err("[MTZ] restore defaults write to slot0 failed\n");
		return false;
	}

	pr_info("[MTZ] restore defaults from metazone_default_table completed\n");
	return true;
}

static bool mtz_recover_bad_crc_regions(void)
{
	bool recovered = false;
	SlotHeader *header;
	char *slot_base;
	CRCflag expected;

	if ((!_pMetaZone) || (!_pMetaHeader) || (!_pu4Value) || (!_pbReserve)) {
		pr_err("[MTZ] recover bad crc regions failed: metazone memory is not ready\n");
		return false;
	}

	slot_base = (char *)_pMetaZone;
	header = (SlotHeader *)(slot_base + MTZ_SLOT_SIZE - MTZ_SLOT_HEADER_SIZE);
	if (header->magic != METAZONE_SLOT_MAGIC || header->state != 0xFF) {
		pr_err("[MTZ] slot header invalid, restore defaults\n");
		return mtz_restore_defaults_from_table();
	}

	expected = header->crc;
	_fgWritableZoneInited = true;

	if (check_dword_crc_region(_pMetaHeader, _pu4Value, expected.crc_dword, "dwValue") != 0) {
		pr_err("[MTZ] dword region CRC invalid, restore dword defaults only\n");
		if (!mtz_restore_dword_defaults_from_table())
			return false;
		recovered = true;
	}

	if (check_binary_crc_region(_pMetaHeader, _u4BinaryStart, expected.crc_binary, "dwBinary") != 0) {
		pr_err("[MTZ] binary region CRC invalid, restore binary defaults only\n");
		if (!mtz_restore_binary_defaults_from_table())
			return false;
		recovered = true;
	}

	if (check_reserved_crc_region(_pMetaHeader, _pbReserve, expected.crc_reserved, "dwReserved") != 0) {
		pr_err("[MTZ] reserved region CRC invalid, restore reserved defaults only\n");
		if (!mtz_restore_reserved_defaults_from_table())
			return false;
		recovered = true;
	}

	if (recovered) {
		pr_info("[MTZ] writable zone bad CRC regions recovered selectively\n");
		if (mtz_write_slot(0, (char *)_pMetaZone, MTZ_SLOT_SIZE) != 0) {
			pr_err("[MTZ] restore defaults write to slot0 failed\n");
			return false;
		}

	}

	return true;
}
#endif



static bool MTZ_WriteWritableZone(void)
{
	char *pbBuffer;

	if (_fgWritableZoneInited && _fgWritableZoneDirty) {
		u32 *pu4Signature;
		Lock();
		pbBuffer = (char *)kmalloc(METAZONE_SIZE_MAX, GFP_KERNEL);
		if (pbBuffer) {
			memset(pbBuffer, 0, METAZONE_SIZE_MAX);
			memcpy(pbBuffer, _pMetaZone, MTZ_SLOT_SIZE);
		}
		_fgWritableZoneDirty = false;
		Unlock();
		if (!pbBuffer) {
			pr_err("[MTZ] Failed to kmalloc memory for Save MetaZone !!!\n");
			return false;
		}
		pu4Signature = (u32 *) (pbBuffer + METAZONE_SIZE_MAX - 4);
		*pu4Signature = METAZONE_SIGNATURE;
//#ifdef NAND_BOOT
#ifdef CONFIG_MTD_NAND_ATC
		mtd_write_metazone(pbBuffer, METAZONE_SIZE_MAX);
#else
		mmc_write_metazone(pbBuffer, METAZONE_SIZE_MAX);
#endif

		kfree(pbBuffer);
	}
	return true;
}

#ifdef CONFIG_MTD_NAND_ATC
static bool mtz_restore_defaults_or_recover_bad_crc(void) {
	pr_err("[MTZ] WriteableZone bad crc regions recovry failed, restore defaults table\n");
	if (!mtz_restore_defaults_from_table())
		return false;

	_fgWritableZoneInited = true;
	pr_info("[MTZ] WritableZone Infomation:\n");
	pr_info("[MTZ] \tDataSize = 0x%x, Binary Item Max Size = %d\n",
		_pMetaHeader->dwDataSize, _pMetaHeader->dwBinaryItemSize);
	pr_info("[MTZ] \tDWORD Value Number = %d, offset = 0x%x\n", _pMetaHeader->dwValueNum,
		_pMetaHeader->dwValueOffset);
	pr_info("[MTZ] \tBinary Value Number = %d, offset = 0x%x\n",
		_pMetaHeader->dwBinaryNum, _pMetaHeader->dwBinaryOffset);
	pr_info("[MTZ] MTZ_InitWritableZone End\n");

	return true;
}

static bool mtz_reload_previous_slot(void)
{
	u32 tried_count = 0;
	u32 slot_idx = MTZ_MAX_SLOTS;
	u32 slot_seq = 0;
	u8 tried[MTZ_MAX_SLOTS];

	memset(tried, 0, sizeof(tried));

	while (tried_count < mtz_slot_count) {
		u32 i;
		bool found = false;
		SlotHeader header;

		slot_idx = MTZ_MAX_SLOTS;
		for (i = 0; i < mtz_slot_count; i++) {
			if (tried[i])
				continue;
			if (mtz_read_slot_header(i, &header) != 0)
				continue;
			if (header.magic != METAZONE_SLOT_MAGIC || header.state != 0xFF)
				continue;
			if (!found || mtz_seq_after(header.seq, slot_seq)) {
				slot_idx = i;
				slot_seq = header.seq;
				found = true;
			}
		}

		if (!found)
			break;

		tried[slot_idx] = 1;
		tried_count++;

		pr_err("[MTZ] try to reload slot %u (seq=%u)\n", slot_idx, slot_seq);
		if (mtz_read_slot(slot_idx, (char *)g_mtz_rsvmem_info->virt_addr,
				g_mtz_rsvmem_info->size) != 0) {
			pr_err("[MTZ] slot %u read failed\n", slot_idx);
			continue;
		}

		_pMetaZone = (void *)g_mtz_rsvmem_info->virt_addr;
		_pMetaHeader = (TMetaZone *) _pMetaZone;
		_pbReserve = (char *)((unsigned int) _pMetaZone + sizeof(TMetaZone));
		_pu4Value = (unsigned int *) ((unsigned int) _pMetaZone + _pMetaHeader->dwValueOffset);
		_u4BinaryStart = (unsigned int) _pMetaZone + _pMetaHeader->dwBinaryOffset;
		if (check_header_crc(_pMetaHeader) == 0 && check_CRC_flag1() == 0) {
			// mtz_current_slot_idx = slot_idx;
			// mtz_current_seq = slot_seq;
			pr_info("[MTZ] slot %u reload OK\n", slot_idx);
			_fgWritableZoneInited = true;

			pr_info("[MTZ] WritableZone Infomation:\n");
			pr_info("[MTZ] \tDataSize = 0x%x, Binary Item Max Size = %d\n",
				_pMetaHeader->dwDataSize, _pMetaHeader->dwBinaryItemSize);
			pr_info("[MTZ] \tDWORD Value Number = %d, offset = 0x%x\n", _pMetaHeader->dwValueNum,
				_pMetaHeader->dwValueOffset);
			pr_info("[MTZ] \tBinary Value Number = %d, offset = 0x%x\n",
				_pMetaHeader->dwBinaryNum, _pMetaHeader->dwBinaryOffset);
			pr_info("[MTZ] MTZ_InitWritableZone End\n");
			return true;
		}
	}

	pr_err("[MTZ] all slots reload failed, try to restore defaults or recover bad crc regions\n");
	return mtz_restore_defaults_or_recover_bad_crc();
}
#endif

static bool MTZ_InitWritableZone(void)
{
	int ret = 0;
	PTMetaZone pMetazoen = NULL;
	SlotHeader *slotheader = NULL;

	if (!_fgWritableZoneInited) {
		pr_info("[MTZ] MTZ_InitWritableZone\n");

#if !USE_RESERVED_MEMORY

		if (g_mtz_rsvmem_info == NULL) {
			pr_err("[MTZ]reserved memory has not been initilized\r\n");
			return false;
		}
//#ifdef NAND_BOOT
#ifdef CONFIG_MTD_NAND_ATC
		ret = mtd_read_metazone((char *)g_mtz_rsvmem_info->virt_addr, MTZ_SLOT_SIZE);
#else
		ret = mmc_read_metazone((char *)g_mtz_rsvmem_info->virt_addr, g_mtz_rsvmem_info->size);
#endif
		if (ret != 0) {
			pr_err("[MTZ]read metazone fail\r\n");
			return false;
		}

		pMetazoen = (PTMetaZone)g_mtz_rsvmem_info->virt_addr;
		
		if (METAZONE_SIGNATURE != pMetazoen->dwSignature) {
			pr_err("[MTZ] MetaZone Signature is invalid!!!\r\n");
			return false;
		}
		if (METAZONE_VERSION != pMetazoen->dwVersion) {
			RETAILMSG(ERROR_LOG, ("[MTZ] MetaZone version is invalid!!!\r\n"));
			return false;
		}
		if (METAZONE_SIZE_MAX < pMetazoen->dwDataSize) {
			pr_err("[MTZ] MetaZone siz is out of range!!!\r\n");
			return false;
		}
		if ((pMetazoen->dwReserveSize + (pMetazoen->dwValueNum * 4) +
			((pMetazoen->dwBinaryItemSize + 4) * pMetazoen->dwBinaryNum)) >= pMetazoen->dwDataSize) {
			pr_err("[MTZ] Calculated size is out of range!!!\n");
			return false;
		}
		pr_info("[MTZ] Writabe metazone size is %d.\n", pMetazoen->dwDataSize);

		_pMetaZone = (void *)g_mtz_rsvmem_info->virt_addr;
		_pMetaHeader = (TMetaZone *) _pMetaZone;
		_pbReserve = (char *)((unsigned int) _pMetaZone + sizeof(TMetaZone));
		_pu4Value = (unsigned int *) ((unsigned int) _pMetaZone + _pMetaHeader->dwValueOffset);
		_u4BinaryStart = (unsigned int) _pMetaZone + _pMetaHeader->dwBinaryOffset;

#else
		/*mtz_read_slot:for the case where the preloader does not support metazone
		memset((char *)g_mtz_rsvmem_info->virt_addr, 0 , g_mtz_rsvmem_info->size);
		ret = mtz_read_slot(mtz_current_slot_idx,(char *)g_mtz_rsvmem_info->virt_addr, MTZ_SLOT_SIZE);
		if (ret != 0) {
			pr_err("[MTZ]read metazone fail\r\n");
			return false;
		}
		*/
		_pMetaZone = (void *)g_mtz_rsvmem_info->virt_addr; // already get address in metazone_probe
		_pMetaHeader = (TMetaZone *) _pMetaZone;
		_pbReserve = (char *)((unsigned int) _pMetaZone + sizeof(TMetaZone));
		_pu4Value = (unsigned int *) ((unsigned int) _pMetaZone + _pMetaHeader->dwValueOffset);
		_u4BinaryStart = (unsigned int) _pMetaZone + _pMetaHeader->dwBinaryOffset;

#endif

#ifdef CONFIG_MTD_NAND_ATC
	slotheader = mtz_get_slot_header_rsv_addr();
		/* First boot: CRC not calculated yet, generate CRCs for all regions */
		if ((METAZONE_SIGNATURE == _pMetaHeader->dwSignature) &&
			(METAZONE_VERSION == _pMetaHeader->dwVersion) &&
			(slotheader->magic != METAZONE_SLOT_MAGIC)){
			pr_info("[MTZ]First boot detected, generate CRC for all regions\n");
			recount_header_crc(_pMetaHeader);
			recount_CRC_value(1);
			recount_CRC_value(2);
			recount_CRC_value(3);
			_fgWritableZoneInited = true;
			MetaZone_Flush(1);
		} else {
			if (check_header_crc(_pMetaHeader) != 0 || check_CRC_flag1() != 0 ) {
				pr_err("[MTZ] WritableZone signature or version is invalid,try to get previous slot.\n");
				return mtz_reload_previous_slot();
			}
		}
#endif
		_fgWritableZoneInited = true;
		pr_info("[MTZ] WritableZone Infomation:\n");
		pr_info("[MTZ] \tDataSize = 0x%x, Binary Item Max Size = %d\n",
			_pMetaHeader->dwDataSize, _pMetaHeader->dwBinaryItemSize);
		pr_info("[MTZ] \tDWORD Value Number = %d, offset = 0x%x\n", _pMetaHeader->dwValueNum,
			_pMetaHeader->dwValueOffset);
		pr_info("[MTZ] \tBinary Value Number = %d, offset = 0x%x\n",
			_pMetaHeader->dwBinaryNum, _pMetaHeader->dwBinaryOffset);
		pr_info("[MTZ] MTZ_InitWritableZone End\n");

	}
	return _fgWritableZoneInited;
}


static bool MTZ_InitFlashDriver(void)
{
	return true;
}

#ifdef KERNEL_STANDARD_API

static int mtz_flush_thread_func(void *pvArg)
{
	pr_info("[MTZ]metazone flush thread start\n");
	while (!kthread_should_stop()) {
		down(&_rSema);
		MTZ_WriteWritableZone();
	}
	pr_info("[MTZ]metazone flush thread end\n");
	return 0;
}

#endif

/* ------------------------------------------------------------------------------ */
/* Local Functions */
/* ------------------------------------------------------------------------------ */


bool MTZ_Deinit(void)
{
	if (!_pMetaZone) {
#if !USE_RESERVED_MEMORY
		kfree(_pMetaZone);
#endif
		_pMetaZone = NULL;
		_fgWritableZoneInited = false;
	}

	if (!_pFsMetaZone) {
		kfree(_pFsMetaZone);
		_pFsMetaZone = NULL;
		_fgFileZoneInited = false;
	}

	#ifdef KERNEL_STANDARD_API
	if (mtz_thread_task) {
		kthread_stop(mtz_thread_task);
		mtz_thread_task = NULL;
	}
	#endif

	return true;
}

/* ----------------------------------------------------------------------------- */
/* FUNCTION */
/* MTZ_Init */
/* DESCRIPTION */
/* This function is used to init MetaZone Driver. */
/* PARAMETERS */
/* none */
/* RETURNS */
/* Return true for success; otherwise, the operation fails. */
/* RETURN VALUE LIST */
/* none */
/* ---------------------------------------------------------------------------- */
#ifdef KERNEL_STANDARD_API


int MTZ_Init(void)
{
	int ret = 0;
	char *t_name = "MTZ_Thread";
	struct sched_param param;

	pr_info("[MTZ] MTZ_Init.\n");

	mutex_init(&_Lock);
	sema_init(&_rSema, 0);

	//#ifdef NAND_BOOT
#ifdef CONFIG_MTD_NAND_ATC
	//If use NandFlash, we need init it.
	MTZ_InitFlashDriver();
	char *partionName = "metazone";
	nandBlockSize = nand_get_blocksize(partionName); // get nand block size for skipping bad block.
	pr_info("[MTZ] metazone partion block size is:0x%x.\n", nandBlockSize);

	if (mtz_init_mtd_layout() != 0)
		return -1;

	mtz_scan_slots_for_latest();
	//mtz_load_current_slot_to_rsvmem();
#endif

	// Load metazone data from storage media;
	if (!MTZ_InitWritableZone()) {
		pr_err("[MTZ]read metazone from storage media fail\r\n");;
	}

	if (mtz_proc_init() != 0) {
		pr_err("[MTZ] create metazone debug fail! \n");
	}

	mtz_thread_task = kthread_create(&mtz_flush_thread_func, NULL, t_name);
	if (IS_ERR(mtz_thread_task)) {
		pr_err("[MTZ]create metazone flush thread failed.\n");
		return -1;
	}

	param.sched_priority = MTZ_DRV_THREAD_PRIORITY;
	ret = sched_setscheduler_nocheck(mtz_thread_task, SCHED_RR, &param);
	wake_up_process(mtz_thread_task);

	pr_info("[MTZ] MTZ_Init End.\n");
	return METAZONE_SIGNATURE;
}

#endif

/* ----------------------------------------------------------------------------- */
/* FUNCTION */
/* MTZ_Open */
/* DESCRIPTION */
/*  */
/* PARAMETERS */
/*  */
/* RETURNS */
/* Return true for success; otherwise, the operation fails. */
/* RETURN VALUE LIST */
/* none */
/* ---------------------------------------------------------------------------- */
unsigned int MTZ_Open(unsigned int context, unsigned int accessCode, unsigned int shareMode)
{
	if (MTZ_InitFlashDriver())
		return context;

	pr_err("[MTZ] MTZ_Open failed!\n");
	return 0;
}


/* ----------------------------------------------------------------------------- */
/* FUNCTION */
/* MTZ_Close */
/* DESCRIPTION */
/*  */
/* PARAMETERS */
/*  */
/* RETURNS */
/* Return true for success; otherwise, the operation fails. */
/* RETURN VALUE LIST */
/* none */
/* ---------------------------------------------------------------------------- */
bool MTZ_Close(unsigned int context)
{
	return true;
}

u32 MetaZone_Read(unsigned int u4Idx, unsigned int *pu4Data)
{
	pr_info("[MTZ] Read u4Idx(0x%x).\n", u4Idx);

	if (MZ_FS_IDX_START <= u4Idx) {
		return MZ_FAILURE;
	}
	
	if (MZ_WR_IDX_START <= u4Idx) {
		return _MetaZone_Read(u4Idx, pu4Data);
	}

	/* else.Send IOCTL to Flash driver to read DataZone. */

	pr_err("[MTZ] Read u4Idx(0x%x) out of range.\n", u4Idx);
	return MZ_FAILURE;
}
EXPORT_SYMBOL(MetaZone_Read);


unsigned int MetaZone_Write(unsigned int u4Idx, unsigned int u4Data)
{
	pr_info("[MTZ] Write u4Idx(0x%x) data(0x%x)\n", u4Idx, u4Data);

	if (MZ_FS_IDX_START <= u4Idx) {
		return MZ_FAILURE;
	}
	
	if (MZ_WR_IDX_START <= u4Idx) {
		if (!_fgWritableZoneInited) {
			pr_err("[MTZ] WZ Write Failed. (No MetaZone).\n");
			return MZ_FAILURE;
		}
		u4Idx -= MZ_WR_IDX_START;
		if (u4Idx < _pMetaHeader->dwValueNum) {
			Lock();
			_pu4Value[u4Idx] = u4Data;
			_fgWritableZoneDirty = true;
			Unlock();
			recount_CRC_value(1); // recount Dword CRC value in memory 
			return MZ_SUCCESS;
		}
		u4Idx += MZ_WR_IDX_START;
	}

	/* else Send IOCTL to Flash driver to write DataZone. */

	pr_err("[MTZ]  Write u4Idx(0x%x) out of range.\n", u4Idx);
	return MZ_FAILURE;
}
EXPORT_SYMBOL(MetaZone_Write);


u32 MetaZone_ReadBinary(unsigned int u4Idx, char *pbData, unsigned int u4Size)
{
	pr_info("[MTZ] Read Binary u4Idx(0x%x).\n", u4Idx);
	if (MZ_FS_IDX_START <= u4Idx) {
		/* Read Binary Value from File Zone */
		if (!_fgFileZoneInited) {
			pr_err("[MTZ] FZ Read Binary u4Idx(0x%x) Failed.\n", u4Idx);
			return MZ_FAILURE;
		}

		u4Idx -= MZ_FS_IDX_START;
		if (u4Idx < MZ_FS_BINARY_NUM) {
			u32 u4Tmp = *(u32 *) (_u4FsBinaryStart + (MZ_FS_BINARY_SIZE + 4) * u4Idx);

			pr_info("[MTZ] R Binary u4Idx(0x%x) size(%d).\n", u4Idx + MZ_FS_IDX_START, u4Tmp);
			if (u4Size > u4Tmp) {
				u4Size = u4Tmp;
			}
			memcpy(pbData,
			       (char *)(_u4FsBinaryStart + (MZ_FS_BINARY_SIZE + 4) * u4Idx + 4),
			       u4Size);
			return u4Size;
		}
		u4Idx += MZ_FS_IDX_START;
	} 

	if (MZ_WR_IDX_START <= u4Idx) {
		u4Size = _MetaZone_ReadBinary(u4Idx, pbData, u4Size);
		if (MZ_FAILURE != u4Size) {
			return u4Size;
		}
	}

	/* else Send IOCTL to Flash driver to read DataZone. */

	pr_err("[MTZ] Read Binary u4Idx(0x%x) out of range.\n", u4Idx);
	return MZ_FAILURE;
}
EXPORT_SYMBOL(MetaZone_ReadBinary);


unsigned int MetaZone_WriteBinary(unsigned int u4Idx, const char *pbData, unsigned int u4Size)
{
	pr_info("[MTZ] Write Binary u4Idx(0x%x).\n", u4Idx);

	if (MZ_FS_IDX_START <= u4Idx) {
		pr_err("[MTZ] Write Binary Index Error: u4Idx(0x%x).\n", u4Idx);
		return MZ_FAILURE;
	}

	if (MZ_WR_IDX_START <= u4Idx) {
		if (!_fgWritableZoneInited) {
			pr_err("[MTZ] WZ Write Binary Failed (No MetaZone).\n");
			return MZ_FAILURE;
		}
		u4Idx -= MZ_WR_IDX_START;
		if (u4Idx < _pMetaHeader->dwBinaryNum) {
			if (u4Size > _pMetaHeader->dwBinaryItemSize)
				u4Size = _pMetaHeader->dwBinaryItemSize;
			Lock();
			*(u32 *) (_u4BinaryStart + (_pMetaHeader->dwBinaryItemSize + 4) * u4Idx) =
			    u4Size;
			memcpy((char *)(_u4BinaryStart +
					(_pMetaHeader->dwBinaryItemSize + 4) * u4Idx + 4), pbData,
			       u4Size);
			_fgWritableZoneDirty = true;
			Unlock();
			recount_CRC_value(2);
			return MZ_SUCCESS;
		}
		u4Idx += MZ_WR_IDX_START;
	}

	/* else  Send IOCTL to Flash driver to read DataZone. */

	pr_err("[MTZ] Write Binary u4Idx(0x%x) out of range.\n", u4Idx);
	return MZ_FAILURE;
}
EXPORT_SYMBOL(MetaZone_WriteBinary);


static u32 _MetaZone_ReadInfo(PMETAZONE_INFO_T prInfo)
{
	memset((void *)prInfo, 0, sizeof(METAZONE_INFO_T));
	if (_fgWritableZoneInited) {
		prInfo->u4WrValueNum = _pMetaHeader->dwValueNum;
		prInfo->u4WrBinaryNum = _pMetaHeader->dwBinaryNum;
		prInfo->u4WrBinarySize = _pMetaHeader->dwBinaryItemSize;
		return MZ_SUCCESS;
	}

	return MZ_NO_INIT;
}


u32 MetaZone_ReadInfo(PMETAZONE_INFO_T prInfo)
{
	u32 u4Ret = 0;

	u4Ret = _MetaZone_ReadInfo(prInfo);
	prInfo->u4FsValueNum = MZ_FS_DWORD_NUM;
	prInfo->u4FsBinaryNum = MZ_FS_BINARY_NUM;
	prInfo->u4FsBinarySize = MZ_FS_BINARY_SIZE;
	return u4Ret;
}
EXPORT_SYMBOL(MetaZone_ReadInfo);


unsigned int MetaZone_ReadReserved(char *pbData, unsigned int u4Size)
{
	pr_info("[MTZ] Read Reserved u4Size(%d).\n", u4Size);
	return _MetaZone_ReadReserved(pbData, u4Size);
}
EXPORT_SYMBOL(MetaZone_ReadReserved);


unsigned int MetaZone_ReadReserved_Offset(char *pbData,unsigned int offset, unsigned int u4Size)
{
	pr_info("[MTZ] Read Reserved offset(%08x) u4Size(%d).\n",offset, u4Size);
	if (!_fgWritableZoneInited)
		return MZ_FAILURE;

	if(offset>_pMetaHeader->dwReserveSize)
		return MZ_FAILURE;

	if((u4Size+offset)>_pMetaHeader->dwReserveSize)
		u4Size=_pMetaHeader->dwReserveSize-offset;
	
	memcpy(pbData, _pbReserve+offset, u4Size);
	
	return u4Size;
}
EXPORT_SYMBOL(MetaZone_ReadReserved_Offset);


u32 MetaZone_WriteReserved(char *pbData, unsigned int u4Size)
{
	pr_info("[MTZ] Write Reserved u4Size(%d).\n", u4Size);

	if (!_fgWritableZoneInited)
		return MZ_FAILURE;

	if (u4Size > _pMetaHeader->dwReserveSize)
		u4Size = _pMetaHeader->dwReserveSize;

	Lock();
	memcpy(_pbReserve, pbData, u4Size);
	_fgWritableZoneDirty = true;
	Unlock();
    recount_CRC_value(3);
	return MZ_SUCCESS;
}
EXPORT_SYMBOL(MetaZone_WriteReserved);


u32 MetaZone_WriteReserved_Offset(char *pbData,unsigned int offset, unsigned int u4Size)
{
	pr_info("[MTZ] Write Reserved offset(%08x) u4Size(%d).\n",offset, u4Size);

	if (!_fgWritableZoneInited)
		return MZ_FAILURE;

	if(offset>_pMetaHeader->dwReserveSize)
		return MZ_FAILURE;

	if((u4Size+offset)>_pMetaHeader->dwReserveSize)
		u4Size=_pMetaHeader->dwReserveSize-offset;

	Lock();
	memcpy(_pbReserve+offset, pbData, u4Size);
	_fgWritableZoneDirty = true;
	Unlock();

	recount_CRC_value(3);

	return MZ_SUCCESS;
}
EXPORT_SYMBOL(MetaZone_WriteReserved_Offset);

u32 MetaZone_Flush(int fgSync)
{
	pr_info("[MTZ] Flush Sync(%d).\n", fgSync);

	if (fgSync) {
		_fgWritableZoneDirty = true;
		MTZ_WriteWritableZone();
	} else {
		up(&_rSema);
	}

	return MZ_SUCCESS;
}
EXPORT_SYMBOL(MetaZone_Flush);



/* ----------------------------------------------------------------------------- */
/* FUNCTION */
/* MTZ_IOControl */
/* DESCRIPTION */
/*  */
/* PARAMETERS */
/*  */
/* RETURNS */
/* Return true for success; otherwise, the operation fails. */
/* RETURN VALUE LIST */
/* none */
/* ---------------------------------------------------------------------------- */
bool MTZ_IOControl(int context, int code, u8 *pInBuffer, int inSize, u8 *pOutBuffer,
		   int outSize, int *pOutSize)
{
	u32 u4Ret = MZ_FAILURE;
	bool fgBlock = false;

	switch (code) {
	case IOCTL_MZ_READ_VALUE:
		if ((!pInBuffer) || (inSize != 4) || (!pOutBuffer) || (4 != outSize))
			return false;

		u4Ret = MetaZone_Read(*(u32 *) pInBuffer, (u32 *) pOutBuffer);
		if ((MZ_FAILURE != u4Ret) && pOutSize)
			*pOutSize = 4;
		break;

	case IOCTL_MZ_WRITE_VALUE:
		if ((!pInBuffer) || (inSize != 8))
			return false;

		u4Ret = MetaZone_Write(*(u32 *) pInBuffer, *((u32 *) pInBuffer + 1));
		break;

	case IOCTL_MZ_READ_BINARY:
		if ((!pInBuffer) || (inSize != 4) || (!pOutBuffer) || (!outSize) || (!pOutSize))
			return false;

		u4Ret = (int)MetaZone_ReadBinary(*(u32 *) pInBuffer, pOutBuffer, outSize);
		*pOutSize = u4Ret;
		break;

	case IOCTL_MZ_WRITE_BINARY:
		if ((!pInBuffer) || (inSize != 4) || (!pOutBuffer) || (!outSize))
			return false;

		u4Ret = MetaZone_WriteBinary(*(u32 *) pInBuffer, pOutBuffer, outSize);
		break;

	case IOCTL_MZ_READ_RESERVED:
		if ((!pOutBuffer) || (!outSize) || (!pOutSize))
			return false;

		u4Ret = (int)MetaZone_ReadReserved((char *)pOutBuffer, (u32) outSize);
		*pOutSize = u4Ret;
		break;

	case IOCTL_MZ_WRITE_RESERVED:
		if ((!pOutBuffer) || (!outSize))
			return false;

		u4Ret = MetaZone_WriteReserved(pOutBuffer, outSize);
		break;

	case IOCTL_MZ_READ_INFO:
		if ((!pOutBuffer) || (sizeof(METAZONE_INFO_T) != outSize))
			return false;

		u4Ret = MetaZone_ReadInfo((PMETAZONE_INFO_T) pOutBuffer);
		if ((MZ_FAILURE != u4Ret) && pOutSize)
			*pOutSize = sizeof(METAZONE_INFO_T);
		break;

	case IOCTL_MZ_FLUSH:
		if (pInBuffer)
			fgBlock = *(int *) pInBuffer;
		u4Ret = MetaZone_Flush(fgBlock);
		break;

	case IOCTL_MZ_READ_RESERVED_OFFSET:
		if ((!pInBuffer) || (inSize != 4) || (!pOutBuffer) || (!outSize)|| (!pOutSize))
			return false;
		u4Ret = (int)MetaZone_ReadReserved_Offset((char *)pOutBuffer,*(u32 *) pInBuffer, (u32) outSize);
		*pOutSize = u4Ret;
		break;

	case IOCTL_MZ_WRITE_RESERVED_OFFSET:
		if ((!pInBuffer) || (inSize != 4) || (!pOutBuffer) || (!outSize))
			return false;
		u4Ret = MetaZone_WriteReserved_Offset((char *)pOutBuffer,*(u32 *) pInBuffer,(u32) outSize);
		break;

	default:
		break;
	}

	return (MZ_FAILURE == u4Ret) ? false : true;
}

static int mtz_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	void *private_data;

	private_data = (void *)MTZ_Open(1, 0, 0);

	/* FIXME */
	file->private_data = private_data;

	return ret;
}

static int mtz_release(struct inode *inode, struct file *file)
{
	int ret = 0;
	void *private_data = file->private_data;

	if (private_data == NULL)
		return -1;

	MTZ_Close((unsigned int)private_data);

	file->private_data = NULL;

	return ret;
}


static long mtz_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void *private_data;
	WIN32_IOCTL_DATA win_ioctl;
	bool bRet = false;
	u8 alloc_in_buf = 0;
	u8 alloc_out_buf = 0;
	u8 alloc_byte_returned = 0;
	long ret = 0;

	private_data = filp->private_data;

	if (!private_data) {
		pr_err("[MTZ] filp private_data error, can't get device handler.\n");
		ret = -2;
		goto ioctl_err;
	}

	if (copy_from_user((void *)&win_ioctl, (void *)arg, sizeof(win_ioctl))) {
		pr_err("[MTZ] copy source ioctl parameter failed.\n");
		ret = -4;
		goto ioctl_err;
	}

	if ((win_ioctl.InSize) && (((WIN32_IOCTL_DATA*)arg)->pInBuf)){
		win_ioctl.pInBuf = kzalloc(win_ioctl.InSize, GFP_KERNEL);
		if (win_ioctl.pInBuf){
			copy_from_user(win_ioctl.pInBuf, ((WIN32_IOCTL_DATA*)arg)->pInBuf, win_ioctl.InSize);
			alloc_in_buf = 1;
		} else {
			pr_err("[MTZ] copy source ioctl parameter (pInBuf) failed.\n");
			ret = -5;
			goto ioctl_err;
		}
	}

	if ((win_ioctl.OutSize) && (((WIN32_IOCTL_DATA*)arg)->pOutBuf)){
		win_ioctl.pOutBuf = kzalloc(win_ioctl.OutSize, GFP_KERNEL);
		if (win_ioctl.pOutBuf){
			copy_from_user(win_ioctl.pOutBuf, ((WIN32_IOCTL_DATA*)arg)->pOutBuf, win_ioctl.OutSize);
			alloc_out_buf = 1;
		} else {
			pr_err("[MTZ] copy source ioctl parameter (pOutBuf) failed.\n");
			ret = -6;
			goto ioctl_err;
		}
	}

	if (((WIN32_IOCTL_DATA*)arg)->pBytesReturned){
		win_ioctl.pBytesReturned = kzalloc(sizeof(unsigned int), GFP_KERNEL);
		if (win_ioctl.pBytesReturned){
			copy_from_user(win_ioctl.pBytesReturned, ((WIN32_IOCTL_DATA*)arg)->pBytesReturned, sizeof(unsigned int));
			alloc_byte_returned = 1;
		} else {
			pr_err("[MTZ] copy source ioctl parameter (pBytesReturned) failed.\n");
			ret = -7;
			goto ioctl_err;
		}
	}

	bRet = MTZ_IOControl((int)private_data,
			     cmd,
			     (u8 *) win_ioctl.pInBuf,
			     win_ioctl.InSize,
			     (u8 *) win_ioctl.pOutBuf,
			     win_ioctl.OutSize, win_ioctl.pBytesReturned);

	/* Process data only in success */
	if (bRet){
		if (alloc_in_buf) {
			ret = copy_to_user((void *)((WIN32_IOCTL_DATA*)arg)->pInBuf,
								(void *)win_ioctl.pInBuf,
								win_ioctl.InSize);
			if (ret) {
				pr_err("[MTZ] copy result to ioctl parameter (pBytesReturned) failed.\n");
				ret = -8;
				bRet = false;
				goto ioctl_err;
			}
		}

		if (alloc_out_buf) {
			ret = copy_to_user((void *)((WIN32_IOCTL_DATA*)arg)->pOutBuf,
								(void *)win_ioctl.pOutBuf,
								win_ioctl.OutSize);
			if (ret) {
				pr_err("[MTZ] copy result to ioctl parameter (pBytesReturned) failed.\n");
				ret = -9;
				bRet = false;
				goto ioctl_err;
			}
		}

		if (alloc_byte_returned) {
			ret = copy_to_user((void *)((WIN32_IOCTL_DATA*)arg)->pBytesReturned,
								(void *)win_ioctl.pBytesReturned,
								sizeof(unsigned int));
			if (ret) {
				pr_err("[MTZ] copy result to ioctl parameter (pBytesReturned) failed.\n");
				ret = -10;
				bRet = false;
				goto ioctl_err;
			}
		}
	}

ioctl_err:
	if (alloc_in_buf) {
		kfree(win_ioctl.pInBuf);
		win_ioctl.pInBuf = NULL;
	}

	if (alloc_out_buf) {
		kfree(win_ioctl.pOutBuf);
		win_ioctl.pOutBuf = NULL;
	}

	if (alloc_byte_returned) {
		kfree(win_ioctl.pBytesReturned);
		win_ioctl.pBytesReturned = NULL;
	}

	return bRet ? 0 : (-1);
}


static const struct file_operations mtz_fops = {
	.open			 = mtz_open,
	.release		 = mtz_release,
	.unlocked_ioctl	 = mtz_ioctl,
};

static int mtz_reboot_notifier_func(struct notifier_block *nb, unsigned long event, void *v)
{

	(void) nb;
	(void) v;
	pr_info("entry %s\n", __func__);

	switch (event) {
	case SYS_RESTART:
	case SYS_POWER_OFF:
		MetaZone_Flush(1);
		break;
	default:
		pr_err("event not register into shutdown flow!");
	}

	return 0;
}

static struct notifier_block reboot_notifier = {
	.notifier_call = mtz_reboot_notifier_func,
};

static int metazone_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct reserved_mem *metazone_reserved_mem = NULL;
	mtzdev_info *metazone_dev = NULL;
	phys_addr_t mtz_base, mtz_size;
	int result = -1;

	pr_info("[MTZ]enter metazone probe\r\n");

	if (!node) {
		pr_err("[MTZ] metazone probe fail because of no metazone device compatible dts node!\r\n");
		return -1;
	}

	metazone_dev = kzalloc(sizeof(mtzdev_info), GFP_KERNEL);
	if (!metazone_dev) {
		result = -ENOMEM;
		pr_err("[MTZ]metazone probe fail because of get alloc memory fail!\n");
		goto err_free_mem;
	}

#if !USE_RESERVED_MEMORY  //dts dynamic addr allocate
	of_reserved_mem_device_init(&(pdev->dev));
	metazone_reserved_mem = (struct reserved_mem *)(pdev->dev.cma_area);
	if (!metazone_reserved_mem) {
		pr_err("[MTZ]metazone reserved memory get error! dev name: %s\r\n", pdev->name);
		goto err_free_mem;
	}
	pr_info("[MTZ]dynamic allocate %s reserved memory base:0x%x, size:0x%x. \r\n",
		metazone_reserved_mem->name, metazone_reserved_mem->base, metazone_reserved_mem->size);
	
	metazone_dev->reserved_mem_info.phys_addr = metazone_reserved_mem->base;
	metazone_dev->reserved_mem_info.size      = metazone_reserved_mem->size;
	metazone_dev->reserved_mem_info.virt_addr = (unsigned long)ioremap(metazone_reserved_mem->base,
		metazone_reserved_mem->size);

	if (0 == metazone_dev->reserved_mem_info.virt_addr) {
		pr_info("[MTZ]metazone fail in ioremap(phyaddr: 0x%08x, size: 0x%08x)!\r\n",
			metazone_reserved_mem->base, metazone_reserved_mem->size);
		goto err_free_mem;
	}
	
#else  //dts static addr allocate , share reserved memory with lk/kernel/arm2
    node =of_find_compatible_node(NULL,NULL,"atc-metazone-mem");
    if (node) {
        if (of_property_read_u32_array(node,"reg",(u32 *)&MTZmem,2)) {
            pr_err("[MTZ] faild get reserved addr and size in dts!\r\n");
        }
        pr_info("[MTZ] metazone static reserved memory start is 0x%x,size is 0x%x\n",MTZmem.start,MTZmem.size);
    }else {
        pr_err("[MTZ] fail to get reserved memory node in dts!\r\n");
    }

	metazone_dev->reserved_mem_info.phys_addr = MTZmem.start;
	metazone_dev->reserved_mem_info.size      = MTZmem.size;
	metazone_dev->reserved_mem_info.virt_addr = (unsigned long)ioremap(metazone_dev->reserved_mem_info.phys_addr,
	                                             metazone_dev->reserved_mem_info.size);
	/*
      */

	if (0 == metazone_dev->reserved_mem_info.virt_addr) {
		pr_err("[MTZ]metazone fail in ioremap \n");
		goto err_free_mem;
	}

#endif
	metazone_dev->dev = &(pdev->dev);
	metazone_dev->cdev.name = MTZ_MISC_DVR_NAME;
	metazone_dev->cdev.minor = MISC_DYNAMIC_MINOR;
	metazone_dev->cdev.fops = &mtz_fops;

	platform_set_drvdata(pdev, metazone_dev);

	result = misc_register(&(metazone_dev->cdev));
	if (result != 0) {
		pr_err("[MTZ]metazone probe fail because of misc_register, error = %d\r\n", result);
		goto err_unset_drvdata;
	}

	result = register_reboot_notifier(&reboot_notifier);
	pr_info("register_reboot_notifier ret %d\n", result);

	g_mtz_rsvmem_info = &(metazone_dev->reserved_mem_info);

	// Init metazone
	MTZ_Init();

	pr_info("[MTZ]exit metazone probe\r\n");
	return 0;

err_unset_drvdata:
	platform_set_drvdata(pdev, NULL);

err_free_mem:
	if (0 != metazone_dev->reserved_mem_info.virt_addr) {
		iounmap((void *)metazone_dev->reserved_mem_info.virt_addr);
	}
	kfree(metazone_dev);
	pr_err("[MTZ]metazone probe fail\r\n");

	return result;
}

static int metazone_remove(struct platform_device *pdev)
{
	mtzdev_info *metazone_dev = platform_get_drvdata(pdev);

	if (0 != metazone_dev->reserved_mem_info.virt_addr)
		iounmap((void *)metazone_dev->reserved_mem_info.virt_addr);

	of_reserved_mem_device_release(&(pdev->dev));

	misc_deregister(&(metazone_dev->cdev));

	platform_set_drvdata(pdev, NULL);

	kfree(metazone_dev);

	return 0;
}
#if 0
static void metazone_shutdown(struct platform_device *pdev)
{
	pr_info("[MTZ]enter metazone shutdown\r\n");

	if(MetaZone_Flush(1) < 0){
		pr_err("[MTZ]flush error in shutdown\r\n");
	}

	pr_info("[MTZ]exit metazone shutdown\r\n");

}
EXPORT_SYMBOL(metazone_shutdown);
#endif

static const struct of_device_id metazone_of_ids[] = {
	{.compatible = "atc,metazone",},
	{}
};

static struct platform_driver metazone_platform_drv = {
	.driver = {
		   .name = "atc-metazone",
		   .owner = THIS_MODULE,
		   .of_match_table = metazone_of_ids,
		   },
	.probe = metazone_probe,
	.remove = metazone_remove,
	//.shutdown = metazone_shutdown,
};


static int __init mtz_init(void)
{
	int ret = 0;
	struct device_node *node = NULL;

	MOD_VERSION_INFO(MTZ_MODE_NAME, MTZ_VER_MAJOR, MTZ_VER_MINOR, MTZ_VER_REV);

	pr_info("[MTZ]enter metazone init\r\n");

	node = of_find_compatible_node(NULL, NULL, "atc,metazone");
	if (!node) {
		pr_err("[MTZ]metazone init fail in find dts compatible node!!\r\n");
		ret = -ENOMEM;
		goto err_node;
	}

	ret = platform_driver_register(&metazone_platform_drv);
	if (ret) {
		pr_err("[MTZ]metazone init fail in platform_driver_register, error = %d\r\n", ret);
		goto err_node;
	}

	pr_info("[MTZ]exit metazone init\r\n");
	
	return 0;

err_node:
	pr_err("[MTZ]exit metazone init with error\r\n");
	return ret;
}

static void __exit mtz_exit(void)
{
	MTZ_Deinit();

	pr_info("[MTZ]enter mtz exit!\r\n");

	platform_driver_unregister(&metazone_platform_drv);

	pr_info("[MTZ]exit mtz ok!\r\n");
}

module_init(mtz_init);
module_exit(mtz_exit);

/*
EXPORT_SYMBOL(MetaZone_Flush);
EXPORT_SYMBOL(MetaZone_Read);
EXPORT_SYMBOL(MetaZone_Write);
EXPORT_SYMBOL(MetaZone_ReadBinary);
EXPORT_SYMBOL(MetaZone_WriteBinary);
EXPORT_SYMBOL(MetaZone_ReadReserved);
EXPORT_SYMBOL(MetaZone_WriteReserved);
EXPORT_SYMBOL(MetaZone_ReadInfo);
*/

MODULE_DESCRIPTION("atc metazone driver");
MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE);

