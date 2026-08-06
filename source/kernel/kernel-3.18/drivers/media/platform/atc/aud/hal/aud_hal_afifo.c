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

/******************************************************************************
*[File]        Aud_hal_3360.c
*[Author]
*[Description] Audio hardware adpator layer main file
*              Chip dependent functions for MT3360
*
******************************************************************************/
#include "aud_debug.h"
#include "aud_hal_intf.h"
#include "drv_config.h"
#include "aud_drv_config.h"

#include "aud_comm_reg_rw.h"
#include "aud_reg_env.h"
#include <media/atc/drv_aud.h>
#include "DspStruct.h"
#include "aud_3360_reg_rw.h"
#include "aud_drv.h"
#include "aud_if.h"


extern u32 g_u4Dec1Afifo[]; //0:start,1:end
extern u32 g_u4Dec4Afifo[];
extern u32 g_u4Dec5Afifo[];
extern u8 *g_ucAdspWorkingBuffer;

// 1MB alignment Bank Address
#define AFIFOBKADDR     (AFIFO_PHYSICAL(DspGetAfifoSA()) & 0x7FF00000)

static u32 s_u4AFIFOBaseAddr = 0;

u32 DspGetAfifoSA(void)
{
   return g_u4Dec1Afifo[0];
}


u32 u4AudHalGetAFIFOBankForInit(void)
{
    u32 u4MinAddr = DspGetAfifoSA();
    s_u4AFIFOBaseAddr = u4MinAddr;

    return ((AFIFO_PHYSICAL(u4MinAddr) & 0x7FF00000) >> 20);
}

/***************************************************************************
    Function : vAudHalSetBufStartAddr
    Description : Set start address of DSP Buffer
    Parameter :
    Return    :
***************************************************************************/
void vAudHalSetBufStartAddr(u8 u1DecId, u8 u1BufType, u32 u4Addr)
{
	u32 u4RegisterTemp = 0;

    if (u1DecId == PRI_DEC)
    {
        //Primary Audio
        if(DSP_AFIFO == u1BufType)
        {
            g_u4Dec1Afifo[0] = u4Addr;
            u4RegisterTemp = (AFIFO_PHYSICAL(u4Addr) - AFIFOBKADDR) & 0x00FFFFFF;
            WriteREG(RW_DSP_SW_BS0_SBLK, (u4RegisterTemp | BS0_HW_SW_MOD_SEL));
        }
    }
    else if (u1DecId == SEC_DEC)
    {
        if(DSP_AFIFO == u1BufType)
        {
            g_u4Dec4Afifo[0] = u4Addr;
            u4RegisterTemp = (AFIFO_PHYSICAL(u4Addr) - AFIFOBKADDR) & 0x00FFFFFF;
            WriteREG(RW_DSP_SW_BS1_SBLK, (u4RegisterTemp | BS0_HW_SW_MOD_SEL));
        }
    }
    else if (u1DecId == TER_DEC)
    {
        if(DSP_AFIFO == u1BufType)
        {
            g_u4Dec5Afifo[0] = u4Addr;
            u4RegisterTemp = (AFIFO_PHYSICAL(u4Addr) -AFIFOBKADDR) & 0x00FFFFFF;
            AUDREG_WRITE(REGENV_SW_BS2_SBLK, (u4RegisterTemp | BS0_HW_SW_MOD_SEL));
        }
    }
}

/***************************************************************************
    Function : vAudHalSetBufEndAddr
    Description : Set end address of DSP Buffer
    Parameter :
    Return    :
***************************************************************************/
void vAudHalSetBufEndAddr(u8 u1DecId, u8 u1BufType, u32 u4Addr)
{
	u32 u4RegisterTemp = 0;

    if (u1DecId == PRI_DEC)
    {
        //Primary Audio
        if(DSP_AFIFO == u1BufType)
        {
            g_u4Dec1Afifo[1] = u4Addr;
            u4RegisterTemp = (AFIFO_PHYSICAL(u4Addr) - AFIFOBKADDR) & 0x00FFFFFF;
            WriteREG(RW_DSP_SW_BS0_EBLK, u4RegisterTemp);
        }
    }
    else if (u1DecId == SEC_DEC)
    {
        if(DSP_AFIFO == u1BufType)
        {
            g_u4Dec4Afifo[1] = u4Addr;
            u4RegisterTemp = (AFIFO_PHYSICAL(u4Addr) - AFIFOBKADDR) & 0x00FFFFFF;
            WriteREG(RW_DSP_SW_BS1_EBLK, u4RegisterTemp);
        }
    }
    else if (u1DecId == TER_DEC)
    {
        if(DSP_AFIFO == u1BufType)
        {
            g_u4Dec5Afifo[1] = u4Addr;
            u4RegisterTemp = (AFIFO_PHYSICAL(u4Addr) - AFIFOBKADDR) & 0x00FFFFFF;
            AUDREG_WRITE(REGENV_SW_BS2_EBLK, u4RegisterTemp);
        }
    }
}

/***************************************************************************
    Function : vAudHalSetBufWPtr
    Description : Set write pointer of Dsp buffer
    Parameter : u4WPtrVal is phisical address(including Audio dram bank info)
    Return    :
***************************************************************************/
void vAudHalSetBufWPtr(u8 u1DecId, u8 u1BufType, u32 u4WPtrVal)
{
    if (u1DecId == PRI_DEC)
    {
        //Primary Audio
        if(DSP_AFIFO == u1BufType)
        {
            //Check if inside AFIFO
            if ((u4WPtrVal < g_u4Dec1Afifo[0])||(u4WPtrVal > g_u4Dec1Afifo[1]))
            {
                LOG(LOG_FAIL, TEXT("Pri wptr out of range: wptr = 0x%x,SA = 0x%x, EA = 0x%x.\n"),
                    u4WPtrVal,g_u4Dec1Afifo[0], g_u4Dec1Afifo[1]);
                AUD_VERIFY(0);
            }
            //Update to register
            WriteREG(RW_DSP_SW_BS0_PPNT, ((AFIFO_PHYSICAL(u4WPtrVal) - AFIFOBKADDR) & 0x00FFFFFF));
        }
    }
    else if (SEC_DEC == u1DecId)
    {
        if(DSP_AFIFO == u1BufType)
        {
            if ((u4WPtrVal < g_u4Dec4Afifo[0])||(u4WPtrVal >= g_u4Dec4Afifo[1]))
            {
                LOG(LOG_FAIL, TEXT("2nd wptr out of range: wptr = 0x%x,SA = 0x%x, EA = 0x%x.\n"),
                    u4WPtrVal,g_u4Dec4Afifo[0], g_u4Dec4Afifo[1]);
                AUD_VERIFY(0);
            }
            //wirte to register dspb
            WriteREG(RW_DSP_SW_BS1_PPNT,((AFIFO_PHYSICAL(u4WPtrVal) - AFIFOBKADDR) & 0x00FFFFFF));
        }
    }
    else if (TER_DEC == u1DecId)
    {
        if(DSP_AFIFO == u1BufType)
        {
            if ((u4WPtrVal < g_u4Dec5Afifo[0])||(u4WPtrVal >= g_u4Dec5Afifo[1]))
            {
                LOG(LOG_DATAF, TEXT("3rd wptr out of range: wptr = 0x%x,SA = 0x%x, EA = 0x%x.\n"),
                    u4WPtrVal,g_u4Dec5Afifo[0], g_u4Dec5Afifo[1]);
                AUD_VERIFY(0);
            }
            //wirte to register dspb
            AUDREG_WRITE(REGENV_SW_BS2_PPNT, ((AFIFO_PHYSICAL(u4WPtrVal) - AFIFOBKADDR) & 0x00FFFFFF));
        }
    }
}


void vAudHalResetBufWPtr(u8 u1DecId)
{
    u32 u4Wptr = 0;
	if(PRI_DEC == u1DecId)
	{
	    u4Wptr = g_u4Dec1Afifo[0];
	}
	else if(SEC_DEC == u1DecId)
	{
	    u4Wptr = g_u4Dec4Afifo[0];
	}
	else if(SEC_DEC == u1DecId)
	{
	    u4Wptr = g_u4Dec5Afifo[0];
	}
    vAudHalSetBufWPtr(u1DecId, DSP_AFIFO, u4Wptr);
}


/***************************************************************************
    Function : u4AudHalGetBufRPtr
    Description : Get read pointer of Dsp buffer
    Parameter :
    Return    :
***************************************************************************/
u32  u4AudHalGetBufRPtr(u8 u1DecId, u8 u1BufType)
{
    u32 u4BufRPtr = 0;
    if (u1DecId == PRI_DEC)
    {
        if (DSP_AFIFO == u1BufType)
        {   //Read pointer
            WriteREG(RW_ABUF0_PNT, 0);
            u4BufRPtr = ((ReadREG(RW_ABUF0_PNT) & 0x00FFFFFF) + s_u4AFIFOBaseAddr);
        }
    }
    else if (u1DecId == SEC_DEC)
    {
        if (DSP_AFIFO == u1BufType)
        {   //Read pointer
            WriteREG(RW_ABUF1_PNT, 0);
            u4BufRPtr = ((ReadREG(RW_ABUF1_PNT) & 0x00FFFFFF) + s_u4AFIFOBaseAddr);
        }
    }
    else if (u1DecId == TER_DEC)
    {
        if (DSP_AFIFO == u1BufType)
        {   //Read pointer
            AUDREG_WRITE(REGENV_SW_BS2_RPTR, 0);
            u4BufRPtr = ((AUDREG_READ(REGENV_SW_BS2_RPTR) & 0x00FFFFFF) + s_u4AFIFOBaseAddr);
        }
    }

    return (u32)(u4BufRPtr);

}


/***************************************************************************
* Function : u4AudHalGetAFIFOWPtr
* Description : Get AFIFO Write pointer -- (Water, WDbg CLI Used)
* Parameter : None
* Return    : AFIFO Write pointer
* Note      :
***************************************************************************/
uintptr_t u4AudHalGetAFIFOWPtr(u32 u4DecId)
{
    u32 u4WPtr = 0;

    switch(u4DecId)
    {
    case PRI_DEC:
        u4WPtr = AUDREG_READ(REGENV_SW_BS0_PPNT);
        u4WPtr += u4AudHalGetAFIFOBaseAddr(u4DecId);
        break;

    case SEC_DEC:
        u4WPtr = AUDREG_READ(REGENV_SW_BS1_PPNT);
        u4WPtr += u4AudHalGetAFIFOBaseAddr(u4DecId);
        break;

    case TER_DEC:
        u4WPtr = AUDREG_READ(REGENV_SW_BS2_PPNT);
        u4WPtr += u4AudHalGetAFIFOBaseAddr(u4DecId);
        break;

    default:
        break;
    }

    return u4WPtr;
}

/***************************************************************************
* Function : u4AudHalGetAFIFOStartAddr
* Description : Get AFIFO Start Address -- (Water, WDbg CLI Used)
* Parameter : None
* Return    : AFIFO Start address
* Note      :
***************************************************************************/
uintptr_t u4AudHalGetAFIFOStartAddr(u32 u4DecId)
{
    u32 u4SAddr = 0;

    switch(u4DecId)
    {
    case PRI_DEC:
        u4SAddr = AUDREG_READ(REGENV_SW_BS0_SBLK) & 0x00FFFFFF;
        u4SAddr += u4AudHalGetAFIFOBaseAddr(u4DecId);
        break;

    case SEC_DEC:
        u4SAddr = AUDREG_READ(REGENV_SW_BS1_SBLK) & 0x00FFFFFF;
        u4SAddr += u4AudHalGetAFIFOBaseAddr(u4DecId);
        break;

    case TER_DEC:
        u4SAddr = AUDREG_READ(REGENV_SW_BS2_SBLK) & 0x00FFFFFF;
        u4SAddr += u4AudHalGetAFIFOBaseAddr(u4DecId);
        break;

    default:
        break;
    }

    return u4SAddr;
}

/***************************************************************************
* Function : u4AudHalGetAFIFOEndAddr
* Description : Get AFIFO End Address -- (Water, WDbg CLI Used)
* Parameter : None
* Return    : AFIFO End address
* Note      :
***************************************************************************/
uintptr_t u4AudHalGetAFIFOEndAddr(u32 u4DecId)
{
    u32 u4EAddr = 0;

    switch(u4DecId)
    {
    case PRI_DEC:
        u4EAddr = AUDREG_READ(REGENV_SW_BS0_EBLK) & 0x00FFFFFF;
        u4EAddr += u4AudHalGetAFIFOBaseAddr(u4DecId);
        break;

    case SEC_DEC:
        u4EAddr = AUDREG_READ(REGENV_SW_BS1_EBLK) & 0x00FFFFFF;
        u4EAddr += u4AudHalGetAFIFOBaseAddr(u4DecId);
        break;
    case TER_DEC:
        u4EAddr = AUDREG_READ(REGENV_SW_BS2_EBLK) & 0x00FFFFFF;
        u4EAddr += u4AudHalGetAFIFOBaseAddr(u4DecId);
        break;

    default:
        break;
    }

    return u4EAddr;
}



