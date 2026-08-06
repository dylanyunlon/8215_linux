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

#ifdef __linux__
#include <media/atc/dmx_define.h>
#else
#include "dmx_define.h"
#endif
#include "dmx_def.h"
#include "dmx_mem.h"
#include "cfa_audin_matdec.h"

/* IEC/MAT defines*/

#if CFA_AUDIN_SUPPORT_MAT

u16 MatMainHeader[10] = { 0xf872, 0x4e1f, 0x0016, 0xeff0, 0x079e, 0x0003,
							0x8401, 0x0101, 0x8000, 0x56a5 };
u16 MatTocHeader[4]  = { 0x3bf4, 0x8183, 0x4980, 0x77e0 };
u16 MatTocFooter[2]  = { 0xc3c1, 0x4249 };
u16 MatBocHeader[4]  = { 0x3bfa, 0x8283, 0x4980, 0x77e0 };
u16 MatBocFooter[12]  = { 0xc3c2, 0xc0c4, 0x0000, 0x0000, 0x0000, 0x0000,
							0x0000, 0x9711, 0x0000, 0x0000, 0x0000, 0x0000 };
u16 u2MatPayloadChunk[2*MAT_PAYLOAD_SIZE];	/* store two payloads*/
u16 u2MatAuBuffer[CFA_MATDEC_AU_MAX_SIZE/2];

/*
//---------------------------------------------------------------------------

// check check_nibble of current access unit.

// if OK then return TRUE

// else return FALSE [result of conditional check (checkNibble == 0x0f) ]

//---------------------------------------------------------------------------
*/

bool fgVerifyCheckNibble(u16 *u2pAccessUnit)
{

	static u32 u4Substreams;
	s32 i;
	u32 u4Substream;
	u16 u2CheckNibble;
	u32 u4PossibleSyncWord;
	u16 u2AccessUnitOne = 0;
	u16 u2AccessUnitTwo = 0;

	u2CheckNibble = 0;

#if 0

	for (i = 0; i < 2; i++)
		u2CheckNibble = (u16)(u2CheckNibble ^ u2pAccessUnit[i]);

	u4PossibleSyncWord = (u2pAccessUnit[i]<<16) + u2pAccessUnit[i+1];

	if ((u4PossibleSyncWord == FORMATSYNC_FBB) || (u4PossibleSyncWord == FORMATSYNC_FBA)) {
		u4Substreams = (u2pAccessUnit[10] >> 12) & 0x0f;

		/* want to jump over major_sync for calculation of check_nibble*/
		i += 13;
		if (u4PossibleSyncWord == FORMATSYNC_FBA) {
			/* need to check for extra_channel_meaning_present*/
			if ((u2pAccessUnit[i-1] & 0x01) == 0x01)
				i += (((u2pAccessUnit[i]>>12) & 0x0f)+1);
		}
		i += 1;			/* jump over major_sync_info_CRC*/
	}

	for (u4Substream = 0; u4Substream < u4Substreams; u4Substream++) {
		u2CheckNibble = (u16)(u2CheckNibble ^ u2pAccessUnit[i++]);

		if ((u2pAccessUnit[i-1] & 0x8000) == 0x8000) {
			/* extra u4Substream word (dynamic_range_control)*/
			u2CheckNibble = (u16)(u2CheckNibble ^ u2pAccessUnit[i++]);
		}
	}

	/* collapse u2CheckNibble down to 4 bits and return result of comparison*/

	u2CheckNibble = (u16)((u2CheckNibble ^ (u2CheckNibble>>8)) & 0xff);
	u2CheckNibble = (u16)((u2CheckNibble ^ (u2CheckNibble>>4)) & 0x0f);

	return (u2CheckNibble == 0x0f);

#else
	for (i = 0; i < 2; i++) {
		LOADB_WORD((u8 *)&u2pAccessUnit[i], u2AccessUnitOne);
		u2CheckNibble = (u16)(u2CheckNibble ^ u2AccessUnitOne);
	}

	LOADB_WORD((u8 *)&u2pAccessUnit[i], u2AccessUnitOne);
	LOADB_WORD((u8 *)&u2pAccessUnit[i+1], u2AccessUnitTwo);

	u4PossibleSyncWord = (u2AccessUnitOne<<16) + u2AccessUnitTwo;

	if ((u4PossibleSyncWord == FORMATSYNC_FBB) || (u4PossibleSyncWord == FORMATSYNC_FBA)) {
		LOADB_WORD((u8 *)&u2pAccessUnit[10], u2AccessUnitOne);
		u4Substreams = (u2AccessUnitOne >> 12) & 0x0f;

		/* want to jump over major_sync for calculation of check_nibble*/

		i += 13;

		if (u4PossibleSyncWord == FORMATSYNC_FBA) {
			/* need to check for extra_channel_meaning_present*/

			LOADB_WORD((u8 *)&u2pAccessUnit[i], u2AccessUnitOne);
			LOADB_WORD((u8 *)&u2pAccessUnit[i-1], u2AccessUnitTwo);

			if ((u2AccessUnitTwo & 0x01) == 0x01)
				i += (((u2AccessUnitOne>>12) & 0x0f) + 1);
		}
		i += 1;			/* jump over major_sync_info_CRC*/
	}

	for (u4Substream = 0; u4Substream < u4Substreams; u4Substream++) {
		LOADB_WORD((u8 *)&u2pAccessUnit[i], u2AccessUnitOne);
		u2CheckNibble = (u16)(u2CheckNibble ^ u2AccessUnitOne);
		i++;

		LOADB_WORD((u8 *)&u2pAccessUnit[i-1], u2AccessUnitTwo);
		if ((u2AccessUnitTwo & 0x8000) == 0x8000) {
			/* extra u4Substream word (dynamic_range_control)*/
			LOADB_WORD((u8 *)&u2pAccessUnit[i], u2AccessUnitOne);
			u2CheckNibble = (u16)(u2CheckNibble ^ u2AccessUnitOne);
			i++;
		}
	}

	/* collapse u2CheckNibble down to 4 bits and return result of comparison*/

	u2CheckNibble = (u16)((u2CheckNibble ^ (u2CheckNibble>>8)) & 0xff);
	u2CheckNibble = (u16)((u2CheckNibble ^ (u2CheckNibble>>4)) & 0x0f);

	return (u2CheckNibble == 0x0f);
#endif
}
/*
//---------------------------------------------------------------------------

// extract MLP stream from MAT payload.

// This should be done as a background process to the main MAT state machine

// Only complete access units should be output to the MLP stream.

//

// To ensure data rate does not exceed system constraints real time

// implementations should preserve the timing information given by the using

// the starting location of the access unit within the MAT stream.	If

// input_timing is used, then it is important to also read output_timing and

// perform seamless branching checks to ensure intended timing relationships

//

// This function buffers an entire access_unit before writing to file.	It

// is important to ensure that realtime implementations maintain timing

// relationships.

//---------------------------------------------------------------------------
*/

s32 i4ExtractMLPstream(HANDLE hSpt, CfaAudInInst *prCfaAudInInst, s32 i4MlpEnd)
{
	u16	u2MlpWord1, u2MlpWord2, u2MlpWord3, u2MlpWord4;
	u16	u2MAuWord1, u2MAuWord2, u2MAuWord3, u2MAuWord4;
	CFA_AUDIO_INFO_T rTxAudInfo;

	MRESULT mrResult = RET_DMX_OK;

	u2MlpWord1 = 0;
	u2MlpWord2 = 0;
	u2MlpWord3 = 0;
	u2MlpWord4 = 0;
	u2MAuWord1 = 0;
	u2MAuWord2 = 0;
	u2MAuWord3 = 0;
	u2MAuWord4 = 0;

	/*
	// mlpPtr = current postion of mlp sniffing
	// i4MlpEnd = end of currently loaded buffer
	*/
	while (prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr < i4MlpEnd) {
		switch (prCfaAudInInst->rAudInMatDecInfo.eMlpStatus) {
		case CFA_AUDIO_IN_MLP_UNLOCKED:		 /* not currently locked - looking for a restart header*/

			u2MlpWord1 = 0;
			/* loop until found non zero data, or at end of payload.*/
			while ((prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr < i4MlpEnd) && (u2MlpWord1 == 0)) {
				u2MAuWord1 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr];
				LOADB_WORD(
					(u8 *)&u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr],
					u2MlpWord1);
				prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++;
			}
			/*
			// if found non zero data in last three words of payload, then ignore and jump to end of payload
			// access units should be on sample boundaries
			//so would never start less than four words from end of payload
			*/
			if (prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr > i4MlpEnd-3) {
				u2MlpWord1 = 0;
				prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr = i4MlpEnd;
			}

			if (u2MlpWord1 != 0) {		  /* if found something, check next 32 bits for format_sync*/
				u32 u4SyncSearch = 0;

			#if 0
				u2MlpWord2 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++];
				u2MlpWord3 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++];
				u2MlpWord4 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++];
			#endif

				u2MAuWord2 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr];
				LOADB_WORD(
					(u8 *)&u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr],
					u2MlpWord2);
				prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++;
				u2MAuWord3 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr];
				LOADB_WORD(
					(u8 *)&u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr],
					u2MlpWord3);
				prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++;
				u2MAuWord4 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr];
				LOADB_WORD(
					(u8 *)&u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr],
					u2MlpWord4);
				prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++;
				u4SyncSearch  = (u2MlpWord3<<16) + u2MlpWord4;

				if ((u4SyncSearch == FORMATSYNC_FBB) || (u4SyncSearch == FORMATSYNC_FBA)) {
					if (prCfaAudInInst->rAudInMatDecInfo.fgVerbose) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d, Found major sync\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
					}

					prCfaAudInInst->rAudInMatDecInfo.i4MlpAuPtr = 0;
					u2MatAuBuffer[prCfaAudInInst->rAudInMatDecInfo.i4MlpAuPtr++] = u2MAuWord1;
					u2MatAuBuffer[prCfaAudInInst->rAudInMatDecInfo.i4MlpAuPtr++] = u2MAuWord2;
					u2MatAuBuffer[prCfaAudInInst->rAudInMatDecInfo.i4MlpAuPtr++] = u2MAuWord3;
					u2MatAuBuffer[prCfaAudInInst->rAudInMatDecInfo.i4MlpAuPtr++] = u2MAuWord4;

					prCfaAudInInst->rAudInMatDecInfo.i4AuLeft = (u2MlpWord1 & 0x0fff) - 4;
					/* already read 64 bits*/
					prCfaAudInInst->rAudInMatDecInfo.eMlpStatus = CFA_AUDIO_IN_MLP_TRANSFERRING_AU;
				}
			}
			break;

		case CFA_AUDIO_IN_MLP_LOCKED:
			u2MlpWord1 = 0;

			while ((prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr < i4MlpEnd) && (u2MlpWord1 == 0)) {
				/* loop until found non zero data, or at end of payload.*/
				u2MAuWord1 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr];
				LOADB_WORD(
					(u8 *)&u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr],
					u2MlpWord1);
				prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++;
			}
			/*
			// if found non zero data in last three words of payload, then ignore and jump to end of payload
			// access units should be on sample boundaries
			//so would never start less than four words from end of payload
			*/
			if (prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr > i4MlpEnd-3) {
				u2MlpWord1 = 0;
				prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr = i4MlpEnd;
			}

			if (u2MlpWord1 != 0) {
				/*
				// u2MlpWord1 is non zero, but need to check whether it is a bit error
				// count number of bits set in u2MlpWord1
				*/
				u16 u2Temp = u2MlpWord1;
				s8 i1bits = 0;

				while (u2Temp) {
					if (u2Temp & 0x0001)
						i1bits++;
					u2Temp >>= 1;
				}
			#if 0
				u2MlpWord2 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++];
				u2MlpWord3 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++];
				u2MlpWord4 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++];
			#endif

				u2MAuWord2 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr];
				LOADB_WORD(
					(u8 *)&u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr],
					u2MlpWord2);
				prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++;

				u2MAuWord3 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr];
				LOADB_WORD(
					(u8 *)&u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr],
					u2MlpWord3);
				prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++;

				u2MAuWord4 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr];
				LOADB_WORD(
					(u8 *)&u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr],
					u2MlpWord4);
				prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++;

				/*
				// bit 33 (position of 'restart_nonexistent' or second bit of 'format_sync')
				//guaranteed to be a '1'
				// so if only have single bit in first word and bit 33 is zero,
				//then we have a single bit error
				*/

				if ((i1bits == 1) && ((u2MlpWord3&0x4000) == 0)) {
					/* single bit error so ignore and carry on looking for access unit*/
					DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
						TEXT("[CFA_AUDIN]%s in line %d,ignoring single bit ")
						TEXT("error in zero padding\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
				} else if ((2*(u2MlpWord1&0x0fff) > CFA_MATDEC_AU_MAX_SIZE) ||
					(2*(u2MlpWord1&0x0fff) < CFA_MATDEC_AU_MIN_SIZE)) {
					/* access_unit_length out of range - more serious error so lose lock*/
					DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
						TEXT("[CFA_AUDIN] %s in line %d,")
						TEXT("invalid access unit length - ignoring and losing lock\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					prCfaAudInInst->rAudInMatDecInfo.eMlpStatus = CFA_AUDIO_IN_MLP_UNLOCKED;
				} else {

					/* everything seems ok, so start loading access unit*/
					prCfaAudInInst->rAudInMatDecInfo.i4MlpAuPtr = 0;
					u2MatAuBuffer[prCfaAudInInst->rAudInMatDecInfo.i4MlpAuPtr++] = u2MAuWord1;
					u2MatAuBuffer[prCfaAudInInst->rAudInMatDecInfo.i4MlpAuPtr++] = u2MAuWord2;
					u2MatAuBuffer[prCfaAudInInst->rAudInMatDecInfo.i4MlpAuPtr++] = u2MAuWord3;
					u2MatAuBuffer[prCfaAudInInst->rAudInMatDecInfo.i4MlpAuPtr++] = u2MAuWord4;
					prCfaAudInInst->rAudInMatDecInfo.i4AuLeft = (u2MlpWord1 & 0x0fff) - 4;
					/* already read 4*16 bits*/
					prCfaAudInInst->rAudInMatDecInfo.eMlpStatus = CFA_AUDIO_IN_MLP_TRANSFERRING_AU;
				}
			}
			break;

		case CFA_AUDIO_IN_MLP_TRANSFERRING_AU:

			while ((prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr < i4MlpEnd) &&
				(prCfaAudInInst->rAudInMatDecInfo.i4AuLeft >= 0)) {
				u2MAuWord1 = u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr++];
				u2MatAuBuffer[prCfaAudInInst->rAudInMatDecInfo.i4MlpAuPtr++] = u2MAuWord1;
				prCfaAudInInst->rAudInMatDecInfo.i4AuLeft--;
			}

			if (prCfaAudInInst->rAudInMatDecInfo.i4AuLeft <= 0) {
				/* have complete access unit so check check_nibble and write to output
				(pass to mlp decoder) */
			#if 0
				s32 i;
			#endif
				u16	u2AuDataLength = 0;
				s32	i4Ret = 0;

				prCfaAudInInst->rAudInMatDecInfo.eMlpStatus = CFA_AUDIO_IN_MLP_LOCKED;

				if (prCfaAudInInst->rAudInMatDecInfo.fgVerbose) {
					LOADB_WORD((u8 *)&u2MatAuBuffer[1], u2MlpWord1);
					DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
						TEXT("[CFA_AUDIN] %s in line %d, Read AU %d,")
						TEXT(" input_timing = 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						prCfaAudInInst->rAudInMatDecInfo.u4AuCount, u2MlpWord1);
					LOADB_WORD((u8 *)&u2MatAuBuffer[2], u2MlpWord1);
					if (u2MlpWord1 == 0xf872) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d, Restart\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
					}
				}

				if (fgVerifyCheckNibble(u2MatAuBuffer)) {
				#if 0
					for (i = 0; i < (u2MatAuBuffer[0] & 0x0fff); i++) {
						fputc(u2MatAuBuffer[i] >> 8, fOut);
						fputc(u2MatAuBuffer[i] & 0xff, fOut);
					}
				#endif
					LOADB_WORD((u8 *)&u2MatAuBuffer[0], u2MlpWord1);
					u2AuDataLength = u2MlpWord1&0x0fff;
					u2AuDataLength = u2AuDataLength * 2;

					dmx_memcpy((u8 *)prCfaAudInInst->rAudInMatDecInfo.pucMatDecAuBuf,
								 (u8 *)&u2MatAuBuffer[0],
								 u2AuDataLength);
					mm_memset(&rTxAudInfo, 0, sizeof(CFA_AUDIO_INFO_T));
					rTxAudInfo.u8FileOfst = 0;
					rTxAudInfo.u8Len = (u64)u2AuDataLength;
					rTxAudInfo.u8Pts = (u64)INVALID_TIMESTAMP; /*change unit in Hz, STC Clock*/
					rTxAudInfo.u4PrsStrmId = 0;/*need modify*/
					rTxAudInfo.eAudType = prCfaAudInInst->eAudApiType;
					rTxAudInfo.fgUnitStart = TRUE;

					/*
					//prCfaAudInInst->u4ReOrderBufValidSize = 0;
					//prCfaAudInInst->eAudInSt = CFA_HDMI_IN_ST_MAT;
					*/

					mrResult = Spt4CfaBuf2AFifoAUCtrl(hSpt,
									prCfaAudInInst->rAudInMatDecInfo.pucMatDecAuBuf,
									&rTxAudInfo,
									rTxAudInfo.u8Len);
					/*DMX_ASSERT(mrResult == RET_DMX_OK);*/
					if (mrResult == RET_DMX_OK) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s: mrResult is not Ok\r\n"), DMX_FUNC_NAME);
						return 0;
					}
					 /*
					i4Ret = prCfaAudInInst->pCfaDrvIntf->pfi4Splitter4CfaBuf2AFifo(hSpt,
						   (u8 *)prCfaAudInInst->rAudInMatDecInfo.pucMatDecAuBuf,
						   0,
						   (u32)INVALID_TIMESTAMP,
						   u2AuDataLength
						   #if	CFA_AAC_UID_TEST
						   ,0
						   #endif
						   );
					*/

					prCfaAudInInst->rAudInMatDecInfo.u4AuCount++;
					/*ASSERT(i4Ret >= 0);*/
					if (i4Ret < 0) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s: return is not valid\r\n"), DMX_FUNC_NAME);
						return 0;
					}
					return CFA_MAT_RETURN_MLP_AGAIN;
				}
				{
					/* check nibble failed so don't output current access unit.
					// loose mlp lock to start restart search*/
					prCfaAudInInst->rAudInMatDecInfo.eMlpStatus = CFA_AUDIO_IN_MLP_UNLOCKED;
					if (prCfaAudInInst->rAudInMatDecInfo.fgVerbose) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d, check_nibble invalid")
							TEXT("in access unit %d, dumping and looking for restart\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							prCfaAudInInst->rAudInMatDecInfo.u4AuCount);
					}

					prCfaAudInInst->rAudInMatDecInfo.u4AuCount++;
				}
			}
			break;
		}
	}

	prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr =
		prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr%(2*MAT_PAYLOAD_SIZE);

	return CFA_OK;
}

void vCfaAudInMatUnlockProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst)
{
	LOADB_WORD(prCfaAudInInst->pucAudBuf, prCfaAudInInst->rAudInMatDecInfo.u2IceFrame);
	prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;
	prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = FALSE;

	if (prCfaAudInInst->rAudInMatDecInfo.u2IceFrame
		== MatMainHeader[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr++]) {
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_MAT_CHECKING_MAIN_HEADER, 2);
	} else {
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_MAT_UNLOCKED, 2);
	}

}

void vCfaAudInMatCheckMainHeaderProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst)
{
	LOADB_WORD(prCfaAudInInst->pucAudBuf, prCfaAudInInst->rAudInMatDecInfo.u2IceFrame);

	if (prCfaAudInInst->rAudInMatDecInfo.u2IceFrame
		!= MatMainHeader[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr++]) {
		prCfaAudInInst->rAudInMatDecInfo.eMlpStatus = CFA_AUDIO_IN_MLP_UNLOCKED;
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_MAT_UNLOCKED, 2);
	} else if (prCfaAudInInst->rAudInMatDecInfo.u4MatPtr == (sizeof(MatMainHeader)/sizeof(u16))) {
		prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_MAT_CHECKING_TOC_HEADER, 2);
	} else {
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_MAT_CHECKING_MAIN_HEADER, 2);
	}

}

void vCfaAudInMatCheckTocHeaderProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst)
{
	LOADB_WORD(prCfaAudInInst->pucAudBuf, prCfaAudInInst->rAudInMatDecInfo.u2IceFrame);

	if (prCfaAudInInst->rAudInMatDecInfo.u2IceFrame !=
		MatTocHeader[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr++]) {
		prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = TRUE;
	} else if (prCfaAudInInst->rAudInMatDecInfo.u4MatPtr ==
		(sizeof(MatTocHeader)/sizeof(u16))) {
		prCfaAudInInst->rAudInMatDecInfo.fgLoadingPayload = TRUE;
		prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst,
			CFA_AUDIO_IN_MAT_LOADING_TOC_PAYLOAD, MAT_PAYLOAD_SIZE);
	} else {
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst,
			CFA_AUDIO_IN_MAT_CHECKING_TOC_HEADER, 2);
	}

}

void vCfaAudInMatCheckBocHeaderProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst)
{
	LOADB_WORD(prCfaAudInInst->pucAudBuf, prCfaAudInInst->rAudInMatDecInfo.u2IceFrame);

	if (prCfaAudInInst->rAudInMatDecInfo.u2IceFrame !=
		MatBocHeader[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr++]) {
		prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = TRUE;
	} else if (prCfaAudInInst->rAudInMatDecInfo.u4MatPtr == (sizeof(MatBocHeader)/sizeof(u16))) {
		prCfaAudInInst->rAudInMatDecInfo.fgLoadingPayload = TRUE;
		prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst,
			CFA_AUDIO_IN_MAT_LOADING_BOC_PAYLOAD, MAT_PAYLOAD_SIZE);
	} else {
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_MAT_CHECKING_TOC_HEADER, 2);
	}

}

void vCfaAudInMatLoadTocPayloadProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst)
{

	/* Copy data from pbbuf to payloadcheckbuffer */

	mm_memcpy(&u2MatPayloadChunk[0], prCfaAudInInst->pucAudBuf, MAT_PAYLOAD_SIZE);

	/* Set payload flag */

	prCfaAudInInst->rAudInMatDecInfo.fgLoadingPayload = FALSE;

	/* To check Toc footer */

	vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_MAT_CHECKING_TOC_FOOTER, 2);

}

void vCfaAudInMatLoadBocPayloadProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst)
{

	/* Copy data from pbbuf to payloadcheckbuffer */

	mm_memcpy(&u2MatPayloadChunk[MAT_PAYLOAD_SIZE], prCfaAudInInst->pucAudBuf, MAT_PAYLOAD_SIZE);

	/* Set payload flag */

	prCfaAudInInst->rAudInMatDecInfo.fgLoadingPayload = FALSE;

	/* To check Boc footer */

	vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_MAT_CHECKING_BOC_FOOTER, 2);

}

void vCfaAudInMatCheckTocFooterProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst)
{

	LOADB_WORD(prCfaAudInInst->pucAudBuf, prCfaAudInInst->rAudInMatDecInfo.u2IceFrame);

	if (prCfaAudInInst->rAudInMatDecInfo.u2IceFrame
		!= MatTocFooter[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr++]) {
		prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = TRUE;
	} else if (prCfaAudInInst->rAudInMatDecInfo.u4MatPtr ==
		(sizeof(MatTocFooter)/sizeof(u16))) {
		if (FALSE == prCfaAudInInst->rAudInMatDecInfo.fgMatDecError) {
			/* // loaded and verified complete Top of channels so extract the MLP stream */
		} else {
			prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr =
				(prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr +
				MAT_PAYLOAD_SIZE) % (2*MAT_PAYLOAD_SIZE);
			prCfaAudInInst->rAudInMatDecInfo.eMlpStatus = CFA_AUDIO_IN_MLP_UNLOCKED;
		}

		prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = FALSE;
		prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;

		vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_MAT_CHECKING_BOC_HEADER, 2);
	} else {
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_MAT_CHECKING_TOC_FOOTER, 2);
	}
}

void vCfaAudInMatCheckBocFooterProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst)
{
	LOADB_WORD(prCfaAudInInst->pucAudBuf, prCfaAudInInst->rAudInMatDecInfo.u2IceFrame);
	if (prCfaAudInInst->rAudInMatDecInfo.u2IceFrame !=
		MatBocFooter[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr++]) {
		prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = TRUE;
	} else if (prCfaAudInInst->rAudInMatDecInfo.u4MatPtr ==
		(sizeof(MatBocFooter)/sizeof(u16))) {
		if (!prCfaAudInInst->rAudInMatDecInfo.fgMatDecError) {
			/* // loaded and verified complete Top of channels so extract the MLP stream */
		} else {
			prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr =
				(prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr +
				MAT_PAYLOAD_SIZE) % (2*MAT_PAYLOAD_SIZE);
			prCfaAudInInst->rAudInMatDecInfo.eMlpStatus = CFA_AUDIO_IN_MLP_UNLOCKED;
		}

		prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = FALSE;
		prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;

		vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_MAT_CHECKING_MAIN_HEADER, 2);
	} else {
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_MAT_CHECKING_TOC_FOOTER, 2);
	}
}

s32 i4CfaAudInMatDecProc(HANDLE hSpt, CfaAudInInst *prCfaAudInInst)
{
	/*ASSERT(NULL != prCfaAudInInst);*/
	if (NULL == prCfaAudInInst) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: NULL == prCfaAudInInst\r\n"), DMX_FUNC_NAME);
		return 0;
	}	

	prCfaAudInInst->rAudInMatDecInfo.fgVerbose = TRUE;

	while (prCfaAudInInst->u4ReOrderBufValidSize > 1) {
		if (prCfaAudInInst->rAudInMatDecInfo.fgLoadingPayload) {
			if (prCfaAudInInst->u4ReOrderBufValidSize >= 2 * MAT_PAYLOAD_SIZE) {
				if (prCfaAudInInst->rAudInMatDecInfo.eMatStatus ==
					CFA_MAT_STATUS_LOADING_TOC_PAYLOAD) {
					/* Copy data from pbbuf to payloadcheckbuffer */

					if (prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength != 0) {
						mm_memcpy(
			&u2MatPayloadChunk[prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength/2],
						(prCfaAudInInst->pucReOrderBuf +
						prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition),
						(MAT_PAYLOAD_SIZE * 2 -
						prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength));

						prCfaAudInInst->u4ReOrderBufValidSize -= (MAT_PAYLOAD_SIZE * 2 -
							prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength);
						prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition +=
							(MAT_PAYLOAD_SIZE * 2 -
							prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength);
						prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength = 0;
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d,")
							TEXT("CFA_MAT_STATUS_LOADING_TOC_PAYLOAD ")
							TEXT("u4PayloaderDataLength != 0\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
					} else {
						mm_memcpy(&u2MatPayloadChunk[0],
								(prCfaAudInInst->pucReOrderBuf +
								prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition),
								MAT_PAYLOAD_SIZE * 2);
						prCfaAudInInst->u4ReOrderBufValidSize -= MAT_PAYLOAD_SIZE * 2;
						prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition +=
							MAT_PAYLOAD_SIZE * 2;
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d,PAYLOAD\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
					}

					prCfaAudInInst->rAudInMatDecInfo.eMatStatus =
						CFA_MAT_STATUS_CHECKING_TOC_FOOTER;
					prCfaAudInInst->rAudInMatDecInfo.fgLoadingPayload = FALSE;
				} else if (prCfaAudInInst->rAudInMatDecInfo.eMatStatus ==
				CFA_MAT_STATUS_LOADING_BOC_PAYLOAD) {
					if (prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength != 0) {
						mm_memcpy(&u2MatPayloadChunk[MAT_PAYLOAD_SIZE +
							prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength/2],
							(prCfaAudInInst->pucReOrderBuf +
							prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition),
							(MAT_PAYLOAD_SIZE * 2 -
							prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength));
						prCfaAudInInst->u4ReOrderBufValidSize -=
							(MAT_PAYLOAD_SIZE * 2 -
							prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength);

						prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition +=
							(MAT_PAYLOAD_SIZE * 2 -
							prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength);

						prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength = 0;

						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d, ")
							TEXT("CFA_MAT_STATUS_LOADING_BOC_PAYLOAD")
							TEXT("u4PayloaderDataLength != 0\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
					} else {
						mm_memcpy(&u2MatPayloadChunk[MAT_PAYLOAD_SIZE],
								 (prCfaAudInInst->pucReOrderBuf +
								 prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition),
								 MAT_PAYLOAD_SIZE * 2);

						prCfaAudInInst->u4ReOrderBufValidSize -= MAT_PAYLOAD_SIZE * 2;
						prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition +=
							MAT_PAYLOAD_SIZE * 2;
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d, ")
							TEXT("CFA_MAT_STATUS_LOADING_BOC_PAYLOAD\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
					}

					prCfaAudInInst->rAudInMatDecInfo.eMatStatus =
						CFA_MAT_STATUS_CHECKING_BOC_FOOTER;
					prCfaAudInInst->rAudInMatDecInfo.fgLoadingPayload = FALSE;
				} else {
				   DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
						TEXT("[CFA_AUDIN] %s in line %d, Failed to paylod One\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
			   }
			} else {
				/* Should save data until next cycle */

				if (prCfaAudInInst->rAudInMatDecInfo.eMatStatus ==
					CFA_MAT_STATUS_LOADING_TOC_PAYLOAD) {
					/* Copy data from pbbuf to payloadcheckbuffer */
					mm_memcpy(&u2MatPayloadChunk[0],
							 (prCfaAudInInst->pucReOrderBuf +
							 prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition),
							 prCfaAudInInst->u4ReOrderBufValidSize);

					prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength =
						prCfaAudInInst->u4ReOrderBufValidSize;

					prCfaAudInInst->u4ReOrderBufValidSize = 0;
					prCfaAudInInst->eAudInSt = CFA_HDMI_IN_ST_SYNC;
					prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition = 0;
					DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
						TEXT("[CFA_AUDIN] %s in line %d,")
						TEXT("Save TOC Payload u4PayloaderDataLength is 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition);

					return CFA_MAT_RETURN_CALL_TXTDONE;
				} else if (prCfaAudInInst->rAudInMatDecInfo.eMatStatus ==
				CFA_MAT_STATUS_LOADING_BOC_PAYLOAD) {
					mm_memcpy(&u2MatPayloadChunk[MAT_PAYLOAD_SIZE],
								 (prCfaAudInInst->pucReOrderBuf +
								 prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition),
								 prCfaAudInInst->u4ReOrderBufValidSize);

					prCfaAudInInst->rAudInMatDecInfo.u4PayloaderDataLength =
						prCfaAudInInst->u4ReOrderBufValidSize;

					prCfaAudInInst->u4ReOrderBufValidSize = 0;
					prCfaAudInInst->eAudInSt = CFA_HDMI_IN_ST_SYNC;
					prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition = 0;

					DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
						TEXT("[CFA_AUDIN] %s in line %d,")
						TEXT("Save BOC Payload u4PayloaderDataLength is 0x%x \r\n"),
						prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition);

					return CFA_MAT_RETURN_CALL_TXTDONE;
				}
				{
					DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
						TEXT("[CFA_AUDIN] %s in line %d, Failed to paylod Two\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
				}
			}
		} else {
			if (prCfaAudInInst->rAudInMatDecInfo.fgShouldCallMlp == FALSE) {
				/* Adjust ptr position */
				if (prCfaAudInInst->rAudInMatDecInfo.u1SaveByte == 0) {
					LOADB_WORD(
						(prCfaAudInInst->pucReOrderBuf +
						prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition),
						prCfaAudInInst->rAudInMatDecInfo.u2IceFrame);

					prCfaAudInInst->u4ReOrderBufValidSize -= 2;
					prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition += 2;
				} else {
					prCfaAudInInst->rAudInMatDecInfo.u2IceFrame = 0;
					prCfaAudInInst->rAudInMatDecInfo.u2IceFrame =
						prCfaAudInInst->rAudInMatDecInfo.u1SaveByte << 8;

					LOAD_BYTE(
						(prCfaAudInInst->pucReOrderBuf +
						prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition),
						prCfaAudInInst->rAudInMatDecInfo.u1SaveByte);

					prCfaAudInInst->rAudInMatDecInfo.u2IceFrame +=
						prCfaAudInInst->rAudInMatDecInfo.u1SaveByte;

					prCfaAudInInst->u4ReOrderBufValidSize -= 1;
					prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition += 1;
					prCfaAudInInst->rAudInMatDecInfo.u1SaveByte = 0;
					DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
						TEXT("[CFA_AUDIN]%s in line %d,Read a byte for Save Byte\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
				}
			}

			switch (prCfaAudInInst->rAudInMatDecInfo.eMatStatus) {
			case CFA_MAT_STATUS_UNLOCKED: {
					prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;
					prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = FALSE;

					if (prCfaAudInInst->rAudInMatDecInfo.u2IceFrame ==
						MatMainHeader[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr++]) {
						prCfaAudInInst->rAudInMatDecInfo.eMatStatus =
							CFA_MAT_STATUS_CHECKING_MAIN_HEADER;

						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d, Change to main header\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
					}

					prCfaAudInInst->rAudInMatDecInfo.u4UnlockNum += 1;
					if ((prCfaAudInInst->rAudInMatDecInfo.u4UnlockNum % 10000) == 0) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d, Mat Status unlock,")
							TEXT("u4ReOrderBufValidSize is %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							prCfaAudInInst->u4ReOrderBufValidSize);
					}
				}
				break;

			case CFA_MAT_STATUS_CHECKING_MAIN_HEADER: {
					if (prCfaAudInInst->rAudInMatDecInfo.u2IceFrame !=
						MatMainHeader[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr++]) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d,")
							TEXT("MAT/IEC header error (expected 0x%x, read 0x%x)\r\n"),
							DMX_FUNC_NAME,
							DMX_LINE_NO,
							MatMainHeader[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr-1],
							prCfaAudInInst->rAudInMatDecInfo.u2IceFrame);
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d, Lose lock and start")
							TEXT("looking for IEC61937 header\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);

						prCfaAudInInst->rAudInMatDecInfo.eMatStatus = CFA_MAT_STATUS_UNLOCKED;
						prCfaAudInInst->rAudInMatDecInfo.eMlpStatus = CFA_AUDIO_IN_MLP_UNLOCKED;
					} else if (prCfaAudInInst->rAudInMatDecInfo.u4MatPtr ==
						(sizeof(MatMainHeader)/sizeof(u16))) {
						prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;
						prCfaAudInInst->rAudInMatDecInfo.eMatStatus =
							CFA_MAT_STATUS_CHECKING_TOC_HEADER;
						/*
						if (prCfaAudInInst->rAudInMatDecInfo.fgVerbose) {
							DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
								TEXT("[CFA_AUDIN] %s in line %d, Read main header\r\n"),
								DMX_FUNC_NAME, DMX_LINE_NO);
						}*/
					}
				}
				break;

			case CFA_MAT_STATUS_CHECKING_TOC_HEADER: {
					if (prCfaAudInInst->rAudInMatDecInfo.u2IceFrame !=
						MatTocHeader[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr++]) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d,")
							TEXT("toc header error (expected 0x%x, read 0x%x)\r\n"),
							DMX_FUNC_NAME,
							DMX_LINE_NO,
							MatTocHeader[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr-1],
							prCfaAudInInst->rAudInMatDecInfo.u2IceFrame);
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d,")
							TEXT("carry on reading, but don't process contents\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);

						prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = TRUE;
					}

					if (prCfaAudInInst->rAudInMatDecInfo.u4MatPtr ==
						(sizeof(MatTocHeader)/sizeof(u16))) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d, toc header\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
						prCfaAudInInst->rAudInMatDecInfo.fgLoadingPayload = TRUE;
						prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;
						prCfaAudInInst->rAudInMatDecInfo.eMatStatus =
							CFA_MAT_STATUS_LOADING_TOC_PAYLOAD;
					}
				}

				break;

			case CFA_MAT_STATUS_CHECKING_TOC_FOOTER:
				if (prCfaAudInInst->rAudInMatDecInfo.fgShouldCallMlp) {
					if (CFA_MAT_RETURN_MLP_AGAIN ==
						i4ExtractMLPstream(hSpt, prCfaAudInInst, MAT_PAYLOAD_SIZE)) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d,")
							TEXT("Succeed TOC MLP stream...,")
							TEXT("should return mlp again No2\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
						prCfaAudInInst->rAudInMatDecInfo.fgShouldCallMlp = TRUE;
						return CFA_OK;
					}
					{
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d,")
							TEXT("Succeed TOC MLP stream...No\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
						prCfaAudInInst->rAudInMatDecInfo.fgShouldCallMlp = FALSE;
						prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = FALSE;
						prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;
						prCfaAudInInst->rAudInMatDecInfo.eMatStatus =
							CFA_MAT_STATUS_CHECKING_BOC_HEADER;

						return CFA_MAT_RETURN_CALL_MAC_DEC;
					}
				}

				if (prCfaAudInInst->rAudInMatDecInfo.u2IceFrame !=
					MatTocFooter[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr++]) {
					DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
						TEXT("[CFA_AUDIN] %s in line %d,")
						TEXT("toc footer error (expected 0x%x, read 0x%x)\r\n"),
						DMX_FUNC_NAME,
						DMX_LINE_NO,
						MatTocFooter[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr-1],
						prCfaAudInInst->rAudInMatDecInfo.u2IceFrame);
					prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = TRUE;
				}

				if (prCfaAudInInst->rAudInMatDecInfo.u4MatPtr ==
					(sizeof(MatTocFooter)/sizeof(u16))) {
					if ((FALSE == prCfaAudInInst->rAudInMatDecInfo.fgMatDecError) &&
						(CFA_MAT_RETURN_MLP_AGAIN ==
						i4ExtractMLPstream(hSpt, prCfaAudInInst,
						MAT_PAYLOAD_SIZE))) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN]%s in line %d,")
							TEXT("Succeed TOC MLP stream...,")
							TEXT("should return mlp again\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
						prCfaAudInInst->rAudInMatDecInfo.fgShouldCallMlp = TRUE;
						return CFA_OK;
					}
					if (FALSE == prCfaAudInInst->rAudInMatDecInfo.fgMatDecError) {
						/* // loaded and verified complete Top
						of channels so extract the MLP stream */
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d,")
							TEXT("extract TOC MLP stream...\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
						{
							DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
								TEXT("[CFA_AUDIN] %s in line %d,")
								TEXT("Succeed TOC MLP stream...\r\n"),
								DMX_FUNC_NAME, DMX_LINE_NO);
							prCfaAudInInst->rAudInMatDecInfo.fgShouldCallMlp =
								FALSE;
						}
					} else {
						prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr =
							(prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr +
							MAT_PAYLOAD_SIZE) % (2*MAT_PAYLOAD_SIZE);
						prCfaAudInInst->rAudInMatDecInfo.eMlpStatus =
							CFA_AUDIO_IN_MLP_UNLOCKED;
					}
					prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = FALSE;
					prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;
					prCfaAudInInst->rAudInMatDecInfo.eMatStatus =
						CFA_MAT_STATUS_CHECKING_BOC_HEADER;
				}
				break;

			case CFA_MAT_STATUS_CHECKING_BOC_HEADER:
				if (prCfaAudInInst->rAudInMatDecInfo.u2IceFrame !=
					MatBocHeader[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr++]) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d,")
							TEXT("boc header error (expected 0x%x, read 0x%x)\r\n"),
						DMX_FUNC_NAME,
						DMX_LINE_NO,
						MatBocHeader[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr-1],
						prCfaAudInInst->rAudInMatDecInfo.u2IceFrame);

					prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = TRUE;
				}
				if (prCfaAudInInst->rAudInMatDecInfo.u4MatPtr ==
					(sizeof(MatBocHeader)/sizeof(u16))) {
					prCfaAudInInst->rAudInMatDecInfo.fgLoadingPayload = TRUE;
					prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;
					prCfaAudInInst->rAudInMatDecInfo.eMatStatus =
						CFA_MAT_STATUS_LOADING_BOC_PAYLOAD;
					DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
						TEXT("[CFA_AUDIN]%s in line %d,Change to BOC Playload\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
				}
				break;

			case CFA_MAT_STATUS_CHECKING_BOC_FOOTER: {
				if (prCfaAudInInst->rAudInMatDecInfo.fgShouldCallMlp) {
					if (CFA_MAT_RETURN_MLP_AGAIN ==
						i4ExtractMLPstream(hSpt, prCfaAudInInst,
						2 * MAT_PAYLOAD_SIZE)) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d,")
							TEXT("Succeed BOC MLP stream..., "
							TEXT("should return mlp again No2\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
						prCfaAudInInst->rAudInMatDecInfo.fgShouldCallMlp = TRUE;
						return CFA_OK;
					}
					{
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN]%s in line %d,")
							TEXT("Succeed BOC MLP stream...No2\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
						prCfaAudInInst->rAudInMatDecInfo.fgShouldCallMlp = FALSE;
						prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = FALSE;
						prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;
						prCfaAudInInst->rAudInMatDecInfo.eMatStatus =
							CFA_MAT_STATUS_CHECKING_MAIN_HEADER;

						return CFA_MAT_RETURN_CALL_MAC_DEC;
					}
				}

				if (prCfaAudInInst->rAudInMatDecInfo.u2IceFrame !=
					MatBocFooter[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr++]) {
					DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
						TEXT("[CFA_AUDIN] %s in line %d,")
						TEXT("boc footer error (expected 0x%x, read 0x%x)\r\n"),
						DMX_FUNC_NAME,
						DMX_LINE_NO,
						MatBocFooter[prCfaAudInInst->rAudInMatDecInfo.u4MatPtr-1],
						prCfaAudInInst->rAudInMatDecInfo.u2IceFrame);
					prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = TRUE;
				}

				if (prCfaAudInInst->rAudInMatDecInfo.u4MatPtr ==
					(sizeof(MatBocFooter)/sizeof(u16))) {
					if ((!prCfaAudInInst->rAudInMatDecInfo.fgMatDecError) &&
						(CFA_MAT_RETURN_MLP_AGAIN ==
						i4ExtractMLPstream(hSpt, prCfaAudInInst,
						2 * MAT_PAYLOAD_SIZE))) {
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d, ")
							TEXT("Succeed BOC MLP stream...,")
							TEXT("should return mlp again\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
						prCfaAudInInst->rAudInMatDecInfo.fgShouldCallMlp = TRUE;
						return CFA_OK;
					}
					if (!prCfaAudInInst->rAudInMatDecInfo.fgMatDecError) {
						/* // loaded and verified complete
						Top of channels so extract the MLP stream */
						DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
							TEXT("[CFA_AUDIN] %s in line %d,")
							TEXT("extract BOC MLP stream...\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
						{
						   DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
								TEXT("[CFA_AUDIN] %s in line %d, ")
								TEXT("Succeed BOC MLP stream..\r\n"),
								DMX_FUNC_NAME, DMX_LINE_NO);
						   prCfaAudInInst->rAudInMatDecInfo.fgShouldCallMlp = FALSE;
						}
					} else {
						prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr =
							(prCfaAudInInst->rAudInMatDecInfo.i4MlpPtr +
							MAT_PAYLOAD_SIZE) % (2*MAT_PAYLOAD_SIZE);
						prCfaAudInInst->rAudInMatDecInfo.eMlpStatus =
							CFA_AUDIO_IN_MLP_UNLOCKED;
					}
					prCfaAudInInst->rAudInMatDecInfo.fgMatDecError = FALSE;
					prCfaAudInInst->rAudInMatDecInfo.u4MatPtr = 0;
					prCfaAudInInst->rAudInMatDecInfo.eMatStatus =
						CFA_MAT_STATUS_CHECKING_MAIN_HEADER;
				}
				break;
			}
			default:
				break;
			}
		}
	}

	if (prCfaAudInInst->u4ReOrderBufValidSize == 1) {
		/* Should save a byte for next use */
		LOAD_BYTE((prCfaAudInInst->pucReOrderBuf +
			prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition),
			prCfaAudInInst->rAudInMatDecInfo.u1SaveByte);
		prCfaAudInInst->u4ReOrderBufValidSize = 0;
		prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition = 0;
		prCfaAudInInst->eAudInSt = CFA_HDMI_IN_ST_SYNC;
	} else {
		prCfaAudInInst->u4ReOrderBufValidSize = 0;
		prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition = 0;
		prCfaAudInInst->rAudInMatDecInfo.u1SaveByte = 0;
		prCfaAudInInst->eAudInSt = CFA_HDMI_IN_ST_SYNC;
	}

	return CFA_MAT_RETURN_CALL_TXTDONE;

}

#endif
