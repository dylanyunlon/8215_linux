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

#ifndef _AUDMHL_IF_H_
#define _AUDMHL_IF_H_
/*-----------------------------------------------------------------------------
                    Include header files
-----------------------------------------------------------------------------*/
#include "audmhl_var.h"
#include <media/atc/drv_aud.h>
#include "drv_if_pbbuf.h"
#include "drv_def.h"

#if CONFIG_DRV_HDMI_RX

//#define HDMI_AUDIO_IN_SW_DEBUG  0

// definition to direct RAW to HDMI In Buffer without through DIR
#define HDMIRX_SPD_DIRECT  1
// definition to direct SD  RAW to HDMI In Buffer without through DIR
#define HDMIRX_SPD_DIRECT_SD  1

// Use for multiple line in master mode after MT8555
//#define MULTI_LINE_LINEIN_MASTER_MODE      //Support Multiple Line In H/W Master mode

#define HBR_REORDER                        // Support HBR reorder
#define MULTI_LINE_BUF_WP_MONITOR          // Multiple Line In H/W buffer writing point monitor

// *********************************************************************
// Type definitions
// *********************************************************************
typedef struct _AUDMHL_IN_BUFFER
{
    u32  u4Buf_W;
    u32  u4Buf_R;
    u32  u4Buf_SA;
    u32  u4Buf_EA;
} AUDMHL_IN_BUFFER;

typedef struct _AUDIN_MULTI_INFO_T
{
    AUDIO_IN_TYPE_T  eAudinType ;
} AUDIN_MULTI_INFO_T;

typedef struct _MULTILINE_IN_INFO_T
{
    SPDIFIN_IN_FORMAT_T   uDataType ;  /* DataType for SPDIF In : PCM/RAW/DTS-CD 16/DTS-CD 14  */
    AUD_DRV_FMT_T         eFormat;
} MULTILINE_IN_INFO_T;

typedef enum
{
    SPDIF_RX_SOURCE = 0,
    HDMI_RX_SOURCE,
} MULTILINE_IN_SOURCE_T;

#define AUDMHL_OK         ((s32)1)
#define AUDMHL_FAIL       ((s32)0)

#define AUDMHL_IS_RAW     ((s32)1)
#define AUDMHL_NOT_RAW    ((s32)0)

/*-----------------------------------------------------------------------------
                    Functions declaraions
-----------------------------------------------------------------------------*/
s32 AudmhlDRVInit(AUDMHL_IN_TYPE eAudinType);
s32 AudmhlDRVUnInit(AUDMHL_IN_TYPE eAudinType);
s32 AudmhlInit(const AUDMHL_SET_T *prAudinSetInfo);
void  AudmhlUnInit(AUDMHL_IN_TYPE u1AudinType);
void  AudmhlInCtrl(bool fgOnOffCmd);
void  AudmhlBufInit(void);
void  AudmhlBufRst(void);
void  AudmhlSetAudInType(AUDIO_IN_TYPE_T uAudinFmt);
AUDIO_IN_TYPE_T AudmhlGetAudInType(void);

bool  AudmhlSetAudOnOff(bool fgOn);

void AudmhlGetAudInInfo(AUDIN_INFO_T *pv_get_info);
void AudmhlGetHDMIRXAUDINFO(HDMI_RX_IN_AUDIO_INFO_T *pv_get_info);

// Interface for Audio In
void AudmhlSetEmphasisFlag(bool fgEmphasis);
void AudmhlSetAudInfo(AUDIN_INFO_T *pHDMIRxAudioInfo, u8  u1MuliChFlag);

void  AudmhlSetMLinHWClk(u32 u4SrcBitNum, u32 u4Cycle);
void  AudmhlNotifySPDAudinType(SPDIFIN_IN_FORMAT_T uAudinType);
u8 AudmhlGetDataType(void);

//API for MW
void AudmhlSwitch(AUDMHL_OPEN_CTRL eCtrl);

s32 AudmhlSetCtrl(u32 u4SetType);

void AudmhlSendDataBuf(void* hCmdQ);
void AudmhlSendAudMsg(u32 u4Cmd, u8 bPri);

//s32 AudmhlGetRecBuf(SEND_BUFFER *prReadBuffer);

typedef void (*atc_hdmiaudio_isr_t)(void *arg);
extern s32 atc_hdmiaudio_register_isr(atc_hdmiaudio_isr_t isr, void *arg);
extern s32 atc_hdmiaudio_unregister_isr(atc_hdmiaudio_isr_t isr, void *arg);
typedef struct atc_hdmiaudio_isr_data {
	atc_hdmiaudio_isr_t	isr;
	void			*arg;
}_atc_hdmiaudio_isr_data;

#endif

#endif
