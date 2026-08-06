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

#ifndef _DMX_DECRYPT_H_
#define _DMX_DECRYPT_H_

#include "x_typedef.h"
#include "x_dmx.h"
#include "dmx_define.h"

#define DECRYPT_PLAY_INVALID_ID			 ((__s32)(-1))

#define DECRYPT_DIVXDRM_DRMINFO_SZ		 (10)
#define DECRYPT_DIVXDRM_SET_RND_SAMPLE_CNT (4)

#define DECRYPT_OPERCODE_DEF(DecryptType, OperNo)  ((__u32)(((DecryptType) << 16) | OperNo))
#define DECRYPT_GETTYPE_FROM_OPERCODE(OperCode)	 ((__u32)((OperCode >> 16) & 0x0000FFFF))

/* ///////////////////////////////////////////////////////////////////////////// */
/* Decrypt Mode Type                                                                  // */
/* ///////////////////////////////////////////////////////////////////////////// */
typedef enum {
	DECRYPT_NONE,
	DECRYPT_DIVXDRM,
	MAX_CNT_OF_DECRYPT_TYPE
} E_DECRYPT_TYPE_T;

/* /> DMX_IOCTL_DECRYPT_EXEC_CMD's Input Param */
typedef struct {
	/* DivxDRM Instance Handle, DMX_IOCTL_DECRYPT_CREATE_INST's output param */
	void *pvInst;
	__u32 u4OperCode;	/* Operation Code */
	void *pvOperParam;	/* Operation Input Param */
	__u32 u4OperParamSz;	/* Operation Input Param Size */
} DECRYPT_OPER_PARAM_T;

/* /> DMX_IOCTL_DECRYPT_RELEASE_INST's Input Param */
typedef struct {
	E_DECRYPT_TYPE_T eDecryptType;	/* Decrypt Type */
	/* DivxDRM Instance Handle, DMX_IOCTL_DECRYPT_CREATE_INST's output param */
	void *pvInst;
} DECRYPT_INST_PARAM_T;

#if DMX_SUPPORT_DIVXDRM
/* ///////////////////////////////////////////////////////////////////////////// */
/* Decrypt Operation Code (Command to Execute) */
/* DeviceIOContrl(hDmx, DMX_IOCTL_DECRYPT_EXEC_CMD, */
/* &rOperParam, sizeof(rOperParam), OutputBuf, OutputBufSz, NULL) */
/* ///////////////////////////////////////////////////////////////////////////// */

/* /> DivxDRM Init System: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.u4OperCode = DIVXDRM_INIT_SYSTEM */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: NULL */
#define DIVXDRM_INIT_SYSTEM						 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 0)

/* /> Init Playback: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.u4OperCode = DIVXDRM_INIT_PLAYABCK */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.pvOperParam = &rDivxDrmHdrInfo, */
/* /> rDivxDrmHdrInfo.pu1DrmhdrData = DRM Header obtained from container's preparser */
/* /> rDivxDrmHdrInfo.u4DrmHdrSz          = DRM Header Size */
/* /> rOperParam.u4OperParamSz = sizeof(rDivxDrmHdrInfo) */
/* /> Output Param: &rRentalStatus or NULL */
#define DIVXDRM_INIT_PLAYBACK					 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 1)

/* /> Commit Playback (call before parse on): */
/* /> Input Param Lists as follow: */
/* /> rOperParam.u4OperCode = DIVXDRM_COMMIT_PLAYBACK */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: NULL */
#define DIVXDRM_COMMIT_PLAYBACK				 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 2)

/* /> Finalize Playback: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.u4OperCode = DIVXDRM_FINALIZE_PLAYBACK */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: NULL */
#define DIVXDRM_FINALIZE_PLAYBACK			 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 3)

/* /> Finalize Playback: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.u4OperCode = DIVXDRM_QUERY_RENTAL_STATUS */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: &rRentalStatus */
#define DIVXDRM_QUERY_RENTAL_STATUS		 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 4)

/* /> Query CGMSA info: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.u4OperCode = DIVXDRM_QUERY_CGMSA, */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: &u1Cgmsa, size=sizeof(__u8) */
#define DIVXDRM_QUERY_CGMSA						 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 5)

/* /> Query Acptb info: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.u4OperCode = DIVXDRM_QUERY_ACPTB */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: &u1Acptb, size=sizeof(__u8) */
#define DIVXDRM_QUERY_ACPTB						 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 6)

/* /> Query Digital Protection info: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.u4OperCode = DIVXDRM_QUERY_DIGITAL_PROTECTION */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: &u1DigitalProc, size=sizeof(__u8) */
#define DIVXDRM_QUERY_DIGITAL_PROTECTION DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 7)

/* /> Query ICT info: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.u4OperCode = DIVXDRM_QUERY_ICT */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: &u1Ict, size=sizeof(__u8) */
#define DIVXDRM_QUERY_ICT		DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 8)

/* /> Set Random Sample Count: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.u4OperCode = DIVXDRM_SET_RANDOM_SAMPLE */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.pvOperParam = &u4RndCnt, */
/* /> rOperParam.u4OperParamSz = sizeof(__u32) */
/* /> Output Param: NULL */
#define DIVXDRM_SET_RANDOM_SAMPLE		DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 9)

/* /> Get Random Sample Counter: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.u4OperCode = DIVXDRM_SET_RANDOM_SAMPLE */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: &u4RndCnt, size=sizeof(__u32) */
#define DIVXDRM_GET_RANDOM_SAMPLE_CNTER  DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 10)

/* /> Get Registation Code String: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.u4OperCode = DIVXDRM_GET_REG_CODE_STR */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: &rCodeStr, size=sizeof(rCodeStr) */
/* /> rCodeString.szCodeStr = szRegCodeStr */
/* /> rCodeString.u4CodeStrSz = szRegCodeStr buffer's size */
#define DIVXDRM_GET_REG_CODE_STR				 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 11)

/* /> Get Deactivation Code String: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.u4OperCode = DIVXDRM_GET_DEACT_CODE_STR */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: &rCodeStr, size=sizeof(rCodeStr) */
/* /> rCodeString.szCodeStr = szDeactCodeStr */
/* /> rCodeString.u4CodeStrSz = szDeactCodeStr buffer's size */
#define DIVXDRM_GET_DEACT_CODE_STR			 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 12)

/* /> Get Activation Message: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.u4OperCode = DIVXDRM_GET_ACT_MESSAGE */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: &rActiveMsg, size=sizeof(rActiveMsg) */
/* /> rActiveMsg.szMsgStr = szMsgStr */
/* /> rActiveMsg.u4MsgStr = szMsgStr buffer's size */
#define DIVXDRM_GET_ACT_MESSAGE				 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 13)

/* /> Get Deactivation Message: */
/* /> Input Param Lists as follow: */
/* /> rOperParam.u4OperCode = DIVXDRM_GET_DEACT_MESSAGE */
/* /> rOperParam.pvInst = pvInst, i.e. obatined by DMX_IOCTL_DECRYPT_CREATE_INST */
/* /> rOperParam.pvOperParam = NULL, */
/* /> rOperParam.u4OperParamSz = 0 */
/* /> Output Param: &rDeacttiveMsg, size=sizeof(rDeacttiveMsg) */
/* /> rDeacttiveMsg.szMsgStr = szMsgStr */
/* /> rDeacttiveMsg.u4MsgStr = szMsgStr buffer's size */
#define DIVXDRM_GET_DEACT_MESSAGE			 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 14)

/* /> Demuxer Internal Use */
/* /> Check Whether DivxDRM has been activated or not: */
/* /> Input Param Lists as follow: */
/* /> rDecryptData.pu1FrameData = pu1EncryptData */
/* /> rDecryptData.u4FrameDataSz = u4EncryptDataSz */
/* /> rDecryptData.pu1DrmInfo = pu1DrmInfo */
/* /> InputParamSz: sizeof(DECRYPT_DIVXDRM_DECRYPT_DATA_T) */
/* /> Output Param: NULL */
#define DIVXDRM_DECRYPT_VIDEO					 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 15)

/* /> Demuxer Internal Use */
/* /> Check Whether DivxDRM has been activated or not: */
/* /> Input Param Lists as follow: */
/* /> rDecryptData.pu1FrameData = pu1EncryptData */
/* /> rDecryptData.u4FrameDataSz = u4EncryptDataSz */
/* /> rDecryptData.pu1DrmInfo = NULL */
/* /> InputParamSz: sizeof(DECRYPT_DIVXDRM_DECRYPT_DATA_T) */
/* /> Output Param: NULL */
#define DIVXDRM_DECRYPT_AUDIO					 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 16)

#define DIVXDRM_GET_VIDEO_KEYINFO			 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 17)

#define DIVXDRM_GET_AUDIO_KEYINFO			 DECRYPT_OPERCODE_DEF(DECRYPT_DIVXDRM, 18)

/* ///////////////////////////////////////////////////////////////////////////// */
/* Operaton Input/Output Param Definitions  */
/* ///////////////////////////////////////////////////////////////////////////// */
/* /> DIVXDRM_INIT_PLAYBACK pvOperParam */
typedef struct {
	__u8 *pu1DrmHdrData;	/* [IN] DRM Header Buffer */
	__u32 u4DrmHdrSz;	/* [IN] DRM Header Buffer's Size */
} DECRYPT_DIVXDRM_HEADER_T;

/* /> DIVXDRM_QUERY_RENTAL_STATUS's Output Param */
typedef struct {
	/* [out] rentalMessageFlag - Returned TRUE or FALSE */
	/* if the rental number of views left screen needs to be displayed. */
	__u8 u1RentalMsgFlag;
	/* [out] useLimit - Returned the number of playbacks allowed for the file. */
	__u8 u1UseLimitCnt;
	/* [out] useCount - Returned the number of time the file has been played back. */
	__u8 u1UseCnt;
} DECRYPT_DIVXDRM_RENTAL_STATUS_T;

/* /> DIVXDRM_GET_XXX_CODE_STR's Output Param */
typedef struct {
	char *szCodeStr;	/* [out] the string's buffer should be allocated by caller */
	__u32 u4CodeStrSz;	/* [out] the string's buffer Size */
} DECRYPT_DIVXDRM_CODE_STRING_T;

/* /> DIVXDRM_GET_XXX_MSG's Output Param */
typedef struct {
	char *szMsgStr;	/* [out] the string's buffer should be allocated by caller */
	__u32 u4MsgStrSz;	/* [out] the string's buffer Size */
} DECRYPT_DIVXDRM_MSG_T;

/* /> DIVXDRM_GET_ACT_STATUS's Output Param */
typedef struct {
	__u8 *pu1UsrId;	/* [out] the UsrID buffer should be allocated by caller */
	__u32 u4UsrIdSz;	/* [out] the UsrID Buffer's size, when out, it is real UsrID's len */
} DECRYPT_DIVXDRM_ACT_STATUS_T;

/* /> DIVXDRM_GET_VERSION's Output Param */
typedef struct {
	char *szOfficialName;	/* [out] OfficialName buffer should be allocated by caller */
	__u32 u4OfficialNameSz;	/* [out] OfficialName Buffer Size */
	char *szCodeName;	/* [out] CodeName buffer should be allocated by caller */
	__u32 u4CodeNameSz;	/* [out] CodeName Buffer Size */
	__u32 u4Major;		/* [out] Major */
	__u32 u4Minor;		/* [out] Minor */
	__u32 u4Fix;		/* [out] Fix */
	__u32 u4Build;		/* [out] Build */
} DECRYPT_DIVXDRM_VERSION_INFO_T;

/* /> DIVXDRM_GET_VERSION's Output Param */
typedef struct {
	__u8 *pu1FrameData;
	__u32 u4FrameDataSz;
	__u8 *pu1DrmInfo;
} DECRYPT_DIVXDRM_DECRYPT_DATA_T;

/* /> DIVXDRM_GET_VIDEO_KEYINFO's Output Param */
typedef struct {
	__u16 u2KeyIndex;
	__u8 *pu1InitVector;
	__u8 *pu1Key;
	__u16 u2KeyLen;
	bool fgCBC;
} DECRYPT_DIVXDRM_VIDEO_KEYINFO_T;

/* /> DIVXDRM_GET_AUDIO_KEYINFO's Output Param */
typedef struct {
	__u8 u1ProtectOffset;
	__u8 u1ProtectSize;
	__u8 *pu1InitVector;
	__u8 *pu1Key;
	__u16 u2KeyLen;
	bool  fgCBC;
} DECRYPT_DIVXDRM_AUDIO_KEYINFO_T;

#endif				/* DMX_SUPPORT_DIVXDRM */

#endif				/* #ifndef _DMX_DECRYPT_H_ */
