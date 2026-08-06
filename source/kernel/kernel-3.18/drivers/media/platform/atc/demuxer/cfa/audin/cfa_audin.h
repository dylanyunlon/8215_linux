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

/*****************************************************************************
 * @par Project
 *	  MT8520
 *
 * @par Description
 *	  CFA audio Header File
 *
 * @par Author_Name
 *	  Qing Li mcn06005
*****************************************************************************/

    /*!
     * @file          cfa_audIn.h
     * @author     mcn06005
     * @version 1.0
     * @brief        The C file of the interface for Audio CFA
     */

#ifndef CFA_AUDIN_H
#define CFA_AUDIN_H

#ifdef __linux__
#include <media/atc/dmx_splitter.h>
#else	/*  */
#include "dmx_splitter.h"
#endif				/* __linux__ */

#include "dmx_spt_cfa.h"

#define CFA_OK 0
#define CFA_AUDIN_INVALID_VALUE (-1)

/* CFA Audio invalid address */
#define CFA_AUDIN_INVALID_ADDRESS (-1)

#define CFA_AUDIO_IN_BURST_HEAD_LEN	(8)
#define CFA_AUDIO_IN_CODEC_NUM			   (53)
#define CFA_AUDIO_IN_TX_1K						(1024)
#define CFA_AUDIO_IN_PCM_UNIT_SIZE		  (1024)
#define CFA_AUDIO_IN_MAX_SEARCH_NUM   (20)

#define CFA_AUDIO_IN_CLI_TEST			1

#if CONFIG_DRV_HDMI_RX
#define CFA_AUDIO_IN_SUPPORT_HDMIIN	 1
#else	/*  */
#define CFA_AUDIO_IN_SUPPORT_HDMIIN	 0
#endif	/*  */

#if CFA_AUDIO_IN_SUPPORT_HDMIIN
#define CFA_AUDIN_SUPPORT_MAT		 0
#else	/*  */
#define CFA_AUDIN_SUPPORT_MAT		 0
#endif	/*  */

#define CFA_AUDIO_IN_BUFF_SIZE		 (1024 * 128)	/* for HDMI In */

#define AUDIN_REORDER_BUF_VALID_SIZE (50 * 1024)

typedef enum {
	CFA_AUDIN_LOG_DEFAULT = (u32)1 << (u32)0,
	CFA_AUDIN_LOG_COMMON  = (u32)1 << (u32)1,
} CfaAudioLogLvl_E;


typedef enum {
	CFA_AUDIO_IN_ST_IDL,
	CFA_AUDIO_IN_ST_INFO,
	CFA_AUDIO_IN_ST_HEAD,
	CFA_AUDIO_IN_ST_TX,
	CFA_AUDIO_IN_ST_SYNC,
	CFA_AUDIO_IN_ST_QUER
#if CFA_AUDIN_SUPPORT_MAT
	, CFA_AUDIO_IN_MAT_UNLOCKED, CFA_AUDIO_IN_MAT_CHECKING_MAIN_HEADER,
	CFA_AUDIO_IN_MAT_CHECKING_TOC_HEADER, CFA_AUDIO_IN_MAT_CHECKING_BOC_HEADER,
	CFA_AUDIO_IN_MAT_LOADING_TOC_PAYLOAD, CFA_AUDIO_IN_MAT_LOADING_BOC_PAYLOAD,
	CFA_AUDIO_IN_MAT_CHECKING_TOC_FOOTER, CFA_AUDIO_IN_MAT_CHECKING_BOC_FOOTER
#endif	/*  */
#if CFA_AUDIO_IN_SUPPORT_HDMIIN
	, CFA_HDMI_IN_ST_NEED_SYNC, CFA_HDMI_IN_ST_SYNC, CFA_HDMI_IN_ST_MAT
#endif	/*  */
} ECfaAudInSt;

typedef enum {
	CFA_AUDIO_IN_UNKNOWN,
	CFA_AUDIO_IN_PCM,
	CFA_AUDIO_IN_RAW,
	CFA_AUDIO_IN_DTS14,
CFA_AUDIO_IN_DTS16
#if CFA_AUDIN_SUPPORT_MAT
, CFA_AUDIO_IN_MAT
#endif	/*  */
} ECfaAudInFormat;

/*
typedef enum
{
	CFA_AUDIO_IN_TYPE_NONE,
	CFA_AUDIO_IN_TYPE_NULL,
	CFA_AUDIO_IN_TYPE_AC3,
	CFA_AUDIO_IN_TYPE_SMPTE,
	CFA_AUDIO_IN_TYPE_PAUSE
}ECfaAudioInType;
*/
typedef struct _ValidDataType {
	u8 u1DataType;
	u8 u1SubDataType;
} ValidDataType_T;
typedef struct _AudioInInfo {

		/* Codec */
	u32 u4Codec;

		/* channel */
	u32 u4ChannelNum;
} AudInInfo_T;
typedef struct _BurstInfo {
	u8 u1DataType;
	u8 u1SubType;
	u8 u1ErrFlag;
	u8 u1DepndInfo;
	u8 u1StreamNum;
} Burst_Info_T;

#if CFA_AUDIN_SUPPORT_MAT
typedef enum {
	CFA_MAT_RETURN_CALL_TXTDONE = 100,
	CFA_MAT_RETURN_MLP_AGAIN,
	CFA_MAT_RETURN_CALL_MAC_DEC
} EMatReturnValue;

typedef enum {
	CFA_AUDIO_IN_MLP_UNLOCKED = 0,
	CFA_AUDIO_IN_MLP_TRANSFERRING_AU,
	CFA_AUDIO_IN_MLP_LOCKED
} EMlpStatusType;

typedef enum {
	CFA_MAT_STATUS_UNLOCKED,
	CFA_MAT_STATUS_CHECKING_MAIN_HEADER,
	CFA_MAT_STATUS_CHECKING_TOC_HEADER,
	CFA_MAT_STATUS_CHECKING_BOC_HEADER,
	CFA_MAT_STATUS_LOADING_TOC_PAYLOAD,
	CFA_MAT_STATUS_LOADING_BOC_PAYLOAD,
	CFA_MAT_STATUS_CHECKING_TOC_FOOTER,
	CFA_MAT_STATUS_CHECKING_BOC_FOOTER
} EMatStatusType;

typedef struct {
	u16 u2IceFrame;
	u32 u4MatPtr;
	u32 u4MatPayLoadPtr;
	bool fgMatDecError;
	bool fgLoadingPayload;
	EMatStatusType eMatStatus;
	EMlpStatusType eMlpStatus;
	s32 i4MlpPtr;
	s32 i4AuLeft;	/* number of words left in AU being transferred */
	s32 i4MlpAuPtr;
	u32 u4AuCount;	/* only used for verbose reporting */
	bool fgVerbose;
	u32 u4ReorderBufPosition;
	u32 u4PayloaderDataLength;
	u8 *pucMatDecAuBuf;	/* for Mat Dec decoder AU to audio fifo. */
	u8 u1SaveByte;
	bool fgShouldCallMlp;
	u32 u4UnlockNum;
} AudInMatDecInfo_T;

#endif	/*  */
typedef struct {
	u64 u8Sa;
	u64 u8Ea;
} CfaAudInPR;

typedef enum {
	CFA_AUDIN_QUERY_TYPE_NONE = CFA_AUD_IN_GENRAL_NONE,
	CFA_AUDIN_QUERY_TYPE_CHANNEL = CFA_AUD_IN_GENRAL_CHANNEL,
	CFA_AUDIN_QUERY_TYPE_CODEC = CFA_AUD_IN_GENRAL_CODEC,
	CFA_AUDIN_QUERY_TYPE_CHNL_CODEC = CFA_AUD_IN_GENRAL_CHNL_CODEC
} ECfaAudInQueryType;

typedef struct {
	CfaAudInPR rRange;
	u8 *pucAudBuf;

#if CFA_AUDIO_IN_SUPPORT_HDMIIN
	u8 *pucReOrderBuf;	/* for hdmi in */
	u32 u4ReOrderBufValidSize;
	bool fgReOrder;	/* true is need ReOrder */
	bool fgNeedMat;		/* true is need mat decoder */
	bool fgIsHBR;

#endif	/*  */
	u32 u4PfrMemAddress;	/* memory address got from Pfr, using sync DMA */
	u32 u4TxLen;
	u64 u8CurrTxOft;
	ECfaAudInSt eAudInSt;
	ECfaAudInFormat eFormat;
	CfaApiAudType eAudApiType;
	u32 u4AudInType;
	u32 u4QueryType;
	HANDLE_T hMutex;
	AudInInfo_T rAudInInfo;
	Burst_Info_T rBurstInfo;
	u32 u4FindReadNum;
	u32 u4PcmUnitSize;
	u8 u1CurStreamNum;

#if CFA_AUDIN_SUPPORT_MAT
	AudInMatDecInfo_T rAudInMatDecInfo;

#endif	/*  */
} CfaAudInInst;
	bool fgNeedFilterData(const CfaAudInInst *prCfaAudInInst);
	void vCfaAudInNextScSearch(HANDLE hSpt, CfaAudInInst *prCfaAudInInst,
		ECfaAudInSt eNextSt, u32 u4TxLen);
	void vGetAudInInfo(HANDLE hSpt, CfaAudInInst *prCfaAudInInst);
	void vData2AFifo(HANDLE hSpt, CfaAudInInst *prCfaAudInInst);
	u16 u2AdjustBurstLen(const CfaAudInInst *prCfaAudInInst, u16 u2Pd);
	void vSearchSyncWord(HANDLE hSpt, CfaAudInInst *prCfaAudInInst, u64 u8TxLen);

#endif				/* _CFA_AUDIO_IN_H_ */
