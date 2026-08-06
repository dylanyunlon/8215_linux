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




#ifndef CFA_RM_H
#define CFA_RM_H

#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/drv_esm_if.h>
#include <media/atc/dmx_cfa_rm.h>
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "drv_esm_if.h"
#include "dmx_cfa_rm.h"
#endif /* __linux__*/

#include "dmx_spt_cfa.h"
#include "cfa_cfg.h"
#include "cfa_if.h"

/*C Header File*/
#ifdef __cplusplus
extern "C"{
#endif

/*-----------------------------------------------------------------------------
		Macros, Typedefs, Enumerations
-----------------------------------------------------------------------------*/

/* Maximum RM CFA instance number */
#define CFA_RM_MAX_INST_NS					(CONFIG_CFA_RM_MAX_INST_NS)

/*System clock unit by KHz per second*/
#define CFA_RM_SYS_CLK						90

#define CFA_RM_CHECK_PIC_TYPE				1

#define CFA_RM_DEINTERLEAVE_AUDIO			1

#if CFA_RM_DEINTERLEAVE_AUDIO
#define CFA_RM_COOK_XOR 					1
#else
#define CFA_RM_COOK_XOR 					0
#endif

#define CFA_RM_READ_LEN 					(1024 * 32)


#define CFA_RM_VIDEO_BUFFER_SIZE			(1024 * 1024)
#define CFA_RM_AUDIO_BUFFER_SIZE			(1024 * 128)

#define RM_MAX_FRAME_NUM_IN_SUP_BLOCK		100

#define CFA_STREAM_NUM_ERR_CNT		0X100

#define CFA_RM_ENDFILE_OFFSET	0X100000 /*1MB*/

#define CFA_RM_TIME_THRESHOLD	750 /* 750ms*/

typedef enum {
	CFA_RM_PRS_BIT_STRM_TYPE_NONE = 0x0,
	CFA_RM_PRS_BIT_STRM_TYPE_VID = 0x1,
	CFA_RM_PRS_BIT_STRM_TYPE_AUD = 0x2,
	CFA_RM_PRS_BIT_STRM_TYPE_PACKET_HDR = 0x3,
} CfaRmPrsBitStrmType_E;

typedef enum {
	CFA_RM_TX_STRM_TYPE_NONE = 0x0,
	CFA_RM_TX_STRM_TYPE_VID = 0x1,
	CFA_RM_TX_STRM_TYPE_AUD = 0x2
} CfaRmTxStrmType_E;

typedef struct {
	bool fgErrCorExist;
	u8 u1DataLen;
} CfaRmErrCorFlag_T;

typedef struct {
	u64 u8FileSize;
	u32 u4HeaderSize;
	u32 u4StreamNum;
	RM_STREAM_TYPE_INFO_T *prStreamInfo;
	u32 u4DataPacketSize;
	u64 u8DataPacketCount;
	u64 u8PrerollTime;
	bool fgCfaRespliter;
} CfaRmFileInfo_T;

typedef enum {
	CFA_RM_LOG_DEFAULT = (u32)1<< 0,
	CFA_RM_LOG_SET = (u32)1<< 1,
	CFA_RM_LOG_STATE = (u32)1<< 2,
	CFA_RM_LOG_SRCH_HDR = (u32)1<< 3,
	CFA_RM_LOG_TXDONE = (u32)1<< 4,
	CFA_RM_LOG_FILLAU = (u32)1<< 5,
}CfaRmLogLvl_E;

typedef enum {
	RM_FRM_TYPE_NONE,
	RM_FRM_TYPE_I,
	RM_FRM_TYPE_P,
	RM_FRM_TYPE_B,
} RM_FRM_TYPE_E;

typedef enum {
	CFA_RM_ANA_ST_IDLE	= 0x00,
	CFA_RM_ANA_ST_SEARCH_HEADER,
	CFA_RM_ANA_ST_TX_AUD,
	CFA_RM_ANA_ST_TX_VIDEO,
	CFA_RM_ANA_ST_TX_A_HDR,

	CFA_RM_ANA_ST_MEDIA_PACKET_HEADER,
	CFA_RM_ANA_ST_READ_PACKET_HEADER,
	CFA_RM_ANA_ST_PATIAL_FRAME,
	CFA_RM_ANA_ST_LAST_PATIAL_FRAME,
	CFA_RM_ANA_ST_MULTIPLE_FRAME,
	CFA_RM_ANA_ST_CBR_AUDIO_DATA,

} CfaRmAnaSt_E;

typedef struct {
	u64 u8LastTxSa;
	u64 u8LastTxLen;
} CfaRmLastTxInfo_T;

/*-------------------------------------------*/
typedef enum {
	CFA_RM_PACKET_TYPE_UNKNOWN,
	CFA_RM_PACKET_TYPE_AUDIO,
	CFA_RM_PACKET_TYPE_VIDEO
} CfaRmPacketType_E;

typedef struct {
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
	#endif
	RM_STREAM_TYPE_E ePacketType;
	u16 u2Length;
	u16 u2StreamNum;
	u32 u4TimeStamp;
	u64 u8Offset;
	/*for aac audio*/
	u16 u2FramNum;
	u16 u2CurFramNum;
	u32 u4FramOffset;
	u16 arFramLen[128];
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
	#endif
} CfaRmPacketInfo_T;

typedef struct {
	u8 u1FramType;
	u8 u1NumPackets;	/*The number of partial-frame packets into which the RealVideo frame is broken*/
	u8 u1PacketNum; 		/*The index within the set of partial-frame packets,[1,u1NumPackets]*/
	u8 u1SequenceNum;		/*a sequence index for each frame.*/
	u16 u2BrokenUpByUs; 	/* 1  bite*/
	u16 u2PatialFramSize;
	u32 u4EntireFramSize;	/*entire frame size by byte*/
	u32 u4PartFrameOffset;	/*offset in entire frame*/
	u32 u4CurStartOffset;	/*start offset of current patial frame in file*/
} CfaRmPartFramInfo_T;

typedef struct {
	u32 u4FrameSize;
	u8 u1FramType;
	u8 u1NumPackets;
	u8 u1PacketNum;
	u8 u1SequenceNum;
	CfaRmPartFramInfo_T rFrameInfo;
} Entire_Frame_Data_T;

typedef struct {
	u32 u4FrameSize;
	u32 u4StartOffset;
	u32 u4TimeStamp;
	u8	u1SequenceNum;
} RM_V_Frame_T;

typedef struct {
	RM_V_Frame_T rFrame;
	u32 u4PacketSize;
	u32 u4ParseredSize;
	u32 u4PacketOffset;
} Multiple_Frame_T;

typedef struct {
	u16 u2PacketSize;
	u16 u2ParseredSize;
	u32 u4PacketOffset;
	u8 u1NumFrames;
	u8 u1TxFrameIdx;
} RM_Audio_Packet_T; /*for sipr,cook,atrc*/
/*------------------------------------------*/

typedef struct {
	CfaRmCfgAudInfo_T rCfgInfo;
	u8 *pauAdtsHeader;
	CfaApiAudType eAudType;
} CfaRmAudInfo_T;

typedef struct {
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
	#endif
	CfaRmCfgVidInfo_T rCfgInfo;
	RM_FRM_TYPE_E eFrmType;
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
	#endif
} CfaRmVidInfo_T;

typedef struct _CFA_RM_SLICE_IFO {
	u8 u1TotalSliceNum;
	RM_VID_SLICE_ELEM_INF rSliceInf[RM_VID_SLICE_MAX_NUM];
} CFA_RM_SLICE_INFO;

typedef struct {
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
	#endif
	/* for cfa rm internal use */
	CfaRmAnaSt_E  eCurAnaSt;  /* current cfa rm analyze state*/
	CfaRmAnaSt_E  eLastAnaSt;
	CfaRmPrsBitStrmType_E  eCurPrsBitStrmType; /*current cfa parsing bit strm type*/
	u8 *pu1HdrBuf;
	u8 u1CurStrmId; /*cfa rm current analysis stream id, maybe audio or video stream*/
	u64 u8Ca;			/*< current analyzed address */
	u64 u8PreCa;	  /*previous valid address*/

	uintptr_t ptrPfrMemAddress;		/* memory address got from Pfr, using sync DMA,2007/12/28 */
	u32 u4PacketHdrSkipr2Fifo;
	u32 u4PaddingLen;
	u32 u4PayloadLen;
	u32 u4CurPrsFlag;
	u32 u4CfaPrsFlag;
	u32 u4TxLen;
	CfaRmTxStrmType_E eCurTxStrmType;

	/*get payload pts*/
	u64 u8PrsPts;

	bool fgTxData2HdrBuf;
	bool fgFirstTxVid;
	bool fgFirstTxAud;

	bool fgEnableVid;
	bool fgEnableAud;
	/* set by playback engine*/
	CfaRmRange_T  rRange;
	CfaApiVidType  eVidCodecType;

	CfaRmAudInfo_T rAudioInfo;
	CfaRmVidInfo_T rVideoInfo;
	CfaRmFileInfo_T rFileInfo;

	RmAacCfgInfo_T rAacCfgInfo;

	/* playback request these information*/
	CfaRmCurPosInfo_T rCurPosInfo;

	/* For I/P/B frame check when filling video pts in AU info.*/
	CfaPicType ePrePicType;
	CfaApiPicTxMode eCurPicType;
	u64 u8PreVPts;
	u64 u8PreVBPts;/*TRUEBPIC*/
	u64 u8AudPts;/*for COOK*/

	/*Payload data first byte, check  I / B /P VOP tx mode*/
	u8 u1FirstBytePay;
	u8 u1SliceFirstByte;
	u16 u2Duration;
	u32 u4First4BytesPay;
	u64 u8ForwardRefPts;
	u64 u8BackwardRefPts;
	u32 u4ForwardRefTr;
	u32 u4BackwardRefTr;
	u32 u4CurPicTr;
	u32 u4PacketSum;/*for COOK time*/
	u32 u4SuperBlockSize;/*for COOK*/
	u32 u4TxSizeInSuperBlock; /*for COOK*/
	u32 u4TxRemainLen; /*for COOK*/

	u32 u4Tx2VFifoLen;		/* the partial frame data length that has transfer to  vfifo*/
	u32 u4CurFrmTotalLen;	  /* the total length that will create an AU*/

	u64 u8Endoffst;
	u64 u8FileSz ;	/*< current file size in bytes */

	u32 u4StrmErrCnt;

	bool   fgSetJumpRange;
	bool   fgFinishRWAU;
	u32 u4RWUnitAULen;
	bool   fgHasVideo;

	/*----------------------------------*/
	CfaRmPacketInfo_T rPacketInfo;
	Entire_Frame_Data_T rFrameData;
	Multiple_Frame_T rMultipleFrame;
	RM_Audio_Packet_T rAudioPacket;
	u16 u2RmPattern[RM_MAX_FRAME_NUM_IN_SUP_BLOCK];
	u8 *puAudioBuf;
	u8 u1FrameIdxInPacket;
	CFA_RM_SLICE_INFO rSliceInf;
	/*----------------------------------*/

	/*splitter4cfa.h  support CFA function pointer list*/
	/*CfaDrvIntf *pCfaDrvIntf;*/
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
	#endif
} CfaRmInst_T;


/*cfa rm interface need to implementation*/
EXTERN CfaIntf _rRmCfaIntf;


/* Cfa Rm Internal Function Declaration */
void CfaRmInitPara(CfaRmInst_T *prCfaRm);

#ifdef __cplusplus
}
#endif

#endif /* CFA_RM_H */

