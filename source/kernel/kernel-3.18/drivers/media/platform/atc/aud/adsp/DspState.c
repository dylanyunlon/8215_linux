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

#define _DSP_STATE_C
/*-----------------------------------------------------------------------------
                    Include header files
-----------------------------------------------------------------------------*/
#include "DspVar.h"
#include "DspFunc.h"
#include "aud_hal_intf.h"
#include "aud_debug.h"

/*-----------------------------------------------------------------------------
                    Functions declaraions
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                    Functions implementations
-----------------------------------------------------------------------------*/
/***************************************************************************
     Function : vDSPState
  Description : DSP State Machine
    Parameter : 
    Return    : None
***************************************************************************/
void vDspState(void)
{
    u32 i = 0;

    if (g_fgDspASInt || g_fgDspBSInt) 
    {
        g_u4DspTimerCnt = 0;
        
        // Send interrupt to DSPA and DSPB
        while (g_fgDspASInt || g_fgDspBSInt)
        {
            if (!fgAudHalDspABusy() && g_fgDspASInt)
            {
                vDspASendInt();   
            }
            if (!fgAudHalDspBBusy() && g_fgDspBSInt) 
            {
                vDspBSendInt();
            }

            if (g_u4DspTimerCnt++ > 1000)
            {
                if (i++ > 600)
                {
                    LOG(LOG_CTRLF, TEXT("WAIT DSP TIME OUT ..., DSP A B busy is (%d, %d)\n"), fgAudHalDspABusy(), fgAudHalDspBBusy());
                    i = 0;
                    Sleep(500);
                }
               g_u4DspTimerCnt = 0;
               Sleep(5);
            }
        }
    }
    else if (g_fgDspHUop || g_fgDspUop)
    {
        if (g_fgDspHUop)
        {
            vDspUopSvc(g_u4DspHUop);
            g_fgDspHUop = FALSE;
            g_u4DspHUop = 0;
        }
        if (g_fgDspUop)
        {
            vDspUopSvc(g_u4DspUop);
            g_fgDspUop = FALSE;
            g_u4DspUop = 0;
        }
    }
    vDspCmdDispatch();
    vDspHCmdDispatch();
}


