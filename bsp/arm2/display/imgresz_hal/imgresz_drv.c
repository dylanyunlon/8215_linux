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
#include "imgresz_drv.h"
#include "imgresz_hal_if.h"
#include "imgresz_hal.h"
#include "drv_imgresz_errcode.h"
#include "drv_thread.h"
#include "drv_config.h"
#include "x_hal_1176.h"
#include "sys_config.h"

#include "media/atc/display.h"
#include "imgresz_log.h"
#include "display.h"
#include "vdp.h"


#define DOUBLE_REG 1

/*-----------------------------------------------------------------------------
data declarations
----------------------------------------------------------------------------*/



IMGRESZ_HW_INST_T   _arImgreszHwInst[IMGRESZ_HW_INST_NUM] = { { 0 }, { 0 } };
IMGRESZ_HAL_RESAMPLE_METHOD_T _eHResampleMethod = IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR;
IMGRESZ_HAL_RESAMPLE_METHOD_T _eVResampleMethod = IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR;


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



#define IMGRESZ_BUF_ALIGN_MASK(value, mask) ((((value) + ((mask) - 1)) / (mask)) * (mask))
#define IMGRESZ_ALIGN 4096
extern BOOL fgIRTDone;
extern BOOL fgVDPNeedInit;
extern VDP_PARAM rData[3];

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


s32 TS_DirectScale(u32 u4HwId, BOOL fg8Tap, VOID   *prSrcImgInfo,
		     VOID  *prDestImgInfo)
{
	s32 i4Ret = 0;
	/*u32 u4SrcBuf = 0;*/
	/*u32 u4DestBuf = 0;*/

	IMGRESZ_HAL_IMG_INFO_T *prSrc, *prDst;
	 VDP_PARAM * prParam = &rData[0];

	 if(!fgIRTDone)
	{
	    IMGR_LOG(IMGR_LOG_LVL_DBG,"irtdma not done\n");
	    return FALSE;
	}
	else
	{
	    fgIRTDone = FALSE;
        //Printf("TS_DirectScale:Irt  done fgVDPNeedInit:%d!\r\n", fgVDPNeedInit);
        if(fgVDPNeedInit)
        {
            IMGR_LOG(IMGR_LOG_LVL_DBG,"Remove global prParam\n");
            fgVDPNeedInit = FALSE;
            prParam->u4Flags |= VDP_UPDATE_OVERLAY;
        }
	}

	prSrc = (IMGRESZ_HAL_IMG_INFO_T *)prSrcImgInfo;
	prDst = (IMGRESZ_HAL_IMG_INFO_T *)prDestImgInfo;

	

	
	IMGR_LOG(IMGR_LOG_LVL_DBG, "%d,%d,%d,%d,%d,%d\r\n ", prSrc->u4ImgWidth, prSrc->u4ImgHeight, prDst->u4ImgWidth,prDst->u4ImgHeight,sizeof(IMGRESZ_HAL_IMG_INFO_T),sizeof(IMGRESZ_HAL_IMG_INFO_T));

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
		//IMGR_LOG(IMGR_LOG_LVL_ERR,"6666\n");

		/*Sleep(1);*/
	}

	i4ImgResz_HAL_Uninit(u4HwId);
	return TRUE;
}
EXPORT_SYMBOL(TS_DirectScale);

