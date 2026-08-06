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

#include "aud_debug.h"
#include "DspStruct.h"
#include "DspVar.h"
#include "DspD2RCInt.h"

/***************************************************************************
      Function : vCliDbgWriteDspSram
   Description : write dsp sram through interrupt
     Parameter : u4Addr: address,u4Value: value in 24 bits
     Return    :
***************************************************************************/
void vCliDbgWriteDspSram(u8 u1DspId,u32 u4Addr, u32 u4Value)
{
    u32 dwTryCnt = 0;

    while (TRUE)
    {
        dwTryCnt++;

        if (!((u1DspId == DSPC_ID)?fgAudHalDspCBusy():(((u1DspId == DSPA_ID) ? fgAudHalDspABusy() : fgAudHalDspBBusy()))))
        {
            if(u1DspId == DSPA_ID)
            {
                vAudHalWriteDSPAIntLD(u4Addr);
                vAudHalSendDSPAInt(0xF8, u4Value);   // short data & interrupt
                break;
            }
            else if(u1DspId == DSPC_ID)
            {
                vAudHalWriteDSPCIntLD(u4Addr);
                vAudHalSendDSPCInt(0xF8, u4Value);   // short data & interrupt
                break;
            }
            else
            {
                vAudHalWriteDSPBIntLD(u4Addr);
                vAudHalSendDSPBInt(0xF8, u4Value);   // short data & interrupt
                break;
            }
        }
        if (dwTryCnt == 100000)
        {
            if(u1DspId == DSPA_ID)
            {
                vAudHalWriteDSPAIntLD(u4Addr);
                vAudHalSendDSPAInt(0xF8, u4Value);   // short data & interrupt
                break;
            }
            else if(u1DspId == DSPC_ID)
            {
                vAudHalWriteDSPCIntLD(u4Addr);
                vAudHalSendDSPCInt(0xF8, u4Value);   // short data & interrupt
                break;
            }
            else
            {
                vAudHalWriteDSPBIntLD(u4Addr);
                vAudHalSendDSPBInt(0xF8, u4Value);   // short data & interrupt
                break;
            }
        }
    }
}

/***************************************************************************
     Function : u4CliDbgReadDspSram
  Description : Read Dsp Sram through interrupt(used for CLI debug only)
    Parameter : u4Addr : address
    Return    : value in UIN32
***************************************************************************/
u32 u4CliDbgReadDspSram(u8 u1DspId, u32 u4Addr)
{
    u32 u4Value = 0, u4TryCnt = 0;

    while (TRUE)
    {
        if (!((u1DspId == DSPC_ID)?fgAudHalDspCBusy() :((u1DspId == DSPA_ID) ? fgAudHalDspABusy() : fgAudHalDspBBusy())))
        {
            if(u1DspId == DSPA_ID)
            {
                vAudHalWriteDSPAIntLD(0);
                vAudHalSendDSPAInt(INT_RC2D_READ_DSP_MEMORY, u4Addr<< 8);   // short data & interrupt
            }
            else if(u1DspId == DSPC_ID)
            {
                vAudHalWriteDSPCIntLD(0);
                vAudHalSendDSPCInt(INT_RC2D_READ_DSP_MEMORY, u4Addr<< 8);   // short data & interrupt
            }
            else
            {
                vAudHalWriteDSPBIntLD(0);
                vAudHalSendDSPBInt(INT_RC2D_READ_DSP_MEMORY, u4Addr<< 8);   // short data & interrupt
            }

            break;
        }

        u4TryCnt++;
        if (u4TryCnt == 1000)
        {
            if(u1DspId == DSPA_ID)
            {
                vAudHalWriteDSPAIntLD(0);
                vAudHalSendDSPAInt(INT_RC2D_READ_DSP_MEMORY, u4Addr<< 8);   // short data & interrupt
            }
            else if(u1DspId == DSPC_ID)
            {
                vAudHalWriteDSPCIntLD(0);
                vAudHalSendDSPCInt(INT_RC2D_READ_DSP_MEMORY, u4Addr<< 8);   // short data & interrupt
            }
            else
            {
                vAudHalWriteDSPBIntLD(0);
                vAudHalSendDSPBInt(INT_RC2D_READ_DSP_MEMORY, u4Addr<< 8);   // short data & interrupt
            }
            break;
        }
    }
    mdelay(1);

    u4Value = u4AudHalGetDspLongData(u1DspId);

    return(u4Value);
}


