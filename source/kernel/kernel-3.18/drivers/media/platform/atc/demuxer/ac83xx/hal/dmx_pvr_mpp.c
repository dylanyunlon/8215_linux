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

/*!
 * @file dmx_pvr_ddi.c
 *
 * @par Project
 *    MT3360
 *
 * @par Description
 *
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
//#include "ac83xx_basic.h"
//#include "ac83xx_gpio_pinmux.h"
#include "x_ckgen_8317.h"
#else
#include "Irqs_vector.h"
#endif // __linux__

#include "x_ckgen.h"
#include "x_assert.h"
#include "x_hal_ic.h"
#include "x_os.h"
#include "x_rtos.h"
#include "dmx_mem.h"
#include "dmx_pvr_if.h"
#include "dmx_pvr.h"
#include "dmx_pvr_mpp.h"
#include "dmx_psr_util.h"

#ifndef __linux__
#pragma warning(disable: 4127) //disable warning C4127: conditional expression is constant
#endif

#define PVR_POLLING_DBG_MPP_STATUS   0

//-----------------------------------------------------------------------------
// Configurations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------
#define MPP_MAX_PACKET_SIZE         255


//-----------------------------------------------------------------------------
// Type definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Static variables
//-----------------------------------------------------------------------------
static DMX_PVRPLAY_STRUCT_T _rPvrMPP;
static DMX_PVRPLAY_COUNTER_T _rPVRPlayCounter;
static u16 _au2PVRPlayFramerPktSize;
static DMX_TSFMT_T _eDmxPVRTSFmt;

#if 0
//-----------------------------------------------------------------------------
// Static functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// The caller of this function must ensure that prMPP is valid.
// This function must ensures that
// (1) _rDmxMPP.u4Threshold is a multiple of 16 bytes
// (2) _rPvrMPP.ptrRp and _rPvrMPP.ptrWp are aligned to the proper boundaries.
//-----------------------------------------------------------------------------
static bool _MPP_AllocBuf(const PVR_MPP_T *prMPP)
{
    u32 ptrBufStart =  0, ptrBufEnd, ptrPhyBufStart, ptrPhyBufEnd;

    if (NULL == prMPP)
    {
        PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid args.\r\n"),
            DMX_FUNC_NAME);
        DMX_ASSERT(0);
        return FALSE;
    }

    if ((0 != _rPvrMPP.ptrBufStart) ||
        (0 != _rPvrMPP.u4BufSize))
    {
        PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for MPP Buffer had been allocated.\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO);
        return FALSE;
    }

    if (!_PVR_IsAligned(prMPP->u4BufSize, DMX_PVR_BUF_ALIGNMENT))
    {
        PVR_LOG_ERR(TEXT("%s line %d fail for The requested MPP buffer size(0x%08x) is not aligned.\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO, prMPP->u4BufSize);
        return FALSE;
    }

    if (!_PVR_IsAligned(prMPP->u4Threshold, DMX_PVR_BUF_ALIGNMENT))
    {
        PVR_LOG_ERR(TEXT("%s line %d fail for The requested threshold size(0x%08x) is not aligned.\n"),
            DMX_FUNC_NAME, DMX_LINE_NO, prMPP->u4Threshold);
        return FALSE;
    }

    if (prMPP->fgAllocBuf)
    {
#ifdef __linux__
        DMX_NewHwAlignMemory(prMPP->u4BufSize, DMX_PVR_BUF_ALIGNMENT, ptrBufStart);
#else
        DMX_NewHwAlignMemory(prMPP->u4BufSize, DMX_PVR_BUF_ALIGNMENT, (void *)ptrBufStart);
#endif // #ifdef __linux__
        if (ptrBufStart == 0)
        {
            PVR_LOG_ERR(TEXT("%s line %d fail for Memory allocation failed!\n"),
                DMX_FUNC_NAME, DMX_LINE_NO);
            return FALSE;
        }
    }
    else
    {
        if (!_PVR_IsAligned(prMPP->ptrBufAddr, DMX_PVR_BUF_ALIGNMENT))
        {
            PVR_LOG_ERR(TEXT("%s line %d fail for The MPP buffer address is not aligned.\n"),
                DMX_FUNC_NAME, DMX_LINE_NO);
            return FALSE;
        }
        ptrBufStart = prMPP->ptrBufAddr;
    }

    ptrBufEnd      = ptrBufStart + prMPP->u4BufSize;
    ptrPhyBufStart = DMX_PHYSICAL(ptrBufStart);
    ptrPhyBufEnd   = DMX_PHYSICAL(ptrBufEnd);

    _PVR_Lock();
    _rPvrMPP.fgAllocBuf  = prMPP->fgAllocBuf;   // Buffer has been allocated
    _rPvrMPP.ptrBufStart  = ptrBufStart;
    _rPvrMPP.u4BufSize   = prMPP->u4BufSize;
    _rPvrMPP.ptrRp        = ptrBufStart;
    _rPvrMPP.ptrWp        = ptrBufStart;
    _rPvrMPP.u4ThresholdSize = prMPP->u4Threshold;
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_START, ptrPhyBufStart);     // Buffer start
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_END, ptrPhyBufEnd);         // Buffer end
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_RP, ptrPhyBufStart);        // Read pointer
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_WP, ptrPhyBufStart);        // Write pointer
    // The update of RP will be done by hardware when DMA is activiated.
    _PVR_Unlock();

    PVR_LOG_TRACE(TEXT("%s line %d -- MPP Src Buf's SA(0x%08x), EA(0x%08x), InitRP(0x%08x), WP(0x%08x), RP(0x%08x)\n"),
        DMX_FUNC_NAME, DMX_LINE_NO, ptrBufStart, ptrBufEnd, ptrBufStart, ptrBufStart, ptrBufStart);

    return TRUE;
}
#endif
//-----------------------------------------------------------------------------
/** _MPP_Reset
*/
// Reset the hardware including the hardware buffer pointers.
//-----------------------------------------------------------------------------
static void _MPP_Reset(void)
{
    if (_rPvrMPP.eState == PVR_PLAY_STATE_PLAYING)
    {
        DMX_ASSERT(FALSE);
    }

	smp_mb();
	
    DMXCMD_WRITE32(PVR_REG_PVR_CONTROL, 0x100);
	smp_mb();
	
    DMXCMD_WRITE32(PVR_REG_PVR_CONTROL, 0x0);
	smp_mb();

    return;
}

//-----------------------------------------------------------------------------
// Static functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/** _DMX_SetFramerPktSize
 */
//-----------------------------------------------------------------------------
void _PVR_MPP_SetFramerMode(bool fgExtSync, bool fgEnable)
{
    u32 u4Ctrl;

    // Reset framer -
    u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1);
    u4Ctrl = (u4Ctrl & 0xFFFFFFFE);
    DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Ctrl);   // disable framer

    if (fgExtSync)
    {
        u4Ctrl = (u4Ctrl & 0xFFFFFFFD) | (0x1 << 1);
    }
    else
    {
        u4Ctrl = (u4Ctrl & 0xFFFFFFFD);
    }

    if (fgEnable)
    {
        u4Ctrl = (u4Ctrl & 0xFFFFFFFE) | 0x1 ;
    }
    else
    {
        u4Ctrl = (u4Ctrl & 0xFFFFFFFE);
    }
    
    PVR_LOG_DBG("%s line %d -- MiniPVR Framer's SyncMode(%s), Enable(%s)\r\n",
        DMX_FUNC_NAME, DMX_LINE_NO, (fgExtSync ? "ExternalSync" : "InternalSync"),
        (fgEnable ? "Enable" : "Disable"));

    DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Ctrl);
}

//-----------------------------------------------------------------------------
// Inter-file functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/** _PVR_MPP_Init
*/
// Disable empty/alert interrupts here, and then enable them when data is
// written to the buffer.  The interrupts also need to be disabled when the
// Empty or Alert interrupts are triggered.
//-----------------------------------------------------------------------------
bool _PVR_MPP_Init(void)
{
    FUNC_ENTRY;

    _PVR_Lock();
    mm_memset(&_rPvrMPP, 0, sizeof(DMX_PVRPLAY_STRUCT_T));
    _rPvrMPP.eMode = PVR_MPP_MODE_STREAM; // default transfer mode
    _rPvrMPP.eState = PVR_PLAY_STATE_STOP;       // in case enum is changed carelessly
    _rPvrMPP.eMPPPortType = DMX_PVRPLAY_PORT_DBM;
    _rPvrMPP.u1PacketSize  = MPP_PACKET_SIZE;
    _rPvrMPP.u1SyncOffset  = 0;
    _rPvrMPP.fgMPPISRInited  = FALSE;
    _rPvrMPP.hMPPNotifyEG  = NULL_HANDLE;
    _rPvrMPP.pfnMPPNotify  = NULL;
    _rPvrMPP.fgAllocBuf = FALSE;

    _rPvrMPP.fgContainTimeStamp = FALSE;
    _rPvrMPP.fgIgnoreTimeStamp = TRUE;

    mm_memset((void*)&_rPVRPlayCounter, 0, sizeof(DMX_PVRPLAY_COUNTER_T));

    _PVR_Unlock();

    smp_mb();

    _MPP_Reset();

    smp_mb();

    if (!_PVR_MPP_SetPacketSize(MPP_PACKET_SIZE, MPP_PACKET_SIZE))
    {
        PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for Cannot set MPP packet size!\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO);
        FUNC_EXIT;
        return FALSE;
    }

    smp_mb();
    
    _PVR_MPP_SetFramerMode(TRUE, TRUE);

    if (!_PVR_MPP_SetMoveMode(DMX_TSFMT_188, TRUE))
    {
        return FALSE;
    }
    
#if DMX_PVR_PLAY_ISR
    if (!_PVR_MPP_InitISR())
    {
        PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_MPP_InitISR!\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO);
        FUNC_EXIT;
        return FALSE;
    }
#endif
    FUNC_EXIT;

    return TRUE;
}

//-----------------------------------------------------------------------------
/** _PVR_MPP_DeInit
*/
//-----------------------------------------------------------------------------
bool _PVR_MPP_DeInit(void)
{
    if (PVR_PLAY_STATE_STOP != _rPvrMPP.eState)
    {
        PVR_LOG_ERR(TEXT("[PVR] %s fail, Please stop MPP before free its buffer\r\n"),
            DMX_FUNC_NAME);
        return FALSE;
    }

    _MPP_Reset();  // Only reset some HW registers (RIP, WP, AP, and etc).

    if (_rPvrMPP.hMPPNotifyEG)
    {
        x_ev_group_set_event(_rPvrMPP.hMPPNotifyEG, PVR_MPP_NOTIFY_EV_BUF_EXIT, X_EV_OP_OR);
        x_ev_group_delete(_rPvrMPP.hMPPNotifyEG);
        _rPvrMPP.hMPPNotifyEG = NULL_HANDLE;
    }

    if (!_PVR_MPP_FreeBuf())
    {
        return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
/** _PVR_MPP_Set
*/
// Todo: consider to let users turn on/off "PCR Rate Compensation/Control".
//-----------------------------------------------------------------------------
bool _PVR_MPP_Set(u32 u4Flags, const DMX_PVRPLAY_STRUCT_T *prPVRPlay)
{
    uintptr_t ptrBufStart = 0, ptrBufEnd = 0;
    uintptr_t ptrPhyBufStart = 0, ptrPhyBufEnd = 0;
    DMX_PVRPLAY_STRUCT_T rPvrPlayStruct;

    DMX_ASSERT(prPVRPlay != NULL);

    if (u4Flags == PVRPLAY_FLAGS_NONE)
    {
        PVR_LOG_ERR("%s line %d fail for u4Flags == PVRPLAY_FLAGS_NONE\r\n", DMX_FUNC_NAME, DMX_LINE_NO);
        return TRUE;
    }

    _PVR_Lock();
    rPvrPlayStruct = _rPvrMPP;
    _PVR_Unlock();


    if (rPvrPlayStruct.eState == PVR_PLAY_STATE_PLAYING)
    {
        PVR_LOG_ERR("%s line %d fail for Now playing, stop first\r\n", DMX_FUNC_NAME, DMX_LINE_NO);
        return FALSE;
    }

    _PVR_Lock();

    if (u4Flags & PVRPLAY_FLAGS_TIMESTAMP)
    {
        rPvrPlayStruct.fgContainTimeStamp = prPVRPlay->fgContainTimeStamp;
        rPvrPlayStruct.fgIgnoreTimeStamp  = prPVRPlay->fgIgnoreTimeStamp;
        rPvrPlayStruct.u2TimeStampFreqDiv = prPVRPlay->u2TimeStampFreqDiv;

        _rPVRPlayCounter.fgUseTimestamp = !prPVRPlay->fgIgnoreTimeStamp;

        if (rPvrPlayStruct.fgContainTimeStamp)
        {
            rPvrPlayStruct.u1PacketSize = PVR_TS_PACKET_SIZE;
            rPvrPlayStruct.u1SyncOffset = 4;
            rPvrPlayStruct.u4PrologSize = PVR_TS_PACKET_SIZE * PVR_PROLOG_PACKETS;
        }
        else
        {
            rPvrPlayStruct.u1PacketSize = MPP_PACKET_SIZE;
            rPvrPlayStruct.u1SyncOffset = 0;
            rPvrPlayStruct.u4PrologSize = MPP_PACKET_SIZE * PVR_PROLOG_PACKETS;
        }
       
    }

    if (u4Flags & PVRPLAY_FLAGS_CALLBACK)
    {
        rPvrPlayStruct.pfnMPPNotify = prPVRPlay->pfnMPPNotify;
    }

    if (u4Flags & PVRPLAY_FLAGS_THRESHOLD)
    {
        rPvrPlayStruct.u4ThresholdSize = prPVRPlay->u4ThresholdSize;
        DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_THRESHOLD, rPvrPlayStruct.u4ThresholdSize);
    }

    if (u4Flags & PVRPLAY_FLAGS_MODE)
    {
        rPvrPlayStruct.eMode = prPVRPlay->eMode;
    }

    if (u4Flags & PVRPLAY_FLAGS_BUFFER)
    {
        if ((rPvrPlayStruct.ptrBufStart != 0) ||
                    (rPvrPlayStruct.ptrBufEnd != 0))
        {
            _PVR_Unlock();
            PVR_LOG_ERR("%s line %d fail for Buffer already allocated\r\n", DMX_FUNC_NAME, DMX_LINE_NO);
            return FALSE;
        }

        if (prPVRPlay->fgAllocBuf)
        {
#ifdef __linux__
            DMX_NewHwAlignMemory(prPVRPlay->u4BufSize, DMX_PVR_BUF_ALIGNMENT, ptrBufStart);
#else
            DMX_NewHwAlignMemory(prPVRPlay->u4BufSize, DMX_PVR_BUF_ALIGNMENT, (void *)ptrBufStart);
#endif // #ifdef __linux__
            if (ptrBufStart == 0)
            {
                PVR_LOG_ERR(TEXT("%s line %d fail for Memory allocation failed!\n"),
                    DMX_FUNC_NAME, DMX_LINE_NO);
                return FALSE;
            }
            ptrBufEnd = ptrBufStart + prPVRPlay->u4BufSize;
        }
        else
        {
            DMX_ASSERT(prPVRPlay->ptrBufStart != 0);
            ptrBufStart = prPVRPlay->ptrBufStart;
            ptrBufEnd = prPVRPlay->ptrBufStart + prPVRPlay->u4BufSize;
        }

        rPvrPlayStruct.fgAllocBuf = prPVRPlay->fgAllocBuf;
        rPvrPlayStruct.ptrBufStart = ptrBufStart;
        rPvrPlayStruct.ptrBufEnd = ptrBufStart + prPVRPlay->u4BufSize;
        rPvrPlayStruct.u4BufSize = prPVRPlay->u4BufSize;
        rPvrPlayStruct.ptrWp = ptrBufStart;
        rPvrPlayStruct.ptrRp = ptrBufStart;

        ptrPhyBufStart = DMX_PHYSICAL(ptrBufStart);
        ptrPhyBufEnd = DMX_PHYSICAL(ptrBufEnd);

        // Set playback buffer
        DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_START, ptrPhyBufStart);
        DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_END, ptrPhyBufEnd - 1);
        DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_RP, ptrPhyBufStart);
        DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_WP, ptrPhyBufStart);
    }

    _rPvrMPP = rPvrPlayStruct;

    _PVR_Unlock();

    return TRUE;
}


//-----------------------------------------------------------------------------
/** _PVR_MPP_Get
*/
//-----------------------------------------------------------------------------
bool _PVR_MPP_Get(u32 u4Flags, DMX_PVRPLAY_STRUCT_T *prPVRPlay)
{
    uintptr_t ptrWp, ptrRp;
    DMX_PVRPLAY_STRUCT_T* prPvrPlayStruct;

    DMX_ASSERT(prPVRPlay != NULL);

    if (u4Flags == PVRPLAY_FLAGS_NONE)
    {
        return TRUE;
    }

    _PVR_Lock();
    prPvrPlayStruct = &_rPvrMPP;
    _PVR_Unlock();


    if (u4Flags & PVRPLAY_FLAGS_TIMESTAMP)
    {
        prPVRPlay->fgContainTimeStamp = prPvrPlayStruct->fgContainTimeStamp;
        prPVRPlay->fgIgnoreTimeStamp = prPvrPlayStruct->fgIgnoreTimeStamp;
        prPVRPlay->u2TimeStampFreqDiv = prPvrPlayStruct->u2TimeStampFreqDiv;
    }

    if (u4Flags & PVRPLAY_FLAGS_THRESHOLD)
    {
        prPVRPlay->u4ThresholdSize = prPvrPlayStruct->u4ThresholdSize;
    }

    if (u4Flags & PVRPLAY_FLAGS_CALLBACK)
    {
        prPVRPlay->pfnMPPNotify = prPvrPlayStruct->pfnMPPNotify;
    }

    if (u4Flags & PVRPLAY_FLAGS_MODE)
    {
        prPVRPlay->eMode = prPvrPlayStruct->eMode;
    }

    if (u4Flags & PVRPLAY_FLAGS_BUFFER)
    {
        prPVRPlay->fgAllocBuf = prPvrPlayStruct->fgAllocBuf;
        prPVRPlay->ptrBufStart = prPvrPlayStruct->ptrBufStart;
        prPVRPlay->ptrBufEnd = prPvrPlayStruct->ptrBufEnd;
        prPVRPlay->u4BufSize = prPvrPlayStruct->u4BufSize;
        prPVRPlay->u4ThresholdSize = prPvrPlayStruct->u4ThresholdSize;

        _PVR_Lock();
        ptrRp = DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_RP);
        ptrWp = DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_WP);
        prPVRPlay->ptrRp = DMX_NONCACHE(ptrRp);
        prPVRPlay->ptrWp = DMX_NONCACHE(ptrWp);
        PVR_LOG_DBG("%s line %d -- MiniPVR Play Input Buffer's RP(0x%08x), RP(0x%08x)\r\n",
            DMX_FUNC_NAME, DMX_LINE_NO, DMX_NONCACHE(ptrRp), DMX_NONCACHE(ptrWp));
        _PVR_Unlock();

    }

    return TRUE;
}


//-----------------------------------------------------------------------------
/** _PVR_MPP_SetPort
*/
//-----------------------------------------------------------------------------
void _PVR_MPP_SetPort(PVR_MPP_PORT_T ePort)
{
    u32 u4Reg;

    PVR_LOG_DBG(TEXT("[MPP] %s(ePort: %d) -- enter\r\n"),
        DMX_FUNC_NAME, ePort);

    smp_mb();

    if (ePort == DMX_PVRPLAY_PORT_DBM)
    {
        PVR_LOG_DBG("\r\n\r\n[DMX][EMU] %s line %d -- DMX_SetDbm_InputSource(TsIdx(%u), PB)!\r\n",
                             DMX_FUNC_NAME, DMX_LINE_NO, u1TsIdx);
        //_PVR_SetDbm_InputSource(DMX_FVR_MM_MOVE_TSIDX, PVR_DBM_INPUT_PB);
    }
    else        // kind of redundant: C type-checking mechanism shall warn us
    {
        PVR_LOG_ERR(TEXT("[MPP] %s(ePort: %d) fail for invalid ePort\r\n"),
            DMX_FUNC_NAME, ePort);
        return;
    }
    
    u4Reg = DMXCMD_READ32(PVR_REG_PES_DBM_STEER_CTRL) & 0xFFFCFFFF;
    u4Reg |= (1 << 17);     // Enable PES/DBM path
    DMXCMD_WRITE32(PVR_REG_PES_DBM_STEER_CTRL, u4Reg);
    
     _PVR_Lock();
    _rPvrMPP.eMPPPortType = ePort;
    _PVR_Unlock();

    PVR_LOG_DBG(TEXT("[MPP] %s(ePort: %d) -- success\r\n"),
        DMX_FUNC_NAME, ePort);
}


//-----------------------------------------------------------------------------
/** _PVR_MPP_GetPort
*/
//-----------------------------------------------------------------------------
PVR_MPP_PORT_T _PVR_MPP_GetPort(void)
{
    PVR_MPP_PORT_T ePort;

    _PVR_Lock();
    ePort = _rPvrMPP.eMPPPortType;
    _PVR_Unlock();

    return ePort;
}

//-----------------------------------------------------------------------------
/** _DMX_PVRPlay_SetMoveMode
 */
//-----------------------------------------------------------------------------
bool _PVR_MPP_SetMoveMode(DMX_TSFMT_T eTSFmt, bool fgForce)
{
    bool fgContainTimestamp, fgUseTimestamp;
    DMX_PVRPLAY_STRUCT_T rPlay;
    DMX_PVRPLAY_STATE_T eState;

    _PVR_Lock();
    eState = _rPvrMPP.eState;
    _PVR_Unlock();

    if (eState == PVR_PLAY_STATE_PLAYING)
    {
        PVR_LOG_ERR(TEXT("PVRPLAY is in Playing mode.\r\n"));
        return FALSE;
    }

    if (fgForce)
    {
        if (_eDmxPVRTSFmt == eTSFmt)
        {
            return TRUE;
        }
    }

    fgContainTimestamp = FALSE;
    fgUseTimestamp = FALSE;

    if ((eTSFmt == DMX_TSFMT_192) || (eTSFmt == DMX_TSFMT_192_ENCRYPT))
    {
        fgContainTimestamp = TRUE;
    }
    else if (eTSFmt == DMX_TSFMT_TIMESHIFT)
    {
        fgContainTimestamp = TRUE;
        fgUseTimestamp = TRUE;
    }

    if (fgContainTimestamp && fgUseTimestamp)
    {
        rPlay.eMode = PVR_MPP_MODE_STREAM;
    }
    else
    {
        rPlay.eMode = PVR_MPP_MODE_SINGLE;
    }

    rPlay.fgContainTimeStamp = fgContainTimestamp;
    rPlay.fgIgnoreTimeStamp = !fgUseTimestamp;
    rPlay.u2TimeStampFreqDiv = DMX_PVRPLAY_TIMESTAMP_DIV_BASE;
    if (!_PVR_MPP_Set((u32)(PVRPLAY_FLAGS_MODE | PVRPLAY_FLAGS_TIMESTAMP), &rPlay))
    {
        return FALSE;
    }

    _PVR_Lock();
    _eDmxPVRTSFmt = eTSFmt;
    _PVR_Unlock();

    return TRUE;
}

//-----------------------------------------------------------------------------
/** _DMX_PVRPlay_GetBufPointer
*/
//-----------------------------------------------------------------------------
bool _PVR_MPP_GetBufPointer(DMX_PVRPLAY_BUFPTR_T *prPtr)
{
    if (prPtr == NULL)
    {
        return FALSE;
    }

    _PVR_Lock();
    prPtr->ptrHwWp = DMX_NONCACHE(DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_WP));
    prPtr->ptrHwRp = DMX_NONCACHE(DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_RP));
    
    prPtr->ptrSwWp = _rPvrMPP.ptrWp;
    prPtr->ptrSwRp = _rPvrMPP.ptrRp;
    _PVR_Unlock();

    return TRUE;
}


//-----------------------------------------------------------------------------
/** _DMX_PVRPlay_SetBuffer
*/
//-----------------------------------------------------------------------------
bool _PVR_MPP_SetBuffer(uintptr_t ptrBufStart, uintptr_t ptrBufEnd, uintptr_t ptrWp, uintptr_t ptrRp)
{
    DMX_ASSERT(ptrBufStart < ptrBufEnd);
    DMX_ASSERT((ptrWp >= ptrBufStart) && (ptrWp < ptrBufEnd));
    DMX_ASSERT((ptrRp >= ptrBufStart) && (ptrRp < ptrBufEnd));
    DMX_ASSERT((ptrBufEnd % 4) == 3);        // End - 1

    if (_rPvrMPP.eState != PVR_PLAY_STATE_STOP)
    {
        return FALSE;
    }

    _PVR_Lock();
    _rPvrMPP.ptrBufStart = ptrBufStart;
    _rPvrMPP.ptrBufEnd = ptrBufEnd + 1;
    _rPvrMPP.ptrWp = ptrWp;
    _rPvrMPP.ptrRp = ptrRp;

    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_START, DMX_PHYSICAL(ptrBufStart));
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_END, DMX_PHYSICAL(ptrBufEnd));
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_WP, DMX_PHYSICAL(ptrWp));
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_RP, DMX_PHYSICAL(ptrRp));
    
    _PVR_Unlock();

    return TRUE;
}

//-----------------------------------------------------------------------------
/** _DMX_SetFramerPktSize
 */
//-----------------------------------------------------------------------------
static void _PVR_MPP_ResetFramer(void)
{
    u32 u4Ctrl = 0, u4Reg = 0;
    u8 i = 0;
    bool  fgEnabled = FALSE;

    smp_mb();
    
    u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1);
    if ((u4Ctrl & 0x01) != 0)
    {
        PVR_LOG_DBG("%s line %d -- disable mini-pvr framer\r\n",
                        DMX_FUNC_NAME, DMX_LINE_NO);
        fgEnabled = TRUE;
        u4Ctrl &= ~0x01;
        DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Ctrl);  // diable framer
        // wait for framer state to idle
        for (i = 0; i < 100; i++)
        {
            u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1);
            if (((u4Reg >> 16) & 0xFF) == 0x01)
            {
                // in idle state
                break;
            }
            DMX_THREAD_DELAY(10);
        }

        if (i >= 100)
        {
            PVR_LOG_ERR(TEXT("[PVR] line %d fail in disable MiniPVR Framer\r\n"),
                DMX_LINE_NO);
            return;
        }
    }
        
    PVR_LOG_DBG("%s line %d -- Reset mini-pvr framer\r\n",
                DMX_FUNC_NAME, DMX_LINE_NO);
    smp_mb();
    
    u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1) & (~0x10);
    u4Ctrl |= 0x1 << 4; // Set Playback Steering Framer Reset
    DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Ctrl);

    u4Ctrl &= ~(0x1 << 4);
    DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Ctrl);

    // enable framer
    if (fgEnabled)
    {
        u4Ctrl = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1) | (0x1);
        DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Ctrl);
    }
}

//-----------------------------------------------------------------------------
/** _PVR_MPP_SetPacketSize
*
*/
//-----------------------------------------------------------------------------
bool _PVR_MPP_SetPacketSize(u16 u2PktSize, u16 u2PktOutSize)
{
    u32 u4Reg = 0;
    bool fgEnabled = FALSE;

	FUNC_ENTRY;
    
    PVR_LOG_DBG("%s line %d -- u2PktSize: 0x%04x, u2PktOutSize: 0x%04x\r\n",
        DMX_FUNC_NAME, DMX_LINE_NO, u2PktSize, u2PktOutSize);

    if (_au2PVRPlayFramerPktSize == u2PktSize)
    {
        return TRUE;
    }
    smp_mb();
    
    u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1);
    if ((u4Reg & 0x01) != 0)
    {
        u32 i;
        fgEnabled = TRUE;
        u4Reg &= ~0x01;
        DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Reg);  // diable framer
        // wait for framer state to idle
        for (i = 0; i < 100; i++)
        {
            u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1);
            if (((u4Reg >> 16) & 0xFF) == 0x01)
            {
                // in idle state
                break;
            }
            DMX_THREAD_DELAY(10);
        }

        if (i >= 100)
        {
            PVR_LOG_ERR(TEXT("[PVR] line %d fail in disable MiniPVR Framer\r\n"),
                DMX_LINE_NO);
            return FALSE;
        }
    }

    // Set PVR Framer's input packet size
    u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL2) & ~0xFF000000;
    u4Reg |= ((u32)(u2PktSize & 0xFF) << 24);
    DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL2, u4Reg);
    
    // Set PVR Framer's output packet size
    u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1) & ~0xFF000000;
    u4Reg |= ((u32)(u2PktOutSize & 0xFF) << 24);
    DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Reg);

    DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Reg | (1 << 6));
    DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Reg | (1 << 5));

    u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1);
    u4Reg |= 0x1 << 2;        // enable pre-byte scheme
    DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Reg);
    
    PVR_LOG_DBG("%s line %d -- MiniPVR Framer Control 1 Reg: 0x%08x\r\n",
        DMX_FUNC_NAME, DMX_LINE_NO, DMXCMD_READ32(PVR_REG_FRAMER_CTRL1));
    
    PVR_LOG_DBG("%s line %d -- MiniPVR Framer Control 2 Reg: 0x%08x\r\n",
        DMX_FUNC_NAME, DMX_LINE_NO, DMXCMD_READ32(PVR_REG_FRAMER_CTRL2));
    
    //Reset Framer
    _PVR_MPP_ResetFramer();
    
    PVR_LOG_DBG("%s line %d -- MiniPVR Framer Control 1 Reg: 0x%08x, After reset\r\n",
        DMX_FUNC_NAME, DMX_LINE_NO, DMXCMD_READ32(PVR_REG_FRAMER_CTRL1));
    
    PVR_LOG_DBG("%s line %d -- MiniPVR Framer Control 2 Reg: 0x%08x, After reset\r\n",
        DMX_FUNC_NAME, DMX_LINE_NO, DMXCMD_READ32(PVR_REG_FRAMER_CTRL2));
    
    PVR_LOG_DBG("%s line %d -- FTI Configuration Reg: 0x%08x, After reset\r\n",
        DMX_FUNC_NAME, DMX_LINE_NO, DMXCMD_READ32(PVR_REG_CONFIG2));

    if (fgEnabled)
    {
        u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1) | (0x1);
        DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4Reg);
    }

    _au2PVRPlayFramerPktSize = u2PktSize;

    return TRUE;
    
}


//-----------------------------------------------------------------------------
/** _PVR_MPP_SetSyncOffset
*
* The function _PVR_MPP_SetPacketSize() calculate a default offset value for the Sync byte.
* Call this function to change the offset value if the default value does not meet user's requirement.
*/
//-----------------------------------------------------------------------------
bool _PVR_MPP_SetSyncOffset(u8 u1Offset)
{
    u8 u1MinOffset = 0, u1MaxOffset;

    u1MaxOffset = _rPvrMPP.u1PacketSize - (u8)188;

    if ((u1MinOffset <= u1Offset) && (u1Offset <= u1MaxOffset))
    {
        _rPvrMPP.u1SyncOffset = u1Offset;
        return TRUE;
    }

    PVR_LOG_ERR(TEXT("[MPP] %s line %d fail for Sync byte offset (%d) is not supported\r\n"),
        DMX_FUNC_NAME, DMX_LINE_NO, u1Offset);

    return FALSE;
}


//-----------------------------------------------------------------------------
/** _PVR_MPP_Free
*/
//-----------------------------------------------------------------------------
bool _PVR_MPP_Free(void)
{
    if (_rPvrMPP.eState != PVR_PLAY_STATE_STOP)
    {
        PVR_LOG_ERR(TEXT("[MPP] %s line %d fail, Please stop MPP before its buffer is freed\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO);
        return FALSE;
    }

    _MPP_Reset();  // Only reset some HW registers (RIP, WP, AP, and etc).

    if (!_PVR_MPP_FreeBuf())
    {
        PVR_LOG_ERR(TEXT("[MPP] %s line %d fail in _MPP_FreeBuf\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO);
        return FALSE;   // Current implementation will never fall into here.
    }

    return TRUE;
}

// Make Source End Address > WPtr
void _PVR_MPP_FixDMAEndAddr(uintptr_t *pptrEA, uintptr_t ptrWPtr)
{
    if ((*pptrEA - ptrWPtr) > DMX_PVR_BUF_ALIGNMENT)
    {
        return;
    }

    *pptrEA += DMX_PVR_BUF_ALIGNMENT;

    if (ptrWPtr >= *pptrEA)
    {
        _PVR_MPP_FixDMAEndAddr(pptrEA, ptrWPtr);
    }
}

bool _PVR_MPP_SetBufNfy(uintptr_t ptrBufStart, u32 u4BufSz,
	PFN_MPP_NOTIFY pfnNotify)
{
	
	DMX_PVRPLAY_STRUCT_T rPlay;
	
	x_memset(&rPlay, 0, sizeof(rPlay));
	
	// Play init
	rPlay.fgAllocBuf = FALSE;
	rPlay.ptrBufStart = ptrBufStart;
	rPlay.u4BufSize = u4BufSz;
	rPlay.u4ThresholdSize = 0x10000000; 	// Not usable
	rPlay.pfnMPPNotify = pfnNotify;
	rPlay.eMode = PVR_MPP_MODE_SINGLE;
	rPlay.fgContainTimeStamp = FALSE;
	rPlay.fgIgnoreTimeStamp = TRUE;
	rPlay.u2TimeStampFreqDiv = DMX_PVRPLAY_TIMESTAMP_DIV_BASE;

	PVR_LOG_DBG("%s line %d -- Set MiniPVR Playback Input Buffer(SA(0x%08x), Sz(0x%08x))\r\n", 
		DMX_FUNC_NAME, DMX_LINE_NO, ptrBufStart, u4BufSz);
	if (!_PVR_MPP_Set((u32)(PVRPLAY_FLAGS_THRESHOLD |
		PVRPLAY_FLAGS_BUFFER |
	   PVRPLAY_FLAGS_CALLBACK |
	   PVRPLAY_FLAGS_MODE |
	   PVRPLAY_FLAGS_TIMESTAMP), &rPlay))
	{
		PVR_LOG_ERR("%s line %d fail in _DMX_PVRPlay_Set, (SA(0x%08x), Sz(0x%08x))\r\n", 
			DMX_FUNC_NAME, DMX_LINE_NO, ptrBufStart, u4BufSz);
		return FALSE;
	}
	
	return TRUE;
}
//-----------------------------------------------------------------------------
/** _PVR_MPP_SingleMove
*   For the Single move mode.
*
*   The parameter ptrBufEnd must be the address right next to the real end of  the buffer.
*   In other words, the data byte addressed by ptrBufEnd does not belong to the buffer.
*   Both start/end addresses must be aligned to 16-byte boundary.
*   Both Read/Write pointers must be aligned to 4-byte boundary.
*/
// The addresses passed into this function shall be DMX_NONCACHE addresses.
//-----------------------------------------------------------------------------
bool _PVR_MPP_SingleMove(uintptr_t ptrBufferSa, uintptr_t ptrBufferEa,
                         uintptr_t ptrSrcAddr, u32 u4Size)
{
    uintptr_t ptrPhyBufStart, ptrPhyBufEnd, ptrPhyRp, ptrPhyWp;
    bool fgRet;
    u32 u4SrcDataSz, u4BufSize;
    u8 u1SkipBytes;

    //DMX_ASSERT(_PVR_IsAligned(ptrBufferSa, DMX_PVR_BUF_ALIGNMENT));
    //DMX_ASSERT(_PVR_IsAligned(ptrBufferEa, DMX_PVR_BUF_ALIGNMENT));
    // PVR WP, RP is byte-alignment

    DMX_ASSERT(_rPvrMPP.eMode == PVR_MPP_MODE_SINGLE);
    DMX_ASSERT(_rPvrMPP.fgIgnoreTimeStamp);
    

    if (_rPvrMPP.eState != PVR_PLAY_STATE_STOP)
    {
        return FALSE;
    }

    if ((ptrBufferSa < 1) ||
        (ptrBufferEa < 1) ||
        (ptrSrcAddr < 1) ||
        (ptrBufferEa - ptrBufferSa < u4Size))
    {
        PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid args(u4BufSa: 0x%x, u4BufEa: 0x%x, u4SrcSz: 0x%x)\r\n"),
            DMX_FUNC_NAME, ptrBufferSa, ptrBufferEa, u4Size);
        DMX_ASSERT(0);
        return FALSE;
    }

    // Buffer Start Address
    ptrPhyBufStart = DMX_PHYSICAL(ptrBufferSa);
    u1SkipBytes = 0;
    if (!_PVR_IsAligned(ptrPhyBufStart, DMX_PVR_BUF_ALIGNMENT))
    {
        ptrPhyBufStart = _PVR_Align_Dec(ptrPhyBufStart, DMX_PVR_BUF_ALIGNMENT, &u1SkipBytes);
    }
    if (!_PVR_IsAligned(ptrPhyBufStart, DMX_PVR_BUF_ALIGNMENT))
    {
        DMX_ASSERT(0);
        return FALSE;
    }

    // Source Data Size MPP_PACKET_SIZE aligned
    u4SrcDataSz = u4Size;
    u4SrcDataSz += (MPP_PACKET_SIZE - 1);
    u4SrcDataSz -= u4SrcDataSz % MPP_PACKET_SIZE;

    // Buffer End Address
    if (ptrSrcAddr + u4SrcDataSz > ptrBufferEa)
    {
        ptrBufferEa = ptrSrcAddr + u4SrcDataSz;
    }

    ptrPhyBufEnd = DMX_PHYSICAL(ptrBufferEa);
    if (!_PVR_IsAligned(ptrPhyBufEnd, DMX_PVR_BUF_ALIGNMENT))
    {
        ptrPhyBufEnd = _PVR_Align(ptrPhyBufEnd, DMX_PVR_BUF_ALIGNMENT);
    }
    if (!_PVR_IsAligned(ptrPhyBufEnd, DMX_PVR_BUF_ALIGNMENT))
    {
        DMX_ASSERT(0);
        return FALSE;
    }

    
    // Buffer Read Address
    ptrPhyRp = DMX_PHYSICAL(ptrSrcAddr);

    if (!((ptrPhyRp >= ptrPhyBufStart) && (ptrPhyRp < ptrPhyBufEnd)))
    {
        PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid args (PhyBufStart: 0x%x, PhyBufEnd: 0x%x, PhyBufRp: 0x%x, Size: 0x%x\r\n"),
            DMX_FUNC_NAME, ptrPhyBufStart, ptrPhyBufEnd, ptrPhyRp, u4Size);
        DMX_ASSERT(0);
        return FALSE;
    }

    ptrPhyWp = ptrPhyRp + u4SrcDataSz;

    // Modify Buffer End Address, if the EA-WP <= DDI_BUF_ALIGNEMENT
    _PVR_MPP_FixDMAEndAddr(&ptrPhyBufEnd, ptrPhyWp);

    u4BufSize = ptrPhyBufEnd - ptrPhyBufStart;

    PVR_LOG_DBG(TEXT("[PVR] %s -- PhyBufStart: 0x%x, PhyBufEnd: 0x%x, PhyBufRp: 0x%x, Size: 0x%x\r\n"),
        DMX_FUNC_NAME, ptrPhyBufStart, ptrPhyBufEnd, ptrPhyBufEnd, u4Size);
    
    // PVR Start
    _PVR_Lock();
    _rPvrMPP.ptrBufStart = ptrBufferSa;
    _rPvrMPP.ptrBufEnd = ptrBufferSa + u4BufSize;
    _rPvrMPP.u4BufSize = u4BufSize;
    _rPvrMPP.ptrRp = ptrSrcAddr;
    _rPvrMPP.ptrWp = _rPvrMPP.ptrRp + (ptrPhyBufEnd - ptrPhyBufStart);
    
    _rPvrMPP.u4ThresholdSize = 0;
	
	PVR_LOG_DBG(TEXT("[PVR] %s (PhyBufStart: 0x%x, PhyBufEnd: 0x%x, PhyBufRp: 0x%x, ptrPhyWp: 0x%x\r\n"),
            DMX_FUNC_NAME, ptrPhyBufStart, ptrPhyBufEnd, ptrPhyRp, ptrPhyWp);
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_START, ptrPhyBufStart);
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_END, ptrPhyBufEnd);
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_RP, ptrPhyRp);
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_WP, ptrPhyWp);

    //if config, will generate a interrupt, generally, this value is equal to data size.
    //DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_THRESHOLD, prData->u4FrameSize);

    _PVR_Unlock();

    if (!_PVR_MPP_Start())
    {
        return FALSE;
    }

    fgRet = TRUE;

    //VERIFY(OSR_OK == x_sema_lock(_hDMXPVRPlaySema, X_SEMA_OPTION_WAIT));
    #if 0
    if (!_PVR_MPP_Stop(TRUE))
    {
        UTIL_Printf("%s line %d fail in _DMX_PVRPlay_Stop(TRUE)\r\n", __FUNCTION__, __LINE__);
        fgRet = FALSE;
    }
    #endif
    return fgRet;
}


//-----------------------------------------------------------------------------
/** _DMX_PVRPlay_Start
 */
//-----------------------------------------------------------------------------
bool _PVR_MPP_Start(void)
{
    u32 u4RegControl;
    DMX_PVRPLAY_STRUCT_T* prPvrPlayStruct;

    _PVR_Lock();
    prPvrPlayStruct = &_rPvrMPP;
    _PVR_Unlock();

	PVR_LOG_DBG("Enter %s line %d .\r\n",
            DMX_FUNC_NAME, DMX_LINE_NO);
    if (prPvrPlayStruct->eState != PVR_PLAY_STATE_STOP)
    {
        PVR_LOG_ERR("%s line %d fail for PVR play already enable!\r\n",
            DMX_FUNC_NAME, DMX_LINE_NO);
        return FALSE;
    }

    if (prPvrPlayStruct->u4BufSize == 0)
    {
        PVR_LOG_ERR("%s line %d fail for No buffer allocated!\r\n",
                DMX_FUNC_NAME, DMX_LINE_NO);
        return FALSE;
    }

    ///////// Set framer ///////////
    //Set TimeStamp Threshold

    DMXCMD_WRITE32(PVR_REG_PVR_TIMESTAMP_THRESHOLD, 0xFFFFFFFF);


    //Buffer Threshold
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_THRESHOLD, 0xFFFFFFFF);

    //Output Steering Logic
    u4RegControl = DMXCMD_READ32(PVR_REG_PES_DBM_STEER_CTRL) & 0xFFFCFFFF;
    u4RegControl |= (1 << 17);
    DMXCMD_WRITE32(PVR_REG_PES_DBM_STEER_CTRL, u4RegControl);

    u4RegControl = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1) | (0x1); // Enable MiniPVR Playback Framer
    DMXCMD_WRITE32(PVR_REG_FRAMER_CTRL1, u4RegControl);

    
    u4RegControl = DMXCMD_READ32(PVR_REG_FRAMER_CTRL1);

    PVR_LOG_DBG("%s line %d -- MiniPVR Framer's SyncMode(%s), Enable(%s)\r\n",
        DMX_FUNC_NAME, DMX_LINE_NO,
        ((0 != (u4RegControl & 0x02)) ? "ExternalSync" : "InternalSync"),
        ((0 != (u4RegControl & 0x01)) ? "Enable" : "Disable"));

    //Mini PVR Playback config
    //       (1 << 14) |                                     // Enable interrupt 
    u4RegControl = 0x0;
    u4RegControl |= (((u32)prPvrPlayStruct->u2TimeStampFreqDiv) << 16) |
		(1 << 14) |                                     // Enable interrupt 
        (1 << 6) |                                      // Enable bit
        ((prPvrPlayStruct->fgIgnoreTimeStamp ? 1: 0) << 3) |   // Ignore timestamp
        ((prPvrPlayStruct->fgContainTimeStamp ? 1 : 0) << 0);  // Contain timestamp

    u4RegControl |= 0x200;          // latest playback TS packet will be ignored time stamp comparison when time stamp is inserted.
    u4RegControl |= 0x20;           // playback buffer enable
//#ifdef CC_DMX_SET_TIMESTAMP_RELOAD_THRESHOLD
    u4RegControl |= 0x10;           // save ts control
    DMXCMD_WRITE32(PVR_REG_PVR_TIMESTAMP_THRESHOLD, 0x5265C0);  // 0.2 second
//#endif
    DMXCMD_WRITE32(PVR_REG_PVR_CONTROL, u4RegControl);

    prPvrPlayStruct->eState = PVR_PLAY_STATE_PLAYING;
	PVR_LOG_DBG("Exit %s line %d .\r\n",
            DMX_FUNC_NAME, DMX_LINE_NO);
    return TRUE;
}

//-----------------------------------------------------------------------------
/** _DmxPVRPlay_DoStop
 */
//-----------------------------------------------------------------------------
static bool _PVR_MPP_DoStop(DMX_PVRPLAY_PTR_T *prPtr)
{
    uintptr_t ptrWp, ptrRp;
    u32 u4NewRp;
    DMX_PVRPLAY_STRUCT_T* prPvrPlayStruct;

    ptrWp = 0;
    ptrRp = 0;
    u4NewRp = 0;

    _PVR_Lock();
    prPvrPlayStruct = &_rPvrMPP;
    _PVR_Unlock();

    if (prPvrPlayStruct->eState != PVR_PLAY_STATE_PLAYING)
    {
        PVR_LOG_INFO("%s line %d -- PVR play already stop!\r\n", DMX_FUNC_NAME, DMX_LINE_NO);
        return TRUE;
    }
    
    if (prPvrPlayStruct->fgIgnoreTimeStamp)
    {
        // Disable PVR play
        _PVR_Lock();
        ptrWp = DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_WP);
        DMXCMD_WRITE32(PVR_REG_PVR_CONTROL, 0);
        _PVR_Unlock();
    }
    else
    {
        //if wp != rp, will DMA all data?
        // Disable PVR play
        DMXCMD_WRITE32(PVR_REG_PVR_CONTROL, 0);
    }

    if (prPtr != NULL)
    {
        _PVR_Lock();
        ptrRp = DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_RP);
        ptrWp = DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_WP);
        prPtr->ptrRp = DMX_NONCACHE(ptrRp);
        prPtr->ptrWp = DMX_NONCACHE(ptrWp);
        _PVR_Unlock();
    }

    prPvrPlayStruct->eState = PVR_PLAY_STATE_STOP;

    return TRUE;
}

//-----------------------------------------------------------------------------
/** _DMX_PVRPlay_Stop
 */
//-----------------------------------------------------------------------------
bool _PVR_MPP_Stop(bool fgForce)
{
    //LOG(3, "%s, force:%d\r\n", __FUNCTION__, (INT32)fgForce);

    if (!_PVR_MPP_DoStop(NULL))
    {
        return FALSE;
    }

    if (fgForce)
    {
        _MPP_Reset();
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
/** _DMX_PVRPlay_FreeBuf
*/
//-----------------------------------------------------------------------------
bool _PVR_MPP_FreeBuf(void)
{
    bool fgAllocBuf;
    u32 ptrBufStart;
    DMX_PVRPLAY_STRUCT_T* prPvrPlayStruct;

    _PVR_Lock();
    prPvrPlayStruct = &_rPvrMPP;
    _PVR_Unlock();

    _PVR_Lock();
    fgAllocBuf = prPvrPlayStruct->fgAllocBuf;
    ptrBufStart = prPvrPlayStruct->ptrBufStart;
    _PVR_Unlock();

    if (fgAllocBuf && (ptrBufStart != 0))
    {
        DMX_FreeHwMemory((void *)ptrBufStart);
    }

    _PVR_Lock();
    prPvrPlayStruct->ptrBufStart= 0x0;
    prPvrPlayStruct->ptrBufEnd = 0x0;
    prPvrPlayStruct->ptrWp = 0x0;
    prPvrPlayStruct->ptrRp = 0x0;

    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_START, 0x0);
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_END, 0x0);
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_RP, 0x0);
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_WP, 0x0);

    _PVR_Unlock();

    return TRUE;
}

//-----------------------------------------------------------------------------
/** _DMX_PVRPlay_FlushBuf
*/
//-----------------------------------------------------------------------------
bool _PVR_MPP_FlushBuf(void)
{
    u32 ptrBufStart, u4BufPhyStart;
    DMX_PVRPLAY_STRUCT_T* prPvrPlayStruct;

    _PVR_Lock();
    prPvrPlayStruct = &_rPvrMPP;
    ptrBufStart = prPvrPlayStruct->ptrBufStart;
    u4BufPhyStart = DMX_PHYSICAL(ptrBufStart);

    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_RP, u4BufPhyStart);
    DMXCMD_WRITE32(PVR_REG_PVR_PLAY_BUF_WP, u4BufPhyStart);

    prPvrPlayStruct->ptrRp = ptrBufStart;
    prPvrPlayStruct->ptrWp = ptrBufStart;
    _PVR_Unlock();

    return TRUE;
}

//-----------------------------------------------------------------------------
/** _DMX_PVRPlay_GetCounter
 */
//-----------------------------------------------------------------------------
void _PVR_MPP_GetCounter(DMX_PVRPLAY_COUNTER_T *prCounter)
{
    u32 ptrRp, ptrWp;

    if (prCounter != NULL)
    {
        _PVR_Lock();

        ptrWp = DMX_NONCACHE(DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_WP));
        ptrRp = DMX_NONCACHE(DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_RP));

        _rPVRPlayCounter.u4DataSize = DMX_DATASIZE(ptrRp, ptrWp, _rPvrMPP.u4BufSize);
        *prCounter = _rPVRPlayCounter;
        _PVR_Unlock();
    }
}
//-----------------------------------------------------------------------------
/** _DMX_PVRPlay_ResetCounter
 */
//-----------------------------------------------------------------------------
void _PVR_MPP_ResetCounter(void)
{
    _PVR_Lock();
    x_memset((void*)&_rPVRPlayCounter, 0, sizeof(DMX_PVRPLAY_COUNTER_T));
    _PVR_Unlock();
}

void _PVR_MPP_EndSingleMove(void)
{
    PVR_INPUT_TYPE_T eInputType;

    eInputType = _PVR_GetInputType();
    if (PVR_IN_PLAYBACK_MM != eInputType)
    {
        PVR_LOG_ERR(TEXT("[PVR] %s fail for Incorrect input type(%d)!\r\n"),
            DMX_FUNC_NAME, eInputType);
        return;
    }

    if (PVR_MPP_MODE_SINGLE != _rPvrMPP.eMode)
    {
        PVR_LOG_ERR(TEXT("[PVR] %s fail, Please switch MPP to SINGLE mode first.\r\n"),
            DMX_FUNC_NAME);
        return;
    }

    // Disable PVR play
    DMXCMD_WRITE32(PVR_REG_PVR_CONTROL, 0);

    _PVR_Lock();
    _rPvrMPP.eState = PVR_PLAY_STATE_STOP;       
    _PVR_Unlock();

    //_PVR_MPP_SetDMAInt(TRUE, FALSE);  // enable Emtpy Int; disable Alert Int

    // Reset MPP to make ReadPtrInit is not 4 Bytes Align.
    _MPP_Reset();

    // Reset DBM for next Single Move
    if (!_PVR_ResetDbmSafely())
    {
        PVR_LOG_ERR(TEXT("%s fail in _PVR_ResetDbmSafely\r\n"), DMX_FUNC_NAME);
    }
}

bool _PVR_MPP_SetISREventHandle(HANDLE_T hEvent)
{
    _rPvrMPP.hMPPNotifyEG = hEvent;
    return TRUE;
}

bool _PVR_MPP_DumpInfo(void)
{
    u32 u4Sa, u4Ea, ptrRp, ptrWp;
	//DMX_PHYSICAL
    PVR_LOG_ERR(TEXT("[PVR] MPP Before DMA -- SrcBufSa: 0x%x, Sz: 0x%x, Rp: 0x%x, Wp: 0x%x\r\n"),
        DMX_PHYSICAL(_rPvrMPP.ptrBufStart), _rPvrMPP.u4BufSize,
        DMX_PHYSICAL(_rPvrMPP.ptrRp), DMX_PHYSICAL(_rPvrMPP.ptrWp));
    u4Sa = DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_START);
    u4Ea = DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_END);
    ptrRp = DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_RP);
    ptrWp = DMXCMD_READ32(PVR_REG_PVR_PLAY_BUF_WP);
    PVR_LOG_ERR(TEXT("[PVR] MPP After DMA -- SrcBufSa: 0x%x, Ea: 0x%x, Rp: 0x%x, Wp: 0x%x\r\n"),
        u4Sa, u4Ea, ptrRp, ptrWp);

    return TRUE;
}

