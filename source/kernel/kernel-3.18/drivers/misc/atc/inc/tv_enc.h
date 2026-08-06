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

#ifndef _X_TV_ENC_H_
#define _X_TV_ENC_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

//#include "x_common.h"
//#include "x_rm.h"

//#include "x_drv_cb.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/


/* Get operations */
#define TV_ENC_GET_TYPE_CTRL         (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 0))
#define TV_ENC_GET_TYPE_FMT          (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 1))
#define TV_ENC_GET_TYPE_FMT_CAP      (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 2))
#define TV_ENC_GET_TYPE_TV_TYPE      (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 3))
#define TV_ENC_GET_TYPE_MV           (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 4))
#define TV_ENC_GET_TYPE_CCI          (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 5))

/* Set operations */
#define TV_ENC_SET_TYPE_CTRL        ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 0)) | RM_SET_TYPE_ARG_NO_REF)
#define TV_ENC_SET_TYPE_FMT         ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 1)) | RM_SET_TYPE_ARG_NO_REF)
#define TV_ENC_SET_TYPE_VBI_CONTENT  (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 2))
#define TV_ENC_SET_TYPE_TV_TYPE     ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 3)) | RM_SET_TYPE_ARG_NO_REF)
#define TV_ENC_SET_TYPE_MV           (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 4))
#define TV_ENC_SET_TYPE_CCI         ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 5)) | RM_SET_TYPE_ARG_NO_REF)
#if UNIFORM_DRV_CALLBACK
#define TV_ENC_SET_TYPE_NFY_FCT      (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 6)) //SCOM set notify function to TVE
#else
#define TV_ENC_SET_TYPE_NFY_FCT     ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 6)) | RM_SET_TYPE_ARG_NO_REF) //SCOM set notify function to TVE
#endif
#define TV_ENC_SET_TYPE_MUTE         ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 7)) | RM_SET_TYPE_ARG_NO_REF)
#define TV_ENC_SET_TYPE_IGONE_CPS   (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 8))

/* TV_ENC_GET_TYPE_CTRL/TV_ENC_SET_TYPE_CTRL data info ***********************/
/* Control settings. */
typedef enum
{
    TV_ENC_CTRL_RESET       = 0,
    TV_ENC_CTRL_DISABLE     = 0x1,  /* disable TV encoder */
    TV_ENC_CTRL_ENABLE      = 0x2   /* enable TV encoder */
}   TV_ENC_CTRL_T;

/* TV_ENC_GET_TYPE_FMT *******************************************************/
/* TV output format */
typedef enum
{
    TV_ENC_FMT_NULL = 0,
    TV_ENC_FMT_CVBS,
    TV_ENC_FMT_S_VIDEO,
    TV_ENC_FMT_YBR,
    TV_ENC_FMT_RGB
}   TV_ENC_FMT_T;

/* TV_ENC_GET_TYPE_FMT_CAP ***************************************************/
/* Capability of TV output formats */
#define TV_ENC_FMT_CAP_NULL			(((UINT32)1) << TV_ENC_FMT_NULL)
#define TV_ENC_FMT_CAP_CVBS			(((UINT32)1) << TV_ENC_FMT_CVBS)
#define TV_ENC_FMT_CAP_S_VIDEO		(((UINT32)1) << TV_ENC_FMT_S_VIDEO)
#define TV_ENC_FMT_CAP_YBR			(((UINT32)1) << TV_ENC_FMT_YBR)
#define TV_ENC_FMT_CAP_RGB			(((UINT32)1) << TV_ENC_FMT_RGB)

typedef UINT32 TV_ENC_FMT_CAP_INFO_T;

/* TV_ENC_SET_TYPE_VBI_CONTENT data info *************************************/
/* Field types */
typedef enum
{
    TV_ENC_FIELD_ODD = 0,
    TV_ENC_FIELD_EVEN,
    TV_ENC_FIELD_BOTH
}   TV_ENC_FIELD_TYPE_T;

/* VBI content info */
/* Unused */
typedef struct _TV_ENC_VBI_CONTENT_INFO_T
{
    TV_ENC_FIELD_TYPE_T e_field;
    CHAR *              ps_content;
}   TV_ENC_VBI_CONTENT_INFO_T;

/* TV_ENC_GET_TYPE_ASPECT_RATIO/TV_ENC_SET_TYPE_ASPECT_RATIO data info *******/
typedef enum
{
    TV_ENC_ASPECT_RATIO_4_3,
    TV_ENC_ASPECT_RATIO_14_9,
    TV_ENC_ASPECT_RATIO_16_9
}   TV_ENC_ASPECT_RATIO_INFO_T;

/* TV_ENC_GET_TYPE_TV_TYPE/TV_ENC_SET_TYPE_TV_TYPE data info *****************/
typedef enum
{
    TV_ENC_TV_TYPE_NTSC_M,
    TV_ENC_TV_TYPE_PAL_B,
    TV_ENC_TV_TYPE_PAL_G,
    TV_ENC_TV_TYPE_PAL_H,
    TV_ENC_TV_TYPE_PAL_I,
    TV_ENC_TV_TYPE_PAL_D,
    TV_ENC_TV_TYPE_PAL_N,
    TV_ENC_TV_TYPE_PAL_M,
    TV_ENC_TV_TYPE_SECAM_B,
    TV_ENC_TV_TYPE_SECAM_G,
    TV_ENC_TV_TYPE_SECAM_H,
    TV_ENC_TV_TYPE_SECAM_D,
    TV_ENC_TV_TYPE_SECAM_K,
    TV_ENC_TV_TYPE_SECAM_K1,
    TV_ENC_TV_TYPE_SECAM_L
}   TV_ENC_TV_TYPE_INFO_T;

/* TV_ENC_GET_TYPE_MV/TV_ENC_SET_TYPE_MV data info ***************************/
#define TV_ENC_MV_CPS_DATA_SIZE      ((SIZE_T) 17)

typedef enum
{
    TV_ENC_MV_TYPE_OFF                       = 0,
    TV_ENC_MV_TYPE_APS                       = 1,
    TV_ENC_MV_TYPE_CPC_CPS                   = 2
}   TV_ENC_MV_TYPE_T;

typedef enum
{
    TV_ENC_MV_APS_TYPE_OFF                   = 0,
    TV_ENC_MV_APS_TYPE_AGC_ONLY              = 1,
    TV_ENC_MV_APS_TYPE_AGC_WI_2L_SPLIT_BURST = 2,
    TV_ENC_MV_APS_TYPE_AGC_WI_4L_SPLIT_BURST = 3
}   TV_ENC_MV_APS_TYPE_T;

typedef struct _TV_ENC_MV_CPC_CPS_T
{
    UINT8               ui1_cpc_data;
    UINT8               aui1_cps_data[TV_ENC_MV_CPS_DATA_SIZE];
}   TV_ENC_MV_CPC_CPS_T;

typedef struct _TV_ENC_MV_INFO_T
{
    TV_ENC_MV_TYPE_T    e_mv_type;

    union
    {
        TV_ENC_MV_APS_TYPE_T     e_aps_type;
        TV_ENC_MV_CPC_CPS_T      t_cpc_cps_data;
    } u;
} TV_ENC_MV_INFO_T;

/* TV_ENC_GET_TYPE_CCI/TV_ENC_SET_TYPE_CCI data info *************************/
typedef enum
{
    TV_ENC_CCI_TYPE_NO_RESTRICT      = 0,
    TV_ENC_CCI_TYPE_NO_FURTHER_COPY  = 1,
    TV_ENC_CCI_TYPE_COPY_ONCE        = 2,
    TV_ENC_CCI_TYPE_COPY_NEVER       = 3
}   TV_ENC_CCI_TYPE_T;

//TV_ENC_SET_TYPE_MUTE

typedef enum
{
    TV_ENC_MUTE_TYPE_ON      = TRUE,
    TV_ENC_MUTE_TYPE_OFF  = FALSE,
}   TV_ENC_MUTE_TYPE_T;

#endif /* _X_TV_ENC_H_ */


/*-----------------------------------------------------------------------------
                    Notify function (TVE to SCOM)
 ----------------------------------------------------------------------------*/

 /* Notify conditions */
typedef enum
{
    TVE_DOT_UPDATED = 0,
    TVE_ICT_UPDATED,
    TVE_CSS_UPDATED,
    TVE_AACS_UPDATED,
    TVE_CGMSA_UPDATED,
    TVE_APS_UPDATED,
    TVE_EPN_UPDATED,
    TVE_NPCNT_UPDATED,
    TVE_DCICCI_UPDATED,
    TVE_CAV_OUT_RES_UPDATED,
    TVE_CGMS_UPDATED,   
    TVE_RCIRCD_UPDATED, 
	TVE_CPRM_UPDATED, 
	TVE_WMDRM_UPDATED, 
	TVE_DIVX_UPDATED, 	
	TVE_MV_UPDATED,     
	TVE_ASPECT_UPDATED      
}   TVE_COND_T;

typedef   struct  _TVE_INFO_T
{
  UINT8 u1TveNfyDOT;
  UINT8 u1TveNfyICT;
  UINT8 u1TveNfyCSS;
  UINT8 u1TveNfyAACS;
  UINT8 u1TveNfyCGMSA;
  UINT8 u1TveNfyAPS;
  UINT8 u1TveNfyEPN;
  UINT8 u1TveNfyNotPassCnt;
  UINT8 u1TveNfyDCICCI;
  UINT8 u1TveNfyCAVOutRes;
  UINT8 u1TveNfyCGMS;
  UINT8 u1TveNfyRCIRCD;
  UINT8 u1TveNfyCPRM;
  UINT8 u1TveNfyWMDRM;
  UINT8 u1TveNfyDIVX;
  UINT8 u1TveNfyMV;
  UINT8 u1TveNfyAspect;

}  TVE_INFO_T;


#if (!UNIFORM_DRV_CALLBACK)

/* Notify function */
typedef VOID (*x_tve_nfy_fct) (
  TVE_COND_T e_tve_nfy_cond,
  UINT8        u1_tve_nfy_info);

#else

typedef   struct  _TVE_CB_INFO_T
{
 TVE_COND_T e_tve_nfy_cond;
 UINT8 u1TVENfyInfo;
 }  TVE_CB_INFO_T;

#endif

//TV_ENC_SET_TYPE_IGONE_CPS
typedef enum
{
    TV_ENC_TV_TYPE_IGONE_CPS,
    TV_ENC_TV_TYPE_REFER_CPS
}   TV_ENC_TV_TYPE_IGONE_CPS_T;

