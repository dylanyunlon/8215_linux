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

#ifndef _CFA_TS_ST_CTRL_H_
#define _CFA_TS_ST_CTRL_H_

#include "x_typedef.h"
#include <media/atc/dmx_define.h>

#include "cfa_ts.h"

/* C header file */
#ifdef __cplusplus
extern "C" {
#endif

#define CFG_CHECK_ONLY_transport_error_indicator	0
#define CFG_SHOW_LOG								1
#define CFG_DISCARD_ERROR_PES						1

#define CFA_TS_MAX_CC_ERR_CNT_IN_PES		1

#define CFA_TS_MIN_PES_HEADER_LEN			6
#define CFA_TS_PES_HEADER_LEN				9
#define CFA_TS_MAX_PES_HEADER_LEN			(9 + 255)

#define CFA_TS_MAX_PES_PACKET_LEN			(200 * 1024)


#define CFA_TS_MS2TSPTS(u8Ms)				(90 * (u8Ms))

#define PTS_1S								90000

#define NO_SIGNAL_TICK_COUNT				(1000 * 3)
#define PTS_INTERVAL						(PTS_1S * 5)
#define MAX_GAP_PTS_BETWEEN_AV				(PTS_1S * 5)
#define PTS_EXCEPTION_INTERVAL				(PTS_1S * 60 * 10) /*pts exception*/
#define CFA_TS_DEFUALT_VID_FRM_DURATION		3000

#define NO_AUD_TICK_COUNT					(5 * 1000)


#define CFA_TS_FLAG_PAYLOAD						0x10
#define CFA_TS_FLAG_ADAPTATION_FIELD			0x20
#define CFA_TS_FLAG_UNIT_START					0x40
#define CFA_TS_FLAG_TRANSPORT_ERROR_INDICATOR	0x80
#define CFA_TS_FLAG_TRANSPORT_PRIORITY			0x20
#define CFA_TS_FLAG_PTS							0x80
#define CFA_TS_FLAG_PES_EXTENSION				0x01

#define CFA_TS_MASK_CONTINUTY_COUNTER			0x0f
#define CFA_TS_MASK_PID							0x00001fff

#define CFA_TS_MAX_ADAPTATION_FIELD_LEN			182
#define CFA_TS_SYNC_WORD						0x47

#define CFA_TS_GET_PID(ucByte1, ucByte2)		(((((u32)(ucByte1)) << 8) | (ucByte2)) & CFA_TS_MASK_PID)


#define CFA_TS_START_CODE_BYTE0					0x00
#define CFA_TS_START_CODE_BYTE1					0x00
#define CFA_TS_START_CODE_BYTE2					0x01

#define CFA_TS_STREAM_ID_PADDING_STREAM			0xbe
#define CFA_TS_STREAM_ID_PROG_STREAM_MAP		0xbc
#define CFA_TS_STREAM_ID_PRIVATE_STREAM_2		0xbf
#define CFA_TS_STREAM_ID_ECM					0xf0
#define CFA_TS_STREAM_ID_EMM					0xf1
#define CFA_TS_SREEAM_ID_PROG_STREAM_DIRECTORY	0xff
#define CFA_TS_STREAM_ID_DSMCC_STREAM			0xf2
#define CFA_TS_STREAM_ID_H2221_TYEPE_E_STREAM	0xf8


typedef enum {
	CFA_TS_ST_PES_IDLE,
	CFA_TS_ST_PES_MIN_HEADER,
	CFA_TS_ST_PES_HEADER,
	CFA_TS_ST_PES_HEADER_OPTIONAL,
	CFA_TS_ST_PES_PAYLOD,
	CFA_TS_ST_PES_SKIP
} ECfaTsPESAnaState;


typedef struct {
	ECfaTsPESAnaState eState;

	u8 aucHeader[CFA_TS_MAX_PES_HEADER_LEN]; /* buffer of header data*/
	u32 u4HeaderLen;				/*header data length in buffer aucHeader*/
	u32 u4PesHeaderDataLen;		/*pes packet header length, include start code*/

	u32 u4PesPacketLen;	/*pes packet length, not include 6 bytes header (start code + pes packet length).*/
	u32 u4PesPacketRealLen;
	u32 u4PesPacketDataLen;		/*data length to be decoded in this pes packet*/

	u64 u8Pts;
#if CFG_SUPPORT_HDCP
	unsigned long u4StreamCounter;
	unsigned long long u8InputCounter;
	bool fgHDCP;
#endif
} CfaTsPesHeader_T;

#define MAX_TABLEID_ONE_PID			5
#define MAX_SECTION_LEN				4096
#define MAX_EIT_TABLE_ID			0x6f
#define MIN_EIT_TABLE_ID			0x4e

#define CFA_TS_PMT_TABLE_ID		((u32)0x02)
#define CFA_TS_PMT_MIN_PID			((u32)0x1fc8)
#define CFA_TS_PMT_MAX_PID			((u32)0x1fcf)

typedef struct {
	u32		u4SectionLen;
	u16		u2TableId;
	u16		u2ServiceId;
	u8		aucDataBuf[MAX_SECTION_LEN];  /*buffer of data to be send to section fifo*/
	u32		u4DataLen;	/*data length to be send to afifo*/
} SectionInfo_T;

/*CC private*/
typedef struct {
	void *pvSptHdl;
	CfaTsInst *prCfaTs;
	bool fgAnaPesHeaderEnd;

	CfaTsPesHeader_T rPesHeader;

	u8 *pucDataBuf;	/*buffer of data to be send to afifo*/
	u32 u4BufLen;	/*buffer length*/
	u32 u4DataLen;  /*data length to be send to afifo*/
} CfaTsCcPrivate_T;

/*section private*/
typedef struct {
	void *pvSptHdl;
	CfaTsInst *prCfaTs;

	SectionInfo_T arSecInfo[MAX_TABLEID_ONE_PID];

	u16		u2TableIdNum;
	u16		u2CurTableIdIdx;
} CfaTsSecPrivate_T;



/*video private*/
typedef struct {
	void *pvSptHdl;
	CfaTsInst *prCfaTs;
	bool fgAnaPesHeaderEnd;

	CfaTsPesHeader_T rPesHeader;

	u32 u4Fps_n;
	u32 u4Fps_d;
	u32 u4BitRate;

	u64 u8FileOffset;
	u8 *pucDataBuf;	/*buffer of data to be send to afifo*/
	u32 u4BufLen;	/*buffer length*/
	u32 u4DataLen;  /*data length to be send to afifo*/

	u16 u2PcrPid;

	/*video codec*/
	CfaApiVidType eVCodec;
} CfaTsVidPrivate_T;

/*audio private*/
typedef struct {
	void *pvSptHdl;
	CfaTsInst *prCfaTs;
	bool fgAnaPesHeaderEnd;
	bool fgCcError;

	CfaTsPesHeader_T rPesHeader;

	u8 *pucDataBuf;	/*buffer of data to be send to afifo*/
	u32 u4BufLen;	/*buffer length*/
	u32 u4DataLen;  /*data length to be send to afifo*/

	u16 u2PcrPid;

	/*audio codec*/
	CfaApiAudType eACodec;

} CfaTsAudPrivate_T;


EXTERN u32 CfaTsSyncPbbuf(void *pvSptHdl, CfaTsInst *prCfaTs, u64 u8ReadLen);

EXTERN void CfaTsTxDoneStCtrl(void *pvSptHdl, u64 u8TxLen, CfaTsInst *prCfaTs);

EXTERN void CfaTsVidPesCb(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4DataLen);

EXTERN void CfaTsBufVidPesCb(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4DataLen);

EXTERN void CfaTsCcPesCb(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4DataLen);

EXTERN void CfaTsAudPesCb(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4DataLen);

EXTERN void CfaTsBufAudPesCb(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4DataLen);

EXTERN void CfaTsSectionCb(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4DataLen);

extern void CfaTsResumeBak(CfaTsInst *prCfaTs);

/* C header file */
#ifdef __cplusplus
}
#endif

#endif


