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

#define _DSP_CMD_C
/*-----------------------------------------------------------------------------
                    Include header files
-----------------------------------------------------------------------------*/
#include "aud_oal.h"
#include "DspUop.h"
#include "DspStruct.h"
#include "DspVar.h"
#include "aud_if.h"

/*-----------------------------------------------------------------------------
                    Data declarations
-----------------------------------------------------------------------------*/
#ifndef __linux__
CRITICAL_SECTION int_queue_lock;
CRITICAL_SECTION cmd_queue_lock;
#else
static DEFINE_SPINLOCK(int_queue_lock);
static DEFINE_SPINLOCK(cmd_queue_lock);
#endif

TDspCmd   _tDspCmd;
TDspCmd   _tDspCmdH;

#define DSP_CMD_Q_NAME    "DSPCmd"
#define DSP_CMD_Q_SIZE    (16)

extern void vDspSetEvent(u32 u4Event);
extern void vDspWaitEvent(u32 u4Event);

/*-----------------------------------------------------------------------------
                    Functions implementations
-----------------------------------------------------------------------------*/
/******************************************************************************
* Function      : vDspCmdInit
* Description   :
* Parameter     :
* Return        :
******************************************************************************/
void vDspCmdInit (void)
{
    _tDspCmd.bCmdNs = 0;
    _tDspCmd.bRdIdx = 0;
    _tDspCmd.bWrIdx = 0;
    _tDspCmdH.bCmdNs = 0;
    _tDspCmdH.bRdIdx = 0;
    _tDspCmdH.bWrIdx = 0;

    LOG(LOG_DATAF, TEXT("vDspCmdInit \n"));
    InitializeCriticalSection(&int_queue_lock);
    InitializeCriticalSection(&cmd_queue_lock);
}


/******************************************************************************
* Function      : vDspCmdUnInit
* Description   :
* Parameter     :
* Return        :
******************************************************************************/
void vDspCmdUnInit (void)
{
    DeleteCriticalSection(&int_queue_lock);
    DeleteCriticalSection(&cmd_queue_lock);
}


/******************************************************************************
* Function      : vSendDspCmd
* Description   :
* Parameter     :
* Return        :
******************************************************************************/
void vSendDspCmd(u32 u4Cmd)
{
    TDspCmd *ptCmd;
    u32 flags = 0;

    ENTERCRITICALSECTION(&cmd_queue_lock, flags);

    ptCmd = &_tDspCmd;
    if((ptCmd->bWrIdx) >= MAX_DSP_CMD_NS)
    {
         AUD_VERIFY(0);
    }
    else
    {
        ptCmd->pu4Cmd[ptCmd->bWrIdx] = u4Cmd;
    }

    ptCmd->bWrIdx++;
    if (ptCmd->bWrIdx >= MAX_DSP_CMD_NS)
    {
        ptCmd->bWrIdx = 0;
    }

    ptCmd->bCmdNs++;
    if (ptCmd->bWrIdx == ptCmd->bRdIdx)
    {
        // if command fifo full, replace old MPV command
        AUD_VERIFY(0);
        ptCmd->bRdIdx++;
        if (ptCmd->bRdIdx >= MAX_DSP_CMD_NS)
        {
            ptCmd->bRdIdx = 0;
        }
    }

    vDspSetEvent(EvDspUop);

    LEAVECRITICALSECTION(&cmd_queue_lock, flags);
}

/***************************************************************************
     Function : vSendDspISR
  Description : DSP interrupt interface to dspctrl
    Parameter :

    Return    : None
***************************************************************************/
void vSendDspISR(u8 u1DspRIntAddr, u32 u4DspRIntSD, u32 u4DspRIntLD, bool fgDspId)
{
    TDspCmd *ptCmd;
    u32 u4Cmd = 0;
    u32 flags = 0;

    if (u1DspRIntAddr == 0x0008)
    {
        return;
    }

    // start critical section
    ENTERCRITICALSECTION(&int_queue_lock, flags);

    ptCmd = &_tDspCmdH;

    u4Cmd = u1DspRIntAddr << 8 | DSP_UOP_INT;
    if((ptCmd->bWrIdx) >= MAX_DSP_CMD_NS)
    {
        AUD_VERIFY(0);
    }
    else
    {
        ptCmd->pu4Cmd[ptCmd->bWrIdx] = u4Cmd;
        ptCmd->prCmd[ptCmd->bWrIdx].u4DspRIntSD = u4DspRIntSD;
        ptCmd->prCmd[ptCmd->bWrIdx].u4DspRIntLD = u4DspRIntLD;
        ptCmd->prCmd[ptCmd->bWrIdx].fgDspId = fgDspId;
    }
    ptCmd->bWrIdx++;
    if (ptCmd->bWrIdx >= MAX_DSP_CMD_NS)
    {
        ptCmd->bWrIdx = 0;
    }

    ptCmd->bCmdNs++;

    if (ptCmd->bWrIdx == ptCmd->bRdIdx)
    {
        AUD_VERIFY(0);
        // if command fifo full, replace old MPV command
        ptCmd->bRdIdx++;
        if (ptCmd->bRdIdx >= MAX_DSP_CMD_NS)
        {
            ptCmd->bRdIdx = 0;
        }
    }

    vDspSetEvent(EvDspIsr);

    // critical section release
    LEAVECRITICALSECTION(&int_queue_lock, flags);

}

/******************************************************************************
* Function      : vDspCmdDispatch
* Description   :
* Parameter     :
* Return        :
******************************************************************************/
void vDspCmdDispatch(void)
{
    TDspCmd *ptCmd;
    u32 flags = 0;

    if (g_fgDspUop)
    {
        return;
    }

    // start critical section
    ENTERCRITICALSECTION(&cmd_queue_lock, flags);

    ptCmd = &_tDspCmd;
    if (ptCmd->bWrIdx != ptCmd->bRdIdx)     // command in
    {
        g_fgDspUop= TRUE;
        if ((ptCmd->bRdIdx) >= MAX_DSP_CMD_NS)
        {
            AUD_VERIFY(0);
        }
        else
        {
            g_u4DspUop = ptCmd->pu4Cmd[ptCmd->bRdIdx];
        }
        ptCmd->bCmdNs--;
        ptCmd->bRdIdx++;
        if (ptCmd->bRdIdx >= MAX_DSP_CMD_NS)
        {
            ptCmd->bRdIdx = 0;
        }
    }
    else
    {
        g_fgDspUop = FALSE;
    }

    // critical section release
    LEAVECRITICALSECTION(&cmd_queue_lock, flags);

}


void vDspHCmdDispatch(void)
{
    TDspCmd *ptCmd;
    u32 flags = 0;

    if (g_fgDspHUop)
    {
        return;
    }
    // start critical section
    ENTERCRITICALSECTION(&int_queue_lock, flags);

    ptCmd = &_tDspCmdH;

    if (ptCmd->bWrIdx != ptCmd->bRdIdx)     // command in
    {
         g_fgDspHUop = TRUE;

         g_u4DspHUop = (ptCmd->pu4Cmd[ptCmd->bRdIdx] | (ptCmd->prCmd[ptCmd->bRdIdx].fgDspId)<<24);

         g_u4DspRIntSD = ptCmd->prCmd[ptCmd->bRdIdx].u4DspRIntSD;
         g_u4DspRIntLD = ptCmd->prCmd[ptCmd->bRdIdx].u4DspRIntLD;

         //#ifdef First_Version_Cmd_control
         ptCmd->bCmdNs--;
         //#endif

         ptCmd->bRdIdx++;
         if (ptCmd->bRdIdx >= MAX_DSP_CMD_NS)
         {
            ptCmd->bRdIdx = 0;
         }
    }
    else
    {
        g_fgDspHUop = FALSE;
    }

    // critical section release
    LEAVECRITICALSECTION(&int_queue_lock, flags);
}


