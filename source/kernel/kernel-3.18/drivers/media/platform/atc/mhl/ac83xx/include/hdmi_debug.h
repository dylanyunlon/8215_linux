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


#ifndef _HDMI_DEBUG_H_
#define _HDMI_DEBUG_H_



#define HDMI_LOG_ERROR                          (0x00000000)
#define HDMI_LOG_WARN                           (0x00000001)
#define HDMI_LOG_DEBUG                          (0x00000002)
#define HDMI_LOG_INFO                           (0x00000004)
#define HDMI_LOG_USER1                          (0x00000008)
#define HDMI_LOG_USER2                          (0x00000010)


/*debug log*/
extern UINT32 g_u4HdmiLogMask;

#define X_Printf  printk


#define HDMI_LOG(lvl, formatStr, args...)\
{ \
	switch (lvl) { \
	case HDMI_LOG_ERROR: \
		pr_err("[HDMI]"formatStr, ##args); \
		break; \
	case HDMI_LOG_WARN: \
		pr_warn("[HDMI]"formatStr, ##args); \
		break; \
	case HDMI_LOG_INFO: \
	    pr_info("[HDMI]"formatStr, ##args); \
	    break; \
	case HDMI_LOG_DEBUG: \
		pr_debug("[HDMI]"formatStr, ##args); \
		break; \
	case HDMI_LOG_USER1: \
	case HDMI_LOG_USER2: \
		break; \
	default: \
		break; \
	} \
}

/*
#define HDMI_LOG(lvl, formatStr, args...)\
			{           \
				if(lvl & g_u4HdmiLogMask){\
					X_Printf("[HDMIDrv]:");  \
					X_Printf(formatStr, ##args);} \
			}
*/



void vHdmiSetDebugMask(UINT32 u4LogMask);





#endif
