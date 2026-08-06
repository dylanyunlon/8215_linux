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
 * @file dmx_pvr_mini.h
 *
 * @par Project
 *    MT3360
 *
 * @par Description
 *    Demuxer pvr ddi related structure, macro, interfaces declarations
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef _DMX_PVR_MPP_H_
#define _DMX_PVR_MPP_H_

//-----------------------------------------------------------------------------
// Include files
//-----------------------------------------------------------------------------

#include "dmx_pvr_if.h"
#include "dmx_pvr.h"
#include "x_hal_ic.h"

//-----------------------------------------------------------------------------
// Configurations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define MPP_POINTER_ALIGNMENT                   (4)
#define DMX_PVR_BUF_ALIGNMENT                   (16)

#define MPP_PACKET_SIZE                         (188)

#define DMX_PVR_PLAY_ISR                        0

#define MPEG_TS_HEADER_SIZE                     (4)
#define PVR_TS_PACKET_SIZE                      (192)
#define PVR_PROLOG_PACKETS                      (3)

#define PVR_ALLOC_BUF_ALIGNMENT                 32

#define DMX_PVRP_STOP_PKT_THRESHOLD         (20)

//
// Mini PVR registers
//

//-----------------------------------------------------------------------------
// Type definitions
//-----------------------------------------------------------------------------
#define PVR_MPP_EVENT_GROUP_NAME        ("PVR_MPP_NOTIFY_EG")

typedef enum
{
    PVR_MPP_NOTIFY_EV_BUF_EMPTY = 1 << 0,
    PVR_MPP_NOTIFY_EV_BUF_ALERT = 1 << 1,
    PVR_MPP_NOTIFY_EV_BUF_EXIT  = 1 << 2
} PVR_MPP_NOTIFY_EVENT_T;

typedef bool (*PFN_MPP_NOTIFY)(MPP_EVENT_CODE_T);

//-----------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------
/// PVR Play
#define PVRPLAY_FLAGS_BUFFER               0x00000001
#define PVRPLAY_FLAGS_TIMESTAMP            0x00000002
#define PVRPLAY_FLAGS_THRESHOLD            0x00000004
#define PVRPLAY_FLAGS_CALLBACK             0x00000008
#define PVRPLAY_FLAGS_MODE                 0x00000010
#define PVRPLAY_FLAGS_ALL                  0xFFFFFFFF
#define PVRPLAY_FLAGS_NONE                 0x00000000


typedef enum
{
	DMX_PVRPLAY_PORT_DBM
} PVR_MPP_PORT_T;

typedef enum
{
    PVR_MPP_MODE_SINGLE,        // Move a single chunk of data
    PVR_MPP_MODE_STREAM,        // Streaming data may have several chunks
    PVR_MPP_MODE_NONBLOCKING    // Non-blocking MPP DMA operation
} PVR_MPP_MODE_T;

typedef enum
{
    PVR_PLAY_STATE_STOP,     // DMA is de-activated (in the Stop state)
    PVR_PLAY_STATE_PLAYING   // DMA is activated (in the Play state)
} DMX_PVRPLAY_STATE_T;

typedef struct
{
    uintptr_t ptrRp;
    uintptr_t ptrWp;
} DMX_PVRPLAY_PTR_T;


typedef struct
{
    PVR_MPP_MODE_T eMode;   // Single mode or Stream mode
    bool   fgAllocBuf;
    uintptr_t ptrBufAddr;
    u32 u4BufSize;
    u32 u4Threshold;     // INT when buffer is decreased to this value
    u32 u4DataSize;      // how much data is in the mini PVR buffer

    PFN_MPP_NOTIFY pfnMPPNotify;
                            // The callback needs to return FALSE if error.
                            // Otherwise, return TRUE.
} PVR_MPP_T;

//current RP and WP
typedef struct
{
    uintptr_t ptrHwRp;
    uintptr_t ptrHwWp;
    uintptr_t ptrSwRp;
    uintptr_t ptrSwWp;
} DMX_PVRPLAY_BUFPTR_T;


typedef struct
{
    bool   fgUseTimestamp;
    u32 u4SingleCount;
    u32 u4SingleSize;
    u32 u4SingleCheck;
    u32 u4MoveCount;
    u32 u4MoveSize;
    u32 u4MoveCheck;
    u32 u4ShiftingCount;
    u32 u4DataSize;
} DMX_PVRPLAY_COUNTER_T;

typedef enum
{
    DMX_TSFMT_NONE,
    DMX_TSFMT_188,
    DMX_TSFMT_192,
    DMX_TSFMT_192_ENCRYPT,
    DMX_TSFMT_204,
    DMX_TSFMT_TIMESHIFT,
} DMX_TSFMT_T;

/// MiniPVR
typedef struct
{
    PVR_MPP_PORT_T  eMPPPortType;
    PVR_MPP_MODE_T  eMode;
    DMX_PVRPLAY_STATE_T eState;
    bool   fgAllocBuf;          // Indicate if MPP allocates buffer for users.
    uintptr_t ptrBufStart;
    uintptr_t ptrBufEnd;
    u32 u4BufSize;           // buffer size
    u32 u4ThresholdSize;         // copy data if the threshold of free space is met
    uintptr_t ptrRp;                // read pointer to buffer
    uintptr_t ptrWp;                // write pointer to buffer
    u8  u1PacketSize;
	u8  u1PrevSyncOffset;
    u8  u1SyncOffset;
    bool   fgMPPISRInited;
	
	u8  *pu1Prolog;
	u32 u4PrologSize;	// _u1PacketSize * PVR_PROLOG_PACKETS

    bool   fgIgnoreTimeStamp;
    bool   fgContainTimeStamp;
    u16 u2TimeStampFreqDiv;
    
    HANDLE       hMPPNotifyEG;
    PFN_MPP_NOTIFY pfnMPPNotify;
} DMX_PVRPLAY_STRUCT_T;

//-----------------------------------------------------------------------------
// Prototype  of inter-file functions
//-----------------------------------------------------------------------------

//
// MPP
//
EXTERN void _PVR_MPP_SetFramerMode(bool fgExtSync, bool fgEnable);

EXTERN bool _PVR_MPP_Init(void);

EXTERN bool _PVR_MPP_Set(u32 u4Flags, const DMX_PVRPLAY_STRUCT_T *prMPP);

EXTERN bool _PVR_MPP_Get(u32 u4Flags, DMX_PVRPLAY_STRUCT_T *prMPP);

EXTERN bool _PVR_MPP_DeInit(void);

EXTERN void _PVR_MPP_SetPort(PVR_MPP_PORT_T ePort);

EXTERN PVR_MPP_PORT_T _PVR_MPP_GetPort(void);

EXTERN bool _PVR_MPP_SetMoveMode(DMX_TSFMT_T eTSFmt, bool fgForce);

EXTERN bool _PVR_MPP_GetBufPointer(DMX_PVRPLAY_BUFPTR_T *prPtr);

EXTERN bool _PVR_MPP_SetPacketSize(u16 u2PktSize, u16 u2PktOutSize);

EXTERN bool _PVR_MPP_SetSyncOffset(u8 u1Offset);

EXTERN bool _PVR_MPP_Free(void);

EXTERN bool _PVR_MPP_SetBufNfy(uintptr_t ptrBufStart, u32 u4BufSz,
	PFN_MPP_NOTIFY pfnNotify);

EXTERN bool _PVR_MPP_SingleMove(uintptr_t ptrBufferSa, uintptr_t ptrBufferEa,
                         uintptr_t ptrSrcAddr, u32 u4Size);

EXTERN bool _PVR_MPP_Stop(bool fgForce);

EXTERN bool _PVR_MPP_Start(void);

EXTERN bool _PVR_MPP_FreeBuf(void);

EXTERN bool _PVR_MPP_FlushBuf(void);

EXTERN bool _PVR_MPP_InitISR(void);

EXTERN void _PVR_MPP_GetCounter(DMX_PVRPLAY_COUNTER_T *prCounter);

EXTERN void _PVR_MPP_ResetCounter(void);

EXTERN void _PVR_MPP_EndSingleMove(void);

EXTERN void _PVR_MPP_FixDMAEndAddr(u32 *pu4EA, u32 u4WPtr);

EXTERN bool _PVR_MPP_SetISREventHandle(HANDLE hEvent);

EXTERN bool _PVR_MPP_SetBuffer(uintptr_t ptrBufStart, uintptr_t ptrBufEnd, uintptr_t ptrWp, uintptr_t ptrRp);

EXTERN bool _PVR_MPP_DumpInfo(void);

#endif  // DMX_MPP_H

