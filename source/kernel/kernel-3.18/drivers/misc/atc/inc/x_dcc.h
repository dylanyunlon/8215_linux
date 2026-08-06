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

#ifndef _X_DCC_H_
#define _X_DCC_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "x_common.h"
#include "x_demux.h"
#include "x_rm.h"
#include "x_drv_cb.h"
#include "x_memtype.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/* Get operations */
#define DCC_GET_TYPE_CTRL               (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  3))
#define DCC_GET_TYPE_V_FIFOFULLNESS_RANGE               (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  4))
#define DCC_GET_TYPE_A_FIFOFULLNESS_RANGE               (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  5))

/* Set operations */
#define DCC_SET_TYPE_CTRL              ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  3)) + RM_SET_TYPE_ARG_NO_REF)
#define DCC_SET_TYPE_NFY_FCT            (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  4))
#define DCC_SET_TYPE_TS_PACK_SZ         (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  5))

/* Notify function */
#if UNIFORM_DRV_CALLBACK
typedef struct
{
    DEMUX_COND_T      e_nfy_cond;
    UINT32              ui4_data_1;
    UINT32              ui4_data_2;
} DCC_CB_INFO_T;
#else
typedef BOOL (*x_dcc_nfy_fct) (VOID*         pv_nfy_tag,
                                 DEMUX_COND_T  e_nfy_cond,
                                 UINT32        ui4_data_1,
                                 UINT32        ui4_data_2);

/* Notify info setting */
typedef struct _DCC_NFY_INFO_T
{
    VOID*  __opaque__ pv_tag;
    x_dcc_nfy_fct __local_space__ pf_dcc_nfy;
}   DCC_NFY_INFO_T;
#endif

/* TS pack size setting */
typedef enum
{
    DCC_TS_PACK_SZ_UNKNOWN = 0,
    DCC_TS_PACK_SZ_188,
    DCC_TS_PACK_SZ_192,
    DCC_TS_PACK_SZ_206
}DCC_TS_PACK_SZ_T;


#endif /* _X_DCC_H_ */
