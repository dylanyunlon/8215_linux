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
#ifndef _X_IMG_DEC2_H_
#define _X_IMG_DEC2_H_


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include "x_gfx.h"
#include "x_img_dec.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "x_drv_cb.h"
#include "sys_config.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/*! \name Get Operations
* @{
*/
#define IMG_GET_TYPE_PROGRESS                       \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 1))
/*! @} */
    
/*! \name Set Operations
* @{
*/
#define IMG_SET_TYPE_FRM_START                      \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 1))
    
#define IMG_SET_TYPE_DECODE                         \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 2))
    
#define IMG_SET_TYPE_STOP                           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 3))
    
#define IMG_SET_TYPE_BUF_FILLED                     \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 4))
    
#define IMG_SET_TYPE_FRM_END                        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 5))

#define IMG_SET_TYPE_DIRECT_DECODE                  \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 6))
/*! @} */

/* Driver error codes */
#define IMG_DRV_HW_ERROR        (-1)
#define IMG_DRV_IO_ERROR        (-2)
#define IMG_DRV_NOT_SUPPORT     (-3)

/* MW buffer-filling return codes */
#define IMG_MW_FILL_OK          ( 0)
#define IMG_MW_FILL_EOF         (-1)
#define IMG_MW_FILL_ERROR       (-2)

/* data passed with IMG_NFY_FILL_BUF */
typedef struct
{
    __u32          ui4_trsn_id;        /* transaction id */
    void            *pv_start_add;      /* starting address */
    __u32          ui4_required_len;   /* required length */
    bool            b_reset_pos;        /* position-resetting flag */
    __u32          ui4_position;       /* the position to be resettd */
#if CONFIG_SYS_MEM_PHASE3
    __u32          u4Offset;           /* for memory phase III, handle replace address*/
#endif
} IMG_FILL_BUF_T;


#if UNIFORM_DRV_CALLBACK
typedef struct
{
    __u32 ui4_img_id;
    IMG_NFY_STATE_T e_state;
    union
    {
        void *pv_cache;
        IMG_FILL_BUF_T rFillBuf;
    }rInfo;
}IMG_NFY_PARAM_T;

#else
/* callback function */
typedef void (*IMG_GIF_NFY_FCT_T)
(
    __u32          ui4_img_id,         /* the image id which causes this notification */
    void            *pv_tag,            /* tag passed to the callback function */
    void            *pv_data,           /* data passed with this notification */
    IMG_NFY_STATE_T e_state);           /* notification state */
#endif

/* data passed with IMG_SET_TYPE_FRM_START */
typedef struct
{
    __u32          ui4_img_id;         /* the image id decoded */
    void            *pv_img_buf;        /* image data */
    __u32          ui4_img_size;       /* image size */
    void            *pv_aux_cache;      /* auxiliary cache data */
    #if UNIFORM_DRV_CALLBACK
    x_drv_cb_nfy_fct pf_func;
    #else
    IMG_GIF_NFY_FCT_T   pf_func;            /* callback function */
    #endif
    void            *pv_tag;            /* tag passed to the callback function */
    IMG_JPG_DECODE_FLAG_E          e_jpg_flag;     /* IMG_JPG_DECODE_FLAG_E */
} IMG_GIF_FRM_START_T;


/* data passed with IMG_SET_TYPE_BUF_FILLED */
typedef struct
{
    __u32          ui4_trsn_id;        /* transaction id */
    __s32           i4_ret;             /* MW buffer-filling return codes */
    __u32          ui4_filled_len;     /* filled length */
} IMG_GIF_BUF_FILLED_T;

/* data passed with IMG_SET_TYPE_DECODE */
typedef struct
{	  
    void            *pv_img_buf;        /* image data */
    void            *pv_img_buf2;       /* image data2 */
    __u32          ui4_img_size;       /* image size */
    void            *pv_type_data;      /* type data */

    __u32          ui4_src_x;          /* x offset in the source image in pixels */
    __u32          ui4_src_y;          /* y offset in the source image in pixels */
    __u32          ui4_src_width;      /* width to be decoded in pixels */
    __u32          ui4_src_height;     /* height to be decoded in pixels */

    void            *pv_dst;            /* destination starting address */
    void            *pv_dst2;           /* additional dst starting address */
    __u32          ui4_dst_x;          /* x offset in the destination in pixels */
    __u32          ui4_dst_y;          /* y offset in the destination in pixels */
    __u32          ui4_dst_width;      /* expected output width in pixels */
    __u32          ui4_dst_height;     /* expected output height in pixels */
    __u32          ui4_dst_pitch;      /* pitch of the destination image */
    __u32          ui4_dst2_pitch;     /* pitch of the additional destination image */
    GFX_COLORMODE_T e_dst_cm;           /* destination color mode */
    
    IMG_ROTATE_T    e_rotate;           /* rotation option */
} IMG_DIRECT_DECODE_T;

typedef struct
{
//ADD XZR
	#if 1
	__u32 			u4RdAddr;
    __u32 			u4WrAddr;
    __u32 			u4FileRdOfst;
    bool   			fgEOI;
	#endif
//END XZR	
    __u32          ui4_img_id;         /* the image id decoded */
    void            *pv_type_data;      /* frame index */

    __u32          ui4_src_x;          /* x offset in the source image in pixels */
    __u32          ui4_src_y;          /* y offset in the source image in pixels */
    __u32          ui4_src_width;      /* width to be decoded in pixels */
    __u32          ui4_src_height;     /* height to be decoded in pixels */

    void            *pv_dst;            /* destination starting address */
    void            *pv_dst2;           /* additional dst starting address */
    __u32          ui4_dst_x;          /* x offset in the destination in pixels */
    __u32          ui4_dst_y;          /* y offset in the destination in pixels */
    __u32          ui4_dst_width;      /* expected output width in pixels */
    __u32          ui4_dst_height;     /* expected output height in pixels */
    __u32          ui4_dst_pitch;      /* pitch of the destination image */
    __u32          ui4_dst2_pitch;     /* pitch of the additional destination image */
    GFX_COLORMODE_T e_dst_cm;           /* destination color mode */
    
    IMG_ROTATE_T    e_rotate;           /* rotation option */
    IMG_QUALITY_FACTOR_T e_quality;     /* quality factor */ //[20081014] BDP00013841
    bool            b_compressed;       /* PNG WT*/
} IMG_GIF_DECODE_T;

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* _X_IMG_DEC_H_ */

