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

#ifndef AC83XX_UPG_STATUS_H
#define AC83XX_UPG_STATUS_H

//============================================================================
// Include files
//============================================================================
#include <asm/arch/x_typedef.h>
#include <asm/arch/ac83xx_part_tbl.h>


//============================================================================
// Include files
//============================================================================


//============================================================================
// Type definitions
//============================================================================
#define US_PROGRESS_UPG_SUC   0
#define US_PROGRESS_UPG_START 1
#define US_PROGRESS_UPG_BE    2
#define US_PROGRESS_UPG_FE    4

#define US_PROGRESS_MASK 0x0000000F
#define US_PRINT_EN 0x00
#define US_PRINT_DIS 0xFF
#define US_PRINT_MASK 0x000000FF

#define ACTIVE_ID_LIST_MAX_NR 32

#define UPG_STATUS_HEAD_SIG     0x55504753
#define UPG_STATUS_END_SIG      0x53475055
#define UPG_STATUS_END_BOOT_SIG 0x62617267

/* error number */
#define UPG_STATUS_OK          0
#define ERR_NO_PART_INFO       1
#define ERR_BLOCK_SIZE_ZERO    2
#define ERR_MALLOC_FAIL        3
#define ERR_NAND_READ_FAIL     4
#define ERR_HEAD_SIG           5
#define ERR_END_SIG            6
#define ERR_INPUT_ARGS         7
#define ERR_NAND_WRITE_FAIL    8
#define ERR_BUF_SZ             9

typedef struct _upg_status_t
{
    UINT32 u4_head_sig;
	UINT32 u4_progress;
	UINT32 au4_active_id_list[ACTIVE_ID_LIST_MAX_NR];
	UINT32 u4_active_id_list_nr;
	UINT32 u4_end_sig;
	UINT32 u4_cec_la;
	UINT32 u4_cec_pa;
	UINT32 u4_cec_switch;
	UINT32 u4_cec_trad_mode;
	UINT32 u4_log_cfg;
	UINT32 u4_sweety_switch;     
	UINT32 u4_end_boot_sig;	
}UPG_STATUS_T;


//============================================================================
// Public functions
//============================================================================
extern int i_get_upg_status(UPG_STATUS_T* pr_upg_status);
extern int i_set_upg_status(UPG_STATUS_T* pr_upg_status);
extern BOOL fg_is_upg_status_inited(UPG_STATUS_T* pr_upg_status);
extern BOOL fg_is_part_active(PART_TBL_ITEM* pr_part, UPG_STATUS_T* pr_upg_status);


#endif  // AC83XX_UPG_STATUS_H

