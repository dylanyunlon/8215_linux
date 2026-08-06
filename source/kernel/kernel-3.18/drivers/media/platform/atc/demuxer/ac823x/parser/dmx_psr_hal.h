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
 * @file dmx_psr_hal.h
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

#ifndef DMX_PSR_HAL_H
#define DMX_PSR_HAL_H


#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/x_vid_dec.h>
#else
#include "dmx_define.h"
#endif

#include "dmx_pvr.h"
#include "dmx_hal_if.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Hdr Dect Que Type*/
typedef enum {
	HDQ_PICTYPE = 0,	/*< Pic type*/
	HDQ_PICADDR,		/*< Pic addr*/
} PSR_HAL_HdrDectQType;

/* Bitstream Stream Type*/
typedef enum {
	BitType_Video = 0,	/*< Video Bitstream*/
	BitType_Audio,		/*< Audio Bitstream*/
	BitType_SubPic0,	/*< Sub-Picture0 Bitstream*/
	BitType_SubPic1,	/*< Sub-Picture1 Bitstream*/
	BitType_Navigation, /*< Navigation Bitstream*/
	BitType_Section,	/*< Section Bitstream*/
} PSR_HAL_BitType;

/* P-Transfer Mode*/
typedef enum {
	PTMode_DMA = 0,		/*< P-Transfer SPT_DATA_BUF Only*/
	PTMode_DMARH,		/*< P-Transfer SPT_DATA_BUF+Read Header*/
	PTMode_RH,			/*< Read Header Only (Np SPT_DATA_BUF)*/
} PSR_HAL_PTMode;

/* OPT Type*/
typedef enum {
	OPTType_PTREANS = 0,		/*< P-Transfer*/
	OPTType_GTREANS,			/*< G-Transfer*/
	OPTType_PINS,				/*< P-Ins*/
	OPTType_DRMPD,				/*< WMDRM PD*/
	OPTType_DRMND,				/*< WMDRM ND*/
} PSR_HAL_OPTType;

/* P-Transfer CPS Mode*/
typedef enum {
	CPSMode_NONE = 0,	/*< P-Transfer Only*/
	CPSMode_SACD,		/*< P-Transfer with SACD*/
	CPSMode_WMDRM,		/*< P-Transfer with WMDRM*/
	CPSMode_WMDRMND,	/*< P-Transfer with WMDRM-ND*/
} PSR_HAL_CPSMode;

typedef struct _PSR_HDRDET_RESULT_T_ {
	u8 u1PicInfoCount;						   /*< Pic Info Count*/

	/*Add one Flag for H264 B Ref Information*/
	u8 u1PicExtInfo;    /*< Pic Ext Info for Decoding (Now this flag only for H264 B Ref Pic Flag)*/

	u8  u1PicType[DMX_MAX_VID_STARTCODE_CNT];	/*< Pic Type*/
	uintptr_t ptrPicAddr[DMX_MAX_VID_STARTCODE_CNT];	/*< Pic Address*/
} PSR_HDRDET_RESULT_T;

/* HAL Status Info*/
typedef struct _PSR_HDRDET_STATUS_T_ {
	u32 u4PTransStatus;			/*< P-Transfer Status Information*/
	u32 u4PTransStatus1;			/*< P-Transfer Status Information 1*/

	/* Add for PSR Flow + Dmx HW*/
	u32 u4HdrDetBufSa;			/*< Header Detection Result Buffer Start Address*/
	u32 u4HdrDetBufEa;			/*< Header Detection Result Buffer End Address  (Sa + Size -1)*/
	u32 u4HdrDetBufWPtr;			/*< Header Detection Result Buffer Write Pointer*/
	u32 u4HdrDetBufRPtr;			/*< Header Detection Result Buffer Read Pointer*/
	u32 u4HdrDetBufStrPtr;		/*< Header Detection Result Buffer Starting Pointer*/

	u32 u4HdrDectQueNum;
	u32 u4HdrDectQueData;
	PSR_HAL_HdrDectQType eHdrDectQueType;
} PSR_HDRDET_STATUS_T;

typedef struct _PSR_HALPT_INFO_T_ {
	bool fgUseCmdQ;
	u8  u1DevID;

	u16 u2EntryRdIdx;
	u16 u2EntryWrIdx;

	u32 u4RdIdxOfst;		/*< Start entry idx maybe divided, need adjust for HW use*/
	u32 u4RdIdxLen;		/*< Start entry idx maybe divided, need adjust for HW use*/

	u32 u4LastValidIdxLen;	/*< End entry idx maybe divided, need adjust for HW use*/

	void *pvCmdQTxEntryBuffer;
	u32 u4GarbageSz;
	uintptr_t	ptrSrcSa;				/*< Src Start Address*/
	u32	u4SrcLen;				/*< Src Data Length*/
	uintptr_t	ptrFifoSa;				/*< Fifo memory start address*/
	u32	u4FifoSz;				/*< Fifo memory size*/
	uintptr_t	ptrFifoRdPtr;			/*< Fifo read pointer*/
	uintptr_t	ptrFifoWrPtr;			/*< Fifo write pointer*/
	uintptr_t	ptrFifoStrPtr;			/*< Fifo Starting Write Pointer (Only for Tx HW)*/

	PSR_HAL_BitType	eBitType;		/*< Bitstream Type*/
	PSR_HAL_PTMode	ePTMode;		/*< P-Trans Mode*/

	PSR_HAL_CPSMode eCPSMode;		 /*< CPS Mode (Only for SACD and WMDRM)*/

	PVR_DESC_MODE_T	eDescMode;

	#if ENABLE_DMX_ADVANCED_VER
	PVR_INST_BYTES_INFO_T rInstBytesInfo;
	#endif /* ENABLE_DMX_ADVANCED_VER*/

	PVR_DRM_PARAM_T		rDRMInfo;

	DMX_PID_NTIFY_INFO_T	rPidFunc;
	DMX_HAL_FUNC_INFO_T		rGlobalFunc;
} PSR_HALPT_INFO_T;

/* Parser Hal callback function prototype*/
typedef u32 (*PSR_HAL_FUNC_CB)(void *pvData, void *pvUserPrivate);

typedef enum {
	DMX_HW_STATE_UNKNOWN,
	DMX_HW_STATE_IDLE,
	DMX_HW_STATE_OCCUPIED,
	DMX_HW_STATE_NEEDSUSPEND,
	DMX_HW_STATE_SUSPEND
} E_DMX_HW_STATE_T;

/* Psr HW Info structure*/
typedef struct {
	bool  fgUsed;
	u8				u1DeviceId; 		  /*< Device ID*/

	/* Buffer info*/
	uintptr_t		ptrSrcSa;				  /*< Src Start Address*/
	u32				u4SrcLen;				  /*< Src Data Length*/
	uintptr_t		ptrFifoSa;				  /*< Fifo memory start address*/
	u32				u4FifoSz;				  /*< Fifo memory size*/
	uintptr_t		ptrFifoRdPtr;			  /*< Fifo read pointer*/
	uintptr_t		ptrFifoWrPtr;			  /*< Fifo write pointer*/

	u32				u4GarbageSz;
	u32				u4HdrDectQueNum;
	u32				u4HdrDectQueData;

	DMX_PM_STATE	ePowerState;

	E_DMX_HW_STATE_T	eState;				  /*< Valid*/

	/* Pic Hdr Dect Status*/
	PSR_HAL_HdrDectQType	eHdrDectQueType;
	VCodeC				eVideoCodec;
	PSR_HAL_BitType		eBitType;
	void				*pvOwner;

	/*Reparsing Info*/
	/*< Video Start Codec (PictureType-PictureAddress) Pairs obtained from Video PES Interrupt*/
	PSR_HDRDET_RESULT_T *prHdrDetResult;
} PSR_STRUCT_T;

MRESULT PSR_HAL_Init(void);

void	PSR_HAL_Uninit(void);

u8	PSR_HAL_HWRes_Obtain(void *pvOwner);

MRESULT PSR_HAL_HWRes_Release(u8 u1DevID);

bool	PSR_HAL_IsHWResOccupied(u8 u1DevID);

void	PSR_HAL_SetSuspend(u8 u1DevID, bool fgSuspend, void **ppvOwner);

VCodeC	PSR_HAL_GetVideoCodec(u8 u1DevID);

MRESULT PSR_HAL_SetVideoCodec(u8 u1DevID, VCodeC eVCodeC, bool fgOperForce);

MRESULT PSR_HAL_SetLastHdrDetStatus(u8 u1DevID, PSR_HAL_BitType eBitType, PSR_HDRDET_STATUS_T *prHdrDetStatus);

MRESULT PSR_HAL_KeepHdrDetResult(u8 u1DevID, DMX_FTI_INTSTATUS_T *pStatus);

u32	PSR_HAL_GetGarbageSz(u8 u1DevID);

MRESULT PSR_HAL_PTransfer(u8 u1DevID, PSR_HALPT_INFO_T *prPTransInfo);

MRESULT PSR_HAL_GetHdrDetResult(void *pvPsrFtr,
	PSR_HDRDET_RESULT_T *prHdrDetResult, PSR_HDRDET_STATUS_T *prHdrDetStatus);

MRESULT PSR_HAL_GetWPtr(u8 u1DevID, PSR_HAL_BitType eBitType, uintptr_t *pptrCurWPtr);

MRESULT PSR_HAL_SetWPtr(u8 u1DevID, PSR_HAL_BitType eBitType, uintptr_t ptrCurWPtr);

MRESULT PSR_HAL_GetRPtr(u8 u1DevID, PSR_HAL_BitType eBitType, uintptr_t *pptrCurRPtr);

MRESULT PSR_HAL_SetRPtr(u8 u1DevID, PSR_HAL_BitType eBitType, uintptr_t ptrCurReadPtr);

MRESULT PSR_HAL_GlobalCB(void *pvData, void *pvUserPrivate);

MRESULT PSR_HAL_PIDCB(u8 u1PidIdx, PVR_NOTIFY_CODE_T eCode, void *pvData,
	const void *pvNotifyTag);

MRESULT PSR_HAL_ParsingIntData(void *pvHwData);

MRESULT PSR_HAL_DumpPidStructInfo(u8 u1Pidx);

MRESULT PSR_HAL_DumpDDIInfo(void);

MRESULT PSR_HAL_SetPowerState(DMX_PM_STATE ePowerState);

DMX_PM_STATE PSR_HAL_GetPowerState(void);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef _PSR_HAL_H_*/
