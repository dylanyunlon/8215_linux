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
 * @file dmx_decrypt.h
 *
 * @par Project
 *
 * @par Description
 *	  Decrypt Interface definations
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

#ifndef __linux__
#pragma warning(push)
#pragma warning(disable : 4115) /*disable warning C4115: named type definition in parentheses*/
#endif

#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_decrypt.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_decrypt.h"
#include "mm_debug.h"
#endif /*__linux__*/

#include "dmx_def.h"
#include "dmx_cpsa.h"
#if DMX_SUPPORT_DIVXDRM
#include "dmx_DivxDRM.h"
#endif  /*#if DMX_SUPPORT_DIVXDRM*/

#ifndef __linux__
#pragma warning(pop)
#endif

MRESULT DmxInitDecryptModule(E_DECRYPT_TYPE_T eDecryptType)
{
	MRESULT mrRet = RET_DMX_OK;

	switch (eDecryptType) {
	case DECRYPT_NONE:
		break;
#if DMX_SUPPORT_DIVXDRM
	case DECRYPT_DIVXDRM:
		{
			mrRet = DivxDRMInitialize();
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT("[DECRYPT] %s line %d failed in initialize drm sdk, mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;
#endif /*DMX_SUPPORT_DIVXDRM*/
	default:
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- unsupport decrypt type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT DmxDeInitDecryptModule(E_DECRYPT_TYPE_T eDecryptType)
{
	MRESULT mrRet = RET_DMX_OK;

	switch (eDecryptType) {
	case DECRYPT_NONE:
		break;
#if DMX_SUPPORT_DIVXDRM
	case DECRYPT_DIVXDRM:
		mrRet = DivxDRMDeInitialize();
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d fail in deinitialize drm sdk, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}
		break;
#endif /*DMX_SUPPORT_DIVXDRM*/
	default:
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- unsupport decrypt type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT DmxInitDecrypt(void)
{
	u32	u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT] %s enter\r\n"),
		DMX_FUNC_NAME);

	for (u4Idx = DECRYPT_NONE; u4Idx < MAX_CNT_OF_DECRYPT_TYPE; u4Idx++) {
		mrRet = DmxInitDecryptModule((E_DECRYPT_TYPE_T)u4Idx);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT]%s fail in initialize decrypt module(decrypt type: %d)\r\n"),
				DMX_FUNC_NAME, u4Idx);
		}
	}

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT] %s success\r\n"),
		DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DmxDeInitDecrypt(void)
{
	E_DECRYPT_TYPE_T eDecryptType = DECRYPT_NONE;
	u32	u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	for (u4Idx = 0; u4Idx < MAX_CNT_OF_DECRYPT_TYPE; u4Idx++) {
		eDecryptType = (E_DECRYPT_TYPE_T)u4Idx;
		mrRet = DmxDeInitDecryptModule(eDecryptType);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s fail in initialize decrypt module(eType; %d), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, eDecryptType, mrRet);
			MM_RETURN(mrRet);
		}
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT DmxDecryptCreateInst(
	E_DECRYPT_TYPE_T eDecryptType,
	void			 **ppvInst)
{
	MRESULT  mrRet = RET_DMX_OK;

	if ((MAX_CNT_OF_DECRYPT_TYPE <= eDecryptType) ||
		(NULL == ppvInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for invalid args, decrypt type(%d), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType, RET_DMX_PARAM_WRONG);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eDecryptType) {
	case DECRYPT_NONE:
		break;
#if DMX_SUPPORT_DIVXDRM
	case DECRYPT_DIVXDRM:
		mrRet = DivxDRMCreateInst(ppvInst);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d fail in create DivXDrm decryption")
				TEXT(" instance, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}
		break;
#endif /*DMX_SUPPORT_DIVXDRM*/
	default:
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- unsupport decrypt type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT DmxDecryptReleaseInst(
	E_DECRYPT_TYPE_T eDecryptType,
	void			 *pvInst)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((MAX_CNT_OF_DECRYPT_TYPE <= eDecryptType) ||
		(NULL == pvInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for invalid args, decrypt")
			TEXT(" type(%d), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType, RET_DMX_PARAM_WRONG);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eDecryptType) {
	case DECRYPT_NONE:
		break;
#if DMX_SUPPORT_DIVXDRM
	case DECRYPT_DIVXDRM:
		mrRet = DivxDRMReleaseInst(pvInst);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d fail in release DivXDrm decryption")
				TEXT("instance, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}
		break;
#endif /*DMX_SUPPORT_DIVXDRM*/
	default:
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- unsupport decrypt type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT DmxDecryptExecCmd(
	void					*pvInst,
	u32					 u4OperCode,
	void					*pvBufIn,
	u32					 u4LenIn,
	void					*pvBufOut,
	u32					 u4LenOut)
{
	MRESULT mrRet          = RET_DMX_OK;
	E_DECRYPT_TYPE_T eType = (E_DECRYPT_TYPE_T)DECRYPT_GETTYPE_FROM_OPERCODE(u4OperCode);

	if (MAX_CNT_OF_DECRYPT_TYPE <= eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for invalid decrypt type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eType) {
	case DECRYPT_NONE:
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- Decrypt Type is DECRYPT_NONE\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		mrRet = RET_DMX_PARAM_WRONG;
		break;
#if DMX_SUPPORT_DIVXDRM
	case DECRYPT_DIVXDRM:
		mrRet = DivxDRMExecCmd(pvInst, u4OperCode, pvBufIn, u4LenIn,
			pvBufOut, u4LenOut);
		break;
#endif /*DMX_SUPPORT_DIVXDRM*/
	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- unsupport Decrypt Type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eType);
		mrRet = RET_DMX_NO_IMPLEMENT;
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT DmxDecryptGetInstLastError(
	E_DECRYPT_TYPE_T eDecryptType,
	void			 *pvInst)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((MAX_CNT_OF_DECRYPT_TYPE <= eDecryptType) ||
		(NULL == pvInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for invalid args,")
			TEXT(" Decrypt Type(%d), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType, RET_DMX_PARAM_WRONG);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eDecryptType) {
	case DECRYPT_NONE:
		break;
#if DMX_SUPPORT_DIVXDRM
	case DECRYPT_DIVXDRM:
		mrRet = DivxDRMGetInstLastError(pvInst);
		break;
#endif /*DMX_SUPPORT_DIVXDRM*/
	default:
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- unsupport Decrypt Type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT DmxDecryptGetPlayInst(
	E_DECRYPT_TYPE_T eDecryptType,
	s32			 i4DecryptId,
	void			 **ppvInst)
{
	MRESULT  mrRet = RET_DMX_OK;

	if ((MAX_CNT_OF_DECRYPT_TYPE <= eDecryptType) ||
		(NULL == ppvInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for invalid args, decrypt Type(%d)")
			TEXT("decrypt id(%d), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType, i4DecryptId,
			RET_DMX_PARAM_WRONG);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eDecryptType) {
	case DECRYPT_NONE:
		break;
#if DMX_SUPPORT_DIVXDRM
	case DECRYPT_DIVXDRM:
		mrRet = DivxDRMGetPlayInst(i4DecryptId, ppvInst);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d fail in get Decrypt instance whose")
				TEXT("decrypt id is %d and is in playing status, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, i4DecryptId, mrRet);
			MM_RETURN(mrRet);
		}
		break;
#endif /*DMX_SUPPORT_DIVXDRM*/
	default:
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- unsupport DecryptType(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT DmxDecryptRelPlayInst(
	E_DECRYPT_TYPE_T eDecryptType,
	void			 *pvInst)
{
	MRESULT  mrRet = RET_DMX_OK;

	if ((MAX_CNT_OF_DECRYPT_TYPE <= eDecryptType) ||
		(NULL == pvInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for invalid args, Decrypt Type(%d),")
			TEXT("mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType, RET_DMX_PARAM_WRONG);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eDecryptType) {
	case DECRYPT_NONE:
		break;
#if DMX_SUPPORT_DIVXDRM
	case DECRYPT_DIVXDRM:
		mrRet = DivxDRMRelPlayInst(pvInst);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d fail in release Decrypt instance whose")
				TEXT("handle is 0x%x, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvInst, mrRet);
			MM_RETURN(mrRet);
		}
		break;
#endif /*DMX_SUPPORT_DIVXDRM*/
	default:
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- unsupport Decrypt Type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT DmxDecryptGetActStatus(
	E_DECRYPT_TYPE_T eDecryptType,
	void			 *pvStatus,
	u32			 u4StatusSz)
{
	MRESULT  mrRet = RET_DMX_OK;

	if ((MAX_CNT_OF_DECRYPT_TYPE <= eDecryptType) ||
		(NULL == pvStatus)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for invalid args, Decrypt Type(%d),")
			TEXT("mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType, RET_DMX_PARAM_WRONG);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eDecryptType) {
	case DECRYPT_NONE:
		break;
#if DMX_SUPPORT_DIVXDRM
	case DECRYPT_DIVXDRM:
		mrRet = DivxDRMGetActivationStatus(pvStatus, u4StatusSz);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d fail in get divxdrm activation ")
				TEXT("status, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}
		break;
#endif /*DMX_SUPPORT_DIVXDRM*/
	default:
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- unsupport Decrypt Type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT DmxDecryptIsDevActived(
	E_DECRYPT_TYPE_T eDecryptType,
	bool			 *pfgActived)
{
	MRESULT  mrRet = RET_DMX_OK;

	if ((MAX_CNT_OF_DECRYPT_TYPE <= eDecryptType) ||
		(NULL == pfgActived)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for invalid args, Decrypt Type(%d), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType, RET_DMX_PARAM_WRONG);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eDecryptType) {
	case DECRYPT_NONE:
		break;
#if DMX_SUPPORT_DIVXDRM
	case DECRYPT_DIVXDRM:
		*pfgActived = DivxDRMIsDeviceActived();
		break;
#endif /*DMX_SUPPORT_DIVXDRM*/
	default:
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- unsupport Decrypt Type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT DmxDecryptGetVersion(
	E_DECRYPT_TYPE_T eDecryptType,
	void			 *pvVersionInfo,
	u32			 u4VersionInfoSz)
{
	MRESULT  mrRet = RET_DMX_OK;

	if ((MAX_CNT_OF_DECRYPT_TYPE <= eDecryptType) ||
		(NULL == pvVersionInfo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for invalid args, Decrypt Type(%d), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType, RET_DMX_PARAM_WRONG);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eDecryptType) {
	case DECRYPT_NONE:
		break;
#if DMX_SUPPORT_DIVXDRM
	case DECRYPT_DIVXDRM:
		mrRet = DivxDRMGetVersion(pvVersionInfo, u4VersionInfoSz);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d fail in get DivXDRM sdk version, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}
		break;
#endif /*DMX_SUPPORT_DIVXDRM*/
	default:
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- unsupport Decrypt Type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);
		break;
	}

	MM_RETURN(mrRet);
}

MRESULT DmxDecryptInitMemory(E_DECRYPT_TYPE_T eDecryptType)
{
	MRESULT  mrRet = RET_DMX_OK;

	if (MAX_CNT_OF_DECRYPT_TYPE <= eDecryptType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for invalid args, Decrypt Type(%d), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType, RET_DMX_PARAM_WRONG);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eDecryptType) {
	case DECRYPT_NONE:
		break;
#if DMX_SUPPORT_DIVXDRM
	case DECRYPT_DIVXDRM:
		mrRet = DivxDRMInitMemory();
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d fail in initialize DivXDRM Memory, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}
		break;
#endif /*DMX_SUPPORT_DIVXDRM*/
	default:
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- unsupport Decrypt Type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);
		break;
	}

	MM_RETURN(mrRet);
}

