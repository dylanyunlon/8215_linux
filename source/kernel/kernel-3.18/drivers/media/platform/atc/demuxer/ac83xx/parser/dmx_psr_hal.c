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
 * @file dmx_psr_hal.c
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

#include "x_os.h"
#include "x_debug.h"
#ifdef __linux__
#include <linux/mm.h>
#include "windows.h"
#include <media/atc/dmx_define.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "mm_debug.h"
#endif				/* __linux__ */

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_spt_os.h"
#include "dmx_parser.h"
#include "dmx_psr_hal.h"
#include "dmx_hal_if.h"
#include "dmx_psr_util.h"
#include "dmx_pfm.h"
#include "dmx_inst.h"

#ifndef __linux__
#pragma warning(disable : 4127)	/*disable warning C4127: conditional expression is constant */
#endif

EXTERN DMX_CLI_MAN_T g_rDmxCliMan;
EXTERN DMX_DUMP_MAN_T g_rDmxDumpMan;
EXTERN PSR_STRUCT_T g_rPSRHalStruct[DMX_DEV_CNT];
EXTERN bool g_fgPSRHalInit;
HANDLE g_hPSRHalSema = NULL;

#define DMX_IS_WMV_VALID_HDR(PicType)				(0xFF != (PicType))
const u8 _au1WMVHdrTable[8] = {
	0x01, 0x02, 0x1E, 0xF0, 0xFF, 0xF1, 0xF2, 0xF3
};

#define	DMX_IS_MPEG2_VALID_HDR(PicType)				(0xFF != (PicType))
#define	DMX_IS_MPEG2_FRAME_HDR(PicType)				(0xC0 == (PicType))
#define	DMX_GET_MPEG2_FRAME_HDR_ADDR(startcodeaddr)	((u8 *)((startcodeaddr) + 5))
#define	DMX_GET_MPEG2_FRAME_TYPE(x)					(((x) >> 3) & 0x07)

const u8 _au1Mpeg2HdrTable[7] = {
	0xC0, 0x08, 0x0A, 0x09, 0xFF, 0xFF, 0xFF
};

#define	DMX_IS_MPEG4_VALID_HDR(PicType)	((0xD1 != (PicType)) && (0xFF != (PicType)))
#define	DMX_IS_MPEG4_FRAME_HDR(PicType)				(0xD0 == (PicType))
#define	DMX_GET_MPEG4_FRAME_HDR_ADDR(startcodeaddr)	((u8 *)((startcodeaddr) + 4))
#define	DMX_GET_MPEG4_FRAME_TYPE(x)					(((x) >> 6) & 0x03)
const u8 _au1Mpeg4HdrTable[7] = {
	0xD0, 0x06, 0x09, 0x0B, 0xFF, 0x05, 0xD1
};

#define DMX_IS_H263_VALID_HDR(PicType)				(0xFF != (PicType))
const u8 _au1H263HdrTable[6] = {
	0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF
};

#define DMX_IS_H264_VALID_HDR(PicType)				(0xFF != (PicType))
#define DMX_IS_H264_AVC_MVC_HDR(PicType)			(0xE2 == (PicType))
const u8 _au1H264HdrTable[11] = {
	0x04, 0x08, 0x1E, 0x10, 0xFF, 0x11, 0x01, 0x02, 0x12, 0x14, 0xE2
};

#define DMX_IS_H265_VALID_HDR(PicType)				(0xFF != (PicType))
#define DMX_IS_H265_RSVNVCL_41_44(x)	((((x)&0x5E) < 0x5A) && (0x52 <= ((x)&0x5E)))
const u8 _au1H265HdrTable[10] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x11, 0x12, 0xF1, 0x1E, 0x1B
};

static MRESULT _PSR_SetPsrStructInfo(PSR_HALPT_INFO_T *prPTransInfo)
{
	PSR_STRUCT_T *prPsrStruct;
	u8 u1DevID = prPTransInfo->u1DevID;

	if ((DMX_HW_STATE_OCCUPIED != g_rPSRHalStruct[u1DevID].eState) &&
	    (DMX_HW_STATE_NEEDSUSPEND != g_rPSRHalStruct[u1DevID].eState)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for PSR Struct is disable!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (!(NULL != prPTransInfo)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrStruct = &g_rPSRHalStruct[u1DevID];

	/*FIFO Info */
	prPsrStruct->ptrFifoSa = prPTransInfo->ptrFifoSa;
	prPsrStruct->u4FifoSz = prPTransInfo->u4FifoSz;
	prPsrStruct->ptrFifoWrPtr = prPTransInfo->ptrFifoWrPtr;
	prPsrStruct->ptrFifoRdPtr = prPTransInfo->ptrFifoRdPtr;

	/*Src Info */
	prPsrStruct->ptrSrcSa = prPTransInfo->ptrSrcSa;
	prPsrStruct->u4SrcLen = prPTransInfo->u4SrcLen;
	prPsrStruct->eBitType = prPTransInfo->eBitType;
	prPsrStruct->u4GarbageSz = prPTransInfo->u4GarbageSz;

	MM_RETURN(RET_DMX_OK);
}

static void _PSR_DumpHdrDectResult(
	PSR_HDRDET_RESULT_T *prHdrDectResult, PSR_STRUCT_T *prPsrStruct,
	u32 u4Idx, bool fgError)
{
	u32 i = 0;
	u8 u4HdrCnt = 0;

	if ((NULL == prHdrDectResult) ||
	    (NULL == prPsrStruct) || (u4Idx >= DMX_MAX_VID_STARTCODE_CNT)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for invalid args(u4Idx: %d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);
		return;
	}

	u4HdrCnt = prHdrDectResult->u1PicInfoCount;
	if (fgError) {
		uintptr_t ptrFifoEa = prPsrStruct->ptrFifoSa + prPsrStruct->u4FifoSz;

		DMXLOG_TRACE(
			    TEXT("[PSR] %s -- u4Idx: %d, PicType: 0x%x, PicAddr: 0x%x\r\n"),
			    DMX_FUNC_NAME, u4Idx, prHdrDectResult->u1PicType[u4Idx],
			    prHdrDectResult->ptrPicAddr[u4Idx]);
		if ((prHdrDectResult->ptrPicAddr[u4Idx] < prPsrStruct->ptrFifoSa +
		     prPsrStruct->u4FifoSz) &&
		    (prPsrStruct->ptrFifoSa <= prHdrDectResult->ptrPicAddr[u4Idx])) {
			u8 *pu1Addr = (u8 *) (prHdrDectResult->ptrPicAddr[u4Idx]);

			DMXLOG_TRACE(
				    TEXT("[PSR] %s enter ---- VFifoSa: 0x%x, VFifoEa: 0x%x\r\n"),
				    DMX_FUNC_NAME, prPsrStruct->ptrFifoSa,
				    prPsrStruct->ptrFifoSa + prPsrStruct->u4FifoSz);

			DMXLOG_TRACE(
				    TEXT("[PSR] %s -- Dump ErrIdx(%d)'s FifoData as follow:\r\n"),
				    DMX_FUNC_NAME, u4Idx);
			for (i = 0; i < 5; i++) {
				DMXLOG_TRACE(TEXT("%u, \r\n"), *pu1Addr);
				pu1Addr++;
				if ((uintptr_t) pu1Addr >= ptrFifoEa)
					pu1Addr = (u8 *) (prPsrStruct->ptrFifoSa);
			}
		}
	} else {
		DMXLOG_TRACE(
			    TEXT("[PSR] %s -- u4Idx: %d, PicType: 0x%x, PicAddr: 0x%x\r\n"),
			    DMX_FUNC_NAME, u4Idx, prHdrDectResult->u1PicType[u4Idx],
			    prHdrDectResult->ptrPicAddr[u4Idx]);
	}
}

static MRESULT _PSR_GetPicHdrDetectStatus_SWKeepResult(
	u8 u1DevID,
	PSR_HDRDET_RESULT_T *prHdrDetResult)
{
	PSR_STRUCT_T *prPSRStruct = (PSR_STRUCT_T *) (&g_rPSRHalStruct[u1DevID]);
	u8 *pbHdrData = NULL;
	PSR_HDRDET_RESULT_T *prPsrStrHdrDetRst = NULL;
	u32 u4HdrCnt = 0;
	u32 u4PicIndex = 0;
	u32 i;
	u8 u1PicHdrType = 0;

	if ((DMX_HW_STATE_OCCUPIED != prPSRStruct->eState) &&
	    (DMX_HW_STATE_NEEDSUSPEND != prPSRStruct->eState)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for PSR Struct is disable!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (NULL == prPSRStruct->prHdrDetResult) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for prPSRStruct->")
					  TEXT("prHdrDetResult == NULL!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	prPsrStrHdrDetRst = prPSRStruct->prHdrDetResult;

	u4HdrCnt = prPsrStrHdrDetRst->u1PicInfoCount;
	if (u4HdrCnt >= DMX_MAX_VID_STARTCODE_CNT) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for Video StartCodes")
					  TEXT(" Cnt(%d) > DMX_MAX_VID_STARTCODE_CNT(%d)!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4HdrCnt, DMX_MAX_VID_STARTCODE_CNT);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	prHdrDetResult->u1PicInfoCount = u4HdrCnt;

	u4PicIndex = 0;
	for (i = 0; i < u4HdrCnt; i++) {
		switch (g_rPSRHalStruct[u1DevID].eVideoCodec) {
		case VC_MPEG2:
			if (prPsrStrHdrDetRst->u1PicType[i] >= DMX_ARRAY_SIZE(_au1Mpeg2HdrTable)) {
				_PSR_DumpHdrDectResult(prPsrStrHdrDetRst, prPSRStruct, i, TRUE);
				DMX_HAL_DumpStartCodes();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_ERR_PIC_TYPE);
			}
			u1PicHdrType = _au1Mpeg2HdrTable[prPsrStrHdrDetRst->u1PicType[i]];
			if (DMX_IS_MPEG2_VALID_HDR(u1PicHdrType)) {
				if (DMX_IS_MPEG2_FRAME_HDR(u1PicHdrType)) {
					/*I, P, B Picture Header (13818-2 P.57 picture_coding_type) */
					pbHdrData =
					    DMX_GET_MPEG2_FRAME_HDR_ADDR(prPsrStrHdrDetRst->
									 ptrPicAddr[i]);

					if (pbHdrData >= (u8 *) (prPSRStruct->ptrFifoSa +
								   prPSRStruct->u4FifoSz))
						pbHdrData -= prPSRStruct->u4FifoSz;
					if ((pbHdrData >= (u8 *) (prPSRStruct->ptrFifoSa +
								    prPSRStruct->u4FifoSz)) ||
					    ((uintptr_t) pbHdrData < prPSRStruct->ptrFifoSa)) {
						_PSR_DumpHdrDectResult(prPsrStrHdrDetRst,
								       prPSRStruct, i, TRUE);
						DMX_HAL_DumpStartCodes();
						DMX_ASSERT(FALSE);
						MM_RETURN(RET_DMX_ERR_PIC_TYPE);
					}
					u1PicHdrType = DMX_GET_MPEG2_FRAME_TYPE(*pbHdrData);
				}
				prHdrDetResult->u1PicType[u4PicIndex] = u1PicHdrType;
				prHdrDetResult->ptrPicAddr[u4PicIndex] =
				    prPsrStrHdrDetRst->ptrPicAddr[i];
				u4PicIndex++;
			}
			break;

		case VC_MPEG4:
		case VC_DIVX4:
		case VC_DIVX6:
			if (prPsrStrHdrDetRst->u1PicType[i] >= DMX_ARRAY_SIZE(_au1Mpeg4HdrTable)) {
				_PSR_DumpHdrDectResult(prPsrStrHdrDetRst, prPSRStruct, i, TRUE);
				DMX_HAL_DumpStartCodes();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_ERR_PIC_TYPE);
			}
			u1PicHdrType = _au1Mpeg4HdrTable[prPsrStrHdrDetRst->u1PicType[i]];
			if (DMX_IS_MPEG4_VALID_HDR(u1PicHdrType)) {
				if (DMX_IS_MPEG4_FRAME_HDR(u1PicHdrType)) {
					/*MP4 I, P, B Picture Type */
					pbHdrData =
					    DMX_GET_MPEG4_FRAME_HDR_ADDR(prPsrStrHdrDetRst->
									 ptrPicAddr[i]);

					if (pbHdrData >= (u8 *) (prPSRStruct->ptrFifoSa +
								   prPSRStruct->u4FifoSz))
						pbHdrData -= prPSRStruct->u4FifoSz;

					if ((pbHdrData >= (u8 *) (prPSRStruct->ptrFifoSa +
								    prPSRStruct->u4FifoSz)) ||
					    ((u32) pbHdrData < prPSRStruct->ptrFifoSa)) {
						_PSR_DumpHdrDectResult(prPsrStrHdrDetRst,
								       prPSRStruct, i, TRUE);
						DMX_HAL_DumpStartCodes();
						DMX_ASSERT(FALSE);
						MM_RETURN(RET_DMX_ERR_PIC_TYPE);
					}

					u1PicHdrType = DMX_GET_MPEG4_FRAME_TYPE(*pbHdrData);
				}

				prHdrDetResult->u1PicType[u4PicIndex] = u1PicHdrType;
				prHdrDetResult->ptrPicAddr[u4PicIndex] =
				    prPsrStrHdrDetRst->ptrPicAddr[i];
				u4PicIndex++;
			}
			break;

		case VC_H263:	/*Short Header */
			if (prPsrStrHdrDetRst->u1PicType[i] >= DMX_ARRAY_SIZE(_au1H263HdrTable)) {
				_PSR_DumpHdrDectResult(prPsrStrHdrDetRst, prPSRStruct, i, TRUE);
				DMX_HAL_DumpStartCodes();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_ERR_PIC_TYPE);
			}
			u1PicHdrType = _au1H263HdrTable[prPsrStrHdrDetRst->u1PicType[i]];
			if (DMX_IS_H263_VALID_HDR(u1PicHdrType)) {
				prHdrDetResult->u1PicType[u4PicIndex] = u1PicHdrType;
				prHdrDetResult->ptrPicAddr[u4PicIndex] =
				    prPsrStrHdrDetRst->ptrPicAddr[i];
				u4PicIndex++;
			}
			break;

		case VC_H264:
			if (prPsrStrHdrDetRst->u1PicType[i] >= DMX_ARRAY_SIZE(_au1H264HdrTable)) {
				_PSR_DumpHdrDectResult(prPsrStrHdrDetRst, prPSRStruct, i, TRUE);
				DMX_HAL_DumpStartCodes();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_ERR_PIC_TYPE);
			}
			u1PicHdrType = _au1H264HdrTable[prPsrStrHdrDetRst->u1PicType[i]];
			if (DMX_IS_H264_VALID_HDR(u1PicHdrType)) {
				if (DMX_IS_H264_AVC_MVC_HDR(u1PicHdrType))
					u1PicHdrType = PSR_HDR_AVC_MVC;

				prHdrDetResult->u1PicType[u4PicIndex] = u1PicHdrType;
				prHdrDetResult->ptrPicAddr[u4PicIndex] =
				    prPsrStrHdrDetRst->ptrPicAddr[i];
				u4PicIndex++;
			}
			break;

		case VC_VC1:
			if (prPsrStrHdrDetRst->u1PicType[i] >= DMX_ARRAY_SIZE(_au1WMVHdrTable)) {
				_PSR_DumpHdrDectResult(prPsrStrHdrDetRst, prPSRStruct, i, TRUE);
				DMX_HAL_DumpStartCodes();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_ERR_PIC_TYPE);
			}
			u1PicHdrType = _au1WMVHdrTable[prPsrStrHdrDetRst->u1PicType[i]];

			if (DMX_IS_WMV_VALID_HDR(u1PicHdrType)) {
				if (0xF0 == u1PicHdrType)
					u1PicHdrType = PSR_HDR_WMV_SLICE;

				if (0xF1 == u1PicHdrType) {
					/*Entry Point Header */
					pbHdrData = (u8 *) (prPsrStrHdrDetRst->ptrPicAddr[i] + 3);

					if (pbHdrData >= (u8 *) (prPSRStruct->ptrFifoSa +
								   prPSRStruct->u4FifoSz))
						pbHdrData -= prPSRStruct->u4FifoSz;

					if ((pbHdrData >= (u8 *) (prPSRStruct->ptrFifoSa +
								    prPSRStruct->u4FifoSz)) ||
					    ((u32) pbHdrData < prPSRStruct->ptrFifoSa)) {
						_PSR_DumpHdrDectResult(prPsrStrHdrDetRst,
								       prPSRStruct, i, TRUE);
						DMX_HAL_DumpStartCodes();
						DMX_ASSERT(FALSE);
						MM_RETURN(RET_DMX_ERR_PIC_TYPE);
					}

					if (0x0E == *pbHdrData)
						u1PicHdrType = PSR_HDR_WMV_EntryPoint;
				}

				if (0xF2 == u1PicHdrType) {
					/*Sequence Header */
					pbHdrData = (u8 *) (prPsrStrHdrDetRst->ptrPicAddr[i] + 3);

					if (pbHdrData >= (u8 *) (prPSRStruct->ptrFifoSa +
								   prPSRStruct->u4FifoSz))
						pbHdrData -= prPSRStruct->u4FifoSz;

					if ((pbHdrData >= (u8 *) (prPSRStruct->ptrFifoSa +
								    prPSRStruct->u4FifoSz)) ||
					    ((u32) pbHdrData < prPSRStruct->ptrFifoSa)) {
						_PSR_DumpHdrDectResult(prPsrStrHdrDetRst,
								       prPSRStruct, i, TRUE);
						DMX_HAL_DumpStartCodes();
						DMX_ASSERT(FALSE);
						MM_RETURN(RET_DMX_ERR_PIC_TYPE);
					}

					if (0x0F == *pbHdrData)
						u1PicHdrType = PSR_HDR_WMV_SeqHdr;
				}

				/*0x0000011C, 0x0000011D */
				if (0xF3 == u1PicHdrType)
					u1PicHdrType = PSR_HDR_WMV_USRDAAT;

				prHdrDetResult->u1PicType[u4PicIndex] = u1PicHdrType;
				prHdrDetResult->ptrPicAddr[u4PicIndex] =
				    prPsrStrHdrDetRst->ptrPicAddr[i];
				u4PicIndex++;
			}
			break;

		case VC_H265:	/*Short Header */
			if (prPsrStrHdrDetRst->u1PicType[i] >= DMX_ARRAY_SIZE(_au1H265HdrTable)) {
				_PSR_DumpHdrDectResult(prPsrStrHdrDetRst, prPSRStruct, i, TRUE);
				DMX_HAL_DumpStartCodes();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_ERR_PIC_TYPE);
			}

			u1PicHdrType = _au1H265HdrTable[prPsrStrHdrDetRst->u1PicType[i]];
			if (DMX_IS_H265_VALID_HDR(u1PicHdrType)) {
				if (0xF1 == u1PicHdrType) {
					/*Sequence Header */
					pbHdrData = (u8 *) (prPsrStrHdrDetRst->ptrPicAddr[i] + 3);

					if (pbHdrData >= (u8 *) (prPSRStruct->ptrFifoSa +
								   prPSRStruct->u4FifoSz))
						pbHdrData -= prPSRStruct->u4FifoSz;

					if ((pbHdrData >= (u8 *) (prPSRStruct->ptrFifoSa +
								    prPSRStruct->u4FifoSz)) ||
					    ((u32) pbHdrData < prPSRStruct->ptrFifoSa)) {
						_PSR_DumpHdrDectResult(prPsrStrHdrDetRst,
								       prPSRStruct, i, TRUE);
						DMX_HAL_DumpStartCodes();
						DMX_ASSERT(FALSE);
						MM_RETURN(RET_DMX_ERR_PIC_TYPE);
					}

					if (DMX_IS_H265_RSVNVCL_41_44(*pbHdrData)) {
						prHdrDetResult->u1PicType[u4PicIndex] =
						    PSR_HDR_H265_RSVNVCL_41_44;
						prHdrDetResult->ptrPicAddr[u4PicIndex] =
						    prPsrStrHdrDetRst->ptrPicAddr[i];
						u4PicIndex++;
					}
				} else {
					prHdrDetResult->u1PicType[u4PicIndex] = u1PicHdrType;
					prHdrDetResult->ptrPicAddr[u4PicIndex] =
					    prPsrStrHdrDetRst->ptrPicAddr[i];
					u4PicIndex++;
				}
			}
			break;

		default:
			break;
		}
	}

	prHdrDetResult->u1PicInfoCount = (u8) u4PicIndex;

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_Init*/
/* Initialize PVR, and Reset Parser Structure*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_HAL_Init(void)
{
	PVR_INPUT_TYPE_T eInputType = PVR_IN_PLAYBACK_MM;
	MRESULT mrRet = RET_DMX_OK;
	u8 u1DevIdx = 0;

	if (g_fgPSRHalInit)
		return RET_DMX_OK;

	g_fgPSRHalInit = TRUE;

	PSR_HAL_LOCK_INIT(mrRet);
	if (DMX_FAILED(mrRet))
		MM_RETURN(mrRet);

	PSR_HAL_LOCK;
	for (u1DevIdx = 0; u1DevIdx < DMX_DEV_CNT; u1DevIdx++) {
		mm_memset(&g_rPSRHalStruct[u1DevIdx], 0, sizeof(PSR_STRUCT_T));
		g_rPSRHalStruct[u1DevIdx].eVideoCodec = VC_UNKNOW;
#ifndef __linux__
		g_rPSRHalStruct[u1DevIdx].ePowerState = D0;
#endif				/* __linux__ */
		g_rPSRHalStruct[u1DevIdx].eState = DMX_HW_STATE_IDLE;
		g_rPSRHalStruct[u1DevIdx].pvOwner = NULL;
		g_rPSRHalStruct[u1DevIdx].u4GarbageSz = 0;
		DMX_NewMemory(sizeof(PSR_HDRDET_RESULT_T), g_rPSRHalStruct[u1DevIdx].prHdrDetResult);
		if (NULL == g_rPSRHalStruct[u1DevIdx].prHdrDetResult) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in alloc g_rPSRHalStruct")
						  TEXT(".prHdrDetResult\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);
			mrRet = RET_DMX_NO_MEM;
			PSR_HAL_UNLOCK;
			PSR_HAL_LOCK_UNINIT;
			g_fgPSRHalInit = FALSE;
			MM_RETURN(mrRet);
		}
	}

	mrRet = DMX_HAL_Init(eInputType);
	if (DMX_FAILED(mrRet)) {
		PSR_HAL_UNLOCK;
		PSR_HAL_LOCK_UNINIT;
		g_fgPSRHalInit = FALSE;
		MM_RETURN(mrRet);
	}

	PSR_HAL_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_Uninit*/
/* Unitialize PVR, and set Vcode type to be unknown*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
void PSR_HAL_Uninit(void)
{
	u8 u1DevIdx = 0;
	
	if (!g_fgPSRHalInit)
		return;

	PSR_HAL_LOCK;

	DMX_HAL_Uninit();

	for (u1DevIdx = 0; u1DevIdx < DMX_DEV_CNT; u1DevIdx++) {
		if (NULL != g_rPSRHalStruct[u1DevIdx].prHdrDetResult) {
			DMX_FreeMemory(g_rPSRHalStruct[u1DevIdx].prHdrDetResult);
			g_rPSRHalStruct[u1DevIdx].prHdrDetResult = NULL;
		}

		mm_memset(&g_rPSRHalStruct[u1DevIdx], 0, sizeof(PSR_STRUCT_T));
		g_rPSRHalStruct[u1DevIdx].eVideoCodec = VC_UNKNOW;
#ifndef __linux__
		g_rPSRHalStruct[u1DevIdx].ePowerState = D4;
#endif				/* __linux__ */
		g_rPSRHalStruct[u1DevIdx].eState = DMX_HW_STATE_UNKNOWN;
		g_rPSRHalStruct[u1DevIdx].pvOwner = NULL;
	}

	PSR_HAL_UNLOCK;

	PSR_HAL_LOCK_UNINIT;

	g_fgPSRHalInit = FALSE;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_HWRes_Obtain*/
/* Get the Parser Structure index, and set the parser structure to be enable*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
u8 PSR_HAL_HWRes_Obtain(void *pvOwner)
{
	UINT8 u1DevID = 0;
	PSR_CC *prPsrCC = (PSR_CC *)pvOwner;
	DMX_INST_T *prDmxInst = (DMX_INST_T *)(prPsrCC->pvDmxInst);

	if (DMX_INVALID_UINT8 == prDmxInst->u1DevID) {
		for (u1DevID = 0; u1DevID < DMX_DEV_CNT; u1DevID++){
			if ((!g_rPSRHalStruct[u1DevID].fgUsed) && ((DMX_HW_STATE_IDLE == g_rPSRHalStruct[u1DevID].eState) ||
		        (DMX_HW_STATE_UNKNOWN == g_rPSRHalStruct[u1DevID].eState))) {
		        g_rPSRHalStruct[u1DevID].eState = DMX_HW_STATE_OCCUPIED;
		        g_rPSRHalStruct[u1DevID].pvOwner = pvOwner;
				g_rPSRHalStruct[u1DevID].fgUsed = TRUE;
				DMXLOG_DEBUG(TEXT("[PSR] %s line %d, get unused u1DevID:%d.(1)\r\n"),
	            			DMX_FUNC_NAME, DMX_LINE_NO, u1DevID);
				prDmxInst->u1DevID = u1DevID;
		        return u1DevID;
		    }
		}
	} else {
		u1DevID = prDmxInst->u1DevID;
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d, cur used u1DevID:%d.(2)\r\n"),
	            			DMX_FUNC_NAME, DMX_LINE_NO, u1DevID);
		if ((DMX_HW_STATE_IDLE == g_rPSRHalStruct[u1DevID].eState) ||
		    (DMX_HW_STATE_UNKNOWN == g_rPSRHalStruct[u1DevID].eState)) {
	        g_rPSRHalStruct[u1DevID].eState = DMX_HW_STATE_OCCUPIED;
	        g_rPSRHalStruct[u1DevID].pvOwner = pvOwner;
	        return u1DevID;
	    }
	}

	return DMX_INVALID_UINT8;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_HWRes_Release*/
/* Set the parser structure to be disable*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_HAL_HWRes_Release(u8 u1DevID)
{
	if (DMX_HW_STATE_NEEDSUSPEND == g_rPSRHalStruct[u1DevID].eState) {
		DMXLOG_TRACE(TEXT("[PSR] %s -- Set PSRHAL state to be suspend\r\n"),
			    DMX_FUNC_NAME);
		PSR_HAL_SetPowerState(D4);
		MM_RETURN(RET_DMX_SUSPEND_OK);
	} else if (DMX_HW_STATE_OCCUPIED == g_rPSRHalStruct[u1DevID].eState) {
		g_rPSRHalStruct[u1DevID].eState = DMX_HW_STATE_IDLE;
		g_rPSRHalStruct[u1DevID].pvOwner = NULL;
		smp_mb();
	}

	MM_RETURN(RET_DMX_OK);
}

bool PSR_HAL_IsHWResOccupied(u8 u1DevID)
{
	E_DMX_HW_STATE_T eState = DMX_HW_STATE_IDLE;
	bool fgRet = FALSE;

	PSR_HAL_LOCK;

	smp_mb();

	eState = g_rPSRHalStruct[u1DevID].eState;

	DMXLOG_TRACE(TEXT("[PVR] %s -- g_rPSRHalStruct.eState: %d\r\n"),
		    DMX_FUNC_NAME, g_rPSRHalStruct[u1DevID].eState);
	fgRet = ((DMX_HW_STATE_OCCUPIED == g_rPSRHalStruct[u1DevID].eState) ||
		 (DMX_HW_STATE_NEEDSUSPEND == g_rPSRHalStruct[u1DevID].eState));

	PSR_HAL_UNLOCK;

	return fgRet;
}

void PSR_HAL_SetSuspend(u8 u1DevID, bool fgSuspend, void **ppvOwner)
{
	PSR_HAL_LOCK;
	if (DMX_HW_STATE_OCCUPIED == g_rPSRHalStruct[u1DevID].eState) {
		g_rPSRHalStruct[u1DevID].eState = DMX_HW_STATE_NEEDSUSPEND;
		if (NULL != ppvOwner)
			*ppvOwner = g_rPSRHalStruct[u1DevID].pvOwner;

		DMXLOG_TRACE(TEXT("[PSR] %s -- Demuxer HW is in occupied, ")
					  TEXT("Set NEEDSUSPEND, PsrCC: 0x%x\r\n"),
			    DMX_FUNC_NAME, g_rPSRHalStruct[u1DevID].pvOwner);
	} else {
		DMXLOG_TRACE(TEXT("[PSR] %s -- Demuxer HW is not in occupied\r\n"),
			    DMX_FUNC_NAME);
	}
	PSR_HAL_UNLOCK;
}

VCodeC PSR_HAL_GetVideoCodec(u8 u1DevID)
{
	VCodeC eCodec = VC_UNKNOW;

	PSR_HAL_LOCK;
	eCodec = g_rPSRHalStruct[u1DevID].eVideoCodec;
	PSR_HAL_UNLOCK;
	return eCodec;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_SetVideoCodec*/
/* Set Video Codec Type to PVR*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_HAL_SetVideoCodec(u8 u1DevID, VCodeC eVCodeC, bool fgOperForce)
{
	PVR_VIDEO_TYPE_T ePvrVideoType = PVR_VIDEO_UNKNOWN;

	PSR_HAL_LOCK;
	if ((g_rPSRHalStruct[u1DevID].eVideoCodec == eVCodeC) && (!fgOperForce)) {
		PSR_HAL_UNLOCK;
		MM_RETURN(RET_DMX_OK);
	}

	g_rPSRHalStruct[u1DevID].eVideoCodec = eVCodeC;

	switch (eVCodeC) {
	case VC_MPEG2:
		ePvrVideoType = PVR_VIDEO_MPEG;
		break;

	case VC_MPEG4:
	case VC_DIVX4:
	case VC_DIVX6:
		ePvrVideoType = PVR_VIDEO_MPEG4;
		break;

	case VC_H263:
		ePvrVideoType = PVR_VIDEO_H263;
		break;

	case VC_VC1:
		ePvrVideoType = PVR_VIDEO_VC1;
		break;

	case VC_H264:
		ePvrVideoType = PVR_VIDEO_H264;
		break;

	case VC_H265:
		ePvrVideoType = PVR_VIDEO_H265;
		break;

	case VC_RV30:
	case VC_RV40:
	case VC_DIVX3:
	case VC_WMV1:
	case VC_WMV2:
	case VC_WMV3:
	case VC_MJPEG:
	case VC_H263_SORENSON:
	case VC_VP6:
	case VC_VP6A:
	case VC_VP8:
	default:
		ePvrVideoType = PVR_VIDEO_UNKNOWN;
		break;
	}

	if (!DMX_HAL_SetVideoType(u1DevID, ePvrVideoType)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] % line %d fail in DMX_HAL_SetVideoType, ePvrVideoType: %d\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, ePvrVideoType);
		PSR_HAL_UNLOCK;
		MM_RETURN(RET_DMX_HW_ERROR);
	}
	PSR_HAL_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

u32 PSR_HAL_GetGarbageSz(u8 u1DevID)
{
	u32 u4GarbageSz = 0;

	PSR_HAL_LOCK;
	u4GarbageSz = g_rPSRHalStruct[u1DevID].u4GarbageSz;
	PSR_HAL_UNLOCK;
	return u4GarbageSz;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_PTransfer*/
/* 1. Get the PID index according to the Bit Type to Tranfer*/
/* 2. Set Destination Fifo Info into the corresponding PID data structure*/
/* 4. Set PID's Callback function*/
/* 5. Set Command Queue and Src Buffer Info into PVR, and trigger DDI transfer*/
/* 6. Set Destination fifo and Src buffer's info into Parser Structure*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_HAL_PTransfer(u8 u1DevID, PSR_HALPT_INFO_T *prPTransInfo)
{
	DMX_CMDCFG_INFO_T rDmxCfgInfo;
	MRESULT mrRet = RET_DMX_OK;

	PSR_HAL_LOCK;

	if (NULL == prPTransInfo) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for invalid args\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);
		PSR_HAL_UNLOCK;
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mm_memset(&rDmxCfgInfo, 0, sizeof(DMX_CMDCFG_INFO_T));

	rDmxCfgInfo.u1DevID = u1DevID;

	if (DDI_PVR_DMA_PATH_ID == u1DevID) {
		if (BitType_Video == prPTransInfo->eBitType) {
			rDmxCfgInfo.rPIDInfo.u4PIDIdx = PVR_PID_IDX_VIDEO;
			#if DMX_PFM_TEST
			DmxPfmStmHwDmaStart(SPT_DATA_V);
			#endif				/* DMX_PFM_TEST */
		} else if (BitType_Audio == prPTransInfo->eBitType) {
			rDmxCfgInfo.rPIDInfo.u4PIDIdx = PVR_PID_IDX_AUDIO;
			#if DMX_PFM_TEST
			DmxPfmStmHwDmaStart(SPT_DATA_A);
			#endif				/* DMX_PFM_TEST */
		} else if (BitType_Section == prPTransInfo->eBitType) {
			rDmxCfgInfo.rPIDInfo.u4PIDIdx = PVR_PID_IDX_SECTION;
		} else {
			rDmxCfgInfo.rPIDInfo.u4PIDIdx = PVR_PID_IDX_SP;
			#if DMX_PFM_TEST
			DmxPfmStmHwDmaStart(SPT_DATA_SP);
			#endif				/* DMX_PFM_TEST */
		}
	} else if (MINI_PVR_DMA_PATH_ID == u1DevID) {
		if (BitType_Video == prPTransInfo->eBitType){
	        rDmxCfgInfo.rPIDInfo.u4PIDIdx = MINI_PVR_PID_IDX_VIDEO;
	        #if DMX_PFM_TEST
	        DmxPfmStmHwDmaStart(SPT_DATA_V);
	        #endif // DMX_PFM_TEST
	    } else if (BitType_Audio == prPTransInfo->eBitType) {
	        rDmxCfgInfo.rPIDInfo.u4PIDIdx = MINI_PVR_PID_IDX_AUDIO;
	        #if DMX_PFM_TEST
	        DmxPfmStmHwDmaStart(SPT_DATA_A);
	        #endif // DMX_PFM_TEST
	    } else if (BitType_Section == prPTransInfo->eBitType) {
	        rDmxCfgInfo.rPIDInfo.u4PIDIdx = MINI_PVR_PID_IDX_SECTION;
	    }
	    else {
	        rDmxCfgInfo.rPIDInfo.u4PIDIdx = MINI_PVR_PID_IDX_SP;
	        #if DMX_PFM_TEST
	        DmxPfmStmHwDmaStart(SPT_DATA_SP);
	        #endif // DMX_PFM_TEST
	    }
	} else {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for u1DevID is invalid.\r\n"),
                DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/* Set Source Buffer info */
	rDmxCfgInfo.ptrSrcBufAddr = prPTransInfo->ptrSrcSa;
	rDmxCfgInfo.u4SrcBufSize = prPTransInfo->u4SrcLen;
	rDmxCfgInfo.ptrSrcEndAddr = prPTransInfo->ptrSrcSa + prPTransInfo->u4SrcLen;
	rDmxCfgInfo.ptrSrcBufRPtr = prPTransInfo->ptrSrcSa;
	rDmxCfgInfo.ptrSrcBufWPtr = prPTransInfo->ptrSrcSa + prPTransInfo->u4SrcLen;

	if (prPTransInfo->fgUseCmdQ) {
		DMX_CMDQ_TX_ENTRY_T *prTxEntry =
		    (DMX_CMDQ_TX_ENTRY_T *) (prPTransInfo->u4CmdQTxEntryBuffer);
		PVR_CMDQ_ENTRY_T arComQItem[DMX_MAX_TX_CNT_FOR_CMD_Q];
		u32 u4CmdQSrcDataSz;
		u32 u4SrcLen = 0;
		/*Command Number */
		u16 u2CmdQEntryCnt = prPTransInfo->u2EntryWrIdx - prPTransInfo->u2EntryRdIdx;
		u16 u2Idx = 0;

		if (u2CmdQEntryCnt > DMX_MAX_TX_CNT_FOR_CMD_Q) {
			PSR_HAL_UNLOCK;
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for CmdQEntryCnt(%d)")
						  TEXT(" > DMX_MAX_TX_CNT_FOR_CMD_Q(%d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    u2CmdQEntryCnt, DMX_MAX_TX_CNT_FOR_CMD_Q);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_OVER_LIMIT);
		}

		if ((u2CmdQEntryCnt > 1) && (NULL == prTxEntry)) {
			PSR_HAL_UNLOCK;
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for CmdQEntryCnt(%d)")
						  TEXT(" > 1, but no CmdEntry\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u2CmdQEntryCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		u4CmdQSrcDataSz = 0;
		u4SrcLen = prPTransInfo->u4RdIdxLen;
		arComQItem[0].u4SkipLen = prPTransInfo->u4RdIdxOfst;
		arComQItem[0].u4PayloadLen = u4SrcLen;
		u4CmdQSrcDataSz += (arComQItem[0].u4SkipLen + arComQItem[0].u4PayloadLen);

		prTxEntry += prPTransInfo->u2EntryRdIdx;

		for (u2Idx = 1; u2Idx < u2CmdQEntryCnt; u2Idx++) {
			prTxEntry++;
			u4SrcLen = (u2Idx == (u2CmdQEntryCnt - 1)) ?
			    prPTransInfo->u4LastValidIdxLen : prTxEntry->u4TxLen;
			arComQItem[u2Idx].u4SkipLen = prTxEntry->u4TxOfst;
			arComQItem[u2Idx].u4PayloadLen = u4SrcLen;
			u4CmdQSrcDataSz += (arComQItem[u2Idx].u4SkipLen +
					    arComQItem[u2Idx].u4PayloadLen);
		}

		if (u4CmdQSrcDataSz > prPTransInfo->u4SrcLen) {
			PSR_HAL_UNLOCK;
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for CmdQEntrys")
						  TEXT
						  ("(Cnt:%d)'s total data Sz(%d) > SrcSz(%d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u2CmdQEntryCnt, u4CmdQSrcDataSz,
				    prPTransInfo->u4SrcLen);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_OVER_LIMIT);
		}

		rDmxCfgInfo.rPTXCmdInfo.u1CmdNum = (u8) u2CmdQEntryCnt;

		rDmxCfgInfo.rPTXCmdInfo.prEntry = &arComQItem[0];
	} else {
		rDmxCfgInfo.rPTXCmdInfo.u1CmdNum = 1;
		rDmxCfgInfo.rPTXCmdInfo.prEntry = NULL;
	}

	/* Set Destination Fifo info */
	rDmxCfgInfo.ptrDstFifoAddr = prPTransInfo->ptrFifoSa;
	rDmxCfgInfo.u4DstFifoSize = prPTransInfo->u4FifoSz;
	rDmxCfgInfo.ptrDstFifoWPtr = prPTransInfo->ptrFifoWrPtr;
	rDmxCfgInfo.ptrDstFifoRPtr = prPTransInfo->ptrFifoRdPtr;
	rDmxCfgInfo.ptrDstFifoSPtr = prPTransInfo->ptrFifoWrPtr;
	/* Keep Payload starting pointer always is equel to WP */

	mm_memset(&(rDmxCfgInfo.rPTXInstBsInfo), 0, sizeof(rDmxCfgInfo.rPTXInstBsInfo));

#if ENABLE_DMX_ADVANCED_VER
	mm_memcpy(&(rDmxCfgInfo.rPTXInstBsInfo), &(prPTransInfo->rInstBytesInfo),
		  sizeof(rDmxCfgInfo.rPTXInstBsInfo));
#endif				/* ENABLE_DMX_ADVANCED_VER */

	rDmxCfgInfo.rPIDInfo.rPidFunc.pfnNotify = prPTransInfo->rPidFunc.pfnNotify;
	rDmxCfgInfo.rPIDInfo.rPidFunc.pvPrivData = prPTransInfo->rPidFunc.pvPrivData;

	rDmxCfgInfo.rGlobalFunc.pfnCB = prPTransInfo->rGlobalFunc.pfnCB;
	rDmxCfgInfo.rGlobalFunc.pvPrivData = prPTransInfo->rGlobalFunc.pvPrivData;

	/* Set Destination fifo and Src buffer's info into Parser Structure */
	mrRet = _PSR_SetPsrStructInfo(prPTransInfo);
	if (DMX_FAILED(mrRet)) {
		PSR_HAL_UNLOCK;
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d fail in _PSR_SetPsrStructInfo, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	rDmxCfgInfo.rPIDInfo.eDescMode = prPTransInfo->eDescMode;

	mm_memcpy(&(rDmxCfgInfo.rPIDInfo.rDRMInfo), &(prPTransInfo->rDRMInfo),
		  sizeof(PVR_DRM_PARAM_T));

	mrRet = DMX_HAL_DMXConfig(&rDmxCfgInfo);
	if (DMX_FAILED(mrRet)) {
		PSR_HAL_UNLOCK;
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in DMX_HAL_DMXConfig,")
					  TEXT(" mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		DMXLOG_ERROR(TEXT("[PVR] %s line %d fail for PID Index(%d) -- ")
					  TEXT
					  ("prPTransInfo(Addr(0x%x), Size(0x%x), WPtr(0x%x), RPtr(0x%x),")
					  TEXT(" SPtr(0x%x))\r\n"), DMX_FUNC_NAME, DMX_LINE_NO,
			    rDmxCfgInfo.rPIDInfo.u4PIDIdx, prPTransInfo->ptrFifoSa,
			    prPTransInfo->u4FifoSz, prPTransInfo->ptrFifoWrPtr,
			    prPTransInfo->ptrFifoRdPtr, prPTransInfo->ptrFifoWrPtr);
		MM_RETURN(mrRet);
	}
	PSR_HAL_UNLOCK;

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_GetWPtr*/
/* Get Fifo Write Address(Vir) for designated BitType*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_HAL_GetWPtr(u8 u1DevID, PSR_HAL_BitType eBitType, uintptr_t *pptrCurWPtr)
{
	u32 u4PIDIdx;
	MRESULT mrRet = RET_DMX_OK;

	if (!(NULL != pptrCurWPtr)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PSR_HAL_LOCK;
	
	if (DDI_PVR_DMA_PATH_ID == u1DevID){
		if (BitType_Video == eBitType)
			u4PIDIdx = PVR_PID_IDX_VIDEO;
		else if (BitType_Audio == eBitType)
			u4PIDIdx = PVR_PID_IDX_AUDIO;
		else if (BitType_Section == eBitType)
			u4PIDIdx = PVR_PID_IDX_SECTION;
		else
			u4PIDIdx = PVR_PID_IDX_SP;
	} else if (MINI_PVR_DMA_PATH_ID == u1DevID) {
		if (BitType_Video == eBitType)
	    	u4PIDIdx = MINI_PVR_PID_IDX_VIDEO;
	    else if (BitType_Audio == eBitType)
	        u4PIDIdx = MINI_PVR_PID_IDX_AUDIO;
	    else if (BitType_Section == eBitType)
	        u4PIDIdx = MINI_PVR_PID_IDX_SECTION;
	    else
	        u4PIDIdx = MINI_PVR_PID_IDX_SP;
	} else {
		PSR_HAL_UNLOCK;
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for u1DevID(%d).\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO, u1DevID);
		MM_RETURN(RET_DMX_HW_ERROR);
	}
	*pptrCurWPtr = (u32) (DMX_NONCACHE(DMX_HAL_GetFIFOWPtr(u4PIDIdx, FFType_ES)));

#ifdef __linux__
#if DMX_CHECK_MEM_VALIBILITY
	if (0 == *pptrCurWPtr) {
		PSR_HAL_UNLOCK;
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for *pptrCurWPtr(0x%x) is 0\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, *pptrCurWPtr);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}
#endif				/* DMX_CHECK_MEM_VALIBILITY */
#endif				/* __linux__ */

	PSR_HAL_UNLOCK;

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_SetWPtr*/
/* Set Fifo Write Address(Vir) for designated BitType*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_HAL_SetWPtr(u8 u1DevID, PSR_HAL_BitType eBitType, uintptr_t ptrCurWPtr)
{
	uintptr_t ptrPhyAddr = 0;
	u32 u4PIDIdx = DMX_INVALID_UINT32;

	PSR_HAL_LOCK;

	if (DDI_PVR_DMA_PATH_ID == u1DevID){
		if (BitType_Video == eBitType)
			u4PIDIdx = PVR_PID_IDX_VIDEO;
		else if (BitType_Audio == eBitType)
			u4PIDIdx = PVR_PID_IDX_AUDIO;
		else if (BitType_Section == eBitType)
			u4PIDIdx = PVR_PID_IDX_SECTION;
		else
			u4PIDIdx = PVR_PID_IDX_SP;
	} else if(MINI_PVR_DMA_PATH_ID == u1DevID) {
		if (BitType_Video == eBitType)
	        u4PIDIdx = MINI_PVR_PID_IDX_VIDEO;
	    else if (BitType_Audio == eBitType)
	        u4PIDIdx = MINI_PVR_PID_IDX_AUDIO;
	    else if (BitType_Section == eBitType)
	        u4PIDIdx = MINI_PVR_PID_IDX_SECTION;
	    else
	        u4PIDIdx = MINI_PVR_PID_IDX_SP;
	} else {
		PSR_HAL_UNLOCK;
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for u1DevID(%d).\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO, u1DevID);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

#ifdef __linux__
#if DMX_CHECK_MEM_VALIBILITY
	if (0 == ptrCurWPtr) {
		PSR_HAL_UNLOCK;
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for ptrCurWPtr(0x%x) is 0\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ptrCurWPtr);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}
#endif				/* DMX_CHECK_MEM_VALIBILITY */
#endif				/* __linux__ */

	ptrPhyAddr = DMX_PHYSICAL(ptrCurWPtr);
	if (0 == ptrPhyAddr) {
		PSR_HAL_UNLOCK;
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DMX_HAL_SetFIFOWPtr(u4PIDIdx, FFType_ES, ptrPhyAddr);
	PSR_HAL_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_GetRPtr*/
/* Get Fifo Read Address(Vir) for designated BitType*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_HAL_GetRPtr(u8 u1DevID, PSR_HAL_BitType eBitType, uintptr_t *pptrCurRPtr)
{
	u32 u4PIDIdx;
	MRESULT mrRet = RET_DMX_OK;

	if (!(NULL != pptrCurRPtr)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PSR_HAL_LOCK;
	
	if (DDI_PVR_DMA_PATH_ID == u1DevID) {
		if (BitType_Video == eBitType)
			u4PIDIdx = PVR_PID_IDX_VIDEO;
		else if (BitType_Audio == eBitType)
			u4PIDIdx = PVR_PID_IDX_AUDIO;
		else if (BitType_Section == eBitType)
			u4PIDIdx = PVR_PID_IDX_SECTION;
		else
			u4PIDIdx = PVR_PID_IDX_SP;
	} else if(MINI_PVR_DMA_PATH_ID == u1DevID) {
		if (BitType_Video == eBitType)
	        u4PIDIdx = MINI_PVR_PID_IDX_VIDEO;
	    else if (BitType_Audio == eBitType)
	        u4PIDIdx = MINI_PVR_PID_IDX_AUDIO;
	    else if (BitType_Section == eBitType)
	        u4PIDIdx = MINI_PVR_PID_IDX_SECTION;
	    else
	        u4PIDIdx = MINI_PVR_PID_IDX_SP;
	} else {
		PSR_HAL_UNLOCK;
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for u1DevID(%d).\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO, u1DevID);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	*pptrCurRPtr = (u32) (DMX_NONCACHE(DMX_HAL_GetFIFORPtr(u4PIDIdx, FFType_ES)));

#ifdef __linux__
#if DMX_CHECK_MEM_VALIBILITY
	if (0 == *pptrCurRPtr) {
		PSR_HAL_UNLOCK;
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for *pptrCurRPtr(0x%x) is 0\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, *pptrCurRPtr);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}
#endif				/* DMX_CHECK_MEM_VALIBILITY */
#endif				/* __linux__ */

	PSR_HAL_UNLOCK;

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_SetRPtr*/
/* Set Fifo Read Address(Vir) for designated BitType*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_HAL_SetRPtr(u8 u1DevID, PSR_HAL_BitType eBitType, uintptr_t ptrCurReadPtr)
{
	u32 u4PIDIdx;
	uintptr_t ptrPhyAddr = 0;

	PSR_HAL_LOCK;

	if (DDI_PVR_DMA_PATH_ID == u1DevID) {
		if (BitType_Video == eBitType)
			u4PIDIdx = PVR_PID_IDX_VIDEO;
		else if (BitType_Audio == eBitType)
			u4PIDIdx = PVR_PID_IDX_AUDIO;
		else if (BitType_Section == eBitType)
			u4PIDIdx = PVR_PID_IDX_SECTION;
		else
			u4PIDIdx = PVR_PID_IDX_SP;
	} else if(MINI_PVR_DMA_PATH_ID == u1DevID) {
		if (BitType_Video == eBitType)
	        u4PIDIdx = MINI_PVR_PID_IDX_VIDEO;
	    else if (BitType_Audio == eBitType)
	        u4PIDIdx = MINI_PVR_PID_IDX_AUDIO;
	    else if (BitType_Section == eBitType)
	        u4PIDIdx = MINI_PVR_PID_IDX_SECTION;
	    else
	        u4PIDIdx = MINI_PVR_PID_IDX_SP;
	} else {
		PSR_HAL_UNLOCK;
        PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for u1DevID(%d).\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO, u1DevID);
        MM_RETURN(RET_DMX_HW_ERROR);
	}
#ifdef __linux__
#if DMX_CHECK_MEM_VALIBILITY
	if (0 == ptrCurReadPtr) {
		PSR_HAL_UNLOCK;
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for ptrCurReadPtr(0x%x) is 0\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ptrCurReadPtr);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}
#endif				/* DMX_CHECK_MEM_VALIBILITY */
#endif				/* __linux__ */

	ptrPhyAddr = DMX_PHYSICAL(ptrCurReadPtr);
	if (0 == ptrPhyAddr) {
		PSR_HAL_UNLOCK;
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DMX_HAL_SetFIFORPtr(u4PIDIdx, FFType_ES, ptrPhyAddr);
	PSR_HAL_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_SetLastHdrDetStatus*/
/* Set the Video's Header Buffer SA, EA, Start Ptr, WP, RP to PVR*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_HAL_SetLastHdrDetStatus(u8 u1DevID, PSR_HAL_BitType eBitType, PSR_HDRDET_STATUS_T *prHdrDetStatus)
{
	MRESULT mrRet = RET_DMX_OK;

	PSR_HAL_LOCK;
	if (DDI_PVR_DMA_PATH_ID == u1DevID) {
		switch (eBitType) {
		case BitType_Video:
			if (NULL == prHdrDetStatus) {
				PSR_HAL_UNLOCK;
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DMX_HAL_SetHdrDetStatus(PVR_PID_IDX_VIDEO,
							&(prHdrDetStatus->u4HdrDetBufSa));
			break;
		case BitType_Audio:
			if (NULL == prHdrDetStatus) {
				PSR_HAL_UNLOCK;
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DMX_HAL_SetHdrDetStatus(PVR_PID_IDX_AUDIO,
							&(prHdrDetStatus->u4HdrDetBufSa));
			break;
		case BitType_SubPic0:
		case BitType_SubPic1:
			mrRet = DMX_HAL_SetHdrDetStatus(PVR_PID_IDX_SP, &(prHdrDetStatus->u4HdrDetBufSa));
			break;
		case BitType_Section:
			mrRet = DMX_HAL_SetHdrDetStatus(PVR_PID_IDX_SECTION,
							&(prHdrDetStatus->u4HdrDetBufSa));
			break;
		default:
			PSR_HAL_UNLOCK;
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	} else if (MINI_PVR_DMA_PATH_ID == u1DevID) { 
		switch (eBitType) {
	    case BitType_Video:
	        if (NULL == prHdrDetStatus)
	        {
	            PSR_HAL_UNLOCK;
	            MM_RETURN(RET_DMX_PARAM_WRONG);
	        }

	        mrRet = DMX_HAL_SetHdrDetStatus(MINI_PVR_PID_IDX_VIDEO, &(prHdrDetStatus->u4HdrDetBufSa));
	        break;
	    case BitType_Audio:
	        if (NULL == prHdrDetStatus)
	        {
	            PSR_HAL_UNLOCK;
	            MM_RETURN(RET_DMX_PARAM_WRONG);
	        }

	        mrRet = DMX_HAL_SetHdrDetStatus(MINI_PVR_PID_IDX_AUDIO, &(prHdrDetStatus->u4HdrDetBufSa));
	        break;
	    case BitType_SubPic0:
	    case BitType_SubPic1:
	        mrRet = DMX_HAL_SetHdrDetStatus(MINI_PVR_PID_IDX_SP, &(prHdrDetStatus->u4HdrDetBufSa));
	        break;
	    case BitType_Section:
	        mrRet = DMX_HAL_SetHdrDetStatus(MINI_PVR_PID_IDX_SECTION, &(prHdrDetStatus->u4HdrDetBufSa));
	        break;
	    default:
	        PSR_HAL_UNLOCK;
	        MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	} else {
		PSR_HAL_UNLOCK;
		PVR_LOG_ERR(TEXT("[PSR] %s, line %d, fail for invalid args(u1DevID:%d).\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO, u1DevID);
		MM_RETURN(RET_DMX_HW_ERROR);
	}
	PSR_HAL_UNLOCK;

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_KeepHdrDetResult*/
/* Get the Video Picturetypes and PictureAddresses from the designated DMX_FTI_INTSTATUS_T*/
/* And set them to the Parser Struture's rHdrDecResult*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_HAL_KeepHdrDetResult(u8 u1DevID, DMX_FTI_INTSTATUS_T *pStatus)
{
	PSR_STRUCT_T *prPSRStruct = (PSR_STRUCT_T *) (&g_rPSRHalStruct[u1DevID]);
	DMX_PIC_INFO_T *prPicInfos = NULL;
	u8 *pu1PicType = NULL;
	u32 *pptrPicAddr = NULL;
	u32 u4HdrCnt = 0;
	u32 i;

	if ((NULL == pStatus) || (NULL == prPSRStruct)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for invalid args\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((DMX_HW_STATE_OCCUPIED != prPSRStruct->eState) &&
	    (DMX_HW_STATE_NEEDSUSPEND != prPSRStruct->eState)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d fail in PSR Struct is disable\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (NULL == prPSRStruct->prHdrDetResult) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for prPSRStruct->")
					  TEXT("prHdrDetResult == NULL!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	dmx_memset((void *) (prPSRStruct->prHdrDetResult), 0, sizeof(PSR_HDRDET_RESULT_T));

	pu1PicType = (u8 *) (&(prPSRStruct->prHdrDetResult->u1PicType[0]));
	pptrPicAddr = (u32 *) (&(prPSRStruct->prHdrDetResult->ptrPicAddr[0]));

	u4HdrCnt = pStatus->rHdrDectResult.u4PicInfoCount;
	prPicInfos = pStatus->rHdrDectResult.prPicsInfo;

	prPSRStruct->prHdrDetResult->u1PicInfoCount = (u8) u4HdrCnt;
	if (NULL == prPicInfos) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d fail for PicInfos is NULL\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	for (i = 0; i < u4HdrCnt; i++, prPicInfos++) {
		*pu1PicType = prPicInfos->u1PicType;
		*pptrPicAddr = prPicInfos->ptrPicStartAddr;

		DMXLOG_DEBUG(
			    TEXT("[PSR] %s -- (%d) --> PicType: %u, PicAddr: 0x%x\r\n"),
			    DMX_FUNC_NAME, prPicInfos->u1PicType, prPicInfos->ptrPicStartAddr);

		pu1PicType++;
		pptrPicAddr++;
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_HAL_GetHdrDetResult*/
/* Get the StartCode PictureType-Address Pairs information from the Parser Structure*/
/* Get the Video PES header Buffer current information, include SA, EA, Start Ptr, WP, */
/*RP, set them into the u4HdrDecBufSa, ....*/
/* @Param prHdrDetStatus [in/out]  -- the Video PES header Buffer current information, */
/*include SA, EA, Start Ptr, WP, RP*/
/* @Param prHdrDetResult [out] obtained StartCode PictureType-Address Pairs info.*/
/*/////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_HAL_GetHdrDetResult(void *pvPsrFtr,
				PSR_HDRDET_RESULT_T *prHdrDetResult,
				PSR_HDRDET_STATUS_T *prHdrDetStatus)
{
	PSR_FILTER *prPsrFtr = (PSR_FILTER *) pvPsrFtr;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrFtr)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	PSR_HAL_LOCK;

	if (DDI_PVR_DMA_PATH_ID == prPsrFtr->ucHwDevId) {
		switch (prPsrFtr->eType) {
		case SPT_DATA_V:
			{
				/* Get the StartCode PictureType-Address Pairs information from the Parser Structure */
				mrRet = _PSR_GetPicHdrDetectStatus_SWKeepResult(prPsrFtr->ucHwDevId, prHdrDetResult);
				if (DMX_FAILED(mrRet)) {
					PSR_HAL_UNLOCK;
					DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in _PSR_Get")
								  TEXT
								  ("PicHdrDetectStatus_SWKeepResult, mrRet: 0x%x\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
					MM_RETURN(mrRet);
				}

				if (NULL == prHdrDetStatus) {
					PSR_HAL_UNLOCK;
					MM_RETURN(RET_DMX_PARAM_WRONG);
				}

				/* Get the Video PES header Buffer current information, include SA, EA, Start Ptr, WP, RP */
				mrRet = DMX_HAL_GetHdrDetStatus(PVR_PID_IDX_VIDEO,
								&(prHdrDetStatus->u4HdrDetBufSa));
			}
			break;
		case SPT_DATA_A:
			{
				if (NULL == prHdrDetStatus) {
					PSR_HAL_UNLOCK;
					MM_RETURN(RET_DMX_PARAM_WRONG);
				}
				/* Get the Audio PES header Buffer current information, include SA, EA, Start Ptr, WP, RP */
				mrRet = DMX_HAL_GetHdrDetStatus(PVR_PID_IDX_AUDIO,
								&(prHdrDetStatus->u4HdrDetBufSa));
			}
			break;
		case SPT_DATA_SP:
			{
				if (NULL == prHdrDetStatus) {
					PSR_HAL_UNLOCK;
					MM_RETURN(RET_DMX_PARAM_WRONG);
				}
				/* Get the SP PES header Buffer current information, include SA, EA, Start Ptr, WP, RP */
				mrRet = DMX_HAL_GetHdrDetStatus(PVR_PID_IDX_SP,
								&(prHdrDetStatus->u4HdrDetBufSa));
			}
			break;
		case SPT_DATA_SECTION:
			{
				if (NULL == prHdrDetStatus) {
					PSR_HAL_UNLOCK;
					MM_RETURN(RET_DMX_PARAM_WRONG);
				}
				/* Get the Section PES header Buffer current information, include SA, EA, Start Ptr, WP, RP */
				mrRet = DMX_HAL_GetHdrDetStatus(PVR_PID_IDX_SECTION,
								&(prHdrDetStatus->u4HdrDetBufSa));
			}
			break;
		default:
			break;
		}
	} else if (MINI_PVR_DMA_PATH_ID == prPsrFtr->ucHwDevId) {
		switch (prPsrFtr->eType) {
		case SPT_DATA_V:
		    {
		        // Get the StartCode PictureType-Address Pairs information from the Parser Structure
		        mrRet = _PSR_GetPicHdrDetectStatus_SWKeepResult(prPsrFtr->ucHwDevId, prHdrDetResult);
		        if (DMX_FAILED(mrRet)) {
		            PSR_HAL_UNLOCK;
		            PVR_LOG_ERR(TEXT("[PSR] %s line %d fail in _PSR_GetPicHdrDetectStatus_SWKeepResult, mrRet: 0x%x\r\n"),
		                DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		            MM_RETURN(mrRet);
		        }

		        if (NULL == prHdrDetStatus) {
		            PSR_HAL_UNLOCK;
		            MM_RETURN(RET_DMX_PARAM_WRONG);
		        }

		        // Get the Video PES header Buffer current information, include SA, EA, Start Ptr, WP, RP
		        mrRet = DMX_HAL_GetHdrDetStatus(MINI_PVR_PID_IDX_VIDEO, &(prHdrDetStatus->u4HdrDetBufSa));
		    }
		    break;
		case SPT_DATA_A:
		    {
		        if (NULL == prHdrDetStatus) {
		            PSR_HAL_UNLOCK;
		            MM_RETURN(RET_DMX_PARAM_WRONG);
		        }
		        // Get the Audio PES header Buffer current information, include SA, EA, Start Ptr, WP, RP
		        mrRet = DMX_HAL_GetHdrDetStatus(MINI_PVR_PID_IDX_AUDIO, &(prHdrDetStatus->u4HdrDetBufSa));
		    }
		    break;
		case SPT_DATA_SP:
		    {
		        if (NULL == prHdrDetStatus) {
		            PSR_HAL_UNLOCK;
		            MM_RETURN(RET_DMX_PARAM_WRONG);
		        }
		        // Get the SP PES header Buffer current information, include SA, EA, Start Ptr, WP, RP
		        mrRet = DMX_HAL_GetHdrDetStatus(MINI_PVR_PID_IDX_SP, &(prHdrDetStatus->u4HdrDetBufSa));
		    }
		    break;
		case SPT_DATA_SECTION:
		    {
		        if (NULL == prHdrDetStatus) {
		            PSR_HAL_UNLOCK;
		            MM_RETURN(RET_DMX_PARAM_WRONG);
		        }
		        // Get the Section PES header Buffer current information, include SA, EA, Start Ptr, WP, RP
		        mrRet = DMX_HAL_GetHdrDetStatus(MINI_PVR_PID_IDX_SECTION, &(prHdrDetStatus->u4HdrDetBufSa));
		    }
		    break;
		default:
		    break;
		}
	} else {
		PSR_HAL_UNLOCK;
		PVR_LOG_ERR(TEXT("[PSR] %s line %d fail for device ID(%d) is invalid.\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->ucHwDevId);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	PSR_HAL_UNLOCK;

	MM_RETURN(mrRet);
}

MRESULT PSR_HAL_ParsingIntData(void *pvHwData)
{
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PSR] %s enter, pvHwData: 0x%x\r\n"),
		    DMX_FUNC_NAME, pvHwData);
	if (NULL == pvHwData) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for invalid arsgs\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(mrRet);
	}

	smp_mb();

	DMXLOG_DEBUG(
		    TEXT("[PSR] %s enter, DMX_HAL_ProcIntData, pvHwData: 0x%x\r\n"),
		    DMX_FUNC_NAME, pvHwData);

	PSR_HAL_LOCK;
	mrRet = DMX_HAL_ProcIntData(pvHwData);
	if (DMX_FAILED(mrRet)) {
		PSR_HAL_UNLOCK;
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in DMX_HAL_ProcIntData,")
					  TEXT(" mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}
	PSR_HAL_UNLOCK;

	DMXLOG_DEBUG(TEXT("[PSR] %s exit, pvHwData: 0x%x\r\n"),
		    DMX_FUNC_NAME, pvHwData);

	MM_RETURN(mrRet);
}

MRESULT PSR_HAL_GlobalCB(void *pvData, void *pvUserPrivate)
{
	PSR_CC *prPsrCC = (PSR_CC *) pvUserPrivate;
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PSR] %s enter\r\n"), DMX_FUNC_NAME);

	if ((NULL == pvUserPrivate) || (NULL == pvData)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d fail for pvUserPrivate is NULL\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC->pvHwData = pvData;

	smp_mb();
	mrRet = SplitterSendNfy(prPsrCC->pvSptHdl, DMX_SPT_NTY_TX_HW_CB);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in SplitterSendNfy")
					  TEXT("(pvSptHdl: 0x%p, DMX_SPT_NTY_TX_HW_CB)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->pvSptHdl);
		MM_RETURN(mrRet);
	}

	DMXLOG_DEBUG(TEXT("[PSR] %s exit, success\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}


void PSR_HAL_SetInsertBytes4ToFifo4CmdQ(PSR_FILTER *prPsrFtr, PSR_CC *prPsrCC)
{
#if ENABLE_DMX_ADVANCED_VER
	u16 u2EntryIdx = 0;
	DMX_CMDQ_TX_ENTRY_T *prTxEntry = NULL;
	u32 u4EndLen = 0;
	uintptr_t ptrFifoVirWPtr = prPsrCC->ptrPTXFifoWrPtr;

	for (u2EntryIdx = prPsrCC->rCmdQTxInf.u2CurTxRngSIdx;
	     (u2EntryIdx <= prPsrCC->rCmdQTxInf.u2CurTxRngEIdx) &&
	     (u2EntryIdx < prPsrCC->u2TxEntryCnt); u2EntryIdx++) {
		bool fgInsert = FALSE;
		u32 u4DmaLen = 0;

		prTxEntry = ((DMX_CMDQ_TX_ENTRY_T *)(prPsrCC->u4CmdQTxEntryBuffer)) +
			u2EntryIdx;
		if (u2EntryIdx == prPsrCC->rCmdQTxInf.u2CurTxRngSIdx) {
			if ((prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen == prTxEntry->u4TxLen) &&
			    (prTxEntry->fgInsertHdr))
				fgInsert = TRUE;
			u4DmaLen = prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen;
		} else if (u2EntryIdx == prPsrCC->rCmdQTxInf.u2CurTxRngEIdx) {
			if ((prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen > 0) &&
			    (prTxEntry->fgInsertHdr))
				fgInsert = TRUE;
			u4DmaLen = prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen;
		} else {
			if (prTxEntry->fgInsertHdr)
				fgInsert = TRUE;
			u4DmaLen = prTxEntry->u4TxLen;
		}

		if (fgInsert) {
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
				TEXT("[PSR] %s -- CmdQDma, InsertHdr(TRUE), EntryIdx(%d),")
				 TEXT(" EntryDmaLen(%d), TxLen(%d)\r\n"), DMX_FUNC_NAME,
				u2EntryIdx, u4DmaLen, prTxEntry->u4TxLen);
			u4EndLen = 0;
			if (ptrFifoVirWPtr + prTxEntry->u4InsertHdrLen > prPsrFtr->ptrESFifoEa) {
				u4EndLen = prPsrFtr->ptrESFifoEa - ptrFifoVirWPtr;
				if (u4EndLen > 0) {
					dmx_memcpy((void *) ptrFifoVirWPtr, prTxEntry->au1InsertHdr,
						   u4EndLen);
					ptrFifoVirWPtr = prPsrFtr->ptrESFifoSa;
				}
			}

			if (prTxEntry->u4InsertHdrLen > u4EndLen) {
				dmx_memcpy((void *) ptrFifoVirWPtr,
					   prTxEntry->au1InsertHdr + u4EndLen,
					   prTxEntry->u4InsertHdrLen - u4EndLen);
			}
			ptrFifoVirWPtr += prTxEntry->u4InsertHdrLen - u4EndLen;
			if (ptrFifoVirWPtr >= prPsrFtr->ptrESFifoEa)
				ptrFifoVirWPtr -= prPsrFtr->u4ESFifoSize;
		} else {
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
				TEXT("[PSR] %s -- CmdQDma, InsertHdr(FALSE), EntryIdx(%d), ")
	      TEXT("EntryDmaLen(%d), TxLen(%d)\r\n"),
				DMX_FUNC_NAME, u2EntryIdx, u4DmaLen, prTxEntry->u4TxLen);
		}

		ptrFifoVirWPtr += u4DmaLen;
		if (ptrFifoVirWPtr >= prPsrFtr->ptrESFifoEa)
			ptrFifoVirWPtr -= prPsrFtr->u4ESFifoSize;
	}
#endif
}

MRESULT PSR_HAL_PIDCB(u8 u1PidIdx, PVR_NOTIFY_CODE_T eCode, u32 u4Data,
		      const void *pvNotifyTag)
{
	PSR_FILTER *prPsrFtr = (PSR_FILTER *) pvNotifyTag;
	PSR_CC *prPsrCC = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PSR] %s enter\r\n"), DMX_FUNC_NAME);

	if (PVR_NOTIFY_CODE_ES != eCode) {
		DMXLOG_TRACE(
			    TEXT("[PSR] %s exit for NotifyCode(%d) != PVR_NOTIFY_CODE_ES\r\n"),
			    DMX_FUNC_NAME, eCode);
		MM_RETURN(RET_DMX_OK);
	}

	if (NULL == prPsrFtr) {
		DMXLOG_TRACE(TEXT("[PSR] %s exit for pvNotifyTag == NULL\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	prPsrCC = (PSR_CC *) prPsrFtr->pvPsrCC;
	if (NULL == prPsrCC) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d fail for prPsrFtr->hPsrCC is NULL\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_CC);
	}

	if (prPsrCC->fgUseCmdQ) {
		prPsrCC->fgChkedAndWaitTx = FALSE;

		/* if unfinished, backup previous cmd queue tx info */
		if (prPsrCC->rCmdQTxInf.u8RmnTotalRealTxLen > 0) {
			dmx_memcpy(&(prPsrCC->rCmdQPrevTxInf), &(prPsrCC->rCmdQTxInf),
				   sizeof(PSR_CMDQ_TX_INF));
		}
#if ENABLE_DMX_ADVANCED_VER
		PSR_HAL_SetInsertBytes4ToFifo4CmdQ(prPsrFtr, prPsrCC);
#endif				/* ENABLE_DMX_ADVANCED_VER */
	}
	/* change state to wait IRQ process */
	PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_IRQ_PROC);

	if (SPT_DATA_V == prPsrFtr->eType) {
		PSR_VFSD *prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;

		if (!PSR_IsNonHdrVideoType(prVFSD->eVCodeC)) {
			mrRet = PSR_HAL_KeepHdrDetResult(prPsrFtr->ucHwDevId, (DMX_FTI_INTSTATUS_T *) u4Data);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					    TEXT("[PSR] %s line %d fail in PSR_HAL_")
					     TEXT
					     ("KeepHdrDetResult(u1PidIdx: %u), mrRet: 0x%x\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, u1PidIdx, mrRet);
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
				MM_RETURN(mrRet);
			}
		}
		DMXLOG_DEBUG(TEXT("[PSR] %s success, (Video) PicCnt: %d\r\n"),
			    DMX_FUNC_NAME,
			    ((DMX_FTI_INTSTATUS_T *) u4Data)->rHdrDectResult.u4PicInfoCount);
	} else {
		DMXLOG_DEBUG(TEXT("[PSR] %s success, (Audio|SP)\r\n"),
			    DMX_FUNC_NAME);
	}

	MM_RETURN(RET_DMX_OK);
}


MRESULT PSR_HAL_SetPowerState(DMX_PM_STATE ePowerState)
{
	u8 u1DevIdx = 0;
	
	PSR_HAL_LOCK;
	if (!DMX_HAL_SetPowerState(ePowerState, PVR_IN_PLAYBACK_MM)) {
		PSR_HAL_UNLOCK;
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	for (u1DevIdx = 0; u1DevIdx < DMX_DEV_CNT; u1DevIdx++){
		g_rPSRHalStruct[u1DevIdx].ePowerState = ePowerState;

		switch (ePowerState) {
		case D0:
		case D1:
		case D2:
			/* Power On */
			if (DMX_HW_STATE_OCCUPIED != g_rPSRHalStruct[u1DevIdx].eState)
				g_rPSRHalStruct[u1DevIdx].eState = DMX_HW_STATE_IDLE;
			break;
		case D3:
		case D4:
			/* Power Down */
			g_rPSRHalStruct[u1DevIdx].eState = DMX_HW_STATE_SUSPEND;
			break;
		default:
			break;
		}
	}
	PSR_HAL_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

DMX_PM_STATE PSR_HAL_GetPowerState(void)
{
	u8 u1DevIdx = 0;
	DMX_PM_STATE ePowerState = D0;

	PSR_HAL_LOCK;

	ePowerState = DMX_HAL_GetPowerState();

	for (u1DevIdx = 0; u1DevIdx < DMX_DEV_CNT; u1DevIdx++) {
		if (ePowerState != g_rPSRHalStruct[u1DevIdx].ePowerState)
			g_rPSRHalStruct[u1DevIdx].ePowerState = ePowerState;
	}

	switch (g_rPSRHalStruct[0].ePowerState) {
	case D0:
		DMXLOG_TRACE(TEXT("[DMX] %s -- CurPowerState: D0\r\n"), DMX_FUNC_NAME);
		break;
	case D1:
		DMXLOG_TRACE(TEXT("[DMX] %s -- CurPowerState: D1\r\n"), DMX_FUNC_NAME);
		break;
	case D2:
		DMXLOG_TRACE(TEXT("[DMX] %s -- CurPowerState: D2\r\n"), DMX_FUNC_NAME);
		break;
	case D3:
		DMXLOG_TRACE(TEXT("[DMX] %s -- CurPowerState: D3\r\n"), DMX_FUNC_NAME);
		break;
	case D4:
		DMXLOG_TRACE(TEXT("[DMX] %s -- CurPowerState: D4\r\n"), DMX_FUNC_NAME);
		break;
	default:
		DMXLOG_TRACE(TEXT("[DMX] %s -- CurPowerState: UNKNOWN\r\n"), DMX_FUNC_NAME);
		break;
	}

	ePowerState = g_rPSRHalStruct[0].ePowerState;

	PSR_HAL_UNLOCK;

	return ePowerState;
}

MRESULT PSR_HAL_DumpPidStructInfo(u8 u1Pidx)
{
	MRESULT mrRet = RET_DMX_OK;

	DMX_HAL_DumpPidStruct(u1Pidx);

	MM_RETURN(mrRet);
}

MRESULT PSR_HAL_DumpDDIInfo(void)
{
	MRESULT mrRet = RET_DMX_OK;

	DMX_HAL_DumpDDIInfo();

	MM_RETURN(mrRet);
}
