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

// *********************************************************************
// Memo
// *********************************************************************
/*
*/

#ifndef _DRV_ESM_IF_H_
#define _DRV_ESM_IF_H_

#include "x_typedef.h"
#include "drv_common.h"
#include "drv_ibc.h"
#include "sys_config.h"
#include "dmx_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Invalid index value in ESM
#define ESM_INVALID_INDEX 0xFFFFFFFF
/// Invalid count value in ESM
#define ESM_INVALID_COUNT 0x0
/// Invalid addrss in ESM
#define ESM_INVALID_ADDRESS ((uintptr_t)0x0)
/// Invalid handle in ESM
#define ESM_INVALID_HANDLE 0xFFFFFFFF
/// Invalid size in ESM
#define ESM_INVALID_SIZE 0xFFFFFFFF

/// Enumerate access unit
typedef enum
{
  AU_DATA = 0,                ///< Data
  AU_CMD,                        ///< in-band command
} AU_TYPE;

typedef enum
{
  AUD_DEFAULT = 0,          ///< default
  AUD_AC3,                    ///< Dolby digital
  AUD_MLP,                    ///< MLP
  AUD_SACD,                   ///< SACD
} AUD_TYPE;

#define RM_VID_SLICE_MAX_NUM    (128)

typedef struct _RM_VID_SLICE_ELEM_INF
{
  __u8 u1SliceElemNum;
  __u16 u2SliceElemSize;
}RM_VID_SLICE_ELEM_INF;

typedef struct _RM_VID_SLICE_INF
{
  __u8 u1TotalSliceNum;
  __u32 u4MemSa; // Record slice element info
}RM_VID_SLICE_INF;

/// Picture information of a Access unit
typedef struct _PIC_IFO
{
    __u32 u4VType;              ///< Byte3~2: Extra Type; Byte 1~0: picture coded type, please refer drv_Common.h
    uintptr_t ptrSAddr;                ///< start address in fifo
    uintptr_t ptrEAddr;              ///< end address in fifo

    uintptr_t ptrSVAddr;                ///< start address in fifo
    uintptr_t ptrEVAddr;              ///< end address in fifo

    __u64 u8Pts;                   ///< PTS
    __u64 u8PrevPTS;          ///< Previous PTS
    __u64 u8Dts;                  ///< DTS
    __u64 u8Offset;              ///< Lba or file offset
    __u64 u8PTSOffset;              ///< PTS Offset From MW
    __u64 u8SoftPts;            ///< Soft PTS
    uintptr_t ptrSeqHdrSa;      ///< Sequence Header Starting Address //Only for WMV789 (No Start Code WMV Source)
    __u32 u4SeqHdrLen;      ///< Sequence Header Data Length //Only for WMV789 (No Start Code WMV Source)
    uintptr_t ptrPPSHdrSa;          ///< Picture Header Starting Address
    __u32 u4PPSHdrLen;        ///< Picture Header Data Length
    uintptr_t ptrCCSa;               ///< Start address of user data which contains closed caption
    DiscType eDiscType;        ///< disc type, please refer drv_common.h
    __u32 u4Duration;         ///< For Variable Frame Rate File Format
    __u32 u4PrevDuration;   ///< For Variable Frame Rate File Format
    __u32 u4WMVSliceAddr[3];   ///< WMV Slice Address information
    bool fgxvColor;
    __u32 u4xvColorR;     ///< xvColor: R data
    __u32 u4xvColorG;     ///< xvColor: G data
    __u32 u4xvColorB;     ///< xvColor: B data
    __u32 u4EsdIndex;
    __u32 u4EsdNums;

    __u32 u4RMSliceNum;
    __u32 auRM4SliceSize[128];
} PicInfo;

/// access unit information of video
typedef struct _AU_VPic
{
  AU_TYPE eAuType;                     ///< Access unit type
  bool      fgIBCSent;                        ///< Inband command had sent back to MW
  union
  {
    PicInfo rInfo;                             ///< Picture information
  } rAUInfo;
}AU_VPic;

/// SubPicture information of a Access unit
typedef struct _SPIC_IFO
{
    uintptr_t ptrAddr;               ///< start address in fifo
    __u32 u4Size;                ///< Size in fifo
    __u64 u8StartPts;          ///< Start PTS
    __u64 u8EndPts;            ///< End PTS
    __u64 u8Offset;              ///< Lba or file offset
    __u64 u8Dts;                  ///< Dts
} SPicInfo;

/// access unit information of Subpicture
typedef struct _AU_SP
{
  AU_TYPE eAuType;                     ///< Access unit type
  bool fgIBCSent;                        ///< Inband command had sent back to MW
  union
  {
    SPicInfo rInfo;                           ///< Subpicture information
  } rAUInfo;
}AU_SP;

/// Audio information of a Access unit
typedef struct _AUD_IFO
{
  __u64 u8Pts;                ///< Pts
  AUD_TYPE eAudType;          ///< 0: AC3, 1:MLP
} AudInfo;

/// access unit information of Audio
typedef struct _AU_AUDIO
{
	bool fgSkipData;               ///< start skip audio data flag

	uintptr_t ptrSAddr;                ///< start address in fifo
	uintptr_t ptrEAddr;              ///< end address in fifo
	AU_TYPE eAuType;                     ///< Access unit type
  
	union
	{
		AudInfo rInfo;                          ///< audio data information
	} rAUInfo;
}AU_AUDIO;

/// access unit information of Section
typedef struct _AU_SECTION
{
  bool fgSkipData;               ///< start skip audio data flag

  uintptr_t ptrSAddr;                ///< start address in fifo
  uintptr_t ptrEAddr;              ///< end address in fifo
  uintptr_t	ptrVirSA;
  uintptr_t	ptrPhySA;
  uintptr_t ptrUserVirSA;
  __u32  u4Size;
  AU_TYPE eAuType;                     ///< Access unit type
}AU_SECTION;

typedef struct
{
	//AU_SP rSPAU;
	__u16 u2OffTop;
	__u16 u2OffBottom;
	__u8  achPAL[32];
	__u32	sx;
	__u32	sy;
	__u32	ex;
	__u32	ey;
	__u32 u4Size;
	uintptr_t ptrRLESWDecVirSA;
	uintptr_t	ptrVirSA;
	uintptr_t	ptrPhySA;
	uintptr_t ptrUserVirSA;    // user space address
	uintptr_t ptrAUEndAddrOfFifo;  // au's end addr of fifo
	__u64 u8StartPts;
	__s64	i8Delay;
}ESM_IO_BUF_SP_INFO;

typedef struct
{
	bool  fgSetSeekInfo;
	__u32 au4SeekInfo[2];
} APEINFO_EXT_T;

typedef union
{
  APEINFO_EXT_T rApe;
} AU_AUDIO_EXT_INFO_T;

typedef struct _ESM_IO_BUF_INFO
{
	__u64 ptrFifoSPAddr;	// phisical address
	__u64 ptrFifoEPAddr;	// phisical address

	__u64 ptrFifoSVAddr;	//  Virtule address
	__u64 ptrFifoEVAddr;	//  Virtule address

	__u32 u4Status;        // dummy decoder's error status
	__u32 u4TimeWait;      // time out

	union
	{
		AU_AUDIO rAudioAU;
		AU_VPic rVPicAU;
		AU_SECTION rSectionAU;
		ESM_IO_BUF_SP_INFO rSPStruct;
	} rAU;

	union
	{
		AU_AUDIO_EXT_INFO_T rAudEx;
	} rAUEx;
} ESM_IO_BUF_INFO;

//add checking audio/video fifo usage is zero
typedef struct _ESM_IO_INFO
{
	bool fgVideoIsNull;
	bool fgAudioIsNull;
	bool fgCCIsNull;
} ESM_IO_INFO;


/// Enumerate Elementary stream interface type
typedef enum
{
  ES_NONE = 0,          ///< don't use this, error check
  ES_V,                       ///< video elementary stream
  ES_A,                 ///< audio elementary stream
  ES_SP,                     ///< sub picture elementary stream
  ES_SECTION,                ///< Section elementary stream
  MAX_ES_TYPE_CNT
} ES_TYPE;

typedef struct _ESM_IO_FIFO_USAGE
{
	__u32 u4FifoSize;	       // Fifo size
	__u32 u4FifoAvailSize;	   // available fifo size
	__u32 u4FifoUsage;        // Fifo usage
	ES_TYPE eType;             // stream interface type
} ESM_IO_FIFO_USAGE;

typedef struct {
	uintptr_t ptrBufAddr;
	__u32	u4BufLen;
	__u64	u8Pts;
} AUD_SEND_BUF_INFO;

typedef struct {
	__u32	u4WritePointer;
	__u32	u4Len;
	uintptr_t	ptrFifoSA;
	uintptr_t	ptrFifoEA;
} AUDIO_BUF_INFO;

typedef struct {
    uintptr_t ptrAfifoRPtr;
    uintptr_t ptrAfifoWPtr;
    uintptr_t ptrAfifoSA;
    uintptr_t ptrAfifoEA;
    uintptr_t ptrAfifoVirSA;
    uintptr_t ptrAfifoVirEA;
} AUD_POSINFO_T;

typedef struct {
    AUD_POSINFO_T posInfo[4];
} AUD_AFIFO_POSINFO_T;


typedef struct {
  __u32    u4DmyDec;
  __u32    u4ESIH;
} ES_DECODER_CB_DATA_T;

typedef struct _AU_TSDMA_DATA
{
	bool fgSkipData;               ///< start skip audio data flag
  
	uintptr_t ptrSAddr;                ///< start address in fifo
	uintptr_t ptrEAddr;              ///< end address in fifo

	uintptr_t	ptrVirSA;
	uintptr_t	ptrPhySA;
	uintptr_t ptrUserVirSA;
	__u32  u4Size;
	AU_TYPE eAuType;                     ///< Access unit type
}AU_TSDMA_DATA
;
typedef struct _ESM_TSDMA_IOBUF_INFO
{
  uintptr_t ptrSAddr;                ///< start address in fifo
  uintptr_t ptrEAddr;              ///< end address in fifo
  uintptr_t	ptrVirSA;
  uintptr_t	ptrPhySA;
  uintptr_t ptrUserVirSA;
  __u32  u4Size;
}ESM_TSDMA_IO_BUF_INFO;


typedef __s32 (*ESM_FUNC_SET_ESIFTYPE)(__u32 u4Handle, ES_TYPE eEsIfType);

typedef __s32 (*ESM_FUNC_SET_64)(__u32 u4Handle, __u64 u8Value);
typedef __u64 (*ESM_FUNC_GET_64)(__u32 u4Handle);

typedef __s32 (*ESM_FUNC_SET_32)(__u32 u4Handle, __u32 u4Value);
typedef __u32 (*ESM_FUNC_GET_32)(__u32 u4Handle);
typedef __u32 (*ESM_FUNC_GET_32_EX)(__u32 u4Handle, __u32 u4Value, __u32 u4Value2);
typedef void (*ESM_FUNC_SYNC_READPTR)(__u32 u4Handle);

typedef __s32 (*ESM_FUNC_GET_AUINFO)(__u32 u4Handle, __u32 u4Value, void **pprAUInfo);
typedef void (*ESM_FUNC_CLEAR)(__u32 u4Handle);

typedef __s32 (*ESM_FUNC_SRFIFO_SWITCH)(__u32 u4Handle, bool fgSwitch);

/// Enumerate Elementary stream interface callback event type
typedef enum
{
  CBE_AU_IN = 0,           ///< Au table data in notify, for decoder, pvData = NULL
  CBE_FIFO_IN,              ///< Fifo data in notify, for decoder, pvData = NULL
  CBE_FIFO_SET,            ///< Fifo start / end address set notify, for decoder, pvData = NULL
  CBE_SYNC_READPTR,   ///< Fifo read pointer sync request notify, for decoder, pvData = NULL
  CBE_AU_OUT,              ///< Au table data out, for filter, pvData = NULL
  CBE_FIFO_OUT,           ///< Fifo data out, for filter, pvData = NULL
  CBE_FIFO_FLUSH,       ///< Fifo flushed, for decoder, pvData = NULL
  CBE_FIFO_DESTROY,  ///< Fifo destroy, for decoder, pvData = NULL
#if (DRV_HANDLE_IBC_CALLBACK_RULE)
  CBE_IBC_FLUSH,        ///< Inband command flush, for filter, pvData = (void*)i4IBCId
#endif
}ES_CBEVENT;
typedef void (*ESM_FUNC_CB)(ES_CBEVENT eEvent, void *pvData, void *pvPrivate);
typedef void (*ESM_FUNC_CBREG)(__u32 u4Handle, ESM_FUNC_CB pfnCB, void* pvPrivate);


/// Interface for decoder operation
typedef struct _DECODER_OP_IF
{
/*! \name ESI Init Operation Interface
* @{
*/
  /// This function will register callback function
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \param pfnCB [IN] Callback function
  /// \param pvPrivate [IN] Caller private data, it will be the paramter of the callback function
  /// \return None.
  ESM_FUNC_CBREG pvESI_CBRegister;
/*! @} */

/*! \name Access Unit Table Operation Interface
* @{
*/
  /// This function will set current access unit read index
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \param u4Value [IN] Read index
  /// \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.
  ESM_FUNC_SET_32 pi4AUT_SetRdIdx;
  /// This function will increase current access unit read index
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \param u4Value [IN] Increase index count
  /// \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.
  ESM_FUNC_SET_32 pi4AUT_IncRdIdx;
  /// This function will decrease current access unit read index
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \param u4Value [IN] Decrease index count
  /// \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.
//  ESM_FUNC_SET_32 pi4AUT_DecRdIdx;
  /// This function will get current access unit read index
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \return Current read index. If ESM_INVALID_INDEX, something is wrong.
  ESM_FUNC_GET_32 pu4AUT_GetRdIdx;
  /// This function will get current access unit write index
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \return Current access unit write index. If ESM_INVALID_INDEX, something is wrong.
  ESM_FUNC_GET_32 pu4AUT_GetWrIdx;
  /// This function will get total unit count
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \return Total unit count. If ESM_INVALID_COUNT, something is wrong.
  ESM_FUNC_GET_32 pu4AUT_GetAUTotalCount;
  /// This function will get current available access unit count
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \return Current available access unit count. If ESM_INVALID_COUNT, something is wrong.
  ESM_FUNC_GET_32 pu4AUT_GetAUCount;
  /// This function will get access unit information
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \param u4Value [IN] Access unit index
  /// \param pprAUInfo [OUT] Access unit information. If elementary stream is video or subpicture, prAUInfo is AU_VInfo.
  /// \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.
  ESM_FUNC_GET_AUINFO pi4AUT_GetAUInfo;
  /// This function will get next access unit index of the reference index
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \param u4Value [IN] Reference access unit index
  /// \param u4Value2 [IN] Access unit jump count
  /// \return Next access unit index. If ESM_INVALID_INDEX, it means that next access unit is not available.
  ESM_FUNC_GET_32_EX pu4AUT_GetNextAUIdx;
  /// This function will get previous access unit index of the reference index
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \param u4Value [IN] Reference access unit index
  /// \param u4Value2 [IN] Access unit jump count
  /// \return previous access unit index. If ESM_INVALID_INDEX, it means that previous access unit is not available.
  ESM_FUNC_GET_32_EX pu4AUT_GetPrevAUIdx;
/*! @} */

/*! \name Fifo Operation Interface
* @{
*/
  /// This function will get fifo start address
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \return Fifo start address. If ESM_INVALID_ADDRESS, something is wrong.
#if CONFIG_SYS_MEM_PHASE3

  ESM_FUNC_GET_32 pu4FIFO_GetHandle;
  /// This function will get fifo end address
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \return Fifo end address. If ESM_INVALID_ADDRESS, something is wrong.
#endif
  ESM_FUNC_GET_32 pu4FIFO_GetSa;
  /// This function will get fifo end address
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \return Fifo end address. If ESM_INVALID_ADDRESS, something is wrong.
  ESM_FUNC_GET_32 pu4FIFO_GetEa;
  /// This function will get fifo write pointer
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \return Fifo write pointer. If ESM_INVALID_ADDRESS, something is wrong.
  ESM_FUNC_GET_32 pu4FIFO_GetWp;
  /// This function will set fifo read pointer
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \param u4Value [IN] Fifo read pointer
  /// \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.
  ESM_FUNC_SET_32 pi4FIFO_SetRp;
  /// This function will get fifo read pointer
  /// \param u4Handle [IN] Elementary stream interface handle
  /// \return Fifo read pointer. If ESM_INVALID_ADDRESS, something is wrong.
  ESM_FUNC_GET_32 pu4FIFO_GetRp;
/*! @} */
}Decoder_OpIf;

#ifdef __cplusplus
}
#endif

#endif //#ifndef _DRV_ESM_IF_H_

