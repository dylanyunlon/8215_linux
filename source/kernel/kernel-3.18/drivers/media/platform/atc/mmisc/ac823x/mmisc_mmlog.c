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

#include "linux/module.h"
#include <linux/kernel.h>
#include <media/atc/mm_debug.h>
#include "types.h"
#include "windows.h"

bool LOG_ModInit(void)
{
	return TRUE;
}
EXPORT_SYMBOL(LOG_ModInit);

void LOG_ModDeinit(void)
{
}
EXPORT_SYMBOL(LOG_ModDeinit);

#define MAX_LEN 256
void vNormalize(LOG_MOD_ID_T tModId, u32 u4LogLvl, const char *wsFmt, va_list vl)
{
	int len = 0;
	char buf[MAX_LEN] = { 0 };

	switch (u4LogLvl) {
	case MM_LOG_DEBUG:
		strcpy(buf, "[D]");
		break;
	case MM_LOG_TRACE:
		strcpy(buf, "[T]");
		break;
	case MM_LOG_ERROR:
		strcpy(buf, "[E]");
		break;
	default:
		strcpy(buf, "[D]");
		break;
	}

	switch (tModId) {
	case LOG_MOD_DMX:
		strcpy(buf + strlen(buf), "[MM][DMX]:");
		break;
	case LOG_MOD_VDEC:
		strcpy(buf + strlen(buf), "[MM][VDEC]:");
		break;
	case LOG_MOD_DVP:
		strcpy(buf + strlen(buf), "[MM][DVP]:");
		break;
	case LOG_MOD_WCM:
		strcpy(buf + strlen(buf), "[MM][WCM]:");
		break;
	case LOG_MOD_MSDKCORE:
		strcpy(buf + strlen(buf), "[MM][MSDKCORE]:");
		break;
	default:
		strcpy(buf + strlen(buf), "[MM][Unknown]:");
		break;
	}

	len = MAX_LEN - 1 - strlen(buf);
	if (len > 0)
		vsnprintf(buf + strlen(buf), len, wsFmt, vl);

	switch (u4LogLvl) {
	case MM_LOG_DEBUG:
		pr_debug("%s", buf);
		break;
	case MM_LOG_TRACE:
		pr_debug("%s", buf);
		break;
	case MM_LOG_ERROR:
		pr_err("%s", buf);
		break;
	default:
		pr_debug("%s", buf);
		break;
	}
}

void LOG_ModWmsgD(LOG_MOD_ID_T tModId, const char *wsFmt, ...)
{
	va_list vl;

	va_start(vl, wsFmt);

	vNormalize(tModId, MM_LOG_DEBUG, wsFmt, vl);
	va_end(vl);
}
EXPORT_SYMBOL(LOG_ModWmsgD);

void LOG_ModWmsgE(LOG_MOD_ID_T tModId, const char *wsFmt, ...)
{
	va_list vl;

	va_start(vl, wsFmt);

	vNormalize(tModId, MM_LOG_ERROR, wsFmt, vl);
	va_end(vl);
}
EXPORT_SYMBOL(LOG_ModWmsgE);

void LOG_ModWmsgT(LOG_MOD_ID_T tModId, const char *wsFmt, ...)
{
	va_list vl;

	va_start(vl, wsFmt);

	vNormalize(tModId, MM_LOG_TRACE, wsFmt, vl);
	va_end(vl);
}
EXPORT_SYMBOL(LOG_ModWmsgT);
