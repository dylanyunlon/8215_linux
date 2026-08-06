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
#ifndef _X_VID_DEC_H_
#define _X_VID_DEC_H_

#include <stddef.h>

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

//#include "u_pbinf.h"
//#include "drv_def.h"
//#include "drv_common.h"
#ifdef __linux__
//#include "drv_config.h"
#endif
/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/


/* Get operations */
#define VID_DEC_GET_TYPE_CTRL                (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 0))
#define VID_DEC_GET_TYPE_STC                 (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 1))
#define VID_DEC_GET_TYPE_RESOLUTION          (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 2))
#define VID_DEC_GET_TYPE_CAPABILITY          (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 3))
#define VID_DEC_GET_TYPE_PCR_ID              (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 4))
#define VID_DEC_GET_TYPE_CC_DATA             (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 5))
#define VID_DEC_GET_TYPE_I_FRAME_SURFACE     (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 6))
#define VID_DEC_GET_TYPE_PB_MODE             (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 7))
#define VID_DEC_GET_TYPE_PICTURE_INFO        (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 8))
#define VID_DEC_GET_TYPE_TRASFER_BITRATE     (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 9))
#define VID_DEC_GET_TYPE_SLOW_RWD_REBUF_INFO (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 10))
#define VID_DEC_GET_TYPE_ERROR_FRAME_COUNT   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 11))
#define VID_DEC_GET_TYPE_AU_COUNT            (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 12))

/* Set operations */
#define VID_DEC_SET_TYPE_CTRL               ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 0)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_DEC_SET_TYPE_NFY_FCT             (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 1))
#define VID_DEC_SET_TYPE_NFY_PTS             (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 2))
#define VID_DEC_SET_TYPE_INP_MODE            (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 3))
#define VID_DEC_SET_TYPE_PLAY_CC             (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 4))
#define VID_DEC_SET_TYPE_STOP_CC             (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 5))
#define VID_DEC_SET_TYPE_PCR_ID             ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 6)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_DEC_SET_TYPE_CAPTURE             (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 7))
#define VID_DEC_SET_TYPE_ALLOC_CC_BUFF      ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 8)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_DEC_SET_TYPE_FREE_CC_BUFF        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 9))
#define VID_DEC_SET_TYPE_FLUSH_CC_BUFF       (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 10))
#define VID_DEC_SET_TYPE_I_FRAME_BUFF        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 11))
#define VID_DEC_SET_TYPE_ALLOC_I_FRAME_BUFF ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 12)) | RM_SET_TYPE_GET_INFO)
#define VID_DEC_SET_TYPE_FREE_I_FRAME_BUFF   (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 13))
#define VID_DEC_SET_TYPE_I_FRAME_DECODE      (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 14))
#define VID_DEC_SET_TYPE_I_FRAME_NFY_FCT     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 15))
#define VID_DEC_SET_TYPE_PB_MODE             (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 16))
#define VID_DEC_SET_TYPE_INQUIRY            ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 17)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_DEC_SET_TYPE_DIGEST_ENABLE       (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 18))
#define VID_DEC_SET_TYPE_DIGEST_DISABLE      (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 19))
#define VID_DEC_SET_TYPE_DIGEST_WINDOW       (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 20))
#define VID_DEC_SET_TYPE_SEQUENCE_INFO       (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 21))
#define VID_DEC_SET_TYPE_SLOW_RWD_PARAMS     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 22)) /* initialization for slow rewind */
#define VID_DEC_SET_TYPE_SR_REBUF_STOP_DONE  (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 23)) /* notify driver the VID_DEC_COND_SLOW_RWD_REBUF_STOP is done */
#define VID_DEC_SET_TYPE_PAUSE_MODE         ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 24)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_DEC_SET_TYPE_ERR_DROP_LEVEL     ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 25)) | RM_SET_TYPE_ARG_NO_REF)
#define VID_DEC_SET_TYPE_PIP_EXIST            (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 26))

/*
#define VID_DEC_SET_TYPE_ONE_COLOR_FRAME     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 26))
*/

/* Registration Flags */
#define VID_DEC_REG_FLAG_NONE                     ((__u32) 0)
#define VID_DEC_REG_FLAG_USED_NORMAL_MODE         ((__u32) 1)
#define VID_DEC_REG_FLAG_USED_IN_QUAD_MODE        ((__u32) 2)

#define MAX_NUM_RPR_SIZES     16

/* VID_DEC_GET_TYPE_RESOLUTION settings **************************************/
#define VID_DEC_SRC_WIDTH_UNKNOWN                 ((__u16) 0xFFFF)
#define VID_DEC_SRC_HEIGHT_UNKNOWN                ((__u16) 0xFFFF)

#define VID_DEC_CAP_FMT_480I        (((__u32) 1) << (__u32)VID_DEC_FMT_480I)
#define VID_DEC_CAP_FMT_480P        (((__u32) 1) << (__u32)VID_DEC_FMT_480P)
#define VID_DEC_CAP_FMT_576I        (((__u32) 1) << (__u32)VID_DEC_FMT_576I)
#define VID_DEC_CAP_FMT_576P        (((__u32) 1) << (__u32)VID_DEC_FMT_576P)
#define VID_DEC_CAP_FMT_720P        (((__u32) 1) << (__u32)VID_DEC_FMT_720P)
#define VID_DEC_CAP_FMT_1080I       (((__u32) 1) << (__u32)VID_DEC_FMT_1080I)
#define VID_DEC_CAP_FMT_1080P       (((__u32) 1) << (__u32)VID_DEC_FMT_1080P)

/* VID_DEC_GET_TYPE_TRASFER_BITRATE *****************************************/
#define VID_DEC_BITRATE_UNKNOWN                 ((__u32) 0xFFFFFFFF)

/* VID_DEC_INFO_T  u4FlagKey type define******************************************/
#define VID_DEC_FLAGKEY_GET_EOS         (0x1 << 0)
#define VID_DEC_FLAGKEY_AU_HAVE_DATA    (0x1 << 1)
#define VID_DEC_FLAGKEY_GET_CSD         (0x1 << 2)

typedef __u32  VID_DEC_CAPABILITY_INFO_T;

typedef enum
{
    VID_DEC_PIP_NONE = 0,
    VID_DEC_PIP_EXIST
} VID_DEC_PIP_TYPE_T;


/* Notify conditions */
typedef enum
{
    VID_DEC_COND_ERROR = -1,
    VID_DEC_COND_CTRL_DONE,
    VID_DEC_COND_RESOLUTION_CHG,
    VID_DEC_COND_I_FRAME_SET_BUFF_DONE,
    VID_DEC_COND_I_FRAME_DECODE_DONE,
    VID_DEC_COND_INQUIRY_DONE,
    VID_DEC_COND_FRAME_ACCURATE_DONE,   /* will be obsolete, replace by VID_DEC_COND_CTRL_DONE */
    VID_DEC_COND_1ST_PIC_DISPLAY,
    VID_DEC_COND_CODEC_NOT_SUPPORT,
    VID_DEC_COND_DECODE_ERROR,
    VID_DEC_COND_SLOW_RWD_REBUF_REQ,
    VID_DEC_COND_SLOW_RWD_REBUF_STOP,
    VID_DEC_COND_I_FRAME_DISPLAY_DONE,
    VID_DEC_COND_INQUIRY_24P_DONE
} VID_DEC_COND_T;


/* VID_DEC_COND_INQUIRY_DONE */
typedef enum
{
    VID_DEC_INQUIRY_COND_ERROR = -1,
    VID_DEC_INQUIRY_COND_OK,
    VID_DEC_INQUIRY_COND_FAIL
} VID_DEC_INQUIRY_COND_T;


/* VID_DEC_COND_CODEC_NOT_SUPPORT */
typedef enum
{
    VID_DEC_CODEC_NOT_SUPPORT_COND_UNKNOWN = -1,
    VID_DEC_CODEC_NOT_SUPPORT_COND_MPEG_4_GMC,
    VID_DEC_CODEC_NOT_SUPPORT_COND_WMV_XINTRA8,
    VID_DEC_CODEC_NOT_SUPPORT_COND_WMV,
    VID_DEC_CODEC_NOT_SUPPORT_COND_DIVX_HD,
    VID_DEC_CODEC_NOT_SUPPORT_COND_OVER_SPEC
} VID_DEC_CODEC_NOT_SUPPORT_COND_T;


/* VID_DEC_COND_DECODE_ERROR */
typedef enum
{
    VID_DEC_DECODE_ERROR_TYPE_UNKNOWN = -1,
    VID_DEC_DECODE_ERROR_TYPE_NO_OUTPUT,
    VID_DEC_DECODE_ERROR_TYPE_SEQUENCE_DATA
} VID_DEC_DECODE_ERROR_T;


/* VID_DEC_SET_TYPE_SLOW_RWD_PARAMS settings *********************************/
typedef enum
{
    VID_DEC_SR_DATA_REQ_TYPE_PTS = 0,
    VID_DEC_SR_DATA_REQ_TYPE_OFFSET
} VID_DEC_SR_DATA_REQ_TYPE_T;


typedef struct _VID_DEC_SLOW_RWD_PARAMS_INFO_T
{
    VID_DEC_SR_DATA_REQ_TYPE_T e_req_type;
    __u32                     ui4_fifo_size;
} VID_DEC_SLOW_RWD_PARAMS_INFO_T;


/* VID_DEC_GET_TYPE_SLOW_RWD_REBUF_INFO settings *****************************/
typedef enum
{
    VID_DEC_SR_REBUF_TYPE_CURRENT = 0,
    VID_DEC_SR_REBUF_TYPE_PREVIOUS
} VID_DEC_SR_REBUF_TYPE_T;


typedef struct _VID_DEC_SR_REBUF_INFO_T
{
    __u64                     ui8_target_picture; /* unit in PTS or offset */
    VID_DEC_SR_REBUF_TYPE_T    e_rebuf_type;       /* if the rebuffer request is from previous sequence */
    __u32                     ui4_num_of_seq;     /* number of sequence */
} VID_DEC_SR_REBUF_INFO_T;


/* VID_DEC_SET_TYPE_CTRL settings ********************************************/
typedef enum
{
    VID_DEC_CTRL_RESET             = 0x00,
    VID_DEC_CTRL_STOP              = 0x01,
    VID_DEC_CTRL_FREEZE            = 0x02,
    VID_DEC_CTRL_PLAY              = 0x04,
    VID_DEC_CTRL_INQUIRY           = 0x05,
    VID_DEC_CTRL_SAME_STREAM_STOP  = 0x06,
    VID_DEC_CTRL_FILL_COLOR        = 0x07,
    VID_DEC_CTRL_PLAY_I_FRAME      = 0x08,
    VID_DEC_CTRL_INIT_I_FRAME      = 0x09,
    VID_DEC_CTRL_END_I_FRAME       = 0x0a,
    VID_DEC_CTRL_SR_TO_FWD_STOP    = 0x0b,
    VID_DEC_CTRL_INQUIRY_DVD_24P   = 0x0c
} VID_DEC_CTRL_T;


typedef enum
{
    VID_DEC_SRC_FRAME_RATE_UNKNOWN = 0,
    VID_DEC_SRC_FRAME_RATE_23_976, /* 24000/1001 (23.976...) */
    VID_DEC_SRC_FRAME_RATE_24,
    VID_DEC_SRC_FRAME_RATE_25,
    VID_DEC_SRC_FRAME_RATE_29_97, /* 30000/1001 (29.97...) */
    VID_DEC_SRC_FRAME_RATE_30,
    VID_DEC_SRC_FRAME_RATE_50,
    VID_DEC_SRC_FRAME_RATE_59_94, /* 60000/1001 (59.94...) */
    VID_DEC_SRC_FRAME_RATE_60,
    VID_DEC_SRC_FRAME_RATE_120,
    VID_DEC_SRC_FRAME_RATE_1,
    VID_DEC_SRC_FRAME_RATE_5,
    VID_DEC_SRC_FRAME_RATE_8,
    VID_DEC_SRC_FRAME_RATE_10,
    VID_DEC_SRC_FRAME_RATE_12,
    VID_DEC_SRC_FRAME_RATE_15,
    VID_DEC_SRC_FRAME_RATE_16,
    VID_DEC_SRC_FRAME_RATE_17,
    VID_DEC_SRC_FRAME_RATE_18,
    VID_DEC_SRC_FRAME_RATE_20,
    VID_DEC_SRC_FRAME_RATE_2,
    VID_DEC_SRC_FRAME_RATE_6,
    VID_DEC_SRC_FRAME_RATE_48,
    VID_DEC_SRC_FRAME_RATE_70,
    VID_DEC_SRC_FRAME_RATE_VARIABLE
} VID_DEC_SRC_FRAME_RATE_T;


/* Display Aspect Ratio (DAR) */
typedef enum
{
    VID_DEC_SRC_ASPECT_RATIO_UNKNOWN = 0,
    VID_DEC_SRC_ASPECT_RATIO_1_1,    /* 1 : 1 */
    VID_DEC_SRC_ASPECT_RATIO_4_3,
    VID_DEC_SRC_ASPECT_RATIO_4_3_LETTERBOX,
    VID_DEC_SRC_ASPECT_RATIO_16_9,
    VID_DEC_SRC_ASPECT_RATIO_2_21_1, /* 2.21 : 1 */
    VID_DEC_SRC_ASPECT_RATIO_UNDEFINED,   /* use for iframe decode */
    VID_DEC_SRC_ASPECT_RATIO_CUSTOMIZED   /* use only for ui2_sample_width and ui2_sample_height */
} VID_DEC_SRC_ASPECT_RATIO_T;


typedef struct _VID_DEC_RESOLUTION_INFO_T
{
    __u16                     ui2_width;
    __u16                     ui2_height;
    __u16                     ui2_frame_rate; /* will be replaced by e_frame_rate */
    VID_DEC_SRC_FRAME_RATE_T   e_frame_rate;
    bool                       b_is_progressive;
    VID_DEC_SRC_ASPECT_RATIO_T e_src_asp_ratio;
} VID_DEC_RESOLUTION_INFO_T;


/* VID_DEC_GET_TYPE_CAPABILITY settings **************************************/
typedef enum
{
    VID_DEC_FMT_UNKNOWN = 0,
    VID_DEC_FMT_480I,
    VID_DEC_FMT_480P,
    VID_DEC_FMT_576I,
    VID_DEC_FMT_576P,
    VID_DEC_FMT_720P,
    VID_DEC_FMT_1080I,
    VID_DEC_FMT_1080P
} VID_DEC_FMT_T;


#if 0//UNIFORM_DRV_CALLBACK
/* VID_DEC_SET_TYPE_NFY_FCT settings *****************************************/
typedef struct _VID_DEC_CB_DATA
{
    VID_DEC_COND_T      e_nfy_cond;
    __u32              ui4_data_1;
    __u32              ui4_data_2;
} VID_DEC_CB_DATA;
#else
typedef void (*x_vid_dec_nfy_fct) (
    void*               pv_nfy_tag,
    VID_DEC_COND_T      e_nfy_cond,
    __u32              ui4_data_1,
    __u32              ui4_data_2
    );

typedef struct _VID_DEC_NFY_INFO_T
{
    void*              pv_tag;
    x_vid_dec_nfy_fct  pf_vid_dec_nfy;
    void*              pv_previous_tag;          /* OUTPUT */
    x_vid_dec_nfy_fct  pf_previous_vid_dec_nfy;  /* OUTPUT */
}   VID_DEC_NFY_INFO_T;
#endif


/* VID_DEC_SET_TYPE_PLAY_CC settings *****************************************/
/* CC notify conditions */
typedef enum
{
    VID_DEC_CC_COND_ERROR = -1,
    VID_DEC_CC_COND_DATA_ARRIVAL
}   VID_DEC_CC_COND_T;

/* error codes used in VID_DEC_CC_COND_ERROR */
#define VID_DEC_CC_ERR_BUFF_OVER_FLOW       ((__u32) 1)

typedef void (*x_vid_dec_cc_nfy_fct) (
    void*                   pv_tag,
    VID_DEC_CC_COND_T       e_cc_cond,
    void*                   pv_arg
    );

typedef struct _VID_DEC_CC_NFY_INFO_T
{
    void*                       pv_tag;
    x_vid_dec_cc_nfy_fct        pf_cc_nfy;
} VID_DEC_CC_NFY_INFO_T;


/* VID_DEC_SET_TYPE_INP_MODE settings ****************************************/
/* Unused */
typedef enum
{
    VID_DEC_INP_MODE_NORMAL,
    VID_DEC_INP_MODE_PULL
} VID_DEC_INP_MODE_T;


typedef __s32 (*x_vid_dec_data_acquire_fct)(
    void*            pv_tag,
    __u8**          ppc_buff,
    size_t*          pz_buff_size
    );

typedef void (*x_vid_dec_data_release_fct)(
    void*            pv_tag,
    __u8*           pc_buff,
    size_t           z_buff_size
    );

typedef struct _VID_DEC_INP_MODE_INFO_T
{
    VID_DEC_INP_MODE_T           e_inp_mode;
    void*                        pv_tag;                      /* pull mode only */
    x_vid_dec_data_acquire_fct   pf_vid_dec_data_acquire_nfy; /* pull mode only */
    x_vid_dec_data_release_fct   pf_vid_dec_data_release_nfy; /* pull mode only */
} VID_DEC_INP_MODE_INFO_T;


/* VID_DEC_SET_TYPE_CAPTURE settings ****************************************/
/* Unused */
typedef struct _VID_DEC_CAPTURE_INFO_T
{
    __u8* /*__cross_space__*/    pc_frame_buff;
    size_t                    z_buff_size;
    bool                      b_is_progressive;
} VID_DEC_CAPTURE_INFO_T;


/* VID_DEC_SET_TYPE_I_FRAME_BUFF settings ***********************************/
/* VID_DEC_SET_TYPE_ALLOC_I_FRAME_BUFF **************************************/
/* VID_DEC_SET_TYPE_FREE_I_FRAME_BUFF ***************************************/
typedef struct _VID_DEC_I_FRAME_BUFF_T
{
    __u8* /*__cross_space__*/    pc_frame_buff_sp;
    size_t                    z_frame_size;
} VID_DEC_I_FRAME_BUFF_T;


/* VID_DEC_GET_TYPE_I_FRAME_SURFACE *****************************************/
/* Unused */
typedef struct _VID_DEC_GET_I_FRAME_SURFACE_T
{
    __u8*                     pc_y_buf;
    __u32                     ui4_y_pitch;
    __u8*                     pc_c_buf;
    __u32                     ui4_c_pitch;
} VID_DEC_GET_I_FRAME_SURFACE_T;


/* VID_DEC_GET_TYPE_PB_MODE *************************************************/
/* VID_DEC_SET_TYPE_PB_MODE *************************************************/
typedef enum _VID_DEC_PB_MODE_TYPE_T
{
    VID_DEC_PB_MODE_TYPE_NORMAL = 0,
    VID_DEC_PB_MODE_TYPE_NONE
} VID_DEC_PB_MODE_TYPE_T;


typedef enum
{
    VID_DEC_SPEED_TYPE_FR_120_00X          = -12000,
    VID_DEC_SPEED_TYPE_FR_100_00X          = -10000,
    VID_DEC_SPEED_TYPE_FR_64_00X           = -6400,
    VID_DEC_SPEED_TYPE_FR_50_00X           = -5000,
    VID_DEC_SPEED_TYPE_FR_32_00X           = -3200,  /* HBI_SpeedFast5(32X) */
    VID_DEC_SPEED_TYPE_FR_30_00X           = -3000,
    VID_DEC_SPEED_TYPE_FR_20_00X           = -2000,
    VID_DEC_SPEED_TYPE_FR_16_00X           = -1600,  /* HBI_SpeedFast4(16X) */
    VID_DEC_SPEED_TYPE_FR_10_00X           = -1000,
    VID_DEC_SPEED_TYPE_FR_08_00X           = -800,   /* HBI_SpeedFast3(8X) */
    VID_DEC_SPEED_TYPE_FR_04_00X           = -400,   /* HBI_SpeedFast2(4X) */
    VID_DEC_SPEED_TYPE_FR_02_00X           = -200,   /* HBI_SpeedFast1(2X) */
    VID_DEC_SPEED_TYPE_FR_01_00X           = -100,
    VID_DEC_SPEED_TYPE_SR_00_90X           = -90,
    VID_DEC_SPEED_TYPE_SR_00_80X           = -80,
    VID_DEC_SPEED_TYPE_SR_00_70X           = -70,
    VID_DEC_SPEED_TYPE_SR_00_60X           = -60,
    VID_DEC_SPEED_TYPE_SR_00_50X           = -50,    /* HBI_SpeedSlow1(1/2X) */
    VID_DEC_SPEED_TYPE_SR_00_25X           = -25,    /* HBI_SpeedSlow2(1/4X) */
    VID_DEC_SPEED_TYPE_SR_00_13X           = -13,    /* HBI_SpeedSlow3(1/8X) */
    VID_DEC_SPEED_TYPE_SR_00_06X           = -6,     /* HBI_SpeedSlow4(1/16X) */
    VID_DEC_SPEED_TYPE_SR_00_03X           = -3,     /* HBI_SpeedSlow5(1/32X) */
    VID_DEC_SPEED_TYPE_STEP_REVERSE        = -1,
    VID_DEC_SPEED_TYPE_PAUSE               = 0,
    VID_DEC_SPEED_TYPE_STEP                = 1,
    VID_DEC_SPEED_TYPE_SF_00_03X           = 3,      /* HBI_SpeedSlow5(1/32X) */
    VID_DEC_SPEED_TYPE_SF_00_06X           = 6,      /* HBI_SpeedSlow4(1/16X) */
    VID_DEC_SPEED_TYPE_SF_00_13X           = 13,     /* HBI_SpeedSlow3(1/8X) */
    VID_DEC_SPEED_TYPE_SF_00_25X           = 25,     /* HBI_SpeedSlow2(1/4X) */
    VID_DEC_SPEED_TYPE_SF_00_50X           = 50,     /* HBI_SpeedSlow1(1/2X) */
    VID_DEC_SPEED_TYPE_SF_00_60X           = 60,
    VID_DEC_SPEED_TYPE_SF_00_70X           = 70,
    VID_DEC_SPEED_TYPE_SF_00_80X           = 80,
    VID_DEC_SPEED_TYPE_SF_00_90X           = 90,
    VID_DEC_SPEED_TYPE_NORMAL              = 100,
    VID_DEC_SPEED_TYPE_FF_01_10X           = 110,
    VID_DEC_SPEED_TYPE_FF_01_20X           = 120,
    VID_DEC_SPEED_TYPE_FF_01_30X           = 130,
    VID_DEC_SPEED_TYPE_FF_01_40X           = 140,
    VID_DEC_SPEED_TYPE_FF_01_50X           = 150,
    VID_DEC_SPEED_TYPE_FF_02_00X           = 200,    /* HBI_SpeedFast1(2X) */
    VID_DEC_SPEED_TYPE_FF_04_00X           = 400,    /* HBI_SpeedFast2(4X) */
    VID_DEC_SPEED_TYPE_FF_08_00X           = 800,    /* HBI_SpeedFast3(8X) */
    VID_DEC_SPEED_TYPE_FF_10_00X           = 1000,
    VID_DEC_SPEED_TYPE_FF_16_00X           = 1600,   /* HBI_SpeedFast4(16X) */
    VID_DEC_SPEED_TYPE_FF_20_00X           = 2000,
    VID_DEC_SPEED_TYPE_FF_30_00X           = 3000,
    VID_DEC_SPEED_TYPE_FF_32_00X           = 3200,   /* HBI_SpeedFast5(32X) */
    VID_DEC_SPEED_TYPE_FF_50_00X           = 5000,
    VID_DEC_SPEED_TYPE_FF_64_00X           = 6400,
    VID_DEC_SPEED_TYPE_FF_100_00X          = 10000,
    VID_DEC_SPEED_TYPE_FF_120_00X          = 12000
} VID_DEC_SPEED_TYPE_T;


typedef enum
{
    VID_DEC_PIC_TYPE_IPB = 0,
    VID_DEC_PIC_TYPE_REF,
    VID_DEC_PIC_TYPE_I,
    VID_DEC_PIC_TYPE_1ST_I_AFTER_SEQ_HDR
} VID_DEC_PIC_TYPE_T;


typedef enum
{
    VID_DEC_FRAME_ACCURATE_TYPE_NONE = 0,
    VID_DEC_FRAME_ACCURATE_TYPE_PTS,
#if 0
    VID_DEC_FRAME_ACCURATE_TYPE_OFFSET,
#endif
    VID_DEC_FRAME_ACCURATE_TYPE_NUM
} VID_DEC_FRAME_ACCURATE_TYPE_T;

typedef struct _VID_DEC_FRAME_ACCURATE_PTS_INFO_T
{
    __u64              ui8_start;
    __u64              ui8_end;
} VID_DEC_FRAME_ACCURATE_PTS_INFO_T;

#if 0
typedef struct _VID_DEC_FRAME_ACCURATE_OFFSET_INFO_T
{
    __u64              ui8_start;
    __u64              ui8_end;
}   VID_DEC_FRAME_ACCURATE_OFFSET_INFO_T;
#endif

typedef struct _VID_DEC_FRAME_ACCURATE_NUM_INFO_T
{
    __u32              ui4_start;      /* minimum is 0 */
    __u32              ui4_end;        /* the last display frame is (ui4_end - 1) */
} VID_DEC_FRAME_ACCURATE_NUM_INFO_T;

typedef enum
{
   VID_DEC_EXTRA_SUPPORT_NONE = 0,
   VID_DEC_EXTRA_SUPPORT_DROPFRAMES,
   VID_DEC_EXTRA_SUPPORT_TRANSITEFF
} VID_DEC_EXTRA_SUPPORT_T;

typedef enum
{
    VID_DEC_TRANSITEFF_TYPE_CUT_IN = 0,
    VID_DEC_TRANSITEFF_TYPE_CUT_OUT,
    VID_DEC_TRANSITEFF_TYPE_FADE_IN,
    VID_DEC_TRANSITEFF_TYPE_FADE_OUT,
    VID_DEC_TRANSITEFF_TYPE_DISSOLVE,
    VID_DEC_TRANSITEFF_TYPE_WIPE_LEFT,
    VID_DEC_TRANSITEFF_TYPE_WIPE_RIGHT,
    VID_DEC_TRANSITEFF_TYPE_WIPE_TOP,
    VID_DEC_TRANSITEFF_TYPE_WIPE_BOTTOM,
    VID_DEC_TRANSITEFF_TYPE_DIAGONAL_LEFT,
    VID_DEC_TRANSITEFF_TYPE_DIAGONAL_RIGHT
} VID_DEC_TRANSITEFF_TYPE_T;

typedef struct _VID_DEC_EXTRA_SUPPORT_TRANSITEFF_INFO_T
{
    VID_DEC_TRANSITEFF_TYPE_T e_eff_type;
    __u16                    ui2_frame_index;
    __u32                    ui4_eff_duration;
} VID_DEC_EXTRA_SUPPORT_TRANSITEFF_INFO_T;

typedef struct _VID_DEC_PB_MODE_T
{
    VID_DEC_PB_MODE_TYPE_T              e_pb_mode_type;
    VID_DEC_SPEED_TYPE_T                e_speed_type;
    __s32                               i4_speed;       /* replace e_speed_type to let up-layer set speed value directly */
    VID_DEC_PIC_TYPE_T                  e_pic_type;

    VID_DEC_FRAME_ACCURATE_TYPE_T       e_frame_accurate_type;
    union
    {
        VID_DEC_FRAME_ACCURATE_PTS_INFO_T         t_frame_accurate_pts;
        VID_DEC_FRAME_ACCURATE_NUM_INFO_T         t_frame_accurate_num;
    } u;

    VID_DEC_EXTRA_SUPPORT_T             e_extra_support;
    union
    {
        //VID_DEC_EXTRA_SUPPORT_DROPFRAMES_INFO_T   t_extra_support_dropframes;
        VID_DEC_EXTRA_SUPPORT_TRANSITEFF_INFO_T   t_extra_support_transiteff;
    } s;
#if 0//BD_DRV_3D_SUPPORT
    bool                                           b_is_3D_mode;
    bool                                           b_is_lthenr;//switch L R video
#endif
} VID_DEC_PB_MODE_T;


/* VID_DEC_SET_TYPE_INQUIRY *************************************************/
typedef enum
{
    VID_DEC_INQUIRY_TYPE_UNKNOWN = 0,
    VID_DEC_INQUIRY_TYPE_SEQ_HDR,
    VID_DEC_INQUIRY_TYPE_GOP,
    VID_DEC_INQUIRY_TYPE_1ST_PIC_W_PTS
} VID_DEC_INQUIRY_TYPE_T;


/* VID_DEC_SET_TYPE_ONE_COLOR_FRAME *****************************************/
typedef enum
{
    VID_DEC_CHROMA_TYPE_444 = 0,
    VID_DEC_CHROMA_TYPE_422,
    VID_DEC_CHROMA_TYPE_420
} VID_DEC_CHROMA_TYPE_T;


typedef struct _VID_DEC_ONE_COLOR_FRAME_INFO_T
{
    __u8                     ui1_r;
    __u8                     ui1_g;
    __u8                     ui1_b;
    VID_DEC_CHROMA_TYPE_T     e_chroma_format;
} VID_DEC_ONE_COLOR_FRAME_INFO_T;


/* VID_DEC_SET_TYPE_DIGEST_ENABLE *******************************************/
typedef enum
{
    VID_DEC_DIGEST_BG_TYPE_UNKNOWN= 0,
    VID_DEC_DIGEST_BG_TYPE_ONE_COLOR_FRAME,
    VID_DEC_DIGEST_BG_TYPE_CURR_DISPLAY_FRAME
} VID_DEC_DIGEST_BG_TYPE_T;


typedef struct _VID_DEC_DIGEST_ENABLE_INFO_T
{
    VID_DEC_DIGEST_BG_TYPE_T       e_background_type;
    union
    {
        VID_DEC_ONE_COLOR_FRAME_INFO_T t_one_color_frame; /* valid for VID_DEC_DIGEST_BG_TYPE_ONE_COLOR_FRAME */
    } u;
} VID_DEC_DIGEST_ENABLE_INFO_T;


/* VID_DEC_SET_TYPE_DIGEST_WINDOW *******************************************/
typedef struct _VID_DEC_DIGEST_WINDOW_INFO_T
{
    __u32                    ui4_x;
    __u32                    ui4_y;
    __u32                    ui4_width;
    __u32                    ui4_height;
} VID_DEC_DIGEST_WINDOW_INFO_T;


/* VID_DEC_SET_TYPE_SEQUENCE_INFO *******************************************/
typedef enum
{
    VID_DEC_SPEC_INFO_SEL_NONE = 0,
    VID_DEC_SPEC_INFO_SEL_WMV,
    VID_DEC_SPEC_INFO_SEL_AVC,
    VID_DEC_SPEC_INFO_SEL_RV
} VID_DEC_SPEC_INFO_SEL_T;


typedef struct _VID_DEC_WMV_SPECIAL_INFO_T
{
    __u32 ui4_codec_special_data_length;
} VID_DEC_WMV_SPECIAL_INFO_T;


typedef struct _VID_DEC_AVC_SPECIAL_INFO_T
{
    bool b_single_sps;
} VID_DEC_AVC_SPECIAL_INFO_T;


typedef enum
{
    RV_FID_REALVIDEO30,        /* 0 (00) */
    RV_FID_RV89COMBO           /* 1 (01) */
} EnumRVBitsVersion;


typedef struct _VID_DEC_RV_SPECIAL_INFO_T
{
    __u32 u4NumofRPRSize;                             //[IN] RPR Size Number
    EnumRVBitsVersion eRVBitVersion;                //[IN] BitVersion (Default: RV_FID_REALVIDEO30)
    __u32 u4RPRSize[MAX_NUM_RPR_SIZES];    //[IN] RPR Size
} VID_DEC_RV_SPECIAL_INFO_T;


typedef struct _VID_DEC_SEQUENCE_INFO_T
{
    __u16                     ui2_width;         /* [IN/OUT] horizontal_size_value */
    __u16                     ui2_height;        /* [IN/OUT] vertical_size_value */
    __u16                     ui2_sample_width;  /* [IN/OUT] dscl_horizontal_size */
    __u16                     ui2_sample_height; /* [IN/OUT] dscl_vertical_size */
    __u16                     ui2_frame_rate;    /* [IN/OUT] frame_rate_code */ /* will be replaced by e_frame_rate */
    __u8                       au1DiscId[32];    /* [IN/OUT] for recoding current DISC ID */
    VID_DEC_SRC_FRAME_RATE_T   e_frame_rate;      /* [IN/OUT] Frame Rate */
    VID_DEC_SRC_ASPECT_RATIO_T e_src_asp;         /* [IN/OUT] Display Asect Ratio (DAR) */
    VID_DEC_SPEC_INFO_SEL_T    e_spec_info_sel;   /* [IN]     special information select */
    union
    {
        VID_DEC_WMV_SPECIAL_INFO_T t_wmv;      /* [IN] */
        VID_DEC_AVC_SPECIAL_INFO_T t_avc;      /* [IN] */
        VID_DEC_RV_SPECIAL_INFO_T t_realvideo;    /* [IN] */
    } u_special_info;
} VID_DEC_SEQUENCE_INFO_T;

/* VID_DEC_GET_TYPE_PICTURE_INFO ********************************************/
typedef enum
{
    VID_DEC_PIC_INFO_TYPE_UNKNOWN= 0,
    VID_DEC_PIC_INFO_TYPE_CURR_DECODE,
    VID_DEC_PIC_INFO_TYPE_LATEST_DECODE_I,
    VID_DEC_PIC_INFO_TYPE_LATEST_GOP,
    VID_DEC_PIC_INFO_TYPE_LATEST_SEQ_HDR
} VID_DEC_PIC_INFO_TYPE_T;

typedef enum
{
    VID_DEC_PIC_YUV_420_MODE = 0,
    VID_DEC_PIC_YUV_422_MODE,
    VID_DEC_PIC_YUV_444_MODE
} VID_DEC_PIC_YUV_CHROMA_E;

typedef struct _VID_DEC_TIME_CODE_INFO_T
{
    __u8                     ui1_hours;    /* 0 - 23 */
    __u8                     ui1_minutes;  /* 0 - 59 */
    __u8                     ui1_seconds;  /* 0 - 59 */
    __u8                     ui1_pictures; /* 0 - 59 */
} VID_DEC_TIME_CODE_INFO_T;

typedef struct _VID_DEC_PIC_INFO_T
{
    __u64                   ui8_pts;
    bool                     b_open_b_pic;
    __s8                       i1QuantizationLevel;
    VID_DEC_PIC_YUV_CHROMA_E eYUVMode;
    __u8   u1Profile;
    __u8   u1Level;
} VID_DEC_PIC_INFO_T;

typedef struct _VID_DEC_PICTURE_INFO_T
{
    VID_DEC_PIC_INFO_TYPE_T   e_type;         /* [IN] */
    __u64                    ui8_offset;     /* [OUT] */
    union
    {
        VID_DEC_PIC_INFO_T       t_pic_info;  /* for VID_DEC_PIC_INFO_TYPE_CURR_DECODE/VID_DEC_PIC_INFO_TYPE_LATEST_DECODE_I */
        VID_DEC_TIME_CODE_INFO_T t_time_code; /* for VID_DEC_PIC_INFO_TYPE_LATEST_GOP */
        VID_DEC_SEQUENCE_INFO_T  t_seq_info;  /* for VID_DEC_PIC_INFO_TYPE_LATEST_SEQ_HDR */
    } u;                                      /* [OUT] */
//    PBINF_V* /*__local_space__*/  pt_pbinf;       /* [IN]/[OUT] */
    __u64                    ui8_custom_1;   /* [OUT] */
    __u64                    ui8_custom_2;   /* [OUT] */
} VID_DEC_PICTURE_INFO_T;


/* VID_DEC_SET_TYPE_PAUSE_MODE **********************************************/
typedef enum
{
    VID_DEC_PAUSE_MODE_1 = 0,
    VID_DEC_PAUSE_MODE_2,
    VID_DEC_PAUSE_MODE_3,
    VID_DEC_PAUSE_MODE_4
} VID_DEC_PAUSE_MODE_T;

/* VID_DEC_SET_TYPE_ERR_DROP_LEVEL ******************************************/
typedef enum
{
    VID_DEC_ERR_DROP_ALL = 0,  /* Level 0: Drop frame if there is error. (Default) */
    VID_DEC_ERR_DROP_I,        /* Level 1: Ignore P or B frame's error.  If I-VOP is error, then, drop I-VOP. */
    VID_DEC_ERR_DROP_NONE      /* Level 2: Show every pictures. (I, P, B) */
} VID_DEC_ERR_DROP_LEVEL_T;

typedef enum
{
    VID_FRAME_OK = 0,
    VID_FRAME_CANNOT_OUTPUT,
    VID_FRAME_FF_NOT_OUTPUT,
    VID_FRAME_GST_FF_NOT_OUTPUT,
    VID_FRAME_ERROR,
    VID_FRAME_ALLOC_MEM_FAILED,
    VID_FRAME_NOT_SUPPORT
} VID_FRAME_OUTTYPE_T;

typedef struct _DISPLAY_INFO
{
    __u32 dwPhyYAddr;
    uintptr_t ptrVirYAddr;
    __u32 dwPhyCAddr;
    uintptr_t ptrVirCAddr;

    __u32 u4ScanMode;

    __u32 u4PicWidth;
    __u32 u4PicHeight;
    __u32 u4AlignWidth;
    __u32 u4AlignHeight;
    __u32 u4CropWidth;
    __u32 u4CropHeight;
    __u32 u4CropTop;
    __u32 u4CropBottom;
    __u32 u4CropLeft;
    __u32 u4CropRight;

    VID_FRAME_OUTTYPE_T eOutType;

    __u8 szName[10];
    __u32 ucCheckSum;
    __s32 i4Rate;
    __u32 u4Duration;

    //DI Pull down param
    bool   bIsNormalPlay;
    bool   bIsProgressive;
    bool   bIsTopFieldFirst;
    bool   bIsRepeatFirstField;
    bool   bIsPullDownFlagValid;
} DISPLAY_INFO_T;

typedef struct _VID_FRAME_INFO
{
    DISPLAY_INFO_T rDisplay_Info;
    __u64 u8Start;
    __u32 u4Duration;
    __u32 u4NotGetAU;
} VID_FRAME_INFO_T;

typedef struct _VID_DEC_INFO
{
    uintptr_t ptrEsmInfo;
    uintptr_t ptrOutBuf;
    bool   bIsOurFilter;
    __u32 u4Width;
    __u32 u4Height;
    VID_DEC_SEQUENCE_INFO_T rSeqInfo;
    bool  bNewSegment;
    __s64 i8PBStartPts;
    __s32 i4FFRate;
    __u32 u4BitRate;
    bool  fgIsDiscontinuity;
    bool  fgIsRepeat;
    //u4FlagKey
    //bit[0]: whether get EOS or not;   1: get EOS; 0: no
    //bit[1]: whether AU has data or not;   1: have data; 0: no
    __u32 u4FlagKey;
    bool fgIsReconfig;
} VID_DEC_INFO_T;

typedef enum
{
    BUFFER_OK = 0,
    BUFFER_NOT_ENOUGH = 1,
    BUFFER_SIZE_CHANGE = 2
}VID_INPUT_BUFFER_STATUS;

typedef struct _VID_INPUT_ERROR_INFO
{
    VID_INPUT_BUFFER_STATUS eBufferStatus;

    //for size change
    __u32 u4SrcWidth;
    __u32 u4SrcHeight;
    __u32 u4CropWidth;
    __u32 u4CropHeight;
}VID_INPUT_ERROR_INFO_T;

typedef struct _VID_FREE_INFO
{
    __u32 u4YPhyAddr;
    __u32 u4CPhyAddr;
} VID_FREE_INFO_T;

typedef struct _VID_WC_INFO
{
    __u32 u4SrcWidth;
    __u32 u4SrcHeight;
} VID_WC_INFO_T;

typedef struct _VID_DECBUF_INFO
{
    __u32 u4YPhyAddr;
    __u32 u4CPhyAddr;
    __u32 u4BufNum;
} VID_DECBUF_INFO_T;

/// Picture information of a media sample
typedef struct _MTK_PIC_IFO
{
    __u32 u4VType;              ///< Byte3~2: Extra Type; Byte 1~0: picture coded type, please refer drv_Common.h
    uintptr_t ptrSAddr;                ///< start address in fifo
    uintptr_t ptrEAddr;              ///< end address in fifo
    __u64 u8Pts;                   ///< PTS
    __u64 u8PrevPTS;          ///< Previous PTS
    __u64 u8Dts;                  ///< DTS
    __u64 u8Offset;              ///< Lba or file offset
    __u64 u8SoftPts;            ///< Soft PTS
    //PBINF_V rPbInf;               ///< Playback information
    __u32 u4SeqHdrSa;      ///< Sequence Header Starting Address //Only for WMV789 (No Start Code WMV Source)
    __u32 u4SeqHdrLen;      ///< Sequence Header Data Length //Only for WMV789 (No Start Code WMV Source)
    __u32 u4CCSa;               ///< Start address of user data which contains closed caption
    //DiscType eDiscType;        ///< disc type, please refer drv_common.h
    __u32 u4Duration;         ///< For Variable Frame Rate File Format
    __u32 u4PrevDuration;   ///< For Variable Frame Rate File Format
    __u32 u4WMVSliceAddr[3];   ///< WMV Slice Address information
    bool   fgxvColor;
    __u32 u4xvColorR;     ///< xvColor: R data
    __u32 u4xvColorG;     ///< xvColor: G data
    __u32 u4xvColorB;     ///< xvColor: B data

    __u32 u4RMSliceNum;    ///< only for rm
    __u32 auRM4SliceSize[128]; ///< only for rm

    __u8 ucPicStruct;
} MTK_PicInfo;

//parameter for parser
typedef struct _DMUX_INFO_
{
    __u8   ucMpvId;        //the id of the vld
    __u8   ucVCodec;
    __u32  u4SFifo;        ///< start address in fifo
    __u32  u4EFifo;        ///< end address in fifo
    MTK_PicInfo rPicInfo;
    VID_DEC_SEQUENCE_INFO_T rSeqInfo;
} VID_DMX_INFO_T;


typedef enum
{
    VID_DEC_CLI_TEST = 0,
    VID_DEC_CLI_HELP,
    VID_DEC_CLI_QUERY_VDEC_STATE,
    VID_DEC_CLI_CMD_LOG,
    VID_DEC_CLI_DUMP_AU,
    VID_DEC_CLI_PRINT_PTS,
    VID_DEC_CLI_DUMP_SPS,
    VID_DEC_CLI_DUMP_PPS,
    VID_DEC_CLI_DUMP_FRMBUF,
    VID_DEC_CLI_DUMP_YUV,
    VID_DEC_CLI_LOG_LEVEL,
    VID_DEC_CLI_FILE_NAME,
}VID_DEC_CLI_TYPE;

typedef struct {

    VID_DEC_CLI_TYPE eVidCliType;

    __u32    u4arg1;//u4InputID;
    __u32    u4arg2;//u4Len;
    __u32    u4arg3;//u4Size;
    __u32    u4arg4;//u4Value;
    void    **ptParam;//filename

}VID_DEC_CLI_CFG;

typedef enum
{
    VID_PROGRAM_ONESEG = 0,
    VID_PROGRAM_FULLSEG,
    VID_PROGRAM_FILE,
    VID_PROGRAM_WFD,
    VID_PROGRAM_DVR,
    VID_PROGRAM_WELINK,
    VID_PROGRAM_MEMORY,
    VID_PROGRAM_CARLIFE,
}VID_PROGRAM_TYPE;

typedef struct
{
    VID_PROGRAM_TYPE eProgramType;
    __u32 u4Reserved;
}VID_DEC_PARAM_CFG_T;

typedef enum
{
    VID_PARAM_UNKNOWN = 0,
    // set param
    VID_PARAM_SET_VERIFY_PARA,

    // get param
    VID_PARAM_GET_PTS,
    VID_PARAM_GET_VERIFY_RESULT,
    VID_PARAM_GET_EFUSE_SUPPORT_CODEC,
    VID_PARAM_QUERY_VDEC_USED,
    VID_PARAM_GET_DVR_ACK,
}VID_PARAM_TYPE;

typedef struct {

    VID_PARAM_TYPE eVidParamType;
    void    *pParamData;
}VID_DEC_PARAM;


/*! \name Source aspect ratio
* @{
*/
#define ENUM_SRC_ASPECT_RATIO

typedef enum {
    SRC_ASP_UNKNOW = 0,
    SRC_ASP_1_1,
    SRC_ASP_4_3_FULL,
    SRC_ASP_14_9_LB,
    SRC_ASP_14_9_LB_T,
    SRC_ASP_16_9_LB,
    SRC_ASP_16_9_LB_T,
    SRC_ASP_16_9_LB_G,
    SRC_ASP_14_9_FULL,
    SRC_ASP_16_9_FULL,
    SRC_ASP_221_1,
    SRC_ASP_16_9_PS,
    SRC_ASP_UNDEFINED,
    SRC_ASP_CUSTOMIZED,
    SRC_ASP_MAX
} SOURCE_ASPECT_RATIO_T;
/*! @} */

typedef struct _VID_ASPECT_RATIO_
{
    SOURCE_ASPECT_RATIO_T eVideoAspectRatio;
    __u32 u4Width;
    __u32 u4Height;
}VID_ASPECT_RATIO_T;

typedef enum
{
    VC_UNKNOW = 0,           ///< unknow type, used for debug
    VC_MPEG2,                ///< mpeg 1/2
    VC_MPEG4,                ///< mpeg 4
    VC_DIVX3,              ///< Divx 3.11
    VC_DIVX4,                ///< Divx 4
    VC_DIVX6,                ///< Divx 5/6
    VC_WMV1,                 ///< WMV7
    VC_WMV2,                 ///< WMV8
    VC_WMV3,                 ///< WMV9
    VC_VC1,                  ///< VC1
    VC_H263,                 ///< H.263
    VC_H263_SORENSON,		 ///< H.263 Sorenson version
    VC_H264,                 ///< H.264 (AVC)
    VC_RV30,                 ///< Real Video 8
    VC_RV40,                 ///< Real Video 9,10
    VC_MJPEG,                ///< motion jpeg
    VC_VP6,                  ///< VP6
    VC_VP6A,                 ///< VP6 with alpha.
    VC_VP8,                  ///< VP8
    VC_H265                  ///< H265 (HEVC)
}VCodeC;

#endif /* _X_VID_DEC_H_ */

