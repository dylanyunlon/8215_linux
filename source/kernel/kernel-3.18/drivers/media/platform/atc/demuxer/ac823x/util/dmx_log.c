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
 * @file dmx_spt_cli.c
 *
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */
#ifdef __linux__
#define pr_fmt(fmt) "[MM]["KBUILD_MODNAME"]" fmt

#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/module.h>
#include "windows.h"
#include <media/atc/ose_mem.h>
#else
#include "OSE_mem.h"
#endif /* __linux__ */

#include "dmx_def.h"
#include "dmx_log.h"

#ifndef __linux__
#pragma warning(disable : 4127) /* disable warning C4127: conditional expression is constant */
#endif

const char *g_szDmxLogLvlName[DMX_MAX_LOG_LEVEL] = {
	TEXT("DEBUG"),
	TEXT("TRACE"),
	TEXT("INFO"),
	TEXT("ERROR")
};

u32 g_au4DmxLogMan[DMX_MAX_LOG_LEVEL][MAX_CNT_OF_DMX_MOD];


void DmxInitLog(void)
{
	mm_memset(g_au4DmxLogMan, 0, sizeof(g_au4DmxLogMan));
}

void DmxDeInitLog(void)
{
	mm_memset(g_au4DmxLogMan, 0, sizeof(g_au4DmxLogMan));
}

void DmxSetDefaultLogLvl(E_DMX_LOG_LEVEL_T eLogLvl)
{
	u32 u4LogLvlIdx = 0;

	for (u4LogLvlIdx = eLogLvl; u4LogLvlIdx < DMX_MAX_LOG_LEVEL; u4LogLvlIdx++) {
		u32 u4ModIdx = 0;

		for (u4ModIdx = 0; u4ModIdx < MAX_CNT_OF_DMX_MOD; u4ModIdx++)
			g_au4DmxLogMan[u4LogLvlIdx][u4ModIdx] = (u32)(-1);
	}
}

EXTERN void DmxComposeLog(
	E_DMX_LOG_LEVEL_T eLogLvl, const char *wsFmt, ...)
{
	char buf[DMX_MAX_STM_LEN];
	va_list vl;

	memset(buf, 0, sizeof(buf));
	va_start(vl, wsFmt);
	vsnprintf(buf, DMX_MAX_STM_LEN - 1, wsFmt, vl);
	va_end(vl);
	switch (eLogLvl) {
	case DMX_LOG_DEBUG:
		pr_debug("%s", buf);
		break;
	case DMX_LOG_INFO:
#ifdef DMX_DEBUG
		pr_info("%s", buf);
#else
		pr_debug("%s", buf);
#endif
		break;
	case DMX_LOG_TRACE:
#ifdef DMX_DEBUG
		pr_info("%s", buf);
#else
		pr_debug("%s", buf);
#endif
		break;
	case DMX_LOG_ERROR:
		pr_err("%s", buf);
		break;
	default:
		pr_debug("%s", buf);
		break;
	}
}

void DmxLogEnable(bool fgEnable, u32 u4LogLvl,
	u32 u4Module, u32 u4ModLogLvlMask)
{
	pr_debug(TEXT("[LOG] %s --> %s Log (Loglevel: %s, ")
		TEXT("Module Sub Log Level Mask: %d) for module(id: %d)\r\n"),
		DMX_FUNC_NAME, (fgEnable ? TEXT("Enable") : TEXT("Disable")),
		g_szDmxLogLvlName[u4LogLvl], (int)u4ModLogLvlMask, (int)u4Module);
	if ((u32)(-1) == u4Module) {
		u32 u4ModIdx = 0;

		for (u4ModIdx = 0; u4ModIdx < MAX_CNT_OF_DMX_MOD; u4ModIdx++) {
			if (fgEnable)
				g_au4DmxLogMan[u4LogLvl][u4ModIdx] |= u4ModLogLvlMask;
			else
				g_au4DmxLogMan[u4LogLvl][u4ModIdx] &= ~u4ModLogLvlMask;
		}
	} else if (u4Module < MAX_CNT_OF_DMX_MOD) {
		if (fgEnable)
			g_au4DmxLogMan[u4LogLvl][u4Module] |= u4ModLogLvlMask;
		else
			g_au4DmxLogMan[u4LogLvl][u4Module] &= ~u4ModLogLvlMask;
	} else {
		pr_err(TEXT("[LOG] %s fail for invalid param -- module id(%d) ")
			TEXT("exceed the limitation\r\n"),
			DMX_FUNC_NAME, (int)u4Module);
		return;
	}
}

void DmxLogDEnable(bool fgEnable, u32 u4Module, u32 u4ModLogLvlMask)
{
	pr_info(TEXT("[LOG] %s --> %s Log (Loglevel: %s, ")
		TEXT("Module Sub Log Level Mask: %d) for module(id: %d)\r\n"),
		DMX_FUNC_NAME, (fgEnable ? TEXT("Enable") : TEXT("Disable")),
		g_szDmxLogLvlName[DMX_LOG_DEBUG], (int)u4ModLogLvlMask, (int)u4Module);

	if ((u32)(-1) == u4Module) {
		u32 u4ModIdx = 0;

		for (u4ModIdx = 0; u4ModIdx < MAX_CNT_OF_DMX_MOD; u4ModIdx++) {
			if (fgEnable)
				g_au4DmxLogMan[DMX_LOG_DEBUG][u4ModIdx] |= u4ModLogLvlMask;
			else
				g_au4DmxLogMan[DMX_LOG_DEBUG][u4ModIdx] &= ~u4ModLogLvlMask;
		}
	} else if (u4Module < MAX_CNT_OF_DMX_MOD) {
		if (fgEnable)
			g_au4DmxLogMan[DMX_LOG_DEBUG][u4Module] |= u4ModLogLvlMask;
		else
			g_au4DmxLogMan[DMX_LOG_DEBUG][u4Module] &= ~u4ModLogLvlMask;
	} else {
		pr_err(TEXT("[LOG] %s fail for invalid param -- module id(%d) ")
			TEXT("exceed the limitation\r\n"),
			DMX_FUNC_NAME, (int)u4Module);
		return;
	}
}

