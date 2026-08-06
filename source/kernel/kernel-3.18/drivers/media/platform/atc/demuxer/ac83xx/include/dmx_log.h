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
 * @file dmx_log.h
 *
 * @par Project
 *
 * @par Description
 *    Demuxer main macros definitions
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_LOG_H_FILE
#define DMX_LOG_H_FILE

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>

#include "dmx_def.h"

#define DMX_DEBUG

typedef enum _DmxDebugLevel {
	DMX_LOG_DEBUG,
	DMX_LOG_TRACE,
	DMX_LOG_INFO,
	DMX_LOG_ERROR,
	DMX_MAX_LOG_LEVEL
} E_DMX_LOG_LEVEL_T;

typedef enum _DmxModule {
	DMX_MOD_CFA_AUDIO = 0, DMX_MOD_CFA_APE,
	DMX_MOD_CFA_RM,
	DMX_MOD_CFA_MPG, DMX_MOD_CFA_AVI,
	DMX_MOD_CFA_MP4 = 5, DMX_MOD_CFA_ASF,
	DMX_MOD_CFA_OGM, DMX_MOD_CFA_MKV,
	DMX_MOD_CFA_FLV, DMX_MOD_CFA_TS = 10,
	DMX_MOD_CFA_SUB, DMX_MOD_CFA_AUDIN,
	DMX_MOD_HW = 16, DMX_MOD_RSP, DMX_MOD_GAU,
	DMX_MOD_ESM, DMX_MOD_INST, DMX_MOD_MEM,
	DMX_MOD_OTH, MAX_CNT_OF_DMX_MOD
} E_DMX_MOD_T;

#define MAX_SUBLVL_OF_DMX_MODULE   32
#define DMX_DEFAULT_MOD_LOGLEVEL   (-1)

#define DMX_MOD_OTH_LOGLVL_DEFAULT      ((u32)1 << 0)
#define DMX_MOD_OTH_LOGLVL_CTRLFLOW     ((u32)1 << 1)
#define DMX_MOD_OTH_LOGLVL_FFRW         ((u32)1 << 2)
#define DMX_MOD_OTH_LOGLVL_STM          ((u32)1 << 3)
#define DMX_MOD_OTH_LOGLVL_INSTBS  			((u32)1 << 4)
#define DMX_MOD_OTH_LOGLVL_SYNCPBBUF    ((u32)1 << 5)
#define DMX_MOD_OTH_LOGLVL_DECRYPT      ((u32)1 << 6)
#define DMX_MOD_OTH_LOGLVL_ESM          ((u32)1 << 7)
#define DMX_MOD_OTH_LOGLVL_DMABUF_V     ((u32)1 << 8)
#define DMX_MOD_OTH_LOGLVL_DMABUF_A     ((u32)1 << 9)
#define DMX_MOD_OTH_LOGLVL_DMAPBBUF_V   ((u32)1 << 11)
#define DMX_MOD_OTH_LOGLVL_DMAPBBUF_A   ((u32)1 << 12)
#define DMX_MOD_OTH_LOGLVL_COMPAU_V  ((u32)1 << 14)
#define DMX_MOD_OTH_LOGLVL_COMPAU_A  ((u32)1 << 15)
#define DMX_MOD_OTH_LOGLVL_COMPAU_SP ((u32)1 << 16)
#define DMX_MOD_OTH_LOGLVL_COMPAU_SEC ((u32)1 << 17)
#define DMX_MOD_OTH_LOGLVL_CMDQ_V        ((u32)1 << 18)
#define DMX_MOD_OTH_LOGLVL_CMDQ_A        ((u32)1 << 19)
#define DMX_MOD_OTH_LOGLVL_FIFOFULL      ((u32)1 << 27)
#define DMX_MOD_OTH_LOGLVL_PFM		     ((u32)1 << 31)

#define DMX_MOD_RSP_LOGLVL_DEFAULT       ((u32)1 << 0)
#define DMX_MOD_RSP_LOGLVL_LOGTX         ((u32)1 << 1)
#define DMX_MOD_RSP_LOGLVL_RSPTX         ((u32)1 << 2)
#define DMX_MOD_RSP_LOGLVL_REBUF         ((u32)1 << 3)
#define DMX_MOD_RSP_LOGLVL_RSPOFF        ((u32)1 << 4)

#define DMX_MOD_GAU_LOGLVL_DEFAULT      ((u32)1 << 0)
#define DMX_MOD_GAU_LOGLVL_GETAU        ((u32)1 << 1)
#define DMX_MOD_GAU_LOGLVL_GETAU_V      ((u32)1 << 2)
#define DMX_MOD_GAU_LOGLVL_GETAU_A      ((u32)1 << 3)
#define DMX_MOD_GAU_LOGLVL_GETAU_SP     ((u32)1 << 4)
#define DMX_MOD_GAU_LOGLVL_GETAU_SEC    ((u32)1 << 5)
#define DMX_MOD_GAU_LOGLVL_RELAU_ALL    ((u32)1 << 6)
#define DMX_MOD_GAU_LOGLVL_RELAU        ((u32)1 << 7)
#define DMX_MOD_GAU_LOGLVL_RELAU_V      ((u32)1 << 8)
#define DMX_MOD_GAU_LOGLVL_RELAU_A      ((u32)1 << 9)
#define DMX_MOD_GAU_LOGLVL_RELAU_SP     ((u32)1 << 10)
#define DMX_MOD_GAU_LOGLVL_RELAU_SEC    ((u32)1 << 11)
#define DMX_MOD_GAU_LOGLVL_THRESHOLD    ((u32)1 << 12)

#define DMX_MOD_HW_LOGLVL_DEFAULT       ((u32)1 << 0)
#define DMX_MOD_HW_LOGLVL_INIT          ((u32)1 << 1)
#define DMX_MOD_HW_LOGLVL_UNINIT        ((u32)1 << 2)
#define DMX_MOD_HW_LOGLVL_INSTBS   ((u32)1 << 3)
#define DMX_MOD_HW_LOGLVL_DMA      ((u32)1 << 4)
#define DMX_MOD_HW_LOGLVL_ISR_V    ((u32)1 << 5)
#define DMX_MOD_HW_LOGLVL_ISR_A    ((u32)1 << 6)
#define DMX_MOD_HW_LOGLVL_ISR_SP   ((u32)1 << 7)
#define DMX_MOD_HW_LOGLVL_ISR_SEC  ((u32)1 << 8)
#define DMX_MOD_HW_LOGLVL_ISR_OTH       ((u32)1 << 9)

#define DMX_MOD_INST_LOGLVL_DEFAULT     ((u32)1 << 0)
#define DMX_MOD_MEM_LOGLVL_DEFAULT      ((u32)1 << 0)

#define DMX_MAX_STM_LEN 256

extern u32 g_au4DmxLogMan[DMX_MAX_LOG_LEVEL][MAX_CNT_OF_DMX_MOD];

EXTERN void DmxComposeLog(E_DMX_LOG_LEVEL_T eLogLvl,
	const char *wsFmt, ...);

#define DmxLogE(eModule, u4ModLogLvl, wsFmt, arg...)  \
do { \
	if (0 == (g_au4DmxLogMan[DMX_LOG_ERROR][eModule] & (u4ModLogLvl))) \
		break; \
	DmxComposeLog(DMX_LOG_ERROR, "[%s:%s:%d]"wsFmt,FILE_ONLY,__FUNCTION__,__LINE__, ##arg); \
} while (0)

#define DmxLogI(eModule, u4ModLogLvl, wsFmt, ...) \
do { \
	if (0 == (g_au4DmxLogMan[DMX_LOG_INFO][eModule] & (u4ModLogLvl))) \
		break; \
	DmxComposeLog(DMX_LOG_INFO, (wsFmt), ##__VA_ARGS__); \
} while (0)

#define DmxLogT(eModule, u4ModLogLvl, wsFmt, ...) \
do { \
	if (0 == (g_au4DmxLogMan[DMX_LOG_TRACE][eModule] & (u4ModLogLvl))) \
		break; \
	DmxComposeLog(DMX_LOG_INFO, (wsFmt), ##__VA_ARGS__); \
} while (0)

#define DmxLogD(eModule, u4ModLogLvl, wsFmt, ...) \
do { \
	if (0 == (g_au4DmxLogMan[DMX_LOG_DEBUG][eModule] & (u4ModLogLvl))) \
		break; \
	DmxComposeLog(DMX_LOG_DEBUG, (wsFmt), ##__VA_ARGS__); \
} while (0)

#define DMXLOG_DEBUG(wsFmt, ...)  \
do { \
	if (0 == (g_au4DmxLogMan[DMX_LOG_DEBUG][DMX_MOD_OTH] & DMX_MOD_OTH_LOGLVL_DEFAULT)) \
		break; \
	DmxComposeLog(DMX_LOG_DEBUG, (wsFmt), ##__VA_ARGS__); \
} while (0)

#define DMXLOG_WARN(wsFmt, ...)  \
do { \
	if (0 == (g_au4DmxLogMan[DMX_LOG_DEBUG][DMX_MOD_OTH] & DMX_MOD_OTH_LOGLVL_DEFAULT)) \
		break; \
	DmxComposeLog(DMX_LOG_DEBUG, (wsFmt), ##__VA_ARGS__); \
} while (0)

#define DMXLOG_TRACE(wsFmt, ...) \
do { \
	if (0 == (g_au4DmxLogMan[DMX_LOG_TRACE][DMX_MOD_OTH] & DMX_MOD_OTH_LOGLVL_DEFAULT)) \
		break; \
	DmxComposeLog(DMX_LOG_TRACE, (wsFmt), ##__VA_ARGS__); \
} while (0)

#define DMXLOG_ERROR(wsFmt, arg...) \
do { \
	if (0 == (g_au4DmxLogMan[DMX_LOG_ERROR][DMX_MOD_OTH] & DMX_MOD_OTH_LOGLVL_DEFAULT)) \
		break; \
	DmxComposeLog(DMX_LOG_ERROR, "[%s:%s:%d]"wsFmt,FILE_ONLY,__FUNCTION__,__LINE__, ##arg); \
} while (0)

#define DMXLOG_FATAL(wsFmt, arg...) \
do { \
	if (0 == (g_au4DmxLogMan[DMX_LOG_ERROR][DMX_MOD_OTH] & DMX_MOD_OTH_LOGLVL_DEFAULT)) \
		break; \
	DmxComposeLog(DMX_LOG_ERROR, "[%s:%s:%d]"wsFmt,FILE_ONLY,__FUNCTION__,__LINE__, ##arg); \
} while (0)

EXTERN void DmxLogEnable(bool fgEnable, u32 u4LogLvl, u32 u4Module,
	u32 u4ModLogLvlMask);
EXTERN void DmxLogDEnable(bool fgEnable, u32 u4Module, u32 u4ModLogLvlMask);
EXTERN void DmxInitLog(void);
EXTERN void DmxDeInitLog(void);

#endif	/* DMX_LOG_H_FILE */
