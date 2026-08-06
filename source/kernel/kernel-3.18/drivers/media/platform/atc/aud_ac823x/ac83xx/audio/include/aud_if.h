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

/****************************************************************************
**  Audio Driver: Interface
****************************************************************************/

#ifndef _AUD_IF_H_
#define _AUD_IF_H_

//#include "x_typedef.h"
#include <linux/types.h>
#include <media/atc/drv_aud.h>

/****************************************************************************
** Type definitions
****************************************************************************/

/****************************************************************************
** Constant definitions
****************************************************************************/
//#define AUD_VER_BRANCH      TEXT("Main")  //branch
#define AUD_MOD_NAME        TEXT("AUD")  //Moudle name
#define AUD_VER_MAIN        04           //main version
#define AUD_VER_MINOR       01           //minor version, Large feature tens add 1 and clear unit,
                                         //other feature and add new function need add 1.
#define AUD_VER_REV         11           //version number, you should add 1 when check in aud
#define AUD_VER_CHANGELIST  150218       //the latest change list of  aud

//code check in time
#define AUD_VER_MONTH  TEXT("08")
#define AUD_VER_DAY    TEXT("9")
#define AUD_VER_YEAR   TEXT("2017")

#define AUD_OK	     (s32)(0)
#define AUD_FAIL     (s32)(-1)
#define AUD_DSPERROR (s32)(-2)

/****************************************************************************
** Export API
****************************************************************************/
extern void AUD_WaitCommandDone(u8 ucDecId, AUD_DRV_CMD_T eAudDecCmd);
extern void AUD_CommandDoneNotify(u8 u1DecId,  AUD_DRV_CMD_T eAudDecCmd);
extern s32 i4AudEsm_Init(void);
extern void vADSPTaskExit(void);
extern void AudDrvThreadExit(void);
extern void vAudEsmThreadExit(void);
extern void AudShowVerInfo(void);

extern s32 AUD_SetDecType(u8 ucDecId,  AUD_DRV_STREAM_FROM_T eStreamFrom,
                            const AUD_DRV_FMT_INFO_T * prDecType);

extern s32 AUD_DSPCmdPlayAsyn(u8 ucDecId);
extern s32 AUD_DSPCmdPauseAsyn(u8 ucDecId);
extern s32 AUD_DSPCmdResumeAsyn(u8 u1DecId);
extern s32 AUD_DSPCmdStopAsyn(u8 ucDecId);
extern s32 AUD_DSPCmdResetAsyn(u8 ucDecId);

extern s32 AUD_GetAudFifo(uintptr_t * pu4Fifo1Start, uintptr_t * pu4Fifo1SEnd,
	                        uintptr_t * pu4Fifo2Start,uintptr_t * pu4Fifo2End);
extern void AUD_GetRWPtr(u8 ucDecId, u32 * pu4Rp, u32 * pu4Wp, u32 *pu4Size);
extern void AudSetDecSrcInfo(u8 u1Dec, AUD_DEC_AUD_INFO_T* prInfo);
extern void vSendADSPCmd(u32 u4Cmd);
extern void vAudSaveFirstAudPts(u32 u4FirstPts, u8 u1DecId);
extern void vAudResetFirstAudPts(u8 u1DecId);


extern void AudAout_SetFRAoutMode(u32 u4Data);

#endif /* _AUD_IF_H_ */
