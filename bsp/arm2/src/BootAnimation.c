/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */
#include "x_types.h"
#include "x_bim_83xx.h"
#include "dual_hal.h"
#include "dual_task.h"
#include "dual_callback.h"
//#include "arm2_comm_data_struct.h"
//#include <linux/types.h>
//#include <linux/fs.h>
//#include <linux/sched.h>

//#include "x_typedef.h"
//#include "windev.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "printf.h"
#include "backcar_cfg.h"
#include "mrf.h"
#include "vdec_drv_slt_h264.h"
#include "arm2system_service.h"
#include <generated/atc_project.h>
#define TAGS "ANI"
#define ANI_INFO pr_info
#define ANI_DBG pr_debug
#define TASK_IDLE    (0x1)
#define TASK_BUSY    (0x2)

#define ANIMATION_HEIGHT     106
#define ANIMATION_WIDTH      462
#define ANIMATION_START_NUM  0

#define ANIMATION_TASK_PRIORITY (3)
#define ANIMATION_TASK_STACK_SIZE (1024)

static int sg_VBAPlayFlag = 1;
static int sg_displayInitFlag = 0;
static unsigned int picnum;
static unsigned int poswidth;
static unsigned int posheight;
static unsigned int flashtime;
static BOOL Last_Source_Ready = FALSE;
static BOOL ANIMATION_STOP = FALSE;
int DIS_WIDTH1 = 1024;
int DIS_HEIGHT1 = 600;
extern void  DisplayLighten(void);
#ifdef CONFIG_ATC_PLATFORM_ac823x
extern unsigned long fbm_base;
#else
extern unsigned int fbm_base;
#endif
extern _u4LCDWidth;
extern _u4LCDHeight;
//extern char vba_data[];
//extern char vba_data_end[];
extern RSV_MEM_T *mm_reserved;
UINT32 vdecIRQIndex = 0;
UINT32 vdecIRQComplete = 1;
enum VBA_PLAY_STATUS {
	VBA_PLAY_STATUS_NONE,
	VBA_PLAY_BOOT_ANIMATION,
	VBA_PLAY_SHUTDOWN_ANIMATION,
	VBA_EXIT_SHUTDOWN_ANIMATION,
};

enum SYSTEM_STATUS {
	SYSTEM_STATUS_NONE,
	SYSTEM_STATUS_HOT,
	SYSTEM_STATUS_COLD,
};

typedef struct tagVBAInfo
{
	UINT32 totalResSize;
	UINT32 hasWhichAni;
	UINT32 bootAniResSize;
	UINT32 shutdownAniResSize;
	UINT32 width;
	UINT32 height;
	UINT32 fps;
}VBAINFO_T;

static enum VBA_PLAY_STATUS PLAY_WHICH_VBA = VBA_PLAY_BOOT_ANIMATION;

static UINT32 renderIndex = 0;
static UINT32 decodeIndex = 0;
static UINT32 aniStartPos = 0;
static UINT32 aniEndPos= 0;
static UINT32 aniBootStartPos = 0;
static UINT32 aniBootEndPos= 0;
static UINT32 aniShutDownStartPos = 0;
static UINT32 aniShutDownEndPos= 0;
static UINT32 index = 0;
static UINT32 pos = 0;
static BOOL aniNeedUpdate = 0;
static BOOL aniEos = 0;
static BOOL aniStop = 0;
static BOOL aniPlayToEndExit = 0;
static BOOL aniRegisterCallback = 0;
static BOOL vdechwIsReady = 0;
static BOOL vdechwIsRequest = 0;
static BOOL vdecIrqEnable = 0;
static SemaphoreHandle_t xVdecSem = NULL;
static SemaphoreHandle_t xAniPlaybackSem = NULL;
static SemaphoreHandle_t xAniStartPlaySem = NULL;
static SemaphoreHandle_t xAniUpdateResSem = NULL;
static SemaphoreHandle_t xAniReadySem = NULL;
static BOOL aniReady = 0;

static UINT32 aniwaitStartPlay = 1;
static BOOL aniStartPlayToEndExit = 1;
static int sg_AbnormalFlag = 0;
static int sg_RebootFlag = 0;
static int sg_CarplayStartFlag = 0;
static int sg_EnterAwtkFlag = 0;
static int sg_CarplayStatusUpdateFlag = 0;
uint8_t sgNeedOpenAwtkLayer = 1;

void SetEnableAwtkFlag(int enable);

void cleanupParam()
{
	vdecIRQIndex = 0;
	renderIndex = 0;
	decodeIndex = 0;
	index = 0;
}

void vdecISR()
{
	vdecIRQIndex++;
	vdecIRQComplete = 1;
	//Printf("[Arm2] vdecISR vdecIRQIndex %d\n", vdecIRQIndex);
}
extern int arm2system_service_ani_end(void *arg);
void enableVdecIRQ()
{
    if (!vdecIrqEnable) {
        ANI_INFO("[VDEC] enableVdecIRQ\n");
        v_enable_bim_irq(VECTOR_VDFUL);
        vdecIrqEnable = 1;
    }
}

void disableVdecIRQ()
{
    if (vdecIrqEnable) {
        ANI_INFO("[VDEC] disableVdecIRQ\n");
        resetHW();
        v_disable_bim_irq(VECTOR_VDFUL);
        cleanupParam();
        vdecIrqEnable = 0;
    }
}

void requestVdecHW()
{
	if (vdechwIsReady && !vdechwIsRequest && !GetAbnormalFlag()) {
		ANI_INFO("[VDEC] requestVdecHW start %d\n", GetBootTime());
		HWSendMessage(MODULE_BOOTANIMATION<<24, MESSAGE_ANIMATION_REQUEST_VDEC,0,0);
		if (xSemaphoreTake(xVdecSem, pdMS_TO_TICKS(3000)) == pdFALSE) {
			ANI_INFO("xSemaphoreTake failed\n");
			return;
		}
		ANI_INFO("[VDEC] requestVdecHW end %d\n", GetBootTime());
		vdechwIsRequest = 1;
	}
}

void releaseVdecHW()
{
	if (vdechwIsRequest == 1 && !GetAbnormalFlag()) {
		vdechwIsRequest = 0;
		ANI_INFO("[VDEC] releaseVdecHW %d\n", GetBootTime());
		HWSendMessage(MODULE_BOOTANIMATION<<24, MESSAGE_ANIMATION_VDEC_FREE,0,0);
	}
}

int PlayBootAnimation(void *arg)
{
	ANI_INFO("[CMD] PlayBootAnimation PLAY_WHICH_VBA %d, aniReady %d \n", PLAY_WHICH_VBA, aniReady);
	/*if (PLAY_WHICH_VBA == VBA_PLAY_BOOT_ANIMATION) {
		ANI_INFO("get same message continuous VBA_PLAY_BOOT_ANIMATION\n");
		return 0;
	}*/
	if (aniReady == 0 || aniwaitStartPlay) {
		if (xSemaphoreTake(xAniReadySem, portMAX_DELAY) == pdFALSE)
		{
			ANI_INFO("xAniReadySem failed\n");
		}
		PLAY_WHICH_VBA = VBA_PLAY_BOOT_ANIMATION;
		aniStartPos = aniBootStartPos;
		aniEndPos = aniBootEndPos;
		aniEos = 0;
		aniStop = 0;
		aniNeedUpdate = 1;
		StartPlayAnimation();
    	xSemaphoreGive(xAniPlaybackSem);
		return 0;
	}
	if (xSemaphoreTake(xAniUpdateResSem, portMAX_DELAY) == pdFALSE)
	{
		ANI_INFO("xAniUpdateResSem failed\n");
	}
	PLAY_WHICH_VBA = VBA_PLAY_BOOT_ANIMATION;
	aniStartPos = aniBootStartPos;
	aniEndPos = aniBootEndPos;
	aniEos = 0;
	aniStop = 0;
	aniNeedUpdate = 1;
	StartPlayAnimation();
	xSemaphoreGive(xAniPlaybackSem);
	xSemaphoreGive(xAniUpdateResSem);
	return 0;
}

int PlayShutDownAnimation(void *arg)
{
	ANI_INFO("[CMD] PlayShutDownAnimation PLAY_WHICH_VBA %d aniReady %d \n", PLAY_WHICH_VBA, aniReady);
	/*if (PLAY_WHICH_VBA == VBA_PLAY_SHUTDOWN_ANIMATION) {
		ANI_INFO("get same message continuous MESSAGE_ANIMATION_PLAY_SHUTDOWN\n");
		return 0;
	}*/
	if (aniReady == 0 || aniwaitStartPlay) {
		if (xSemaphoreTake(xAniReadySem, portMAX_DELAY) == pdFALSE)
		{
			ANI_INFO("xAniReadySem failed\n");
		}
		PLAY_WHICH_VBA = VBA_PLAY_SHUTDOWN_ANIMATION;
		aniStartPos = aniShutDownStartPos;
		aniEndPos = aniShutDownEndPos;
		aniEos = 0;
		aniStop = 0;
		aniNeedUpdate = 1;
		StartPlayAnimation();
		xSemaphoreGive(xAniPlaybackSem);
		return 0;
	}
	if (xSemaphoreTake(xAniUpdateResSem, portMAX_DELAY) == pdFALSE)
	{
		ANI_INFO("xAniUpdateResSem failed\n");
	}
	PLAY_WHICH_VBA = VBA_PLAY_SHUTDOWN_ANIMATION;
	aniStartPos = aniShutDownStartPos;
	aniEndPos = aniShutDownEndPos;
	aniEos = 0;	
	aniStop = 0;
	aniNeedUpdate = 1;
	StartPlayAnimation();
	xSemaphoreGive(xAniPlaybackSem);
	xSemaphoreGive(xAniUpdateResSem);
	return 0;
}

void StopAnimation(void *arg)
{
	ANI_INFO("[CMD] StopAnimation aniReady %d \n", aniReady);

	if (!aniReady) {
		if (xSemaphoreTake(xAniReadySem, portMAX_DELAY) == pdFALSE)
		{
			ANI_INFO("xAniReadySem failed\n");
		}
		ANI_INFO("[CMD] StopAnimation ani is ready\n");
		aniStartPos = aniBootStartPos;
		aniEndPos = aniBootStartPos;
		aniEos = 0;
		aniStop = 1;
		aniNeedUpdate = 1;
		StartPlayAnimation();
		xSemaphoreGive(xAniPlaybackSem);
	} else {
		aniStartPos = aniBootStartPos;
		aniEndPos = aniBootStartPos;
		aniEos = 0;
		aniStop = 1;
		aniNeedUpdate = 1;
		StartPlayAnimation();
		xSemaphoreGive(xAniPlaybackSem);
	}
}

void StartPlayAnimation()
{
	if (xAniStartPlaySem == NULL) {
		ANI_INFO("[Arm2][VBA]StartPlayback xAniStartPlaySem is NULL\n");
		return;
	}
	if (aniwaitStartPlay) {
	    xSemaphoreGive(xAniStartPlaySem);
	}
	aniwaitStartPlay = 0;
}

BOOL PlayVideoBootAnimationUseIRQAndBuffering()
{
	UINT32 u4VDPDstYPA;
	UINT32 u4VDPDstCPA;
	UINT32 width;
	UINT32 height;
	static UINT32 s_u4PrevTime = 0;
	static BOOL s_fgFirstInit = FALSE;
	UINT32 u4Curtime = 0;
	static char *src = 0;
	static RSV_MEM_T *ani = NULL;
	static UINT32 logo_base = 0;
	static UINT32 filesize = 0;
	struct DecCropParams crop = {0};
	static RECT   SrcRect = {0};
	static UINT32 fps = 33;

	static int timetickDecPred = 6;
	static VIDINFO vidInfo[MAX_OUTPUT] = {0};
	static VBAINFO_T *vbaInfo;

	if (!s_fgFirstInit) {
		s_u4PrevTime = GetARM2TickCount();
		ANI_INFO("[PlayVideoBootAnimation] at %d\n", GetBootTime());
		//ani = get_rsv_mem_by_name("animation");
		//logo_base = (UINT32)(ani->start_addr + ani->size - MRF_LOAD_OFFSET);
		RSV_MEM_T * vba_reserved = get_rsv_mem_by_name("vba");
		mm_reserved = get_rsv_mem_by_name("multimedia");
		logo_base = vba_reserved->start_addr;
		src = ARM1PHY2ARM2UCV(logo_base);
		vbaInfo = (VBAINFO_T *)src;
		filesize = vbaInfo->totalResSize;
		fps = vbaInfo->fps;
		ANI_INFO("[PlayVideoBootAnimation] logo_base 0x%x, 0x%x 0x%x 0x%x 0x%x filesize 0x%x fps 0x%x\n", logo_base, src[0], src[1], src[2], src[3], filesize, fps);
		pos = sizeof(VBAINFO_T);
		fps = ((fps == 0) ? 33 : (1000 / fps));
		aniBootStartPos = pos;
		aniBootEndPos = pos + vbaInfo->bootAniResSize;
		aniShutDownStartPos = aniBootEndPos;
		aniShutDownEndPos = aniBootEndPos + vbaInfo->shutdownAniResSize;
		aniStartPos = aniBootStartPos;
		aniEndPos = aniBootEndPos;
		s_fgFirstInit = TRUE;
		if (vbaInfo->bootAniResSize == 0) {
			DisplayLighten();
		}
		aniReady = 1;
		xSemaphoreGive(xAniReadySem);
	} 

	if (index == 0 && aniwaitStartPlay && xSemaphoreTake(xAniStartPlaySem, portMAX_DELAY) == pdFALSE)
	{
		ANI_INFO("xAniStartPlaySem failed\n");
		return FALSE;
	}
	if (aniNeedUpdate && vdecIRQComplete == 1) {
		pos  = aniStartPos;
		aniNeedUpdate = 0;
		aniEos = 0;
		cleanupParam();
		aniPlayToEndExit = 1;
		ANI_INFO("arm2 start vdec proc %dms, play start at pos 0x%x \n", GetBootTime(), pos);
	} else if (aniNeedUpdate) {
		return TRUE;
	}

	if (pos >= aniEndPos) {
		if (renderIndex == decodeIndex) {
			aniEos = 1;
			disableVdecIRQ();
			releaseVdecHW();
			vTaskDelay(pdMS_TO_TICKS(33));
			return FALSE;
		}
	}
	requestVdecHW();
	enableVdecIRQ();
	u4Curtime = GetARM2TickCount();
	while ((index == 0) ||
			(((renderIndex == decodeIndex) ||
			((abs(u4Curtime - s_u4PrevTime) / fps < 1) && (abs(s_u4PrevTime + fps - u4Curtime) >= timetickDecPred))) &&
			vdecIRQComplete == 1 && (decodeIndex - renderIndex < MAX_OUTPUT - 2) && (pos < aniEndPos))) {

		UINT32 srcSize = (UINT32)((src[pos] &0xFF)
							  |(src[pos+1] &0xFF)<<8
							  |(src[pos+2] &0xFF)<<16
							  |(src[pos+3] &0xFF)<<24);
		pos += 4;
		if (!srcSize || (pos + srcSize > aniEndPos)) {
			ANI_INFO("invalid param  pos 0x%x, size 0x%x aniEndPos 0x%x \n", pos, srcSize, aniEndPos);
			pos = aniEndPos;
			break;
        }
		vdecIRQComplete = 0;

		ANI_DBG("arm2 start vdec proc %dms,[%d] pos 0x%x, size 0x%x \n", GetBootTime(), index, pos, srcSize);
		decode(logo_base, logo_base+filesize, pos, srcSize);
		if (index == 0) {
			waitVdecHwReady(&vidInfo[decodeIndex++ % MAX_OUTPUT]);
		} else {
			noWaitVdecHwReady(&vidInfo[decodeIndex++ % MAX_OUTPUT]);
		}
		//fgVDec_SLT_AVC_Proc(logo_base, logo_base+filesize, pos, srcSize, &u4VDPDstYPA, &u4VDPDstCPA, &width, &height);
		getcrop(&crop);
		SrcRect.left = crop.crop_left_offset;
		SrcRect.top = crop.crop_top_offset;
		SrcRect.right = crop.crop_left_offset + crop.crop_out_width;
		SrcRect.bottom= crop.crop_top_offset + crop.crop_out_height;
		ANI_DBG("left %d, SrcRect.top %d, SrcRect.right %d, SrcRect.bottom %d\n", (int)SrcRect.left, (int)SrcRect.top, (int)SrcRect.right, (int)SrcRect.bottom);

		pos += srcSize;
		ANI_DBG("_u4LCDWidth %d, _u4LCDHeight %d ,decend time %dms\n",  _u4LCDWidth, _u4LCDHeight, GetBootTime());
		if (index == 0) {
			break;
		}
	}
	u4Curtime = GetARM2TickCount();
	if (index != 0 && abs(u4Curtime - s_u4PrevTime) / fps < 1) {
		if ((decodeIndex - renderIndex < MAX_OUTPUT - 2)) {
			return TRUE;
		}		
		vTaskDelay(pdMS_TO_TICKS(fps-abs(u4Curtime - s_u4PrevTime)));
		u4Curtime = GetARM2TickCount();
	}
	if (index == 0 || (renderIndex < vdecIRQIndex)) {
		if (index % 30 == 0 || pos >= aniEndPos) {
		    ANI_INFO("VBAOverlayInit %d index %d renderIndex %d , vdecIRQIndex %d decodeIndex %d \n", GetBootTime(), index, renderIndex , vdecIRQIndex, decodeIndex);
		}
		VIDINFO *pVidInfo = &vidInfo[renderIndex++ % MAX_OUTPUT];
		VBAOverlayInit(pVidInfo->u4VDPDstYPA, pVidInfo->u4VDPDstCPA, pVidInfo->width, pVidInfo->height , &SrcRect);
		AwtkOsdEnable(0);
		sgNeedOpenAwtkLayer = 1;
//		Printf("[Arm2] VBAOverlayInit end %d\n", GetBootTime());
		if (index == 0) {
			DisplayLighten();
			ANI_INFO("DisplayLighten end %d\n", GetBootTime());
		}
		index++;
		s_u4PrevTime = u4Curtime;
	}

	return TRUE;
}

BOOL PlayStaticLogo(HANDLE hMrf)
{
	BOOL fgRet = FALSE;
	BITMAPOBJINFO rBitmapInfo = {0};
	void *pu2FB = NULL;
	void *pu2FrameBuf = NULL; 
#ifdef CONFIG_ATC_OS_linux 
	fgRet = GetBitmapInfo(hMrf, 0, &rBitmapInfo);
	pu2FrameBuf = ARM1PHY2ARM2UCV(logo_base);
	pu2FB = (void *)GetRCObjectMemAddr(hMrf,(RCOBJECT *)&rBitmapInfo);
	memcpy((void *)(pu2FrameBuf), (const void *)pu2FB,rBitmapInfo.u4Height*rBitmapInfo.u4Width*rBitmapInfo.u4BitCount/8);
#else
	BootAnimationOsdInit(logo_base,TRUE);
#endif
	BootAnimationOsdInit(logo_base,FALSE);

	DisplayLighten();
	
    

    return fgRet;
}

BOOL PlayLogoAnimation(HANDLE hMrf)
{
	UINT8 bpp_size = 0;
	static UINT16 s_u2Index = ANIMATION_START_NUM;
	static UINT32 s_u4PrevTime = 0;
	static BOOL s_fgFirstInit = FALSE;
	static BOOL s_fgFirstOSDInit = FALSE;
	UINT32 u4Curtime = 0;
	BOOL fgRet = FALSE;
	BITMAPOBJINFO rBitmapInfo = {0};
	void *pu2FB = NULL;
	UINT32 u2bytes = 0, u2Cnt = 0;
	static void *pu2FrameBuf = NULL; 
	MRFHEADER     *pMrfHeader = (MRFHEADER *)hMrf;
	picnum = pMrfHeader->u4picturecount;
	/*if(pMrfHeader->u4bitcount == 16) {
	picnum = picnum -1;
	}*/
	poswidth = pMrfHeader->u4poswidth;
	posheight = pMrfHeader->u4posheight;
	flashtime = pMrfHeader->u4frequence;
	DIS_WIDTH1 = _u4LCDWidth;
	DIS_HEIGHT1 = _u4LCDHeight;
    //Printf("enter PlayLogoAnimation \r\n");
	if (!s_fgFirstInit) {
        Printf("enter PlayLogoAnimation the 1st time\r\n");
        s_fgFirstInit = TRUE;
		//BootAnimationOsdInit(logo_base);
		pu2FrameBuf = ARM1PHY2ARM2UCV(logo_base);
        s_u4PrevTime = GetARM2TickCount();
    }

    u4Curtime = GetARM2TickCount();

	if (s_u2Index > (picnum - 2))
		s_u2Index = ANIMATION_START_NUM;
	
    if ((s_u2Index <= (picnum - 2))
		&& (abs(u4Curtime - s_u4PrevTime) / flashtime>= 1))
    {
        fgRet = GetBitmapInfo(hMrf, s_u2Index, &rBitmapInfo);
		
        
        if(fgRet)
        {
            
            
            pu2FB = (void *)GetRCObjectMemAddr(hMrf,(RCOBJECT *)&rBitmapInfo);
			
			//Printf("ID %d  w* h is %d*%d , u4BitCount: %d\r\n",s_u2Index,rBitmapInfo.u4Width,rBitmapInfo.u4Height, rBitmapInfo.u4BitCount);
            if(DIS_WIDTH1 == rBitmapInfo.u4Width)
                memcpy((void *)(pu2FrameBuf), (const void *)pu2FB,rBitmapInfo.u4Height*rBitmapInfo.u4Width*rBitmapInfo.u4BitCount/8);
            else
            {
                bpp_size = rBitmapInfo.u4BitCount / 8;
                u2bytes = rBitmapInfo.u4Width * bpp_size;
                for(u2Cnt = 0; u2Cnt < rBitmapInfo.u4Height; u2Cnt++)
                {
                    memcpy((void *)(pu2FrameBuf+((u2Cnt + posheight) * DIS_WIDTH1 + poswidth) * bpp_size), (const void *)(pu2FB + u2Cnt * rBitmapInfo.u4Width * bpp_size), u2bytes);
                }
            }
			if (!s_fgFirstOSDInit)
			{
				//Printf("DIS_WIDTH1: %d, DIS_HEIGHT1: %d, u4bitcount: %d, u4BitCount: %d, picnum: %d\n", DIS_WIDTH1, DIS_HEIGHT1, pMrfHeader->u4bitcount, rBitmapInfo.u4BitCount, picnum);
				BootAnimationOsdInit(logo_base, FALSE);
				s_fgFirstOSDInit = TRUE;
			}
			if(s_u2Index==0)
				DisplayLighten();
            
            s_u2Index = s_u2Index + 1;
            
        }
        s_u4PrevTime = u4Curtime;
    }

    return fgRet;
}

#if defined(CONFIG_ATC_PLATFORM_ac823x)
#define PDWNC_BASE 0x10024000
#endif

//#define LBA_ENABLE
#define ENABLE_LOAD_MRF    0

UINT32 BootAnimation_StateMach()
{
    unsigned int u4pdwncmode = 0;
	static BOOL s_fgFirstShowlogo = FALSE;
    u4pdwncmode = IO_READ32(PDWNC_BASE, 0x160);
#if ENABLE_LOAD_MRF
	static HANDLE hMrf = NULL;
	if(hMrf == NULL)
	{
		hMrf = ARM1PHY2ARM2UCV(LOGO_MRF_BUFFER_PA_ADDR);
	}
#endif

    //Printf("BootAnimation_StateMach\r\n");
    
    if (u4pdwncmode == PDWNCMODE)
    {
        //Printf("BootAnimation read power mode normal\r\n");
#ifdef CONFIG_ATC_OS_linux
		v_enable_bim_irq(VECTOR_VSYNC);
		v_enable_bim_irq(VECTOR_PANEL_SCALER);

		if (Last_Source_Ready == TRUE && (!aniStartPlayToEndExit || (aniStartPlayToEndExit && aniEos))) {
		#ifdef LBA_ENABLE
			HideBootAnimationOsd();
		#else
			HideVBAOverlay();
			disableVdecIRQ();
			releaseVdecHW();
		#endif
			//HWSendMessage(MODULE_BOOTANIMATION<<24,0,0,0);
			/*Make Sure SW reg write to HW reg in ISR*/
			//Sleep(20);
			//v_disable_bim_irq(VECTOR_VSYNC);
			//v_disable_bim_irq(VECTOR_PANEL_SCALER);

			ANI_INFO("[BootAnimation] Last_Source_Ready, aniStartPlayToEndExit %d , aniEos %d \r\n", aniStartPlayToEndExit, aniEos);
			//Last_Source_Ready = FALSE;

			return TASK_IDLE;
		}
		 if(fgDualHALGetUpgradeMode() == 1) {
		 	Printf("[BootAnimation]fgDualHALGetUpgradeMode is Upgrade OS\r\n");
			BootAnimationOsdInit(logo_base,FALSE);
			DisplayLighten();
			/*Make Sure SW reg write to HW reg in ISR*/
			vTaskDelay(pdMS_TO_TICKS(20));
			//v_disable_bim_irq(VECTOR_VSYNC);
			//v_disable_bim_irq(VECTOR_PANEL_SCALER);
			return TASK_IDLE;
		 }
		
		 if(fgDualHALGetUpgradeMode() == 2) {
		 	Printf("[BootAnimation]fgDualHALGetUpgradeMode is Recovery Mode\r\n");
			HideBootAnimationOsd();
			DisplayLighten();
			/*Make Sure SW reg write to HW reg in ISR*/
			vTaskDelay(pdMS_TO_TICKS(20));
			//v_disable_bim_irq(VECTOR_VSYNC);
			//v_disable_bim_irq(VECTOR_PANEL_SCALER);
			return TASK_IDLE;
		 }
#if ENABLE_LOAD_MRF
		MRFHEADER     *pMrfHeader = (MRFHEADER *)hMrf;
		picnum = pMrfHeader->u4picturecount;
		//Printf("[BootAnimation] picnum: %d\r\n", picnum);
#endif
		if (ANIMATION_STOP == TRUE) {
			Printf("[BootAnimation] ANIMATION_STOP\r\n");
			return TASK_BUSY;
		 }
	#ifdef LBA_ENABLE
		#if ENABLE_LOAD_MRF
		if(picnum > 3)
		{
			Printf("[BootAnimation] PlayLogoAnimation\r\n");
			PlayLogoAnimation(hMrf);
			return TASK_BUSY;
		}
		else
		{
			if(!s_fgFirstShowlogo)
			{
				Printf("[BootAnimation] PlayStaticLogo\r\n");
				PlayStaticLogo(hMrf);
				s_fgFirstShowlogo = TRUE;
			}
			return TASK_BUSY;
		}
		#endif
	#else
		if (xSemaphoreTake(xAniUpdateResSem, portMAX_DELAY) == pdFALSE)
		{
			ANI_INFO("xAniUpdateResSem failed\n");
		}
		if (FALSE == PlayVideoBootAnimationUseIRQAndBuffering() && aniPlayToEndExit && !aniNeedUpdate && (Last_Source_Ready == TRUE || aniStop)) {
			if (PLAY_WHICH_VBA == VBA_PLAY_SHUTDOWN_ANIMATION) {
				vDisableBacklight();//need modify , maybe shutdown backlight by system
			} else {
				DisplayLighten();
			}
			//maybe need notify to system playback end
			disableVdecIRQ();
			releaseVdecHW();
			ANI_INFO("vba end sg_EnterAwtkFlag %d\n", sg_EnterAwtkFlag);
			if (!sg_EnterAwtkFlag) {
			    HideVBAOverlay();
			}
			aniPlayToEndExit = 0;
			vdecIRQComplete = 1;
			arm2system_service_ani_end(NULL);
			xSemaphoreGive(xAniUpdateResSem);
			return TASK_IDLE;
		}
		xSemaphoreGive(xAniUpdateResSem);
	#endif
		return TASK_BUSY;
#else
	#if ENABLE_LOAD_MRF
		PlayStaticLogo(hMrf);
	#endif
        return TASK_IDLE;
#endif
    }
    else
    {
        Printf("BootAnimation read power mode suspend\r\n");
        return TASK_IDLE;
    }   
}

int getDisplayInitDoneFlag(void)
{
	return sg_displayInitFlag;
}
void BootAnimationTask(void * parameters)
{
	( void ) parameters;
	unsigned int u4pdwncmode = 0;

	xVdecSem = xQueueCreateCountingSemaphore(1,0);
	xAniPlaybackSem = xQueueCreateCountingSemaphore(1, 0);
	xAniStartPlaySem = xQueueCreateCountingSemaphore(1, 0);
	xAniUpdateResSem = xQueueCreateCountingSemaphore(1, 1);

	u4pdwncmode = IO_READ32(PDWNC_BASE, 0x160);
	if (u4pdwncmode == PDWNCMODE)
	{
		Printf("\nARM2 read power mode: normal\r\n");
		BL_DisplayInit();
		sg_displayInitFlag = 1;
	}

	ANI_INFO("BootAnimationTask IN\r\n");
	while (1) {
		if (TASK_IDLE == BootAnimation_StateMach()) {
			ANI_INFO("BootAnimationTask IN Idle\r\n");
			sg_VBAPlayFlag = 0;
			aniEos = 0;
			if (xSemaphoreTake(xAniPlaybackSem, portMAX_DELAY) == pdFALSE)
			{
				ANI_INFO("xAniPlaybackSem failed\n");
				continue;
			}
			if (aniNeedUpdate == 0) {
				if (xSemaphoreTake(xAniPlaybackSem, portMAX_DELAY) == pdFALSE)
				{
					ANI_INFO("xAniPlaybackSem failed second\n");
					continue;
				}
			}
			sg_VBAPlayFlag = 1;
			ANI_INFO("BootAnimationTask IN Run\r\n");
		}
	}
}

unsigned int BootAnimation_Init(void)
{
	xAniReadySem = xQueueCreateCountingSemaphore(1, 0);
	if (aniRegisterCallback == 0) {
		arm2system_service_status_handler_register(PlayBootAnimation, STATUS_ANI_ON, NULL);
		arm2system_service_status_handler_register(PlayShutDownAnimation, STATUS_ANI_OFF, NULL);
		arm2system_service_status_handler_register(StopAnimation, STATUS_ANI_STOP, NULL);
		aniRegisterCallback = 1;
	}
	xTaskCreate(BootAnimationTask,
		"BootAnimation",
		ANIMATION_TASK_STACK_SIZE,
		NULL,
		ANIMATION_TASK_PRIORITY,
		NULL );
	ANI_INFO("BootAnimation task create done\r\n");
    return 0;
}


UINT32 BootAnimation_Callback(UINT32 u4ModuleID, UINT32 u4Param1, UINT32 u4Param2, UINT32 u4Param3)
{
	if (u4Param1 == MESSAGE_LAST_SOURCE_READY) {
		ANI_INFO("get message last source ready\n");
		Last_Source_Ready = TRUE;
        SetEnableAwtkFlag(0);
		AwtkOsdEnable(0);
	}
	else if (u4Param1 == MESSAGE_ANIMATION_STOP) {
		ANI_INFO("get message animation stop");
		ANIMATION_STOP = TRUE;
	}
	else if (u4Param1 == MESSAGE_ANIMATION_VDEC_FREE) {
		ANI_INFO("get message MESSAGE_ANIMATION_VDEC_FREE");
		xSemaphoreGiveFromISR(xVdecSem, NULL);
	}
	else if (u4Param1 == MESSAGE_VDEC_HW_READY) {
		ANI_INFO("get message MESSAGE_VDEC_HW_READY");
		vdechwIsReady = 1;
	}
	else
	{
		if (u4Param1 == MESSAGE_ANIMATION_PLAY_BOOT) {
			ANI_INFO("get message MESSAGE_ANIMATION_PLAY_BOOT\n");
			if (PLAY_WHICH_VBA == VBA_PLAY_BOOT_ANIMATION) {
				ANI_INFO("get same message continuous VBA_PLAY_BOOT_ANIMATION\n");
				return 0;
			}
			PLAY_WHICH_VBA = VBA_PLAY_BOOT_ANIMATION;
			aniStartPos = aniBootStartPos;
			aniEndPos = aniBootEndPos;
			aniNeedUpdate = 1;
			xSemaphoreGiveFromISR(xAniPlaybackSem, NULL);
		}
		else if (u4Param1 == MESSAGE_ANIMATION_PLAY_SHUTDOWN) {
			ANI_INFO("get message MESSAGE_ANIMATION_PLAY_SHUTDOWN\n");
			if (PLAY_WHICH_VBA == VBA_PLAY_SHUTDOWN_ANIMATION) {
				ANI_INFO("get same message continuous MESSAGE_ANIMATION_PLAY_SHUTDOWN\n");
				return 0;
			}
			PLAY_WHICH_VBA = VBA_PLAY_SHUTDOWN_ANIMATION;
			aniStartPos = aniShutDownStartPos;
			aniEndPos = aniShutDownEndPos;
			aniNeedUpdate = 1;
			xSemaphoreGiveFromISR(xAniPlaybackSem, NULL);
		}
	}
	return(0);
}

extern uint8_t dpu_init_flag;
extern int awtk_task_quit(void);

int GetVBAPlayFlag(void)
{
	return sg_VBAPlayFlag;
}

int SetCarplayStatusUpdateFlag(int flag)
{
	sg_CarplayStatusUpdateFlag = flag;
	return 0;
}

int GetCarplayStatusUpdateFlag(void)
{
	return sg_CarplayStatusUpdateFlag;
}

int SetCarplayStartFlag(int flag)
{
	sg_CarplayStartFlag = flag;
	return 0;
}

int GetCarplayStartFlag(void)
{
	return sg_CarplayStartFlag;
}

int SetAbnormalFlag(int flag)
{
	//dpu_init_flag = 0;
	//v_enable_bim_irq(VECTOR_VSYNC);
	//v_enable_bim_irq(VECTOR_PANEL_SCALER);
	sg_AbnormalFlag = flag;
	return 0;
}

int GetAbnormalFlag(void)
{
	return sg_AbnormalFlag;
}

int SetRebootFlag(int flag)
{
	sg_RebootFlag = flag;
	return 0;
}

int GetRebootFlag(void)
{
	return sg_RebootFlag;
}

//extern int awtk_task_quit(void);

void SetEnableAwtkFlag(int enable)
{
	sg_EnterAwtkFlag = enable;
	//Printf("SetEnableAwtkFlag enable: %d\n", enable);
}

int GetEnableAwtkFlag(void)
{
	return sg_EnterAwtkFlag;
}
void NotifyArm1ReleaseAwtkRes(void)
{
	HWSendMessage(MODULE_BOOTANIMATION<<24,0,0,0);
	Printf("NotifyArm1ReleaseAwtkRes\n");
}
