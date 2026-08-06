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

#ifndef _X_RM_H_
#define _X_RM_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "x_common.h"
#include "x_sys_name.h"
#include "section.h"
/*alfonso add for linux*/
#if (CONFIG_SECTION_BUILD_LINUX_PROG)
#include "u_handle.h"
#endif

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
/* Operation types. */
typedef UINT32 DRV_CONN_TYPE_T;
typedef UINT32 DRV_DISC_TYPE_T;
typedef UINT32 DRV_GET_TYPE_T;
typedef UINT32 DRV_SET_TYPE_T;

/* Get types */
#define RM_GET_TYPE_LAST_ENTRY  ((DRV_GET_TYPE_T) 256)

/* Set types */
#define RM_SET_TYPE_LAST_ENTRY  ((DRV_SET_TYPE_T) 256)


#define RM_LIGHT_SUPPORT    1

/* Max number of characters in a Resource Manager name */
/* (excluding the zero terminating character).         */
#define RM_NAME_LEN  SYS_NAME_LEN

/* Do not increase the number of ports past the value '32'. If that is  */
/* truly required, then one must also check the code to ensure that bit */
/* mask remain valid etc.                                               */
#define MAX_NUM_PORTS  ((UINT8)  32)
#define MAX_COMP_ID    ((UINT16) 0xffff)

#define ALL_PORTS     ((UINT8) 0xff)
#define SUPPORT_PORT  ((UINT8) 0xfe)  /* Indicates support port. */

typedef UINT16 DRV_TYPE_T;
typedef UINT32 RM_COND_T;

/* Selects the union member in DRV_COMP_REG_T */
typedef enum
{
    ID_TYPE_IND = 0,
    ID_TYPE_RANGE,
    ID_TYPE_LIST
}   ID_TYPE_T;

/* Individual id, tag and port value */
typedef struct _ID_IND_T
{
    UINT16  ui2_id;

    VOID*  pv_tag;

    UINT8  ui1_port;
}   ID_IND_T;

/* List of id, tag and port values */
typedef struct _ID_LIST_T
{
    ID_IND_T*  pt_list;

    UINT16  ui2_num_of_ids;
}   ID_LIST_T;

/* Range of id, tag and port values */
typedef struct _ID_RANGE_T
{
    UINT16  ui2_first_id;
    UINT16  ui2_delta_id;

    VOID*  pv_first_tag;
    VOID*  pv_delta_tag;

    UINT16  ui2_num_of_ids;

    UINT8  ui1_port;
}   ID_RANGE_T;


/* Driver component id used by the driver API's */
typedef struct _DRV_COMP_ID_T
{
    VOID*  pv_tag;

    DRV_TYPE_T  e_type;

    UINT16  ui2_id;

    BOOL  b_sel_out_port;

    union
    {
        UINT8  ui1_inp_port;
        UINT8  ui1_out_port;
    }   u;
}   DRV_COMP_ID_T;


/* Driver registration structure */
typedef struct _DRV_COMP_REG_T
{
    DRV_TYPE_T  e_type;

    ID_TYPE_T  e_id_type;

    union
    {
        ID_IND_T    t_ind;
        ID_RANGE_T  t_range;
        ID_LIST_T   t_list;
    }   u;
}   DRV_COMP_REG_T;

#if (1 == RM_LIGHT_SUPPORT || 1 == CONFIG_SECTION_BUILD_LINUX_KO || 1 == CONFIG_SECTION_BUILD_LINUX_PROG)
typedef enum
{
  GT_Unknown       = 0,
  GT_MainDisplay   = 0x001,
  GT_SubDisplay    = 0x002,
  GT_AuxDisplay    = 0x004,
  GT_ThirdDisplay  = 0x800,
  GT_AudMixSound_0 = 0x008,
  GT_AudMixSound_1 = 0x010,
  GT_AudMixSound_2 = 0x020,
  GT_AudMixSound_3 = 0x040,
  GT_AudMixSound_4 = 0x080,
  GT_AudMixSound_5 = 0x100,
  GT_AudMixSound_6 = 0x200,
  GT_AudMixSound_7 = 0x400,
}Group_Type_T;
#endif

/* Connection list direction. */
typedef enum
{
    CONN_DIR_OUT_TO_INP = 0,
    CONN_DIR_INP_TO_OUT,
    CONN_DIR_TO_SUPPORT,
    CONN_DIR_FROM_SUPPORT
}   CONN_DIR_TYPE_T;


/* Callback condition */
typedef enum
{
    DRV_COND_DISCONNECTED = 0,
    DRV_COND_CONNECTED,
    DRV_COND_STATUS
}   DRV_COND_T;

/* Status or notify condition groups */
typedef enum
{
    COND_GRP_OBJ_STATE = 0,
    COND_GRP_CONN_STATE,
    COND_GRP_SHARE_STATE,
    COND_GRP_ARBITRATION,
    COND_GRP_REASON,
    COND_GRP_CONN_ORIG,
    COND_GRP_OPERATION,
    COND_GRP_MON_COMP_STATE
}   COND_GRP_T;

/* Connect reasons */
#define RM_CONN_REASON_AS_REQUESTED  ((UINT32)   0)

/* Disconnect reasons */
#define RM_DISC_REASON_AS_REQUESTED  ((UINT32)   0)

/* Driver callback function. */
typedef VOID (*x_rm_nfy_fct) (DRV_COMP_ID_T*  pt_comp_id,
                              DRV_COND_T      e_nfy_cond,
                              VOID*           pv_tag,
                              UINT32          ui4_data);

/* Connection types */
#define RM_CONN_TYPE_IGNORE   ((DRV_CONN_TYPE_T)   0)
#define RM_CONN_TYPE_RESTORE  ((DRV_CONN_TYPE_T)   1)
#define RM_CONN_TYPE_COMP_ID  ((DRV_CONN_TYPE_T)   2)

/* Driver control functions. */
typedef INT32 (*x_rm_connect_fct) (DRV_COMP_ID_T*   pt_comp_id,
                                   DRV_CONN_TYPE_T  e_conn_type,
                                   const VOID*      pv_conn_info,
                                   SIZE_T           z_conn_info_len,
                                   VOID*            pv_tag,
                                   x_rm_nfy_fct     pf_nfy);


/* Disconnect types */
#define RM_DISC_TYPE_IGNORE   ((DRV_DISC_TYPE_T)   0)
#define RM_DISC_TYPE_COMP_ID  ((DRV_DISC_TYPE_T)   1)

typedef INT32 (*x_rm_disconnect_fct) (DRV_COMP_ID_T*   pt_comp_id,
                                      DRV_DISC_TYPE_T  e_disc_type,
                                      const VOID*      pv_disc_info,
                                      SIZE_T           z_disc_info_len);

/* Get types */
#define RM_GET_TYPE_IGNORE       ((DRV_GET_TYPE_T)   0)
#define RM_GET_TYPE_ISR_CONTEXT  ((DRV_GET_TYPE_T) 0x00100000)

typedef INT32 (*x_rm_get_fct) (DRV_COMP_ID_T*  pt_comp_id,
                               DRV_GET_TYPE_T  e_get_type,
                               VOID*           pv_get_info,
                               SIZE_T*         pz_get_info_len);

/* Set types */
#define RM_SET_TYPE_IGNORE       ((DRV_SET_TYPE_T)   0)
#define RM_SET_TYPE_ISR_CONTEXT  ((DRV_SET_TYPE_T) 0x00100000)
#define RM_SET_TYPE_ARG_NO_REF   ((DRV_SET_TYPE_T) 0x00200000)
#define RM_SET_TYPE_GET_INFO     ((DRV_SET_TYPE_T) 0x00400000)

typedef INT32 (*x_rm_set_fct) (DRV_COMP_ID_T*  pt_comp_id,
                               DRV_SET_TYPE_T  e_set_type,
                               const VOID*     pv_set_info,
                               SIZE_T          z_set_info_len);

/* Driver component exclusion notify function. */
typedef VOID (*x_rm_comp_excl_nfy_fct) (DRV_COMP_ID_T*  pt_pas_comp_id,
                                        DRV_COMP_ID_T*  pt_act_comp_id);


/* Driver function table */
typedef struct _DRV_COMP_FCT_TBL_T
{
    x_rm_connect_fct     pf_rm_connect;
    x_rm_disconnect_fct  pf_rm_disconnect;
    x_rm_get_fct         pf_rm_get;
    x_rm_set_fct         pf_rm_set;
}   DRV_COMP_FCT_TBL_T;

/* Component flags. */
#define DRV_FLAG_ASYNC_CONN_OR_DISC     ((UINT32) 0x00000001)
#define DRV_FLAG_SINGLE_CONN_ON_OUTPUT  ((UINT32) 0x00000002)

/* Resource Manager API return values */
#define RMR_ARBITRATION                ((INT32)    2)
#define RMR_ASYNC_NFY                  ((INT32)    1)
#define RMR_OK                         ((INT32)    0)
#define RMR_NOT_INIT                   ((INT32)   -1)
#define RMR_ALREADY_INIT               ((INT32)   -2)
#define RMR_INV_ARG                    ((INT32)   -3)
#define RMR_INV_HANDLE                 ((INT32)   -4)
#define RMR_INV_NAME                   ((INT32)   -5)
#define RMR_OUT_OF_HANDLES             ((INT32)   -6)
#define RMR_NO_COMP_FOUND              ((INT32)   -7)
#define RMR_INV_CONNECT                ((INT32)   -8)
#define RMR_INV_DISCONNECT             ((INT32)   -9)
#define RMR_INV_COND                   ((INT32)  -10)
#define RMR_DUPLICATE_COMP_ID          ((INT32)  -11)
#define RMR_DUPLICATE_NAME             ((INT32)  -12)
#define RMR_INV_COMP_ID                ((INT32)  -13)
#define RMR_INV_LIST_ENTRY             ((INT32)  -14)
#define RMR_NOT_ENOUGH_SPACE           ((INT32)  -15)
#define RMR_DRV_ERROR                  ((INT32)  -16)
#define RMR_NO_RIGHTS                  ((INT32)  -17)
#define RMR_NOT_ENOUGH_MEM             ((INT32)  -18)
#define RMR_OUT_OF_RESOURCES           ((INT32)  -19)
#define RMR_HARD_WIRED_CONFLICT        ((INT32)  -20)
#define RMR_SUPPORT_COMP_CONFLICT      ((INT32)  -21)
#define RMR_INV_PORT_NUMBER            ((INT32)  -22)
#define RMR_INV_OBJ_TO_COMP_REL        ((INT32)  -23)
#define RMR_INV_CTRL_TYPE              ((INT32)  -24)
#define RMR_PIPE_IS_CLOSING            ((INT32)  -25)
#define RMR_SINGLE_INP_PORT_COMP       ((INT32)  -26)
#define RMR_MULTI_INP_PORT_COMP        ((INT32)  -27)
#define RMR_DUPLICATE_PORT             ((INT32)  -28)
#define RMR_COMP_IS_CLOSING            ((INT32)  -29)
#define RMR_COMP_NOT_CONNECTED         ((INT32)  -30)
#define RMR_HARD_WIRED                 ((INT32)  -31)
#define RMR_CONN_CONFLICT              ((INT32)  -32)
#define RMR_CONNECT_FAILED             ((INT32)  -33)
#define RMR_DISCONNECT_FAILED          ((INT32)  -34)
#define RMR_INV_SET                    ((INT32)  -35)
#define RMR_INV_GET                    ((INT32)  -36)
#define RMR_COMP_EXCL_NFY_SET          ((INT32)  -37)
#define RMR_NO_GET_INFO                ((INT32)  -38)
#define RMR_DUPLICATE_MC_NFY_FCT_TBL   ((INT32)  -39)
#define RMR_DUPLICATE_MC_NFY_DEV_ID    ((INT32)  -40)
#define RMR_UNKNOWN_MC_NFY_DEV_ID      ((INT32)  -41)
#define RMR_DUPLICATE_MC_NFY_INSTANCE  ((INT32)  -42)
#define RMR_FAILED                     ((INT32)  -43)

#define RMR_DRV_INV_CONN_INFO          ((INT32) -256)
#define RMR_DRV_INV_DISC_INFO          ((INT32) -257)
#define RMR_DRV_INV_GET_INFO           ((INT32) -258)
#define RMR_DRV_INV_SET_INFO           ((INT32) -259)
#define RMR_DRV_CONN_FAILED            ((INT32) -260)
#define RMR_DRV_DISC_FAILED            ((INT32) -261)
#define RMR_DRV_SET_FAILED             ((INT32) -262)
#define RMR_DRV_GET_FAILED             ((INT32) -263)
#define RMR_DRV_NO_GET_INFO            ((INT32) -264)
#define RMR_DRV_NOT_ENOUGH_SPACE       ((INT32) -265)


/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

#if (1 == CONFIG_SECTION_BUILD)
extern INT32 rm_init (UINT16           ui2_num_comps,
                      THREAD_DESCR_T*  pt_thread_descr);
extern INT32 rm_uninit(VOID);

#endif

#if (1 == CONFIG_SECTION_BUILD_LINUX_KO)
extern INT32 rm_krn_init(void);
extern INT32 rm_krn_uninit(void);
extern INT32 rm_krn_reg_group_type(const Group_Type_T e_group_type, const DRV_COMP_REG_T* pt_comp_ids, UINT16 ui2_num_entries);
extern INT32 rm_krn_reg_cmp(DRV_COMP_REG_T* pt_comp_id, Group_Type_T e_combgrp_type, UINT32 ui4_comp_flags, DRV_COMP_FCT_TBL_T* pt_comp_fct_tbl);
extern INT32 rm_krn_unreg_cmp(DRV_TYPE_T eDrvType);
#endif

#if (1 == CONFIG_SECTION_BUILD_LINUX_PROG)
extern INT32 rm_usr_init(void);
extern INT32 rm_usr_uninit(void);
extern INT32 rm_usr_open_comp(DRV_TYPE_T e_type, UINT16 ui2_id, const CHAR* ps_name, HANDLE_T* ph_comp, RM_COND_T* pe_cond);
extern INT32 rm_usr_close(HANDLE_T h_obj, RM_COND_T*  pe_cond);
extern INT32 rm_usr_connect(HANDLE_T h_obj, const VOID* pv_conn_info, RM_COND_T* pe_cond);
extern INT32 rm_usr_disconnect(HANDLE_T h_obj, RM_COND_T* pe_cond);
extern INT32 rm_usr_set(HANDLE_T h_obj, DRV_SET_TYPE_T e_set_type, const VOID* pv_set_info, SIZE_T z_set_info_len);
extern INT32 rm_usr_get(HANDLE_T h_obj, DRV_GET_TYPE_T e_get_type, VOID* pv_get_info, SIZE_T* pz_get_info_len);
extern INT32 rm_usr_num_comps(DRV_TYPE_T e_type, UINT16* pui2_num_comps);
extern INT32 rm_usr_get_comp_info(DRV_TYPE_T e_type, UINT16 ui2_idx, UINT16* pui2_id, UINT32* pui4_comp_flags);
extern INT32 rm_usr_get_comp_info_from_handle(HANDLE_T h_obj, DRV_TYPE_T* pe_type, UINT16* pui2_id, UINT32* pui4_comp_flags);
extern RM_COND_T rm_usr_cond_chg(RM_COND_T e_new_cond, RM_COND_T e_old_cond, COND_GRP_T e_cond_grp);
#endif

#if (1 == RM_LIGHT_SUPPORT || 1 == CONFIG_SECTION_BUILD_LINUX_KO)
extern INT32 x_rm_reg_group_type (const Group_Type_T     e_group_type,
                                  const DRV_COMP_REG_T*  pt_comp_ids,
                                  UINT16                 ui2_num_entries);
#endif

extern INT32 x_rm_reg_comp (DRV_COMP_REG_T*      pt_comp_id,
                            UINT8                ui1_num_inp_ports,
                            UINT8                ui1_num_out_ports,
                            const CHAR*          ps_comp_name,
                            UINT32               ui4_comp_flags,
                            DRV_COMP_FCT_TBL_T*  pt_comp_fct_tbl,
                            const VOID*          pv_comp_data,
                            SIZE_T               z_comp_data_len);
extern INT32 x_rm_unreg_comp(DRV_TYPE_T eDrvType);

extern INT32 x_rm_get_comp_info_from_name (const CHAR*  ps_name,
                                           UINT16       ui2_idx,
                                           DRV_TYPE_T*  pe_type,
                                           UINT16*      pui2_id,
                                           UINT8*       pui1_num_inp_ports,
                                           UINT8*       pui1_num_out_ports,
                                           UINT32*      pui4_comp_flags);
extern INT32 x_rm_get_num_comps_of_type_from_name (const CHAR*  ps_name,
                                                   DRV_TYPE_T   e_type,
                                                   UINT16*      pui2_num_comps);
extern INT32 x_rm_reg_conn_list (const DRV_COMP_REG_T*  pt_comp_id,
                                 const DRV_COMP_REG_T*  pt_conn_ids,
                                 UINT16                 ui2_num_entries,
                                 CONN_DIR_TYPE_T        e_conn_dir,
                                 BOOL                   b_hard_wired);
extern INT32 x_rm_reg_comp_excl_list (const DRV_COMP_REG_T*  pt_comp_id,
                                      const DRV_COMP_REG_T*  pt_comp_excl_ids,
                                      UINT16                 ui2_num_entries);
extern INT32 x_rm_reg_comp_excl_nfy (const DRV_COMP_REG_T*   pt_comp_ids,
                                     UINT16                  ui2_num_entries,
                                     x_rm_comp_excl_nfy_fct  pf_rm_comp_excl_nfy);
extern INT32 x_rm_reg_conn_excl_list (const DRV_COMP_REG_T*  pt_comp_id,
                                      const DRV_COMP_REG_T*  pt_conn_excl_ids,
                                      UINT16                 ui2_num_entries);
extern INT32 x_rm_reg_group_name (const CHAR*            ps_group_name,
                                  const DRV_COMP_REG_T*  pt_comp_ids,
                                  UINT16                 ui2_num_entries);

#endif /* _X_RM_H */
