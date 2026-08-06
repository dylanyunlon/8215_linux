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




#ifndef CFA_ASF_H
#define CFA_ASF_H

/*C Header File*/
#ifdef __cplusplus
extern "C"{
#endif

#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/dmx_cfa_asf.h>
#else
#include "dmx_cfa_asf.h"
#endif /* __linux__*/

#include "cfa_if.h"
#include "cfa_cfg.h"
#include "dmx_spt_cfa.h"
#include "dmx_spt.h"

#ifndef __linux__
#pragma warning(disable : 4127)/*conditional expression is constant*/
#pragma warning(disable : 4100)/*unreferenced formal parameter*/
#endif /* #ifndef __linux__*/

/*-----------------------------------------------------------------------------
Macros, Typedefs, Enumerations
-----------------------------------------------------------------------------*/
/* Maximum ASF CFA instance number */
#define CFA_ASF_MAX_INST_NS (CONFIG_CFA_ASF_MAX_INST_NS)

/* ASF CFA header buffer start address offset */
#define CFA_ASF_HDR_BUF_SA_OFST (0)

/* CFA ASF header buffer size  for using sync DMA*/
#define CFA_ASF_HDR_BUF_SZ (128)
/*#define CFA_ASF_HDR_BUF_SZ(64*2)*/

/*CFA ASF compressed payload data header size*/
#define CFA_ASF_COMPRESS_PAYLOAD_HDR_SZ (20)

/* CFA ASF DRM temp buffer size */
#define CFA_ASF_DRM_TMP_BUF_SZ (128)

/*CFA ASF data object header size*/
#define CFA_ASF_DATA_OBJECT_HDR_SIZE (50)

/*System clock unit by KHz per second*/
#define CFA_ASF_SYS_CLK 90

#define CFA_ASF_SUPPORT_VC1_SC1

#define CFA_ASF_SUPPORT_VOD_DRM 1

#define CFA_ASF_CHECK_PIC_TYPE1


#ifdef SPLITTER_SUPPORT_NRD
#define CFA_ASF_NRD_SUPPORT (1)
#else
#define CFA_ASF_NRD_SUPPORT (0)
#endif

#define CFA_ASF_SYNC_BUF_BEFORE_TX	 1


typedef enum {
	CFA_ASF_PRS_BIT_STRM_TYPE_NONE = 0x0,
	CFA_ASF_PRS_BIT_STRM_TYPE_VID = 0x1,
	CFA_ASF_PRS_BIT_STRM_TYPE_AUD = 0x2,
	CFA_ASF_PRS_BIT_STRM_TYPE_PACKET_HDR = 0x3,
	CFA_ASF_PRS_BIT_STRM_TYPE_PAYLOAD_HDR = 0x4,
} CfaAsfPrsBitStrmType_E;

typedef enum {
	CFA_ASF_TX_STRM_TYPE_NONE = 0x0,
	CFA_ASF_TX_STRM_TYPE_VID = 0x1,
	CFA_ASF_TX_STRM_TYPE_AUD = 0x2
} CfaAsfTxStrmType_E;

typedef struct {
	bool fgErrCorExist;
	u8 u1DataLen;
} CfaAsfErrCorFlag_T;

typedef enum {
	Field_Type_UNEXIST = 0x00,
	Field_Type_BYTE = 0x01,
	Field_Type_WORD = 0x02,
	Field_Type_DWORD = 0x03
} CfaAsfFieldType_E;

typedef enum {
	CFA_ASF_LOG_DEFAULT = (u32)1<< (u32)0,
	CFA_ASF_LOG_SET = (u32)1<< (u32)1,
	CFA_ASF_LOG_STATE = (u32)1<< (u32)2,
	CFA_ASF_LOG_SRCH_HDR = (u32)1<< (u32)3,
	CFA_ASF_LOG_TXDONE = (u32)1<< (u32)4,
	CFA_ASF_LOG_FILLAU = (u32)1<< (u32)5,
}CfaAsfLogLvl_E;

typedef struct {
	bool   fgMultiPayloadExist;
	CfaAsfFieldType_E eSequenceType;
	CfaAsfFieldType_E ePadLenType;
	CfaAsfFieldType_E ePacketLenType;
} CfaAsfLenTypeFlag_T;

typedef struct {
	CfaAsfFieldType_E eRepDataLenType;
	CfaAsfFieldType_E ePreTimeType;
	CfaAsfFieldType_E eObjNumType;
	CfaAsfFieldType_E eStrmNumType;
} CfaAsfPropertyFlag_T;

typedef struct {
	u8 u1StrmNum;
	u8 u1ObjNum;
	u32 u4PreTime;
	u8 u1RepDataLen;
} CfaAsfPayload_T;

typedef struct {
	CfaAsfPayload_T  rSinglePayload;
	bool fgCompressExist;
	u8 u1PreTimeDelta;
} CfaAsfSinglePayload_T;

typedef struct {
	u32 u4PayloadNum;
	CfaAsfFieldType_E ePayloadLenType;
} CfaAsfMultiPayloadStart_T;

typedef struct {
	u8 u1PayloadNum;
	CfaAsfFieldType_E ePayloadLenType;
	CfaAsfPayload_T rMultiPayload;
	bool   fgCompressExist;
	u32 u4PayloadLen;
} CfaAsfMultiPayload_T;

typedef struct {
	bool  fgDRMExist;
	u32 u4SecretDataLen;
	u32 u4ProtectTypeLen;
	u8 *pcProtectType;
	u32 u4KeyIDLen;
	u8 *pcKeyID;
	u32 u4LicenseURLLen;
} CfaAsfDRMInfo_T;

typedef struct {
	u64 u8FileSize;
	u64 u8HeaderObjectSize;
	u32 u4DataPacketSize;
	u64 u8DataPacketCount;
	u64 u8PrerollTime;
	bool fgCfaRespliter;
	bool fgOnlyWma;
#if CFA_ASF_NRD_SUPPORT
	bool fgIsNrd;
#endif
} CfaAsfFileInfo_T;

typedef struct {
	u8 u1StrmNum;
	CfaStrmTypeInfo_E eStrmType;
	CfaAsfPayloadExtSysInfo_T arPayloadExtSysInfo[MAX_ASF_PL_EXT_SYS_ID];
} CfaAsfStrmInfo_T;

typedef struct {
	CfaAsfStrmInfo_T   rCfaAsfStrmInfo;
	AVCODECID_T eCodecID;
	u16 u2ChannelNum; /*the number of audio channel, monaural ,stereo, 5.1 audio*/
	u32 u4SamPS;/*Samples per second */
	u32 u4AveBytePS;/* average number of bytes per second*/
	u16 u2BlockAlign;/* the block alignment, or block size*/
	u16 u2BitPerSample; /* the number of bits per sample of  monaural data*/
	/*MTK40504*/
	u32 u4AudCodecSpecDataLen;
	u8	*au1AudCodecSpecData;
	u8 auheader[7];
	bool fgVBR;
	bool fgNeedAdtsHeader;
} CfaAsfAudInfo_T;

typedef struct {
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
#endif
	CfaAsfStrmInfo_T  rCfaAsfStrmInfo;
	AVCODECID_T eCodecID;
	VCODECVERSION_T  eCodecVer;
	u32 u4CodecSpecDataLen;
	u64 u8CodecSpecDataOft;
	/*u8  au1VidCodecSpecData[0x200];*/
	u8 *pu1VidCodecSpecData;
	u64 u8AUTotalLen;
	u64 u8SentLen;
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
#endif
} CfaAsfVidInfo_T;

/*CFA ASF instance state */
/* (unknown)  --init ---> (inited) --turn on---> (ready)*/
/* (ready)--finish ---> (inited)*/
typedef enum {
	CFA_ASF_INST_ST_UNKNOWN = 0x00,
	CFA_ASF_INST_ST_INITED = 0x01,
	CFA_ASF_INST_ST_READY = 0x02
} CfaAsfInstSt_E;

typedef enum {
	CFA_ASF_ANA_ST_IDLE = 0x00,
	CFA_ASF_ANA_ST_SEARCH_PACKET_HEADER = 0x01,
	CFA_ASF_ANA_ST_SEARCH_PAYLOAD_HEADER = 0x02,
	CFA_ASF_ANA_ST_SEARCH_HEADER = 0x03,
	CFA_ASF_ANA_ST_PRS_COMPRESS_PAYLOAD = 0x04,
	CFA_ASF_ANA_ST_TX_VC1_SC_DONE = 0x05,
	CFA_ASF_ANA_ST_TX_PAYLOAD_DATA = 0x06,
	CFA_ASF_ANA_ST_TX_A_HDR = 0x07,
	CFA_ASF_ANA_ST_TX_AAC_ADTS = 0x08,
} CfaAsfAnaSt_E;

typedef struct {
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
#endif
	u64 u8LastTxSa;
	u64 u8LastTxLen;
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
#endif
} CfaAsfLastTxInfo_T;

#if CFA_ASF_NRD_SUPPORT
/* Brief NF information */
typedef struct {
	CFANrdDataType eNrdType; /* Current NF stream type. Video/audio/mux */
	u64 u8AudioStreamID; /* Current audio stream ID. Only avaliable when current NF stream is audio or mux */
	u64 u8VideoStreamID; /* Current video stream ID. Only avaliable when current NF stream is video or mux */
	u64 u8Preroll; /* Preroll time of current NF stream */
	u64 u8PacketSize; /* Packet size of current NF stream */
} CfaAsfNfInfo_t;
#endif

typedef struct {
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
#endif
	/* for cfa asf internal use */
	CfaAsfInstSt_E eCurInstSt; /* current cfa asf instance state */
	CfaAsfAnaSt_E  eCurCfaAsfAnaSt;  /* current cfa asf analyze state*/
	CfaAsfAnaSt_E  eLastCfaAsfAnaSt;
	CfaAsfPrsBitStrmType_E eCurCfaAsfPrsBitStrmType; /*current cfa parsing bit strm type*/
	u8 *pu1HdrBuf;
	u32 u4HdrBufDataLen;
	bool fgRealSyncPbbuf;
	u8 u1CurStrmId; /*cfa asf current analysis stream id, maybe audio or video stream*/
	u64 u8Ca;

	u64 u8LastCa;/*add by Mingxu Wang*/
	u32 u4MemDataLen;/*add by Mingxu Wang, for data lenth from u4PfrMemAddress*/
	uintptr_t ptrPfrMemAddress;  /* memory address got from Pfr, using sync DMA,2007/12/28 */
	bool   fgNoSyncPbb; /*add by Mingxu Wang*/
	bool   fgTxData2HdrBuf;    /*for using sync DMA,2007/12/28 */
	u64 u8PacketStartAdr;
	u32 u4PacketHdrSkipr2Fifo;
	u32 u4PaddingLen;
	u32 u4PayloadLen;
	u32 u4CurPrsFlag;
	u32 u4CfaPrsFlag;
	CfaAsfTxStrmType_E eCurCfaAsfTxStrmType;
	u32 u4OftInMeidaObj;
	u32 u4MediaObjSize;

	u32 u4PacketLen;  /* error handle , maybe each packet length  is not identical*/
	u32 u4SkipLen;
	bool fgSkipErrPacket;
	bool fgErrSingleCompressed; /*this single payload's packet size smaller than packet size in file info.*/

	/*get payload pts*/
	u64 u8PrsPts;
	u8 u1PtsDelta;

	u64 u8PreAudPts;
	u64 u8DiffPts;

	u32 u4PacketSCR;

	/*for Time Search fo PB Engine*/
	bool fgKeyFrame;
	u64 u8CurPacketId;
	bool fgFirstTxVid;
	bool fgFirstTxAud;
	u8 u1AudAUCnt;

	bool fgEnableVid;
	bool fgEnableAud;
	/* set by playback engine*/
	CfaAsfRange_T  rCfaAsfRange;
	CfaApiVidType  eVidCodecType;

	u32 u4CurAudStrmId; /*< current audio stream idx. */
	u8 u1CurAudInfoIdx; /*< current audio info idx related to rCfaAsfAudInfo[]. */
	CfaAsfAudInfo_T raCfaAsfAudInfo[MAX_ASF_AUD_STRM_NUM];
	CfaAsfVidInfo_T rCfaAsfVidInfo;
	CfaAsfStrmInfo_T rCfaAsfStrmInfo;
	CfaAsfFileInfo_T rCfaAsfFileInfo;
	/* playback request these information*/
	CfaAsfCurPosInfo_T rCfaAsfCurPosInfo;
	u64 u8KeyPacketId;
	u64 u8LastKeyFramePacketId;
	u64 u8LastKeyFramePts;

	u32 u4VidObjNum;

	/*for analyzing data packet header*/
	CfaAsfErrCorFlag_T rCfaAsfErrCorFlag;
	CfaAsfPropertyFlag_T rCfaAsfPropertyFlag;
	CfaAsfMultiPayloadStart_T rCfaAsfMultiPayloadStart;
	bool fgExistMultiPayload;
	bool fgFirstParsingMultiPayload;

	/*for analyze compressed payload data*/
	bool fgNeedPrs;
	bool fgExistCompressData;
	u32 u4SubPayloadNum;

	bool fgNeedSyncBufForSpecData;
	bool fgNeedEBIHInfo;
	/*u8 *pucVidCodecSpecData;*/

	u32 u4QueryInfType;  /*< query information type*/
	CfaAsfQIVc1Mode_T rCfaAsfQIVc1Mode;
	CfaAsfLastTxInfo_T rCfaAsfLastTxInfo;

	/*sequence info of codec specical data*/
	bool fgPrsSeqFrameInterpolation;
	bool fgPrsPreProcRange;
	u32 u4PrsNumBFrames;

	/* For I/P/B frame check when filling video pts in AU info.*/
	CfaPicType ePrePicType;
	u64 u8PreVPts;
	bool fgFirstVidAU;/* If it is first video AU. If yes, previous PTS should be invalid*/
	bool fgDisplayOrder;  /* If it is display order or not*/

	/*Payload data first byte, check WMV I / B /P VOP tx mode*/
	u8 u1FirstBytePay;
	/*Check vc1 start code exist or not */
	u32 u4Vc1StartCode;
	bool fgExsitVc1StratCode;
	u8 *pu1CfaAsfVc1Sc;

	/*Audio codec LPCM, should Emphasis*/
	bool fgSetLpcm;

	/*For VOD stream playback DRM encrypt data*/
	CfaAsfCfgDRMInfo_T rCfaAsfDrmInfo;
	bool fgDrmEncrypt;
	CFA_CPS_INFO_T rCfaAsfCpsInfo;
	u64 u8ExtSampleId;

	/* Brief NF data */
#if CFA_ASF_NRD_SUPPORT
	CfaAsfNfInfo_t rCfaAsfNfInfo;
#endif

	bool fgHasAdts;
	bool fgVidData;

	/*splitter4cfa.h  support CFA function pointer list*/
	/*CfaDrvIntf *pCfaDrvIntf;*/

	/*for FR*/
	u32 u4FRAudDataTxLen;
	bool fgFRCurSmpFinish;
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
#endif
} CfaAsfInst_T;

/*cfa asf instrance pool*/
EXTERN CfaAsfInst_T raCfaAsfInst[CFA_ASF_MAX_INST_NS];

/*cfa asf interface need to implementation*/
EXTERN CfaIntf _rAsfCfaIntf;


/* Cfa Asf Internal Function Declaration */
void CfaAsfInitPara(CfaAsfInst_T *prCfaAsfInst);

EXTERN void CfaAsf_FinishPrsNrd(void *pvSptHdl, CfaAsfInst_T *prCfaAsfInst);


#ifdef __cplusplus
}
#endif

#endif /* CFA_ASF_H */

