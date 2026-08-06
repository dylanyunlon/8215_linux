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

/** @file ddr_includes.h
 *      include all header file using in dramk.
 */

#ifndef DDR_INCLUDES_H
#define DDR_INCLUDES_H

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
//#include "drv_config.h"
#include "c_model.h"
#include "x_typedef.h"
#include "x_bim.h"
#include "x_ckgen.h"
#include "x_serial.h"
#include "x_printf.h"
#include "x_dram.h"
#include "mt85xx_dram.h"
#include "ddr.h"
#include "../drv_cust/drvcust_if.h" //for cust dram setting
#include "time.h"
 
/************************************************************/
#define CC_MTK_PRELOADER
//#define DRAM_DEBUG_MSG
//#define DRAM_DEBUG_MSG2



#if 1
// move from  ddr.h
#define mcSHOW_ERROR_CHIP_DisplayString 		Printf
#define mcSHOW_ERROR_CHIP_DisplayInteger(x)		Printf("%d", x)
#define mcSHOW_ERROR_CHIP_DisplayHex(x)			Printf("0x%x", x)

#ifdef DRAM_DEBUG_MSG
#define mcSHOW_DBG_MSG 	Printf
#else
#define mcSHOW_DBG_MSG(fmt...)
#endif

#ifdef DRAM_DEBUG_MSG2
#define mcSHOW_DBG_MSG2	Printf
#else
#define mcSHOW_DBG_MSG2(fmt...)	
#endif
#endif




#define LOG(level, fmt...)          \
do                                  \
{                                   \
    if (level <= DDR_GetLogLevel()) \
    {                               \
        Printf(fmt);                \
    }                               \
} while (0)

void DDR_Delay(UINT32 u4Micros);

//#define mcDELAY_us(x) DDR_Delay(x)
#define mcDELAY_us(x) TIM_DelayUS(x)
    
//============================================================================
//****************************************************************************
typedef struct DRAM_DESC
{
    UINT32 u4DramFlags;
    UINT32 u4DramSize;
    UINT32 u4DramDataWidth;
    UINT32 u4DramDlyCellPert;
    UINT32 u4DramFlags1; //for STR core off
    UINT32 u4DramFlags2;
    UINT32 u4DramFlags3;
    UINT32 u4DramFlags4;
}DRAM_DESC_T;

extern DRAM_DESC_T g_dram_desc;

#define TCM_DRAM_FLAGS    		(g_dram_desc.u4DramFlags)
#define TCM_DRAM_SIZE      		(g_dram_desc.u4DramSize)
#define TCM_DRAM_DATA_WIDTH     (g_dram_desc.u4DramDataWidth)
#define TCM_DRAM_DLYCELL_PERT   (g_dram_desc.u4DramDlyCellPert)
#define TCM_DRAM_FLAGS1      	(g_dram_desc.u4DramFlags1)
#define TCM_DRAM_FLAGS2     	(g_dram_desc.u4DramFlags2)
#define TCM_DRAM_FLAGS3     	(g_dram_desc.u4DramFlags3)
#define TCM_DRAM_FLAGS4     	(g_dram_desc.u4DramFlags4)


/****************TCM_DRAM_FLAGS bits define***********************/
/*
[11: 0] dram clock
[15:12] dram TYPE_SHIFT
[20] support channel B
[21] channel B active
[22] channel A 32bit ?
[23] channel B 32bit?
[24] QFP package
[25] SWAP 16bit?
[26] DMSSON?
[27] DC Balance
[28] dynamic odt
[29] BUSX8
[30] Suspend status
*/
#define DRAM_CLOCK_MASK         (0x00000FFFU) //bit[11:0]
#define DRAM_TYPE_MASK          (0x0000F000U) //bit[15:12]
#define TYPE_SHIFT       	    (12)
#define DRAM_DDR_CL_MASK        (0x000F0000U) //bit[19:16]
#define CL_SHIFT       		    (16)

#define CHB_SUPPORT_SHIFT       (20)
#define DRAM_CHANNELB_SUPPORT   (1U << CHB_SUPPORT_SHIFT)
#define CHB_ACTIVE_SHIFT        (21)
#define DRAM_CHANNELB_ACTIVE    (1U << CHB_ACTIVE_SHIFT)
#define CHA_FORCE32_SHIFT       (22)
#define DRAM_CHA_FORCE32        (1U << CHA_FORCE32_SHIFT)
#define CHB_FORCE32_SHIFT       (23)
#define DRAM_CHB_FORCE32        (1U << CHB_FORCE32_SHIFT)
#define QFP_SHIFT               (24)
#define DRAM_DDR_QFP            (1U << QFP_SHIFT)
#define SWAP16BIT_SHIFT         (25)
#define DRAM_SWAP16BIT          (1U << SWAP16BIT_SHIFT)
#define DMSSON_SHIFT            (26)
#define DRAM_DMSSON				(1U << DMSSON_SHIFT)
#define DC_BALANCE_SHIFT		(27)
#define DRAM_DC_BALANCE        	(1U << DC_BALANCE_SHIFT)
#define DYNAMIC_ODT_SHIFT       (28)
#define DRAM_DYNAMIC_ODT        (1U << DYNAMIC_ODT_SHIFT)
#define BUSX8_SHIFT				(29)
#define DRAM_BUSX8				(1U << BUSX8_SHIFT)
#define SUSPEND_STATE_SHIFT		(30)
#define DRAM_SUSPEND_STATE		(1U << SUSPEND_STATE_SHIFT)
/************************************************************/

#define BASE_DDR_CLK			        1000000
#define TCMGET_DDR_CLK()				((TCM_DRAM_FLAGS & DRAM_CLOCK_MASK) * BASE_DDR_CLK)
#define TCMGET_DRAMTYPE()				((TCM_DRAM_FLAGS & DRAM_TYPE_MASK) >> TYPE_SHIFT)
#define TCMGET_DDR_CL()					((TCM_DRAM_FLAGS & DRAM_DDR_CL_MASK) >> CL_SHIFT)
#define TCMGET_CHANNELA_SIZE()			(TCM_DRAM_SIZE & 0xFFFF) // size in Mbytes. 
#define TCMGET_CHANNELB_SIZE()			((TCM_DRAM_SIZE >> 16) & 0xFFFF) // size in Mbytes.
#define TCMGET_DLYCELL_PERT				(IS_DRAM_CHANNELB_ACTIVE() ? \
												(TCM_DRAM_DLYCELL_PERT&0xff00)>>8 \
												: (TCM_DRAM_DLYCELL_PERT&0x00ff))
												

#define TCMSET_CHANNELA_ACTIVE()    	(TCM_DRAM_FLAGS &= ~DRAM_CHANNELB_ACTIVE)
#define TCMSET_CHANNELB_ACTIVE()    	(TCM_DRAM_FLAGS |= DRAM_CHANNELB_ACTIVE)
#define TCMSET_CHANNELA_SIZE(size)		(TCM_DRAM_SIZE = (TCM_DRAM_SIZE & 0xffff0000) + (size & 0xffff))   // size in Mbytes.
#define TCMSET_CHANNELB_SIZE(size)		(TCM_DRAM_SIZE = (TCM_DRAM_SIZE & 0xffff) + (size << 16))
#define TCMSET_DRAM_SUSPEND()			(TCM_DRAM_FLAGS |= DRAM_SUSPEND_STATE)
#define TCMSET_DLYCELL_PERT(delaycell_pert)	TCM_DRAM_DLYCELL_PERT = (IS_DRAM_CHANNELB_ACTIVE() ? \
												((TCM_DRAM_DLYCELL_PERT&0x00ff)|((delaycell_pert)<<8)) \
												: ((TCM_DRAM_DLYCELL_PERT&0xff00)|(delaycell_pert)))
#define TCMSET_DRAM_NORMAL()			(TCM_DRAM_FLAGS &= ~DRAM_SUSPEND_STATE)


#define IS_FORCE32()					(IS_DRAM_CHANNELB_ACTIVE() ? \
                                                ((TCM_DRAM_FLAGS & DRAM_CHB_FORCE32) ? 1 : 0) : \
                                                ((TCM_DRAM_FLAGS & DRAM_CHA_FORCE32) ? 1 : 0))
#define IS_DDR_QFP()					((TCM_DRAM_FLAGS & DRAM_DDR_QFP) ? 1 : 0)
#define IS_DDR_SWAP16BIT()				((TCM_DRAM_FLAGS & DRAM_SWAP16BIT) ? 1 : 0)
#define IS_DDR_DMSSON()					((TCM_DRAM_FLAGS & DRAM_DMSSON) ? 1 : 0)
#define IS_DDR_DCBALANCEON()			((TCM_DRAM_FLAGS & DRAM_DC_BALANCE) ? 1 : 0)
#define IS_DDR_DYNAMICODTON()			((TCM_DRAM_FLAGS & DRAM_DYNAMIC_ODT) ? 1 : 0)
#define IS_DDR_BUSX8()					((TCM_DRAM_FLAGS & DRAM_BUSX8) ? 1 : 0)
#define IS_DDR_SUSPENDSTATE()			((TCM_DRAM_FLAGS & DRAM_SUSPEND_STATE) ? 1 : 0)	
#define IS_DRAM_CHANNELB_SUPPORT()		((TCM_DRAM_FLAGS & DRAM_CHANNELB_SUPPORT) ? 1 : 0)
#define IS_DRAM_CHANNELB_ACTIVE()		((TCM_DRAM_FLAGS & DRAM_CHANNELB_ACTIVE) ? 1 : 0)

#define TCMGET_DATA_WIDTH()            (IS_DRAM_CHANNELB_ACTIVE() ? \
                                                ((TCM_DRAM_DATA_WIDTH & 0x0000ff00)>>8) : \
                                                ((TCM_DRAM_DATA_WIDTH & 0x000000ff)) )

#define TCMSET_DATA_WIDTH(width)      (TCM_DRAM_DATA_WIDTH = IS_DRAM_CHANNELB_ACTIVE() ? \
                                                ((TCM_DRAM_DATA_WIDTH & (~0x0000ff00)) | ((width)>>8)) : \
                                                ((TCM_DRAM_DATA_WIDTH & (~0x000000ff)) | (width) ))

/************************************************************/


#endif // DDR_INCLUDES_H

