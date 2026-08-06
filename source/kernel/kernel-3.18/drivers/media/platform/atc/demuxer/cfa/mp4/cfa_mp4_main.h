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


#ifndef _CFA_MP4_MAIN_H_
#define _CFA_MP4_MAIN_H_

#include "x_typedef.h"

#ifdef __cplusplus
extern "C" {

#endif
MRESULT CfaMp4CfgStmTbl(MP4_STBL_INFO **prMp4ToTbl, MP4_STBL_INFO *prMp4FromTbl);
void CfaMp4InternalFreeMem(CfaMp4Inst *prCfaMp4);
MRESULT CfaMp4PlayNextMoof(CfaMp4Inst *prCfaMp4);
void CfaMp4ParseRange(CfaMp4Inst *prCfaMp4);
MRESULT mrCfaMp4ParseTable(CfaMp4Inst *prCfaMp4);
bool BadInterleaved(void *pvSptHdl, Mp4BadIntlvdCheck *prMp4BadIntlvd);

#ifdef __cplusplus
}
#endif

#endif				/* #ifndef _CFA_MP4_MAIN_H_ */
