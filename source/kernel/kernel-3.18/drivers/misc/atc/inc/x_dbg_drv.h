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

#ifndef _X_DBG_DRV_H_
#define _X_DBG_DRV_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_dbg_drv.h"
#include "x_dbg.h"
#include "x_common.h"


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

#define SYS_EXIT  ((UINT32) 0)


typedef VOID (*x_dbg_data_rec_fct) (UINT8  ui1_data);

#ifdef DEBUG
#ifdef EXT_DBG_DEV
typedef void (*PF_DBG_INPUT_T) (INT32 i4Ch);
typedef INT32 (*x_dbg_SetCallback)(BOOL fgEnable, PF_DBG_INPUT_T pfInputCb);
extern VOID x_dbg_reg_setcallback(x_dbg_SetCallback pf_setcallback);
typedef INT32 (*x_dbg_putstmt)(const CHAR* ps_stmt, INT32 i4Cnt);
extern VOID x_dbg_reg_putstmt(x_dbg_putstmt pf_putstmt);
#endif
#endif

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

/* The following API's must ONLY be implemented by the manufacturer */
/* if the debug output is not routed via a driver registered with   */
/* the Resource Manager. This means, the debug library has been     */
/* compiled with the define EXT_DBG_DRV set.                        */
extern INT32 x_dbg_open_output (const CHAR*        ps_output_name,
                                DBG_OUTPUT_TYPE_T  e_output_type,
                                const VOID*        pv_output_info);
extern VOID x_dbg_put_stmt (const CHAR* ps_stmt);
extern VOID x_dbg_reg_data_rec (x_dbg_data_rec_fct  pf_data_rec);


#endif /* _X_DBG_DRV_H_ */
