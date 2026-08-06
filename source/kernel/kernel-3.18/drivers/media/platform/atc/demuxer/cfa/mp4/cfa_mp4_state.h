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



#ifndef _CFA_MP4_STATE_H_
#define _CFA_MP4_STATE_H_

#include "x_typedef.h"
#include "cfa_mp4.h"

#ifdef __cplusplus
extern "C" {

#endif
void CfaMp4TxDoneStCtrl(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
void CfaMp4LpcmChunk2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4);

#ifdef __cplusplus
}
#endif

#endif				/* #ifndef _CFA_MP4_STATE_H_*/
