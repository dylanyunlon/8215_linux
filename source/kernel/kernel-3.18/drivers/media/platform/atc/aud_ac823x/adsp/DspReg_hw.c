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

#include "aud_oal.h"
#include "x_ckgen.h"
#include "drv_config.h"
#include "DspStruct.h"
#include "aud_debug.h"
#include "DspVar.h"
#include <media/atc/drv_aud.h>

#ifndef __linux__
#else
#include <linux/spinlock_types.h>
static DEFINE_SPINLOCK(ac83xx_dspreg_hw_lock);
#endif

#include "aud_3360_reg_rw.h"

#define NEW_ROM_SPEED

u32 g_u4DspRC2DIntIdx = 0;
u32 g_u4DspAIntIdx    = 0;
u32 g_u4DspBIntIdx    = 0;

TDspUopInt g_tDspRC2DIntHist[32];
TDspUopInt g_tDspAIntHist[MAX_DSP_CMD_NS];
TDspUopInt g_tDspBIntHist[MAX_DSP_CMD_NS];

void vAudSendIntToDsp(u8 u1DspId, u32 u4DspIntAddr, u32 _u4DspIntSD, u32 _u4DspIntLD)
{
    u32 flags;
    u32 u4Cnt = 0;
    u8 u1DecId = ((u4DspIntAddr>>16) & (0x00FF));

    g_tDspRC2DIntHist[g_u4DspRC2DIntIdx].fgDspId = u1DecId;
    g_tDspRC2DIntHist[g_u4DspRC2DIntIdx].u4DspIntAddr = u4DspIntAddr;
    g_tDspRC2DIntHist[g_u4DspRC2DIntIdx].u4DspRIntSD = _u4DspIntSD;
    g_tDspRC2DIntHist[g_u4DspRC2DIntIdx].u4DspRIntLD = _u4DspIntLD;

    g_u4DspRC2DIntIdx = ((g_u4DspRC2DIntIdx+1)>=32)?0:(g_u4DspRC2DIntIdx+1);

    spin_lock_irqsave(&ac83xx_dspreg_hw_lock, flags);
    // Send INT to DSPA
    if (u1DspId == DSPA_ID)
    {
        while (TRUE)
        {
            if (!fgAudHalDspABusy())
            {
                break;
            }
            else if (u4Cnt++ > 1000)
            {
                u4Cnt = 0;
                LOG(LOG_CTRLF, TEXT("WAIT DSP TIME OUT ..., DSP A is busy\n"));
                Sleep(5);
            }
        }
        vAudHalWriteDSPAIntLD(_u4DspIntLD);  // long data
        vAudHalSendDSPAInt((u4DspIntAddr & 0xFF), _u4DspIntSD);   // short data & interrupt
    }
    else if (u1DspId == DSPB_ID)// Send INT to DSPB
    {
        while (TRUE)
        {
            if (!fgAudHalDspBBusy())
            {
                break;
            }
            else if (u4Cnt++ > 1000)
            {
                u4Cnt = 0;
                LOG(LOG_CTRLF, TEXT("WAIT DSP TIME OUT ..., DSP B is busy\n"));
                Sleep(5);
            }
        }
        vAudHalWriteDSPBIntLD(_u4DspIntLD);  // long data
        vAudHalSendDSPBInt((u4DspIntAddr & 0xFF), _u4DspIntSD);   // short data & interrupt
    }
    else if (u1DspId == DSPC_ID)// Send INT to DSPB
    {
        while (TRUE)
        {
            if (!fgAudHalDspCBusy())
            {
                break;
            }
            else if (u4Cnt++ > 1000)
            {
                u4Cnt = 0;
                LOG(LOG_CTRLF, TEXT("WAIT DSP TIME OUT ..., DSP C is busy\n"));
                Sleep(5);
            }
        }
        vAudHalWriteDSPCIntLD(_u4DspIntLD);  // long data
        vAudHalSendDSPCInt((u4DspIntAddr & 0xFF), _u4DspIntSD);   // DSP-C short data & interrupt
    }
    spin_unlock_irqrestore(&ac83xx_dspreg_hw_lock, flags);
}

/***************************************************************************
     Function : vDSPASendInt
  Description : Send Interrupt to DSP in main loop for both ShortInt or LongInt
    Parameter : None
    Return    : None
***************************************************************************/
void vDspASendInt(void)
{
    // Add dsp A interrupt history
    g_tDspAIntHist[g_u4DspAIntIdx].fgDspId = g_fgDspId;
    g_tDspAIntHist[g_u4DspAIntIdx].u4DspIntAddr = (g_u4DspASIntAddr & 0xFF);
    g_tDspAIntHist[g_u4DspAIntIdx].u4DspRIntLD = g_u4DspASIntLD;
    g_tDspAIntHist[g_u4DspAIntIdx].u4DspRIntSD = g_u4DspASIntSD;
    g_u4DspAIntIdx = ((g_u4DspAIntIdx+1)>=MAX_DSP_CMD_NS)? 0 : (g_u4DspAIntIdx+1);

    if (g_fgDspId == DSPC_ID)
    {
        vAudSendIntToDsp(DSPC_ID, g_u4DspASIntAddr, g_u4DspASIntSD, g_u4DspASIntLD);
    }
    else
    {
        vAudSendIntToDsp(DSPA_ID, g_u4DspASIntAddr, g_u4DspASIntSD, g_u4DspASIntLD);
    }

    g_fgDspId = DSPA_ID;
    g_fgDspASInt = FALSE;
}


/***************************************************************************
     Function : vDSPBSendInt
  Description : Send Interrupt to DSP in main loop for both ShortInt or LongInt
    Parameter : None
    Return    : None
***************************************************************************/
void vDspBSendInt(void)
{
    u8 u1DspId = ((g_u4DspBSIntAddr>>16) & (0x00FF));
    // Add dsp B interrupt history
    g_tDspBIntHist[g_u4DspBIntIdx].fgDspId = u1DspId;
    g_tDspBIntHist[g_u4DspBIntIdx].u4DspIntAddr = (g_u4DspBSIntAddr & 0xFF);
    g_tDspBIntHist[g_u4DspBIntIdx].u4DspRIntLD = g_u4DspBSIntLD;
    g_tDspBIntHist[g_u4DspBIntIdx].u4DspRIntSD = g_u4DspBSIntSD;
    g_u4DspBIntIdx = ((g_u4DspBIntIdx+1)>=MAX_DSP_CMD_NS)? 0 : (g_u4DspBIntIdx+1);

    if ((u1DspId == PRI_DEC) || (u1DspId == SEC_DEC) || (u1DspId == TER_DEC))
    {
        vAudSendIntToDsp(DSPB_ID, g_u4DspBSIntAddr, g_u4DspBSIntSD, g_u4DspBSIntLD);
    }
    else
    {
        vAudSendIntToDsp(DSPC_ID, g_u4DspBSIntAddr, g_u4DspBSIntSD, g_u4DspBSIntLD);
    }

    g_fgDspBSInt = FALSE;
}

