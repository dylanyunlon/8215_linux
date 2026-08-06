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
#include "BCLib.h"
#include "custom_protocol.h"
#include <generated/atc_project.h>
//#ifdef ARM1_EXCEPTION_ARM2_BACKCAR_ENABLE
#include "dual_task.h"
//#endif
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#include "arm2pcmplay.h"
#endif
#if ARM2_TRACK_ENABLE
#include "track.h"
#endif
#define START_PACKET_SIGNATURE (0xEF)
//#define UART4_ENABLE (1)
#define GPIO_ENABLE (1)

#ifdef CONFIG_ATC_PLATFORM_ac823x
#include "ac823x_pinmux_table.h"
#define GPIOPIN_NUM  (PIN_197_GPIO7)
#else
#include "ac83xx_pinmux_table.h"
#include "gpio.h"
#define GPIOPIN_NUM  (131)
#endif

#define GPIO_HIGH	 (1)//backcar off(or need to stop baccar)
#define GPIO_LOW	 (0)//backcar on(or need to start backcar)
#define PCM_PLAY_ENABLE (0)
#define GET_SIGNAL_TIME_OUT          100

static UINT16 *g_DateBuffer[50];
BOOL s_fgBackCarStart = FALSE;
extern void  DisplayLighten(void);


extern g_fgIsArm2BCTaskExist;

#ifdef ARM1_EXCEPTION_ARM2_BACKCAR_ENABLE
extern BOOL g_arm1_ack;
extern BOOL g_arm1_bc_success;
static BOOL arm1_is_alive = TRUE;

#define ARM1_ACK_TIME_OUT                   100
#define ARM1_BACKCAR_START_TIME_OUT         1300
#endif

BOOL SendAck(UINT32 u4Satus)
{
    TransferStatus  TS;

    TS.u4Status = u4Satus;

    BCUartWriteBlockData(&TS, sizeof(TransferStatus));

    return TRUE;
}

BOOL OnDistance(UINT32 u4Dis)
{
    ShowDistance(u4Dis);

    return TRUE;
}

#ifdef ARM1_EXCEPTION_ARM2_BACKCAR_ENABLE
static BOOL Check_Arm1_Is_Alive()
{
    BOOL ret = FALSE;
    UINT32 start_time = 0;
    UINT32 end_time = 0;

    if (FALSE == arm1_is_alive) {
        return ret;
    }

    g_arm1_ack = FALSE;
    HWSendMessage(MSG_COMBINE(MODULE_BCAR, MSG_QUERY_ARM1_ISALIVE), 0, 0, 0);
    start_time = GetARM2TickCount();
    do {
        end_time = GetARM2TickCount();
    } while ((g_arm1_ack == FALSE) && (end_time - start_time) <= ARM1_ACK_TIME_OUT);

    if (g_arm1_ack == TRUE) {
        ret = TRUE;
    } else {
        arm1_is_alive = FALSE;
        Printf("Arm1 is not alive\n");
    }
    return ret;
}

static BOOL Check_Arm1_BC_Is_Success()
{
    BOOL ret = FALSE;
    UINT32 start_time = 0;
    UINT32 end_time = 0;
    int gpio_status = GPIO_HIGH;

    start_time = GetARM2TickCount();
    do {
        /*if the backcar exit in the specified timeout, we no loger detect*/
        gpio_status = BCGetGPIOVal();
        if (gpio_status == GPIO_HIGH) {
            Printf("Arm1 backcar start success and now it exit\n");
            return TRUE;
        }
        end_time = GetARM2TickCount();
    } while ((g_arm1_bc_success == FALSE) && (end_time - start_time) <= ARM1_BACKCAR_START_TIME_OUT);

    if (g_arm1_bc_success == TRUE) {
        ret = TRUE;
    } else {
        Printf("Arm1 backcar failed\n");
    }
    return ret;
}

static BOOL Start_Backcar()
{
    BOOL ret = FALSE;

    g_fgIsArm2BCTaskExist = TRUE;
    ret = CustomUIInit();
    if (FALSE == ret) {
        Printf("custom ui init failed\n");
        g_fgIsArm2BCTaskExist = FALSE;
        return ret;
    }

    ret = BCAllocateResource();
    if (FALSE == ret) {
        Printf("start backcar failed\n");
        g_fgIsArm2BCTaskExist = FALSE;
        return ret;
    }
    return ret;
}

static BOOL Stop_Backcar()
{
    BOOL ret = FALSE;

    ret = BCReleaseResource();
    if (FALSE == ret) {
        Printf("stop back car failed\n");
        return ret;
    }
    g_fgIsArm2BCTaskExist = FALSE;
    return ret;
}

BOOL BCCustomEntry()
{
    int gpio_status = GPIO_HIGH;
    BOOL arm1_ready = FALSE;
    BOOL arm1_is_alive = FALSE;
    BOOL arm1_backcar_Success = FALSE;
    BOOL ret = FALSE;

    BCGPIOInit(GPIOPIN_NUM);
    while(TRUE) {
        gpio_status = BCGetGPIOVal();

        /*when gpio is  high, it main that we want to stop backcar or it is in initial state */
        if (GPIO_HIGH == gpio_status) {
            if (g_fgIsArm2BCTaskExist) {
                Stop_Backcar();
                arm1_is_alive = Check_Arm1_Is_Alive();
                if (arm1_is_alive) {
                    NotifyARM1(MSG_ARM2_RESPONSE, ARM2_STATUS_NO_BACK_CAR);
                }
            }
        } else {
            if (!g_fgIsArm2BCTaskExist) {
                arm1_ready = fgArm1Ready();
                if (!arm1_ready) {
                    Start_Backcar();
                } else {
                    arm1_is_alive = Check_Arm1_Is_Alive();
                    if (!arm1_is_alive) {
                        Printf("the arm1 is not alive,so start arm2 backcar\n");
                        Start_Backcar();
                    } else {
                        arm1_backcar_Success = Check_Arm1_BC_Is_Success();
                        if (!arm1_backcar_Success) {
                            /*It is necessary to check the gpio status again, because the gpio state may change after the delay of 1 sec*/
                            gpio_status = BCGetGPIOVal();
                            if (GPIO_LOW == gpio_status) {
                                Printf("the arm1 is alive,but arm1 backcar start failed,so start arm2 backcar\n");
                                Start_Backcar();
                            }
                        } else {
                            msleep(2);
                        }
                    }
                }
            }
        }
    }
}

#else
#ifdef CONFIG_ATC_PLATFORM_ac823x
extern void enableArm2LogoUI(void);
#endif

void BCCustomInit(){
#ifdef CONFIG_ATC_PLATFORM_ac83xx
    ARM2PCM_Init(AUD_TWO_SPEAKERS);
#endif

#if UART4_ENABLE
        BCUartInit(4, 115200);
#endif

#if GPIO_ENABLE
    BCGPIOInit(GPIOPIN_NUM);
    #if ARM2_TRACK_ENABLE
    // track module need uart
    BCUartInit(4, 115200);
    #endif
#endif
}

DWORD BCCustomEntry()
{
    UINT32 start_time = 0;
    UINT32 end_time = 0;
    static BOOL first_enter = FALSE;
    if(!first_enter) {
        Printf("BCCustomEntry enter and consume time is %usms\n", GetBootTime());
        first_enter = TRUE;
    }

    g_fgIsArm2BCTaskExist = TRUE;
#if UART4_ENABLE
    PCMDPacket prFscPacket = NULL;
    static DWORD s_u4PrevSyncCount = (DWORD)-1;
    UINT16 u2Dis = -1;
#endif

#if GPIO_ENABLE
    BCGPIOInit(GPIOPIN_NUM);
    int LastGPIOVal = GPIO_HIGH;//BCGetGPIOVal();
    int CurGPIOVal = GPIO_HIGH;//BCGetGPIOVal();
    #if ARM2_TRACK_ENABLE
    // track module need uart
    PCMDPacket prFscPacket = NULL;
    static DWORD s_u4PrevSyncCount = (DWORD)-1;
    #endif
#endif//GPIO_ENABLE

#if UART4_ENABLE

    start_time = GetARM2TickCount();
    while(TRUE)
    {
        if(fgArm1Ready() && !s_fgBackCarStart)
        {
            Printf("BCCustomEntry(): ARM1 is ready, close arm2 backcar!\r\n");
            NotifyARM1(MSG_ARM2_RESPONSE, ARM2_STATUS_NO_BACK_CAR);
            g_fgIsArm2BCTaskExist = FALSE;
            return ARM2_BC_FOREVER_EXIT;
        }

        end_time = GetARM2TickCount();
        if (!s_fgBackCarStart && (end_time - start_time) >= GET_SIGNAL_TIME_OUT)
        {
            return ARM2_BC_NEED_RE_ENTER;
        }

        prFscPacket = GetUARTPacket();

        if (!prFscPacket
            || MAIN_FUNC_BACKCAR != prFscPacket->uMainFunc
            || s_u4PrevSyncCount == prFscPacket->u4CmdSync)
        {
            continue;
        }

        switch(prFscPacket->uSubFunc)
        {
        case SUBFUNC_BACKCAR_START:
            if (!s_fgBackCarStart)
            {
                Printf("[BackCar]Start backcar.\r\n");
                s_fgBackCarStart = TRUE;
                CustomUIInit();
                #if ARM2_TRACK_ENABLE
                BCTrackInit();
                #endif
                BCAllocateResource();
                DisplayLighten(); /*Light the screen */
            }
            break;
        case SUBFUNC_BACKCAR_STOP:
            if (s_fgBackCarStart)
            {
                Printf("[BackCar]Stop backcar.\r\n");
                s_fgBackCarStart = FALSE;
                #if ARM2_TRACK_ENABLE
                BCTrackUnInit();
                #endif
                BCReleaseResource();
            }
            break;
        case SUBFUNC_BACKCAR_DISTANCE:
            if (s_fgBackCarStart)
            {
                u2Dis = prFscPacket->u2Data;
                Printf("[BackCar] OnDistance with para 0x%x \r\n",prFscPacket->u2Data);
                OnDistance(prFscPacket->u2Data);
            }
            break;
        #if ARM2_TRACK_ENABLE
        case SUBFUNC_BAKCCAR_TURNLEFT:
        case SUBFUNC_BAKCCAR_TURNRIGHT:
            if(s_fgBackCarStart){
                if( 0 != BCTrackUpdate(prFscPacket)){
                    Printf("[BackcarTrack] error: BCTrackUpdate fail ! \n");
                }
            }
            break;
        #endif
        default:
            break;
        }

        if (RET_IGNORE != prFscPacket->uRetType)
        {
            BCDoAck(prFscPacket, sizeof(CMDPacket));
        }

        s_u4PrevSyncCount = prFscPacket->u4CmdSync;
    }

#endif

#if GPIO_ENABLE
    while (TRUE)
    {
        //************************ Read Start Status Start ****************************
        start_time = GetARM2TickCount();
        do {
            if (fgArm1Ready())
            {
                Printf("BCCustomEntry(): arm1 is ready, close arm2 backcar!\r\n");
                NotifyARM1(MSG_ARM2_RESPONSE, ARM2_STATUS_NO_BACK_CAR);
                g_fgIsArm2BCTaskExist = FALSE;

                return ARM2_BC_FOREVER_EXIT;
            }
            end_time = GetARM2TickCount();
            if ((end_time - start_time) >= GET_SIGNAL_TIME_OUT)
            {
                //Printf("BCCustomEntry(): Quit arm2 backcar periodically! Interval time = %d\r\n", (end_time - start_time));
                return ARM2_BC_NEED_RE_ENTER;
            }

            //receive the message from GPIO 131  that can start backcar
            CurGPIOVal = BCGetGPIOVal();
#if ((defined CONFIG_ATC_OS_linux) && (defined DISABLE_BACKCAR_IN_UPGRADE_MODE))
            if (fgDualHALGetUpgradeMode())
            {
                CurGPIOVal = LastGPIOVal;//ATC0031 diable backcar in upgrade mode
            }
#endif

#ifdef CONFIG_ATC_PLATFORM_ac823x
            if (CurGPIOVal == LastGPIOVal)
                enableArm2LogoUI();
#endif
        } while(CurGPIOVal == LastGPIOVal);

        Printf("BCCustomEntry(): BCAllocateResource, CurGPIOVal is %d\r\n", CurGPIOVal);
#ifdef CONFIG_ATC_PLATFORM_ac83xx
    Printf("mcu_test:BCCustomEntry get start backcar event at: %d \n",GetARM2TickCount());
    Printf("consume time is %usms\n", GetBootTime());
#endif
        CustomUIInit();
        #if ARM2_TRACK_ENABLE
        BCTrackInit();
        #endif
        BCAllocateResource();
        DisplayLighten(); /*Light the screen */
        Printf("display light have enabled and consume time %ums\n", GetBootTime());
#ifndef CONFIG_ATC_PLATFORM_ac823x
        ARM2PCM_Open();
#endif
        //************************ Read Start Status end****************************
        //************************ Read Runing Distance start****************************
        while (TRUE)
        {
            //Sleep(100);
            CurGPIOVal = BCGetGPIOVal();
            if (CurGPIOVal == LastGPIOVal) {
                Printf("BCCustomEntry(): stop arm2 backcar CurGPIOVal %d!\r\n", CurGPIOVal);
                break;//notiStatu.u4Status = BACKCAR_STOP;
            } else if (CurGPIOVal != LastGPIOVal) {
                #if ARM2_TRACK_ENABLE
                // will not block, if no packet, prFscPacket will be NULL
                prFscPacket = GetUARTPacket();
                if (!prFscPacket){
                    // happens always, so do not log here
                }else if( MAIN_FUNC_BACKCAR != prFscPacket->uMainFunc){
                    //Printf("[BackcarTrack] info: GPIO RECV packet, no backcar! \n");
                }else if( s_u4PrevSyncCount == prFscPacket->u4CmdSync){
                    //Printf("[BackcarTrack] info: GPIO RECV packet, backcar, previous packet! \n");
                }else{
                    // track related packet , update track
                    Printf("[BackcarTrack] info: GPIO RECV packet, backcar, new packet! \n");
                    if(SUBFUNC_BAKCCAR_TURNLEFT == prFscPacket->uSubFunc ||SUBFUNC_BAKCCAR_TURNRIGHT == prFscPacket->uSubFunc){
                        if( 0 != BCTrackUpdate(prFscPacket)){
                            Printf("[BackcarTrack] error: BCTrackUpdate fail ! \n");
                        }
                     }
                     // do ack
                    if (RET_IGNORE != prFscPacket->uRetType)
                    {
                        BCDoAck(prFscPacket, sizeof(CMDPacket));
                    }
                    s_u4PrevSyncCount = prFscPacket->u4CmdSync;
                }
                #endif

                #if PCM_PLAY_ENABLE
                PlayToneSound();
                #endif
            }
        }
        //************************ Read Runing Distance End Backcar Stop****************************
        #if ARM2_TRACK_ENABLE
        BCTrackUnInit();
        #endif
        BCReleaseResource();
        Printf("BCCustomEntry(): arm2 backcar has been stopped!\r\n");
        #ifdef ATC_AOSP_EHANCEMENT_ATCATE
        //pull low
        Printf("mcu_test: pull low:pin %d --> %d, at %d \r\n",BACKCAR_NOTIFY_MCU_GPIO_PIN,BACKCAR_NOTIFY_MCU_GPIO_LOW,GetARM2TickCount());
        if(0 != GPIO_MultiFun_Set(BACKCAR_NOTIFY_MCU_GPIO_PIN, PINMUX_LEVEL_GPIO_END_FLAG)){
		     Printf("mcu_test: GPIO_MultiFun_Set fail \r\n");
        }
        if( 0 > gpio_direction_output(BACKCAR_NOTIFY_MCU_GPIO_PIN, BACKCAR_NOTIFY_MCU_GPIO_LOW)){
             Printf("mcu_test: gpio_direction_output fail \r\n");
        }
        Printf("mcu_test: pxn %d == %d \r\n",BACKCAR_NOTIFY_MCU_GPIO_PIN,ac83xx_gpio_get_value_reg(BACKCAR_NOTIFY_MCU_GPIO_PIN));
        #endif
        if(fgArm1Ready()) {
            Printf("BCCustomEntry(): arm2 backcar has been stopped and tooken by arm1!\r\n");
            NotifyARM1(MSG_ARM2_RESPONSE,ARM2_STATUS_NO_BACK_CAR);
            g_fgIsArm2BCTaskExist = FALSE;
            return ARM2_BC_FOREVER_EXIT;
        }
    }
#endif

    Printf("BCCustomEntry(): Exit!\r\n");
    g_fgIsArm2BCTaskExist = FALSE;
    return ARM2_BC_NEED_RE_ENTER;
}
#endif

