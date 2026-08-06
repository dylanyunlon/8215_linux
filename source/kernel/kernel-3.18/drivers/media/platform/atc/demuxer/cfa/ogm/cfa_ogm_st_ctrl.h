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


#ifndef _CFA_OGM_ST_CTRL_H_
#define _CFA_OGM_ST_CTRL_H_

#include "cfa_ogm.h"

/* C header file */
#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------------
				Macros, Typedefs, Enumerations
-----------------------------------------------------------------------------*/
#define OGM_TIME					10000000
#define OGM_TIME_S_TO_PTS           90000
#define OGM_MAX_DIFFRENCE_TIME      5

/*--- from Mps_util.h */
#define CfaOgmFourCC(a, b, c, d)		(((d)<<24)|((c)<<16)|((b)<<8)|(a))
#define CfaOgmTwoCC(a, b)			(((b)<<8)|(a))
#define CfaOgmIs4cc(addr, a, b, c, d)	(*((u32 *)(addr)) == CfaOgmFourCC(a, b, c, d))



/*-----------------------------------------------------------------------------
 * Name: vCfaOgmTxDoneStCtrl
 *
 * Description:
 *	Ogm cfa state control for transfer done
 *	This function will be called after a transfer is complete.
 *
 * Inputs: -
 *
 * Outputs: -
 *
 * Returns: None
 *-----------------------------------------------------------------------------*/
void CfaOgmTxDoneStCtrl(void *pvSptHdl, u64 u8TxLen, CfaOgmInst *prCfaOgm);

/*-----------------------------------------------------------------------------
 * Name: ucCfaOgmGetAudIndex
 *
 * Description:
 *      Get the index of audio info by audio stream ID.
 *      For multi-channel.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: the index of audio info
 *
 *-----------------------------------------------------------------------------*/
u8 CfaOgmGetAudIndex(CfaOgmInst *prCfaOgm, u32 u4StrmID);

void CfaOgmNextState(void *pvSptHdl, CfaOgmInst *prCfaOgm, u64 u8ReadLen, CfaOgmAnaState_E  eCfaOgmNextState);


/* C header file */
#ifdef __cplusplus
}
#endif

#endif  /* _CFA_OGM_ST_CTRL_H_ */



