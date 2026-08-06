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

#ifndef AC83XX_PART_TBL_H
#define AC83XX_PART_TBL_H

//============================================================================
// Include files
//============================================================================
#include <asm/arch/x_typedef.h>
//============================================================================
// Include files
//============================================================================
#define HEAD_SIG 0x8530EADC
#define TAIL_SIG 0xC0CAC01A

/* parition name */
#define KERN_PART_NAME          "kernel_1"
#define KERN_BK_UP_PART_NAME    "kernel_2"
#define ROOTFS_PART_NAME        "initrd_1"
#define ROOTFS_BK_UP_PART_NAME  "initrd_2"
#define UPG_STATUS_PART_NAME    "upg_status"

#define SUPPORT_V0_FORMAT   1   
#if ADAPT_MTD_LINUX
extern UINT32 u4_adpt;
#endif
//============================================================================
// Type definitions
//============================================================================
#define PART_NAME_SIZE (16)
typedef struct _part_tbl_item
{
    CHAR pc_name[PART_NAME_SIZE];
	UINT32 u4_id;
	UINT32 u4_info;
	UINT32 u4_offset;
	UINT32 u4_size;
	
    /* new format column */	
	UINT32 u4_enc_unit_sz;	
	UINT32 u4_no_enc_unit_sz;
	
	//UINT32 u4Reserve[6];
	
}PART_TBL_ITEM;

#define PIT_ITEM_V0_SIZE (sizeof(PART_TBL_ITEM) - (sizeof(UINT32) * 2))


typedef struct _pit_header_t
{
	UINT32 u4_head_sig_1;
	UINT32 u4_head_sig_2;
	UINT32 u4_version;
	UINT32 u4_header_len;
	UINT32 u4_item_len;
	UINT32 u4_item_num;

	//UINT32 u4Reserve[10];
	
}PIT_HEADER_T;

typedef enum
{
	PIFT_MTD_RAW 	= 0,
	PIFT_MTD_YAFFS2	= 1,
	PIFT_MTD_SQUASH	= 2,
	PIFT_MTD_UBI	= 3,
	PIFT_UBI_VOL	= 4,
}PI_FORMAT_TYPE;

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
extern PART_TBL_ITEM r_part_tbl[];
extern UINT32 u4_part_nr;

//============================================================================
// Public functions
//============================================================================
extern PART_TBL_ITEM* ac83xx_get_part_info_by_name(const char* part_name);
extern BOOL fg_is_primary_part(PART_TBL_ITEM* pr_part_item);
extern UINT32 u4_get_backup_part_id(PART_TBL_ITEM* pr_part_item);
extern UINT32 u4_get_format_type(PART_TBL_ITEM* pr_part_item);
extern UINT32 u4_get_parent_part_id(PART_TBL_ITEM* pr_part_item);
extern PART_TBL_ITEM* get_parent_part_info(PART_TBL_ITEM* pr_part);
extern PART_TBL_ITEM* get_backup_part_info(PART_TBL_ITEM* pr_part);

#endif  // AC83XX_PART_TBL_H

