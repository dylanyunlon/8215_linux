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
#ifndef _U_DBG_DRV_H_
#define _U_DBG_DRV_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_common.h"

#ifndef _BDP_ADV_RELEASE
#undef DEBUG
#define DEBUG
#endif

#undef EXT_DBG_DEV
#define EXT_DBG_DEV

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

/* The following API's must be implemented by the manufacturer. */
extern VOID x_dbg_abort (CHAR*   ps_file_path,
                         UINT32  ui4_line_num,
                         UINT32  ui4_code);


#endif /* _U_DBG_DRV_H_ */
