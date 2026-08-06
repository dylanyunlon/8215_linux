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

#ifndef DUAL_HAL_H
#define DUAL_HAL_H

//#include "x_typedef.h"
typedef struct
{
	unsigned int jump_instr;
	unsigned char unused[0x400 - 4];
	unsigned long dram_size;
	unsigned int upgrade_mode;
	unsigned int dtb_status;
	unsigned int system_index;
#if ATC_AB_PARTITION_SUPPORT
	unsigned int ab_slot;
	unsigned int cache_rsv[3];
#else
	unsigned int cache_rsv[4];
#endif
	unsigned int arm2_dtb_status;
#ifdef CONFIG1
	unsigned int board_type;
	unsigned int mcu_version;
#endif
} *ARGS_TO_ARM2_P;  /*args to arm2 from uboot/lk*/

typedef enum dtb_
{
	STATUS_WAIT_LOAD,
	STATUS_LOAD_READY,
	STATUS_MODIFY_END,
}dtb_status_t;


#define DUAL_DRAMB_OFFSET_ALIGNMENT  0x400;
#define MESSAGE_LAST_SOURCE_READY 0x1
#define MESSAGE_ANIMATION_STOP 0x2
#define MESSAGE_ANIMATION_PLAY_BOOT 0x3
#define MESSAGE_ANIMATION_PLAY_SHUTDOWN 0x4

#define MESSAGE_ANIMATION_REQUEST_VDEC 0x5
#define MESSAGE_ANIMATION_VDEC_FREE 0x6
#define MESSAGE_VDEC_HW_READY 0x7


EXTERN UINT32 g_u4Arm2MemroyOffset;
EXTERN UINT32 g_u4MemorySize;

EXTERN BOOL fgDualHALInit(void);
EXTERN BOOL fgDualHALStart(void);
EXTERN BOOL fgDualHALStop(void);
EXTERN BOOL fgDualHALINTEachOther(void);

EXTERN UINT32 fgDualHALGetOffset(void);

EXTERN UINT32 u4DualHALOffsetAlignment(void);

EXTERN BOOL fgDualHALSetRemap(void);

EXTERN BOOL fgDualHALSetBootUpParameter(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4);
EXTERN BOOL fgDualHALGetBootUpParameter(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4);

EXTERN BOOL fgDualHALSetSendCommandParameter(UINT32 u4P1, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4);

EXTERN BOOL fgDualHALGetReturnParameter(UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4);
EXTERN BOOL HWGetMessage(UINT32 u4ModuleID, UINT32 *pu4P1, UINT32 *pu4P2, UINT32 *pu4P3, UINT32 *pu4P4);
EXTERN BOOL HWSendMessage(UINT32 u4MessageHeader, UINT32 u4P2, UINT32 u4P3, UINT32 u4P4);

UINT32 fgDualHALSetDtbStatus(unsigned int status);
UINT32 fgSystemIndex(void);
UINT32 fgSlotSuffix(void);
UINT32 fgDualHALGetDtbStatus(void);
UINT32 fgDualHALGetUpgradeMode(void);
EXTERN void fgClearGroup(void);
#endif  // DUAL_HAL_H
