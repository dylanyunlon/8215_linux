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



#ifndef _CFA_MP4_ANA_H_
#define _CFA_MP4_ANA_H_

#include "x_typedef.h"
#include "cfa_mp4.h"

#ifdef __cplusplus
extern "C" {

#endif
void CfaMp4FinishPrs(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
void CfaMp4PrsNextState(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
void CfaMp4AnaGetVPts2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
void CfaMp4AnaGetAPts2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
void CfaMp4AnaGetSPts2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
void CfaMp4TxAvcPayload2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
void CfaMp4TxwmvPayload2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
void CfaMp4TxWVC1Payload2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
void CfaMp4AnaPrsVRange(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
void CfaMp4AnaPrsARange(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
void CfaMp4AnaPrsSRange(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
#if MP4_SUPPORT_FRAGMENT
void CfaMp4AnaPrsMoofHeader(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
MRESULT  CfaMp4AnaPrsMoofTfhd(void *pvBuf, CfaTfhdInfo *mTfhdInfo, u32 u4BufLen);
MRESULT  CfaMp4AnaPrsMoofTrunParse(MP4_MOOF_INFO *TCfaMoofInfo);
MRESULT  CfaMp4AnaPrsMoofTrun(void *pvSptHdl, CfaMp4Inst *prCfaMp4);
MRESULT  CfaMp4MoofInit(MP4_MOOF_INFO *TCfaMoofInfo);
MRESULT  CfaMp4MoofTrunMemAlloc(MP4_MOOF_INFO *TCfaMoofInfo);
#endif

#ifdef __cplusplus
}
#endif

#endif				 /*#ifndef _CFA_MP4_ANA_H_z*/
