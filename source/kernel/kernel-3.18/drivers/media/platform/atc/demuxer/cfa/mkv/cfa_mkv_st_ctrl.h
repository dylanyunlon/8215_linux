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



#ifndef CFA_MKV_ST_CTRL_H
#define CFA_MKV_ST_CTRL_H


/* C header file */
#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------------
				Macros, Typedefs, Enumerations
-----------------------------------------------------------------------------*/

#define dwCfaMkvFourCC(a, b, c, d)		(((d)<<24)|((c)<<16)|((b)<<8)|(a))
#define dwCfaMkvTwoCC(a, b)				(((b)<<8)|(a))
#define fgCfaMkvIs4cc(addr, a, b, c, d)	(*((u32 *)(addr)) == dwCfaMkvFourCC((a), (b), (c), (d)))

#define MKV_CLUSTER_ID_SIZE					4

#define MKV_BLOCK_HEAD_NOLACING				4

#define MKV_BLOCK_HEAD_LACING			  5

#define MKV_NS_TO_MS					 (u64)1000000

#define MKV_PTS_1S						 (u64)90000

/* DRM chunk header info, 'xxdd' chunk */
#define MKV_DRM_INFO_BYTES_FRMKEYIDX	 (2)
#define MKV_DRM_INFO_BYTES_ENCRYPTOFST	 (4)
#define MKV_DRM_INFO_BYTES_ENCRYPTLEN	 (4)
#define MKV_DRM_CHUNK_BYTES			(MKV_DRM_INFO_BYTES_FRMKEYIDX +	\
									MKV_DRM_INFO_BYTES_ENCRYPTOFST +	\
									MKV_DRM_INFO_BYTES_ENCRYPTLEN)

#define MS2PTS(u8TimeMs)				((u8TimeMs)  * ((u64)CFA_STC_CLK / (u64)1000))

#define PRINT_TONEXTSTATE_CALLBACK 0

#define CFA_MKV_VORBIS_DATA_MAX_SIZE		 (u64)((u64)0x0000FE01 & (u64)0xFFFFFFFF)
#define CFA_MKV_VORBIS_ID_TOTAL_LENGTH		 ((u64)58 & (u64)0xFF)

/*< [IN] uintptr_t of splitter */
/*< [IN] Actual transferred data length.  Normally this value should
be equal to the u4Len in the previous transfer issue, unless file end is hit. */
/*< [IN] pointer to CfaMkvInst */
EXTERN void CfaMkvTxDoneStCtrl(void *pvSptHdl,
					u64 u8TxLen,
					CfaMkvInst *prCfaMkv);

EXTERN u8 CfaMkvGetAudIndex(const CfaMkvInst *prCfaMkv,	/*< [IN] pointer to CfaMkvInst */
				   u64 u8StrmID);	/*< [IN] audio stream ID. */

EXTERN u8 CfaMkvGetSpIndex(const CfaMkvInst *prCfaMkv, u64 u8StrmID);


void MkvGetPts(void *pvSptHdl, CfaMkvInst *prCfaMkv);
void MkvFinishPrs(void *pvSptHdl, CfaMkvInst *prCfaMkv);
void MkvSkipBlock(void *pvSptHdl, CfaMkvInst *prCfaMkv);


void vCfaMkvVorbisPageNumCal(CfaMkvInst *prCfaMkv);

void vCfaMkvSetOggHdr(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv);

void vCfaMkvSendOggHdr(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv);

void vCfaMkvSendVorbisData(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv);

void vCfaMkvSendVorbisID(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv);

void vCfaMkvSendVorbisIdHead(void *pvSptHdl, CfaMkvInst *prCfaMkv);

void vCfaMkvSendVorbisPrivOggHdr(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv);

void vCfaMkvSendVorbisComSetupHead(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv);


#if PRINT_TONEXTSTATE_CALLBACK
#define ToNextState(a, b, c, d) ToNextState_I(a, b, c, d, (char *)(DMX_FUNC_NAME), __LINE__)

EXTERN void ToNextState_I(void *pvSptHdl,
			  CfaMkvInst *prCfaMkv,
			  u64 u8ReadLen,
			  ECfaMkvAnaState eCfaMkvNextState, char *function, u32 u4Line);
#else
EXTERN void ToNextState(void *pvSptHdl,
			CfaMkvInst *prCfaMkv,
			u64 u8ReadLen, ECfaMkvAnaState eCfaMkvNextState);
#endif

/* C header file */
#ifdef __cplusplus
}
#endif
#endif				/* _CFA_MKV_ST_CTRL_H_ */
