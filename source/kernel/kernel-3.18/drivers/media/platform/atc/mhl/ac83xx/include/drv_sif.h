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

#ifndef _DRV_SIF_H_
#define _DRV_SIF_H_

#include "x_typedef.h"
#include "drv_config.h"
#include "chip_ver.h"
#include "drv_sif_sw.h"
#include "drv_sif_read_write.h"
/******************************************************************************
* PMX API
******************************************************************************/
extern INT32 SIF_Init(void);
#if (!CONFIG_DRV_LINUX)
extern INT32 SIF_Uninit(void);
#else
extern INT32 i4SIF_Uninit(UINT32 u4Case);
#endif
/* extern UINT32 SIF_RanAddr_Read(UINT32 u4ClkDiv, UCHAR ucDev, UINT32 u4Addr,
							SIF_BIT_T ucAddrType, UCHAR *pucValue, UINT32 u4Count);
extern UINT32 SIF_CurAddr_Read(UINT32 u4ClkDiv, UCHAR ucDev, UCHAR *pucValue, UINT32 u4Count);
extern UINT32 SIF_RanAddr_Write(UINT32 u4ClkDiv, UCHAR ucDev, UINT32 u4Addr,
							SIF_BIT_T ucAddrType, const UCHAR *pucValue, UINT32 u4Count); */

#if (CONFIG_DRV_DENON_SUPPORT && CONFIG_DRV_CUSTOM_CDN)
extern UINT32 SIF_RanAddr_ReadUPG(UINT32 u4ClkDiv, UCHAR ucDev, UCHAR *pucValue, UINT32 u4Count);
extern UINT32 SIF_RanAddr_WriteUPG(UINT32 u4ClkDiv, UCHAR ucDev, UCHAR ucCmd, UINT32 u4Addr, SIF_BIT_T ucAddrType,
								const UCHAR *pucValue, UINT32 u4Count);
#endif

extern UINT32 SIF_RanAddr_Read_CP20C(UINT32 u4ClkDiv, UCHAR ucDev, UINT32 u4Addr,
							SIF_BIT_T ucAddrType, UCHAR *pucValue, UINT32 u4Count);
extern UINT32 SIF_RanAddr_Write_CP20C(UINT32 u4ClkDiv, UCHAR ucDev, UINT32 u4Addr,
							SIF_BIT_T ucAddrType, const UCHAR *pucValue, UINT32 u4Count);

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8520)
extern void SIF_SlaveEnable(UINT8 ucDecAdr, UINT8 ucDataAddr);
extern UINT32 SIF1_RanAddr_Read(UINT32 u4ClkDiv, UCHAR ucDev, UINT32 u4Addr, UCHAR *pucValue, UINT32 u4Count);
extern UINT32 SIF1_CurAddr_Read(UINT32 u4ClkDiv, UCHAR ucDev, UCHAR *pucValue, UINT32 u4Count);
extern UINT32 SIF1_RanAddr_Write(UINT32 u4ClkDiv, UCHAR ucDev, UINT32 u4Addr, const UCHAR *pucValue, UINT32 u4Count);
extern void vEnableHDMIHardwareDDC(void);
#elif (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8530)
extern INT32 SIFS_Init(void);
extern INT32 SIFS_Uninit(void);
extern INT32 DDCM_Init(void);
extern INT32 DDCM_Uninit(void);
extern UINT32 DDCM_RanAddr_Read(UINT32 u4ClkDiv, UCHAR ucDev, UINT32 u4Addr,
								SIF_BIT_T ucAddrType, UCHAR *pucValue, UINT32 u4Count);
extern UINT32 DDCM_CurAddr_Read(UINT32 u4ClkDiv, UCHAR ucDev, UCHAR *pucValue, UINT32 u4Count);
extern UINT32 DDCM_RanAddr_Write(UINT32 u4ClkDiv, UCHAR ucDev, UINT32 u4Addr, SIF_BIT_T ucAddrType,
								const UCHAR *pucValue, UINT32 u4Count);
#endif
#endif /* _DRV_SIF_H_ */

