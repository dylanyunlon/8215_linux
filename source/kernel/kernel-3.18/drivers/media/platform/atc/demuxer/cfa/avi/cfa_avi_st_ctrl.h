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



#ifndef CFA_AVI_ST_CTRL_H
#define CFA_AVI_ST_CTRL_H


/* C header file */
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
					Macros, Typedefs, Enumerations
-----------------------------------------------------------------------------*/

/*--- from Mps_util.h */
#define dwCfaAviFourCC(a, b, c, d)  ((((u32)(d))<<(u32)24)|(((u32)(c))<<(u32)16)|(((u32)(b))<<(u32)8)|((u32)(a)))
#define dwCfaAviTwoCC(a, b)	((((u16)(b))<<(u16)8)|((u16)(a)))
#define fgCfaAviIs4cc(addr, a, b, c, d) (*((u32 *)(addr)) == dwCfaAviFourCC((a),(b), (c), (d))

/* original: fgToPlay(arg) */
#define CfaAviToPlay(variable, format) (((variable) & (format)) > 0)

/*--- from Pr_mps.h */
#define CFA_AVI_HDRBUF_SZ	 1024	/* must be smaller than the smallest possible sector size (2048) */
#define CFA_AVI_SC_NOT_FOUND ((u16)32767)
#define CFA_AVI_SC_DMC_FOUND ((u16)32766)

/* CFA AVI will take care of the video frame type of DivX3. */
/* In DivX3, the frame type will be P if the first byte after "00dc+4bytesChunkSize" is 0x4?. */
#define DIVX3_P_FRM ((u8)0x40)
#define DIVX3_I_FRM (0x00)
#define WMV_PVOP	((u8)0x01)

#define CFA_AVI_SRCH_KNOWNCHK_TIMES (28)
/*#define CFA_AVI_SRCH_KNOWNCHK_TIMES (10) //3 */

#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
/*if more than half the file is bad, CFA finish, need check...*/
#define CFA_AVI_OFFSET_JUMP_TIMES			(25)
#endif

#define ERR_CHUNKRANG_END_OFST_SCALE		8

#define AVI_4CC_BYTES (4)

#define AVI_READ_EXTRA_DATA_LEN_BYTES (1)
#define AVI_VC1_CHK_START_CODE_BYTE (4)

#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
#define AVI_CHK_VBR_GARBAGE_DATA_BYTE ((u64)2)
#endif

#define AVI_SZ_BYTES (4)
#define AVI_DIVX3_CHK_BYTE (1)

/*-----------------------------------------------------------------------------
// AVI general read bytes to header buffer,
// Previously: almost 8 bytes, in order to support DivX3, may read 9 bytes
// related definition: DIVX3_P_FRM
-----------------------------------------------------------------------------*/
#define AVI_GEN_READ_BYTES ((u64)(AVI_4CC_BYTES+AVI_SZ_BYTES))/*+AVI_DIVX3_CHK_BYTE)*/



/* DivX subtitle duration have 27 bytes. */
#define DIVX_SUBT_DURATION ((u64)27)

/* DRM chunk header info, 'xxdd' chunk */
#define DRM_INFO_BYTES_FRMKEYIDX (2)
#define DRM_INFO_BYTES_ENCRYPTOFST (4)
#define DRM_INFO_BYTES_ENCRYPTLEN (4)
#define DRM_CHUNK_BYTES (DRM_INFO_BYTES_FRMKEYIDX+DRM_INFO_BYTES_ENCRYPTOFST+DRM_INFO_BYTES_ENCRYPTLEN)


/*-----------------------------------------------------------------------------
					data declarations
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
					function declarations
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
* Name: dwCfaAviConvert2Stc
*
* Description:
*		!!! tmp exported.
*		Calculate (u4A * STC_CKL / u4B) without overflow
*		the original function: dwConvert2Stc
*
* Inputs: -
*
* Outputs: -
*
* Returns:
*-----------------------------------------------------------------------------*/
EXTERN u64 CfaAviConvert2Stc(u32 u4A, u32 u4B);



/*-----------------------------------------------------------------------------
* Name: u8CfaAviChunk2Pts
*
* Description:
*		calculate the PTS from a given chunk number
*		the original function: dAviChunk2Pts
*
* Inputs:
*
* Outputs:
*
* Returns: PTS in CFA_STC_CLK unit
*
*-----------------------------------------------------------------------------*/
EXTERN u64 CfaAviChunk2Pts(const TCfaAviStrmInf *ptStrmInf, u32 u4ChunkNo);


/*-----------------------------------------------------------------------------
* Name: u8CfaAviGetAvaTxSa
*
* Description:
*		AVI CFA get available transfer start address
*
* Inputs:
*
* Outputs:
*
* Returns: available transfer start address
*
*-----------------------------------------------------------------------------*/
EXTERN u64 CfaAviGetAvaTxSa(CfaAviInst *prCfaAvi);


/*-----------------------------------------------------------------------------
* Name: vCfaAviNextScSearch
*
* Description:
*		AVI CFA search next start code
*		1. If file offset is over transfer range, ???
*		   AVI CFA state is changed to CFA_AVI_ANA_ST_IDLE and finish the parsing
*		2. the original function: vMpsPrsNextG
*
* Inputs: -
*
* Outputs: -
*
* Returns: None
*-----------------------------------------------------------------------------*/
EXTERN void CfaAviNextScSearch
(
void *pvSptHdl,   /*< [IN] uintptr_t of splitter */
CfaAviInst *prCfaAvi,  /*< [IN] pointer to CfaAviInst */
CfaAviAnaSt eNxtAnaSt,	   /*< [IN] next analyze state */
u64		u8AdvLen,	   /*< [IN] advance file length before search next start code */
u64		u8ReadLen,	   /*< [IN] the data length we want to read. */
u32		u4DestOfst	   /*< [IN] header buffer ofst. to put the data from this ofst. */
);


/*-----------------------------------------------------------------------------
* Name: vCfaAviTxDoneStCtrl
*
* Description:
*		AVI CFA state control for transfer done
*		This function will be called after a transfer is complete.
*
* Inputs: -
*
* Outputs: -
*
* Returns: None
*-----------------------------------------------------------------------------*/
EXTERN void CfaAviTxDoneStCtrl
(
void *pvSptHdl,  /*< [IN] uintptr_t of splitter */
u64		u8TxLen,
/*< [IN] Actual transferred data length.	Normally this value should be equal to the u4Len
in the previous transfer issue, unless file end is hit. */
CfaAviInst *prCfaAvi  /*< [IN] pointer to CfaAviInst */
);


/*-----------------------------------------------------------------------------
* Name: ucCfaAviGetAudInfoIdx
*
* Description:
*		Get the index of audio info by audio stream ID.
*		For multi-channel.
*
* Inputs: -
*
* Outputs: -
*
* Returns: the index of audio info
*-----------------------------------------------------------------------------*/
EXTERN u8 CfaAviGetAudInfoIdx(
const CfaAviInst *prCfaAvi,	/*< [IN] pointer to CfaAviInst */
u32 u4StrmID				/*< [IN] audio stream ID. */
);


EXTERN void CfaAviFinishPrs(
void *pvSptHdl,  /*< [IN] uintptr_t of splitter */
CfaAviInst *prCfaAvi,  /*< [IN] pointer to CfaAviInst */
u64 u8EndAddr
);


/*-----------------------------------------------------------------------------
* Name: ucCfaAviGetSpInfoIdx
*
* Description:
*		Get the index of SP info by SP stream ID.
*		For multi-channel.
*
* Inputs:
*
* Outputs:
*
* Returns: the index of SP info
*
*-----------------------------------------------------------------------------*/
#if CONFIG_CFA_AVI_TX_ALL_SP
EXTERN u8 CfaAviGetSpInfoIdx(
const CfaAviInst *prCfaAvi,  /*< [IN] pointer to CfaAviInst */
u32 u4StrmID				/*< [IN] audio stream ID. */
);
#endif
/* C header file */
#ifdef __cplusplus
}
#endif

#endif	/* _CFA_AVI_ST_CTRL_H_ */
