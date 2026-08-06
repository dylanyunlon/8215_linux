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
 * @file dmx_psr_filter.h
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_PSR_FILTER_H
#define DMX_PSR_FILTER_H

#include "x_typedef.h"
#include "drv_common.h"
#ifdef __linux__
#include <media/atc/drv_aud.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_decrypt.h>
#else
#include "drv_aud.h"
#include "dmx_splitter.h"
#include "dmx_decrypt.h"
#endif				/* __linux__ */

#include "dmx_def.h"
#include "dmx_psr_hal.h"
#include "dmx_spt_cfa.h"

#ifdef __cplusplus
}
#endif

#define AUD_AU_CNT_THRESHOLD                     4
#define AUD_FIFO_USAGE_RATE_THRESHOLD            10
#define AUD_AU_CNT_MAX_FOR_THRESHOLD             500
#define SP_AU_CNT_THRESHOLD                      60
#define SP_FIFO_USAGE_RATE_THRESHOLD             25

#define AVAILABLE_AUD_PTS_THRD_IF_RSP_SP        135000

/* Parser Filter Update Write Index Flag */
#define PSR_UPTWINX_IBC                         (1 << 0)
#define PSR_UPTWINX_VOBUSTILL                     (1 << 1)

#define V_FIFO_USAGE_MAX                         97
#define V_FIFO_USAGE_MIN                         70
#define V_FIFO_USAGE_MAX_AVI                     V_FIFO_USAGE_MAX
#define V_FIFO_USAGE_MIN_AVI                     V_FIFO_USAGE_MIN

#define PURE_AUDIO_A_FIFO_USAGE_MIN              50
#define PURE_AUDIO_A_FIFO_USAGE_MAX              90

/* WMV - VC1 */
#define PSR_HDR_WMV_FIELD                       0x1
#define PSR_HDR_WMV_FRAME                       0x2
#define PSR_HDR_WMV_EntryPoint                  0x4
#define PSR_HDR_WMV_SeqHdr                      0x8
#define PSR_HDR_WMV_SeqEnd                      0x1E	/* Manually Set Pattern [7:0] */
#define PSR_HDR_WMV_USRDAAT                     0x1B	/* Sync Dmx uP Header Detection */
#define PSR_HDR_WMV_SLICE                       0x1D	/* VC1 Slice Start Code */

/* MPEG2 */
#define PSR_HDR_MP2_IPic                         0x1
#define PSR_HDR_MP2_PPic                         0x2
#define PSR_HDR_MP2_BPic                         0x3
#define PSR_HDR_MP2_SeqHdr                       0x8
#define PSR_HDR_MP2_GOP                          0x9
#define PSR_HDR_MP2_SeqEnd                       0xA


/* MPEG4 */
#define PSR_HDR_MP4_IVOP                         0x0	/* /< vop_start_code                 000001B6 */
#define PSR_HDR_MP4_PVOP                         0x1	/* /< vop_start_code                 000001B6 */
#define PSR_HDR_MP4_BVOP                         0x2	/* /< vop_start_code                 000001B6 */
#define PSR_HDR_MP4_SVOP                         0x3	/* /< vop_start_code                 000001B6 */
#define PSR_HDR_MP4_VIDOBJ                       0x4	/* /< video_object_start_code,       000001[00~1f] */
#define PSR_HDR_MP4_VIDOBJLAY                    0x5	/* /< video_object_layer_start_code, 000001[20-2f] */
#define PSR_HDR_MP4_VISOBJSEQ                    0x6	/* /< visual_object_sequence_start_code, 000001B0 */
#define PSR_HDR_MP4_GOVOP                        0x9	/* /< group_of_vop_start_code,       000001B3 */
#define PSR_HDR_MP4_VISOBJ                       0xB	/* /< visual_object_start_code,      000001B5 */
#define PSR_HDR_MP4_SHIVOP                       0x18	/* /< short_video_start_marker */
#define PSR_HDR_MP4_SHPVOP                       0x19	/* /< short_video_start_marker */

/* AVC - H264 */
#define PSR_HDR_AVC_NONIDR                       0x1
#define PSR_HDR_AVC_IDR                          0x2
#define PSR_HDR_AVC_SEI                          0x4
#define PSR_HDR_AVC_SeqPar                       0x8
#define PSR_HDR_AVC_PicParam                     0x10
#define PSR_HDR_AVC_AUD                          0x11
#define PSR_HDR_AVC_FILTER                       0x12
#define PSR_HDR_AVC_STMEND                       0x14
#define PSR_HDR_AVC_SeqEnd                       0x1E	/* Manually Set Pattern [7:0] */
#define PSR_HDR_AVC_MVC                          0x1B	/* Sync Dmx uP Header Detection */

/* HEVC - H265 */
#define PSR_HDR_H265_VPS                         0x1
#define PSR_HDR_H265_SPS                         0x2
#define PSR_HDR_H265_PPS                         0x4
#define PSR_HDR_H265_AUD                         0x8
#define PSR_HDR_H265_EOS                         0x10
#define PSR_HDR_H265_EOB                         0x11
#define PSR_HDR_H265_PREFIX_SEI                  0x12
#define PSR_HDR_H265_RSVNVCL_41_44               0x14	/* Reserved NVCL 41~44 (Non-VCL) */
#define PSR_HDR_H265_UNSPEC_48_55                0x1E	/* UnSpec 48~55(Non-VCL) */
#define PSR_HDR_H265_VCL                         0x1B	/* VCL */

/* / Parser Filter flag */
/* Indicate whether the Parser Filter is in use or not */
#define FF_USED                                 (1 << 0)
#define FF_TX_PBBUF                             (1 << 1)	/* Indicate Tx source is pbbbuf */
#define FF_TX_TO_FIFO                           (1 << 2)	/* Indicate Data Needs to Tx to FIFO */
#define FF_ENABLE                               (1 << 3)	/* Indicate Parser Filter is Enable */
#define FF_LOCK                                 (1 << 4)

/* /SPT_DATA_BUF specific data */
typedef struct _PSR_DMA_SPECIFIC {
	uintptr_t ptrTxTgtMemSa;	/* /< Tx target memory start address */
	uintptr_t ptrPrivMemSa;	/* /< Tx target memory start address for sync command */
	u32 u4PrivMemSz;	/* /< Private memory size */
	bool fgHdrParsing;	/* /< Header parsing */
	uintptr_t *pptrTgtHdrPrsSa;	/* /< Hdr Parsing target memory start address */
	u32 *pu4AvailSz;	/* /< Available pbbuf size from target memory sa in current slot */
} PSR_DMASD;

/* / Extra CFA information */
typedef struct _EXT_INFO_T {
	u32 fgCreateAU:1;	/* [0], AUItem start notify by CFA */
	u32 fgAUEnd:1;	/* [1], AUItem end notify by CFA */
	u32 fgDST:1;		/* [2], only for SACD */
	u32 fg_reserved1:29;	/* [31:3] */
	u32 u4Vtype;		/* Divx311 Video Picture Type */
} EXT_INFO_T;

/* / DivxDRM Decryption Interface Information */
typedef struct _PSR_DivxDRMInfo {
	bool fgTurnOn;		/* /< [IN] TRUE: turn on, FALSE: turn off */
	u16 u2FrameKeyIdx;	/* /< [IN] Frame Key Index (DRM Chunk Info)           ///< AU Item Info */
	u64 u8DecryptStOfst;	/* /< [IN] Decrypt data start offset (DRM Chunk Info) */
	u32 u4DecryptLen;	/* /< [IN] Decryot data length (DRM Chunk Info) */
} PSR_DivxDRMInfo;

typedef struct _PSR_CC_DivxDRMInfo {
	PSR_DivxDRMInfo rPsrDivxDRM;
	u8 *pu1DivxDRMBuf;
	u32 u4DivxDRMBufSz;
} PSR_CC_DIVXDRM_MAN_T;

typedef struct _PSR_AUCtrlInfo {
	bool fgCreateAU;	/* /< [IN] Create a AUItem or not, */
	/* /< when call PSR_Filter_TxPBBuf2FifoWithAUCtrl, if this flag is TRUE,
	* it means we should set PSR_FILTER's fgAUCtrlByLen flag and u8TotalAULen
	* which means we only create AU when the tx len >= u8TotalAULen */
	/* /< when call PSR_Filter_TxPBBuf2FifoWithAUEnd, if this flag is TRUE,
	* it means we should set PSR_FILTER's fgAUCtrlByEnd flag which means we
	* only create AU when the EXT_INFO_T's fgAUEnd is TRUE */
	/* /< [IN] Total transfer Len for this AU, only used when fgCreateAU = TRUE. Unit: bytes. */
	u64 u8TotalAULen;
	u64 u8Len;		/* /< [IN] Transfer Len, Unit: bytes. */
	/* /< [IN] Video type, only for video filter, please reference drv_common.h */
	u32 u4Vtype;
	bool fgQueryWVC1Mode;	/* /< [IN] Query WVC1 Mode or not */

	bool fgUseCmdQ;
	bool fgAUByCmdQEnd;
	u16 u2TxEntryCnt;
	u64 u8RealTxLen;
	DMX_CMDQ_TX_ENTRY_T *parCmdQTxEntry;

#if ENABLE_DMX_ADVANCED_VER
	bool fgInsertHdr;
	u32 u4InsertHdrLen;
	u8 *pu1InsertHdrBuf;
#endif				/* ENABLE_DMX_ADVANCED_VER */
} PSR_AUCtrlInfo;

/* / Video filter specific data */
typedef struct _PSR_VF_SPECIFIC {
	VCodeC eVCodeC;		/* /< Video CodeC type */
	PSR_HDRDET_STATUS_T *prHALStatus;
	PSR_HDRDET_STATUS_T rKeepHALStatus;	/* /< HW status */
	PSR_HDRDET_RESULT_T rPicDetResult;
	u32 u4RealWrIdx;	/* /< Real write index of ESI */
	bool fgUseRealWrIdx;	/* /< Whether use real write index */
	u32 u4DummyAURealWrIdx;	/* /< Real write index of ESI */
	bool fgUseDummyAURealWrIdx;	/* /< Whether use real write index, initial value is FALSE */
	u32 u4VType;		/* /< Video type for Divx311/WMV123 */
	bool fgWMVInterlace;	/* /< WMV Interface Flag */
	u32 u4WMVSecondFieldPicType;	/* /< WMV Second Pic Type */
	bool fgQueryWVC1Mode;	/* /< Query WVC1 Mode or not */
	u8 uWVC1Mode;	/* /< WVC1 Mode for CFA Tx Control, 0=Normal Tx, 1=Add 0x0000010D before Tx */
	bool fgWVC1KeepHdrInfo;	/* /< Keep Prev Header Detection Result or not , initial value is FALSE */
	u32 u4PrevSCAddr;	/* /< Prev Start Code Addr (Should be 0x00 00 01 0D) */
	bool fgDummyTxWakeUp;	/* /< initial value is FALSE */
	bool fgDummyAUEnd;
	bool fgDummyCmdAU;

	/*sequence info of codec specical data */
	bool fgPrsSeqFrameInterpolation;
	bool fgPrsPreProcRange;
	u32 u4PrsNumBFrames;

	bool fgHasSetPics;
	bool fgCreNoHdrAUWaitPkt;
} PSR_VFSD;

/* / Other filter specific data except Video */
typedef struct _PSR_AUDF_SPECIFIC {
	AUD_DRV_FMT_T eAudFmtType;
	PSR_HDRDET_STATUS_T *prHALStatus;	/* /< HW status(Video Header Buffer Information, SA, EA,....) */
} PSR_AUDFSD;

/* / Other filter specific data except Video */
typedef struct _PSR_NORMALF_SPECIFIC {
	PSR_HDRDET_STATUS_T *prHALStatus;	/* /< HW status(Video Header Buffer Information, SA, EA,....) */
} PSR_NORMALFSD;

/* / Filter data structure, sizeof(PSR_FILER): 200 */
typedef struct _PSR_FILTER {
	/* /< Create a AU after transfer total AU
	* lengthmatch(e.g. Total TX Len(u8CurAULen)>=u8TotalAULen) */
	bool fgAUCtrlByLen;
	bool fgAUCtrlByEnd;	/* /< Create a AU after CFA notify it's an end of AU(e.g. fgAUEnd is TRUE) */
	bool fgAUEnd;		/* /< AU end notify, only used when fgAUCtrlByEnd is TRUE */
	bool fgFirstAUInRng;	/* Designate whether to search the first au in the range set by the CFA_SET_RANGE, */
	u8 ucHwDevId;	/* /< Used HW Resource */

	u32 u4ESIH;		/* /< elementary stream interface handle */
	u32 u4GAU;
	uintptr_t ptrESFifoSa;	/* /< elementary stream fifo sa */
	uintptr_t ptrESFifoEa;	/* /< elementary stream fifo ez */
	u32 u4ESFifoSize;	/* /< elementary stream fifo size */
	u32 u4StmType;	/* /< Filter component type */
	u32 u4StmUID;	/* /< Filter component ID */
	u32 u4Flag;		/* /< Filter flag */
	uintptr_t ptrBkWrPtr;	/* /< backup write pointer for subpicture, navigator and video DIVX311 filter */
	uintptr_t ptrSacdAuSa;	/* /< SACD AU start address in fifo before jump 5 bytes */
	uintptr_t ptrHdrBufAddr;
	uintptr_t ptrPsrHwCurWPtr;	/* /< For SACD Flow, Keep Current Write Ptr. */
	u32 u4AUExtCnt;	/* /< AU full extended count */
	u32 u4AUCntFromIFrm;
	u32 u4IFrmCnt;

	E_SPT_DATA_TYPE_T eType;	/* /< filter type */
	void *pvFilterSpecific;

	/*! @name Filter User (Mainly for Session handler) Related Information */
	void *pvCBParam;	/* /< for user private data, this is structure pointer */
	void *pvPsrCC;		/* /< CC handle */
	void *pvDmxInst;

	u64 u8DecSendBufMask;	/* /< SW Dec Mask */
	u64 u8TxCurrOffset;	/* /< Tx current offset */
	u64 u8TotalAULen;	/* /< Total AU length need transfer for AU create control */
	u64 u8CurAULen;	/* /< Current transfer length for AU create control */
	u64 u8WMDRMTxLen;	/* /< WMDRM TX Length */
	u8 bSampleID[8];	/* /< Test Only */
	u64 u8LastPTS;	/* /< PTS info for AU */
	u64 u8HdrPTS;	/* /< PTS info for AU */

#ifdef MM_SUPPORT_DIVXHT31
	u64 u81stPTS;	/* /< PTS info for AU */
	u64 u8PrevPTS;	/* /< PTS info for AU */
#endif				/* MM_SUPPORT_DIVXHT31 */
} PSR_FILTER;

MRESULT PSR_Filter_Reset(PSR_FILTER *prPsrFtr);

MRESULT PSR_Filter_MoveFifoWp2AUIdx(PSR_FILTER *prPsrFtr, u32 u4AUIdx);

MRESULT PSR_Filter_Create(void *pvDmxInst, u32 u4PsrFtrType, u32 u4PsrFtrId,
			  void **ppvHandle);

MRESULT PSR_Filter_Destroy(PSR_FILTER *pPsrFtr);

MRESULT PSR_Filter_TxToGround(PSR_FILTER *pPsrFtr);

MRESULT PSR_Filter_TXDecide(PSR_FILTER *prPsrFtr);

MRESULT PSR_Filter_Tx4HdrParsing(PSR_FILTER *pPsrFtr);

MRESULT PSR_Filter_TxPbbuf(PSR_FILTER *pPsrFtr, u64 u8Offset);

MRESULT PSR_Filter_TxMem(PSR_FILTER *pPsrFtr);

MRESULT PSR_Filter_SetVDummyInfo(PSR_FILTER *prPsrFtr, bool fgDummyEnd, bool fgDummyCmdAU);

MRESULT PSR_Filter_AddDummyAU(PSR_FILTER *pPsrFtr, bool fgDummyAUEnd, bool fgDummyCmdAU);

MRESULT PSR_Filter_TX2Fifo(PSR_FILTER *pPsrFtr, bool fgChkFifo);


MRESULT PSR_Filter_TriggerHALGTx(PSR_FILTER *pPsrFtr);

MRESULT PSR_Filter_TriggerHALPTx(PSR_FILTER *pPsrFtr);

MRESULT PSR_Filter_IRQ_Proc(PSR_FILTER *pPsrFtr);

MRESULT PSR_Filter_SetVCodeC(PSR_FILTER *pPsrFtr);

MRESULT PSR_Filter_Enable(PSR_FILTER *pPsrFtr);

MRESULT PSR_Filter_Disable(PSR_FILTER *pPsrFtr);

MRESULT PSR_Filter_SetStreamInfo(PSR_FILTER *pPsrFtr, u32 u4StreamUID);

MRESULT PSR_Filter_SetType(PSR_FILTER *pPsrFtr, E_SPT_DATA_TYPE_T eType, u64 u8DecSendBufMask);

MRESULT PSR_Filter_Flush(PSR_FILTER *pPsrFtr);

MRESULT PSR_Filter_GetCodeC(PSR_FILTER *prPsrFtr, u32 *pu4Codec);

MRESULT PSR_Filter_SetCodeC(PSR_FILTER *pPsrFtr, u32 u4CodeC);

MRESULT PSR_Filter_SwitchFifo(PSR_FILTER *prPsrFtr);

MRESULT PSR_Filter_DMAPBBuf4HdrParsing(PSR_FILTER *pPsrFtr, u64 u8Offset,
				       u32 u4Len, uintptr_t *pptrTgtSa, u32 *pu4AvailSz);

MRESULT PSR_Filter_TxMemory2Fifo(PSR_FILTER *pPsrFtr,
	void *pvSrcSa, u32 u4Len);

MRESULT PSR_Filter_TxMemory2FifoWithAUCtrl(PSR_FILTER *pPsrFtr, 
	void *pvSrcSa, PSR_AUCtrlInfo *prAUCtrlInfo);

MRESULT PSR_Filter_IsFifoFull(PSR_FILTER *pPsrFtr, bool *pfgFull, u64 u8TxLen);

MRESULT PSR_Filter_TxPBBuf2FifoWithAUEnd(PSR_FILTER *pPsrFtr, u64 u8Offset,
					 u64 u8Len, EXT_INFO_T *prExtInf);

MRESULT PSR_Filter_TxPBBuf2FifoWithAUCtrl(PSR_FILTER *pPsrFtr, u64 u8Offset,
					  PSR_AUCtrlInfo *prAUCtrlInfo);


#ifdef __cplusplus
}
#endif

#endif				/* #ifndef _PSR_FILTER_H_ */
