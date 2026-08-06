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

#include "x_assert.h"
#include "x_os.h"
/*#include "u_os.h"*/
#include "x_rtos.h"
#include "windows.h"
#include "imgresz_drv_inst.h"
#include "imgresz_drv.h"
#include "imgresz_hal_if.h"
#include "imgresz_hal.h"
#include "drv_imgresz_errcode.h"
#include "drv_thread.h"
#include "drv_config.h"
#include "x_hal_1176.h"


#include "sys_config.h"
#include <linux/module.h>
#include "media/atc/display.h"
#include <asm/page.h>
#include <asm/uaccess.h>

#include <linux/syscalls.h>
/*#include <linux/vfs.h>*/
#include <linux/fs.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/of_reserved_mem.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>

#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#include "imgresz_log.h"

#include <generated/atc_project.h>


#define DOUBLE_REG 1

/*-----------------------------------------------------------------------------
data declarations
----------------------------------------------------------------------------*/
struct task_struct *h_imgresz_thread_0 = NULL;
struct task_struct *h_imgresz_thread_1 = NULL;
extern struct Linebuf TempLine_Reserved;  /* added by mtk68119 to store the base address of reserved memory*/

static spinlock_t filplock;

IMGRESZ_HAL_IMG_INFO_T SrcBuf;
IMGRESZ_HAL_IMG_INFO_T DstBuf;
static bool   NeedToStop;
static bool   StopCanReturn;
static struct file *CurrFilp;
static void *Dst;

IMGRESZ_HW_INST_T   _arImgreszHwInst[IMGRESZ_HW_INST_NUM] = { { 0 }, { 0 } ,{ 0 },{ 0 }};
bool        _fgImgreszInit = FALSE;
IMGRESZ_HAL_RESAMPLE_METHOD_T _eHResampleMethod = IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR;
IMGRESZ_HAL_RESAMPLE_METHOD_T _eVResampleMethod = IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR;
struct mutex g_ImgReszMutex;

u32 _u4IMGR_DBG_LVL = IMGR_LOG_LVL_HAL;
u8 *_pcImgreszLogLevel[] = {
	"[IMGR OFF]",
	"[IMGR ERR]",
	"[IMGR WARN]",
	"[IMGR CLI]",
	"[IMGR INFO]",
	"[IMGR HAL]",
	"[IMGR IRQ]",
	"[IMGR TRACE]",
	"[IMGR DBG]",
	"[IMGR REGRW]",
};

#if CONFIG_SUSPEND_TO_DRAM

int i4ImgResz_Suspend(void *param)
{
	return 0;
}


int i4ImgResz_Resume(void *param)
{
	return 0;
}


struct pm_operations imgresz_pm_ops = {
	i4ImgResz_Suspend,
	i4ImgResz_Resume,
};

#endif

#define IMGRESZ_BUF_ALIGN_MASK(value, mask) ((((value) + ((mask) - 1)) / (mask)) * (mask))
#define IMGRESZ_ALIGN 4096

#define GEN_MASK(x) ((1<<(x))-1)
#define ROUND_UP_X(v, x) (((v) + GEN_MASK(x)) & ~GEN_MASK(x))
#define ROUND_UP_2(x) ROUND_UP_X((x), 1)
#define ROUND_UP_4(x) ROUND_UP_X((x), 2)
#define ROUND_UP_8(x) ROUND_UP_X((x), 3)
#define ROUND_UP_16(x) ROUND_UP_X((x), 4)
#define ROUND_UP_32(x) ROUND_UP_X((x), 5)
#define DIV_ROUND_UP_X(v, x) (((v) + GEN_MASK(x)) >> (x))
void vImgreszCopyMemory(void *pvDestination, void *pvSource, SIZE_T Length)
{
	x_memcpy(pvDestination, pvSource, Length);
}


void vImgreszSetMemory(void *pvDestination, u8 u1Value, SIZE_T Length)
{
	x_memset(pvDestination, 0, Length);
}


s32 ImgReszHwInstNotifyCallback(s32 i4State, void *pvPrivData)
{
	s32 i4Ret;

	i4Ret = x_ev_group_set_event(_arImgreszHwInst[(unsigned long)pvPrivData].hEventHandle,
				     IMGRESZ_EV_FINISH_SCALE, X_EV_OP_OR);
	VERIFY(i4Ret == OSR_OK);

	return 0;
}

/* extern void vHwImgReszWaitCountReset(u32 u4HwId); */

#if 0
static int framenum;
static char dumptempbuf[1920 * 1920];
static int writedumpfile(char *filename, void *data, u32 len)
{
	IMGR_LOG(IMGR_LOG_LVL_DBG, "writefile start filename %s  data 0x%p len %ld\n", filename, data, len);
	/*struct save_wavbuf *Ptrwav_buf = (struct save_wavbuf *) data;*/
	struct file *fd = NULL;
	/*uint32_t data_pos = 0;*/
	loff_t t_cur_pos = 0;

	ssize_t u4Size = 0;
	mm_segment_t fs;

	if (NULL == data) {
		return -1;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);
	fd = filp_open(filename , O_RDWR | O_CREAT | O_TRUNC , 0);

	if (IS_ERR(fd)) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "********************wavfile_fd is err*******************\n");
		set_fs(fs);
		return -1;
	}

	memcpy(dumptempbuf, data, len);
	u4Size = vfs_write(fd, dumptempbuf, len, &t_cur_pos);

	if (u4Size < 0) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "************************vfs write err %i****************\n", u4Size);
		filp_close(fd, NULL);
		set_fs(fs);
		return -1;
	}

	filp_close(fd, NULL);

	set_fs(fs);

	return 0;
} /* End of ktimer_thread */
#endif
#if 0
extern UINT32 u4HalGetTTB1(void);

extern UINT32 u4HalGetTTB0(void);

static void _hal_flush_d_cache_range(u32 u4Start, u32 u4End)
{
	u4Start = u4Start & ~(0x1F);	/* cache line align */
	__asm__ volatile ("1:\n"
			  "    MCR p15, 0, %0, c7, c14, 1\n"
			  "    ADD	%0, %0, #32\n"
			  "    CMP  %0, %1\n"
			  "    BLO 1b\n" "    ISB\n" "    DSB\n" :  : "r" (u4Start), "r"(u4End)
: "cc");
}

static void Img_FlushDCacheRange(u32 u4Start, u32 u4Len)
{
	_hal_flush_d_cache_range(u4Start, (u4Start + u4Len - 0x1));
	//IMGR_LOG(IMGR_LOG_LVL_ERR,"end flushDcacherange\n");
}

static u32 first_desc_addr(u32 vaddr)
{
	return (vaddr >= PAGE_OFFSET ? u4HalGetTTB1() : u4HalGetTTB0()) | ((vaddr & 0xFFF00000) >> 18);
}


static u32 sec_desc_addr(u32 vaddr)
{
	u32 first_table;
	u32 l2_index;

	first_table = *(u32 *)__va(first_desc_addr(vaddr));
	if (0x01 != (first_table & 0x3)) {
		pr_err("Error: the first table is error(0x%x).\n", vaddr);
		return 0;
	}
	l2_index = (vaddr & 0x000FF000) >> 12;

	return (first_table & 0xFFFFFC00) | (l2_index << 2);
}

static void Img_FlushPte(u32 vaddr, u32 size)
{
	u32 flush_cnt, flush_size, first_addr, addr_tmp;

	for (flush_size = 0, flush_cnt = 0, first_addr = 0, addr_tmp = 0;
			flush_size <= ((size + 0xC00) & 0xFFFFF000); flush_size += 0x1000, flush_cnt++) {
		first_addr = (u32)__va(first_desc_addr(vaddr + flush_size));
		if (first_addr != addr_tmp) {
			Img_FlushDCacheRange(first_addr, 4);
			addr_tmp = first_addr;
		}
		/* section mode, we just flush the 1st desc*/
		if ((*(u32 *)first_addr & 0x3) == 0x01)
			Img_FlushDCacheRange((u32)__va(sec_desc_addr(vaddr + flush_size)), 4);
	}
		IMGR_LOG(IMGR_LOG_LVL_DBG,"finish Img_flushPte\n");	
}

bool ImgreszSetParam(struct file * filp,IMGRESZ_MW_PARAM * pParam)
{
	unsigned long flags;
	
	
	
	if ((pParam->SrcColorMode!=IMGRESZ_DRV_INPUT_COL_MD_420_BLK) ||
		(pParam->DstColorMode!=IMGRESZ_DRV_OUTPUT_COL_MD_RGB_565)) {
		IMGR_LOG(IMGR_LOG_LVL_ERR,"source or dest color mode invalid \n");
		return false;
	}

	if ((pParam->SrcIsVirtual==true) || (pParam->DstIsVirtual==false)) {
		IMGR_LOG(IMGR_LOG_LVL_ERR,"source buffer or dest buffer invalid \n");
		return false;
	}

	if (((pParam->SrcBufWidth%16)!=0) || ((pParam->SrcBufHeight%32)!=0)) {
		IMGR_LOG(IMGR_LOG_LVL_ERR,"SrcBufWidth or SrcBufHeight invalid \n");
		return false;
		
	}

	if ((pParam->DstBufWidth%16)!=0) {
		IMGR_LOG(IMGR_LOG_LVL_ERR,"DstBufWidth invalid \n");
		return false;
		
	}
		
	IMGRESZ_DRV_TICKET_T ticket;
	if (i4ImgResz_Drv_GetTicket(&ticket)!=0) {
		IMGR_LOG(IMGR_LOG_LVL_ERR,"failed to get driver ticket\n");
		return false;
	}

	IMGR_LOG(IMGR_LOG_LVL_DBG,"get ticket now\n");

	spin_lock_irqsave(&filplock, flags);
	if (CurrFilp== NULL) {
		CurrFilp= filp;
		spin_unlock_irqrestore(&filplock, flags);
	} else {
		spin_unlock_irqrestore(&filplock, flags);
		IMGR_LOG(IMGR_LOG_LVL_ERR,"failed to set CurrFilp\n");
		i4ImgResz_Drv_ReleaseTicket(&ticket);
		IMGR_LOG(IMGR_LOG_LVL_ERR,"release ticket now\n");
		return false;
	}
	/*set source buffer*/
	memset(&SrcBuf,0,sizeof(IMGRESZ_HAL_IMG_INFO_T));

	SrcBuf.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
	SrcBuf.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
	SrcBuf.rBufferFormat.fgBlockMode = TRUE;
	SrcBuf.rBufferFormat.fgProgressiveFrame = TRUE;
	SrcBuf.rBufferFormat.fgAddrSwap = FALSE;

	SrcBuf.u4BufWidth = pParam->SrcBufWidth;
	SrcBuf.u4BufHeight = pParam->SrcBufHeight;
	SrcBuf.u4ImgWidth = pParam->SrcWidth;
	SrcBuf.u4ImgHeight = pParam->SrcHeight;
	SrcBuf.u4ImgXOff = pParam->SrcXoff;
	SrcBuf.u4ImgYOff = pParam->SrcYoff;
	SrcBuf.u4BufSA1 = pParam->SrcBuf[0];
	SrcBuf.u4BufSA2 = pParam->SrcBuf[1];

	/*set dest buffer*/
	memset(&DstBuf,0,sizeof(IMGRESZ_HAL_IMG_INFO_T));

	DstBuf.rBufferFormat.eBufferMainFormat =IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER;
	DstBuf.rBufferFormat.eARGBBufferFormat= IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888;
	DstBuf.rBufferFormat.fgBlockMode = FALSE;
	DstBuf.rBufferFormat.fgAddrSwap = FALSE;
	DstBuf.rBufferFormat.fgProgressiveFrame = TRUE;
	
	DstBuf.u4BufWidth = pParam->DstBufWidth*4;
	DstBuf.u4BufHeight =pParam->DstBufHeight;
	DstBuf.u4ImgWidth = pParam->DstWidth;
	DstBuf.u4ImgHeight = pParam->DstHeight;
	DstBuf.u4ImgXOff = pParam->DstXoff;
	DstBuf.u4ImgYOff = pParam->DstYoff;

	Dst = (void *)pParam->DstBuf[0];

	if (NeedToStop==true) {
		IMGR_LOG(IMGR_LOG_LVL_ERR,"Need to stop after getticket\n");
		NeedToStop=false;
		spin_lock_irqsave(&filplock, flags);
		CurrFilp= NULL;
		spin_unlock_irqrestore(&filplock, flags);
		IMGR_LOG(IMGR_LOG_LVL_ERR,"release ticket now\n");
		memset(&SrcBuf,0,sizeof(IMGRESZ_HAL_IMG_INFO_T));
		memset(&DstBuf,0,sizeof(IMGRESZ_HAL_IMG_INFO_T));
		StopCanReturn=true;
		i4ImgResz_Drv_ReleaseTicket(&ticket);
		return false;
	}

	return true;
}
bool ImgreszScaleFire(struct file * filp)
{
	unsigned long flags;

	IMGRESZ_DRV_TICKET_T ticket;

	ticket.u4Ticket=0;
	
	
	
	spin_lock_irqsave(&filplock, flags);
	if (CurrFilp!=filp) {
		IMGR_LOG(IMGR_LOG_LVL_ERR,"other process is using HW,cannot run ScaleFire\n");
		spin_unlock_irqrestore(&filplock, flags);
		return false;
	}
	spin_unlock_irqrestore(&filplock, flags);

	DstBuf.u4BufSA1= (__u32)vmalloc(DstBuf.u4BufHeight*DstBuf.u4BufWidth);

	IMGR_LOG(IMGR_LOG_LVL_DBG,"DstBuf.u4BufSA1 is %x\n",DstBuf.u4BufSA1);

	i4ImgResz_HAL_Init(0, u4HalGetTTB1());
	i4ImgResz_HAL_Set_Resize_Mode(0, IMGRESZ_HAL_RESIZE_MODE_FRAME);
	i4ImgResz_HAL_Set_Resample_Method(0, IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR,
					  IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR);
	i4ImgResz_HAL_Set_LumaKey(0, 0, 0);
	i4ImgResz_HAL_Set_Source_Image_Info(0, &SrcBuf);
	i4ImgResz_HAL_Set_Destination_Image_Info(0, &DstBuf);
	Img_FlushPte(DstBuf.u4BufSA1,DstBuf.u4BufHeight*DstBuf.u4BufWidth);
	Img_FlushDCacheRange(DstBuf.u4BufSA1,DstBuf.u4BufHeight*DstBuf.u4BufWidth);
	i4ImgResz_HAL_Resize(0);
	
	while (TRUE) {
		if (i4ImgResz_HAL_Get_Resize_Status(0) >= 0||NeedToStop==true) {
			IMGR_LOG(IMGR_LOG_LVL_DBG,"exit while\n");	
			break;
		}
		/*Sleep(1);*/
	}
	IMGR_LOG(IMGR_LOG_LVL_DBG,"exit while\n");	

	if (NeedToStop==true) {
		i4ImgResz_HAL_Uninit(0);
		IMGR_LOG(IMGR_LOG_LVL_ERR,"need to stop when scaling\n");
		NeedToStop=false;
		spin_lock_irqsave(&filplock, flags);
		CurrFilp= NULL;
		spin_unlock_irqrestore(&filplock, flags);
		vfree((void *)DstBuf.u4BufSA1);
		IMGR_LOG(IMGR_LOG_LVL_ERR,"release ticket now\n");
		memset(&SrcBuf,0,sizeof(IMGRESZ_HAL_IMG_INFO_T));
		memset(&DstBuf,0,sizeof(IMGRESZ_HAL_IMG_INFO_T));
		StopCanReturn=true;
		i4ImgResz_Drv_ReleaseTicket(&ticket);
		return false;
	}

	i4ImgResz_HAL_Uninit(0);
	
	IMGR_LOG(IMGR_LOG_LVL_DBG,"begin second \n");
	SrcBuf.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER;
	SrcBuf.rBufferFormat.eARGBBufferFormat = IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888;
	SrcBuf.rBufferFormat.fgBlockMode = FALSE;
	SrcBuf.rBufferFormat.fgProgressiveFrame = TRUE;
	SrcBuf.rBufferFormat.fgAddrSwap = FALSE;
	SrcBuf.u4BufWidth = DstBuf.u4BufWidth;
	SrcBuf.u4BufHeight = DstBuf.u4BufHeight;
	SrcBuf.u4ImgWidth = DstBuf.u4ImgWidth;
	SrcBuf.u4ImgHeight = DstBuf.u4ImgHeight;
	/**/
	SrcBuf.u4BufSA1 = DstBuf.u4BufSA1;
	SrcBuf.u4BufSA2 = 0;
	
	DstBuf.rBufferFormat.eBufferMainFormat =IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER;
	DstBuf.rBufferFormat.eARGBBufferFormat= IMGRESZ_HAL_ARGB_BUFFER_FORMAT_0565;
	DstBuf.u4BufWidth = DstBuf.u4BufWidth / 2;
	
	DstBuf.u4BufSA1= (__u32)vmalloc(DstBuf.u4BufWidth*DstBuf.u4BufHeight);
	
	IMGR_LOG(IMGR_LOG_LVL_DBG,"DstBuf.u4BufSA1 is %x\n",DstBuf.u4BufSA1);

	Img_FlushPte(DstBuf.u4BufSA1,DstBuf.u4BufHeight*DstBuf.u4BufWidth);
	
	Img_FlushDCacheRange(DstBuf.u4BufSA1,DstBuf.u4BufHeight*DstBuf.u4BufWidth);
	
	i4ImgResz_HAL_Init(0, u4HalGetTTB1());
	i4ImgResz_HAL_Set_Resize_Mode(0, IMGRESZ_HAL_RESIZE_MODE_FRAME);
	i4ImgResz_HAL_Set_Resample_Method(0, IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR,
					  IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR);
	i4ImgResz_HAL_Set_LumaKey(0, 0, 0);
	i4ImgResz_HAL_Set_Source_Image_Info(0, &SrcBuf);
	i4ImgResz_HAL_Set_Destination_Image_Info(0, &DstBuf);
	Img_FlushPte(SrcBuf.u4BufSA1,SrcBuf.u4BufHeight*SrcBuf.u4BufWidth);
	Img_FlushDCacheRange(SrcBuf.u4BufSA1,SrcBuf.u4BufHeight*SrcBuf.u4BufWidth);
	Img_FlushPte(DstBuf.u4BufSA1,DstBuf.u4BufHeight*DstBuf.u4BufWidth);
	Img_FlushDCacheRange(DstBuf.u4BufSA1,DstBuf.u4BufHeight*DstBuf.u4BufWidth);
	i4ImgResz_HAL_Resize(0);

	while (TRUE) {
		if (i4ImgResz_HAL_Get_Resize_Status(0) >= 0||NeedToStop==true) {
			break;
		}
		/*Sleep(1);*/
	}

	if (NeedToStop==true) {
		i4ImgResz_HAL_Uninit(0);
		IMGR_LOG(IMGR_LOG_LVL_ERR,"need to stop when scaling\n");
		NeedToStop=false;
		spin_lock_irqsave(&filplock, flags);
		CurrFilp= NULL;
		spin_unlock_irqrestore(&filplock, flags);
		IMGR_LOG(IMGR_LOG_LVL_ERR,"release ticket now\n");
		vfree((void *)DstBuf.u4BufSA1);
		vfree((void *)SrcBuf.u4BufSA1);
		memset(&SrcBuf,0,sizeof(IMGRESZ_HAL_IMG_INFO_T));
		memset(&DstBuf,0,sizeof(IMGRESZ_HAL_IMG_INFO_T));
		StopCanReturn=true;
		i4ImgResz_Drv_ReleaseTicket(&ticket);
		return false;
	}

	i4ImgResz_HAL_Uninit(0);
	
	copy_to_user(Dst,(void *)DstBuf.u4BufSA1,DstBuf.u4BufHeight*DstBuf.u4BufWidth);
	spin_lock_irqsave(&filplock, flags);
	CurrFilp= NULL;
	spin_unlock_irqrestore(&filplock, flags);
	vfree((void *)DstBuf.u4BufSA1);
	vfree((void *)SrcBuf.u4BufSA1);
	IMGR_LOG(IMGR_LOG_LVL_DBG,"release ticket now\n");
	memset(&SrcBuf,0,sizeof(IMGRESZ_HAL_IMG_INFO_T));
	memset(&DstBuf,0,sizeof(IMGRESZ_HAL_IMG_INFO_T));
	i4ImgResz_Drv_ReleaseTicket(&ticket);

	return true;	
}
bool ImgreszStopScale(struct file * filp)
{
	unsigned long flags;
	

	spin_lock_irqsave(&filplock, flags);

	if (CurrFilp==NULL) {
	spin_unlock_irqrestore(&filplock, flags);
	return true;
	}
	spin_unlock_irqrestore(&filplock, flags);
	
	while (true) {
		spin_lock_irqsave(&filplock, flags);
		if(CurrFilp==filp) {
			spin_unlock_irqrestore(&filplock, flags);
			break;
			}
		spin_unlock_irqrestore(&filplock, flags);
		Sleep(1);
	}

	NeedToStop=true;

	while (StopCanReturn!=true) {
		Sleep(1);
	}

	StopCanReturn=false;
	
	return true;
}
#endif
s32 YUV420BlkToARGB8888(u32 u4HwId, VOID   *prSrcImgInfo,
			  VOID  *prDestImgInfo)
{
	s32 i4Ret = 0;
	/*u32 u4SrcBuf = 0;*/
	/*u32 u4DestBuf = 0;*/

	IMGRESZ_HAL_IMG_INFO_T *prSrc, *prDst;

	prSrc = (IMGRESZ_HAL_IMG_INFO_T *)prSrcImgInfo;
	prDst = (IMGRESZ_HAL_IMG_INFO_T *)prDestImgInfo;

	/*step 1:convert yuv420blk to ayuv8888*/
	prDst->rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER;
	i4ImgResz_HAL_Init(u4HwId, 0);
	i4ImgResz_HAL_Set_Resize_Mode(u4HwId, IMGRESZ_HAL_RESIZE_MODE_FRAME);
	i4ImgResz_HAL_Set_Resample_Method(u4HwId, IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR,
					  IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR);
	i4ImgResz_HAL_Set_LumaKey(u4HwId, 0, 0);
	i4ImgResz_HAL_Set_Source_Image_Info(u4HwId, prSrc);

	i4ImgResz_HAL_Set_Destination_Image_Info(u4HwId, prDst);
	i4ImgResz_HAL_Resize(u4HwId);

	while (TRUE) {
		if (i4ImgResz_HAL_Get_Resize_Status(u4HwId) >= 0) {
			break;
		}
		msleep(1);
		/*Sleep(1);*/
	}

	i4ImgResz_HAL_Uninit(u4HwId);
	prSrc->rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER;
	prSrc->rBufferFormat.eARGBBufferFormat = IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888;
	prSrc->rBufferFormat.fgBlockMode = FALSE;
	prSrc->rBufferFormat.fgProgressiveFrame = TRUE;
	prSrc->rBufferFormat.fgAddrSwap = FALSE;
	prSrc->u4BufWidth = prSrc->u4ImgWidth * 4;
	/**/
	prSrc->u4BufWidth = prDst->u4BufWidth;
	prSrc->u4BufHeight = prDst->u4BufHeight;
	prSrc->u4ImgWidth = prDst->u4ImgWidth;
	prSrc->u4ImgHeight = prDst->u4ImgHeight;
	/**/
	prSrc->u4BufSA1 = prDst->u4BufSA1;
	prSrc->u4BufSA2 = 0;

	prDst->rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER;
	i4ImgResz_HAL_Init(u4HwId, 0);
	i4ImgResz_HAL_Set_Resize_Mode(u4HwId, IMGRESZ_HAL_RESIZE_MODE_FRAME);
	i4ImgResz_HAL_Set_Resample_Method(u4HwId, IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR,
					  IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR);
	i4ImgResz_HAL_Set_LumaKey(u4HwId, 0, 0);
	i4ImgResz_HAL_Set_Source_Image_Info(u4HwId, prSrc);

	i4ImgResz_HAL_Set_Destination_Image_Info(u4HwId, prDst);
	i4ImgResz_HAL_Resize(u4HwId);

	while (TRUE) {
		if (i4ImgResz_HAL_Get_Resize_Status(u4HwId) >= 0) {
			break;
		}
		msleep(1);
		/*Sleep(1);*/
	}

	i4ImgResz_HAL_Uninit(u4HwId);
	return i4Ret;
}
EXPORT_SYMBOL(YUV420BlkToARGB8888);
void vYUV420_Block_TO_ARGB8888(void *lpInBuffer)
{
	IMGRESZ_HAL_IMG_INFO_T rSrcBufInfo;
	IMGRESZ_HAL_IMG_INFO_T rDstBufInfo;
	YUV420BLOCK_TO_ARGB8888_BUF_T *yc_to_rgb = (YUV420BLOCK_TO_ARGB8888_BUF_T *)lpInBuffer;

	/*	void *vaddr;*/

	/* Set source buffer info*/
	memset(&rSrcBufInfo, 0, sizeof(rSrcBufInfo));
	rSrcBufInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
	rSrcBufInfo.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
	rSrcBufInfo.rBufferFormat.fgBlockMode = yc_to_rgb->fgBlock;//TRUE;
	rSrcBufInfo.rBufferFormat.fgProgressiveFrame = TRUE;
	rSrcBufInfo.rBufferFormat.fgAddrSwap = FALSE;

	rSrcBufInfo.u4BufWidth = IMGRESZ_BUF_ALIGN_MASK(yc_to_rgb->bufwidth, 16);
	rSrcBufInfo.u4BufHeight = IMGRESZ_BUF_ALIGN_MASK(yc_to_rgb->bufheight, 32);
	rSrcBufInfo.u4ImgWidth = yc_to_rgb->picwidth;
	rSrcBufInfo.u4ImgHeight = yc_to_rgb->picheight;
	rSrcBufInfo.u4ImgXOff = 0;
	rSrcBufInfo.u4ImgYOff = 0;
	rSrcBufInfo.u4BufSA1 = yc_to_rgb->ycbuf[0];
	rSrcBufInfo.u4BufSA2 = yc_to_rgb->ycbuf[1];
	/* Set target buffer info*/
	memset(&rDstBufInfo, 0, sizeof(rSrcBufInfo));

	/*vaddr = OSE_MemAllocCustom(OSE_DEMUXER, yc_to_rgb->width * yc_to_rgb->height * 4, 0x1000, &u4PhysAddr);*/
	rDstBufInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER;
	rDstBufInfo.rBufferFormat.eARGBBufferFormat = IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888;
	rDstBufInfo.rBufferFormat.fgBlockMode = FALSE;
	rDstBufInfo.rBufferFormat.fgAddrSwap = FALSE;
	rDstBufInfo.rBufferFormat.fgProgressiveFrame = TRUE;
	rDstBufInfo.u4BufWidth = yc_to_rgb->dstWidth * 4;
	rDstBufInfo.u4BufHeight = yc_to_rgb->dstHeight;
	rDstBufInfo.u4ImgWidth = yc_to_rgb->dstWidth;
	rDstBufInfo.u4ImgHeight = yc_to_rgb->dstHeight;
	rDstBufInfo.u4ImgXOff = 0;
	rDstBufInfo.u4ImgYOff = 0;
	rDstBufInfo.u4BufSA1 = yc_to_rgb->u4DestARGB8888Pa;

	YUV420BlkToARGB8888(1,  &rSrcBufInfo, &rDstBufInfo);

#if 0

	if (yc_to_rgb->vaddr) {
		copy_to_user(yc_to_rgb->vaddr,
			     vaddr, yc_to_rgb->width * yc_to_rgb->height * 4);
		IMGR_LOG(IMGR_LOG_LVL_DBG "yc_to_rgb->vaddr = %p, width = %d,h = %d", yc_to_rgb->vaddr,
			 yc_to_rgb->width, yc_to_rgb->height);
	}

	OSE_MemFreeCustom(OSE_DEMUXER, vaddr);

#endif
}
EXPORT_SYMBOL(vYUV420_Block_TO_ARGB8888);

void vYUV420_Block_TO_NV12(void *lpInBuffer1){
	IMGRESZ_HAL_IMG_INFO_T rSrcBufInfo;
	IMGRESZ_HAL_IMG_INFO_T rDstBufInfo;
	YUV420BLOCK_TO_NV12_BUF_T * lpInBuffer = (YUV420BLOCK_TO_NV12_BUF_T *)lpInBuffer1 ;

	//set source buffer
	memset(&rSrcBufInfo, 0, sizeof(rSrcBufInfo));
	rSrcBufInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
	rSrcBufInfo.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
	rSrcBufInfo.rBufferFormat.fgBlockMode = TRUE;
	rSrcBufInfo.rBufferFormat.fgProgressiveFrame = TRUE;
	rSrcBufInfo.rBufferFormat.fgAddrSwap = FALSE;

	rSrcBufInfo.u4BufWidth = (lpInBuffer->srcbufwidth);
	rSrcBufInfo.u4BufHeight = (lpInBuffer->srcbufheight);
	rSrcBufInfo.u4ImgWidth = lpInBuffer->srcwidth;
	rSrcBufInfo.u4ImgHeight = lpInBuffer->srcheight;
	rSrcBufInfo.u4ImgXOff = 0;
	rSrcBufInfo.u4ImgYOff = 0;
	rSrcBufInfo.u4BufSA1 = lpInBuffer->srcbuf[0];
	rSrcBufInfo.u4BufSA2 = lpInBuffer->srcbuf[1];

	//set dest buffer
	memset(&rDstBufInfo, 0, sizeof(rDstBufInfo));
	rDstBufInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
	rDstBufInfo.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
	rDstBufInfo.rBufferFormat.fgBlockMode = FALSE;
	rDstBufInfo.rBufferFormat.fgProgressiveFrame = TRUE;
	rDstBufInfo.rBufferFormat.fgAddrSwap = FALSE;

	rDstBufInfo.u4BufWidth = (lpInBuffer->dstbufwidth);
	rDstBufInfo.u4BufHeight = (lpInBuffer->dstbufheight);
	rDstBufInfo.u4ImgWidth = lpInBuffer->dstwidth;
	rDstBufInfo.u4ImgHeight = lpInBuffer->dstheight;
	rDstBufInfo.u4ImgXOff = 0;
	rDstBufInfo.u4ImgYOff = 0;
	rDstBufInfo.u4BufSA1 = lpInBuffer->dstbuf[0];
	rDstBufInfo.u4BufSA2 = lpInBuffer->dstbuf[1];

	i4ImgResz_HAL_Init(1, 0);
	i4ImgResz_HAL_Set_Resize_Mode(1, IMGRESZ_HAL_RESIZE_MODE_FRAME);
	i4ImgResz_HAL_Set_Resample_Method(1, IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR,
					  IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR);
	i4ImgResz_HAL_Set_LumaKey(1, 0, 0);
	i4ImgResz_HAL_Set_Source_Image_Info(1, &(rSrcBufInfo));

	i4ImgResz_HAL_Set_Destination_Image_Info(1, &(rDstBufInfo));
	i4ImgResz_HAL_Resize(1);

	while (TRUE) {
		if (i4ImgResz_HAL_Get_Resize_Status(1) >= 0) {
			break;
		}
		msleep(1);
		//Sleep(1);
	}

	i4ImgResz_HAL_Uninit(1);
	
}
EXPORT_SYMBOL(vYUV420_Block_TO_NV12);

s32 TS_DirectScale(u32 u4HwId, bool fg8Tap, VOID   *prSrcImgInfo,
		     VOID  *prDestImgInfo)
{
	s32 i4Ret = 0;
	/*u32 u4SrcBuf = 0;*/
	/*u32 u4DestBuf = 0;*/

	IMGRESZ_HAL_IMG_INFO_T *prSrc, *prDst;

	prSrc = (IMGRESZ_HAL_IMG_INFO_T *)prSrcImgInfo;
	prDst = (IMGRESZ_HAL_IMG_INFO_T *)prDestImgInfo;

	if ((prSrc->u4ImgWidth < 4) || (prSrc->u4ImgHeight < 4) ||
	    (prDst->u4ImgWidth < 4) || (prDst->u4ImgHeight < 4)) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "[imgresz] TS_DirectScale error args src %d x %d dst %d x %d \r\n ",
			 (int)prSrc->u4ImgWidth, (int)prSrc->u4ImgHeight, (int)prDst->u4ImgWidth,
			 (int)prDst->u4ImgHeight);

		return FALSE;
	}


	i4ImgResz_HAL_Init(u4HwId, 0);
	i4ImgResz_HAL_Set_Resize_Mode(u4HwId, IMGRESZ_HAL_RESIZE_MODE_FRAME);

	if (fg8Tap) {
		i4ImgResz_HAL_Set_Resample_Method(u4HwId, IMGRESZ_HAL_RESAMPLE_METHOD_8_TAP,
						  IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR);
	} else {
		i4ImgResz_HAL_Set_Resample_Method(u4HwId, IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR,
						  IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR);
	}

	i4ImgResz_HAL_Set_LumaKey(u4HwId, 0, 0);
	i4ImgResz_HAL_Set_Source_Image_Info(u4HwId, prSrc);
	i4ImgResz_HAL_Set_Destination_Image_Info(u4HwId, prDst);
	i4ImgResz_HAL_Resize(u4HwId);

	while (TRUE) {
		if (i4ImgResz_HAL_Get_Resize_Status(u4HwId) >= 0) {
			break;
		}
		msleep(1);
		/*Sleep(1);*/
	}

	i4ImgResz_HAL_Uninit(u4HwId);
	return i4Ret;
}
EXPORT_SYMBOL(TS_DirectScale);

s32 TS_DirectScale2(u32 u4HwId, VOID   *prSrcImgInfo,
		      VOID  *prDestImgInfo)
{
	s32 i4Ret = 0;
	/*u32 u4SrcBuf = 0;*/
	/*u32 u4DestBuf = 0;*/
	unsigned long u4TempLineBufSa = 0;
	IMGRESZ_HAL_IMG_INFO_T *prSrc, *prDst;
	IMGRESZ_HAL_JPEG_INFO_T rJpegInfo;
	IMGRESZ_HAL_PARTIAL_BUF_INFO_T rSrcRowBufInfo;

	prSrc = (IMGRESZ_HAL_IMG_INFO_T *)prSrcImgInfo;
	prDst = (IMGRESZ_HAL_IMG_INFO_T *)prDestImgInfo;


	i4ImgResz_HAL_Init(u4HwId, 0);
	i4ImgResz_HAL_Set_Resize_Mode(u4HwId, IMGRESZ_HAL_RESIZE_MODE_FRAME);
	i4ImgResz_HAL_Set_Resample_Method(u4HwId, IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR,
					  IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR);
	i4ImgResz_HAL_Set_LumaKey(u4HwId, 0, 0);
	i4ImgResz_HAL_Set_Source_Image_Info(u4HwId, prSrc);
	i4ImgResz_HAL_Set_Destination_Image_Info(u4HwId, prDst);

	memset(&rJpegInfo, 0, sizeof(IMGRESZ_HAL_JPEG_INFO_T));

	rJpegInfo.fgPictureMode = FALSE;
	rJpegInfo.fgPreloadMode = FALSE;
	rJpegInfo.fgYExist = TRUE;
	rJpegInfo.fgCbExist = TRUE;
	rJpegInfo.fgCrExist = TRUE;
	i4ImgResz_HAL_Set_Jpeg_Info(u4HwId, &rJpegInfo);

	memset(&rSrcRowBufInfo, 0, sizeof(IMGRESZ_HAL_PARTIAL_BUF_INFO_T));


	u4TempLineBufSa = (unsigned long)x_alloc_aligned_nc_mem(prDst->u4BufWidth * 4 * 3, 16);
	VERIFY(u4TempLineBufSa != 0);

	rSrcRowBufInfo.u4RowBufHeight = 0;

	rSrcRowBufInfo.fgFirstRowBuf = TRUE;
	rSrcRowBufInfo.fgLastRowBuf = FALSE;

	i4ImgResz_HAL_Set_Partial_Mode_Info(u4HwId, &rSrcRowBufInfo, u4TempLineBufSa);

	i4ImgResz_HAL_Resize(u4HwId);

	while (TRUE) {
		if (i4ImgResz_HAL_Get_Resize_Status(u4HwId) >= 0) {
			break;
		}
		msleep(1);
		/*Sleep(1);*/
	}

	x_free_aligned_nc_mem((void *)u4TempLineBufSa);
	i4ImgResz_HAL_Uninit(u4HwId);
	return i4Ret;
}
EXPORT_SYMBOL(TS_DirectScale2);

void vImgReszHwInstResz(u32 u4HwId, IMGRESZ_INST_T *prImgReszInst)
{
	IMGRESZ_HAL_IMG_INFO_T rSrcImgInfo;
	IMGRESZ_HAL_IMG_INFO_T rDestImgInfo;
	IMGRESZ_HAL_IMG_INFO_T rBldImgInfo;
	IMGRESZ_HAL_NOTIFY_CB_REG_T rNofifyCallback;
	bool fgPartialMode = (prImgReszInst->eImgReszScaleMd == IMGRESZ_DRV_PARTIAL_SCALE)
			     || (prImgReszInst->eImgReszScaleMd == IMGRESZ_DRV_JPEG_PIC_SCALE);
	bool fgInterlaced = _arImgreszHwInst[u4HwId].fgInterlaced;

	IMGRESZ_HAL_RESAMPLE_METHOD_T eHResampleMethod = _eHResampleMethod;/*IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR;*/
	IMGRESZ_HAL_RESAMPLE_METHOD_T eVResampleMethod = _eVResampleMethod;/*IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR;*/
	IMGR_LOG(IMGR_LOG_LVL_DBG,"enter imgresz hwinstresz\n");

	if (!fgPartialMode || prImgReszInst->tImgReszPartialBufInfo.fgFirstRow) {
		/* Initialization*/
		i4ImgResz_HAL_Init(u4HwId, prImgReszInst->u4tableaddr);

		/* Set resize mode*/
		if (fgPartialMode) {
			i4ImgResz_HAL_Set_Resize_Mode(u4HwId, IMGRESZ_HAL_RESIZE_MODE_PARTIAL);
		} else {
			i4ImgResz_HAL_Set_Resize_Mode(u4HwId, IMGRESZ_HAL_RESIZE_MODE_FRAME);
		}

		/* Set resize method*/
		if (prImgReszInst->fgLumaKeyEnable) {
			eVResampleMethod = IMGRESZ_HAL_RESAMPLE_METHOD_4_TAP;
		}

		i4ImgResz_HAL_Set_Resample_Method(u4HwId, eHResampleMethod,
						  eVResampleMethod);

		/* Set luma key*/
		i4ImgResz_HAL_Set_LumaKey(u4HwId, prImgReszInst->fgLumaKeyEnable, prImgReszInst->u1LumaKey);

		/* Set 1:1 scaling*/
		if (prImgReszInst->fg1To1Scale) {
			i4ImgResz_HAL_Set_Scale1to1(u4HwId, prImgReszInst->fg1To1Scale);
		}

		/* Set 1/4 scaling , Y component only*/
		if (prImgReszInst->fgYSrcOnly) {
			i4ImgResz_HAL_Set_Scale4to1(u4HwId, prImgReszInst->fgYSrcOnly);
		}

		/* Set source buffer info*/
		x_memset(&rSrcImgInfo, 0, sizeof(IMGRESZ_HAL_IMG_INFO_T));
		rSrcImgInfo.rBufferFormat.fgBlockMode = FALSE;
		rSrcImgInfo.rBufferFormat.fgAddrSwap = FALSE;

		switch (prImgReszInst->tImgReszSrcBufInfo.eSrcColorMode) {
		case IMGRESZ_DRV_INPUT_COL_MD_ARGB_8888:
			rSrcImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER;
			rSrcImgInfo.rBufferFormat.eARGBBufferFormat = IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888;
			break;

		case IMGRESZ_DRV_INPUT_COL_MD_ARGB_4444:
			rSrcImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER;
			rSrcImgInfo.rBufferFormat.eARGBBufferFormat = IMGRESZ_HAL_ARGB_BUFFER_FORMAT_4444;
			break;

		case IMGRESZ_DRV_INPUT_COL_MD_ARGB_1555:
			rSrcImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER;
			rSrcImgInfo.rBufferFormat.eARGBBufferFormat = IMGRESZ_HAL_ARGB_BUFFER_FORMAT_1555;
			break;

		case IMGRESZ_DRV_INPUT_COL_MD_RGB_565:
			rSrcImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER;
			rSrcImgInfo.rBufferFormat.eARGBBufferFormat = IMGRESZ_HAL_ARGB_BUFFER_FORMAT_0565;
			break;

		case IMGRESZ_DRV_INPUT_COL_MD_8BPP_IDX:
			rSrcImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER;
			rSrcImgInfo.rBufferFormat.eIndexBufferFormat = IMGRESZ_HAL_INDEX_BUFFER_FORMAT_8BPP;
			rSrcImgInfo.rBufferFormat.prColorPallet =
				(IMGRESZ_HAL_ARGB_COLOR_T *)prImgReszInst->abColorPallet;
			break;

		case IMGRESZ_DRV_INPUT_COL_MD_4BPP_IDX:
			rSrcImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER;
			rSrcImgInfo.rBufferFormat.eIndexBufferFormat = IMGRESZ_HAL_INDEX_BUFFER_FORMAT_4BPP;
			rSrcImgInfo.rBufferFormat.prColorPallet =
				(IMGRESZ_HAL_ARGB_COLOR_T *)prImgReszInst->abColorPallet;
			break;

		case IMGRESZ_DRV_INPUT_COL_MD_2BPP_IDX:
			rSrcImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER;
			rSrcImgInfo.rBufferFormat.eIndexBufferFormat = IMGRESZ_HAL_INDEX_BUFFER_FORMAT_2BPP;
			rSrcImgInfo.rBufferFormat.prColorPallet =
				(IMGRESZ_HAL_ARGB_COLOR_T *)prImgReszInst->abColorPallet;
			break;

		case IMGRESZ_DRV_INPUT_COL_MD_AYUV:
			rSrcImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER;
			rSrcImgInfo.rBufferFormat.eARGBBufferFormat = IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888;
			break;

		case IMGRESZ_DRV_INPUT_COL_MD_JPG_DEF:
			IMGR_LOG(IMGR_LOG_LVL_DBG,"source color mode JPG_DEF\n");
			rSrcImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER;
			rSrcImgInfo.rBufferFormat.fgJpg = TRUE;

			rSrcImgInfo.rBufferFormat.u4HSampleFactor[0] =
				prImgReszInst->tImgReszSrcBufInfo.rCompFactor.u1YCompFactorH;
			rSrcImgInfo.rBufferFormat.u4HSampleFactor[1] =
				prImgReszInst->tImgReszSrcBufInfo.rCompFactor.u1CbCompFactorH;
			rSrcImgInfo.rBufferFormat.u4HSampleFactor[2] =
				prImgReszInst->tImgReszSrcBufInfo.rCompFactor.u1CrCompFactorH;
			rSrcImgInfo.rBufferFormat.u4VSampleFactor[0] =
				prImgReszInst->tImgReszSrcBufInfo.rCompFactor.u1YCompFactorV;
			rSrcImgInfo.rBufferFormat.u4VSampleFactor[1] =
				prImgReszInst->tImgReszSrcBufInfo.rCompFactor.u1CbCompFactorV;
			rSrcImgInfo.rBufferFormat.u4VSampleFactor[2] =
				prImgReszInst->tImgReszSrcBufInfo.rCompFactor.u1CrCompFactorV;
			break;

		case IMGRESZ_DRV_INPUT_COL_MD_420_BLK:
			rSrcImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
			rSrcImgInfo.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
			rSrcImgInfo.rBufferFormat.fgBlockMode = TRUE;
			rSrcImgInfo.rBufferFormat.fgAddrSwap = TRUE;
			break;

		default:
			break;
		}

		rSrcImgInfo.rBufferFormat.fgProgressiveFrame = TRUE;

		rSrcImgInfo.u4BufWidth = prImgReszInst->tImgReszSrcBufInfo.u4BufWidth;
		rSrcImgInfo.u4BufHeight = prImgReszInst->tImgReszSrcBufInfo.u4BufHeight;
		rSrcImgInfo.u4ImgWidth = prImgReszInst->tImgReszSrcBufInfo.u4PicWidth;
		rSrcImgInfo.u4ImgHeight = prImgReszInst->tImgReszSrcBufInfo.u4PicHeight;

		rSrcImgInfo.u4ImgXOff = prImgReszInst->tImgReszSrcBufInfo.u4PicPosX;
		rSrcImgInfo.u4ImgYOff = prImgReszInst->tImgReszSrcBufInfo.u4PicPosY;
		rSrcImgInfo.u4BufSA1 = prImgReszInst->tImgReszSrcBufInfo.u4YBufAddr;
		rSrcImgInfo.u4BufSA2 = prImgReszInst->tImgReszSrcBufInfo.u4CbBufAddr;
		rSrcImgInfo.u4BufSA3 = prImgReszInst->tImgReszSrcBufInfo.u4CrBufAddr;

		rSrcImgInfo.rBufferFormat.fgWT = prImgReszInst->tImgReszSrcBufInfo.fgWTEnable;
		IMGR_LOG(IMGR_LOG_LVL_DBG,"src bufwidth:%d,bufheight:%d,imgwidth:%d,imgheight:%d\n",rSrcImgInfo.u4BufWidth,
			rSrcImgInfo.u4BufHeight,rSrcImgInfo.u4ImgWidth,rSrcImgInfo.u4ImgHeight);
		IMGR_LOG(IMGR_LOG_LVL_DBG,"src x off:%d,y off:%d,bufsa1:%lx,bufsa2:%lx,bufsa3:%lx\n",rSrcImgInfo.u4ImgXOff,rSrcImgInfo.u4ImgYOff,
			rSrcImgInfo.u4BufSA1,rSrcImgInfo.u4BufSA2,rSrcImgInfo.u4BufSA3);

		i4ImgResz_HAL_Set_Source_Image_Info(u4HwId, &rSrcImgInfo);
		/* Set destination buffer info*/
		x_memset(&rDestImgInfo, 0, sizeof(IMGRESZ_HAL_IMG_INFO_T));
		rDestImgInfo.rBufferFormat.fgBlockMode = FALSE;
		rDestImgInfo.rBufferFormat.fgAddrSwap = FALSE;

		switch (prImgReszInst->tImgReszDstBufInfo.eDstColorMode) {
		case IMGRESZ_DRV_OUTPUT_COL_MD_RGB_565:
			rDestImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER;
			rDestImgInfo.rBufferFormat.eARGBBufferFormat = IMGRESZ_HAL_ARGB_BUFFER_FORMAT_0565;
			break;

		case IMGRESZ_DRV_OUTPUT_COL_MD_ARGB_8888:
			rDestImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER;
			rDestImgInfo.rBufferFormat.eARGBBufferFormat = IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888;
			break;

		case IMGRESZ_DRV_OUTPUT_COL_MD_ARGB_4444:
			rDestImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER;
			rDestImgInfo.rBufferFormat.eARGBBufferFormat = IMGRESZ_HAL_ARGB_BUFFER_FORMAT_4444;
			break;

		case IMGRESZ_DRV_OUTPUT_COL_MD_AYUV:
			IMGR_LOG(IMGR_LOG_LVL_DBG,"dst color mode AYUV\n");
			rDestImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER;
			rDestImgInfo.rBufferFormat.eARGBBufferFormat = IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888;
			break;

		case IMGRESZ_DRV_OUTPUT_COL_MD_420_RS:
			rDestImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
			rDestImgInfo.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
			break;

		case IMGRESZ_DRV_OUTPUT_COL_MD_420_BLK:
			rDestImgInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
			rDestImgInfo.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
			rDestImgInfo.rBufferFormat.fgBlockMode = TRUE;
			rDestImgInfo.rBufferFormat.fgAddrSwap = TRUE;
			break;

		default:
			break;
		}

		rDestImgInfo.rBufferFormat.fgProgressiveFrame = TRUE;

		rDestImgInfo.u4BufWidth = prImgReszInst->tImgReszDstBufInfo.u4BufWidth;
		rDestImgInfo.u4BufHeight = prImgReszInst->tImgReszDstBufInfo.u4BufHeight;
		rDestImgInfo.u4ImgWidth = prImgReszInst->tImgReszDstBufInfo.u4PicWidth;

		if (rDestImgInfo.u4ImgWidth == 1) {
			rDestImgInfo.u4ImgWidth = 2;
		}

		rDestImgInfo.u4ImgHeight = prImgReszInst->tImgReszDstBufInfo.u4PicHeight;

		rDestImgInfo.u4ImgXOff = prImgReszInst->tImgReszDstBufInfo.u4PicPosX;
		rDestImgInfo.u4ImgYOff = prImgReszInst->tImgReszDstBufInfo.u4PicPosY;
		rDestImgInfo.u4BufSA1 = prImgReszInst->tImgReszDstBufInfo.u4YBufAddr;
		rDestImgInfo.u4BufSA2 = prImgReszInst->tImgReszDstBufInfo.u4CBufAddr;

		if (prImgReszInst->tImgReszRmInfo.fgRPRMode) {
			/* RPR mode should be 16 alignment*/
			rDestImgInfo.u4ImgHeight = (rDestImgInfo.u4ImgHeight + 15) / 16 * 16;
		}
		IMGR_LOG(IMGR_LOG_LVL_DBG,"dst bufwidth:%d.bufheight:%d,imgwidth:%d,imgheight:%d\n",rDestImgInfo.u4BufWidth,
			rDestImgInfo.u4BufHeight,rDestImgInfo.u4ImgWidth,rDestImgInfo.u4ImgHeight);
		IMGR_LOG(IMGR_LOG_LVL_DBG,"dst x off:%d,y off:%d,bufsa1:%lx,bufsa2:%lx\n",rDestImgInfo.u4ImgXOff,rDestImgInfo.u4ImgYOff,
			rDestImgInfo.u4BufSA1,rDestImgInfo.u4BufSA2);

		rDestImgInfo.rBufferFormat.fgWT = prImgReszInst->tImgReszDstBufInfo.fgWTEnable;

		i4ImgResz_HAL_Set_Destination_Image_Info(u4HwId, &rDestImgInfo);
		/* Set blending buffer info*/
		x_memset(&rBldImgInfo, 0, sizeof(IMGRESZ_HAL_IMG_INFO_T));
		/*rBldImgInfo.rBufferFormat // Blending buffer format is the same as destination buffer*/
		/*rBldImgInfo.u4BufWidth    // Blending buffer width is the same as destination buffer*/
		/*rBldImgInfo.u4ImgHeight   // Blending buffer height is the same as destination buffer*/

		rBldImgInfo.u4BufSA1 = prImgReszInst->tImgReszBldBufInfo.u4YBufAddr;
		rBldImgInfo.u4BufSA2 = prImgReszInst->tImgReszBldBufInfo.u4CbBufAddr;
		/*rBldImgInfo.u4BufSA3 = prImgReszInst->tImgReszBldBufInfo.u4CrBufAddr;*/
		rBldImgInfo.u1Alpha = prImgReszInst->tImgReszBldBufInfo.u1Alpha;
		i4ImgResz_HAL_Set_Blending_Image_Info(u4HwId, &rBldImgInfo);
		/* Register notification callback function*/
		rNofifyCallback.pvCallBackFunc = ImgReszHwInstNotifyCallback;
		rNofifyCallback.pvPrivData = (void *)u4HwId;
		i4ImgResz_HAL_Reg_Notify_Callback(u4HwId, &rNofifyCallback);

		if (prImgReszInst->tImgReszRmInfo.fgRPRMode) {
			IMGRESZ_HAL_RM_INFO_T rRMInfo;

			x_memset(&rRMInfo, 0, sizeof(IMGRESZ_HAL_RM_INFO_T));

			rRMInfo.fgRPRMode = prImgReszInst->tImgReszRmInfo.fgRPRMode;
			rRMInfo.fgRPRRacingModeEnable = prImgReszInst->tImgReszRmInfo.fgRacingMode;

			i4ImgResz_HAL_Set_RM_Info(u4HwId, &rRMInfo);
		}

		/* Set Jpeg info*/
		if (prImgReszInst->tImgReszSrcBufInfo.eSrcColorMode == IMGRESZ_DRV_INPUT_COL_MD_JPG_DEF) {
			IMGRESZ_HAL_JPEG_INFO_T rJpegInfo;

			/*x_memset(&rJpegInfo,0,sizeof(IMGRESZ_HAL_JPEG_INFO_T));*/

			rJpegInfo.fgPictureMode = (prImgReszInst->eImgReszScaleMd == IMGRESZ_DRV_JPEG_PIC_SCALE);
			rJpegInfo.fgPreloadMode = prImgReszInst->tImgReszJpegInfo.fgPreload;
			rJpegInfo.fgYExist = prImgReszInst->tImgReszJpegInfo.fgExistY;
			rJpegInfo.fgCbExist = prImgReszInst->tImgReszJpegInfo.fgExistCb;
			rJpegInfo.fgCrExist = prImgReszInst->tImgReszJpegInfo.fgExistCr;
			i4ImgResz_HAL_Set_Jpeg_Info(u4HwId, &rJpegInfo);
		}
	}

	if (prImgReszInst->eImgReszScaleMd == IMGRESZ_DRV_JPEG_PIC_SCALE) { /* Jpeg picture mode*/
		IMGRESZ_HAL_PARTIAL_BUF_INFO_T rSrcRowBufInfo = { 0 };
       /*added by mtk68119 to set the value of TempLineBufSa*/
		if(prImgReszInst->tImgReszDstBufInfo.u4BufWidth * 3 > TempLine_Reserved.size){

			VERIFY(0);
	}
		prImgReszInst->u4TempLineBufSa = (unsigned long)TempLine_Reserved.base;
		VERIFY(prImgReszInst->u4TempLineBufSa != 0);
		IMGR_LOG(IMGR_LOG_LVL_DBG," prImgReszInst->u4TempLineBufSa is %x,size is %x\n",prImgReszInst->u4TempLineBufSa,
			TempLine_Reserved.size);
	  /*added by mtk68119  end*/
		rSrcRowBufInfo.u4RowBufHeight = prImgReszInst->tImgReszPartialBufInfo.u4YBufLine;

		rSrcRowBufInfo.fgFirstRowBuf = prImgReszInst->tImgReszPartialBufInfo.fgFirstRow;
		rSrcRowBufInfo.fgLastRowBuf = prImgReszInst->tImgReszPartialBufInfo.fgLastRow;

		rSrcRowBufInfo.u4CurRowBufSA1 = prImgReszInst->tImgReszSrcBufInfo.u4YBufAddr;
		rSrcRowBufInfo.u4CurRowBufSA2 = prImgReszInst->tImgReszSrcBufInfo.u4CbBufAddr;
		rSrcRowBufInfo.u4CurRowBufSA3 = prImgReszInst->tImgReszSrcBufInfo.u4CrBufAddr;

		rSrcRowBufInfo.u4PrevRowBufSA1 = prImgReszInst->tImgReszPartialBufInfo.u4YBufAddr;
		rSrcRowBufInfo.u4PrevRowBufSA2 = prImgReszInst->tImgReszPartialBufInfo.u4CbBufAddr;
		rSrcRowBufInfo.u4PrevRowBufSA3 = prImgReszInst->tImgReszPartialBufInfo.u4CrBufAddr;

		i4ImgResz_HAL_Set_Partial_Mode_Info(u4HwId, &rSrcRowBufInfo, prImgReszInst->u4TempLineBufSa);

		/*vHwImgReszWaitCountReset(u4HwId);*/

		i4ImgResz_HAL_Resize(u4HwId);

		/* Notify Jpeg driver that image resizer is ready for picture mode.*/
		IMGR_LOG(IMGR_LOG_LVL_DBG," send jpeg mode ready\n");
		vImgResz_Inst_NotifyCallback(&(_arImgreszHwInst[u4HwId]), S_IMGRESZ_DRV_RESIZE_PIC_MODE_READY);
	} else if (!fgPartialMode) { /* Frame mode*/
		if (fgInterlaced) { /* Interlaced frame*/
			rSrcImgInfo.rBufferFormat.fgProgressiveFrame = FALSE;
			rSrcImgInfo.u4ImgHeight /= 2;
			rDestImgInfo.u4ImgHeight /= 2;
			i4ImgResz_HAL_Set_Destination_Image_Info(u4HwId, &rDestImgInfo);

			/* Field resize*/
			rSrcImgInfo.rBufferFormat.fgTopField = _arImgreszHwInst[u4HwId].fgCurrTopField;
			i4ImgResz_HAL_Set_Source_Image_Info(u4HwId, &rSrcImgInfo);

			/*vHwImgReszWaitCountReset(u4HwId);*/

			i4ImgResz_HAL_Resize(u4HwId);
		} else { /* Progressive frame*/

			/*vHwImgReszWaitCountReset(u4HwId);*/

			i4ImgResz_HAL_Resize(u4HwId);

			if (prImgReszInst->tImgReszRmInfo.fgRPRMode && prImgReszInst->tImgReszRmInfo.fgRacingMode) {
				vImgResz_Inst_NotifyCallback(&(_arImgreszHwInst[u4HwId]),
							     S_IMGRESZ_DRV_RESIZE_PIC_MODE_READY);
			}

		}
	} else { /* Partial mode*/
		IMGRESZ_HAL_PARTIAL_BUF_INFO_T rSrcRowBufInfo = { 0 };

		if (prImgReszInst->tImgReszPartialBufInfo.fgFirstRow) {
			/*added by mtk68119 to set the value of TempLineBufSa*/
			if(prImgReszInst->tImgReszDstBufInfo.u4BufWidth * 3>TempLine_Reserved.size) {
			   VERIFY(0);
			}
			prImgReszInst->u4TempLineBufSa = (unsigned long)TempLine_Reserved.base;
			VERIFY(prImgReszInst->u4TempLineBufSa != 0);
		}
		/*added by mtk68119 end*/

		rSrcRowBufInfo.u4RowBufHeight = prImgReszInst->tImgReszPartialBufInfo.u4YBufLine;

		rSrcRowBufInfo.fgFirstRowBuf = prImgReszInst->tImgReszPartialBufInfo.fgFirstRow;
		rSrcRowBufInfo.fgLastRowBuf = prImgReszInst->tImgReszPartialBufInfo.fgLastRow;

		rSrcRowBufInfo.u4PrevRowBufSA1 = prImgReszInst->u4PrevRowBufSa1;
		rSrcRowBufInfo.u4PrevRowBufSA2 = prImgReszInst->u4PrevRowBufSa2;
		rSrcRowBufInfo.u4PrevRowBufSA3 = prImgReszInst->u4PrevRowBufSa3;

		rSrcRowBufInfo.u4CurRowBufSA1 = prImgReszInst->tImgReszPartialBufInfo.u4YBufAddr;
		rSrcRowBufInfo.u4CurRowBufSA2 = prImgReszInst->tImgReszPartialBufInfo.u4CbBufAddr;
		rSrcRowBufInfo.u4CurRowBufSA3 = prImgReszInst->tImgReszPartialBufInfo.u4CrBufAddr;

		i4ImgResz_HAL_Set_Partial_Mode_Info(u4HwId, &rSrcRowBufInfo, prImgReszInst->u4TempLineBufSa);
#if IMGRESZ_SUPPORT_RESET_DEST_BUFFER

		i4ImgResz_HAL_Set_DstBuf_Reset(u4HwId, prImgReszInst->tImgReszDstBufInfo.fgResetSA);

		if (prImgReszInst->tImgReszDstBufInfo.fgResetSA) {
			/* reset desination image info*/

			x_memset(&rDestImgInfo, 0, sizeof(IMGRESZ_HAL_IMG_INFO_T));
			rDestImgInfo.rBufferFormat.fgBlockMode = FALSE;
			rDestImgInfo.rBufferFormat.fgAddrSwap = FALSE;

			switch (prImgReszInst->tImgReszDstBufInfo.eDstColorMode) {
			case IMGRESZ_DRV_OUTPUT_COL_MD_ARGB_8888:
				rDestImgInfo.rBufferFormat.eBufferMainFormat =
					IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER;
				rDestImgInfo.rBufferFormat.eARGBBufferFormat =
					IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888;
				break;

			case IMGRESZ_DRV_OUTPUT_COL_MD_ARGB_4444:
				rDestImgInfo.rBufferFormat.eBufferMainFormat =
					IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER;
				rDestImgInfo.rBufferFormat.eARGBBufferFormat =
					IMGRESZ_HAL_ARGB_BUFFER_FORMAT_4444;
				break;

			case IMGRESZ_DRV_OUTPUT_COL_MD_AYUV:
				rDestImgInfo.rBufferFormat.eBufferMainFormat =
					IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER;
				rDestImgInfo.rBufferFormat.eARGBBufferFormat =
					IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888;
				break;

			case IMGRESZ_DRV_OUTPUT_COL_MD_420_RS:
				rDestImgInfo.rBufferFormat.eBufferMainFormat =
					IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
				rDestImgInfo.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
				break;

			case IMGRESZ_DRV_OUTPUT_COL_MD_420_BLK:
				rDestImgInfo.rBufferFormat.eBufferMainFormat =
					IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
				rDestImgInfo.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
				rDestImgInfo.rBufferFormat.fgBlockMode = TRUE;
				rDestImgInfo.rBufferFormat.fgAddrSwap = TRUE;
				break;

			default:
				break;
			}

			rDestImgInfo.rBufferFormat.fgProgressiveFrame = TRUE;

			rDestImgInfo.u4BufWidth = prImgReszInst->tImgReszDstBufInfo.u4BufWidth;
			rDestImgInfo.u4BufHeight = prImgReszInst->tImgReszDstBufInfo.u4BufHeight;
			rDestImgInfo.u4ImgWidth = prImgReszInst->tImgReszDstBufInfo.u4PicWidth;

			if (rDestImgInfo.u4ImgWidth == 1) {
				rDestImgInfo.u4ImgWidth = 2;
			}


			rDestImgInfo.u4ImgHeight = prImgReszInst->tImgReszDstBufInfo.u4PicHeight;

			rDestImgInfo.u4ImgXOff = prImgReszInst->tImgReszDstBufInfo.u4PicPosX;
			rDestImgInfo.u4ImgYOff = prImgReszInst->tImgReszDstBufInfo.u4PicPosY;
			rDestImgInfo.u4BufSA1 = prImgReszInst->tImgReszDstBufInfo.u4YBufAddr;
			rDestImgInfo.u4BufSA2 = prImgReszInst->tImgReszDstBufInfo.u4CBufAddr;

			rDestImgInfo.rBufferFormat.fgWT = prImgReszInst->tImgReszDstBufInfo.fgWTEnable;

			i4ImgResz_HAL_Set_Destination_Image_Info(u4HwId, &rDestImgInfo);

		}

#endif

		/*vHwImgReszWaitCountReset(u4HwId);*/

		i4ImgResz_HAL_Resize(u4HwId);

		prImgReszInst->u4PrevRowBufSa1 = rSrcRowBufInfo.u4CurRowBufSA1;
		prImgReszInst->u4PrevRowBufSa2 = rSrcRowBufInfo.u4CurRowBufSA2;
		prImgReszInst->u4PrevRowBufSa3 = rSrcRowBufInfo.u4CurRowBufSA3;
	}

}


void vImgReszHwInstStopScale(IMGRESZ_HW_INST_T *ptImgReszHwInst)
{
	VERIFY(!ptImgReszHwInst->fgWaitLock);

	if (ptImgReszHwInst->fgImgReszActive) {
		/*notify stop callback*/
		/*i4ImgResz_Inst_NotifyCallbackProc(ptImgReszHwInst->u2ImgReszCompId,S_IMGRESZ_DRV_RESIZE_STOP);*/

		vImgResz_Inst_ReleaseServicedInstanceAndNotifyCallback(ptImgReszHwInst,
								       FALSE, S_IMGRESZ_DRV_RESIZE_STOP);
	} else {
		ASSERT(0);
	}
}


void vImgReszHwInstTimeout(IMGRESZ_HW_INST_T *ptImgReszHwInst)
{
	if (ptImgReszHwInst->fgImgReszActive) {
		/*stop HAL*/
		i4ImgResz_HAL_Uninit(ptImgReszHwInst->u2ImgReszHwId);

		/*notify stop callback*/
		/*i4ImgResz_Inst_NotifyCallbackProc(ptImgReszHwInst->u2ImgReszCompId,E_IMGRESZ_DRV_RESIZE_TIMEOUT);*/

		vImgResz_Inst_ReleaseServicedInstanceAndNotifyCallback(ptImgReszHwInst,
								       FALSE, E_IMGRESZ_DRV_RESIZE_TIMEOUT);
	} else {
		ASSERT(0);
	}
}


void vImgReszHwInstScaleNotify(bool fgForceHwId, u32 *pu4HwId)
{
	s32 i4, i4Ret;

	vImgReszInstLog(0x11, fgForceHwId);

	/*udelay(10);*/
	if (fgForceHwId) {
		vImgReszInstLog(0x12, *pu4HwId);
		/*sent a event to related HW instance*/
		i4Ret = x_ev_group_set_event(_arImgreszHwInst[*pu4HwId].hEventHandle, IMGRESZ_EV_DO_SCALE, X_EV_OP_OR);
		VERIFY(i4Ret == OSR_OK);
	} else {
		for (i4 = 0; i4 < IMGRESZ_HW_INST_NUM; i4++) {
			if ((_arImgreszHwInst[i4].fgImgReszActive == FALSE) && !_arImgreszHwInst[i4].fgLock) {
				vImgReszInstLog(0x12, i4);

				/*sent a event to related HW instance*/
				i4Ret = x_ev_group_set_event(_arImgreszHwInst[i4].hEventHandle,
							     IMGRESZ_EV_DO_SCALE, X_EV_OP_OR);
				VERIFY(i4Ret == OSR_OK);
				break;
			}
		}
	}

}


s32 i4ImgReszHwInstLockNotify(u32 u4HwInstId, bool fgLock)
{
	s32 i4Ret;

	if (fgLock) {
		/*sent a event to related HW instance*/
		i4Ret = x_ev_group_set_event(_arImgreszHwInst[u4HwInstId].hEventHandle, IMGRESZ_EV_LOCK, X_EV_OP_OR);
		VERIFY(i4Ret == OSR_OK);
	} else {
		/*sent a event to related HW instance*/
		i4Ret = x_ev_group_set_event(_arImgreszHwInst[u4HwInstId].hEventHandle, IMGRESZ_EV_UNLOCK, X_EV_OP_OR);
		VERIFY(i4Ret == OSR_OK);
	}

	return 0;
}


s32 i4ImgReszHwInstStopNotify(u32 u4HwInstId)
{
	s32 i4Ret;

	i4Ret = x_ev_group_set_event(_arImgreszHwInst[u4HwInstId].hEventHandle, IMGRESZ_EV_STOP_SCALE, X_EV_OP_OR);
	VERIFY(i4Ret == OSR_OK);

	return S_IMGRESZ_DRV_OK;
}


s32 i4ImgReszHwInstGetNum(void)
{
	return IMGRESZ_HW_INST_NUM;
}


bool fgImgReszHwInstIsActive(u32 u4HwInstId)
{
	return _arImgreszHwInst[u4HwInstId].fgImgReszActive;
}


int vImgReszHwInstMain(void *arg)
{
	s32               i4Ret = 1;
	u8              u4ThreadId = *(u8 *)arg;

	IMGRESZ_HW_INST_T  *ptImgReszHwInst = NULL;
	EV_GRP_EVENT_T      eImgReszEvent;
	EV_GRP_EVENT_T      eImgReszEventGet;

	if (u4ThreadId > (IMGRESZ_HW_INST_NUM - 1)) {
		ASSERT(0);
	}

	ptImgReszHwInst = &(_arImgreszHwInst[u4ThreadId]);
	ptImgReszHwInst->u2ImgReszHwId = u4ThreadId;

	while (TRUE) {
		if (!ptImgReszHwInst->fgImgReszActive) {
			eImgReszEvent = IMGRESZ_EV_FINISH |
					IMGRESZ_EV_DO_SCALE | IMGRESZ_EV_LOCK | IMGRESZ_EV_UNLOCK |
					IMGRESZ_EV_FINISH_SCALE;
			i4Ret = x_ev_group_wait_event(ptImgReszHwInst->hEventHandle, eImgReszEvent,
						      &eImgReszEventGet, X_EV_OP_OR_CONSUME);
			VERIFY(i4Ret == OSR_OK);

			/* Last stop cause un-serviced finish-scale event.*/
			/* Skip finish scale event.*/
			eImgReszEvent &= ~IMGRESZ_EV_FINISH_SCALE;
		} else {
			u32 u4TimeOut;
			IMGRESZ_INST_T *prImgreszInst;

			i4ImgResz_Inst_GetInstanceObject(ptImgReszHwInst->u2ImgReszCompId, &prImgreszInst);

			if ((prImgreszInst->eImgReszScaleMd == IMGRESZ_DRV_JPEG_PIC_SCALE)
			    || (prImgreszInst->tImgReszRmInfo.fgRPRMode &&
				prImgreszInst->tImgReszRmInfo.fgRacingMode)) {
				u4TimeOut = 0xFFFFFFFF;
			} else {
#ifdef DEBUG
				u4TimeOut = 0xFFFFFFFF;
#else
				u4TimeOut = 1000;
#endif
			}

			eImgReszEvent = IMGRESZ_EV_FINISH |
					IMGRESZ_EV_DO_SCALE | IMGRESZ_EV_STOP_SCALE | IMGRESZ_EV_FINISH_SCALE |
					IMGRESZ_EV_LOCK | IMGRESZ_EV_UNLOCK;
			i4Ret = x_ev_group_wait_event_timeout(ptImgReszHwInst->hEventHandle, eImgReszEvent,
							      &eImgReszEventGet, X_EV_OP_OR_CONSUME, u4TimeOut);

			if (i4Ret == OSR_TIMEOUT) {
				vImgReszHwInstTimeout(ptImgReszHwInst);
				continue;
			} else {
				VERIFY(i4Ret == OSR_OK);
			}
		}

		/* Terminate thread*/
		if (eImgReszEventGet & IMGRESZ_EV_FINISH) {
			ptImgReszHwInst->fgWaitThreadFinish = TRUE;
		}

		/* State machine for lock and unlock*/
		if ((eImgReszEventGet & IMGRESZ_EV_LOCK) || (eImgReszEventGet & IMGRESZ_EV_UNLOCK)) {
			bool fgLock;
			u32 u4InstId;

			i4Ret = i4ImgResz_Inst_GetLockInstance(ptImgReszHwInst, &fgLock, &u4InstId);
			VERIFY(i4Ret >= 0);

			if (ptImgReszHwInst->fgImgReszActive) {
				if (!ptImgReszHwInst->fgLock && fgLock) {
					ptImgReszHwInst->fgWaitLock = TRUE;
				} else if (ptImgReszHwInst->fgLock && !fgLock) {
					VERIFY(FALSE);
				}
			}

			/* When lock to unlock, trigger itself to search other unserviced instance.*/
			if (ptImgReszHwInst->fgLock && !fgLock) {
				eImgReszEventGet |= IMGRESZ_EV_DO_SCALE;
			}

			if (!ptImgReszHwInst->fgWaitLock) {
				ptImgReszHwInst->fgLock = fgLock;
				ptImgReszHwInst->u2ImgReszCompId = u4InstId;
			} else {
				ptImgReszHwInst->u4WaitLockInstId = u4InstId;
			}
		}

		/*check component state, and then do scale or stop*/
		if (ptImgReszHwInst->fgImgReszActive) {
			IMGRESZ_INST_T *prImgreszInst;

			i4ImgResz_Inst_GetInstanceObject(ptImgReszHwInst->u2ImgReszCompId, &prImgreszInst);

			if (eImgReszEventGet & IMGRESZ_EV_STOP_SCALE) {
				IMGRESZ_INST_T *prImgreszInst_inter;

				/*stop HAL*/
				i4ImgResz_HAL_Uninit(ptImgReszHwInst->u2ImgReszHwId);

				i4ImgResz_Inst_GetInstanceObject(ptImgReszHwInst->u2ImgReszCompId,
								 &prImgreszInst_inter);
				
				/*stop scale*/
				vImgReszHwInstStopScale(ptImgReszHwInst);
			} else if (eImgReszEventGet & IMGRESZ_EV_FINISH_SCALE) {
				IMGRESZ_INST_T *prImgreszInst_inter;

				i4ImgResz_Inst_GetInstanceObject(
					ptImgReszHwInst->u2ImgReszCompId, &prImgreszInst_inter);

				vHwImgReszWaitCountReset(ptImgReszHwInst->u2ImgReszCompId);

				/* State machine for multi-resize (interlaced video)*/
				if (ptImgReszHwInst->fgInterlaced) {
					IMGRESZ_INST_T *prImgreszInst_inter2;

					i4ImgResz_Inst_GetInstanceObject(
						ptImgReszHwInst->u2ImgReszCompId, &prImgreszInst_inter2);

					if (ptImgReszHwInst->fgCurrTopField &&
					    prImgreszInst_inter2->tImgReszSrcBufInfo.fgBottomField) {
						/* Keep doing scale bottom field*/
						ptImgReszHwInst->fgCurrTopField = FALSE;
						vImgReszHwInstResz(
							(u32)(ptImgReszHwInst->u2ImgReszHwId), prImgreszInst_inter2);
						continue;
					}
				}

				if (!(prImgreszInst_inter->eImgReszScaleMd == IMGRESZ_DRV_PARTIAL_SCALE) ||
				    prImgreszInst_inter->tImgReszPartialBufInfo.fgLastRow) {
					i4ImgResz_HAL_Uninit(u4ThreadId);
				}

				/*notify finish callback*/
				/*i4ImgResz_Inst_NotifyCallbackProc(
					ptImgReszHwInst->u2ImgReszCompId,S_IMGRESZ_DRV_RESIZE_FINISH);*/

				vImgResz_Inst_ReleaseServicedInstanceAndNotifyCallback(ptImgReszHwInst,
					TRUE, S_IMGRESZ_DRV_RESIZE_FINISH);

				if (ptImgReszHwInst->fgWaitLock) {
					ptImgReszHwInst->fgWaitLock = FALSE;
					ptImgReszHwInst->fgLock = TRUE;
					ptImgReszHwInst->u2ImgReszCompId = ptImgReszHwInst->u4WaitLockInstId;
					i4Ret = x_ev_group_set_event(ptImgReszHwInst->hEventHandle,
						IMGRESZ_EV_DO_SCALE, X_EV_OP_OR);
					VERIFY(i4Ret == OSR_OK);
				}
			}
		} else {
			/* Terminate thread*/
			if (ptImgReszHwInst->fgWaitThreadFinish) {
				ptImgReszHwInst->fgWaitThreadFinish = FALSE;
				break;
			}

			if (eImgReszEventGet & IMGRESZ_EV_DO_SCALE) {
				IMGRESZ_INST_T *prImgreszInst_inter;

				i4Ret = i4ImgResz_Inst_GetUnservicedInstance(ptImgReszHwInst);

				/*start do scale*/
				if (i4Ret >= 0) {
					i4ImgResz_Inst_GetInstanceObject(
						ptImgReszHwInst->u2ImgReszCompId, &prImgreszInst_inter);
					/* Interlaced mode preprocess*/

					ptImgReszHwInst->fgInterlaced =
						prImgreszInst_inter->tImgReszSrcBufInfo.fgInterlaced;

					if (prImgreszInst_inter->tImgReszSrcBufInfo.fgInterlaced) {
						if (prImgreszInst_inter->tImgReszSrcBufInfo.fgTopField) {
							ptImgReszHwInst->fgCurrTopField = TRUE;
						} else {
							ptImgReszHwInst->fgCurrTopField = FALSE;
						}
					}

					if (prImgreszInst_inter->eImgReszScaleMd == IMGRESZ_DRV_PARTIAL_SCALE) {
						if (prImgreszInst_inter->tImgReszPartialBufInfo.fgFirstRow) {
							prImgreszInst_inter->u4PartialRowCnt = 0;
						} else {
							prImgreszInst_inter->u4PartialRowCnt++;
						}
					}

					vImgReszHwInstResz(
						(u32)(ptImgReszHwInst->u2ImgReszHwId), prImgreszInst_inter);
				}
			}
		}
	}

	ptImgReszHwInst->fgThreadFinish = TRUE;
	complete_and_exit(NULL, 0);

	return 0;
}

#if 0/*def FPGA*/
u32 *g_pu4RezReset = (u32 *)0XA00000A0;
u32 *g_pu4RezClkEn = (u32 *)0XA00000BC;
u32 *g_pu4RezLogEn = (u32 *)0XA000006C;
#endif

s32 i4_thread_id_0 = 0;
s32 i4_thread_id_1 = 1;
s32 i4ImgResz_Drv_Init(void)
{
	s32   i4Ret = 1;
	s32   i4;
	int sched_priority;

	if (_fgImgreszInit) {
		return 0;
	}

#if 0/*def FPGA*/
	/*RESET AND ENABLE CLOCK*/
	*g_pu4RezReset = 0X1FF;
	*g_pu4RezClkEn = 0X1FF;
	*g_pu4RezLogEn = 0X0;
#endif
	/* HAL boot init*/
	i4ImgResz_HAL_Boot_Init();

	/*zero instance data*/
	vImgreszSetMemory((void *)_arImgreszHwInst, 0, sizeof(_arImgreszHwInst));


	for (i4 = 0; i4 < IMGRESZ_HW_INST_NUM; i4++) {
		_arImgreszHwInst[i4].u2ImgReszCompId = IMGRESZ_INVALID16;
		_arImgreszHwInst[i4].fgImgReszActive = FALSE;
	}

	/*create event*/
	i4Ret = x_ev_group_create(&_arImgreszHwInst[0].hEventHandle, "ImgReszEv0", IMGRESZ_EV_INITIAL);
	VERIFY(i4Ret == OSR_OK);
#if DOUBLE_REG
	i4Ret = x_ev_group_create(&_arImgreszHwInst[1].hEventHandle, "ImgReszEv1", IMGRESZ_EV_INITIAL);
	VERIFY(i4Ret == OSR_OK);
#endif
	/* Instance init*/
	vImgResz_Inst_Init();

	/*create imgresz thread*/
	sched_priority = 100 - (int)(IMGRESZ_THREAD_PRIORITY * 100) / 256;
	h_imgresz_thread_0 = kthread_create(vImgReszHwInstMain, (void *)&i4_thread_id_0, IMGRESZ0_THREAD_NAME);
	if (IS_ERR(h_imgresz_thread_0)) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "imgresz0 create thread fail !\r\n");
	} else {
		struct sched_param param;
		int ret;

		param.sched_priority = sched_priority;
		//ret = sched_setscheduler_nocheck(h_imgresz_thread_0, SCHED_RR, &param);
		//ASSERT(ret == 0);
	}
	wake_up_process(h_imgresz_thread_0);

#if DOUBLE_REG
	h_imgresz_thread_1 = kthread_create(vImgReszHwInstMain, (void *)&i4_thread_id_1, IMGRESZ1_THREAD_NAME);
	if (IS_ERR(h_imgresz_thread_1)) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "imgresz1 create thread fail !\r\n");
	} else {
		struct sched_param param;
		int ret;

		param.sched_priority = sched_priority;
		//ret = sched_setscheduler_nocheck(h_imgresz_thread_1, SCHED_RR, &param);
		//ASSERT(ret == 0);
	}
	wake_up_process(h_imgresz_thread_1);
#endif

#if CONFIG_SUSPEND_TO_DRAM
	register_pm_ops(&imgresz_pm_ops);
#endif

	mutex_init(&g_ImgReszMutex);
	spin_lock_init(&filplock);
	
	NeedToStop=false;
	StopCanReturn=false;
	CurrFilp=NULL;
	Dst=NULL;
	memset(&SrcBuf,0,sizeof(IMGRESZ_HAL_IMG_INFO_T));
	memset(&DstBuf,0,sizeof(IMGRESZ_HAL_IMG_INFO_T));
	_fgImgreszInit = TRUE;
	/*MOD_VERSION_INFO(MMISC_MODE_NAME, MMISC_VER_MAJOR, MMISC_VER_MINOR, MMISC_VER_REV);*/
	return i4Ret;
}
EXPORT_SYMBOL(i4ImgResz_Drv_Init);


s32 i4ImgResz_DrvUninit(u32 u4Case)
{
	s32   i4Ret;

	if (!_fgImgreszInit) {
		return 0;
	}

	/* Destroy thread*/
	i4Ret = x_ev_group_set_event(_arImgreszHwInst[0].hEventHandle, IMGRESZ_EV_FINISH, X_EV_OP_OR);
	VERIFY(i4Ret == OSR_OK);
#if DOUBLE_REG
	i4Ret = x_ev_group_set_event(_arImgreszHwInst[1].hEventHandle, IMGRESZ_EV_FINISH, X_EV_OP_OR);
	VERIFY(i4Ret == OSR_OK);
#endif

	/* Wait thread finish*/
	while (!_arImgreszHwInst[0].fgThreadFinish) {
		mdelay(1);
	}

#if DOUBLE_REG

	while (!_arImgreszHwInst[1].fgThreadFinish) {
		mdelay(1);
	}

#endif

	/* Instance uninit*/
	vImgResz_Inst_Uninit();


	/*Delete event*/
	i4Ret = x_ev_group_delete(_arImgreszHwInst[0].hEventHandle);
	VERIFY(i4Ret == OSR_OK);
#if DOUBLE_REG
	i4Ret = x_ev_group_delete(_arImgreszHwInst[1].hEventHandle);
	VERIFY(i4Ret == OSR_OK);
#endif
	/* HAL boot uninit*/
	i4ImgResz_HAL_Boot_Uninit();

	_fgImgreszInit = FALSE;
#if CONFIG_SUSPEND_TO_DRAM
	unregister_pm_ops(&imgresz_pm_ops);
#endif

	return i4Ret;
}

s32 i4ImgResz_Drv_Uninit(void)
{
	i4ImgResz_DrvUninit(0);
	return 0;
}
EXPORT_SYMBOL(i4ImgResz_Drv_Uninit);










