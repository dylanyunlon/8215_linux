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

#ifndef _X_BSP_H_
#define _X_BSP_H_

#include "x_typedef.h"
#include "x_ioopt.h"
#ifdef  __cplusplus
extern "C" {
#endif

//
//extern UINT32 BDP_GetBootLoaderVersion(void);
extern CHAR* BSP_GetIcSubVersion(void);
extern BOOL BSP_GetIcFunc(UINT32	u4FuncID);
extern CHAR* BSP_GetIcSubVersion4L(void);

#define PROT_DivXHD						0
#define PROT_DivXUltra				1
#define PROT_DivXHT						2
#define PROT_SACD							3
#define PROT_DolbyTrueHD			4
#define PROT_DTSHD						5
#define PROT_DLNA							6
#define PROT_DolbyDDCO				7
#define PROT_DivXPlus					8
#define PROT_AACS							9
#define PROT_DolbyHeadphone		10
#define PROT_DolbyPLIiIixIiz	11
#define PROT_DolbyEX					12
#define PROT_DolbyDigitalPlus	13
#define PROT_DolbyVSpeaker		14
#define PROT_DTSSurdSensation	15
#define PROT_Multichannel			16
#define PROT_DVDAudio					17
#define PROT_Playready				18
#define PROT_RM								19
#define PROT_WMDRM						20
#define PROT_CinemaNow				21
#define PROT_Netflix					22
#define PROT_Macrovision			23
#define PROT_YahooWidget			24
#define PROT_Browser					25
#define PROT_AVCHD						26
#define PROT_AdobeFlashLite		27
#define PROT_Rhapsody					28
#define PROT_NTFS							29
#define PROT_2ndARM						30
#define PROT_3DVideo					31
#define PROT_DolbyAAC					32
#define PROT_WMV9   					33
#define PROT_MAX_NUM					34

#define Func_DivXHD()     BSP_GetIcFunc(PROT_DivXHD)

#ifdef  __cplusplus
}
#endif

extern void BSP_CleanDCacheRange(UINT32 u4Start, UINT32 u4Len);
extern void BSP_InvDCacheRange(UINT32 u4Start, UINT32 u4Len);

#endif  // _X_BSP_H_
