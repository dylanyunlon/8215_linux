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
#include "BCLib.h"
#include "dual_hal.h"
#include "dual_task.h"
#include "dual_callback.h"
#include "backcar_msg.h"
#include "backcar_cfg.h"
#include "tvd_drv_if.h"
#include "fsc_sound.h"
#include "wch_drv.h"
#include "x_pdwnc.h"
#include <generated/atc_project.h>
#include "arm2_comm_data_struct.h"

#ifdef CONFIG_ATC_PLATFORM_ac823x
#include "ac823x_gpio_pinmux.h"
#include "ac823x_pinmux_table.h"
#else
#include "ac83xx_gpio_pinmux.h"
#include "ac83xx_pinmux_table.h"
#include "pinmux.h"
#include "gpio.h"
#include "arm2pcmplay.h"
#endif

#if (BACK_CAR_SRC_VGA||BACK_CAR_SRC_YPBPR)
#include "ybr_vga_oal.h"
#include "vga_hal_api.h"
#endif

// used in BackCar_Callback() : MSG_GET_ARM2_VERSION
#define ARM2_VERSION "20111124"


#ifdef CONFIG_ATC_PLATFORM_ac823x
#define VECTOR_PMXIRQ    45
#endif

static BOOL g_fArm1Ready = FALSE;

BOOL g_fgYBRVGAInit = FALSE;
BOOL g_fgIsArm2BCTaskExist = FALSE;
BOOL g_fFrameHandleStop = FALSE;

#ifdef CONFIG_ATC_PLATFORM_ac823x
BOOL g_fWchStatus = FALSE;
#endif


WCH_SRC_APP_ID_E  g_srcAppId = SRC_APP_UNKNOWN;
WCH_DATA_SRC_E g_dataSrc = DATA_SRC_UNKNOWN;

UINT32 g_u4WchId = -1;
WCH_BUFF_INFO_T g_WchBufferInfo;

// set in BCGPIOInit(unsigned long u4GPIOPinNum)  and used in BCGetGPIOVal()
static long u4UsingGPIOPinNum;

#ifdef CONFIG_ATC_PLATFORM_ac823x
WCH_BUF_T g_WchBufferT;
#endif

#ifdef ARM1_EXCEPTION_ARM2_BACKCAR_ENABLE
BOOL g_arm1_ack = FALSE;
BOOL g_arm1_bc_success = FALSE;
#endif

/*this name is just to suit display which have extern this variable*/
u8 gu4CurTVMode = AV_MODE_NONE;

// tvd signal state. set in signal_ready_process, used in arm2_tvd_frame_handle
static TVD_SIG_STATE_T  signal_state = TVD_SIG_NONE;

// frame count wch sends to backcar
static u8 count = 0;

// used for doing something when the first frame from wch is sent to backcar. example in arm2_tvd_frame_handle.
static bool is_first_buffer = true;

extern unsigned int backcarUI_base;
extern unsigned int backcarUI_size;


extern BOOL HideBackCarOverlay(BOOL fgHideOverlay);

BOOL NotifyARM1(UINT32 u4Message, UINT32 u4BackCarSatus)
{
    HWSendMessage(MSG_COMBINE(MODULE_BCAR, u4Message), u4BackCarSatus, 0, 0);
    //ARM2PCMClose();
    Printf("arm2 notify arm1 arm2 will exit\n");
#ifdef CONFIG_ATC_PLATFORM_ac823x
// 8237 empty
#else
// 8317 & linux. keep the legacy code before know why it does this OR it causes bugs
    if (u4BackCarSatus == ARM2_STATUS_NO_BACK_CAR)
    {//close tvd clock
        *(UINT32 *)0xFD0000B4 = ((*(UINT32 *)0xFD0000B4) & 0XFFFFFFF9);
        *(UINT32 *)0xFD0000D0 = ((*(UINT32 *)0xFD0000D0) & 0XFFFFFFFD);
    }
#endif
    return TRUE;
}

BOOL BCUartInit(UINT32 u4Port, UINT32 u4BaudRate)
{
    UartInit(u4BaudRate);

    return TRUE;
}

void BCGPIOInit(unsigned long u4GPIOPinNum)
{
    u4UsingGPIOPinNum = u4GPIOPinNum;

#ifdef CONFIG_ATC_PLATFORM_ac823x
    GPIO_Pull_UpDown(u4GPIOPinNum, PULLUP);
    ac823x_gpio_inout_sel_reg(u4GPIOPinNum, 0);
#else
    GPIO_MultiFun_Set(u4GPIOPinNum, PINMUX_LEVEL_GPIO_END_FLAG);
    //gpio_request(u4GPIOPinNum,"BackCar_Init_GPIO");
    gpio_direction_input(u4GPIOPinNum);
    //printk("GPIO %d Init Success!\r\n", u4GPIOPinNum);
#endif

}

int BCGetGPIOVal()
{
    int u8Val = 0;
#ifdef CONFIG_ATC_PLATFORM_ac823x
    u8Val = ac823x_gpio_get_value_reg(u4UsingGPIOPinNum);
#else
    u8Val = gpio_get_value(u4UsingGPIOPinNum);
#endif

    return u8Val;
}

BOOL BCUartReadBlockData(BYTE *pData, UINT32 u4ByteToRead)
{
    ReadBlockData(pData, u4ByteToRead);

    return TRUE;
}

BOOL BCUartWriteBlockData(BYTE *pData, UINT32 u4ByteToWrite)
{
    WriteBlockData(pData, u4ByteToWrite);

    return TRUE; 
}

VOID WCHInit()
{
    UINT32 i = 0;
#ifdef CONFIG_ATC_PLATFORM_ac823x
    g_WchBufferT.u1WchId = WCH_5;
    if (WCH_SUCCESS != OpenWch(WCH_5, SRC_APP_BACKCAR_WCH5)) {
        Printf("execute the OpenWch failed!\r\n");
        return;
    }

    g_fWchStatus = TRUE;
    if(WCH_SUCCESS != WchGetBufferAddress(&g_WchBufferT)){
        Printf("execute the WchGetBufferAddress failed!\r\n");
        return;
    }

    for (i = 0; i < g_WchBufferT.tWchBuf.u4BufCnt; i++) {
        Printf("BuffCnt:%d, YAddr:0x%08x, CAddr:0x%08x\r\n",
                i, g_WchBufferT.tWchBuf.u4YBuf[i], g_WchBufferT.tWchBuf.u4CBuf[i]);
    }
#else
    #if BACK_CAR_SRC_YPBPR
        g_srcAppId = SRC_APP_YPBPR;
        g_dataSrc = DATA_SRC_YPBPR;
    #elif BACK_CAR_SRC_VGA
        g_srcAppId = SRC_APP_VGA;
        g_dataSrc = DATA_SRC_VGA;
    #else
        g_srcAppId = SRC_APP_BACKCAR;
    #endif

    WchIoControl(1, IOCTL_WCH_OPEN, &g_srcAppId, sizeof(WCH_SRC_APP_ID_E),
        &g_u4WchId, sizeof(UINT32), NULL);
    Printf("ARM2 Open WCH ID is %d used for src type:%d\r\n", g_u4WchId, g_srcAppId);

    #if !NEW_WCH_EVENT_NAME // create event in wch open in new case
    WchCreateEvent();
    #endif

    #if (BACK_CAR_SRC_YPBPR ||BACK_CAR_SRC_VGA)
    YBRFrameDoneEvtInit();
    #endif

    WchIoControl(1, IOCTL_WCH_GET_ADDR, &g_srcAppId, sizeof(WCH_SRC_APP_ID_E),
        &g_WchBufferInfo, sizeof(WCH_BUFF_INFO_T), NULL);
    // check buf count
    if(g_WchBufferInfo.u4BufCnt <= 0){
        Printf("error: %s: BufCnt: %d \r\n",__FUNCTION__,g_WchBufferInfo.u4BufCnt);
    }

    for (i = 0; i < g_WchBufferInfo.u4BufCnt; i++) {
        Printf("BuffCnt:%d, YAddr:0x%08x, CAddr:0x%08x\r\n",
                i, g_WchBufferInfo.u4YBuf[i], g_WchBufferInfo.u4CBuf[i]);
    }
#endif
}

#ifdef CONFIG_ATC_PLATFORM_ac823x
void arm2_tvd_frame_handle(unsigned int *u4pData)
{
    u32 buf_index = (*u4pData & TVD_IDX_FLAG_MASK);
#else
void arm2_tvd_frame_handle(u32 u4Data)
{
    u32 buf_index = (u4Data & TVD_IDX_FLAG_MASK);
#ifdef CONFIG_ATC_OS_linux
    /*discard 4 frames to handle wch data problem for now*/
    if (count < 4) {
        count++;
        return;
    }
#endif
#endif

    // check buf_index before using it
#ifdef CONFIG_ATC_PLATFORM_ac83xx
    if(buf_index < 0 || buf_index >= g_WchBufferInfo.u4BufCnt)
#else
    if(buf_index < 0 || buf_index >= g_WchBufferT.tWchBuf.u4BufCnt)
#endif
    {
        Printf("error: arm2_tvd_frame_handle recv wrong buf_index: %d\n", buf_index);
        return ;
    }

    // this function is called in wch isr.
    //Before enqueuing frame to VDP, check if main thread has called BCreleaseResource ( hide the surface.)
    if(!g_fFrameHandleStop){
        // yes. video surface has not been hidden. we are allowed to enqueue frame to VDP.
        if ((TVD_SIG_READY == signal_state) ||  (TVD_SIG_CHANGE_DONE == signal_state)) {
            bool di_flag = false;
            tvd_get_di_flag(&di_flag);
            if (is_first_buffer) {
                // first log is familiar to developers of TVD IN LK feature. second log is used for BVT ext test.
                // rm one log when it is a good time.
                Printf("arm2_tvd_frame_handle flip first frame and consume time is %ums\n", GetBootTime());
                Printf("[Arm2] start time - enque frame to vdp: %d\n", GetBootTime());
#ifdef ATC_AOSP_EHANCEMENT_ATCATE
                //pull up
                Printf("mcu_test: pull up :pin %d --> %d, at: %d\r\n",BACKCAR_NOTIFY_MCU_GPIO_PIN,BACKCAR_NOTIFY_MCU_GPIO_HIGH, GetARM2TickCount());
                if(0 != GPIO_MultiFun_Set(BACKCAR_NOTIFY_MCU_GPIO_PIN, PINMUX_LEVEL_GPIO_END_FLAG)){
                    Printf("mcu_test: GPIO_MultiFun_Set fail \r\n");
                }
                if(0 > gpio_direction_output(BACKCAR_NOTIFY_MCU_GPIO_PIN, BACKCAR_NOTIFY_MCU_GPIO_HIGH)){
                    Printf("mcu_test: gpio_direction_output fail \r\n");
                }
                Printf("mcu_test: pin %d == %d \r\n",BACKCAR_NOTIFY_MCU_GPIO_PIN,ac83xx_gpio_get_value_reg(BACKCAR_NOTIFY_MCU_GPIO_PIN));
#endif
                is_first_buffer = false;
            }

#ifdef CONFIG_ATC_PLATFORM_ac823x
            BackCarVdpFlip(g_WchBufferT.tWchBuf.u4YBuf[buf_index], g_WchBufferT.tWchBuf.u4CBuf[buf_index], di_flag);
#else
            BackCarVdpFlip(g_WchBufferInfo.u4YBuf[buf_index], g_WchBufferInfo.u4CBuf[buf_index], di_flag); //now, have not di
#endif
        }
    }else {
        Printf("arm2_tvd_frame_handle been stoped when BCReleaseResourcee() called\n");
    }
}

static void signal_ready_process(int mode)
{
    u32 width;
    u32 heigh;

    // do not change ths static modifier.
    // No this modifier, Linux arm2 backcar video will
    // be Horizonal Flip. A Very Strange BUG.
    static BOOL bIsPAL = FALSE;
#ifdef CONFIG_ATC_PLATFORM_ac823x
    WCH_CFG_T wch_ctl_param;
    memset(&wch_ctl_param, 0, sizeof(WCH_CFG_T));
    wch_ctl_param.eSrcId = SRC_APP_BACKCAR_WCH5;
    wch_ctl_param.u1WchId = WCH_5;
    wch_ctl_param.u4ScanLineMode = 1;
    wch_ctl_param.fgVSyncPolarity = FALSE; /* FALSE is LOW level present sync.*/
    wch_ctl_param.fgHSyncPolarity = TRUE; /* TRUE is High.*/
    wch_ctl_param.eInputSrc = DATA_SRC_TVD0;
    wch_ctl_param.eInputFmt = DATA_FMT_YUV444;
    wch_ctl_param.fgProgressive = FALSE;
    wch_ctl_param.eOutputFmt = DATA_FMT_YUV420;
#else
    WCH_CTL_PARAM_T wch_ctl_param;
    memset(&wch_ctl_param, 0, sizeof(WCH_CTL_PARAM_T));
    wch_ctl_param.eSrcId = SRC_APP_BACKCAR;
    wch_ctl_param.tWchCfg.fgVSyncPolarity = FALSE; /* FALSE is LOW level present sync.*/
    wch_ctl_param.tWchCfg.fgHSyncPolarity = TRUE; /* TRUE is High.*/
    wch_ctl_param.tWchCfg.eInputSrc = DATA_SRC_TVD;
    wch_ctl_param.tWchCfg.eInputFmt = DATA_FMT_YUV422;
    wch_ctl_param.tWchCfg.fgProgressive = FALSE;
    wch_ctl_param.tWchCfg.eOutputFmt = DATA_FMT_YUV420;
    wch_ctl_param.tWchCfg.u1YSel = 1;/*may change cause of different hw*/
    wch_ctl_param.tWchCfg.u1USel = 5;/*above*/
    wch_ctl_param.tWchCfg.u1VSel = 5;/*above*/
#endif

    switch (mode) {
    case AV_MODE_PAL:
        width = PAL_FRAME_WIDTH;
        heigh = PAL_FRAME_HEIGHT;

#ifdef CONFIG_ATC_PLATFORM_ac823x
        wch_ctl_param.fgBotFieldFirst = 0;
        wch_ctl_param.u4SrcStartYTop = 2;
        wch_ctl_param.u4SrcStartYBot = 2;
        wch_ctl_param.u4SrcStartX = 0x3B;
#else
        wch_ctl_param.tWchCfg.fgBotFieldFirst = 0;
        wch_ctl_param.tWchCfg.u4SrcStartYTop = 2;
        wch_ctl_param.tWchCfg.u4SrcStartYBot = 2;
#endif
        Printf("the current signal mode is PAL\n");
        break;
#ifdef CONFIG_ATC_PLATFORM_ac823x
    case AV_MODE_PAL_M:
    case AV_MODE_PAL60:
#endif
    case AV_MODE_NTSC443:
    case AV_MODE_NTSC:
        width = NTSC_FRAME_WIDTH;
        heigh = NTSC_FRAME_HEIGHT;

#ifdef CONFIG_ATC_PLATFORM_ac823x
        wch_ctl_param.fgBotFieldFirst = 1;
        wch_ctl_param.u4SrcStartYTop = 0;
        wch_ctl_param.u4SrcStartYBot = 0;
#else
        wch_ctl_param.tWchCfg.fgBotFieldFirst = 1;
        wch_ctl_param.tWchCfg.u4SrcStartYTop = 0;
        wch_ctl_param.tWchCfg.u4SrcStartYBot = 0;
#endif
        Printf("the current signal mode is NTSC or NTSC443\n");
        break;

    case AV_MODE_SECAM:
        width = SECAM_FRAME_WIDTH;
        heigh = SECAM_FRAME_HEIGHT;

#ifdef CONFIG_ATC_PLATFORM_ac823x
        wch_ctl_param.fgBotFieldFirst = 0;
        wch_ctl_param.u4SrcStartYTop = 1;
        wch_ctl_param.u4SrcStartYBot = 1;
#else
        wch_ctl_param.tWchCfg.fgBotFieldFirst = 0;
        wch_ctl_param.tWchCfg.u4SrcStartYTop = 1;
        wch_ctl_param.tWchCfg.u4SrcStartYBot = 1;
#endif
        Printf("the current signal mode is SECAM\n");
        break;

    default:
        Printf("the current signal mode is not support\n");
        return;
    }

    gu4CurTVMode = mode;
    if (AV_MODE_PAL == gu4CurTVMode) {
         bIsPAL = TRUE;
    } else {
         bIsPAL = FALSE;
    }

    // init VDP here.
    // TODO: check the implementation of BackCarOverlayInit with Display team whether it will show the frame addressed by u4YBuf[0] and u4CBuf[0].
    // If it will, this will lead to a grabage frame to be shown.
#ifdef  CONFIG_ATC_PLATFORM_ac823x
    // use NULL as first parameter because the forth parameter is TRUE. see the implementation of BackCarOverlayInit
    BackCarOverlayInit(NULL, g_WchBufferT.tWchBuf.u4YBuf[0],
        g_WchBufferT.tWchBuf.u4CBuf[0], TRUE, bIsPAL, TRUE);
#else
    // use NULL as first parameter because the forth parameter is TRUE. see the implementation of BackCarOverlayInit
    BackCarOverlayInit(NULL, g_WchBufferInfo.u4YBuf[0],
        g_WchBufferInfo.u4CBuf[0], TRUE, bIsPAL, TRUE);
#endif

#ifdef CONFIG_ATC_PLATFORM_ac823x
    wch_ctl_param.u4SrcWidth = width;
    wch_ctl_param.u4SrcHeight = heigh;
    wch_ctl_param.u4DstWidth = width;
    wch_ctl_param.u4DstHeight = heigh;
    wch_ctl_param.GetWchBufIndx = arm2_tvd_frame_handle;

    if (WCH_SUCCESS != ConfigWch(&wch_ctl_param)) {
        Printf("execute the ConfigWch failed!\n");
        return;
    }

    if (WCH_SUCCESS != StartWch(WCH_5, wch_ctl_param.eSrcId)) {
        Printf("execute the StartWch failed!\n");
        return;
    }
#else
    wch_ctl_param.tWchCfg.u4SrcWidth = width;
    wch_ctl_param.tWchCfg.u4SrcHeight = heigh;
    wch_ctl_param.tWchCfg.u4DstWidth = width;
    wch_ctl_param.tWchCfg.u4DstHeight = heigh;

    if (WchIoControl(1, IOCTL_WCH_CONFIG, (u8 *)&wch_ctl_param, sizeof(WCH_CTL_PARAM_T),NULL, 0, NULL)) {
        Printf("execute the IOCTL_WCH_CONFIG command failed\n");
        return;
    }

    WCH_SRC_APP_ID_E app_id = SRC_APP_BACKCAR;
    if (WchIoControl(1, IOCTL_WCH_START, (u8 *)&app_id, sizeof(app_id),NULL, 0, NULL)) {
        Printf("execute the IOCTL_WCH_START command failed\n");
        return;
    }
#endif

}

static void signal_lost_process(void)
{
    //arg is false, only VDP stream off.
    HideBackCarOverlay(false);

#ifdef CONFIG_ATC_PLATFORM_ac823x
    if (StopWch(WCH_5, SRC_APP_BACKCAR_WCH5)) {
        Printf("execute the StopWch command failed\n");
        return;
    }
#else
    WCH_SRC_APP_ID_E app_id = SRC_APP_BACKCAR;
    if(WchIoControl(1, IOCTL_WCH_STOP, (u8 *)&app_id, sizeof(WCH_SRC_APP_ID_E),NULL, 0, NULL)) {
        Printf("execute the IOCTL_WCH_STOP command failed\n");
        return;
    }
#endif

    return;
}

static void mode_change_start_process(void)
{
    signal_lost_process();
}

static void mode_change_done_process(u32 mode)
{
    signal_ready_process(mode);
}

static void arm2_tvd_signal_status(void *signal_status)
{
    TVD_SIG_INFORMATION *signal_info = NULL;
    signal_info = (TVD_SIG_INFORMATION *)signal_status;
    Printf("arm2_tvd_signal_status enter\n");

    if(NULL == signal_info) {
        Printf("arm2_tvd_signal_status leave,because the input parameter is NULL,so failed\n");
        return;
    }

    switch(signal_info->signal_state) {
        case TVD_SIG_READY:
            Printf("the signal is ready\n");
            signal_ready_process(signal_info->arg);
            count = 0;
            break;
        case TVD_SIG_LOST:
            Printf("the signal is lost\n");
            signal_lost_process();
            break;
        case TVD_SIG_CHANGE_START:
            Printf("the signal mode start to change\n");
            mode_change_start_process();
            break;
        case TVD_SIG_CHANGE_DONE:
            Printf("the signal mode change done\n");
            mode_change_done_process(signal_info->arg);
            count = 0;
            break;
        default:
            break;
    }
    signal_state = signal_info->signal_state;
    Printf("arm2_tvd_signal_status leave\n");

}

u32 init_tvd(void)
{
    u32 ret = 0;

    /*register notify call back function to tvd*/
    tvd_register_notify(&arm2_tvd_signal_status,NULL);
#if ((defined CONFIG_ATC_PLATFORM_ac83xx) && (defined INIT_TVD_BEFORE_ARM2_START))
    static bool first_init = true;
    if (first_init) {
        /*enable VPRES & Mode Change interrupt */
        u32 value = 0;
        value = (*((volatile u32 *)(0xFD0A740C)));
        (*((volatile u32 *)(0xFD0A740C))) = (value & (~ (((1U << 1)) | (1U << 0))));
        ret = tvd_internal_ioctl(OPTIMIZATION_TVD, NULL, NULL);
        first_init = false;
        return ret;
    } else {
        Printf("this is not first start backcar, so work normal follow\n");
    }
#endif
    TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T tvd_init_param;
    memset(&tvd_init_param, 0, sizeof(TVD_DRV_CAMERA_PREVIEW_INIT_INFO_T));
    tvd_init_param.eVdoInFmt     = TVD_VDOFMT_YUV444;
    tvd_init_param.eVdoOutFmt    = TVD_VDOFMT_YUV420;
#ifdef CONFIG_ATC_PLATFORM_ac823x
    tvd_init_param.channel_id    = TVD_CH_0;
#endif
    tvd_init_param.u4CHACvbsInxP = CVBSIN_1P;
    tvd_init_param.u4CHBCvbsInxP = CVBSIN_NONE;
    tvd_init_param.u4CHAOutDest  = TVD_CFG_OUT_DRAM;
    tvd_init_param.u4CHBOutDest  = TVD_CFG_OUT_NONE;
    tvd_init_param.source_type   = TVD_APP_ID_BACKCAR_ARM2;
    ret = tvd_internal_ioctl(TVD_CONTROL_CODE_INIT, (u8 *)&tvd_init_param, NULL);
    return ret;
}

u32 start_tvd(void)
{
    u32 ret = 0;
    return ret;
}

u32 stop_tvd(void)
{
    u32 ret = 0;
    TVD_DRV_STOP_PARA_T tvd_stop_param;
    memset(&tvd_stop_param, 0, sizeof(TVD_DRV_STOP_PARA_T));
    tvd_stop_param.stop_channel      = TVD_CHA;
    tvd_stop_param.source_type    = TVD_APP_ID_BACKCAR_ARM2;
#ifdef CONFIG_ATC_PLATFORM_ac823x
    tvd_stop_param.channel_id = TVD_CH_0;
#endif
    ret = tvd_internal_ioctl(TVD_CONTROL_CODE_STOP, (u8 *)&tvd_stop_param, NULL);
    tvd_unregister_notify();
    return ret;
}

static HANDLE  wch_backcar_event   = NULL;

BOOL BCAllocateResource()
{
    //in BCReleaseResource we first HideBackCarOverlay then disable WCH irq. SO we need this variable
    //used in arm2_tvd_frame_handle to avoid enque frame to VDP after HideBackCarOverlay.
    g_fFrameHandleStop = FALSE;  

    // used for printing the first frame log. 
    // Must set it to true here(in BCAllocateResource) in order to print first frame EVERY time backcar ON
    is_first_buffer = true;

    // init backcar UI osd, will show car image(for 8317 M) or track(for 8317 Linux) when this function return.
    // pay attention to the forth args of BackCarOverlayInit. 
    // here it is FALSE, so we must set a valid address for the first args of BackCarOverlayInit
    // Another example of BackCarOverlayInit is in signal_ready_process. it is used to init VDP
    BackCarOverlayInit(backcarUI_base, NULL, NULL, FALSE, FALSE, FALSE);

    #if BACK_CAR_SRC_YPBPR
    g_u4SrcType = SRC_YBR;
    Printf("BCAllocateResource:Config SRC_YBR\r\n");
    vDrvVideoResume();
    #elif BACK_CAR_SRC_VGA
    g_u4SrcType = SRC_VGA;
    Printf("BCAllocateResource:Config SRC_VGA\r\n");
    vDrvVideoResume();
    #else
    Printf("BCAllocateResource:Config SRC_TVD\r\n");
    Printf("consume time is %ums when register tvd irq process in arm2\n", GetBootTime());
    init_tvd();

#ifdef CONFIG_ATC_PLATFORM_ac83xx
    if (wch_backcar_event = X_CreateEvent(WCH_BACKCAR_EVENT))
    {
        Printf("create wch_backcar event\n");
        ((PARM2_EVT_T)wch_backcar_event)->pfEvtCallBack =  arm2_tvd_frame_handle;
    } else {
        Printf("Fail to create tvd frame done event\r\n");
    }
#endif

    #endif

    WCHInit();

    #if (BACK_CAR_SRC_YPBPR || BACK_CAR_SRC_VGA)
     v_enable_bim_irq(VECTOR_YPBPRINT);
    #else
    v_enable_bim_irq(VECTOR_VDOIN);
    #endif

#ifdef CONFIG_ATC_PLATFORM_ac823x
    v_enable_bim_irq(VECTOR_PMXIRQ);
#else
    v_enable_bim_irq(VECTOR_DDMANEW);
    v_enable_bim_irq(VECTOR_VSYNC);
    v_enable_bim_irq(VECTOR_PANEL_SCALER);
    v_enable_bim_irq(VECTOR_WCHNL);
    v_enable_bim_irq(VECTOR_WCHNL2);
#endif

    #if (BACK_CAR_SRC_YPBPR || BACK_CAR_SRC_VGA)
    g_fgYBRVGAInit = TRUE;
    #endif

    return TRUE;
}

BOOL BCReleaseResource()
{
    g_fFrameHandleStop = TRUE;

#ifdef CONFIG_ATC_PLATFORM_ac83xx
    UINT32 u4Context = 1;
    UINT32 u4CtlCode = IOCTL_WCH_STOP;
    Printf("BCReleaseResource: enter\r\n");
    #if BACK_CAR_SRC_YPBPR
    g_srcAppId = SRC_APP_YPBPR;
    #elif BACK_CAR_SRC_VGA
    g_srcAppId = SRC_APP_VGA;
    #else
    g_srcAppId = SRC_APP_BACKCAR;
    #endif
#endif
    //if (!g_fArm1Ready)
    {
        HideBackCarOverlay(TRUE);
        #if (BACK_CAR_SRC_YPBPR ||BACK_CAR_SRC_VGA)
        v_disable_bim_irq(VECTOR_YPBPRINT);
        #else
        v_disable_bim_irq(VECTOR_VDOIN);
        #endif

#ifdef CONFIG_ATC_PLATFORM_ac823x
        v_disable_bim_irq(VECTOR_PMXIRQ);
#else
        v_disable_bim_irq(VECTOR_VSYNC);
        v_disable_bim_irq(VECTOR_PANEL_SCALER);
        v_disable_bim_irq(VECTOR_DDMANEW);
        v_disable_bim_irq(VECTOR_WCHNL);
        v_disable_bim_irq(VECTOR_WCHNL2);
#endif


#ifdef CONFIG_ATC_PLATFORM_ac823x
        if(g_fWchStatus){
            if ( WCH_SUCCESS != CloseWch(WCH_5, SRC_APP_BACKCAR_WCH5)) {
                Printf("close: wch close Failed! \r\n");
            }
            g_fWchStatus = FALSE;
        }

#else
        if(WchIoControl(u4Context, u4CtlCode, &g_srcAppId, sizeof(g_srcAppId),
            &g_u4WchId, sizeof(UINT32), NULL))
        {
            Printf("start: wch stop fail! \r\n");
        }

        u4CtlCode = IOCTL_WCH_CLOSE;
        if(WchIoControl(u4Context, u4CtlCode, &g_srcAppId, sizeof(g_srcAppId),
            &g_u4WchId, sizeof(UINT32), NULL))
        {
            Printf("start: wch close fail! \r\n");
        }
#endif

#if (BACK_CAR_SRC_YPBPR ||BACK_CAR_SRC_VGA)    //need add for SRC_YPBPR
        vDrvVideoSuspend();
        g_fgYBRVGAInit = FALSE;
#else
        stop_tvd();
#endif

    }
#ifdef CONFIG_ATC_PLATFORM_ac83xx
    ARM2PCM_Close();
#endif
    Printf("BCReleaseResource: Leave\r\n");
    return TRUE;
}

BOOL BCWaitInitFin()
{
    return TRUE;
}

BOOL BCPlaySound(void* psndmem, UINT32 u4sndsz)
{
#ifdef CONFIG_ATC_PLATFORM_ac83xx
   return PlaySound(psndmem, u4sndsz);
#endif
}

DWORD BCCustomEntry();
VOID  BCCustomInit();

BOOL ShowBootingLogo()
{
    return TRUE;
}

BOOL fgArm1Ready()
{
    return g_fArm1Ready;
}

#ifdef CONFIG_ATC_PLATFORM_ac823x
#define PDWNC_BASE 0x10024000
#endif
UINT32 BackCar_StateMach()
{
    unsigned int u4pdwncmode = 0;
    u4pdwncmode = IO_READ32(PDWNC_BASE, 0x160);
    //Printf("BackCar_StateMach\r\n");

    if (u4pdwncmode == PDWNCMODE)
    {
        //Printf("Backcar read power mode normal %d\r\n", g_fArm1Ready);
        #ifdef ARM1_EXCEPTION_ARM2_BACKCAR_ENABLE
        BCCustomEntry();
        #else
        if (BCCustomEntry() == ARM2_BC_FOREVER_EXIT)
        {
            return TASK_IDLE; //TASK_IDLE means shit task should never been re-runned.
        }
        else
        {
            return TASK_BUSY; //TASK_BUSY means arm2 backcar quit periodically or normally.
        }
        #endif
    }
    else
    {
        Printf("Backcar read power mode suspend\r\n");
        return TASK_IDLE;
    }
}

UINT32 BackCar_Init()
{
    unsigned int u4pdwncmode = 0;
    Printf("[Arm2] start time - BackCar init+ : %d\n", GetBootTime());
    u4pdwncmode = IO_READ32(PDWNC_BASE, 0x160);

    if (u4pdwncmode == PDWNCMODE)/*normal boot*/ {
        g_fArm1Ready = FALSE;
        BCCustomInit();
#ifdef ATC_AOSP_EHANCEMENT_ATCATE
       //pull low
       Printf("mcu_test: pull low:pin %d --> %d, at %d \r\n",BACKCAR_NOTIFY_MCU_GPIO_PIN,BACKCAR_NOTIFY_MCU_GPIO_LOW,GetARM2TickCount());
       if(0 != GPIO_MultiFun_Set(BACKCAR_NOTIFY_MCU_GPIO_PIN, PINMUX_LEVEL_GPIO_END_FLAG)){
           Printf("mcu_test: GPIO_MultiFun_Set fail \r\n");
       }
       if( 0 > gpio_direction_output(BACKCAR_NOTIFY_MCU_GPIO_PIN, BACKCAR_NOTIFY_MCU_GPIO_LOW)){
           Printf("mcu_test: gpio_direction_output fail \r\n");
       }
       Printf("mcu_test: pin %d == %d \r\n",BACKCAR_NOTIFY_MCU_GPIO_PIN,ac83xx_gpio_get_value_reg(BACKCAR_NOTIFY_MCU_GPIO_PIN));
#endif

        /*Printf("Back Car Init\r\n");*/
    } else {/*quickboot */
        g_fArm1Ready = TRUE;
    }

    // Move from ARM1 aud_config -- config aout automute
    // Because of GPIO162 maybe used as other function by customer.
#ifdef CONFIG_ATC_PLATFORM_ac83xx
    vAudMuteCircuitCtrl(FALSE);
#endif
    Printf("ARM2_BackCar_Init - \r\n");
    return 0;
}

UINT32 BackCar_Callback(UINT32 u4ModuleID, UINT32 u4Param1, UINT32 u4Param2, UINT32 u4Param3)
{
    UINT32 u4lenth;
    UINT32 u4MessageID = GETMESSAGEID(u4ModuleID);
    
    if (MSG_ANDROID_APP_READY == u4MessageID)
    {
        Printf("ARM2 Detect Android Ready\r\n");
        if (!g_fgIsArm2BCTaskExist) {
            Printf("BackCar_Callback(): ARM2 Backcar checking Thread is not exist So Inform Arm1 immediately\r\n");
            NotifyARM1(MSG_ARM2_RESPONSE,ARM2_STATUS_NO_BACK_CAR);
        }
        g_fArm1Ready = TRUE;
    }
    else if (MSG_ANDROID_GET_MCU_INFO == u4MessageID)
    {
        Printf("MSG_ANDROID_GET_MCU_INFO share memory phy address 0x%x\r\n", u4Param1);
        HWSendMessage(MSG_COMBINE(MODULE_BCAR, MSG_ARM2_RESPONSE), 0, 0, 0);
    }
    else if (MSG_GET_ARM2_VERSION == u4MessageID)
    {
        Printf("MSG_GET_ARM2_VERSION share memory phy address 0x%x\r\n", u4Param1);
        u4lenth = sizeof(ARM2_VERSION);
        memcpy((0xAC000000 + u4Param1 + u4Param2), (const void *)ARM2_VERSION, u4lenth);
        HWSendMessage(MSG_COMBINE(MODULE_BCAR, MSG_ARM2_RESPONSE), u4lenth, 0, 0);
    }
    else if (MSG_NOTIFY_ARM2_STOP == u4MessageID)
    {
        Printf("ARM2 stop tvd\r\n");
        HideBackCarOverlay(TRUE);
#ifdef CONFIG_ATC_PLATFORM_ac83xx
        ARM2PCM_Close(); //add
#endif
#if (BACK_CAR_SRC_YPBPR ||BACK_CAR_SRC_VGA)
        v_disable_bim_irq(VECTOR_YPBPRINT);
        vDrvVideoSuspend();
        g_fgYBRVGAInit = FALSE;
#else
        v_disable_bim_irq(VECTOR_VDOIN);
        stop_tvd();
#endif
        v_disable_bim_irq(VECTOR_VSYNC);
        v_disable_bim_irq(VECTOR_PANEL_SCALER);
        *(UINT32 *)0xFD0000B4 = ((*(UINT32 *)0xFD0000B4) & 0XFFFFFFF9);
        *(UINT32 *)0xFD0000D0 = ((*(UINT32 *)0xFD0000D0) & 0XFFFFFFFD);
    }
    else if(MSG_NOTIFY_ARM2_BUFF_MEMSET == u4MessageID)
    {
        BufMemset();
    }
    else if(MSG_NOTIFY_ARM2_ANIMATION_STOP == u4MessageID)
    {
        Printf("Eboot Notify ARM2 stop ANIMATION\r\n");
    }

#ifdef ARM1_EXCEPTION_ARM2_BACKCAR_ENABLE
    else if (MSG_ARM1_ACK == u4MessageID)
    {
        g_arm1_ack = TRUE;
    }
    else if (MSG_ARM1_BC_STARTED_SUCCESS == u4MessageID) {
        Printf("have receive Arm1 backcar start success message\n");
        g_arm1_bc_success = TRUE;
    }
    else if (MSG_ARM1_BC_STARTED_FLAG_RESET == u4MessageID) {
        Printf("Arm1 backcar have stop, so reset the flag\n");
        g_arm1_bc_success = FALSE;
    }
#endif

    return(0);
}

extern UINT32 ARM2PCMStop();

