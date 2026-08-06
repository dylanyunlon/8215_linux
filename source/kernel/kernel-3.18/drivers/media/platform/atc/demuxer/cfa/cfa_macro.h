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



#ifndef _DMX_CFA_MACRO_H_
#define _DMX_CFA_MACRO_H_

#include "x_typedef.h"

#ifdef __cplusplus
extern "C" {

#endif				/*  */

/*! CfaStreamType, for all u4StrmToPrs variables and parameters */
#define CFA_STRM_V0				((u32)1 << 0)	/*!< video channel 0 */
#define CFA_STRM_V1				((u32)1 << 1)	/*!< video channel 1 */
#define CFA_STRM_V2				((u32)1 << 2)	/*!< video channel 2 */
#define CFA_STRM_V				CFA_STRM_V0	/*!< video default */

#define CFA_STRM_A0				((u32)1 << 8)	/*!< audio channel 0 */
#define CFA_STRM_A1				((u32)1 << 9)	/*!< audio channel 1 */
#define CFA_STRM_A2				((u32)1 << 10)	/*!< audio channel 2 */
#define CFA_STRM_A				CFA_STRM_A0	/*!< audio defaule */

#define CFA_STRM_SP0			((u32)1 << 16)	/*!< subpicture channel 0 */
#define CFA_STRM_SP1			((u32)1 << 17)	/*!< subpicture channel 1 */
#define CFA_STRM_SP				CFA_STRM_SP0	/*!< subpicture default */

#define CFA_STRM_ST0			((u32)1 << 20)	/*!< text subtitle channel 0 */
#define CFA_STRM_ST1			((u32)1 << 21)	/*!< text subtitle channel 1 */
#define CFA_STRM_ST				CFA_STRM_ST0	/*!< text subtitle default */

#define CFA_STRM_NV0			((u32)1 << 30)	/*!< nv pack default channel 0 */
#define CFA_STRM_NV				CFA_STRM_NV0	/*!< nv pack default */

#define CFA_STRM_SEC0			((u32)1 << 29)	/*!< section pack default channel 0 */
#define CFA_STRM_SEC			CFA_STRM_SEC0	/*!< section pack default */

#define fgIsCfaStmToPlay(CurPrsFlag, flag)  (0 != ((CurPrsFlag) & (flag)))

#define CFA_ALIGN_SZ(u4Sz, u4Aligned) (((u4Sz) + (u4Aligned)) / (u4Aligned) * (u4Aligned))

/* DIVX311_IVOP 0x00 */
#define DIVX311_PVOP				(u8)0x01

#define CFA_STC_CLK			    90000

#ifdef __cplusplus
}
#endif				/*  */
#endif				/* #ifndef _DMX_CFA_MACRO_H_ */
