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



#ifndef CFA_RM_ST_CTRL_H
#define CFA_RM_ST_CTRL_H

#include "cfa_rm.h"

/* C header file */
#ifdef __cplusplus
extern "C" {
#endif


/*............................Macro  Define................................*/
#define CfaRmToPlay(variable, format) (((variable) & (format)) > 0)


/* RM CFA state control for transfer done
@return None
@note  This function will be called after a transfer is complete.*/
EXTERN void CfaRmTxDoneStCtrl
(
void *pvSptHdl,	   /*< [IN] handle of splitter*/

/*< [IN] Actual transferred data length.  Normally this value should be equal to the u4Len
in the previous transfer issue, unless file end is hit.*/
u64		   u8TxLen,
CfaRmInst_T *prCfaRm	   /*< [IN] pointer to CfaRmInst*/
);


/*Descripiton: Start to search next tag header and get parsing payload info.
@return None*/
EXTERN void CfaRmSearchNextSc
(
void *pvSptHdl,				/*<[IN] handle of splitter*/
CfaRmInst_T *prCfaRm,	/*<[IN] pointer to CfaRmInst*/
CfaRmAnaSt_E eNextAnaSt,	/*<[IN] next analyze state  */
u64 u8PreLen,			/*<[IN] previous transfer file length before search next start code*/
u64 u8ReadLen,			/*<[IN] the data length we want to read*/
u32 u4DestOft			/*<[IN] header buffer ofst. to put the data from this ofst*/
);

u64 CfaRmGetTxSa(CfaRmInst_T *prCfaRm);

/* C header file */
#ifdef __cplusplus
}
#endif

#endif /* cfa_rm_st_ctrl.h */
