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


#ifndef TVD_LOG_H
#define TVD_LOG_H

#include "tvd_cfg.h"
#ifndef __ARM2__
#include "x_os.h"
#endif

/**************************************************************************
*  LOG RELATED SECTION
**************************************************************************/
/**************************************************************************
* Log Level
*************************************************************************/

/*pr_err*/
#define TVD_LOG_LVL_ERR            0U

/*pr_info*/
#define TVD_LOG_LVL_OFF            1U
#define TVD_LOG_LVL_TRACE          2U
#define TVD_LOG_LVL_INFO           3U
#define TVD_LOG_LVL_WARN           4U

/*pr_debug*/
#define TVD_LOG_LVL_HAL            5U
#define TVD_LOG_LVL_IRQ            6U
#define TVD_LOG_LVL_DBG            7U

#define ERROR_NONE						0
#define ERROR_INVALID_APP_ID					1
#define ERROR_INVALID_APP_INFO					2
#define ERROR_INVALID_PARA					3
#define ERROR_INVALID_TVD_HAL_STATE				4
#define ERROR_INVALID_TVD_STATE					5
#define ERROR_NO_FREE_TVD_CH					6
#define ERROR_INVALID_VDOBUF_CNT				7
#define ERROR_INVALID_ALLOC_VDOBUF_FAIL				8
#define ERROR_INVALID_OUTPUT_FMT				9
#define ERROR_INVALID_OUTPUT_BUF				10
#define ERROR_CAN_IGNORE					11
#define ERROR_MAX						12

extern u32 debug;

#ifndef __ARM2__
#define TVD_LOG(lvl, formatStr, args...)\
({ \
	u8 *tvd_log_level[] = {\
		"[TVD][ERR]",\
		"[TVD][OFF]",\
		"[TVD][TRACE]",\
		"[TVD][INFO]",\
		"[TVD][WARN]",\
		"[TVD][HAL]",\
		"[TVD][IRQ]",\
		"[TVD][DBG]"\
	};\
	switch ((lvl)) {\
	case TVD_LOG_LVL_ERR:\
		pr_err("%s%s:%s():%d: "formatStr, tvd_log_level[(lvl)], FILE_ONLY, __func__, __LINE__, __LINE__, ##args);\
		break;\
	case TVD_LOG_LVL_OFF:\
	case TVD_LOG_LVL_TRACE:\
	case TVD_LOG_LVL_INFO:\
	case TVD_LOG_LVL_WARN:\
		pr_info("%s%s(): "formatStr, tvd_log_level[(lvl)], __func__, ##args);\
		break;\
	case TVD_LOG_LVL_DBG:\
	case TVD_LOG_LVL_HAL:\
	case TVD_LOG_LVL_IRQ:\
		pr_debug("%s%s(): "formatStr, tvd_log_level[(lvl)], __func__, ##args);\
		break;\
	default:\
		break;\
	} \
})
#else
#define TVD_LOG(lvl, formatStr, args...)\
({ \
	s8 *tvd_log_level[] = {\
		"[ERR]",\
		"[OFF]",\
		"[TRACE]",\
		"[INFO]",\
		"[WARN]",\
		"[DBG]",\
		"[HAL]",\
		"[IRQ]",\
	};\
	if ((lvl) <= debug) {\
		Printf("[ARM2_TVD]%s ", tvd_log_level[(lvl)]);\
		Printf(formatStr, ##args);\
	} \
})
#endif


#endif








