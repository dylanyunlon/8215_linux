/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */


/*****************************************************************************
*  Audio Driver: Interface
*****************************************************************************/

#ifndef MM_LOG_H
#define MM_LOG_H

#include <linux/types.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef LOG_TAG
#define LOG_TAG "[MM]"
#endif

#define TEXT(x) x

#define LOG_LEVEL_DEFALUT_MASK ((__u32)0xe)	/* only print Error andr fatal */
#define MMLOG_DECLARATION(mod)	\
        int curLogLevel_##mod = MM_LOG_INFO

typedef enum mm_log_level
{
    MM_LOG_FATAL     = 0,  /**< "fatal": serious errors; */
    MM_LOG_ERROR     = 1,  /**< "error": error happens; */
    MM_LOG_WARN      = 2,  /**< "warn": may not well used; */
    MM_LOG_INFO      = 3,  /**< "info": provide some information; */
    MM_LOG_DEBUG     = 4,  /**< "debug": provide debug information; */
    MM_LOG_VERBOSE   = 5,  /**< "": every thing; */
    MM_LOG_COUNT     = 6
} mm_log_level;

#define MMLOGBUFMAX 1022

#define LOG_ModSetLevel(mod, level) \
	curLogLevel_##mod = level

#define LOG_MOD_DT "dt"
#define LOG_MOD_OMX "OMX"
#define LOG_MOD_VDEC "VDEC"
#define LOG_MOD_MMISC "MMISC"
#define LOG_MOD_CTA "CTA"
#define LOG_MOD_MSCAN "MSCAN"

extern int curLogLevel_LOG_MOD_DT;
extern int curLogLevel_LOG_MOD_OMX;
extern int curLogLevel_LOG_MOD_VDEC;
extern int curLogLevel_LOG_MOD_MMISC;
extern int curLogLevel_LOG_MOD_CTA;
extern int curLogLevel_LOG_MOD_MSCAN;

#define MMLOG_DEBUG(mod, fmt, arg...)  \
    if (curLogLevel_##mod >= MM_LOG_DEBUG) \
        LOG_ModWmsgD(mod, "[" LOG_TAG "][%s:%d]" fmt,__FUNCTION__,__LINE__, ##arg)

#define MMLOG_TRACE(mod, fmt, arg...)  \
    if (curLogLevel_##mod >= MM_LOG_INFO) \
        LOG_ModWmsgT(mod, "[" LOG_TAG "][%s:%d]" fmt,__FUNCTION__,__LINE__, ##arg)

#define MMLOG_INFO(mod, fmt, arg...)  \
    if (curLogLevel_##mod >= MM_LOG_INFO) \
        LOG_ModWmsgT(mod, "[" LOG_TAG "][%s:%d]" fmt,__FUNCTION__,__LINE__, ##arg)

#define MMLOG_WARN(mod, fmt, arg...)  \
    if (curLogLevel_##mod >= MM_LOG_WARN) \
        LOG_ModWmsgW(mod, "[" LOG_TAG "][%s:%s:%d]" fmt,basename((char*)__FILE__),__FUNCTION__,__LINE__, ##arg)

#define MMLOG_ERROR(mod, fmt, arg...) \
    if (curLogLevel_##mod >= MM_LOG_ERROR) \
        LOG_ModWmsgE(mod, "[" LOG_TAG "][%s:%s:%d]" fmt,basename((char*)__FILE__),__FUNCTION__,__LINE__, ##arg)

#define MMLOG_FATAL(mod, fmt, arg...) \
    LOG_ModWmsgE(mod, "[" LOG_TAG "][%s:%s:%d]" fmt,basename((char*)__FILE__),__FUNCTION__,__LINE__, ##arg)

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

#define MOD_VERSION_INFO(mod, major, minor,rev) \
    LOG_ModWmsgT(NULL, "[VER][%s] V%02d.%02d_%02d\r\n", mod, major, minor, rev)


bool LOG_ModInit(void);

void LOG_ModDeinit(void);

void LOG_ModWmsgD(char *mod, const char *wsFmt, ...);
void LOG_ModWmsgW(char *mod, const char *wsFmt, ...);
void LOG_ModWmsgE(char *mod, const char *wsFmt, ...);
void LOG_ModWmsgT(char *mod, const char *wsFmt, ...);

#define DLLENTRY

#ifdef __cplusplus
}
#endif
#endif /* MM_LOG_H */
