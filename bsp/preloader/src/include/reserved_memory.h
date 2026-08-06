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

#ifndef RESERVE_MEMORY_H
#define RESERVE_MEMORY_H

#define READ64(addr) (*((volatile unsigned long long *)(addr)))
#define WRITE64(addr, value) (*((volatile unsigned long long *)(addr))) = (value)
#define MRF_LOAD_OFFSET   0x700000

#define RSV_MEM_SIZE         0x500
#define RSV_MEM_BUF_BASE     CONFIG_ARGS_START 
typedef struct {
    char magic[4];              //"RSV"
    unsigned int rsv_mem_num;   //reserved memory number
    unsigned int checksum;      //checksum use CRC32 etc.
    unsigned int reserved;      //reserved for future use
} RSV_MEM_HEADER;

/* Reserved Memory Entry Struct*/
typedef struct {
    char name[24];                  //reserved memory name
    unsigned long long start_addr;  //physical start addr in ap view from dts file
    unsigned long long size;        //size
} RSV_MEM_T, *P_RSV_MEM_T;


#define  MAX_RSV_MEM_NUM  (RSV_MEM_SIZE - sizeof(RSV_MEM_HEADER))/sizeof(RSV_MEM_T)

typedef struct {
    RSV_MEM_HEADER rsv_mem_header;
    RSV_MEM_T  rsv_mem[MAX_RSV_MEM_NUM];
} RSV_MEM_INFO_T, *P_RSV_MEM_INFO_T;

const RSV_MEM_T *get_rsv_mem_by_name(const char *name);
void dump_rsv_mem_info(void);
#endif
