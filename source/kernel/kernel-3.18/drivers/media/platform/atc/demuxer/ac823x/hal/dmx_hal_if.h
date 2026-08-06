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
 * @file dmx_hal_if.h
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *	  Excapsluate the PVR interface to DMX interface, in order to do not modify splitter code.
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

#ifndef	DMX_HAL_IF_H
#define	DMX_HAL_IF_H
#include	"x_typedef.h"
#include	"drv_common.h"
#include	"dmx_def.h"

#include "dmx_pvr.h"
#ifdef __cplusplus
extern "C" {
#endif

/* FIFO Type*/
typedef enum {
	FFType_Normal = 0,					/*< HW Auto Mode*/
	FFType_ES,						/*< HW Detection + SW Calculate*/
	FFType_PES,						/*< SW Control*/
} DMX_HAL_FIFOType;

/* Header Detection Result (For Callback Function)*/
typedef struct _PVR_HDRDECT_RESULT_T_ {
	u32	u4PESHdrStartAddr;		/*< PES Header Buffer Start Address*/
	u32	u4PESHdrLen;			/*< PES Header Data Length*/
	u32	u4PicInfoCount;			/*< Pic Info Count*/
	DMX_PIC_INFO_T	*prPicsInfo;		/*< Pic Type-Address array*/
} PVR_HDRDECT_RESULT_T;

typedef struct _PVR_MM_PID_INFO_T_ {
	u32				u4PIDIdx;
	PVR_PID_TYPE_T			ePidType;	/*< PID type*/
	PVR_DESC_MODE_T			eDescMode;
	PVR_DRM_PARAM_T			rDRMInfo;
	DMX_PID_NTIFY_INFO_T		rPidFunc;	/* Pid Structure Callback info*/
} PVR_MM_PID_INFO_T;

/* DMX HW Contorl Parameter*/
typedef struct _PVR_CMDCFG_INFO_T_ {
	u8 u1DevID;
	PVR_MM_PID_INFO_T rPIDInfo;

	uintptr_t ptrSrcBufAddr;	/*< Src Buffer Start Address*/
	uintptr_t ptrSrcEndAddr;	/*< Src Buffer End Address*/
	u32 u4SrcBufSize;	/*< Src Buffer Size*/
	uintptr_t ptrSrcBufRPtr;	/*< Src Buffer Read Ptr*/
	uintptr_t ptrSrcBufWPtr;	/*< Src Buffer Write Ptr*/

	uintptr_t ptrDstFifoAddr;	/*< Output FIFO Address*/
	u32 u4DstFifoSize;	/*< Output FIFO Size*/
	uintptr_t ptrDstFifoWPtr;	/*< Output FIFO Write Ptr*/
	uintptr_t ptrDstFifoRPtr;	/*< Output FIFO Read Ptr*/
	uintptr_t ptrDstFifoSPtr;	/*< Output FIFO Starting Ptr*/

	PTX_CMDQ_INFO_T			rPTXCmdInfo;	/*< Command Queue Info*/
	PVR_INST_BYTES_INFO_T		rPTXInstBsInfo;
	DMX_HAL_FUNC_INFO_T		rGlobalFunc;	/* Demuxer Global Callback info*/
} DMX_CMDCFG_INFO_T;

/* FILTER CallBack (For Callback Function)*/
typedef struct _PVR_FILTER_INTSTATUS_T_ {
	u32	u4HWPIDIndex;				/*< HW PID Index*/
	u32	u4DataStartAddr;			/*< PSI/PES Output Start Addr*/
	u32	u4DataEndAddr;				/*< PSI/PES Output Start Addr*/
	/*< Header Detection Result (Only for eDmxStatus = DMXST_ES)*/
	PVR_HDRDECT_RESULT_T rHdrDectResult;
} PVR_FILTER_INTSTATUS_T, DMX_FTI_INTSTATUS_T;

/* DMX HAL Interface*/
EXTERN bool    DMX_HAL_SetVideoType(u8 u1DevID, PVR_VIDEO_TYPE_T eVCodecType);

EXTERN MRESULT DMX_HAL_Init(PVR_INPUT_TYPE_T eInputType);

EXTERN void    DMX_HAL_Uninit(void);

EXTERN MRESULT DMX_HAL_DMXConfig(DMX_CMDCFG_INFO_T *prDMXCfgInfo);

EXTERN MRESULT DMX_HAL_ProcIntData(void *pvArg);

EXTERN MRESULT DMX_HAL_SetHdrDetStatus(u32 u4HWPIDIndex,
	u32 *pu4HdrBufInfo);

EXTERN MRESULT DMX_HAL_GetHdrDetStatus(u32 u4HWPIDIndex,
	u32 *pu4HdrBufInfo);

EXTERN u32 DMX_HAL_GetFIFOWPtr(u32 u4HWPIDIndex,
	DMX_HAL_FIFOType eFIFOType);

EXTERN u32 DMX_HAL_GetFIFORPtr(u32 u4HWPIDIndex,
	DMX_HAL_FIFOType eFIFOType);

EXTERN void    DMX_HAL_SetFIFOWPtr(u32 u4HWPIDIndex,
	DMX_HAL_FIFOType eFIFOType, u32 u4CurWritePtr);

EXTERN void    DMX_HAL_SetFIFORPtr(u32 u4HWPIDIndex,
	DMX_HAL_FIFOType eFIFOType, u32 u4CurReadPtr);

EXTERN bool    DMX_HAL_DumpStartCodes(void);

EXTERN bool    DMX_HAL_DumpPidStruct(u8 u1Pidx);

EXTERN bool    DMX_HAL_DumpDDIInfo(void);

EXTERN void    DMX_HAL_PVR_DumpRegisters(u32 u41stRegAddr,
	u32 u4RegsCnt);
EXTERN void DMX_HAL_PVR_DumpLocalarbiter(void);

EXTERN void DMX_HAL_PVR_DumpDramKeyRegs(bool fg1stClear);

EXTERN DMX_PM_STATE DMX_HAL_GetPowerState(void);

EXTERN bool DMX_HAL_SetPowerState(DMX_PM_STATE PowerState,
	PVR_INPUT_TYPE_T eInputType);

#ifdef __cplusplus
}
#endif

#endif /*#ifndef _PVR_PVR_HAL_IF_H_*/

