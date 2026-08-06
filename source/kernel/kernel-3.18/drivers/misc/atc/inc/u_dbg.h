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

#ifndef _U_DBG_H_
#define _U_DBG_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_dbg_drv.h"
#include "u_common.h"


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/* Retun values. */
#define DBGR_OK                 ((INT32)   0)
#define DBGR_OPEN_FAIL          ((INT32)  -1)
#define DBGR_INV_ARG            ((INT32)  -2)
#define DBGR_NOT_ENOUGH_MEM     ((INT32)  -3)
#define DBGR_ALREADY_INIT       ((INT32)  -4)
#define DBGR_NOT_INIT           ((INT32)  -5)
#define DBGR_NO_TRACE_BUFFER    ((INT32)  -6)
#define DBGR_NO_OUTPUT_DEVICE   ((INT32)  -7)
#define DBGR_INV_OUTPUT_DEVICE  ((INT32)  -8)
#define DBGR_NOT_ENABLED        ((INT32)  -9)
#define DBGR_DUMP_IN_PROGRESS   ((INT32) -10)
#define DBGR_REG_CB_ACTIVE      ((INT32) -11)


/* The debug code is the concatenation of three values. The most significat   */
/* 12 bits contain the modul identification, the next 4 bits contain a        */
/* a category value and the least significat 16 bits contain a value, which   */
/* is defined by the module. Note that an abort code value of '0' is reserved */
/* for special usage. The following defines are the module definitions.       */
#define DBG_MOD_DELTA  ((UINT32) 0x00100000)

#define DBG_MOD_DEBUG       ((UINT32) ( 1 * DBG_MOD_DELTA))
#define DBG_MOD_HANDLE      ((UINT32) ( 2 * DBG_MOD_DELTA))
#define DBG_MOD_OS          ((UINT32) ( 3 * DBG_MOD_DELTA))
#define DBG_MOD_RES_MNGR    ((UINT32) ( 4 * DBG_MOD_DELTA))
#define DBG_MOD_CONN_MNGR   ((UINT32) ( 5 * DBG_MOD_DELTA))
#define DBG_MOD_STRM_MNGR   ((UINT32) ( 6 * DBG_MOD_DELTA))
#define DBG_MOD_TBL_MNGR    ((UINT32) ( 7 * DBG_MOD_DELTA))
#define DBG_MOD_RWLOCK      ((UINT32) ( 8 * DBG_MOD_DELTA))
#define DBG_MOD_START       ((UINT32) ( 9 * DBG_MOD_DELTA))
#define DBG_MOD_CDB         ((UINT32) (10 * DBG_MOD_DELTA))
#define DBG_MOD_SCDB        ((UINT32) (11 * DBG_MOD_DELTA))
#define DBG_MOD_GRAPHIC     ((UINT32) (12 * DBG_MOD_DELTA))
#define DBG_MOD_PRC_MNGR    ((UINT32) (13 * DBG_MOD_DELTA))
#define DBG_MOD_SVL         ((UINT32) (14 * DBG_MOD_DELTA))
#define DBG_MOD_TSL         ((UINT32) (15 * DBG_MOD_DELTA))
#define DBG_MOD_SET_JMP     ((UINT32) (16 * DBG_MOD_DELTA))
#define DBG_MOD_AEE         ((UINT32) (17 * DBG_MOD_DELTA))
#define DBG_MOD_BRDCST      ((UINT32) (18 * DBG_MOD_DELTA))
#define DBG_MOD_SVL_BLDR    ((UINT32) (19 * DBG_MOD_DELTA))
#define DBG_MOD_EVTCTX      ((UINT32) (20 * DBG_MOD_DELTA))
#define DBG_MOD_IO_MNGR     ((UINT32) (21 * DBG_MOD_DELTA))
#define DBG_MOD_FLM         ((UINT32) (22 * DBG_MOD_DELTA))
#define DBG_MOD_CL          ((UINT32) (23 * DBG_MOD_DELTA))
#define DBG_MOD_ATV_MNGR    ((UINT32) (24 * DBG_MOD_DELTA))
#define DBG_MOD_CFG         ((UINT32) (25 * DBG_MOD_DELTA))
#define DBG_MOD_FONT        ((UINT32) (26 * DBG_MOD_DELTA))
#define DBG_MOD_UTIL        ((UINT32) (27 * DBG_MOD_DELTA))
#define DBG_MOD_PSI_ENG     ((UINT32) (28 * DBG_MOD_DELTA))
#define DBG_MOD_IMG         ((UINT32) (29 * DBG_MOD_DELTA))
#define DBG_MOD_CLI         ((UINT32) (30 * DBG_MOD_DELTA))
#define DBG_MOD_SEC_MNGR    ((UINT32) (31 * DBG_MOD_DELTA))
#define DBG_MOD_POD         ((UINT32) (32 * DBG_MOD_DELTA))
#define DBG_MOD_SVCTX       ((UINT32) (33 * DBG_MOD_DELTA))
#define DBG_MOD_DATE_TIME   ((UINT32) (34 * DBG_MOD_DELTA))
#define DBG_MOD_OUTOFBAND   ((UINT32) (35 * DBG_MOD_DELTA))
#define DBG_MOD_1394_STACK  ((UINT32) (36 * DBG_MOD_DELTA))
#define DBG_MOD_DSM         ((UINT32) (37 * DBG_MOD_DELTA))
#define DBG_MOD_ABRDCST     ((UINT32) (38 * DBG_MOD_DELTA))
#define DBG_MOD_AVC_HANDLER ((UINT32) (39 * DBG_MOD_DELTA))
#define DBG_MOD_SECT_ENG    ((UINT32) (40 * DBG_MOD_DELTA))
#define DBG_MOD_STRM_SBTL   ((UINT32) (41 * DBG_MOD_DELTA))
#define DBG_MOD_DSMCC_ENG   ((UINT32) (42 * DBG_MOD_DELTA))
#define DBG_MOD_MHP_ENG     ((UINT32) (43 * DBG_MOD_DELTA))
#define DBG_MOD_RRCTX       ((UINT32) (44 * DBG_MOD_DELTA))
#define DBG_MOD_WGL         ((UINT32) (45 * DBG_MOD_DELTA))
#define DBG_MOD_STRM_TTX    ((UINT32) (46 * DBG_MOD_DELTA))
#define DBG_MOD_SOCK        ((UINT32) (47 * DBG_MOD_DELTA))
#define DBG_MOD_MHEG_5      ((UINT32) (48 * DBG_MOD_DELTA))
#define DBG_MOD_PLAYBACK    ((UINT32) (49 * DBG_MOD_DELTA))
#define DBG_MOD_MSVCTX      ((UINT32) (50 * DBG_MOD_DELTA))
#define DBG_MOD_CEC_MNGR    ((UINT32) (51 * DBG_MOD_DELTA))
#define DBG_MOD_PCL         ((UINT32) (52 * DBG_MOD_DELTA))
#define DBG_MOD_LPCH        ((UINT32) (53 * DBG_MOD_DELTA))
#define DBG_MOD_CPS_MNGR    ((UINT32) (54 * DBG_MOD_DELTA))
#define DBG_MOD_SCOM        ((UINT32) (55 * DBG_MOD_DELTA))
#define DBG_MOD_FM          ((UINT32) (56 * DBG_MOD_DELTA))
#define DBG_MOD_DM          ((UINT32) (57 * DBG_MOD_DELTA))
#define DBG_MOD_WMDRM       ((UINT32) (58 * DBG_MOD_DELTA))
#define DBG_MOD_ANIM        ((UINT32) (59 * DBG_MOD_DELTA))
/**
 * Network module
 */
#define DBG_MOD_ACCESS_IP   ((UINT32) (90 * DBG_MOD_DELTA))
#define DBG_MOD_ACCESS_DLNA ((UINT32) (91 * DBG_MOD_DELTA))
#define DBG_MOD_EXPAT_XML   ((UINT32) (92 * DBG_MOD_DELTA))

//#ifdef NRD_ADAPTING_SUPPORT
#define DBG_MOD_NFM         ((UINT32) (93 * DBG_MOD_DELTA))
//#endif


/* For applications, we reserve the module range 0xf0000000 to 0xff000000 (exclusive).  */
/* Note that individual applications must define their own individual module value. For */
/* example:                                                                             */
/*   #define DBG_MOD_NAVIGATOR  DBG_MOD_APPL + (1 * DBG_MOD_DELTA)                      */
/*   #define DBG_MOD_EPG        DBG_MOD_APPL + (2 * DBG_MOD_DELTA)                      */
#define DBG_MOD_APPL       ((UINT32) 0xf0000000)

/* The following defines are used for special modules or init functions. */
#define DBG_MOD_CUSTOM_INIT  ((UINT32) 0xffc00000)
#define DBG_MOD_APPL_INIT    ((UINT32) 0xffd00000)
#define DBG_MOD_BOARD        ((UINT32) 0xffe00000)
#define DBG_MOD_QA           ((UINT32) 0xfff00000)

/* The following defines are the category definitions. */
#define DBG_CAT_MEMORY          ((UINT32) 0x00010000)
#define DBG_CAT_INV_OP          ((UINT32) 0x00020000)
#define DBG_CAT_SEMAPHORE       ((UINT32) 0x00030000)
#define DBG_CAT_THREAD          ((UINT32) 0x00040000)
#define DBG_CAT_MESSAGE         ((UINT32) 0x00050000)
#define DBG_CAT_HANDLE          ((UINT32) 0x00060000)
#define DBG_CAT_TIMER           ((UINT32) 0x00070000)
#define DBG_CAT_CB_RETURN_VAL   ((UINT32) 0x00080000)
#define DBG_CAT_DRV_RETURN_VAL  ((UINT32) 0x00090000)
#define DBG_CAT_INIT            ((UINT32) 0x000a0000)
#define DBG_CAT_IO              ((UINT32) 0x000b0000)
#define DBG_CAT_NO_RECOVERY     ((UINT32) 0x000c0000)
#define DBG_CAT_TRACE_REC       ((UINT32) 0x000f0000)


/* Macro for debug abort / assert. */
#define DBG_ABORT(_code)  x_dbg_abort (((CHAR*) __FILE__), ((UINT32) __LINE__), _code)
#define DBG_ASSERT(_expr, _code)  { if (! (_expr)) DBG_ABORT (_code); }


#define dbg_abort(_code)  DBG_ABORT (_code)

/* Debug level defines. */
#define DBG_LEVEL_NONE   ((UINT16) 0x0000)
#define DBG_LEVEL_ERROR  ((UINT16) 0x0001)
#define DBG_LEVEL_API    ((UINT16) 0x0002)
#define DBG_LEVEL_INFO   ((UINT16) 0x0004)
#define DBG_LEVEL_SANITY   ((UINT16) 0x0008)
//#define DBG_LEVEL_ALL    ((UINT16) 0xffff)
#define DBG_LEVEL_ALL    ((UINT16) 0x00ff)
#define DBG_LAYER_APP    ((UINT16) 0x0100)
#define DBG_LAYER_MMW    ((UINT16) 0x0200)
#define DBG_LAYER_MW     ((UINT16) 0x0400)
#define DBG_LAYER_SYS    ((UINT16) 0x0800)

/* Common macros to perform CLI controlled debug statements. */
/* Note that an individual SW Module MUST set the macro      */
/* DBG_LEVE_MODULE else a compile error will occur.          */
#undef DBG_ERROR
#undef DBG_API
#undef DBG_INFO
#undef DBG_SANITY


#define DBG_ERROR(_stmt)  DBG_LEVEL_STMT (DBG_LEVEL_MODULE, DBG_LEVEL_ERROR, _stmt)
#define DBG_API(_stmt)    DBG_LEVEL_STMT (DBG_LEVEL_MODULE, DBG_LEVEL_API,   _stmt)
#define DBG_INFO(_stmt)   DBG_LEVEL_STMT (DBG_LEVEL_MODULE, DBG_LEVEL_INFO,  _stmt)
#define DBG_SANITY(_stmt)   \
{                                                              \
    extern void vATSendRespToRespCenter(UINT16  ui2_dbg_level, UINT16  ui2_stmt_level, const CHAR* pResponse);  \
                                                               \
    vATSendRespToRespCenter(DBG_LEVEL_MODULE ,DBG_LEVEL_SANITY , _stmt ); \
}

extern INT32 dbg_app_printf(const CHAR *ps_format, ...);
extern INT32 dbg_mmw_printf(const CHAR *ps_format, ...);
extern INT32 dbg_mw_printf(const CHAR *ps_format, ...);
extern INT32 dbg_sys_printf(const CHAR *ps_format, ...);
extern VOID vset_dbg_level(UINT16 u2_stmt_level, UINT16 u2_layer_level);
extern INT32 dbg_level_printf(const CHAR *ps_format, ...);
#endif /* _U_DBG_H_ */

