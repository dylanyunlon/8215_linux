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

#ifndef _DRV_SIF_SW_H_
#define _DRV_SIF_SW_H_

#include "x_typedef.h"

#include "drv_config.h"

#if (CONFIG_DRV_LINUX)
#include <linux/ioctl.h>
#endif
#include "drv_sif_read_write.h"

#include "drv_def.h"

#define SOFT_SIF
#if CONFIG_DRV_CUSTOM_0
#define SIF_USE_HW_MODE_ONLY 0
#else
#define SIF_USE_HW_MODE_ONLY 1
#endif

#if CONFIG_SUPPORT_SS
#ifdef SIF_USE_HW_MODE_ONLY
#undef SIF_USE_HW_MODE_ONLY
#endif
#define SIF_USE_HW_MODE_ONLY 0
#endif

/******************************************************************************
* Type definitions
******************************************************************************/
#if 0/* CONFIG_DRV_CUSTOM_KLG */
typedef enum {
	SIF_NORMAL,
	SIF_OTHER,
}   SIF_TYPE_T;

typedef enum {
	SIF_8_BIT,
	SIF_16_BIT,
}   SIF_BIT_T;

typedef struct _SIF_MODE_T {
	SIF_BIT_T  eBit;     /* 8 bit,16 bit */
	SIF_TYPE_T eType;    /* normal, other */
} SIF_MODE_T;
#endif

#if (CONFIG_DRV_LINUX)
#define SIF_IO_MAGIC   's'

#define CMD_SIF_SET_DATA  _IOW(SIF_IO_MAGIC, 1, int)
#define CMD_SIF_SET_CLOCK   _IOW(SIF_IO_MAGIC, 5, int) /* < [To set sif clock divider. 3M module clock], _SIF_IO_T */
#define CMD_SIF_GET_DATA  _IOR(SIF_IO_MAGIC, 2, int)
#define CMD_SIF_GET_DATA_ASYNC  _IOR(SIF_IO_MAGIC, 3, int)

#if CONFIG_SUPPORT_MCU_SIF
#define CMD_SIF_SET_DATA_MCU  _IOW(SIF_IO_MAGIC, 3, int)
#define CMD_SIF_GET_DATA_MCU  _IOR(SIF_IO_MAGIC, 4, int)
#endif

#define CMD_SIF_VIXS_UPG  _IOW(SIF_IO_MAGIC, 6, int)

#if 1 /* CONFIG_SUPPORT_WELTREND_MCU_UPG */
#define CMD_SIF_WT_START       _IOW(SIF_IO_MAGIC, 7, int)
#define CMD_SIF_WT_ERASE       _IOW(SIF_IO_MAGIC, 8, int)
#define CMD_SIF_WT_FINISH      _IOR(SIF_IO_MAGIC, 9, int)
#define CMD_SIF_WT_RESET       _IOR(SIF_IO_MAGIC, 10, int)
#define CMD_SIF_WT_WRITE       _IOR(SIF_IO_MAGIC, 11, int)
#define CMD_SIF_WT_READ        _IOR(SIF_IO_MAGIC, 12, int)
#define CMD_SIF_WT_HOLD        _IOW(SIF_IO_MAGIC, 13, int)
#define CMD_SIF_WT_FREE        _IOW(SIF_IO_MAGIC, 14, int)
#define CMD_SIF_WT_SET_H_ADDR  _IOW(SIF_IO_MAGIC, 15, int)
#define CMD_SIF_WT_PAGE_ERASE  _IOW(SIF_IO_MAGIC, 16,  int)
#define CMD_SIF_WT_TEST        _IOW(SIF_IO_MAGIC, 17, int)
#define CMD_APP_GET_DRV_INFO   _IOW(SIF_IO_MAGIC, 18, int)
#define CMD_SIF_DO_SPEC_VERIFY _IOW(SIF_IO_MAGIC, 19, int)  /* Support special Region Verify */
#define CMD_DARBEE_OS_WRITE    _IOW(SIF_IO_MAGIC, 20, int)
#endif


typedef enum {
	SIF_TYPE_NORMAL,
	SIF_TYPE_IPOD,
}   SIF_I2C_DEVICE_TYPE;


typedef struct _SIF_IO_T {
	BYTE bDevice;
	UINT16 u2Data_Addr;
	BYTE bDataCount;
	BYTE *prData;
	SIF_MODE_T rSifMode;
	SIF_I2C_DEVICE_TYPE rSifFunctype;
	unsigned long u4SifClkDivider;

} SIF_IO_T;

typedef struct _SIF_IO_CLKDIV_T {
	unsigned long u4SifClkDivider;       /* < [Device address of the slave.] */
} SIF_IO_CLKDIV_T;


typedef struct _SIF_VIXS_IO_T {
	BYTE bDevice;
	UINT16 u2Data_Addr;
	UINT32 u4DataCount;
	BYTE *prData;
	SIF_MODE_T rSifMode;
} SIF_VIXS_IO_T;


#endif

/******************************************************************************
* PMX API
******************************************************************************/
extern INT32 sif_get_stat(void);

extern INT32 SIF_SW_Init(void);
#if (CONFIG_DRV_LINUX)
extern INT32 i4SIF_SW_Uninit(UINT32 u4Case);
#else
extern INT32 SIF_SW_Uninit(void);
#endif
/* extern BOOL fgSIFDataRead(BYTE bDevice, UINT16 u2Data_Addr, BYTE bDataCount, BYTE *prData, SIF_MODE_T rSifMode);*/
/* extern BOOL fgSIFDataWrite(BYTE bDevice, UINT16 u2Data_Addr, BYTE bDataCount, BYTE *prData, SIF_MODE_T rSifMode);*/

extern BOOL fgSIFDataReadCP20C(BYTE bDevice, UINT16 u2Data_Addr, BYTE bDataCount, BYTE *prData, SIF_MODE_T rSifMode);
extern BOOL fgSIFDataWriteCP20C(BYTE bDevice, UINT16 u2Data_Addr, BYTE bDataCount, BYTE *prData, SIF_MODE_T rSifMode);

#if CONFIG_DRV_CUSTOM_JSN
extern BOOL fgSIFDataGeneralWrite(BYTE bDevice, UINT16 u2Data_Addr, BYTE bDataCount, BYTE *prData, SIF_MODE_T rSifMode);
extern BOOL fgSIFDataGeneralRead(BYTE bDevice, UINT16 u2Data_Addr, BYTE bDataCount, BYTE *prData, SIF_MODE_T rSifMode);
#endif
#if (CONFIG_SUPPORT_MCU_SIF)
extern BOOL fgSIFDataReadMCU(BYTE bDevice, UINT16 u2Data_Addr, BYTE bDataCount, BYTE *prData, SIF_MODE_T rSifMode);
extern BOOL fgSIFDataWriteMCU(BYTE bDevice, UINT16 u2Data_Addr, BYTE bDataCount, BYTE *prData, SIF_MODE_T rSifMode);
#endif
#if (CONFIG_DRV_DENON_SUPPORT && CONFIG_DRV_CUSTOM_CDN)
extern BOOL fgSIFDataReadUPG(BYTE bDevice, BYTE bDataCount, BYTE *prData, SIF_MODE_T rSifMode);
extern BOOL fgSIFDataWriteUPG(BYTE bDevice, BYTE bCmd, UINT16 u2Data_Addr, UINT16 u2DataCount,
								BYTE *prData, SIF_MODE_T rSifMode);
#endif
#endif /* _DRV_SIF_SW_H_ */

