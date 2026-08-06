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

#ifndef _SECURE_INFO_H_
#define _SECURE_INFO_H_

#include "x_typedef.h"

/* define */
#define BIN_NAME_SIZE (16)
#define SIGNATURE_SIZE (16)
#define SIT_ITEM_MAX_NR (16)
#define FW_KEY_SIZE (16)

/* sit struct */
typedef struct _sit_item_t
{
	CHAR pc_bin_name[BIN_NAME_SIZE];
	UINT32 u4_part_id;
	UINT32 u4_bin_info;
	UINT32 u4_offset;
	UINT32 u4_size;
	CHAR pc_signature[SIGNATURE_SIZE];
}SIT_ITEM_T;

typedef struct _sit_t
{
	UINT32 u4_sig_cal_size;
	UINT32 u4_item_nr;
	CHAR pc_sit_sig[SIGNATURE_SIZE];
	CHAR pc_fw_enc_key[FW_KEY_SIZE];
	SIT_ITEM_T sit_raw[SIT_ITEM_MAX_NR];
}SIT_T;

/* public functions */
extern int i_get_boot_sit(SIT_T* sit, UINT32 start, UINT32 length);
extern int i_get_active_sit(SIT_T* sit, UPG_STATUS_T* upg_status);
extern int i_get_backup_sit(SIT_T* sit, UPG_STATUS_T* upg_status);
extern BOOL fg_is_sit_item_need_inte_check(SIT_ITEM_T* item);

#endif //_SECURE_INFO_H_


