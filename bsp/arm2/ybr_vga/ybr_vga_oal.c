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
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER 
AGREES
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
#include "vga_hal_api.h"
#include "x_typedef.h"
#include "ybr_vga_oal.h"
#include "dual_task.h"
#include "x_timer.h"
#include "wch_drv.h"
#include "backcar_cfg.h"
#include "types.h"
#include "x_bim_83xx.h"

extern WCH_BUFF_INFO_T g_WchBufferInfo;
extern WCH_SRC_APP_ID_E g_srcAppId;
extern WCH_DATA_SRC_E g_dataSrc;

extern u32 g_u4WchId;
extern WCH_BUFF_INFO_T g_WchBufferInfo;
WCH_CTL_PARAM_T g_mWchCtl;
WCH_CFG_T g_mWchCfg;
HANDLE  g_VdoBufEvt    = NULL;
bool g_fgPAL = FALSE;
bool g_fgDeInterlace = FALSE;
extern bool g_fgYBRVGAInit;
extern bool g_fgWchStopped;

extern unsigned int backcarUI_base;
extern unsigned int backcarUI_size;

bool g_fgStopWch = FALSE;
bool g_fgStartWch = FALSE;

typedef VOID (*PFNArm2EvtCBFunc)(u32 u4Data);

typedef struct
{
   TCHAR            szEvtName[20];
   PFNArm2EvtCBFunc pfEvtCallBack;
   u32           u4EvtData;
} ARM2_EVT_T, *PARM2_EVT_T;


void vUtDelay1ms(u32 n)
{
    u32 i = 0;
    u32 start = GetMinisecond() ;
    if (i <= n)
    {
        i = GetMinisecond() -start;
        
    }
}
void vUtDelay2us(u32 n)
{
    u32 i = 0;
    u32 start = GetMicrosecond() ;
    if (i <= n)
    {
        i = GetMicrosecond() -start;    
    }
}

void HAL_GetTime(HAL_TIME_T* pTime) 
{
    
    u32 u4Time = GetMinisecond();
    pTime->u4Seconds = u4Time / 1000;
    pTime->u4Micros = 1000*(u4Time % 1000);
}

void HAL_GetDeltaTime(HAL_TIME_T* pResult, HAL_TIME_T* pT0,  HAL_TIME_T* pT1)
{
    
    u32 u4Value_second = 0;
    u32 u4Value_mico = 0;

    if(pT1->u4Micros >= pT0->u4Micros)
    {
        pResult->u4Seconds = pT1->u4Seconds -  pT0->u4Seconds;
        pResult->u4Micros = pT1->u4Micros -  pT0->u4Micros;
    }
    else
    {
        pResult->u4Seconds = pT1->u4Seconds -  pT0->u4Seconds - 1;
        pResult->u4Micros = 1000000 + pT1->u4Micros -  pT0->u4Micros;
    }
}

VOID Arm2YBRFrameDoneHandler(u32 u4Data)
{    
        u32 u4BufId = (u4Data & 0xF);       
     //   Printf("Arm2YBRFrameDoneHandler:wch id:%d\r\n", u4BufId);
        BackCarVdpFlip(g_WchBufferInfo.u4YBuf[u4BufId], g_WchBufferInfo.u4CBuf[u4BufId], g_fgDeInterlace); //
}

void YBRFrameDoneEvtInit(void)
{
        g_VdoBufEvt = NULL;
        if(NULL == g_VdoBufEvt) {
#if NEW_WCH_EVENT_NAME
              if (g_srcAppId == SRC_APP_YPBPR) {
                  g_VdoBufEvt = X_CreateEvent(WCH_YPBPR_EVENT);
              } else {
                  g_VdoBufEvt = X_CreateEvent(WCH_VGA_EVENT);
              }
#else
              if (g_u4WchId == WCH_1) {
                  g_VdoBufEvt = X_CreateEvent(WCH1_FRAME_DONE_EVENT);
              } else {
                  g_VdoBufEvt = X_CreateEvent(WCH2_FRAME_DONE_EVENT);
              }
#endif
              if(NULL == g_VdoBufEvt) {
                  Printf("YBRFrameDoneEvtInit(): g_VdoBufEvt Evt Create FAILED!\r\n");
              } else {
              
                 ((PARM2_EVT_T)g_VdoBufEvt)->pfEvtCallBack =  Arm2YBRFrameDoneHandler;
                  Printf("YBRFrameDoneEvtInit(): g_VdoBufEvt Evt: %s Create Success!\r\n", ((PARM2_EVT_T)g_VdoBufEvt)->szEvtName);
              }
         }
}

/**
    video Main loop
*/
int   u4DrvVideoMainLoop(void *arg)
{
    //Printf("u4DrvVideoMainLoop:Signal Type is %d, Main Loop Enter\n", g_u4SrcType);
    
    if(g_fgYBRVGAInit)
    {
        // VGA State Machine
        if(!_IsVgaDetectDone)
        {
            vVgaModeDetect();
        }
        else
        {
            vVgaChkModeChange();
        }

        // HDTV State Machine
        if(!_IsHdtvDetectDone)
        {        
            vHdtvModeDetect();
        }
        else
        {
            vHdtvChkModeChange();
        }
        //Signal Status Process
        g_u4SigStatus = bDrvVideoSignalStatus();
        if (g_u4SigStatus != g_u4SigPreStatus)
        {
            //if (NULL != g_hSigStateEvt) 
            {
                switch(g_u4SigStatus)
                {
                    case SV_VDO_NOSIGNAL:
                    case SV_VDO_NOSUPPORT:
                        //stop write channel
                        if (!g_fgStopWch && g_fgStartWch) {
                             if(WchIoControl(1, IOCTL_WCH_STOP, &g_srcAppId, sizeof(g_srcAppId), 
                                &g_u4WchId, sizeof(UINT32), NULL))
                            {
                                Printf("u4DrvVideoMainLoop: wch stop fail! \r\n"); 
                            } else {
                                Printf("u4DrvVideoMainLoop: wch stop sucess!\r\n");                                 
                                g_fgStopWch = TRUE;
                                g_fgStartWch = FALSE;
                                g_fgWchStopped = TRUE;
                            }
                        }       
                        Printf("u4DrvVideoMainLoop: Signal is loss or unsupport or unknown!\r\n");
                    break;

                    case SV_VDO_STABLE:
                        g_mWchCfg.fgVSyncPolarity = FALSE; // FALSE is LOW level present sync.
                        g_mWchCfg.fgHSyncPolarity = TRUE; // TRUE is High. 
                        g_mWchCfg.eInputSrc = g_dataSrc;

                        g_mWchCfg.eInputFmt = DATA_FMT_YUV444;
                        g_mWchCfg.u4SrcWidth = (unsigned int) wDrvVideoInputWidth();
                        g_mWchCfg.u4SrcHeight = (unsigned int) wDrvVideoInputHeight();
                        
                        g_mWchCfg.u4DstWidth =  (unsigned int) wDrvVideoInputWidth();
                        g_mWchCfg.u4DstHeight = (unsigned int) wDrvVideoInputHeight();
                        g_mWchCfg.eOutputFmt = DATA_FMT_YUV420;

                        g_mWchCfg.u1YSel = 1;//may change cause of different hw
                        g_mWchCfg.u1USel = 5;//above
                        g_mWchCfg.u1VSel = 5;//above
                        
                        g_mWchCfg.fgProgressive =  (bDrvVideoIsSrcInterlace() ==1)?0: 1;
                        g_fgPAL = (g_mWchCfg.u4SrcHeight == 576) ?TRUE:FALSE;
                                         
                        g_mWchCfg.fgBotFieldFirst = 0;

                        g_mWchCtl.tWchCfg = g_mWchCfg;
                        g_mWchCtl.eSrcId = g_srcAppId;
                 
                        g_fgDeInterlace = bDrvVideoIsSrcInterlace();
                        
                        BackCarOverlayInit(backcarUI_base, g_WchBufferInfo.u4YBuf[0], g_WchBufferInfo.u4CBuf[0], TRUE, g_fgPAL, g_fgDeInterlace);
                        
                        if(WchIoControl(1, IOCTL_WCH_CONFIG, &g_mWchCtl, sizeof(WCH_CTL_PARAM_T), 
                                NULL, 0, NULL))
                        {         
                            Printf("u4DrvVideoMainLoop: wch config fail!\r\n");
                            return FALSE;
                        } else {     
                            Printf("u4DrvVideoMainLoop: wch config success: src app id:%d, data src id:%d!Video Width:%d Height:%d, Interlace:%d, DI:%d, BottomFirst:%d\r\n",
                                  g_mWchCtl.eSrcId, g_mWchCfg.eInputSrc,g_mWchCfg.u4SrcWidth, g_mWchCfg.u4SrcHeight, bDrvVideoIsSrcInterlace(), g_fgDeInterlace, g_mWchCfg.fgBotFieldFirst);
                        }
                            
                            
                        if(WchIoControl(1, IOCTL_WCH_START, &g_srcAppId, sizeof(g_srcAppId), 
                            NULL, 0, NULL))
                        {         
                            Printf("u4DrvVideoMainLoop: wch start fail!"); 
                            return FALSE;
                        } else {                      
                            Printf("u4DrvVideoMainLoop: wch start success!");
                            g_fgStopWch = FALSE;
                            g_fgStartWch = TRUE;
                        }
                         g_u1Timing = bDrvVideoGetTiming();
                         Printf("Signal is Stable, Timing is %d\n", g_u1Timing);
                         //enable blank Level Adjust                      
                         vDrvEnableBlankLevelAdjust();
                     //doing auto
                     //vDrvVideoAuto(); 
                     break;

                    case SV_VDO_UNKNOWN:
                        Printf("Signal is unknown\n");                        
                    break;                  
                }
            } 

            UTIL_Printf("Signal Status(Pre,Cur):(%d, %d)\n", g_u4SigPreStatus, g_u4SigStatus);
            g_u4SigPreStatus = g_u4SigStatus; 
        }
        
        // VGA auto state machine
        vVdoSP0AutoState();
        vDrvAdjustBlankLevel();
        vDrvOnChipAutoColorIteration(); // do auto color
        vDrvPGALinearityVerify();
    }
    
    return 0;
}

void YbrIsr(u16 u2Vector) 
{   
    if (u2Vector == 29)
    {
            //Irq Status Process
            g_u4IrqStatus = u4DrvVideoGetIrqStatus();         
            if (g_u4IrqStatus != 0)
            {
                  vDrvVideoClearIrqStatus(g_u4IrqStatus);                       
                 if (g_u4IrqStatus == (0x1 << 2)) {
                    if (!g_fgStopWch && g_fgStartWch) {
                         if(WchIoControl(1, IOCTL_WCH_STOP, &g_srcAppId, sizeof(g_srcAppId), 
                            &g_u4WchId, sizeof(UINT32), NULL))
                        {
                            Printf("YbrIsr: wch stop fail! \r\n"); 
                        } else {
                            Printf("YbrIsr: wch stop sucess!\r\n"); 
                            g_fgStopWch = TRUE;
                            g_fgStartWch = FALSE;
                            g_fgWchStopped = TRUE;
                        }
                    }
                 }
                 
                 switch (g_u4SrcType) 
                 {
                     case SRC_YBR:
                       vHdtvISR();
                       break;

                     case SRC_VGA:
                       vVgaISR();
                       break;
                }
            }    
            ac83xx_mask_ack_bim_irq(u2Vector);            
    }
}


