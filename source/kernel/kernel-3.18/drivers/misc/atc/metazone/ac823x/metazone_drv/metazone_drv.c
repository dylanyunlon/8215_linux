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

#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_address.h>
#include <linux/of_reserved_mem.h>
#include <linux/dma-mapping.h>

#include <asm/uaccess.h>
#include <asm/page.h>


#include "metazone_inter.h"
#include "metazone.h"
#include "metazone_ioctl.h"
#include "windev.h"
#include "x_ver.h"


#define KERNEL_STANDARD_API

#ifdef KERNEL_STANDARD_API
#include <linux/kthread.h>

static struct task_struct *mtz_thread_task;
#endif

#define MTZ_MODE_NAME		("MTZ")
#define MTZ_VER_MAJOR		(01)
#define MTZ_VER_MINOR		(01)
#define MTZ_VER_REV			(00)
#define MTZ_MISC_DVR_NAME	("mtz") // /dev/mtz
#define MTZ_MAX_SIZE		(0x200000)	// Phy storage media size for each metazone, in byte

struct resvd_mem_info {
	unsigned long phys_addr;
	unsigned long virt_addr;
	unsigned long size;
};

unsigned long reser_phys_addr;
unsigned long reser_size;

typedef struct mtz_dev_info {
	struct miscdevice cdev;	/* Char device structure */
	struct device *dev;
	struct resvd_mem_info reserved_mem_info;
} mtzdev_info;

struct resvd_mem_info *g_mtz_rsvmem_info = NULL;
#ifdef __aarch64__
static void *_memcpy(void *dest,const void *src,size_t count)
{
		char *tmp=dest;
		const char *s=src;
		while(count--)
			*tmp++=*s++;
		return dest;
	}
#define memcpy  _memcpy

static void *_memset(void *s,int c,size_t count)
{
		char *tmp=(char *)s;
		while(count--)
			*tmp++=c;
		return s;
	}
#define memset  _memset

#endif

#ifdef NAND_BOOT
int mtd_write_metazone(unsigned char *pbBuffer, unsigned int u4Size);
#endif

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
unsigned long _u4BinaryStart = 0;
static bool _fgWritableZoneDirty;

#define USE_RESERVED_MEMORY			(0)

#ifndef KERNEL_STANDARD_API
static HANDLE_T _hMtzFlushThread;
#endif

#define METAZONE_FS_SIZE			(0x10000)	/* 64KB */
#define METAZONE_FS_BINARY_OFFSET	(10240)

static unsigned long _u4FsBinaryStart;
static void *_pFsMetaZone;
static bool _fgFileZoneInited;
static struct mutex _Lock;
static struct semaphore _rSema;
static void *_hEvtFlushFinished;
#define FLUSH_EVENT				("MTZ_FLUSH")

#define Lock()		mutex_lock(&_Lock)
#define Unlock()	mutex_unlock(&_Lock)

#define MTZ_PARTITION_ANDROID  "/dev/block/metazone"
#define MTZ_PARTITION_LINUX  "/dev/metazone"

Wflag p1;
Wflag p2;

static int update_wflag(unsigned char sector,unsigned int offset)
{
	struct file *fp;
	mm_segment_t old_fs;
	loff_t pos;
	int ret;
	char *pbBuffer;
	unsigned u4Size=0x200;
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

#ifndef NAND_BOOT
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

	pos = 0x80000;
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
	update_wflag(1,2);
	
	//pr_err("metazone update_wflag \n");
	/*before write to metazone 2*/
	update_wflag(2,1);
	
	write_metazone2(pbBuffer,u4Size);
	/*after write to metazone 2*/
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
int mtd_write_metazone(unsigned char *pbBuffer, unsigned int u4Size)
{
	struct file *fp;
	mm_segment_t old_fs;
	loff_t pos;
	int ret;

	fp = filp_open(MTZ_PARTITION_LINUX, O_RDWR, 0644U);
	if (IS_ERR(fp)) {
		pr_err("[MTZ] open %s file error\n",MTZ_PARTITION_LINUX);
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	pos = 0x0;
	ret = vfs_write(fp, pbBuffer, u4Size, &pos);

	pos = 0x80000;
	ret = vfs_write(fp, pbBuffer, u4Size, &pos);

	filp_close(fp, NULL);
	set_fs(old_fs);

	return 0;
}
static int mtd_read_metazone(char *pbBuffer, unsigned long u4Size)
{
	struct file *fp;
	mm_segment_t old_fs;
	loff_t pos;
	int ret;

	fp = filp_open(MTZ_PARTITION_LINUX, O_RDWR, 0644U);
	if (IS_ERR(fp)) {
		pr_err("[MTZ] open %s file error\n",MTZ_PARTITION_LINUX);
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	pos = 0x0;
	ret = vfs_read(fp, pbBuffer, u4Size, &pos);
	if (ret != u4Size) {
		pr_err("[MTZ] read metazone from %s fail\n",MTZ_PARTITION_LINUX);
		filp_close(fp, NULL);
		set_fs(old_fs);
		return -1;
	}

	filp_close(fp, NULL);
	set_fs(old_fs);

	return 0;
}
#endif

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

	pos = 0x200000;
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


static bool MTZ_WriteWritableZone(void)
{
	char *pbBuffer;

	if (_fgWritableZoneInited && _fgWritableZoneDirty) {
		u32 *pu4Signature;

		_pu4Value[0]++;
		Lock();
		pbBuffer = (char *)kmalloc(METAZONE_SIZE_MAX, GFP_KERNEL);
		if (pbBuffer) {
			int cpSize =
			    (_pMetaHeader->dwDataSize <
			     METAZONE_SIZE_MAX) ? _pMetaHeader->dwDataSize : METAZONE_SIZE_MAX;
			memcpy(pbBuffer, _pMetaZone, cpSize);
		}
		_fgWritableZoneDirty = false;
		Unlock();
		if (!pbBuffer) {
			pr_err("[MTZ] Failed to kmalloc memory for Save MetaZone !!!\n");
			return false;
		}
		pu4Signature = (u32 *) (pbBuffer + METAZONE_SIZE_MAX - 4);
		*pu4Signature = METAZONE_SIGNATURE;
#ifdef NAND_BOOT
		mtd_write_metazone(pbBuffer, METAZONE_SIZE_MAX);
#else
		mmc_write_metazone(pbBuffer, METAZONE_SIZE_MAX);
#endif

		kfree(pbBuffer);
	}
	return true;
}


static bool MTZ_InitWritableZone(void)
{
	int ret = 0;
	PTMetaZone pMetazoen = NULL;
	
	if (!_fgWritableZoneInited) {
		pr_info("[MTZ] MTZ_InitWritableZone\n");

#if !USE_RESERVED_MEMORY

		if (g_mtz_rsvmem_info == NULL) {
			pr_err("[MTZ]reserved memory has not been initilized\r\n");
			return false;
		}
#ifdef NAND_BOOT
		ret = mtd_read_metazone((char *)g_mtz_rsvmem_info->virt_addr, g_mtz_rsvmem_info->size);
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
			pr_err("[MTZ] MetaZone version is invalid!!!\r\n");
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
		_pbReserve = (char *)((unsigned long) _pMetaZone + sizeof(TMetaZone));
		_pu4Value = (unsigned int *) ((unsigned long) _pMetaZone + _pMetaHeader->dwValueOffset);
		_u4BinaryStart = (unsigned long) _pMetaZone + _pMetaHeader->dwBinaryOffset;

#else
		_pMetaZone = (void *)MEMRSV_PHY_TO_VIRT(METAZONE_MEM_PA);
		_pMetaHeader = (TMetaZone *) _pMetaZone;
		_pbReserve = (char *)((unsigned long) _pMetaZone + sizeof(TMetaZone));
		_pu4Value = (unsigned int *) ((unsigned long) _pMetaZone + _pMetaHeader->dwValueOffset);
		_u4BinaryStart = (unsigned long) _pMetaZone + _pMetaHeader->dwBinaryOffset;
#endif
		if ((METAZONE_SIGNATURE != _pMetaHeader->dwSignature)
		    || (METAZONE_VERSION != _pMetaHeader->dwVersion)) {
			pr_err("[MTZ] WritableZone signature or version is invalid.\n");
			return false;
		}
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


static int mtz_flush_thread_func(void *pvArg)
{
	pr_info("[MTZ]metazone flush thread start\n");
	while (true) {
		down(&_rSema);
		MTZ_WriteWritableZone();
		//if (_hEvtFlushFinished)
		//	x_event_set(_hEvtFlushFinished);
	}
	pr_info("[MTZ]metazone flush thread end\n");
	return 0;
}

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

int MTZ_Init(void)
{
	int ret = 0;
	char *t_name = "MTZ_Thread";
	struct sched_param param;

	pr_info("[MTZ] MTZ_Init.\n");

	mutex_init(&_Lock);
	sema_init(&_rSema, 0);
	//_hEvtFlushFinished = x_event_create(NULL, true, false, FLUSH_EVENT);

	#ifdef NAND_BOOT
	//If use NandFlash, we need init it.
	MTZ_InitFlashDriver();
	#endif
	
	/*// Load metazone data from storage media;
	if (!MTZ_InitWritableZone()) {
		pr_err("[MTZ]read metazone from storage media fail\r\n");;
	}*/
	
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
unsigned long MTZ_Open(unsigned int context, unsigned int accessCode, unsigned int shareMode)
{
	/*if (MTZ_InitFlashDriver())
		return context;*/
	// Load metazone data from storage media;
	if (!MTZ_InitWritableZone()) {
		pr_err("[MTZ]read metazone from storage media fail\r\n");;
		}
	else 
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
		if (!_fgWritableZoneInited)
			return MZ_FAILURE;

		u4Idx -= MZ_WR_IDX_START;
		if (u4Idx < _pMetaHeader->dwValueNum) {
			*pu4Data = _pu4Value[u4Idx];
			return MZ_SUCCESS;
		}
		u4Idx += MZ_WR_IDX_START;
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
		if (!_fgWritableZoneInited)
			return MZ_FAILURE;
		u4Idx -= MZ_WR_IDX_START;
		if (u4Idx < _pMetaHeader->dwBinaryNum) {
			u32 u4Tmp = *(u32 *) (_u4BinaryStart +
				      (_pMetaHeader->dwBinaryItemSize + 4) * u4Idx);
			if (u4Size > u4Tmp)
				u4Size = u4Tmp;
			memcpy(pbData,(char *)(_u4BinaryStart +
					(_pMetaHeader->dwBinaryItemSize + 4) * u4Idx + 4), u4Size);
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
	if (!_fgWritableZoneInited)
		return MZ_FAILURE;

	if (u4Size > _pMetaHeader->dwReserveSize)
		u4Size = _pMetaHeader->dwReserveSize;
	memcpy(pbData, _pbReserve, u4Size);

	return u4Size;
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

	return MZ_SUCCESS;
}
EXPORT_SYMBOL(MetaZone_WriteReserved_Offset);


u32 MetaZone_Flush(int fgSync)
{
	pr_info("[MTZ] Flush Sync(%d).\n", fgSync);

	
	if (fgSync)
		MTZ_WriteWritableZone();
	else
		up(&_rSema);
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
bool MTZ_IOControl(long context, int code, u8 *pInBuffer, int inSize, u8 *pOutBuffer,
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

		u4Ret = MetaZone_WriteReserved((char *)pOutBuffer, (u32)outSize);
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

	MTZ_Close((unsigned long)private_data);

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

	bRet = MTZ_IOControl((long)private_data,
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

/*Reserved memory by device tree!*/
int reserve_memory_metazone_fn(struct reserved_mem *rmem)
{
	pr_err(" name: %s, base: 0x%llx, size: 0x%llx\n", rmem->name,
			   (unsigned long long)rmem->base, (unsigned long long)rmem->size);

	reser_phys_addr=rmem->base;
	reser_size=rmem->size;
	return 0;
}


RESERVEDMEM_OF_DECLARE(reserve_memory_metazone, "atc,metazone-mem", reserve_memory_metazone_fn);

static int metazone_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct reserved_mem *metazone_reserved_mem = NULL;
	mtzdev_info *metazone_dev = NULL;
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

#if 0
	of_reserved_mem_device_init(&(pdev->dev));
	metazone_reserved_mem = (struct reserved_mem *)(pdev->dev.cma_area);
	if (!metazone_reserved_mem) {
		pr_err("[MTZ]metazone reserved memory get error! dev name: %s\r\n", pdev->name);
		goto err_free_mem;
	}
	pr_err("[MTZ]%s reserved memory base:0x%x, size:0x%x. \r\n",
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
#else	
	metazone_dev->reserved_mem_info.phys_addr = 0x11A500000;
	metazone_dev->reserved_mem_info.size	  = 0x100000;
	metazone_dev->reserved_mem_info.virt_addr = (unsigned long)ioremap(metazone_dev->reserved_mem_info.phys_addr,
	metazone_dev->reserved_mem_info.size);
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

static void metazone_shutdown(struct platform_device *pdev)
{
	pr_info("[MTZ]enter metazone shutdown\r\n");

	if(MetaZone_Flush(1) < 0){
		pr_err("[MTZ]flush error in shutdown\r\n");
	}

	pr_info("[MTZ]exit metazone shutdown\r\n");

}
EXPORT_SYMBOL(metazone_shutdown);

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
	.shutdown = metazone_shutdown,
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

late_initcall(mtz_init);
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

