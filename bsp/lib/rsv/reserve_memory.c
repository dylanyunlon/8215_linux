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
 * AutoChips Inc. (C) 2021. All rights reserved.
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
#include "reserved_memory.h"
#include "printf.h"

#ifdef TRUSTZONE
int strncmp(const char *cs, const char *ct, int count)
{
    signed char __res = 0;

    while (count) {
        if ((__res = *cs - *ct++) != 0 || !*cs++)
            break;
        count--;
    }
    return __res;
}
#define NULL   0
#define pr_info UART_Printf 

#endif
#ifdef ARM2
#define TAGS "RSV"
extern UINT32 ARM1PHY2ARM2UCV(UINT32 u4ARM1phyadress);
#endif

const RSV_MEM_T *get_rsv_mem_by_name(const char *name)
{
#ifdef ARM2
    P_RSV_MEM_INFO_T p_rsv_mem_info = (P_RSV_MEM_INFO_T) (ARM1PHY2ARM2UCV(RSV_MEM_BUF_BASE));
#else 
    P_RSV_MEM_INFO_T p_rsv_mem_info = (P_RSV_MEM_INFO_T) RSV_MEM_BUF_BASE;
#endif
    const RSV_MEM_T *start = p_rsv_mem_info->rsv_mem;
    const RSV_MEM_T *found = NULL;
    unsigned int i = 0;
    unsigned int g_rsv_mem_num = 0;
    g_rsv_mem_num = p_rsv_mem_info->rsv_mem_header.rsv_mem_num;

    for (i = 0; i< g_rsv_mem_num ; i++) {
        if (!strncmp(name, start[i].name, 24)) {
            found = start + i;
            break;
        }
    }
	if (NULL == found)
    	pr_info("get_rsv_mem_by_name %s not found\n", name);
    return found;
}

void dump_rsv_mem_info(void)
{
#ifdef ARM2
    P_RSV_MEM_INFO_T p_rsv_mem_info = (P_RSV_MEM_INFO_T) (ARM1PHY2ARM2UCV(RSV_MEM_BUF_BASE));
#else 
    P_RSV_MEM_INFO_T p_rsv_mem_info = (P_RSV_MEM_INFO_T) RSV_MEM_BUF_BASE;
#endif
    const RSV_MEM_T *start = p_rsv_mem_info->rsv_mem;
    unsigned int i = 0;
    unsigned int g_rsv_mem_num = 0;
    g_rsv_mem_num = p_rsv_mem_info->rsv_mem_header.rsv_mem_num;
	pr_info("Dump Reserved Memory Info magic:%s size:%d\n", p_rsv_mem_info->rsv_mem_header.magic, p_rsv_mem_info->rsv_mem_header.rsv_mem_num);
//	pr_info("Dump Reserved Memory Info magic:%s size:%d\n", p_rsv_mem_info->rsv_mem_header.magic, p_rsv_mem_info->rsv_mem_header.rsv_mem_num);
    for (i = 0; i< g_rsv_mem_num ; i++) {
        pr_info("name :%s ,start addr: 0x%x\n", start[i].name, start[i].start_addr & 0xFFFFFFFF);
        pr_info("name :%s ,start size: 0x%x\n", start[i].name, start[i].size & 0xFFFFFFFF);
    }
}

