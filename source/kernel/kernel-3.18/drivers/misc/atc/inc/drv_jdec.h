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


#ifndef __DRV_JDEC_H
#define __DRV_JDEC_H

#include "x_common.h"
#include "x_img_dec.h"

INT32 i4JdecInit(uintptr_t u4Inst);
INT32 i4JdecUnInit(uintptr_t u4Inst);
INT32 i4JdecFillBuf(/*UINT32 u4Inst, */IMG_BUF_FILLED_T *prBufFilled);
extern INT32 i4JdecDecode(/*UINT32 u4Inst, */IMG_DECODE_T *prDecode);
extern INT32 i4JdecPrepare(/*UINT32 u4Inst,*/IMG_DEC_PREPARE_T *prPrepare);
extern INT32 i4JdecSetDecParam(/*UINT32 u4Inst,  */IMG_DEC_PARAM_T *prDecParam);
extern INT32 i4JdecClear(VOID);/*UINT32 u4Inst*/
INT32 i4JdecStop(VOID);/*UINT32 u4Inst*/
INT32 i4JdecGetDstBuffer(/*UINT32 u4Inst,*/ IMG_BUF_T *prBuf, BOOL *pfgSelfAlloc);
INT32 i4JdecGetDstYCBuffer(IMG_BUF_T *prYBuf,IMG_BUF_T *prCBuf);
extern INT32 i4JdecGetSrcBuffer(/*UINT32 u4Inst,*/ IMG_BUF_T *prBuf, BOOL *pfgSelfAlloc);
extern INT32 i4JdecGetPicInfoBuffer(/*UINT32 u4Inst,*/ IMG_BUF_T *prBuf, BOOL *pfgSelfAlloc);
INT32 i4JdecGetFinishState(/*UINT32 u4Inst, */IMG_DEC_FINISH_STATE_T *peState);

#ifdef __linux__
INT32 i4jdecGetNotifyBuffer(IMG_BUF_T *prBuf, BOOL *pfgSelfAlloc);
#endif


#endif //__DRV_ESE_H
