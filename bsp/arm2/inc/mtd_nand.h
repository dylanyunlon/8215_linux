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
#include "x_typedef.h"

#define _SUPPORT_WP_
#define ASSERT(e) \
      do{ \
        if(!(e)){ \
          while(1); \
        } \
      } \
      while(0)


typedef struct {
	UINT16		m_physical_blk[128*1024];
	UINT8		m_reserved[8];
  UINT32  version;
} Nand_ImageInfo_S;

#define TEMP_MEM_SIZE 4096
#define MAX_NFB_BLOCK_NUM 4096
typedef struct
{
    UINT16 p2NFBTable[MAX_NFB_BLOCK_NUM];
}PARTITION_NFB_TABLE;

/* Data Structures */
typedef struct _NFIType
{
   //UINT16   IOInterface;     /* IO_8BITS or IO_16BITS */
   UINT16   pageSize;        /* 512, 2048 */
   UINT16   spareSize;
   UINT16   addressCycle;   
   UINT16   pageShift;
} NFI_MENU;

typedef struct _BOOTLHeader_
{
   char ID1[12];
   char version[4];
   UINT32 length;
   UINT32 startAddr;
   UINT32 checksum;
   char ID2[8];
   NFI_MENU  NFIinfo;
   UINT16 pagesPerBlock;   
   UINT16  totalBlocks;
   UINT16  blockShift;
   UINT16  linkAddr[6];   
   UINT16  lastBlock;
} BOOTL_HEADER;

extern INT32 i4NFBInit(void);
extern INT32 i4NFBRead(UINT32* p4MemPtr, UINT32 u4Addr, UINT32 u4Length);
extern INT32 i4NFBPartitionRead(UINT32 u4Partition, UINT32* p4MemPtr, UINT32 u4Addr, UINT32 u4Length);
extern UINT32 u4NFBDWRDRead(UINT32 u4Addr);
extern BOOL fgBootLoaderHeaderVerification(UINT32* pu4Source);
extern BOOL MTD_Init(UINT32* _pTempMemPtr);

#define MTD_BOOT_LOADER     0
#define MTD_BINARY          1
#define MTD_MISC_DATA       2
#define MTD_BD_APP_DATA     3
#define MTD_APP_CFG_DATA    4
#define MTD_CPS_MGR_DATA    5
#define MTD_BINDING_DATA    6
#define MTD_BD_JAR_FILE     7
#define MTD_KEY_BLOCK_1     8
#define MTD_KEY_BLOCK_2     9
#define MTD_LOG_STORG       10
#define MTD_IRS_STORG       11
#define MTD_NAND_INFO       12
#define MTD_RESERVED        13
#define NVM_CUST_EXT_RES    14
#define MTD_ADV_RW_AREA     15
#define MTD_DIAG_SRV_LOG    16
#define MTD_APP_CFG_NAND    17
#define MTD_LOADER_BACKUP      18
#define MTD_LOADER_BINARY       19
#define MTD_LOADER_FA               20
#define MTD_LOADER_PWRCURVE  21
#define MTD_LOADER_PARAM         22
#define MTD_MAX_ITEM        23

#define NVM_NOR  1
#define NVM_NAND 2

#define NVM_PART_BOOT  1
#define NVM_PART_STORG 2
#define NVM_PART_LOG   3

#define NFB_BOOT_LOADER_HEADER_LENGTH 0x200
#define DEFAULT_MTD_BOOT_LOADER_SIZE 0x400000

#define NFI_LOCK_SEMA() \
    BIM_GETHWSemaphore(HSMPHE_NAND,0)

#define NFI_UNLOCK_SEMA() \
    BIM_ReleaseHWSemaphore(HSMPHE_NAND)

typedef struct
{
    UINT32 u4Item;
    UINT32 u4FlashType;
    UINT32 u4PartBase;
    UINT32 u4Addr;
    UINT32 u4Size;
}NE_TABLE_ITEM_T;

typedef enum {
     S_DONE = 0
    ,S_TIMEOUT
    ,S_IN_PROGRESS  
    ,S_ECC_UNCORRECT_ERR
    ,S_ECC_CORRECTABLE_ERR  
    ,S_UNKNOWN_ERR
}STATUS_E;
