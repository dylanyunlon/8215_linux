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
 */

/*!
 * @file dmx_spt_cfa.h
 *
 * @par Project
 *
 * @par Description
 *    Demuxer Splitter Interfaces, structures For CFA declarations
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INTERNAL_SPT_CFA_H
#define DMX_INTERNAL_SPT_CFA_H

/*! @name Splitter Interface For CFA */

/*! @name Splitter Interface For CFA Include Header File (6.0) */
/*! @{ */

#include "x_typedef.h"
#include "drv_common.h"
#include <media/atc/x_vid_dec.h>
#include "cfa_if.h"

#ifdef __linux__
#if CONFIG_DRV_HDMI_RX
#include <media/atc/x_audin.h>
#endif				/* CONFIG_DRV_HDMI_RX */
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_decrypt.h>
#else				/* __linux__ */
#if CONFIG_DRV_HDMI_RX
#include "x_audin.h"
#endif				/* CONFIG_DRV_HDMI_RX */
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "dmx_decrypt.h"
#endif				/* __linux__ */

#include "dmx_def.h"

/*! @} */

#ifdef __cplusplus
extern "C" {

#endif

/*! @name Splitter Interface For CFA Constants, enumerations and macros (6.1) */
/*! @{ */

/* / CFA Stream Type */
typedef enum {
	CFA_BIT_V = 1,	/* /< Video stream type */
	CFA_BIT_A = 2,	/* /< Audio stream type */
	CFA_BIT_SP = 3,	/* /< Sub Picture type stream type */
	CFA_BIT_ST = 4,	/* /< Subtitle stream type */
	CFA_BIT_NV = 5	/* /< NV stream type */
} CfaApiBitType;

typedef enum {
	ES_IN_NONE = 0, ES_IN_DIAG = 1, ES_IN_BROADCAST_TS = 0x10,
	ES_IN_PLAYBACK_TS = 0x100, ES_IN_PLAYBACK_PS = 0x200,
	ES_IN_PLAYBACK_MM = 0x300, ES_IN_PVR_TS = 0x400,
	ES_IN_PLAYBACK_ES = 0x500
} ES_INPUT_TYPE_T;

/* / CFA Audio Codec Type */
typedef enum {
	CFA_AUD_DRV_FMT_UNKNOWN = 0, CFA_AUD_DRV_FMT_MPEG, CFA_AUD_DRV_FMT_AC3,
	CFA_AUD_DRV_FMT_PCM, CFA_AUD_DRV_FMT_MP3, CFA_AUD_DRV_FMT_AAC,
	CFA_AUD_DRV_FMT_DTS, CFA_AUD_DRV_FMT_WMA, CFA_AUD_DRV_FMT_RA,
	CFA_AUD_DRV_FMT_HDCD, CFA_AUD_DRV_FMT_MLP, CFA_AUD_DRV_FMT_MTS,	/* remove */
	CFA_AUD_DRV_FMT_EU_CANAL_PLUS, CFA_AUD_DRV_FMT_TV_SYS, CFA_AUD_DRV_FMT_EAC3,
	CFA_AUD_DRV_FMT_EAC3_SEC, CFA_AUD_DRV_FMT_DTSHD_PRI_XLL,
	CFA_AUD_DRV_FMT_DTSHD_PRI_NO_XLL, CFA_AUD_DRV_FMT_DTSHD_SEC,
	CFA_AUD_DRV_FMT_DTSCD, CFA_AUD_DRV_FMT_TRUE_HD, CFA_AUD_DRV_FMT_CDDA,
	CFA_AUD_DRV_FMT_SACD,	/* DSD */
	CFA_AUD_DRV_FMT_VORBIS, CFA_AUD_DRV_FMT_DST, CFA_AUD_DRV_FMT_COOK,
	/*Real Play audio,cook */
	CFA_AUD_DRV_FMT_FLAC, /* FLAC added by Mingxu Wang @2011/2/26 */
	CFA_AUD_DRV_FMT_APE /* APE added by mtk68014 @2014/07/26 */
} CfaApiAudType;

/* / CFA Video Picture Type */
typedef enum {
			/*!
		     * @brief cfa picture type no picture
		     *
		     * The data in this transfer includes no picture.
		     * This mode is used when CFA knows it's transferring the latter part of a DivX-3 picture.
		     * The eVidType parameter is ignored if eTxMode is set to this value.
		     *
		     * @see CfaApiVidType
		     */
	CFA_PTM_NO_PIC,
		    /*!
		     * @brief cfa picture type DivX3 I-Frame
		     *
		     * The data in this transfer includes one and only one DivX-3 I-picture,
		     * which starts from the 1st byte of this transfer.
		     * The eVidType parameter is ignored if eTxMode is set to this value.
		     *
		     * @see CfaApiVidType
		     */
	CFA_PTM_ONE_PIC_DX3_I,
		    /*!
		     * @brief cfa picture type DivX3 P-Frame
		     *
		     * The data in this transfer includes one and only one DivX-3 P-picture,
		     * which starts from the 1st byte of this transfer.
		     * The eVidType parameter is ignored if eTxMode is set to this value.
		     *
		     * @see CfaApiVidType
		     */
	 CFA_PTM_ONE_PIC_DX3_P,
		    /*!
		     * @brief cfa picture exact position (normal case)
		     *
		     * The data in this transfer may include one or more pictures,
		     * and the "position" field attached with each picture should be set as
		     * the exact offset of the picture starting byte.
		     * CFA must specify what kind of pictures is in this transfer by specifying eVidType parameter.
		     *
		     * @see CfaApiVidType
		     */
	CFA_PTM_EXACT_POS,
		    /*!
		     * @brief cfa picture same position
		     *
		     * The data in this transfer may include one or more pictures,
		     * and the "position" field attached with each picture should be the same,
		     * and is set as the "u8FileOfst" of this transfer.
		     * CFA must specify what kind of pictures is in this transfer by specifying eVidType parameter.
		     *
		     * @see CfaApiVidType
		     */
	CFA_PTM_SAME_POS,
		    /*!
		     * @brief cfa picture
		     *
		     * The data in this transfer may dummy frame, mainly for DivX311
		     *
		     * @see CfaApiVidType
		     */
	CFA_PTM_DUMMY,
		    /*!
		     * @brief cfa picture type WMV1/2/3 I-Frame
		     *
		     * The data in this transfer includes one and only one WMV1/2/3 I-picture,
		     * which starts from the 1st byte of this transfer.
		     * The eVidType parameter is ignored if eTxMode is set to this value.
		     *
		     * @see CfaApiVidType
		     */
	CFA_PTM_WMV_I,
		    /*!
		     * @brief cfa picture type WMV1/2/3 P-Frame
		     *
		     * The data in this transfer includes one and only one WMV1/2/3 P-picture,
		     * which starts from the 1st byte of this transfer.
		     * The eVidType parameter is ignored if eTxMode is set to this value.
		     *
		     * @see CfaApiVidType
		     */
	CFA_PTM_WMV_P,
		    /*!
		     * @brief cfa picture type WMV1/2/3 B-Frame
		     *
		     * The data in this transfer includes one and only one WMV1/2/3 B-picture,
		     * which starts from the 1st byte of this transfer.
		     * The eVidType parameter is ignored if eTxMode is set to this value.
		     *
		     * @see CfaApiVidType
		     */
	CFA_PTM_WMV_B,
		    /*!
		     * @brief cfa picture type WMV1/2/3 Sequence Header
		     *
		     * The data in this transfer includes one and only one WMV1/2/3 Sequence Header,
		     * which starts from the 1st byte of this transfer.
		     * The eVidType parameter is ignored if eTxMode is set to this value.
		     *
		     * @see CfaApiVidType
		     */
	CFA_PTM_WMV_SEQHDR,
		    /*!
		     * @brief cfa picture type WMV1/2/3 Skip frame
		     *
		     * The data in this transfer includes one and only one WMV1/2/3 Skip frame,
		     * which starts from the 1st byte of this transfer.
		     * The eVidType parameter is ignored if eTxMode is set to this value.
		     *
		     * @see CfaApiVidType
		     */
	CFA_PTM_WMV_SKIPFRAME
		    /*!
		     * @brief cfa picture type H263 Sorenson I-Frame
		     *
		     * The data in this transfer includes one and only one H263 Sorenson I-picture,
		     * which starts from the 1st byte of this transfer.
		     * The eVidType parameter is ignored if eTxMode is set to this value.
		     *
		     * @see CfaApiVidType
		     */
	, CFA_PTM_H263_SORENSON_I,
		    /*!
		     * @brief cfa picture type H263 Sorenson P-Frame
		     *
		     * The data in this transfer includes one and only one H263 Sorenson P-picture,
		     * which starts from the 1st byte of this transfer.
		     * The eVidType parameter is ignored if eTxMode is set to this value.
		     *
		     * @see CfaApiVidType
		     */
	CFA_PTM_H263_SORENSON_P

	, CFA_PTM_RM_INTRAPIC
	, CFA_PTM_RM_FORCED_INTRAPIC
	, CFA_PTM_RM_INTERPIC
	, CFA_PTM_RM_TRUEBPIC
	, CFA_PTM_MJPEG_I
	, CFA_PTM_ONE_PIC_VP6_I
	, CFA_PTM_ONE_PIC_VP6_P
	, CFA_PTM_ONE_PIC_VP8_I
	, CFA_PTM_ONE_PIC_VP8_P
} CfaApiPicTxMode;

/* / CFA Video Codec Type */
typedef enum {
	CFA_VID_UNKNOWN = VC_UNKNOW,	/* /< Unkown video codec */
	CFA_VID_MPEG2 = VC_MPEG2,	/* /< MPEG1 or MPEG2 video codec */
	CFA_VID_DIVX3 = VC_DIVX3,	/* /< DivX 311 video codec */
	CFA_VID_DIVX4 = VC_DIVX4,	/* /< DivX 4 video codec */
	CFA_VID_DIVX6 = VC_DIVX6,	/* /< DivX 5/6 video codec */
	CFA_VID_MPEG4 = VC_MPEG4,	/* /< MPEG4 video codec */
	CFA_VID_H263 = VC_H263,	/* /< H.263 video codec */
	CFA_VID_H263_SORENSON = VC_H263_SORENSON,	/* /< H.263 Sorenson video codec */
	CFA_VID_H264 = VC_H264,	/* /< H.264 video codec */
	CFA_VID_WMV7 = VC_WMV1,	/* /< MS wmv 7 video codec */
	CFA_VID_WMV8 = VC_WMV2,	/* /< MS wmv 8 video codec */
	CFA_VID_WMV9 = VC_WMV3,	/* /< MS wmv 9 video codec */
	CFA_VID_VC1 = VC_VC1,	/* /< MS wmv vc-1 video codec */
	CFA_VID_RV30 = VC_RV30,
	CFA_VID_RV40 = VC_RV40,
	CFA_VID_MJPEG = VC_MJPEG,	/* /< MJPEG video codec */
	CFA_VID_VP6 = VC_VP6,	/* /< VP6 video codec */
	CFA_VID_VP6A = VC_VP6A,	/* /< VP6 with alpha channel video codec */
	CFA_VID_VP8 = VC_VP8,	/* /< VP8 video codec */
	CFA_VID_H265 = VC_H265,	/* /< H265 video codec */
	CFA_VID_MAX
} CfaApiVidType;

typedef struct {
	/**
	 * One AU will be composed when encounter that the fgEndAU is TRUE.
	 * e.g.          cmdEntry0, CmdEntry1, CmdEntry2, CmdEntry3, CmdEntry4, CmdEntry5
	 *      fgEnd:      FALSE        TRUE           FALSE          FALSE          TRUE        FALSE
	 *                ----------AU-------|--------------AU--------------|---------|**/
	 bool fgEndAU;

#if ENABLE_DMX_ADVANCED_VER
	
	/* Indicate whether to insert header data into fifo before the cmd entry dma by hw */
	bool fgInsertHdr;
	u8 au1InsertHdr[DMX_MAX_INST_BYTES_CNT];
	u32 u4InsertHdrLen;
	CfaApiPicTxMode eTxMode;	/* /< [IN] CFA Video Picture Type */
#endif				/* ENABLE_DMX_ADVANCED_VER */
	u32 u4TxOfst;
	u32 u4TxLen;
} DMX_CMDQ_TX_ENTRY_T;

/*!* @brief SACD type info structure**/

typedef struct _cfa_sacd_info {
	bool fgDST;
	/* notify Parser if this SACD is DST type */
} CFA_SACD_INFO_T;

typedef union _exinf_a {
	CFA_SACD_INFO_T rSACD;	/* /< Sacd type information */
} EXINF_A;

/*!
 * @brief Audio type stream structure
 *
 */
typedef struct {
	bool fgDummyAU; /* /< [IN] Indicate it's a dummy AU */
	bool fgUnitStart;	/* /< [IN] Transfer is an unit start */
	bool fgAUCompleteByEnd; /* /< [IN] AU complete by CFA notify AU end, not by count Total AU Length. */
	bool fgUnitEnd; /* /< [IN] Transfer is an unit end */
	bool fgUseCmdQ;
	bool fgAUByCmdQEnd;
	EXINF_A rExInf;	/* /< [IN] Extra information of certain audio type */
	
	u16 u2TxEntryCnt;
	
#if ENABLE_DMX_ADVANCED_VER
	bool fgInsertHdr;
	u32 u4InsertHdrLen;
	u8 *pu1InsertHdrBuf;
#endif	/* ENABLE_DMX_ADVANCED_VER */

	uintptr_t ptrFromFileAddress;
	u32 u4PrsStrmId;	/* /< [IN] Current parse stream id for RSP. */
	CfaApiAudType eAudType;	/* /< [IN] Audio type */
	
	u64 u8TotalAULen;	/* /< [IN] Total Data Length of one AU */
	u64 u8FileOfst;	/* /< [IN] Transfer start offset (u8) */
	u64 u8Len;	/* /< [IN] Transfer length (u8) */	
	u64 u8Pts;	/* /< [IN] Transfer Data's start PTS (STC) */
	u64 u8RealTxLen;	/* Can calculate from all entry */

	DMX_CMDQ_TX_ENTRY_T *parCmdQTxEntry;

} CFA_AUDIO_INFO_T;

/*!
 * @brief Sub Picture type stream structure
 *
 */

typedef struct{
	u64 u8FileOfst;	/* /< [IN] Transfer start offset (u8) */
	u64 u8Len;	/* /< [IN] Transfer length (u8) */
	u32 u4PrsStrmId;	/* /< [IN] Current parse stream id for RSP. */
	bool fgDummyAU;	/* /< [IN] Indicate it's a dummy AU */
	bool fgUnitStart;	/* /< [IN] Transfer is an unit start */
	u64 u8Pts;	/* /< [IN] Transfer Data's start PTS (STC) */
	u64 u8EndPts;	/* /< [IN] Transfer Data's end PTS (STC) */
	u32 u4SpuPos;	/* /< [IN] Transfer start offset (u8), TODO: Remove */
} CFA_SUBPIC_INFO_T;

/*!
 * @brief CPS (CSS/CPRM) structure
 */

typedef struct{
	bool fgOn;	/* /< [IN] Flag of on/off */
	u32 u4CpsType;	/* /< [IN]refer to CfaCpsType */
	u64 u8Offset;	/* /< [IN] Decrypt data start offset */
	u64 u8DCI_CCI;	/* /< [IN] DCI_CCI (Only in CPRM) */
	u64 u8DCI_CCI_VERIFY;	/* /< [IN] DCI_CCI verify data h (Only in CPRM) */
	u64 u8DecLen;	/* /< [IN] Decrypt data length (For SACD) */
	u32 u4MaxPacketSize;	/* /<[IN] Max Packet Size (For WMDRM PD) */
	u32 u4MediaObjSize;	/* /<[IN] Media Obj Size (For WMDRM ND) */
	u8 bSampleID[8];	/* /<[IN] Sample ID (For WMDRM ND) */
	u8 bIVData[16];	/* /< [IN] IV Data (Only for Marlin MP4) */
	u32 u4IPMPDescriptorID;	/* /< [IN] IPMP_DescriptorID (Only for Marlin MP4) */
	u32 u4RealSampleSize;	/* /< [OUT] Real Sample Size (Only for Marlin MP4) */
	u64 *pu8DTKC;
} CFA_CPS_INFO_T;

/*!
 * @brief Audio type stream structure
 *
 */

typedef struct{
	u64 u8FileOfst;	/* /< [IN] Transfer start offset (u8) */
	u64 u8Len;	/* /< [IN] Transfer length (u8) */
	u32 u4PrsStrmId;	/* /< [IN] Current parse stream id for RSP. */
	CfaApiPicTxMode eTxMode;	/* /< [IN] CFA Video Picture Type */
	CfaApiVidType eVidType;	/* /< [IN] CFA Video Codec Type */
	u64 u8TotalAULen;	/* /< [IN] Total Data Length of one AU */
	bool fgQueryWVC1Mode;	/* /< [IN] Query WVC1 Mode or not */
	bool fgDummyAU;	/* /< [IN] Indicate it's a dummy AU */
	bool fgUnitStart;	/* /< [IN] Transfer is an unit start */

	/* /< [IN] AU complete by CFA notify AU end, not by count Total AU Length. */
	bool fgAUCompleteByEnd;
	bool fgUnitEnd;	/* /< [IN] Transfer is an unit end */
	bool fgDummyAUEnd;
	bool fgDummyCmdAU;
	bool fgUseCmdQ;
	u16 u2TxEntryCnt;
	u64 u8RealTxLen;	/* Can calculate from all entry */
	DMX_CMDQ_TX_ENTRY_T *parCmdQTxEntry;

#if ENABLE_DMX_ADVANCED_VER
	bool fgAUByCmdQEnd;
 /* Indicate whether to insert header data into fifo before dma by hw while not using CmdQ dma */
	bool fgInsertHdr;

/* the buffer contains the header data to insert */
	u8 *pu1InsertHdrBuf;

/* the length of the header data to insert */
	u32 u4InsertHdrLen;

#endif				/* ENABLE_DMX_ADVANCED_VER */

#if CONFIG_DRV_SUPPORT_RM_VID_DYNC_MEM
	u16 u2RmCurAuSliceNum;

#endif
} CFA_VIDEO_INFO_T;

/*!
 * @brief section type stream structure
 *
 */

typedef struct{
	u64 u8FileOfst;	/* /< [IN] Transfer start offset (u8) */
	u64 u8Len;	/* /< [IN] Transfer length (u8) */
	u32 u4PrsStrmId;	/* /< [IN] Current parse stream id for RSP. */
	bool fgDummyAU;	/* /< [IN] Indicate it's a dummy AU */
	bool fgUnitStart;	/* /< [IN] Transfer is an unit start */
	u64 u8TotalAULen;	/* /< [IN] Total Data Length of one AU */
	bool fgAUCompleteByEnd;	/* /< [IN] AU complete by CFA notify AU end, not by count Total AU Length. */
	bool fgUnitEnd;	/* /< [IN] Transfer is an unit end */

	/* MPG */
	u32 u4PackCnt;
	bool fgUseCmdQ;
	u16 u2TxEntryCnt;
	u64 u8RealTxLen;	/* Can calculate from all entry */
	DMX_CMDQ_TX_ENTRY_T *parCmdQTxEntry;
} CFA_SECTION_INFO_T;

/*! @} */

/*! @name Splitter Interface For CFA Structures (6.2) */
/*! @{ */
/* / PBBuf Status Information */
typedef struct {
	u32 u4ValideDataSize;	/* /<  Valid Total Data Size in PBBUF */
	u32 u4TotalBufSize;	/* /<  Total PBBUF Size */
} CFA_PBBUFInfo;

/** Attributes for the cps common info.
 *  please refer the IBC_CpsCommonInfoParamsDef in x_drv_comm.h
 *  use the definition for u4InfoValid and CGMS IBC
 */

typedef struct {
	u32 u4InfoValid;
	u8 u1Cgms;
	u8 u1Aps;
	u8 u1Epn;
	u8 u1AnalogSrc;
	u64 u8DCI_CCI;	/* /< [IN] DCI_CCI (Only in CPRM) */
	u64 u8DCI_CCI_VERIFY;	/* /< [IN] DCI_CCI verify data h (Only in CPRM) */
	u16 u2DiscCpsType;
} CFA_CPS_INFO_PARAMS;

/* / CFA Parser Finished type */
typedef enum {
	CFA_PAR_FIN_TYPE_NORMAL,	/* /< normal parser end */
	/* /< for DVD AUDIO m-seconds-parser-n-seconds-skip range parser end */
	CFA_PAR_FIN_TYPE_DVDA_M_PARSER_N_SKIP,
	CFA_PAR_FIN_TYPE_DVDA_M_PARSER	/* /< for DVD AUDIO m-seconds-parser and end */
} CfaParFinType;

/*!
 * @brief CFA send pause stream information
 *
 * It is for CFA to send pause stream information to parser.
 *
 */
typedef struct {
	u32 u4AstrmNs;	/* < decoding audio stream Ns */
	u32 u4VstrmNs;	/* < decoding video stream Ns */
	u16 u2DecAudStId[8];	/* < decoding audio stream id ((MPEG stream id<<8) | MPEG sub-stream id)) */
	u8 ucDecVidStId[2];	/* < decoding vid stream id */
	bool fgVobuStill;	/* < indicate it's vobu still pause */
} Cfa2PsrStrmInfo;

/**
 * @brief cfa parser finish  structure
 */
typedef struct {
	u64 u8Ea;	/* /< [IN] CFA range end address */
	CfaParFinType eCfaParFinType;	/* /< [IN] CFA parser finish type */
} CFA_FINISH_INFO_PARAMS;

/*!
 * @brief Divx DRM structure
 *
 */
typedef struct {
	bool fgOn;	/* /< [IN] Flag of on/off */
	u16 u2FrameKeyIdx;	/* /< [IN] Frame Key Index (DRM Chunk Info) */
	u64 u8DecryptStOfst;	/* /< [IN] Decrypt data start offset (DRM Chunk Info) */
	u32 u4DecryptLen;	/* /< [IN] Decrypt data length (DRM Chunk Info) */
} CFA_DIVXDRM_INFO_T;

/*!
 * @brief parse end call back structure
 *
 * It is mapping to DVR FUN_PTR_FDMX_PRS_END_CB parameter.
 * We use this for pvEventData structure when the u4EventNo is SPLITTER_EVENTS_CFA_END.
 *
 * @see SPLITTER_EVENTS_CFA_END
 */
typedef struct {
/* / Nothing Inside */
	void *pvNull;
} SPLITTER4CFA_EVENTS_END_INTO_T;

/*!
 * @brief inquiry call back structure
 *
 * It is mapping to DVR FUN_PTR_FDMX_INQ_NOTIFY parameter.
 * We use this for pvEventData structure when the u4EventNo is SPLITTER_EVENTS_CFA_INQUIRY.
 *
 * @see SPLITTER_EVENTS_CFA_INQUIRY
 */
typedef struct {
	u32 u4InfType;	/* /< inquiry id */
	void *pvInqInf;
} SPLITTER4CFA_EVENTS_INQUIRY_INTO_T;

/*!
 * @brief demxuxer normal call back structure
 *
 * It is mapping to DVR SPLITTER_EVENTS_DMX_NORMAL parameter.
 * We use this for pvEventData structure when the u4EventNo is SPLITTER_EVENTS_CFA_NORMAL.
 *
 * @see SPLITTER_EVENTS_CFA_NORMAL
 */
typedef struct {
	u32 u4CbEvt;	/* /< cfa event id */
	void *pvCbPar;
} SPLITTER4CFA_EVENTS_NORMAL_INTO_T;

/*! @} */

/* Follow function call from splitter. */
void *SptGetCfaInterface(u32 u4CfaType);
MRESULT SptCfaInit(void *pvSptHdl);
MRESULT SptCfaUninit(void *pvSptHdl);
MRESULT SptCfaSetInquirer(void *pvSptHdl, u32 u4InquirerTypes);
MRESULT SptCfaConfigure(void *pvSptHdl, const void *pvCfaParameter, bool fgIsUserMem);
MRESULT SptCfaSetRange(void *pvSptHdl, const void *pvCfaRange, bool fgIsUserMem);
MRESULT SptCfaGetPosition(void *pvSptHdl, const void *pvCfaPosition);
MRESULT SptCfaGetGeneral(void *pvSptHdl, u32 u4CfaFID, const void *pvCfaParameter,
	u32 u4CfaParameterSize);
MRESULT SptCfaSetGeneral(void *pvSptHdl, u32 u4CfaFID, const void *pvCfaParameter,
	u32 u4CfaParameterSize);
MRESULT SptCfaEnableStream(void *pvSptHdl, u32 u4CFAStmType, bool fgEnable);
MRESULT SptCfaSetTxDone(void *pvSptHdl, u64 u8TotalTxLen, bool fgRsp);
MRESULT SptCfaSetTurnOn(void *pvSptHdl);
MRESULT SptCfaSetJumpInfo(void *pvSptHdl, const void *pvJumpInfo);
MRESULT SptCfaGetParam(void *pvSptHdl, u32 u4ParamID, const void *pvCfaParameter,
	u32 u4CfaParameterSize);
MRESULT SptCfaSetAUInfo(void *pvSptHdl, const void *pvAUInf, const void *pvAUExtInf);
MRESULT SptCfaSetStmInfo(void *pvSptHdl, u32 u4StreamToSet, u32 u4StmUID);
MRESULT SptCfaProcCliCmd(void *pvSptHdl, E_DMX_CFA_CLI_TYPE_T eCliType,	/* /< [IN] Cfa Cli Command */
	u32 arg1, u32 arg2, u32 arg3, const char *szParam);
MRESULT Spt4CfaPbb2SyncBuf(void *pvSptHdl, u64 u8FileOfst, u64 u8Len, u8 *pucBuf);
MRESULT Spt4CfaPbb2SyncBufEx(void *pvSptHdl, u64 u8FileOfst, u64 u8Len,
	u8 *pucBuf, u32 *pu4AvailSize);
MRESULT Spt4CfaBuf2VFifo(void *pvSptHdl, u8 *pucSrc, u64 u8Position,
	CfaApiPicTxMode eTxMode, CfaApiVidType eVidType, u64 u8Len);
MRESULT Spt4CfaBuf2VFifoAUCtrl(void *pvSptHdl, u8 *pucSrc, CFA_VIDEO_INFO_T *prVidInf);
MRESULT Spt4CfaBuf2AFifo(void *pvSptHdl, u8 *pucSrc, u64 u8Len, u32 u4TxUID,
	CfaApiAudType eAudType);
MRESULT Spt4CfaBuf2AFifoAUCtrl(void *pvSptHdl, u8 *pucSrc, CFA_AUDIO_INFO_T *prAudInf,
	u64 u8TotalAULen);
MRESULT Spt4CfaPbb2VFifoAUCtrl(void *pvSptHdl, CFA_VIDEO_INFO_T *prVidInf);
MRESULT Spt4CfaPbb2AFifoAUCtrl(void *pvSptHdl, CFA_AUDIO_INFO_T *prAudInf);
MRESULT Spt4CfaPbb2SpFifoAUCtrl(void *pvSptHdl, CFA_SUBPIC_INFO_T *prSubPicInf,
	u64 u8TotalAULen);
MRESULT Spt4CfaBuf2SectionFifo(void *pvSptHdl, u8 *pucSrc, u64 u8Position,
	u64 u8Len, u32 u4TxUID);
MRESULT Spt4CfaBuf2SectionFifoAUCtrl(void *pvSptHdl, u8 *pucSrc,
	CFA_SECTION_INFO_T *prSectionInf, u64 u8TotalAULen);
MRESULT Spt4CfaBuf2SpFifo(void *pvSptHdl, u8 *pucSrc, u64 u8Position,
	u64 u8Len, u32 u4TxUID);
MRESULT Spt4CfaBuf2SpFifoAUCtrl(void *pvSptHdl, u8 *pucSrc,
	CFA_SUBPIC_INFO_T *prSubPicInf, u64 u8TotalAULen);
MRESULT Spt4CfaPbb2SectionFifoAUCtrl(void *pvSptHdl, CFA_SECTION_INFO_T *prSectionInf);
MRESULT Spt4CfaPbb2AFifo(void *pvSptHdl, u64 u8FileOfst, u64 u8Pts, u64 u8Len,
	CfaApiAudType eAudType);
MRESULT Spt4CfaFinishedEx(void *pvSptHdl, u64 u8Offset, bool fgCompulsory,
	u32 u4Status);
MRESULT Spt4CfaNotiCurStrmInf(void *pvSptHdl, bool fgHighBitrate);
MRESULT Spt4CfaGetWMVParsingMode(void *pvSptHdl, u8 *pucWMVParsingMode);
bool Spt4CfaSetESInputType(void *pvSptHdl, ES_INPUT_TYPE_T eESType);

MRESULT Spt4CfaClearAllStmData(void *pvSptHdl);
MRESULT Spt4CfaTriggleAUCmdRelease(void *pvSptHdl, E_SPT_DATA_TYPE_T eDataType);

MRESULT Spt4CfaClearAllGAUEvents(void *pvSptHdl);
bool Spt4CfaIsNonHdrDectVCodec(CfaApiVidType eVidType);

/* ///////////////////////////////////////////////////////////////////////////// */
/* The following should be removed later */
/* ///////////////////////////////////////////////////////////////////////////// */
MRESULT Spt4CfaSubPicFound(void *pvSptHdl, u8 ucSpStId);
MRESULT Spt4CfaAutoPause(void *pvSptHdl, void *pvAutoPausePar);
MRESULT Spt4CfaPbb2Skip(void *pvSptHdl, u64 u8FileOfst, u64 u8Len);
MRESULT Spt4CfaInqInfNotify(void *pvSptHdl, u32 u4InfType);
MRESULT Spt4CfaPTSNotify(void *pvSptHdl, u64 u8CfaPTS);
MRESULT Spt4CfaSetLpcmEmphasis(void *pvSptHdl, bool fgEmp);
MRESULT Spt4CfaTurnCPS(void *pvSptHdl, CFA_CPS_INFO_T *prCPSInf);
MRESULT Spt4CfaSetCPSInfo(void *pvSptHdl, void *pvPathPar, CFA_CPS_INFO_PARAMS *prCPS);
MRESULT Spt4CfaTurnDivxDRM(void *pvSptHdl, CFA_DIVXDRM_INFO_T *prDivxDRMInf);
MRESULT Spt4CfaSCRGapCB(void *pvSptHdl, u64 u8ScrGap);
MRESULT Spt4CfaSCRNotify(void *pvSptHdl, u64 u8CfaSCR);

/* To Get the Destination Fifo's Size of the designated Stream */
u32 Spt4CfaGetStreamFifoSize(void *pvSptHdl, E_SPT_DATA_TYPE_T eType);

#if CONFIG_DRV_HDMI_RX
MRESULT Spt4CfaGetAudInParsingInfo(void *pvSptHdl, AUDIN_PARSING_INFO_T *prPsrInfo);
bool Spt4CfaAudInIsRAW(void *pvSptHdl);

#endif				/* CONFIG_DRV_HDMI_RX */

/*! @} */

/*! @} */

#ifdef __cplusplus
}
#endif

#endif				/* / #ifndef DMX_INTERNAL_SPT_CFA_H */
