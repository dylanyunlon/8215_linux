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

#ifndef DRV_PSR_H
#define DRV_PSR_H

#include "x_typedef.h"
#include "x_lint.h"

LINT_EXT_HEADER_BEGIN

#include "x_os.h"

LINT_EXT_HEADER_END

//===========================================================================
//
// Parser interfaces
//
//===========================================================================

//---------------------------------------------------------------------
// Parameters

//---------------------------------------------------------------------
// Constant definitions

#define MAX_CHANNEL_NUM                 8
#define MAX_VLD_NUM                     2
#define MAX_DSP_NUM                     2
#define MAX_PIC_HEADERS                 16
#define MAX_PIC_HEADER_COUNT            256

/*
#define HD_VIDEO_BUF_SIZE               1230000             // 1.17MB
#define SD_VIDEO_BUF_SIZE               230000              // 224KB
*/

#ifndef HD_VIDEO_BUF_SIZE
#define HD_VIDEO_BUF_SIZE               0x300000            // 3MB
#endif

#define SD_VIDEO_BUF_SIZE               0x60000             // 384KB

#define MAX_HD_VIDEO_NUM                2
#define MAX_SD_VIDEO_NUM                8

#define AUDIO_BUF_BOUNDARY              0x1000000           // 16MB

#define PES_HEADER_BUF_SIZE             1024
#define HEADER_BUF_ALIGNMENT            256
#define FIFO_ALIGNMENT                  256

#define FIFO_ALMOST_FULL_THRESHOLD      80                  // In percentage

// Soft parser parameters
#define PSR_SOFT_FIFO_ALIGNMENT         256
#define PSR_MAX_PDMA_SIZE               0xf7ffff            // 15.5M bytes


//---------------------------------------------------------------------
// Type definitions

// Stream type
typedef enum
{
    PSR_STREAM_UNKNOWN,
    PSR_STREAM_VIDEO,
    PSR_STREAM_AUDIO
} PSR_STREAM_T;

typedef enum
{
    PSR_PIC_UNKNOWN,
    PSR_PIC_I,
    PSR_PIC_P,
    PSR_PIC_B,
    PSR_PIC_SEQ_START,
    PSR_PIC_SEQ_END,
    PSR_PIC_GOP
} PSR_PICTURE_TYPE_T;

typedef struct
{
    UINT64              u8Pts;
    UINT64              u8Dts;
    PSR_PICTURE_TYPE_T  eType;
    UINT32              u4Addr;
//    UINT32              u4Postpone;
} PIC_HEADER_T;

typedef struct
{
//    UINT64              u8Pts;
//    UINT64              u8Dts;
    UINT32              u4FifoStart;
    UINT32              u4FifoEnd;
    UINT32              u4HeaderNum;
    PIC_HEADER_T        arHeader[MAX_PIC_HEADERS];
} PSR_VIDEO_PES_T;

typedef struct
{
    UINT64              u8Pts;
    UINT64              u8Dts;
    UINT32              u4PayloadAddr;
    UINT16              u2PayloadSize;
} PSR_AUDIO_PES_T;

typedef enum
{
    CS_FREE,
    CS_RUNNING,
    CS_STOP
} CHANNEL_STATE_T;

typedef struct
{
    CHANNEL_STATE_T     eState;
    PSR_STREAM_T        eStreamType;
    UINT32              u4FifoSize;
    UINT32              u4AlmostFullThreshold;
    void*               pvFifoAddr;
    void*               pvHeader;
    UINT8               u1DeviceId;
} CHANNEL_T;

// Parser events
typedef enum
{   // Event type                       // pvData
    //
    PSR_EVENT_UNKNOWN,                  // NULL
    PSR_EVENT_PES_PACKET,               // PSR_VIDEO_PES_T/PES_AUDIO_PES_T
    PSR_EVENT_FIFO_FULL                 // NULL
} PSR_EVENT_T;

// Parser event handler
// Note: In the callback handler, it's not allowed to copy the pointer pvData
// for later use. Once the handler returned, the pointer pvData is invalid
// immediately.
typedef void (*PFN_PARSER_CALLBACK)(PSR_EVENT_T eEvent, UINT8 u1ChannelId,
    UINT8 u1DeviceId, PSR_STREAM_T eStreamType, void* pvData);

typedef struct
{
    CHANNEL_T           arChannel[MAX_CHANNEL_NUM];
    HANDLE_T            hSemLock;
    UINT32              au4AudioBufAddr[MAX_DSP_NUM];
    UINT32              au4AudioBufSize[MAX_DSP_NUM];
    PFN_PARSER_CALLBACK pfnHandler;
 } PARSER_T;

typedef struct
{
    CHANNEL_STATE_T     eState;
    PSR_STREAM_T        eStreamType;
    UINT32              u4FifoStart;
    UINT32              u4FifoEnd;
    UINT32              u4Wp;
    UINT32              u4Rp;
    UINT32              u4DataSize;
    UINT32              u4MaxDataSize;
    UINT8               u1DeviceId;
    BOOL                fgFull;
} CHANNEL_INFO_T;

//---------------------------------------------------------------------
// Exported functions

/*
extern PARSER_T* GetParserObject(void);

extern void ReleaseParserObject(PARSER_T* prParser);
*/

 //---------------------------------------------------------------------
// Init and exit

extern BOOL PSR_Init(const UINT32 au4AudioBufAddr[MAX_DSP_NUM],
    const UINT32 au4AudioBufSize[MAX_DSP_NUM], PFN_PARSER_CALLBACK pfnHandler);

extern BOOL PSR_Exit(void);


//---------------------------------------------------------------------
// Operations

extern BOOL PSR_Reset(void);

extern BOOL PSR_Start(void);

extern BOOL PSR_Stop(void);

extern BOOL PSR_Flush(void);


//---------------------------------------------------------------------
// Channel operations

extern BOOL PSR_SetChannel(UINT8 u1Channel, PSR_STREAM_T eStreamType,
    UINT8 u1DeviceId, BOOL fgIsHD);

extern BOOL PSR_FreeChannel(UINT8 u1Channel);

extern BOOL PSR_SemiFreeChannel(UINT8 u1Channel);

extern BOOL PSR_IsChannelActive(UINT8 u1Channel);

extern BOOL PSR_IsChannelFree(UINT8 u1Channel);

extern BOOL PSR_StartChannel(UINT8 u1Channel);

extern BOOL PSR_StopChannel(UINT8 u1Channel);

extern BOOL PSR_ResetChannel(UINT8 u1Channel);

extern BOOL PSR_FlushChannel(UINT8 u1Channel);

extern PSR_STREAM_T PSR_GetChannelType(UINT8 u1Channel);

extern UINT8 PSR_GetDeviceId(UINT8 u1Channel);

extern BOOL PSR_GetChannelInfo(UINT8 u1Channel, CHANNEL_INFO_T* prInfo);

extern BOOL PSR_IsChannelAlmostFull(UINT8 u1Channel);


//---------------------------------------------------------------------
// Misc

extern PFN_PARSER_CALLBACK PSR_GetHandler(void);

extern BOOL PSR_SetDma(BOOL fgEnableDma);

extern BOOL PSR_SetMessagePeriod(UINT32 u4Period);

extern BOOL PSR_IsInterruptInhibited(void);

extern INT32 PSR_Diag(void);

extern void PSR_PowerControl(BOOL fgOn);

// Update VLD read pointer
extern BOOL PSR_UpdateReadPointer(UINT8 u1Channel, UINT8 u1DeviceId,
    PSR_STREAM_T eStreamType, UINT32 u4ReadPointer, UINT32 u4PicAddr);

// VLD switch channel
extern BOOL PSR_VldSwitchChannel(UINT8 u1DeviceId, UINT8 u1Channel);


//===========================================================================
// Soft parser interface

extern BOOL PSR_SoftInit(void);

extern BOOL PSR_SoftReset(void);

extern BOOL PSR_SoftTransfer(PSR_STREAM_T eStreamType, UINT8 u1DeviceId,
    UINT32 u4SrcAddr, UINT32 u4Size,
    UINT32 u4DstAddr, UINT32 u4DstFifoStart, UINT32 u4DstFifoEnd,
    BOOL fgPollMode);
extern BOOL PSR_SoftSetAudioFIFO(UINT8 ucDecId, UINT32 u4DstFifoStart,
    UINT32 u4DstFifoEnd);

extern BOOL PSR_RiscSetAudioWp(UINT8 u1DeviceId, UINT32 u4WritePointer);

extern BOOL PSR_RiscSetAudFIFO(UINT8 u1DeviceId, UINT32 u4DstFifoStart,
	UINT32 u4DstFifoEnd);

#endif  // DRV_PSR_H

