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

//============================================================================
// Include files
//============================================================================
#include <FreeRTOS.h>
#include <task.h>
#include "x_bim.h"
#include "x_printf.h"
#include "dual_callback.h"
#include "dual_hal.h"
#include "dual_task.h"
#include "backcar_cfg.h"
#include "share_memory.h"
#include "arm2system_service.h"

//============================================================================
// Config settings
//============================================================================

//============================================================================
// Constant definitions
//============================================================================

//============================================================================
// Static variables
//============================================================================


//============================================================================
// Static functions
//============================================================================


//============================================================================
// Public functions
//============================================================================



UINT32 Test_Timer20ms(void)
{
    unsigned long time = (UINT32)GetARM2TickCount();
    //Printf("Test_Timer20ms %d\n", time);

    return(0);
}

#if 0
UINT32 Test_YBRVGA10ms(void)
{
    u4DrvVideoMainLoop();
    return 0;
}
#endif

UINT32 TEST_Init(void)
{
    HWSendMessage(MSG_COMBINE(MODULE_TEST, 2), 0, 0xAAAAAAAA, 0xBBBBBBBB);

    return(0);
}

UINT32 TEST_StateMach(void)
{
    return(TASK_IDLE);
}

static UINT32 u4TestCount = 0;
UINT32 TEST_Callback(UINT32 u4ModuleID, UINT32 u4Param1, UINT32 u4Param2, UINT32 u4Param3)
{
    Printf("TEST_Callback %d\n", u4Param1);
    if (u4TestCount < 10)
    {
        HWSendMessage(MSG_COMBINE(MODULE_TEST, 2), u4Param1 + 1, u4Param2, u4Param3);
        u4TestCount++;
    }
    else
    {
        HWSendMessage(MSG_COMBINE(MODULE_TEST, 88), u4Param1 + 1, u4Param2, u4Param3);
        Printf("TEST_Callback send 88\n");
    }

    return(0);
}

//-----------------------------------------------------
UINT32 TEST_ShareMemory_Init(void)
{
    HWSendMessage(MSG_COMBINE(MOUDLE_REV2, 22), 0, 0, 0);

    return(0);
}

UINT32 TEST_ShareMemory_StateMach(void)
{
    return(TASK_IDLE);
}

UINT32 TEST_ShareMemory_Callback(UINT32 u4ModuleID, UINT32 u4Param1, UINT32 u4Param2, UINT32 u4Param3)
{
    UINT32 shareMemroyVA;
    int i;

    Printf("TEST_ShareMemory_Callback %d\n", u4Param1);

    shareMemroyVA = ARM1PHY2ARM2UCV(u4Param1);

    for (i = 0; i < 1024; i++)
    {
        if (((UINT32 *)shareMemroyVA)[i] != i)
        {
            Printf("TEST_ShareMemory_Callback test failed! \n");
            return(0);
        }
    }
    Printf("TEST_ShareMemory_Callback test successful! \n");

    return(0);
}

extern void v_disable_bim_irq(UINT32 u4Id);
extern void EnterSleepMode(void);
extern void v_enable_bim_irq(UINT32 u4Id);

extern UINT32 SpeechCB(UINT32 u4MsgID, UINT32 u4Param1, UINT32 u4Param2, UINT32 u4Param3);
extern UINT32 SpeechInit(VOID);
extern UINT32 SpeechStateMachine(VOID);
extern UINT32 BackCar_StateMach();
extern UINT32 BackCar_Init();
extern UINT32 BackCar_Callback(UINT32 u4ModuleID, UINT32 u4Param1, UINT32 u4Param2, UINT32 u4Param3);

// ARM2 ShareMemory
extern UINT32 ShareMemory_StateMach();
extern UINT32 ShareMemory_Init();
extern UINT32 ShareMemory_Callback(UINT32 u4ModuleID, UINT32 u4Param1, UINT32 u4Param2, UINT32 u4Param3);

extern UINT32 BootAnimation_StateMach();
extern UINT32 BootAnimation_Init();
extern UINT32 BootAnimation_Callback(UINT32 u4ModuleID, UINT32 u4Param1, UINT32 u4Param2, UINT32 u4Param3);
//awtk
extern void Awtk_SysTick_Timer_IRQ_Handler(void);
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
static TASK_DEFINE_T sg_rTasks[] =
{
    {MODULE_BOOTANIMATION, NULL, NULL, BootAnimation_Callback},
   	{MODULE_ARM2SYSTEMSERVICE, NULL, NULL, arm2system_service_callback}
};

//extern UINT32 Tvd_Timer320ms(void);
static TIMER_DEFINE_T sg_rTimers[] =
{
    //{10, BackCar_Timer10ms},
    //{1, 1, Awtk_SysTick_Timer_IRQ_Handler},
#if (BACK_CAR_SRC_YPBPR || BACK_CAR_SRC_VGA)
    {10,10,Test_YBRVGA10ms},
#endif
   // {1,1,Tvd_Timer320ms}
};

BOOL fgIdle[ARRAY_SIZE(sg_rTasks)] = {0};

void MainLoop(void)
{
    int tasknum = ARRAY_SIZE(sg_rTasks);
    UINT32 _u4MiniSecond = 0;
    UINT32 u4Temp= 0;
    int i;
 	bool all_idle = TRUE;

    for (i = 0; i < tasknum; i++)
    {
		if (NULL != sg_rTasks[i].pfn_Init)
        	(sg_rTasks[i].pfn_Init)();
    }

    while (1)
    {
		vTaskDelay( 50 );
        // Printf(("[COM arm2] while, _u4MiniSecond(%d),u4Temp(%d)\r\n"), _u4MiniSecond, u4Temp);
        // Printf(("[COM arm2] while, tasknum(%d)\r\n"),tasknum);
        all_idle = TRUE;
        for (i = 0; i < tasknum; i++)
        {

  //          u4Temp = GetTickCount();
  //          if ((u4Temp - _u4MiniSecond) > 10000)
  //          {
  //              Printf(("[COM arm2] MainLoop\r\n"));
  //              _u4MiniSecond = u4Temp;
  //          }
			if (fgIdle[i] == FALSE)
			{
				if (NULL != sg_rTasks[i].pfn_StateMach)
				{
					if (TASK_IDLE == (sg_rTasks[i].pfn_StateMach)())
					{
						fgIdle[i] = TRUE;
					}
				}
			}
		}
		/*if (fgIdle[0])
		{
			EnterAwtkFlag();
		}*/

		for (i = 0; i < ARRAY_SIZE(sg_rTasks); i++)
		{
			if (FALSE == fgIdle[i])
			{
				all_idle = FALSE;
				break;
			}
		}
        if (TRUE == all_idle)
        {
            v_disable_bim_irq(VECTOR_T0);
            Printf("step Sleep! \r\n");
            EnterSleepMode();
            Printf("step Wakeup! \r\n");
            v_enable_bim_irq(VECTOR_T0);
        }

    }
}

static UINT32 _u4CurTick = 0;

void TimerLoop(void)
{
    int tasknum = DIM(sg_rTimers, sg_rTimers[0]);
    int i;

    _u4CurTick ++;
#if 0
    if (!(_u4CurTick % 1000))
    {
        Printf("TimerLoop(%d) 64bits Timer: High(0x%x): Low(0x%x).\r\n",
        _u4CurTick, T64B_GET_HIGH(), T64B_GET_LOW());
    }
#endif
    for(i = 0; i < tasknum; i++)
    {
        sg_rTimers[i].u4Interval -= 1;

        if(sg_rTimers[i].u4Interval == 0)
        {
            (sg_rTimers[i].pfn_Timer)();
            sg_rTimers[i].u4Interval = sg_rTimers[i].u4IntervalBack;
        }
    }
}

void vDualIsr(void)
{
    UINT32 u4Tmp1, u4Tmp2;
    UINT32 tasknum = DIM(sg_rTasks, sg_rTasks[0]);
    UINT32 u4MoudleID;
    UINT32 u4P1;
    UINT32 u4P2;
    UINT32 u4P3;

   // Printf("\r\n [COM arm2]vDualIsr Start ***!\n");

    for (u4Tmp1 = 0; u4Tmp1 < MODULE_MAX; u4Tmp1++)
    {
        if (HWGetMessage(u4Tmp1, &u4MoudleID, &u4P1, &u4P2, &u4P3) == FALSE)
        {
            //ASSERT(FALSE);
        }
   // 	Printf("\r\n [COM arm2]vDualIsr get msg :%x***!\n", u4MoudleID);

        if (GETMESSAGEDIR(u4MoudleID) != ARM1TOARM2)
        {
            continue;
        }

        for (u4Tmp2 = 0; u4Tmp2 < tasknum; u4Tmp2++)
        {
            if (sg_rTasks[u4Tmp2].u4MoudleID == GETMODULEID(u4MoudleID) && sg_rTasks[u4Tmp2].pfn_Callback != 0)
            {
                //Execute Callack
                (sg_rTasks[u4Tmp2].pfn_Callback)(u4MoudleID, u4P1, u4P2, u4P3);
                break;
            }
        }
    }
}

UINT64 GetHiTimerTick(void)
{
    UINT32 high = T64B_GET_HIGH();
    UINT32 low  = T64B_GET_LOW();

    while (high != T64B_GET_HIGH())
    {
        high = T64B_GET_HIGH();
        low = T64B_GET_LOW();
    }

    return((((UINT64)high << 32) | low));
}

UINT32 GetHiTimerFrequency(void)
{
    return(27000000);
}


UINT64 GetSysFrequery()
{
    return ((UINT64) GetHiTimerFrequency());
}


UINT64 GetSysTick()
{
    return (GetHiTimerTick());
}

UINT32 GetMinisecond (VOID)
{
    UINT64 u8Ticks = 0;
    u8Ticks = GetSysTick();
    u8Ticks = (UINT64)(u8Ticks  / (GetHiTimerFrequency()/1000));

    return (UINT32) u8Ticks;
}

UINT32 GetMicrosecond (VOID)
{
    UINT64 u8Ticks = 0;
    u8Ticks = GetSysTick();
    u8Ticks = (UINT64)(u8Ticks  / (GetHiTimerFrequency()/1000000));

    return (UINT32) u8Ticks;
}

UINT32 GetARM2TickCount(VOID)
{
    return _u4CurTick;
}

UINT32 GetBootTime(VOID)
{
#if defined(CONFIG_ATC_PLATFORM_ac83xx)
	return ((0xFFFFFFFF - (*((volatile uint32_t*)(0xF000814C))))/27000);
#elif defined(CONFIG_ATC_PLATFORM_ac823x)
	return ((0xFFFFFFFF - (*((volatile uint32_t*)(0x1000814C))))/27000);
#endif

}

UINT32 ARM1PHY2ARM2UCV(UINT32 u4ARM1phyadress)
{
    UINT32 arm2phyadress;

    configASSERT (u4ARM1phyadress >= g_u4Arm2MemroyOffset);

    arm2phyadress = u4ARM1phyadress - g_u4Arm2MemroyOffset;

    return arm2phyadress;
}

UINT32 ARM2UCV2ARM1PHY(UINT32 u4ARM2UCVAdress)
{
    UINT32 arm1phyadress;

    arm1phyadress = g_u4Arm2MemroyOffset + u4ARM2UCVAdress;
    configASSERT (arm1phyadress <= g_u4MemorySize);

    return (arm1phyadress);
}


