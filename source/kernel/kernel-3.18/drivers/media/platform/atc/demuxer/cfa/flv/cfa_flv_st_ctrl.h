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



#ifndef _CFA_FLV_ST_CTRL_H
#define _CFA_FLV_ST_CTRL_H

/* C header file */
#ifdef __cplusplus
extern "C" {
#endif


/*............................Macro  Define................................*/
/*--------------------------------------------------------------
* FLV CFA state control for transfer done
* @return None
* @note  This function will be called after a transfer is complete.
* [IN] void *pvSptHdl, handle of splitter
* [IN] u64 u8TxLen, Actual transferred data length.  Normally
*        this value should be equal to the u4Len in the previous
*        transfer issue, unless file end is hit.
* [IN] CfaFlvInst_T *prCfaFlvInst, pointer to CfaFlvInst
*--------------------------------------------------------------*/
	EXTERN void CfaFlvTxDoneStCtrl(void *pvSptHdl,
				       u64 u8TxLen,
				       CfaFlvInst_T *prCfaFlv
	    );


/*-------------------------------------------------------------------
* Descripiton: Start to search next tag header and get parsing payload info.
* @return None
*-------------------------------------------------------------------*/
	EXTERN void CfaFlvSearchNextSc(void *pvSptHdl,	/* [IN] handle of splitter */
				       CfaFlvInst_T *prCfaFlv,	/* [IN] pointer to CfaFlvInst */
				       CfaFlvAnaSt_E eNextAnaSt,	/* [IN] next analyze state */
				       u64 u8PreLen,/* [IN] previous transfer
				       file length before search next start code */
				       u64 u8ReadLen	/* [IN] the data length we want to read */
	    );

	u64 CfaFlvGetTxSa(CfaFlvInst_T *prCfaFlv);

	EXTERN void CfaFlvReleaseCfgBuf(CfaFlvInst_T *prCfaFlv);

	EXTERN u32 Spt4CfaGetPitureType(u32 u4CfaPictureType);

/* C header file */
#ifdef __cplusplus
}
#endif
#endif				/* cfa_flv_st_ctrl.h */
