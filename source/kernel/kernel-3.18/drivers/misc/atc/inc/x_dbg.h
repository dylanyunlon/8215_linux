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

#ifndef _X_DBG_H_
#define _X_DBG_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_dbg.h"
#include "u_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/* Macro to be used for level control debug statements. Note that */
/* the extern API declaration in the macro DBG_LEVEL_STMT was     */
/* placed there so that that API's external declaration is not    */
/* visible outside the macro's namespace.                         */
#undef DBG_LEVEL_STMT


#ifdef DEBUG
#if 0
#define DBG_LEVEL_STMT(_dbg_level, _stmt_level, _stmt)         \
{                                                              \
    extern UINT16 dbg_eval_dbg_level (UINT16  ui2_dbg_level);  \
                                                               \
    if ((_stmt_level & dbg_eval_dbg_level (_dbg_level)) != 0)  \
    {                                                          \
        x_dbg_ctrl_stmt _stmt;                                 \
    }                                                          \
}
#else
#define DBG_LEVEL_STMT(_dbg_level, _stmt_level, _stmt)         \
{                                                              \
    vset_dbg_level(_stmt_level, _dbg_level);               \
    dbg_level_printf _stmt;                                \
}
#endif
#else
#define DBG_LEVEL_STMT(_dbg_level, _stmt_level, _stmt)
#endif


/* Debug setup types. */
typedef enum
{
    DBG_OUTPUT_TYPE_IGNORE = 0,
    DBG_OUTPUT_TYPE_SERIAL_PORT
}   DBG_OUTPUT_TYPE_T;


/* Callback function definitions. */
typedef VOID (*x_dbg_output_fct) (const CHAR*  ps_stmt);
typedef VOID (*x_dbg_trace_rec_fct) (BOOL         b_trace_ena,
                                     UINT32       ui4_code,
                                     const VOID*  pv_data,
                                     SIZE_T       z_len);
typedef VOID (*x_dbg_data_inp_fct) (UINT8  ui1_inp_data,
                                    VOID*  pv_tag);


/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

/* The following API's are exported by the Middleware. */
extern INT32 x_dbg_dump_trace_buff (x_dbg_output_fct  pf_output);
extern INT32 x_dbg_flush_trace_buff (VOID);
extern BOOL x_dbg_get_trace_buff_ctrl (VOID);
extern INT32 x_dbg_reg_trace_rec_cb (x_dbg_trace_rec_fct  pf_trace_rec);
extern INT32 x_dbg_reg_data_inp_cb (x_dbg_data_inp_fct  pf_data_inp,	VOID* pv_tag);
extern INT32 x_dbg_set_trace_buff_ctrl (BOOL  b_trace_ena);
extern INT32 x_dbg_stmt (const CHAR*  ps_format, ...);
extern INT32 x_dbg_ctrl_stmt (const CHAR*  ps_format, ...);

#ifdef __cplusplus
}
#endif

#endif /* _X_DBG_H_ */
