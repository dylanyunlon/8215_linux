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

#ifndef _X_HANDLE_H_
#define _X_HANDLE_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_handle.h"
#include "x_common.h"


/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

extern INT32 x_handle_free (HANDLE_T  h_handle);
extern INT32 x_handle_get_info (HANDLE_T        h_handle,
                                HANDLE_TYPE_T*  pe_type,
                                VOID**          ppv_tag);
extern INT32 x_handle_get_tag (HANDLE_T  h_handle,
                               VOID**    ppv_tag);
extern INT32 x_handle_get_type (HANDLE_T        h_handle,
                                HANDLE_TYPE_T*  pe_type);
extern INT32 x_handle_set_tag (HANDLE_T  h_handle,
                               VOID*     pv_tag);
extern INT32 x_handle_status (UINT16*  pui2_num_handles,
                              UINT16*  pui2_num_free);
extern BOOL x_handle_valid (HANDLE_T  h_handle);


#endif /* _X_HANDLE_H_ */
