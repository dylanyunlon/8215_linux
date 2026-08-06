/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/



#ifndef CFA_ASF_ST_CTRL_H
#define CFA_ASF_ST_CTRL_H

/* C header file */
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "x_typedef.h"
#include "windows.h"
#include "cfa_asf.h"



/*............................Macro  Define................................*/
#define WMV_PVOP								0x01
#define CFA_ASF_MULTI_PAYLOAD_START_B		1
#define CFA_ASF_VC1_SC_LEN						4

/*/mtk40504*/
#define CFA_ASF_AVSYNC_THRETHOLD				200 /*1000ms*/
#ifndef DIFF
#define DIFF(x, y)				(((x) > (y)) ? ((x) - (y)) : ((y) - (x)))
#endif

/*// ASF CFA state control for transfer done
/// @return None
/// @note  This function will be called after a transfer is complete.
//< [IN] handle of splitter
//< [IN] Actual transferred data length.
Normally this value should be equal to the u4Len in the previous transfer issue, unless file end is hit.
//< [IN] pointer to CfaAsfInst*/
EXTERN void CfaAsfTxDoneStCtrl(void *pvSptHdl, u64 u8TxLen, CfaAsfInst_T *prCfaAsf);

/*//Description: Get the index of audio info by audio stream ID.
///For multi audio.
///@return the index of audio info.
//< [IN] pointer to CfaAsfInst_T
//<[IN] audio stream Id*/
EXTERN u8 CfaAsfGetCurAudInfoIdx(const CfaAsfInst_T *prCfaAsf, u32 u4StrmId);

/*//Descripiton: Start to search packet header and get parsing payload info.
///@return None
//<[IN] handle of splitter
//<[IN] pointer to CfaAsfInst
//<[IN] next analyze state
//<[IN] previous transfer file length before search next start code
//<[IN] the data length we want to read
//<[IN] header buffer ofst. to put the data from this ofst */
EXTERN void CfaAsfSearchHeader(void *pvSptHdl, CfaAsfInst_T *prCfaAsf,
CfaAsfAnaSt_E eNextAnaSt, u64 u8PreLen, u64 u8ReadLen, u32 u4DestOft);

/* C header file */
#ifdef __cplusplus
}
#endif

#endif /* cfa_asf_st_ctrl.h */
