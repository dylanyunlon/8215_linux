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
/*----------------------------------------------------------------------------*
 * $RCSfile: dbg.h,v $
 * $Revision: #1 $
 * $Date: 2015/07/09 $
 * $Author: bin.yang $
 *
 * Description:
 *         This header file contains debug related definitions, which are
 *         known to the whole Middleware.
 *---------------------------------------------------------------------------*/

#ifndef _DBG_H_
#define _DBG_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "x_common.h"
#include "x_dbg.h"
#include "x_dbg_drv.h"
#include "x_handle.h"


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/* Maximum trace record length. */
#define MAX_TRACE_REC_LEN  12

#define app 0x80
#define driver 0x40
#define mediamw 0x20
#define mw 0x10
#define sys 0x08

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

extern INT32 dbg_add_thread_name (CHAR*  ps_name);
extern INT32 dbg_add_trace_rec (BOOL    b_isr,
                                UINT32  ui4_code,
                                VOID*   pv_data,
                                SIZE_T  z_len);

extern INT32 dbg_output_init (const CHAR*        ps_out_name,
                              DBG_OUTPUT_TYPE_T  e_output_type,
                              const VOID*        pv_output_info);
extern INT32 dbg_trace_init (UINT16  ui2_num_trace_recs);

extern INT32 dbg_set_ctrl_stmt (BOOL  b_ctrl);
extern BOOL dbg_get_ctrl_stmt (VOID);
extern INT32 dbg_reg_ctrl_stmt_cb (x_dbg_output_fct  pf_output);
extern s32 dbg_printf (u32 Layer,const char *ps_format, ...);
extern s32 vdbg_printf (u32 Layer,const char *ps_format, va_list t_ap);


#endif /* _DBG_H_ */
