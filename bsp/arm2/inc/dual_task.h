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

#ifndef DUAL_TASK_H
#define DUAL_TASK_H

//#include "x_typedef.h"

//#include "drv_dual.h"

//============================================================================
// Type definitions
//============================================================================
typedef UINT32 (*PFN_Init)(void);
typedef UINT32 (*PFN_StateMach)(void);
typedef UINT32 (*PFN_Callback)(UINT32 u4MsgID, UINT32 u4Param1, UINT32 u4Param2, UINT32 u4Param3);
typedef UINT32 (*PFN_Timer)(void);

#define CFG_CLOCK_PER_TICKS (27000)


#define HW_SEM_MSG_UART      (1U<<0)  //Debugging UART0    
#define HW_SEM_BACKCAR_UART  (1U<<1)  //Back Car UART3
#define HW_SEM_BACKCAR_OSD   (1U<<2)  // Back Car OSD3
typedef enum 
{
	MODULE_AEC,
	MODULE_BCAR,
	MODULE_BOOTANIMATION,
	MODULE_ARM2SYSTEMSERVICE,
	MODULE_MAX,
}module_t;
#define MODULE_TEST  (0x0)
#define MODULE_SMEM  (0x0)	// Share Memory
#define MOUDLE_REV2  (0x1)

//arm2 sleep status
#define TASK_IDLE    (0x1)
#define TASK_BUSY    (0x2)

#define DIM(m, n) (sizeof(m)/sizeof(n))

//#define ARM1PHY2ARM2UCV(m) (m - g_u4Arm2MemroyOffset + 0xC0000000)
//#define ARM2UCV2ARM1PHY(m) (m + g_u4Arm2MemroyOffset - 0xC0000000)
extern UINT32 ARM1PHY2ARM2UCV(UINT32 u4ARM1phyadress);
extern UINT32 ARM2UCV2ARM1PHY(UINT32 u4ARM2UCVAdress);

//#define MSG_COMBINE(module, messdir, id) ((module<<24)|(messdir<<16)|(id))
#define MSG_COMBINE(module, id) ((module<<24)|(id))

typedef struct
{
  UINT32        u4MoudleID;
  PFN_Init      pfn_Init;
  PFN_StateMach pfn_StateMach;
  PFN_Callback  pfn_Callback;
} TASK_DEFINE_T;

typedef struct
{
  UINT32 u4Interval; //1ms
  UINT32 u4IntervalBack;  //same with u4Interval
  PFN_Timer  pfn_Timer;
} TIMER_DEFINE_T;


/*
EXTERN void vDualIsr(void);
EXTERN BOOL fgDualCallBackInit(void);

EXTERN BOOL fgRegisterCallBack(UINT32 u4MoudleID, void (*DUAL_CALLBACK)(UINT32 u4MoudleID, UINT32 u4P1, UINT32 u4P2, UINT32 u4P3));
EXTERN BOOL fgReleaseCallBack(UINT32 u4MoudleID);
*/

EXTERN void MainLoop(void);
EXTERN void TimerLoop(void);
EXTERN void vDualIsr(void);
EXTERN UINT64 GetHiTimerTick(void);
EXTERN UINT32 GetHiTimerFrequency(void);
EXTERN UINT64 GetSysFrequery(VOID);
EXTERN UINT64 GetSysTick(VOID);
EXTERN UINT32 GetMicrosecond (VOID);
EXTERN UINT32 GetMinisecond (VOID);
EXTERN UINT32 GetARM2TickCount(VOID);

typedef unsigned long long ULONGLONG;

#define MICROSECOND 1000000ull
#define MINISECOND  1000ul


//DEBUGMSG(Condition, Printf_expr);
//RETAILMSG(Condition, Printf_expr);


#endif  // DUAL_TASK_H
