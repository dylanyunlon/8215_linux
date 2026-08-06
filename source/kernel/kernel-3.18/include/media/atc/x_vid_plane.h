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
#ifndef _X_VID_PLANE_H_
#define _X_VID_PLANE_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_pbinf.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
/* Get operations */
#define VID_PLA_GET_TYPE_CTRL              (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 0))
#define VID_PLA_GET_TYPE_MODE              (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 1))
#define VID_PLA_GET_TYPE_BG                (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 2))
#define VID_PLA_GET_TYPE_DISP_FMT          (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 3))
#define VID_PLA_GET_TYPE_DISP_REGION       (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 4))
#define VID_PLA_GET_TYPE_SRC_REGION        (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 5))
#define VID_PLA_GET_TYPE_BLENDING          (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 6))
#define VID_PLA_GET_TYPE_ENHANCE           (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 7))
#define VID_PLA_GET_TYPE_CAPABILITY        (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 8))
#define VID_PLA_GET_TYPE_QV_INP            (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 9))
#define VID_PLA_GET_TYPE_BRIGHTNESS        (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)10))
#define VID_PLA_GET_TYPE_CONTRAST          (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)11))
#define VID_PLA_GET_TYPE_HUE               (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)12))
#define VID_PLA_GET_TYPE_SATURATION        (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)13))
#define VID_PLA_GET_TYPE_CTI               (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)14))
#define VID_PLA_GET_TYPE_ETI               (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)15))
#define VID_PLA_GET_TYPE_SHARPNESS         (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)16))
#define VID_PLA_GET_TYPE_COLOR_SUPPRESS    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)17))
#define VID_PLA_GET_TYPE_OVER_SCAN_CLIPPER (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)18))
#define VID_PLA_GET_TYPE_MIN_MAX           (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)19))
#define VID_PLA_GET_TYPE_COLOR_GAIN        (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)20))
#define VID_PLA_GET_TYPE_COLOR_GAIN_MAX    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)21))
#define VID_PLA_GET_TYPE_COLOR_GAIN_MIN    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)22))
#define VID_PLA_GET_TYPE_COLOR_OFFSET      (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)23))
#define VID_PLA_GET_TYPE_COLOR_OFFSET_MAX  (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)24))
#define VID_PLA_GET_TYPE_COLOR_OFFSET_MIN  (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)25))
#define VID_PLA_GET_TYPE_NR                (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)26))
#define VID_PLA_GET_TYPE_BLACK_LVL_EXT     (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)27))
#define VID_PLA_GET_TYPE_WHITE_PEAK_LMT    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)28))
#define VID_PLA_GET_TYPE_FLESH_TONE        (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)29))
#define VID_PLA_GET_TYPE_LUMA              (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)30))
#define VID_PLA_GET_TYPE_MAX               (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)31))
#define VID_PLA_GET_TYPE_3D_NR             (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)32))
#define VID_PLA_GET_TYPE_FINAL_DISP_REGION (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)33))
#define VID_PLA_GET_TYPE_FINAL_SRC_REGION  (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)34))
#define VID_PLA_GET_TYPE_LETTER_BOX_DETECT (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)35))
#define VID_PLA_GET_TYPE_DYNAMIC_SCALING   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)36))
#define VID_PLA_GET_TYPE_PICTURE_INFO      (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)37))
#define VID_PLA_GET_TYPE_SCALE_FAC_RAGNGE  (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)38))
#define VID_PLA_GET_TYPE_CHK_DISP_REGION   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)39))
#define VID_PLA_GET_TYPE_CHK_SRC_REGION    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)40))
#define VID_PLA_GET_TYPE_BG_RESOLUTION     (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)41))
#define VID_PLA_GET_TYPE_OPTION            (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)42))
#define VID_PLA_GET_SAMPLE_BASED_ASPECT_RATIO (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)43))
#define VID_PLA_GET_DROP_CNT               (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)44))

/* Set operations */
#define VID_PLA_SET_TYPE_CTRL             ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 0)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_MODE             ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 1)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_BG                (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 2))
#define VID_PLA_SET_TYPE_DISP_FMT         ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 3)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_DISP_REGION       (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 4))
#define VID_PLA_SET_TYPE_SRC_REGION        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 5))
#define VID_PLA_SET_TYPE_BLENDING         ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 6)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_ENHANCE           (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 7))
#define VID_PLA_SET_TYPE_QV_INP            (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 8))
#define VID_PLA_SET_TYPE_BRIGHTNESS       ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 9)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_CONTRAST         ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)10)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_HUE              ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)11)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_SATURATION       ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)12)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_CTI              ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)13)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_ETI              ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)14)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_SHARPNESS        ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)15)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_COLOR_SUPPRESS   ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)16)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_OVER_SCAN_CLIPPER (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)17))
#define VID_PLA_SET_TYPE_COLOR_GAIN        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)18))
#define VID_PLA_SET_TYPE_COLOR_OFFSET      (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)19))
#define VID_PLA_SET_TYPE_NR               ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)20)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_BLACK_LVL_EXT    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)21)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_WHITE_PEAK_LMT   ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)22)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_FLESH_TONE       ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)23)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_LUMA             ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)24)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_3D_NR            ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)25)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_LETTER_BOX_DETECT ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)26)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_DYNAMIC_SCALING  ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)27)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_NFY_FCT           (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)28))
#define VID_PLA_SET_TYPE_BG_RESOLUTION     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)29))
#define VID_PLA_SET_TYPE_UP_LIMIT_LUMA_KEY (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)30))
#define VID_PLA_SET_TYPE_INJECT_VDP        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)32))
#define VID_PLA_SET_TYPE_FLUSH_FRAME_BUF   (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)34))
#define VID_PLA_SET_TYPE_OPTION           ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)35)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_CAPTURE           (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)36))
#define VID_PLA_SET_TYPE_PAUSE_MODE       ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)37)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_FULL_SCREEN   (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)38))
#define VID_PLA_SET_TYPE_IGNORE_CPS        ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)39)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_DISABLE_CPS       ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)40)) | RM_SET_TYPE_ARG_NO_REF)

// old UI setting - mtk01546
#define VID_PLA_SET_TYPE_POST_P_VIDEO_MODE ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)50)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_POST_P_SHARPNESS ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)51)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_POST_P_MAGNITUDE  (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)52))
#define VID_PLA_SET_TYPE_POST_P_SWITCH     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)53))
// new UI setting - mtk01546
//#define VID_PLA_SET_TYPE_POST_P_MAGNITUDE  (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)50))
//#define VID_PLA_SET_TYPE_POST_P_VIDEO_MODE (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)51))
#define VID_PLA_SET_TYPE_BNR               ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)54)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_MNR               ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)55)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_PLA_SET_TYPE_FNR               ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)56)) | RM_SET_TYPE_ARG_NO_REF)

#define VID_PLA_SET_VIDEO_BLACK            ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)57)))
/* Notify conditions */
typedef enum
{
    VID_PLA_COND_ERROR = -1,
    VID_PLA_COND_CTRL_DONE,
    VID_PLA_COND_INBAND_CMD_DONE,
    VID_PLA_COND_AUTO_PAUSE_DONE,
    VID_PLA_COND_INJECT_DONE,
    VID_PLA_COND_CAPTURE_DONE,
    VID_PLA_COND_TV_SYS_CHG,
    VID_PLA_COND_TRANSITION_EFF_DONE
}   VID_PLA_COND_T;

/* VID_PLA_SET_TYPE_NFY_FCT settings *****************************************/
#if UNIFORM_DRV_CALLBACK
typedef struct
{
  VID_PLA_COND_T      e_nfy_cond;
  UINT32              ui4_data_1;
  UINT32              ui4_data_2;
} VID_PLA_CB_PAYLOAD_T;

#else
typedef VOID (*x_vid_pla_nfy_fct) (
    VOID*               pv_nfy_tag,
    VID_PLA_COND_T      e_nfy_cond,
    UINT32              ui4_data_1,
    UINT32              ui4_data_2
    );

typedef struct _VID_PLA_NFY_INFO_T
{
    VOID*              pv_tag;
    x_vid_pla_nfy_fct  pf_vid_pla_nfy;
}   VID_PLA_NFY_INFO_T;
#endif
/* VID_PLA_GET_TYPE_CTRL/VID_PLA_SET_TYPE_CTRL Control settings. *************/
typedef enum
{
    VID_PLA_CTRL_RESET = 0,
    VID_PLA_CTRL_DISABLE,
    VID_PLA_CTRL_ENABLE,
    VID_PLA_CTRL_HIDE,
    VID_PLA_CTRL_UNHIDE
}   VID_PLA_CTRL_T;


/* VID_PLA_GET_TYPE_MODE/VID_PLA_SET_TYPE_MODE Plane modes *******************/
typedef enum
{
    VID_PLA_NORMAL = 0,
    VID_PLA_BLANK,
    VID_PLA_QUAD_VIDEO,
    VID_PLA_DEINT,
    VID_PLA_FREEZE
}   VID_PLA_MODE_T;

/* VID_PLA_GET_TYPE_DISP_FMT/VID_PLA_SET_TYPE_DISP_FMT display format ********/
typedef enum
{
    VID_PLA_DISP_FMT_NORMAL = 0,
    VID_PLA_DISP_FMT_LETTERBOX,
    VID_PLA_DISP_FMT_PAN_SCAN,
    VID_PLA_DISP_FMT_USER_DEFINED,
    VID_PLA_DISP_FMT_NON_LINEAR_ZOOM,
    VID_PLA_DISP_FMT_DOT_BY_DOT,
    VID_PLA_DISP_FMT_CUSTOM_0,
    VID_PLA_DISP_FMT_CUSTOM_1,
    VID_PLA_DISP_FMT_CUSTOM_2,
    VID_PLA_DISP_FMT_CUSTOM_3,
    VID_PLA_DISP_FMT_CUSTOM_4,
    VID_PLA_DISP_FMT_CUSTOM_5,
    VID_PLA_DISP_FMT_CUSTOM_6,
    VID_PLA_DISP_FMT_CUSTOM_7,
    VID_PLA_DISP_FMT_NLZ_CUSTOM_0, /* NONE LINEAR ZOOM */
    VID_PLA_DISP_FMT_NLZ_CUSTOM_1, /* NONE LINEAR ZOOM */
    VID_PLA_DISP_FMT_NLZ_CUSTOM_2, /* NONE LINEAR ZOOM */
    VID_PLA_DISP_FMT_NLZ_CUSTOM_3  /* NONE LINEAR ZOOM */
}   VID_PLA_DISP_FMT_T;

/* VID_PLA_GET_TYPE_BLENDING/VID_PLA_SET_TYPE_BLENDING ***********************/
typedef UINT8    VID_PLA_BLENDING_T;

/* VID_PLA_GET_TYPE_CAPABILITY Capability info *******************************/
#define VID_PLA_CAP_NONE                 ((UINT32) 0)
#define VID_PLA_CAP_MODE_NORMAL          MAKE_BIT_MASK_32( 0 )
#define VID_PLA_CAP_MODE_BLANK           MAKE_BIT_MASK_32( 1 )
#define VID_PLA_CAP_MODE_QUAD_VIDEO      MAKE_BIT_MASK_32( 2 )
#define VID_PLA_CAP_MODE_DEINT           MAKE_BIT_MASK_32( 3 )
#define VID_PLA_CAP_BG                   MAKE_BIT_MASK_32( 4 )
#define VID_PLA_CAP_DISP_FMT             MAKE_BIT_MASK_32( 5 )
#define VID_PLA_CAP_DISP_REGION          MAKE_BIT_MASK_32( 6 )
#define VID_PLA_CAP_SRC_REGION           MAKE_BIT_MASK_32( 7 )
#define VID_PLA_CAP_BLENDING             MAKE_BIT_MASK_32( 8 )
#define VID_PLA_CAP_ENHANCE              MAKE_BIT_MASK_32( 9 )
#define VID_PLA_CAP_BRIGHTNESS           MAKE_BIT_MASK_32( 10 )
#define VID_PLA_CAP_CONTRAST             MAKE_BIT_MASK_32( 11 )
#define VID_PLA_CAP_HUE                  MAKE_BIT_MASK_32( 12 )
#define VID_PLA_CAP_SATURATION           MAKE_BIT_MASK_32( 13 )
#define VID_PLA_CAP_CTI                  MAKE_BIT_MASK_32( 14 )
#define VID_PLA_CAP_ETI                  MAKE_BIT_MASK_32( 15 )
#define VID_PLA_CAP_SHARPNESS            MAKE_BIT_MASK_32( 16 )
#define VID_PLA_CAP_COLOR_SUPPRESS       MAKE_BIT_MASK_32( 17 )
#define VID_PLA_CAP_OVER_SCAN_CLIPPER    MAKE_BIT_MASK_32( 18 )
#define VID_PLA_CAP_COLOR_GAIN           MAKE_BIT_MASK_32( 19 )
#define VID_PLA_CAP_COLOR_OFFSET         MAKE_BIT_MASK_32( 20 )
#define VID_PLA_CAP_NR                   MAKE_BIT_MASK_32( 21 )
#define VID_PLA_CAP_BLACK_LVL_EXT        MAKE_BIT_MASK_32( 22 )
#define VID_PLA_CAP_WHITE_PEAK_LMT       MAKE_BIT_MASK_32( 23 )
#define VID_PLA_CAP_FLESH_TONE           MAKE_BIT_MASK_32( 24 )
#define VID_PLA_CAP_LUMA                 MAKE_BIT_MASK_32( 25 )

/* INJECT_VDP setting **********/
typedef enum
{
    VID_PLA_SRC_ASPECT_RATIO_UNKNOWN,
    VID_PLA_SRC_ASPECT_RATIO_4_3,
    VID_PLA_SRC_ASPECT_RATIO_16_9,
    VID_PLA_SRC_ASPECT_RATIO_2_21_1,
} VID_PLA_SRC_ASPECT_RATIO_T;

typedef struct _VID_PLA_INJECT_VDP_T
{
    VOID        *pv_src;
    VOID        *pv_src2;
    UINT32                     ui4_src_x;
    UINT32                     ui4_src_y;
    UINT32                     ui4_src_pitch;
    UINT32                     ui4_src2_pitch;
    UINT32                     ui4_dst_x;
    UINT32                     ui4_dst_y;
    UINT32                     ui4_width;
    UINT32                     ui4_height;
    // GL_COLORMODE_T             e_src_cm;
    VID_PLA_SRC_ASPECT_RATIO_T e_asp_ratio;
}   VID_PLA_INJECT_VDP_T;

/* VID_PLA_GET_TYPE_BG/VID_PLA_SET_TYPE_BG background color setting **********/
typedef struct _VID_PLA_BG_COLOR_T
{
    UINT8       ui1_r;
    UINT8       ui1_g;
    UINT8       ui1_b;
}   VID_PLA_BG_COLOR_T;

/* VID_PLA_GET_TYPE_DISP_REGION/VID_PLA_SET_TYPE_DISP_REGION *****************/
/* VID_PLA_GET_TYPE_SRC_REGION/VID_PLA_SET_TYPE_SRC_REGION *******************/
/* VID_PLA_GET_TYPE_CHK_DISP_REGION/VID_PLA_GET_TYPE_CHK_SRC_REGION **********/
typedef enum
{
    VID_PLA_REGION_TYPE_UNKNOWN = 0,
    VID_PLA_REGION_TYPE_PIXEL,
    VID_PLA_REGION_TYPE_PERMILLE,
/*
    SM_VSH_REGION_TYPE_PERCENT
*/
} VID_PLA_REGION_TYPE_T;

typedef struct _VID_PLA_VID_REGION_T
{
    BOOL        b_set_to_full_scr;
    VID_PLA_REGION_TYPE_T     e_region_type;      /* [IN] */
    UINT32                    ui4_x;              /* [IN/OUT] */
    UINT32                    ui4_y;              /* [IN/OUT] */
    UINT32                    ui4_width;          /* [IN/OUT] */
    UINT32                    ui4_height;         /* [IN/OUT] */
    UINT32                    ui4_delay;          /* [IN] */
}   VID_PLA_VID_REGION_T;

/* VID_PLA_SET_TYPE_BG_RESOLUTION ********************************************/
/* VID_PLA_GET_TYPE_BG_RESOLUTION ********************************************/
typedef struct _VID_PLA_BG_RESOLUTION_INFO_T
{
    UINT16                     ui2_width;
    UINT16                     ui2_height;
    VID_PLA_SRC_ASPECT_RATIO_T e_asp_ratio;
}   VID_PLA_BG_RESOLUTION_INFO_T;

/* VID_PLA_GET_TYPE_SCALE_FAC_RAGNGE data info *******************************/
typedef struct _VID_PLA_SCALE_FAC_RANGE_INFO_T
{
    FLOAT           f_min_hor_scaling;            /* [OUT] */
    FLOAT           f_max_hor_scaling;            /* [OUT] */
    FLOAT           f_min_ver_scaling;            /* [OUT] */
    FLOAT           f_max_ver_scaling;            /* [OUT] */
}   VID_PLA_SCALE_FAC_RANGE_INFO_T;

/* VID_PLA_GET_TYPE_ENHANCE/VID_PLA_SET_TYPE_ENHANCE *************************/
/* video plane enhance mode setting */
typedef enum
{
    VID_PLA_ENHANCE_DISABLE = 0,
    VID_PLA_ENHANCE,
    VID_PLA_BLUR
}   VID_PLA_ENHANCE_MODE_T;

typedef struct _VID_PLA_ENHANCE_T
{
    VID_PLA_ENHANCE_MODE_T    e_mode;
    UINT8                     ui1_level;
}   VID_PLA_ENHANCE_T;

/* VID_PLA_GET_TYPE_QV_INP/VID_PLA_SET_TYPE_QV_INP ***************************/
/* The input port definition of quad video */
typedef enum
{
    VID_PLA_QV_INP_NULL = 0,
    VID_PLA_QV_INP_1,           /* port 0 */
    VID_PLA_QV_INP_2,           /* port 1 */
    VID_PLA_QV_INP_3,           /* port 2 */
    VID_PLA_QV_INP_4            /* port 3 */
}   VID_PLA_QV_INP_T;

/* Quad-video info */
typedef struct _VID_PLA_QV_INFO_T
{
    VID_PLA_QV_INP_T        e_tl;
    VID_PLA_QV_INP_T        e_tr;
    VID_PLA_QV_INP_T        e_br;
    VID_PLA_QV_INP_T        e_bl;
}   VID_PLA_QV_INFO_T;

/* VID_PLA_GET_TYPE_MIN_MAX **************************************************/
typedef struct _VID_PLA_MIN_MAX_INFO_T
{
    UINT32      e_get_type;         /* get type */
    UINT32      ui4_min_value;      /* minimum */
    UINT32      ui4_max_value;      /* maximum */
} VID_PLA_MIN_MAX_INFO_T;

/* VID_PLA_GET_TYPE_COLOR_GAIN/VID_PLA_SET_TYPE_COLOR_GAIN *******************/
/* VID_PLA_GET_TYPE_COLOR_GAIN_MIN/VID_PLA_GET_TYPE_COLOR_GAIN_MAX ***********/
/* VID_PLA_GET_TYPE_COLOR_OFFSET/VID_PLA_SET_TYPE_COLOR_OFFSET ***************/
/* VID_PLA_GET_TYPE_COLOR_OFFSET_MIN/VID_PLA_GET_TYPE_COLOR_OFFSET_MAX *******/
typedef struct _VID_PLA_COLOR_GAIN_T
{
    UINT8       ui1_r_gain;
    UINT8       ui1_g_gain;
    UINT8       ui1_b_gain;
}   VID_PLA_COLOR_GAIN_T;

/* VID_PLA_GET_TYPE_OVER_SCAN_CLIPPER/VID_PLA_SET_TYPE_OVER_SCAN_CLIPPER *****/
typedef struct _VID_PLA_OVER_SCAN_CLIPPER_T
{
    UINT32      ui4_top;
    UINT32      ui4_bottom;
    UINT32      ui4_left;
    UINT32      ui4_right;
}   VID_PLA_OVER_SCAN_CLIPPER_T;

/* VID_PLA_GET_TYPE_MAX ******************************************************/
typedef struct _VID_PLA_MAX_INFO_T
{
    UINT32      e_get_type;         /* get type */
    UINT32      ui4_max_value;      /* maximum */
} VID_PLA_MAX_INFO_T;

/* VID_PLA_GET_TYPE_PICTURE_INFO *********************************************/
typedef enum
{
    VID_PLA_PIC_INFO_TYPE_UNKNOWN= 0,
    VID_PLA_PIC_INFO_TYPE_CURR_DISPLAY,
    VID_PLA_PIC_INFO_TYPE_LATEST_DISPLAY_I,
}   VID_PLA_PIC_INFO_TYPE_T;

typedef struct _VID_PLA_PICTURE_INFO_T
{
    VID_PLA_PIC_INFO_TYPE_T   e_type;       /* [IN] */
    UINT64                    ui8_offset;   /* [OUT] */
    UINT64                    ui8_pts;      /* [OUT] */
    BOOL                      b_open_b_pic; /* [OUT] */
    PBINF_V*   pt_pbinf;     /* [IN]/[OUT] */
    UINT64                    ui8_custom_1; /* [OUT] */
    UINT64                    ui8_custom_2; /* [OUT] */
}   VID_PLA_PICTURE_INFO_T;

/* VID_PLA_SET_TYPE_UP_LIMIT_LUMA_KEY ****************************************/
typedef UINT8    VID_PLA_LUMA_T;

typedef struct _VID_PLA_LUMA_KEY_INFO_T
{
    BOOL                      b_on;               /* [IN] on/off*/
    VID_PLA_LUMA_T            t_up_lmt_luma_key;  /* [IN] up_limit_luma_key */
}   VID_PLA_LUMA_KEY_INFO_T;

/* VID_PLA_SET_TYPE_CAPTURE **************************************************/
typedef enum
{
    VID_PLA_CAPTURE_NONE = 0,    /* to sync cancel the previous capture request */
    VID_PLA_CAPTURE_NTH_PIC,     /* to async capture the n-th picture after current rendering one */
    VID_PLA_CAPTURE_WITH_PTS     /* to async capture the picture with the designated PTS */
}   VID_PLA_CAPTURE_MODE_T;

typedef struct _VID_PLA_CAPTURE_INFO_T
{
    VID_PLA_CAPTURE_MODE_T    e_mode;
    union
    {
        UINT32    ui4_nth_pic;    /* VID_PLA_CAPTURE_NTH_PIC */
        UINT64    ui8_pts;        /* VID_PLA_CAPTURE_WITH_PTS */
    } u;

    /* destination buffer pointer */
    UCHAR*     pc_buffer_y;
    UCHAR*     pc_buffer_c;
    /* destination size */
    UINT16                    ui2_width;
    UINT16                    ui2_height;
}   VID_PLA_CAPTURE_INFO_T;


/* VID_PLA_GET_SAMPLE_BASED_ASPECT_RATIO ****************************************/
typedef struct
{
    UINT32  asp_width;
    UINT32 asp_height;
}  VID_PLA_SAMPLE_BASED_ASPECT_RATIO_T;

/* VID_PLA_SET_TYPE_PAUSE_MODE ****************************************/
typedef enum
{
    VID_PLA_PAUSE_MODE_AUTO = 0,
    VID_PLA_PAUSE_MODE_FRAME
}   VID_PLA_PAUSE_MODE_T;

/* VID_PLA_SET_TYPE_FULL_SCREEN ***********************************/
typedef struct
{
  BOOL b_full_screen_on;
}VID_PLA_SET_FULL_SCREEN_T;

/* VID_PLA_SET_TYPE_POST_P_VIDEO_MODE ****************************************/
typedef enum 
{
    VID_PLA_PP_STD_MODE = 0,
    VID_PLA_PP_VIVID_MODE,
    VID_PLA_PP_CINEMA_MODE,
    VID_PLA_PP_CUSTOMER_MODE,        
    VID_PLA_PP_OTHER_MODE   
        
}  VID_PLA_POST_P_VIDEO_MODE_T;

/* VID_PLA_SET_TYPE_POST_P_MAGNITUDE ****************************************/
typedef enum 
{
    VID_PLA_PP_BRIGHTNESS = 0,
    VID_PLA_PP_CONTRAST,
    VID_PLA_PP_SATURATION,
    VID_PLA_PP_HUE,
    VID_PLA_PP_SHARPNESS,
    VID_PLA_PP_CTI,
    VID_PLA_PP_ADAPTIVE_LUMA_ONOFF,
    VID_PLA_PP_SCE_ONOFF,
    VID_PLA_PP_COLOR_RED_Y,
    VID_PLA_PP_COLOR_RED_S,
    VID_PLA_PP_COLOR_RED_H,
    VID_PLA_PP_COLOR_GREEN_Y,
    VID_PLA_PP_COLOR_GREEN_S,
    VID_PLA_PP_COLOR_GREEN_H,
    VID_PLA_PP_COLOR_BLUE_Y,
    VID_PLA_PP_COLOR_BLUE_S,
    VID_PLA_PP_COLOR_BLUE_H,
    VID_PLA_PP_COLOR_YELLOW_Y,
    VID_PLA_PP_COLOR_YELLOW_S,
    VID_PLA_PP_COLOR_YELLOW_H,
    VID_PLA_PP_COLOR_CYAN_Y,
    VID_PLA_PP_COLOR_CYAN_S,
    VID_PLA_PP_COLOR_CYAN_H,
    VID_PLA_PP_COLOR_MAGENTA_Y,
    VID_PLA_PP_COLOR_MAGENTA_S,
    VID_PLA_PP_COLOR_MAGENTA_H,    
}  VID_PLA_POST_P_MAGNITUDE_TYPE_T;

typedef struct 
{
    VID_PLA_POST_P_MAGNITUDE_TYPE_T type;
    INT16   i2UiMin;
    INT16   i2UiMax;
    INT16   i2UiDft;
    INT16   i2UiCur;
}  VID_PLA_POST_P_MAGNITUDE_T;

/* VID_PLA_GET_TYPE_NR, VID_PLA_SET_TYPE_NR *******************************/
typedef enum
{
    VID_PLA_NR_OFF = 0,
    VID_PLA_NR_LOW,
    VID_PLA_NR_MIDDLE,
    VID_PLA_NR_HIGH
}  VID_PLA_NR_STRENGTH_T;

/* VID_PLA_GET_TYPE_SHARP, VID_PLA_SET_TYPE_SHARP *******************************/
typedef enum
{
    VID_PLA_SHARP_OFF = 0,
    VID_PLA_SHARP_LV1,
    VID_PLA_SHARP_LV2,
    VID_PLA_SHARP_LV3,
    VID_PLA_SHARP_LV4,
    VID_PLA_SHARP_LV5
}  VID_PLA_SHARP_STRENGTH_T;

/* VID_PLA_SET_VIDEO_BLACK *******************************/
typedef struct 
{
    UCHAR ucY;
    UCHAR ucCb;
    UCHAR ucCr;
}  VID_PLA_SET_VIDEO_COLOR_T;

#endif /* _X_VID_PLANE_H_ */

