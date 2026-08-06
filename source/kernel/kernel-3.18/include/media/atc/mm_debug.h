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


/*****************************************************************************
*  Audio Driver: Interface
*****************************************************************************/

#ifndef MM_DEBUG_H
#define MM_DEBUG_H

#include <linux/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	DEBUG_LEVEL_DEFAULT = ((__u32)0),
	DEBUG_LEVEL_VERBOSE = ((__u32)(1 << 0)),
	DEBUG_LEVEL_FFRW = ((__u32)(1 << 1)),
	DEBUG_LEVEL_AUINFO = ((__u32)(1 << 2)),
	DEBUG_LEVEL_SUBINFO = ((__u32)(1 << 3)),
	DEBUG_LEVEL_SEEKINFO = ((__u32)(1 << 4)),
	DEBUG_LEVEL_SENDBUFMASKINFO = ((__u32)(1 << 5)),
	DEBUG_LEVEL_FILEOPER = ((__u32)(1 << 6)),
	DEBUG_LEVEL_ID3INFO = ((__u32)(1 << 7)),
	DEBUG_LEVEL_TRACE = ((__u32)(1 << 8)),
	DEBUG_LEVEL_TXDATA    = ((__u32)(1 << 9)),
	DEBUG_LEVEL_RELEASEBUF = ((__u32)(1 << 10)),
	DEBUG_LEVEL_SUBDUMP    = ((__u32)(1 << 11)),
	DEBUG_LEVEL_QUEUEAU    = ((__u32)(1 << 12)),
	DEBUG_LEVEL_DEQUEUEAU    = ((__u32)(1 << 13)),
	DEBUG_LEVEL_MESSAGE    = ((__u32)(1 << 14)),
	DEBUG_LEVEL_AUDAUINFO = ((__u32)(1 << 15)),
	DEBUG_LEVEL_VIDAUINFO = ((__u32)(1 << 16))
} E_DEBUG_LEVEL;

#define VA_LIST						va_list
#define VA_START(ap, a1)	va_start((ap), (a1))
#define VA_ARG(ap, t)			va_arg(ap, t)
#define VA_END(ap)				va_end((ap))

#define TEXT(x) x

#ifdef __linux__
typedef struct _DBGPARAM {
   char   lpszName[32];
   char   rglpszZones[16][32];
   __u32  u4ZoneMask;
} DBGPARAM, *LPDBGPARAM;
#else
typedef struct _DBGPARAM {
   char   lpszName[32];
   char   rglpszZones[16][32];
   __u32  u4ZoneMask;
} DBGPARAM, *LPDBGPARAM;
#endif

#ifdef __KERNEL__
#define MM_RETAILMSG(fg, x... )  \
do { \
    if (fg) \
        printk x; \
} while (0)
#define MM_DEBUGMSG(fg, x... )  \
	do { \
			if (fg) \
				printk x ;\
	} while (0)
#else
#define MM_RETAILMSG(fg, x... )  \
do { \
    if (fg) \
        printf x; \
} while (0)
#define MM_DEBUGMSG(fg, x... )  \
	do { \
			if (fg) \
				printf x ;\
	} while (0)
#endif

#define LOG_LEVEL_DEFALUT_MASK ((__u32)0xe)	/* only print Error andr fatal */
#ifdef DEBUG
#define MMLOG_DECLARATION(mod)	\
		DBGPARAM dpCurSettings = {	\
		mod, {	\
		"Debug", "Warn", "Error", "Fatal"},	\
		0xf	\
	 }
#else
#define MMLOG_DECLARATION(mod)	\
		DBGPARAM dpCurSettings = {	\
		mod, {	\
		"Debug", "Warn", "Error", "Fatal"},	\
		LOG_LEVEL_DEFALUT_MASK			\
	 }
#endif

typedef enum {
	LOG_MOD_NONE = 0,	/* "  " */
	LOG_MOD_DMX = 1,	/* "DMX" : Demxer driver */
	LOG_MOD_VDEC = 2,	/* "Vdec" : Video decoder driver */
	LOG_MOD_MSDKCORE = 3,	/* "Filter" : ds filter */
	LOG_MOD_APP = 4,	/* "Filter" : ds filter */
	LOG_MOD_WCM = 5,	/* "WCM" : WriteChannel memory */
	LOG_MOD_DVP = 6,	/*"DVP"   :DVP */
	LOG_MOD_OMX = 7,
	LOG_MOD_MS  = 8,
	LOG_MOD_DMC = 9,
	LOG_MOD_MMISC = 10,
	LOG_MOD_DVR = 11,
	LOG_MOD_ALL = 0XFF	/* log all module */
} LOG_MOD_ID_T;

#define MMLOGBUFMAX 1022

#define LOG_ModSetLevel(pcLogLevel) do { \
	if (0 == strcmp(pcLogLevel, "Debug")) \
			dpCurSettings.u4ZoneMask = (MM_LOG_DEBUG | MM_LOG_TRACE | MM_LOG_ERROR | MM_LOG_FATAL); \
	else if (0 == strcmp(pcLogLevel, "Trace")) \
			dpCurSettings.u4ZoneMask = (MM_LOG_TRACE | MM_LOG_ERROR | MM_LOG_FATAL); \
	else if (0 == strcmp(pcLogLevel, "Error")) \
			dpCurSettings.u4ZoneMask = (MM_LOG_ERROR | MM_LOG_FATAL); \
	else if (0 == strcmp(pcLogLevel, "Fatal")) \
			dpCurSettings.u4ZoneMask =	MM_LOG_FATAL; \
	else \
			MM_RETAILMSG(1, ("[MM_Debug] Set Log Level(%s) Failed!\r\n", pcLogLevel)); \
} while (0)

extern DBGPARAM dpCurSettings;

#define MM_LOG_DEBUG		(0x00000001U << (0U))
#define MM_LOG_WARN			(0x00000001U << (1U))
#define MM_LOG_TRACE		(0x00000001U << (2U))
#define MM_LOG_ERROR		(0x00000001U << (3U))
#define MM_LOG_FATAL		(0x00000001U << (4U))

#define LOG_LVL_FATAL		(dpCurSettings.u4ZoneMask & (__u8)MM_LOG_FATAL)
#define LOG_LVL_ERROR		(dpCurSettings.u4ZoneMask & (__u8)MM_LOG_ERROR)
#define LOG_LVL_INFO            (dpCurSettings.u4ZoneMask & (__u8)MM_LOG_ERROR)
#define LOG_LVL_TRACE		(dpCurSettings.u4ZoneMask & (__u8)MM_LOG_TRACE)
#define LOG_LVL_WARN		(dpCurSettings.u4ZoneMask & (__u8)MM_LOG_WARN)
#define LOG_LVL_DEBUG		(dpCurSettings.u4ZoneMask & (__u8)MM_LOG_DEBUG)

#define MMLOG_DEBUG  if (0U == LOG_LVL_DEBUG) {; } else LOG_ModWmsgD
#define MMLOG_TRACE  if (0U == LOG_LVL_TRACE) {; } else LOG_ModWmsgT
#define MMLOG_INFO   if (0U == LOG_LVL_INFO)  {; } else LOG_ModWmsgT
#define MMLOG_WARN(mod, fmt, arg...)  if (0U == LOG_LVL_WARN)  {; } else LOG_ModWmsgW(mod, "[%s:%s:%d]" fmt,basename((char*)__FILE__),__FUNCTION__,__LINE__, ##arg)
#define MMLOG_ERROR(mod, fmt, arg...) if (0U == LOG_LVL_ERROR) {; } else LOG_ModWmsgE(mod, "[%s:%s:%d]" fmt,basename((char*)__FILE__),__FUNCTION__,__LINE__, ##arg)
#define MMLOG_FATAL(mod, fmt, arg...) if (0U == LOG_LVL_FATAL) {; } else LOG_ModWmsgE(mod, "[%s:%s:%d]" fmt,basename((char*)__FILE__),__FUNCTION__,__LINE__, ##arg)
#define MMLOG_DEBUG_IF	LOG_ModWmsgD_IF

#define MM_ASSERT(arg)  do { \
	if (!(arg)) \
		MM_RETAILMSG(1, ("DBGCHK Failed:%s at line %d in %s\r\n",\
			__func__, __LINE__, __FILE__)); \
} while (0)

/* #define MM_ATE_CHECK */
#ifdef MM_ATE_CHECK
#define MMATE_INIT_STRUCT(structname) \
		(structname).u4MMATECHKStart = (structname).u4MMATECHKEnd = 0

#define MMATE_INIT_POINTER(ptrtname) do { \
		if (NULL == (ptrtname)) \
				MM_ASSERT(0); \
		else \
				(ptrtname)->u4MMATECHKStart = (ptrtname)->u4MMATECHKEnd = 0; \
} while (0)

#define MMATE_INIT_VAL(val) ((val) = 0)

#define MMATE_CHECK_STRUCT(structname) do { \
		if (0 != (structname).u4MMATECHKStart) \
				MM_ASSERT(0); \
		else if (0 != (structname).u4MMATECHKEnd)\
				MM_ASSERT(0); \
} while (0)


#define MMATE_CHECK_POINTER(ptrtname) do { \
		if (NULL == (ptrtname)) \
				MM_ASSERT(0); \
		if (0 != (ptrtname)->u4MMATECHKStart) \
				MM_ASSERT(0); \
		else if (0 != (ptrtname)->u4MMATECHKEnd)\
				MM_ASSERT(0); \
} while (0)

#define MMATE_CHECK_VAL(val) do { \
		if (0 != (val)) \
				MM_ASSERT(0); \
} while (0)
#else
#define MMATE_INIT_STRUCT(structname)
#define MMATE_INIT_POINTER(ptrtname)
#define MMATE_INIT_VAL(val)
#define MMATE_CHECK_STRUCT(structname)
#define MMATE_CHECK_POINTER(ptrtname)
#define MMATE_CHECK_VAL(val)
#endif

bool LOG_ModInit(void);

void LOG_ModDeinit(void);

void LOG_ModWmsgD(LOG_MOD_ID_T tModId, const char *wsFmt, ...);
void LOG_ModWmsgW(LOG_MOD_ID_T tModId, const char *wsFmt, ...);
void LOG_ModWmsgE(LOG_MOD_ID_T tModId, const char *wsFmt, ...);
void LOG_ModWmsgT(LOG_MOD_ID_T tModId, const char *wsFmt, ...);
void LOG_ModWmsgD_IF(bool fgPrintout, LOG_MOD_ID_T tModId, const char *wsFmt, ...);

#define DLLENTRY

#ifdef __cplusplus
}
#endif
#endif				/* MM_DEBUG_H */
