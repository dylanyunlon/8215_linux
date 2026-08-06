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

#ifndef _FAST_LOGO_API_H_
#define _FAST_LOGO_API_H_

/*************************************************
 * include files
 **************************************************/
#include "x_typedef.h"


/*************************************************
 * include files
 **************************************************/
#define ARM2_WORK_SPACE_SIZE		0x00100000
#define	ARM1_WORK_SPACE_SA			0xC0000000

/*
 * Fastlogo binary info
 */
 
typedef struct _fl_bin_info
{
    UINT32 u4_fl_str_sig;
    UINT32 u4_fl_text_base;
    UINT32 u4_fl_armboot_start;
    UINT32 u4_fl_bss_base;
    UINT32 u4_fl_bss_end;
    UINT32 u4_fl_end_sig;
}FL_BIN_INFO;

#define FL_BIN_INFO_OFFSET  0x40
#define FL_BI_STR_SIG     	0xBE3085EF
//#define FL_BI_END_SIG     	0xFE5803EB
#define AVM_BI_STR_SIG			0xDCBA4321
//#define AVM_BI_END_SIG     	0x1234ABCD

typedef enum
{
    FL_BI_OFST_STR_SIG   = FL_BIN_INFO_OFFSET + 0x00,
    FL_BI_OFST_TEXT_BASE = FL_BIN_INFO_OFFSET + 0x04,
    FL_BI_OFST_ARMBOOT   = FL_BIN_INFO_OFFSET + 0x08,
    FL_BI_OFST_BSS_BASE  = FL_BIN_INFO_OFFSET + 0x0c,
    FL_BI_OFST_BSS_END   = FL_BIN_INFO_OFFSET + 0x10,
    FL_BI_OFST_END_SIG   = FL_BIN_INFO_OFFSET + 0x14,
}FL_BIN_OFFSET;

#define FL_SEARCH_LIMIT 		0x00400000 /* 4M */
#define FL_SEARCH_INTERVAL 	0x200  /* 4K */

/*
 * error code
 */
#define LD_OK 									0
#define LD_FIND_FL_ERR 					0x1d000002

/*
 * DRAM_CONFIG
 */
 
#define	DRAM_SIZE_TYPE_1				0x10000000
#define	DRAM_SIZE_TYPE_2				0x20000000
#define	DRAM_OFFSET_LIMIT				0x1000000	// For fastlogo purpose
#define AVM_OFFSET_LIMIT				0x0100000 // For AVM purpose

/*
 *	For Standby On purpose
 */
#if defined(CONFIG_ATC_PLATFORM_ac823x)
#define IO_BASE 						(0x10000000)
#define PDWNC_BASE						(IO_BASE + 0x24000)
#endif

#define PDWNC_READ32(offset)					IO_READ32(PDWNC_BASE, offset) 
#define PDWNC_WRITE32(offset, value)	IO_WRITE32(PDWNC_BASE, offset, (value))
#define	REG_RW_ARM_IND_DATA0					0x1c4
#define	REG_RW_RESRV1									0x164
#define	REG_RW_RESRV2									0x168
#define	REG_RW_RESRV3									0x16c
#define STDBY_CHECK_VALUE							0x24000164
#define	RW_ARM_STDBY_INFO_1						(1U << 30)
#define	RW_ARM_STDBY_INFO_2						(1U << 31)

/*================================================
 * public function
 *================================================*/
extern BOOL fg_fast_logo_arm2_entry();
//extern void v_dram_no_reserve(void);

#endif /*_FAST_LOGO_API_H_ */
