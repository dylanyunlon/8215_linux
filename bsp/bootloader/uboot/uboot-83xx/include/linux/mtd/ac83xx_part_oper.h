/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef AC83XX_PART_OPER_H
#define AC83XX_PART_OPER_H

//============================================================================
// Include files
//============================================================================
#include <asm/arch/x_typedef.h>
//============================================================================
// Include files
//============================================================================
/* parition name */
#define KERN_PART_NAME          "kernel_1"
#define KERN_BK_UP_PART_NAME    "kernel_2"
#define ROOTFS_PART_NAME        "rootfs_1"
#define ROOTFS_BK_UP_PART_NAME  "rootfs_2"
#define UPG_STATUS_PART_NAME    "upg_status"

#if ADAPT_MTD_LINUX
#define	PART_TBL_ITEM_NR_MAX		64
#define	SEARCH_PART_TBL_START		0x00000000 /* 0MB */
#define	SEARCH_PART_TBL_END		0x01000000	/* 16 MB */
#define	SEARCH_PART_TBL_INTERNAL		0x00040000	/* 1MB */

#define HEAD_SIG 0x8530EADC
#define TAIL_SIG 0xC0CAC01A


#define ADPT_HEAD_SIG   0x8530AAAA
#define ADPT_TAIL_SIG   0x5441494C

#define ADPT_PIT_OFFSET 0x100000

enum PIT_VER_T{
	V0 = 1,
	V1,
	V2,
	V3,
	V_ERR
};

typedef struct _new_part_tbl_item
{
    CHAR pc_name[PART_NAME_SIZE];
	UINT32 u4_id;
	UINT32 u4_info;
	UINT32 u4_offset;
	UINT32 u4_size;
	UINT32 u4_enc_size;
	UINT32 u4_no_enc_size;
	UINT32 u4_reserved[6];
}NEW_PART_TBL_ITEM;

typedef struct _v1_pit_header_t
{
    UINT32 u4_head_sig_1;
	UINT32 u4_head_sig_2;
	UINT32 u4_version;
	UINT32 u4_header_len;
	UINT32 u4_item_len;
	UINT32 u4_item_num;
}V1_PIT_HEADER_T;
#endif

//============================================================================
// Type definitions
//============================================================================


//============================================================================
// Public Data
//============================================================================
/* partition table format --
 * r_part_tbl[0]			: head signature
 * r_part_tbl[1] 			: partition 1
 * ......					: partition xx...
 * r_part_tbl[u4_part_nr]	: partition u4_part_nr (the last partition)
 * r_part_tbl[u4_part_nr+1]	: end signature
 */
//============================================================================
// Public functions
//============================================================================
UINT32 get_page_size(void);
UINT32 get_block_size(void);
INT32 i4NFBPartitionRead(UINT32 u4DevId,UINT32 u8Offset,UINT32 u4MemPtr,UINT32 u4MemLen);
INT32 i4NFBPartitionWrite(UINT32 u4DevId,UINT32 u8Offset,UINT32 u4MemPtr,UINT32 u4MemLen);
INT32 i4PartWriteTest(void);
INT32 i4SetNewFlashPIT(UINT32 bin_pit);
INT32 i4CompareFlashPIT(UINT32 bin_pit, UINT32 len);
INT32 i4GetAdptPit(void);
INT32 i4StoreAdptPit(void);



//============================================================================
// Public functions
//============================================================================
extern UINT32 get_page_size(void);
extern UINT32 get_block_size(void);
extern INT32 i4NFBPartitionRead(UINT32 u4DevId,UINT32 u8Offset,UINT32 u4MemPtr,UINT32 u4MemLen);
extern INT32 i4NFBPartitionWrite(UINT32 u4DevId,UINT32 u8Offset,UINT32 u4MemPtr,UINT32 u4MemLen);
extern INT32 i4PartWriteTest(void);
extern INT32 i4SetNewFlashPIT(UINT32 bin_pit);
extern INT32 i4CompareFlashPIT(UINT32 bin_pit, UINT32 len);
extern INT32 i4GetAdptPit(void);
extern INT32 i4StoreAdptPit(void);

extern UINT32 *orig_pit;
extern UINT32 *adpt_pit;
extern enum PIT_VER_T _pit_ver;

extern UINT32 _u4_orig_part_nr;
extern UINT32 _u4_same;

#endif  // AC83XX_PART_OPER_H

