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

#ifdef __ARM2__

#include <x_types.h>
#include "arm2_comm_data_struct.h"
#include "tvd_log.h"


#define TVD_ARM2_EVENT_MAX_CNT  10
typedef void   *HANDLE;
typedef void (*x_os_isr_fct)(u16 vector_id);
ARM2_EVT_T g_rArm2EvtDB[TVD_ARM2_EVENT_MAX_CNT] = {0};
typedef enum {
	X_ISR_FLAG_SAMPLE_RANDOM = 0X40
} ISR_FLAG_T;

u32     g_Arm2EvtCurID = 0;
int _tcscmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;

	while (1) {
		c1 = *cs++;
		c2 = *ct++;

		if (c1 != c2) {
			return c1 < c2 ? -1 : 1;
		}

		if (!c1) {
			break;
		}
	}

	return 0;
}

HANDLE X_CreateEvent(const char *lpName)
{
	HANDLE handle = NULL;

	if (g_Arm2EvtCurID < TVD_ARM2_EVENT_MAX_CNT) {
		if (lpName) {
			u32 i = 0;

			for (; i < TVD_ARM2_EVENT_MAX_CNT; i++) {
				if (!_tcscmp(lpName, (g_rArm2EvtDB[i].szEvtName))) {
					break;
				}
			}

			if (i == TVD_ARM2_EVENT_MAX_CNT) {
				s8 *pszDest = g_rArm2EvtDB[g_Arm2EvtCurID].szEvtName;

				i = 0;

				while ((i < TVD_EVT_NAME_MAX_LENGTH) && (*pszDest = *lpName)) {
					i++;
					pszDest++;
					lpName++;
				}

				handle = ((HANDLE)(&g_rArm2EvtDB[g_Arm2EvtCurID++]));
			} else {
				handle = ((HANDLE)(&g_rArm2EvtDB[i]));
			}
		} else {
			handle = ((HANDLE)(&g_rArm2EvtDB[g_Arm2EvtCurID++]));
		}


	} else {
		TVD_LOG(TVD_LOG_LVL_ERR, "X_CreateEvent: Cannot create more event\r\n");
	}

	return handle;
}


bool X_SetEvent(HANDLE hEvent)
{
	bool ret = false;

	if (((u32)hEvent >= (u32)&g_rArm2EvtDB[0]) && ((u32)hEvent < (u32)&g_rArm2EvtDB[TVD_ARM2_EVENT_MAX_CNT])) {
		/*TVD_LOG(TVD_LOG_LVL_ERR, "X_SetEvent: litchi\r\n");*/
		PFNArm2EvtCBFunc pfCB = ((PARM2_EVT_T)hEvent)->pfEvtCallBack;

		if (pfCB) {
			pfCB(((PARM2_EVT_T)hEvent)->u4EvtData);
			ret = true;
		} else {
			TVD_LOG(TVD_LOG_LVL_ERR, "X_SetEvent: CallBack Function is NULL\r\n");
			Printf("X_SetEvent: CallBack Function is NULL\r\n");
			ret = false;
		}
	} else {
		Printf("X_SetEvent: Input Event Handle is invalid\r\n");
		TVD_LOG(TVD_LOG_LVL_ERR, "X_SetEvent: Input Event Handle is invalid\r\n");
		ret = false;
	}

	return ret;
}

bool X_SetEventData(HANDLE hEvent, u32 dwData)
{
	if (((u32)hEvent >= (u32)&g_rArm2EvtDB[0]) && ((u32)hEvent < (u32)&g_rArm2EvtDB[TVD_ARM2_EVENT_MAX_CNT])) {
		((PARM2_EVT_T)hEvent)->u4EvtData = dwData;

		return true;
	} else {
		return false;
	}
}

u32  X_GetEventData(HANDLE hEvent)
{
	if (((u32)hEvent >= (u32)&g_rArm2EvtDB[0]) && ((u32)hEvent < (u32)&g_rArm2EvtDB[TVD_ARM2_EVENT_MAX_CNT])) {
		return (((PARM2_EVT_T)hEvent)->u4EvtData);
	} else {
		return 0XFFFFFFFF;
	}
}

bool X_DestroyEvent(HANDLE hEvent)
{
	return true;
}

bool X_CloseHandle(HANDLE hObject)
{
	return true;
}

#endif








