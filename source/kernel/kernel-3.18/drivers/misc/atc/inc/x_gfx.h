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

#ifndef _X_GFX_H_
#define _X_GFX_H_
/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/

#include "u_common.h"
#include "x_drv_cb.h"
#include "x_memtype.h"
#include "x_os.h"
#include "x_typedef.h"
#include "drv_config.h"
#include "chip_ver.h"
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#define GFX_HAL_HW_INST_NUM     1
 
#ifndef  USE_RESOURCE_MANAGER  
//#define USE_RESOURCE_MANAGER 
#endif
#ifndef GL_XOR
#define GL_XOR
#endif

#include "dram_model.h"

//#if CONFIG_DRAM256_MODEL
//#endif


#if (CONFIG_DRV_ONLY)
#else
#define SUPPORT_FRAME_ACCURATE
#endif

#ifdef SUPPORT_FRAME_ACCURATE
    #ifndef GFX_SUPPORT_DOUBLE_BUFFER
    #define GFX_SUPPORT_DOUBLE_BUFFER
    #endif
#endif

#if CONFIG_DRV_3D_SUPPORT
    #ifndef GFX_SUPPORT_SINGLE_BUFFER 
    //#define GFX_SUPPORT_SINGLE_BUFFER
    #endif
    #ifndef GFX_SUPPORT_3D_DOUBLE_BUFFER
    #define GFX_SUPPORT_3D_DOUBLE_BUFFER
    #endif

    #ifdef GFX_SUPPORT_3D_DOUBLE_BUFFER
    //#define GFX_3D_DOUBLE_BUF_TEST
    #endif
    #ifdef GFX_SUPPORT_SINGLE_BUFFER
    #define GFX_SINGLE_BUFFER_TEST  0
    #endif
#endif


#define DFB_SUPPORT_GFX_ADAPTER 0

#define   GFX_MODE_ID_FRAME_BUFFER            (0x12345678)

#define GFX_DRV_CMDBUF_DYNAMIC_SIZE         1

/*! \name Get Operations
* @{
*/
/* get types of OSD driver component */
#define OSD_GET_TYPE_PLANE_CAPS             \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  1))

#define OSD_GET_TYPE_PLANE_BLEND_LEVEL      \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  2))

#define OSD_GET_TYPE_RGNLIST_CREATE         \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  3))

#define OSD_GET_TYPE_REGION_CREATE          \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  4))

#define OSD_GET_TYPE_REGION_INFO            \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  5))

#define OSD_GET_TYPE_BITMAP_CREATE          \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  6))

#define OSD_GET_TYPE_BITMAP_LOCK            \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  7))

#define OSD_GET_TYPE_BITMAP_PALETTE         \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  8))

#define OSD_GET_TYPE_PALETTE_CREATE         \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  9))

#define OSD_GET_TYPE_PALETTE_ENTRIES        \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 10))

#define OSD_GET_TYPE_PALETTE_PARAMS         \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 11))

#define OSD_GET_TYPE_PLANE_ADV_SCALER       \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 12))

#define OSD_GET_TYPE_PLANE_ADV_SCALER_FILTER_CNT    \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 13))

#define OSD_GET_TYPE_PLANE_NO_ALPHA_AREA    \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 14))

#define OSD_GET_TYPE_PLANE_FADING_RATIO     \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 15))

#define OSD_GET_TYPE_PLANE_POS              \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 16))

#define OSD_GET_TYPE_BITMAP_CREATE_FROM_FBM \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 17))

#define OSD_GET_TYPE_BITMAP_CREATE_EX       \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 18))
    
#define OSD_GET_TYPE_PALETTE_CREATE_EX         \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  19))

#define OSD_GET_TYPE_BITMAP_CREATE_IN_POOL       \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 20))
/*! @} */

/*! \name Set Operations
* @{
*/
/* set types of OSD driver component */
#define OSD_SET_TYPE_PLANE_ENABLE           \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  1)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_PLANE_FLIP             \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  2)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_PLANE_BLEND_LEVEL      \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  3)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_RGNLIST_DELETE         \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  4)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_RGNLIST_DETACH_ALL     \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  5)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_REGION_DELETE          \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  6)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_REGION_ATTACH          \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  7))

#define OSD_SET_TYPE_REGION_DETACH          \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  8))

#define OSD_SET_TYPE_REGION_ATTR            \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  9))

#define OSD_SET_TYPE_BITMAP_DELETE          \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 10)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_BITMAP_UNLOCK          \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 11)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_BITMAP_PALETTE         \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 12))

#define OSD_SET_TYPE_PALETTE_DELETE         \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 13)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_PALETTE_ENTRIES        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 14))

#define OSD_SET_TYPE_PALETTE_PARAMS         \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 15))

#define OSD_SET_TYPE_PLANE_ADV_SCALER       \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 16))

#define OSD_SET_TYPE_PLANE_NO_ALPHA_AREA    \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 17))

#define OSD_SET_TYPE_PLANE_FADING_RATIO     \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 18)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_PLANE_POS              \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 19))

#define OSD_SET_TYPE_BITMAP_DELETE_FROM_FBM \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 20)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_PLANE_FLIP_VSYNC        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 21)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_PLANE_ENABLE_VSYNC      \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 22)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_PLANE_SOURCE_ATT      \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 23))

    
#define OSD_SET_TYPE_PLANE_ENABLE_VSYNC_NOWAIT      \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 24)) | RM_SET_TYPE_ARG_NO_REF)
    
#define OSD_SET_TYPE_PLANE_TRYLOCK_VSYNC_NOWAIT      \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 25)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_PLANE_FLIP_VSYNC_NOWAIT        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 26)) | RM_SET_TYPE_ARG_NO_REF)

#define OSD_SET_TYPE_PLANE_FLIP_MULTI_VSYNC      \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 27))

#define OSD_SET_TYPE_PLANE_ENABLE_MULTI_VSYNC         \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 28))   

#define OSD_SET_TYPE_PLANE_FLIP_MULTI_VSYNC_WITH_TIMER      \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 29))

#define OSD_SET_TYPE_PLANE_FLIP_VSYNC_NOWAIT_AUTO        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 30)) | RM_SET_TYPE_ARG_NO_REF)

#ifdef SUPPORT_FRAME_ACCURATE
#define OSD_SET_TYPE_PLANE_PTS_FLIP_VSYNC        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 31)) )
    
    
#define OSD_SET_TYPE_NOTIFY_CB        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 32)) )

#define OSD_SET_TYPE_PTS_PALETTE_ENTRIES        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 33))
#define OSD_SET_CANCLE_ALL_PTS_CALL     \
   (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 34))

    
#define OSD_SET_TYPE_SET_ENTIES_NOTIFY_CB        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 35)) )

#define OSD_SET_TYPE_PTS_CALL     \
   (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 36))

#endif

#ifdef CONFIG_IC_MT8530
#define OSD_SET_TYPE_SET_RNG_XVYCC_EN        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 37)) )
#endif

#define OSD_SET_TYPE_CREATE_MEM_POOL        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 38)) )

#define OSD_SET_TYPE_OSD5_ONLY_CVBS        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 39)) )

#define OSD_SET_TYPE_DISABLE_IG_PLANE        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 40)) )

#if  1//CONFIG_DRV_3D_SUPPORT
#define OSD_SET_TYPE_PLANE_3D_FLIP_VSYNC      \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 41)) )

#define OSD_SET_TYPE_PLANE_3D_PTS_FLIP_VSYCN     \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 42)) )

#define OSD_SET_TYPE_PLANE_3D_SET_IG_OFFSET_SEQ_ID        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 43)) )

#define OSD_SET_TYPE_PLANE_3D_SET_PG_OFFSET_SEQ_ID        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 44)) )

#define OSD_SET_TYPE_PLANE_3D_SET_BDJ_OFFSET_SEQ_ID        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 45)) )

#define OSD_SET_TYPE_PLANE_3D_FIXED_OFFSET_IG_FLIP_VSYNC      \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 46)) )

#define OSD_SET_TYPE_PLANE_3D_SET_BDJ_PRESENTATION_TYPE        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 47)) )

#define OSD_SET_TYPE_PAL_3D_SET_ENTRIES  \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 48)) )
#define OSD_SET_TYPE_PAL_3D_PTS_SET_ENTRIES  \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 49)) )    
    
#define OSD_SET_TYPE_3D_SET_BDJ_OFFSET_SCALE_FACTOR  \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 50)) )       

#define OSD_SET_TYPE_PLANE_3D_ASYNC_FLIP_VSYNC      \
        ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 51)) )

#define OSD_SET_TYPE_PLANE_3D_RECONFIG_POS     \
        ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 52)) )


#endif

#define OSD_SET_TYPE_PLANE_SCREEN_SAVER_ENTER        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 60)) )

#define OSD_SET_TYPE_PLANE_SCREEN_SAVER_LEAVE        \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 61)) )

//for OSD drv (call function)    
#define OSD_SET_TYPE_FLIP_CALLBACK     \
       (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 62))

#ifdef GFX_SUPPORT_3D_DOUBLE_BUFFER
    //for OSD drv (call function)    
#define OSD_SET_TYPE_3D_FLIP_CALLBACK     \
           (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 63))
#endif


#define OSD_SET_TYPE_LOCK_SCREEN     \
       (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 64))

/* set types of graphics driver component */
#define GFX_SET_TYPE_FILL_RECT              \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  1))

#define GFX_SET_TYPE_DRAW_LINE_H            \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  2))

#define GFX_SET_TYPE_DRAW_LINE_V            \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  3))

#define GFX_SET_TYPE_EXT_ALPHA_MUL          \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  4))

#define GFX_SET_TYPE_SELF_ALPHA_MUL         \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  5))

#define GFX_SET_TYPE_SELF_ALPHA_DIV         \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  6))

#define GFX_SET_TYPE_ALPHA_BITBLT           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  7))

#define GFX_SET_TYPE_COLOR_BITBLT           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  8))

#define GFX_SET_TYPE_BITBLT                 \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T)  9))

#define GFX_SET_TYPE_STRETCH_BITBLT         \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 10))

#define GFX_SET_TYPE_TRANSPARENT_BITBLT     \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 11))

#define GFX_SET_TYPE_TRANSPARENT_FILL       \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 12))

#define GFX_SET_TYPE_ALPHA_BLENDING         \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 13))

#define GFX_SET_TYPE_ALPHA_COMPOSITION      \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 14))

#define GFX_SET_TYPE_FLUSH_OPQUEUE          \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 15)) | RM_SET_TYPE_ARG_NO_REF)

#if UNIFORM_DRV_CALLBACK
#define GFX_SET_TYPE_NOTIFY_FCT             \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 16))
#else
#define GFX_SET_TYPE_NOTIFY_FCT             \
    ((RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 16)) | RM_SET_TYPE_ARG_NO_REF)
#endif

#define GFX_SET_TYPE_ALPHAMAP_BITBLT        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 17))

#define GFX_SET_TYPE_ADV_STRETCH_BITBLT     \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 18))

#define GFX_SET_TYPE_YCBCR_TO_RGB           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 19))

#define GFX_SET_TYPE_ROTATE_90              \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 20))

#define GFX_SET_TYPE_PALETTE_BITBLT         \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 21))

#define GFX_SET_TYPE_ROP_BITBLT             \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 22))

#define GFX_SET_TYPE_JPG_Y_SCALE            \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 23))

#define GFX_SET_TYPE_JPG_CB_SCALE           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 24))

#define GFX_SET_TYPE_JPG_CR_SCALE           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 25))

#define GFX_SET_TYPE_REPLACE_COLOR          \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 26))

#define GFX_SET_TYPE_RELEASE_CMD_BUFF            \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 27))

#define GFX_SET_TYPE_STOP_CMD_BUFF           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 28))
    
#define GFX_SET_TYPE_IGPG           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 29))
#define GFX_SET_TYPE_FILL_COMPOSITION      \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 30))

#define GFX_SET_TYPE_STRETCH_COMPOSITION           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 31))

#define GFX_SET_TYPE_ADV_STRETCH_COMPOSITION           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 32))

#define GFX_SET_TYPE_ROTATE_STRETCH          \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 33))

#define GFX_SET_TYPE_ROTATE_ADV_STRETCH            \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 34))

#define GFX_SET_TYPE_ROTATE_STRETCH_COMPOSITION           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 35))
    
#define GFX_SET_TYPE_ROTATE_ADV_STRETCH_COMPOSITION           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 36))

#define GFX_SET_TYPE_ROTATE_COMPOSITION           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 37))

#define GFX_SET_TYPE_BITBLT_YUV2RGB     \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 38))

#define GFX_SET_TYPE_ALPHA_COMPOSITION_YUV2RGB       \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 39))

#define GFX_SET_TYPE_STRETCH_BITBLT_YUV2RGB       \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 40))

#define GFX_SET_TYPE_XOR       \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 41))

#define GFX_SET_TYPE_XOR_YUV2RGB       \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 42))

#define GFX_SET_TYPE_BITBLT_IDX2RGB     \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 43))

#define GFX_SET_TYPE_ALPHA_COMPOSITION_IDX2RGB       \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 44))

#define GFX_SET_TYPE_STRETCH_BITBLT_IDX2RGB       \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 45))

#define GFX_SET_TYPE_XOR_IDX2RGB       \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 46))
    
#define  GFX_SET_TYPE_ROTATE_ONLY                       \
(RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 47))

#define  GFX_SET_TYPE_ROTATE_YUV2RGB                  \
(RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 48))

#define  GFX_SET_TYPE_ROTATE_IDX2RGB                  \
(RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 49))

#define GFX_SET_TYPE_STRETCH_COMPOSITION_IDX2RGB           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 50))

#define GFX_SET_TYPE_ADV_STRETCH_COMPOSITION_IDX2RGB           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 51))

#define GFX_SET_TYPE_ROTATE_STRETCH_IDX2RGB          \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 52))

#define GFX_SET_TYPE_ROTATE_ADV_STRETCH_IDX2RGB            \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 53))

#define GFX_SET_TYPE_ROTATE_STRETCH_COMPOSITION_IDX2RGB           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 54))
    
#define GFX_SET_TYPE_ROTATE_ADV_STRETCH_COMPOSITION_IDX2RGB           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 55))

#define GFX_SET_TYPE_ROTATE_COMPOSITION_IDX2RGB           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 56))

#define GFX_SET_TYPE_STRETCH_COMPOSITION_YUV2RGB           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 57))

#define GFX_SET_TYPE_ADV_STRETCH_COMPOSITION_YUV2RGB           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 58))

#define GFX_SET_TYPE_ROTATE_STRETCH_YUV2RGB          \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 59))

#define GFX_SET_TYPE_ROTATE_ADV_STRETCH_YUV2RGB            \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 60))

#define GFX_SET_TYPE_ROTATE_STRETCH_COMPOSITION_YUV2RGB           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 61))
    
#define GFX_SET_TYPE_ROTATE_ADV_STRETCH_COMPOSITION_YUV2RGB           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 62))

#define GFX_SET_TYPE_ROTATE_COMPOSITION_YUV2RGB           \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 63))

#define GFX_SET_TYPE_ONECMDIN_CMD_BUFF            \
        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 64))
    
#define GFX_SET_TYPE_WAITDONE_CMD_BUFF            \
        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 65))

#ifdef GFX_SUPPORT_SINGLE_BUFFER
#define GFX_SET_TYPE_SET_SINGLEBUF_MODE            \
        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 66))
#endif


/*the RLE set type is from 80-90*/

#define RLE_SET_TYPE_REPLACE_COLOR        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 80))

#define RLE_SET_TYPE_BITBLT        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 81))

    
#define RLE_SET_TYPE_FILL_RECTANGLE        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 82))


#define RLE_SET_TYPE_DVD_SPU_DEC        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 83))


#define RLE_SET_TYPE_PGIG_DEC        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 84))


#define RLE_SET_TYPE_STOP        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 85))

#define RLE_SET_TYPE_FLUSH        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 86))


#define RLE_SET_TYPE_RELEASE_CMD_BUFFER        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 87))

#define RLE_GET_TYPE_GET_CMD_BUFFER        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 88))

#ifdef SUPPORT_FRAME_ACCURATE
#define RLE_SET_IGPG_DEC_CALLBACK        \
    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 89))
#endif


/* get types of graphics driver component */
#define GFX_GET_TYPE_GPU_CAPS             \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  1))

#define GFX_GET_TYPE_CMD_BUFF             \
    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T)  2))




/*! @} */

#define GFX_WT_ENBLE    1
#define GFX_WT_BYPASS   0

/* enumerations */
typedef enum _GFX_WT_COLORMODE_T
{
    GFX_WT_COLORMODE_ARGB    =  0,
    GFX_WT_COLORMODE_AYCbCR  =  1
} GFX_WT_COLORMODE_T;

typedef enum _GFX_COLORMODE_T
{
    GFX_COLORMODE_AYCbCr_CLUT2  = 0,
    GFX_COLORMODE_AYCbCr_CLUT4  = 1,
    GFX_COLORMODE_AYCbCr_CLUT8  = 2,
    GFX_COLORMODE_CbYCrY_16     = 3,
    GFX_COLORMODE_YCbYCr_16     = 4,
    GFX_COLORMODE_AYCbCr_D8888  = 5,
    GFX_COLORMODE_ARGB_CLUT2    = 6,
    GFX_COLORMODE_ARGB_CLUT4    = 7,
    GFX_COLORMODE_ARGB_CLUT8    = 8,
    GFX_COLORMODE_RGB_D565      = 9,
    GFX_COLORMODE_ARGB_D1555    = 10,
    GFX_COLORMODE_ARGB_D4444    = 11,
    GFX_COLORMODE_ARGB_D8888    = 12,
    GFX_COLORMODE_RGB_D888		= 13,
    GFX_COLORMDOE_YUV_420_BLK   = 14,
    GFX_COLORMODE_YUV_420_RS    = 15,
    GFX_COLORMDOE_YUV_422_BLK   = 16,
    GFX_COLORMODE_YUV_422_RS    = 17,
    GFX_COLORMDOE_YUV_444_BLK   = 18,
    GFX_COLORMODE_YUV_444_RS    = 19
} GFX_COLORMODE_T;

typedef enum _GFX_YCBCR_FORMAT_T
{
    GFX_YCBCR_420_MB  = 0,
    GFX_YCBCR_420_LNR = 1,
    GFX_YCBCR_422_LNR = 2       /* MT538x only */
} GFX_YCBCR_FORMAT_T;

/* constants */
#define GFX_CAP_AYCbCr_CLUT2    ((UINT32) 1 << (UINT32)GFX_COLORMODE_AYCbCr_CLUT2)
#define GFX_CAP_AYCbCr_CLUT4    ((UINT32) 1 << (UINT32)GFX_COLORMODE_AYCbCr_CLUT4)
#define GFX_CAP_AYCbCr_CLUT8    ((UINT32) 1 << (UINT32)GFX_COLORMODE_AYCbCr_CLUT8)
#define GFX_CAP_CbYCrY_16       ((UINT32) 1 << (UINT32)GFX_COLORMODE_CbYCrY_16   )
#define GFX_CAP_YCbYCr_16       ((UINT32) 1 << (UINT32)GFX_COLORMODE_YCbYCr_16   )
#define GFX_CAP_ARGB_CLUT2      ((UINT32) 1 << (UINT32)GFX_COLORMODE_ARGB_CLUT2  )
#define GFX_CAP_ARGB_CLUT4      ((UINT32) 1 << (UINT32)GFX_COLORMODE_ARGB_CLUT4  )
#define GFX_CAP_ARGB_CLUT8      ((UINT32) 1 << (UINT32)GFX_COLORMODE_ARGB_CLUT8  )
#define GFX_CAP_RGB_D565        ((UINT32) 1 << (UINT32)GFX_COLORMODE_RGB_D565    )
#define GFX_CAP_ARGB_D1555      ((UINT32) 1 << (UINT32)GFX_COLORMODE_ARGB_D1555  )
#define GFX_CAP_ARGB_D4444      ((UINT32) 1 << (UINT32)GFX_COLORMODE_ARGB_D4444  )
#define GFX_CAP_ARGB_D8888      ((UINT32) 1 << (UINT32)GFX_COLORMODE_ARGB_D8888  )
#define GFX_CAP_AYCbCr_D8888    ((UINT32) 1 << (UINT32)GFX_COLORMODE_AYCbCr_D8888)

/* alpha blending levels */
#define GFX_BLEND_TRANSPARENT   ((UINT8)   0)
#define GFX_BLEND_OPAQUE        ((UINT8) 255)

#define GFX_MAX_SUBPLANE        ((UINT32)  4)

#define GFX_CLUT_ALPHA_0        ((UINT8) 0)
#define GFX_CLUT_ALPHA_1_0      ((UINT8) 1)
#define GFX_CLUT_ALPHA_2_0      ((UINT8) 2)
#define GFX_CLUT_ALPHA_3_0      ((UINT8) 3)
#define GFX_CLUT_ALPHA_4_1      ((UINT8) 4)
#define GFX_CLUT_ALPHA_5_2      ((UINT8) 5)
#define GFX_CLUT_ALPHA_6_3      ((UINT8) 6)
#define GFX_CLUT_ALPHA_7_4      ((UINT8) 7)

/* palette entry color order */
#define GFX_CLUT_HIBYTE         ((UINT8) 3)
#define GFX_CLUT_MHBYTE         ((UINT8) 2)
#define GFX_CLUT_MLBYTE         ((UINT8) 1)
#define GFX_CLUT_LOBYTE         ((UINT8) 0)

#if 0
#define SELECT_A                ((UINT8) 3)
#define SELECT_R                ((UINT8) 2)
#define SELECT_G                ((UINT8) 1)
#define SELECT_B                ((UINT8) 0)

#define GFX_COLOR_PALETTE(a, r, g, b)       ((((UINT32)(a) & 0xff) << (SELECT_A * 8)) | (((UINT32)(r) & 0xff) << (SELECT_R * 8)) | (((UINT32)(g) & 0xff) << (SELECT_G * 8)) | (((UINT32)(b) & 0xff) << (SELECT_B * 8)))
#endif

#define GFX_COLOR_RGB565(r, g, b)           (((((UINT16) (r)) & 0xf8) << 8) | ((((UINT16) (g)) & 0xfc) << 3) | ((((UINT16) (b)) & 0xf8) >> 3))
#if 0
#define GFX_COLOR_ARGB1555(a, r, g, b)      (((((UINT32) (a)) & 0x80) << 8) | ((((UINT32) (r)) & 0xf8) << 7) | ((((UINT32) (g)) & 0xf8) << 2) | ((((UINT32) (b)) & 0xf8) >> 3))
#else
#define GFX_COLOR_ARGB1555(a, r, g, b)      (((((UINT16) (a)) > 0) ? (UINT16) 0x8000 : 0) | ((((UINT16) (r)) & 0xf8) << 7) | ((((UINT16) (g)) & 0xf8) << 2) | ((((UINT16) (b)) & 0xf8) >> 3))
#endif
#define GFX_COLOR_ARGB4444(a, r, g, b)      (((((UINT16) (a)) & 0xf0) << 8) | ((((UINT16) (r)) & 0xf0) << 4) | (((UINT16) (g)) & 0xf0) | ((((UINT16) (b)) & 0xf0) >> 4))
#define GFX_COLOR_ARGB8888(a, r, g, b)      (((((UINT32) (a)) & 0xff) << 24) | ((((UINT32) (r)) & 0xff) << 16) | ((((UINT32) (g)) & 0xff) << 8) | (((UINT32) (b)) & 0xff))

#define GFX_FIX_FRACTION_BITS       12
#define GFX_INTTOFIX(i)             (GFX_FP_T) ((UINT32) (i) << GFX_FIX_FRACTION_BITS)
#define GFX_FIXTOINT(f)             (UINT32) ((f) >> GFX_FIX_FRACTION_BITS)

/* region attr flags */
#define GFX_RGN_ATTR_NULL               ((UINT32)      0)
#define GFX_RGN_ATTR_SHRINK_X           ((UINT32) 0x0001)
#define GFX_RGN_ATTR_SHRINK_Y           ((UINT32) 0x0002)
#define GFX_RGN_ATTR_BLEND_OPTION       ((UINT32) 0x0004)
#define GFX_RGN_ATTR_BLEND_LEVEL        ((UINT32) 0x0008)
#define GFX_RGN_ATTR_COLORKEY_ENABLE    ((UINT32) 0x0010)
#define GFX_RGN_ATTR_COLORKEY           ((UINT32) 0x0020)
#define GFX_RGN_ATTR_WIDTH              ((UINT32) 0x0040)
#define GFX_RGN_ATTR_HEIGHT             ((UINT32) 0x0080)
#define GFX_RGN_ATTR_SURF_OFFSET_X      ((UINT32) 0x0100)
#define GFX_RGN_ATTR_SURF_OFFSET_Y      ((UINT32) 0x0200)
#define GFX_RGN_ATTR_YCBCR709EN      ((UINT32) 0x0400)
#define GFX_RGN_ATTR_ALL                ((UINT32) 0xffffffff)

/* alpha blending option */
#define GFX_BLENDING_OPT_NONE       ((UINT8) 0)
#define GFX_BLENDING_OPT_PIXEL      ((UINT8) 1)
#define GFX_BLENDING_OPT_REGION     ((UINT8) 2)
#define GFX_BLENDING_OPT_PLANE      ((UINT8) 3)

/* 2D graphics operation */
#define GFX_OP_FILLRECT             ((UINT32)0x000001)
#define GFX_OP_DRAWHLINE            ((UINT32)0x000002)
#define GFX_OP_DRAWVLINE            ((UINT32)0x000004)
#define GFX_OP_BITBLT               ((UINT32)0x000008)
#define GFX_OP_TRANSPARENT_BITBLT   ((UINT32)0x000010)
#define GFX_OP_TRANSPARENT_FILL     ((UINT32)0x000020)
#define GFX_OP_ALPHA_BLENDING       ((UINT32)0x000040)
#define GFX_OP_ALPHA_COMPOSITION    ((UINT32)0x000080)
#define GFX_OP_ALPHAMAP_BITBLT      ((UINT32)0x000100)
#define GFX_OP_EXT_ALPHA_MUL        ((UINT32)0x000200)
#define GFX_OP_SELF_ALPHA_MUL       ((UINT32)0x000400)
#define GFX_OP_SELF_ALPHA_DIV       ((UINT32)0x000800)
#define GFX_OP_ALPHA_BITBLT         ((UINT32)0x001000)
#define GFX_OP_COLOR_BITBLT         ((UINT32)0x002000)
#define GFX_OP_STRETCH_BITBLT       ((UINT32)0x004000)
#define GFX_OP_ADV_STRETCH_BITBLT   ((UINT32)0x008000)
#define GFX_OP_YCBCR_TO_RGB         ((UINT32)0x010000)
#define GFX_OP_ROTATE_90            ((UINT32)0x020000)
#define GFX_OP_PALETTE_BITBLT       ((UINT32)0x040000)
#define GFX_OP_ROP_BITBLT           ((UINT32)0x080000)
#define GFX_OP_JPG_SCALE            ((UINT32)0x100000)

/* scaler function enabling flags */
#define GFX_SCALER_FUNC_SCALING         ((UINT16) 0x01)
#define GFX_SCALER_FUNC_FILTERING       ((UINT16) 0x02)

/* scaler setting flags */
#define GFX_SCALER_FUNC_ENABLE          ((UINT16) 0x01)
#define GFX_SCALER_SRC_SIZE             ((UINT16) 0x02)
#define GFX_SCALER_DST_SIZE             ((UINT16) 0x04)
#define GFX_SCALER_BUF_ARRANGEMENT      ((UINT16) 0x08)
#define GFX_SCALER_COLOR_MODE           ((UINT16) 0x10)
#define GFX_SCALER_FILTER_AS_DEFAULT    ((UINT16) 0x20)
#define GFX_SCALER_FILTER_AS_SPECIFIED  ((UINT16) 0x40)

/* scaler line buffer arrangement */
#define GFX_SCALER_BUF_SHARED           ((UINT8) 0)
#define GFX_SCALER_BUF_EXCLUSIVE        ((UINT8) 1)

/* scaler color mode setting */
#define GFX_SCALER_CM_16BPP             ((UINT8) 0)
#define GFX_SCALER_CM_32BPP             ((UINT8) 1)

/* extra capabilities */
#define GFX_CAPS_ADV_SCALER             ((UINT32) 0x01)
#define GFX_CAPS_NO_ALPHA_AREA          ((UINT32) 0x02)
#define GFX_CAPS_FADING_RATIO           ((UINT32) 0x04)
#define GFX_CAPS_PLANE_REPOS            ((UINT32) 0x08)

/* callback condition */
typedef UINT32 GFX_COND_T;

#define GFX_COND_IDLE_IN_ISR        ((GFX_COND_T) 0)
#define GFX_COND_IDLE_IN_THREAD     ((GFX_COND_T) 1)

/* callback status */
#define GFX_EXEC_OK                 ((UINT32) 0)
#define GFX_EXEC_FAIL               ((UINT32) 1)

/* Z order */
#define GFX_ZORDER_TOPMOST          ((UINT32) 0)

/* enumerations */
typedef enum
{
    GFX_CLEAR = 0,
    GFX_DST_IN,
    GFX_DST_OUT,
    GFX_DST_OVER,
    GFX_SRC,
    GFX_SRC_IN,
    GFX_SRC_OUT,
    GFX_SRC_OVER
} GFX_PD_RULE_T;

typedef enum
{
    GFX_CLOCKWISE   = 0,
    GFX_C_CLOCKWISE,
    GFX_ROTATE_NULL
} GFX_ROTATE_FLAG_T;

/* typedefs */
typedef struct
{
    UINT32 ui4_regionlist;
    BOOL    b_compressed;
    UINT32    ui4_plane_idx;
}  GFX_HRGNLIST_EX_T;


typedef struct
{
    UINT32 ui4_unused;
} *GFX_HRGNLIST_T;

typedef struct
{
    UINT32 ui4_unused;
} *GFX_HREGION_T;

typedef struct
{
    UINT32 ui4_unused;
} *GFX_HBITMAP_T;

typedef struct
{
    UINT32 ui4_unused;
} *GFX_HPALETTE_T;

#if UNIFORM_DRV_CALLBACK

typedef struct _GFX_NFY_FCT_PAR_T	
{
     GFX_COND_T e_cond;
     UINT32 ui4_data ;
} GFX_NFY_FCT_PAR_T;

#endif

typedef enum
{
    GFX_PALETTE_MSB = 0,
    GFX_PALETTE_LSB = 1,
} GFX_BYTE_ALIGNED_T;

typedef enum
{
    GFX_ROP_NOT_SRC = 4,
    GFX_ROP_NOT_DST,
    GFX_ROP_SRC_XOR_DST,
    GFX_ROP_SRC_XNOR_DST,
    GFX_ROP_SRC_AND_DST,
    GFX_ROP_NOT_SRC_AND_DST,
    GFX_ROP_SRC_AND_NOT_DST,
    GFX_ROP_NOT_SRC_AND_NOT_DST,
    GFX_ROP_SRC_OR_DST,
    GFX_ROP_NOT_SRC_OR_DST,
    GFX_ROP_SRC_OR_NOT_DST,
    GFX_ROP_NOT_SRC_OR_NOT_DST
} GFX_ROP_TYPE_T;

typedef enum
{
    OSD_BITMAP_CH_DONT_CARE = 0,
    OSD_BITMAP_CH_1,
    OSD_BITMAP_CH_2,   
} OSD_BITMAP_CH_T;

typedef UINT16 GFX_FP_T;
typedef UINT32 GFX_COLOR_PACKED_T;

#if UNIFORM_DRV_CALLBACK
#else
typedef INT32 (*GFX_NFY_FCT_T) (GFX_COND_T e_cond, const VOID* pv_tag, UINT32 ui4_data);

#endif

/* structures */
#define GFX_PALETTE(x)          struct                                          \
                                {                                               \
                                    UINT16              ui2_size;               \
                                    UINT8               ui1_alpha_bit_select;   \
                                    UINT8               ui1_alpha_select;       \
                                    UINT8               ui1_YR_select;          \
                                    UINT8               ui1_UG_select;          \
                                    UINT8               ui1_VB_select;          \
                                    GFX_COLOR_PACKED_T  at_clut_entry[x];       \
                                }

typedef GFX_PALETTE(1)          GFX_PALETTE1_T;
typedef GFX_PALETTE(2)          GFX_PALETTE2_T;
typedef GFX_PALETTE(4)          GFX_PALETTE4_T;
typedef GFX_PALETTE(16)         GFX_PALETTE16_T;
typedef GFX_PALETTE(256)        GFX_PALETTE256_T;
typedef GFX_PALETTE1_T*         GFX_PALETTE_PTR;

/*-----------------------------------------------------------------------------
                    structures
 ----------------------------------------------------------------------------*/

typedef struct _GFX_PALETTE_HEADER_T
{
    GFX_COLORMODE_T e_color_mode;
    UINT16          ui2_size;
    UINT8           ui1_alpha_bit_select;
    UINT8           ui1_alpha_select;
    UINT8           ui1_YR_select;
    UINT8           ui1_UG_select;
    UINT8           ui1_VB_select;
} GFX_PALETTE_HEADER_T;

typedef struct _GFX_BITMAP_INIT_T
{
    GFX_COLORMODE_T     e_colormode;
    UINT32              ui4_bmp_w;
    UINT32              ui4_bmp_h;
    GFX_HPALETTE_T      h_palette;
    GFX_HBITMAP_T       h_new_bitmap;
    BOOL                  b_compressed;
    UINT32*              pu4_assigned_buffer;//must be NULL if no use.
    OSD_BITMAP_CH_T     e_bitmap_ch;
} GFX_BITMAP_INIT_T;

/* remember sync this definition with \middleware\graphic\u_gl.h */
typedef struct _GFX_AUX_DATA_T
{
    /* the content of this structure should be synchronized with GL_AUX_DATA_T */
    UINT32 ui4_plane;

    //is it YCbCr709
    BOOL b_ycbcr709;
} GFX_AUX_DATA_T;

typedef struct _GFX_BITMAP_INIT_EX_T
{
    GFX_BITMAP_INIT_T   t_init;
    GFX_AUX_DATA_T   __local_space__   *pt_aux_data;
} GFX_BITMAP_INIT_EX_T;

typedef struct _GFX_PALETTE_INIT_T
{
    GFX_PALETTE_PTR  __cross_space__   pt_palette;
    GFX_HPALETTE_T      h_new_palette;
    UINT32              ui4_id;

    /* msz00439 08-03-18 */
    /* this is used to remember the dram channel ingormation */
    GFX_AUX_DATA_T	t_aux_data;
    UINT32 * __cross_space__  p_palette;
} GFX_PALETTE_INIT_T;


typedef struct _GFX_REGION_INIT_T
{
    UINT32              ui4_out_w;
    UINT32              ui4_out_h;

    GFX_HBITMAP_T       h_bitmap;

    UINT8               ui1_blend_option;
    UINT8               ui1_blend_level;

    BOOL                b_enable_colorkey;
    GFX_COLOR_PACKED_T  ui4_colorkey;

    GFX_FP_T            t_shrink_x;
    GFX_FP_T            t_shrink_y;

    GFX_HREGION_T       h_new_region;
    UINT32              u4_flag;
    /* msz00439 */
    /* this is used to remember the dram channel ingormation */
    GFX_AUX_DATA_T	t_aux_data;

} GFX_REGION_INIT_T;

typedef struct _GFX_REGION_INFO_T
{
    GFX_HREGION_T   h_region;

    INT32           i4_out_x;
    INT32           i4_out_y;
    UINT32          ui4_out_w;
    UINT32          ui4_out_h;

    GFX_FP_T        t_shrink_x;
    GFX_FP_T        t_shrink_y;

    UINT8           ui1_blend_option;
    UINT8           ui1_blend_level;

    BOOL            b_colorkey;
    UINT32          ui4_colorkey;

    GFX_HBITMAP_T   h_bitmap;

    GFX_COLORMODE_T e_colormode;
    UINT32          ui4_bmp_w;
    UINT32          ui4_bmp_h;
    
    UINT32          ui4_bitmap_addr;    
    UINT32          ui4_palette_addr;    
    UINT32          ui4_pitch;    
    UINT32          ui4_clip_x;    
    UINT32          ui4_clip_y;
}   GFX_REGION_INFO_T;

typedef struct _GFX_BITMAP_LOCK_INFO_T
{
    GFX_HBITMAP_T       h_bitmap;

    GFX_COLORMODE_T     e_colormode;
    BOOL         b_compressed;
    SIZE_T              z_subplane;
    UINT32*        pu4_assigned_buffer;

    struct _SUB_PLANE_INFO_T {
        void*      __cross_space__  pv_bits;
        UINT32      ui4_width;
        UINT32      ui4_height;
        UINT32      ui4_pitch;

    } at_subplane[GFX_MAX_SUBPLANE];
} GFX_BITMAP_LOCK_INFO_T;

typedef struct _GFX_REGION_POS_T
{
    GFX_HREGION_T   h_region;
    GFX_HRGNLIST_T  h_rgnlist;
    INT32           i4_out_x;
    INT32           i4_out_y;
} GFX_REGION_POS_T;

typedef struct _GFX_BITMAP_PALETTE_T
{
    GFX_HBITMAP_T   h_bitmap;
    GFX_HPALETTE_T  h_palette;
} GFX_BITMAP_PALETTE_T;

typedef struct _GFX_PALETTE_PARAMS_T
{
    GFX_HPALETTE_T  h_palette;
    UINT8           ui1_alpha_bit_select;
    UINT8           ui1_alpha_select;
    UINT8           ui1_YR_select;
    UINT8           ui1_UG_select;
    UINT8           ui1_VB_select;
} GFX_PALETTE_PARAMS_T;

typedef struct _GFX_PALETTE_ENTRIES_T
{
    GFX_HPALETTE_T      h_palette;
    UINT16              ui2_start;
    UINT16              ui2_count;
    GFX_COLOR_PACKED_T* __cross_space__ pui4_entries;
} GFX_PALETTE_ENTRIES_T;

#ifdef SUPPORT_FRAME_ACCURATE
typedef struct _GFX_PALETTE_ENTRIES_EXT_T
{
    GFX_HPALETTE_T      h_palette;
    UINT16              ui2_start;
    UINT16              ui2_count;
    GFX_COLOR_PACKED_T* __cross_space__ pui4_entries;
    UINT64 u8Pts;
    UINT64 u8Tickets;
} GFX_PALETTE_ENTRIES_EXT_T;

typedef struct _OSD_CB_INFO_T
{
    UINT32          u4Plane;
    UINT64          u8Tickets;
    UINT64          u8Pts;
} OSD_CB_INFO_T;
#endif

typedef struct _GFX_SET_RNG_XVYCC_EN_T
{
   GFX_HREGION_T   h_region;
   BOOL fgXvyccEn;
}GFX_SET_RNG_XVYCC_EN_T;

typedef struct _GFX_CREATE_MEM_POOL
{
   UINT8*   pu4Mem;
   UINT32  u4MemSize;
}GFX_CREATE_MEM_POOL;

typedef struct _GFX_PLANE_CAPS_T
{
    UINT32              ui4_cm_caps;
    UINT32              ui4_func_caps;
#if 0
    UINT32              ui4_display_mask;
    INT32               i4_left;
    INT32               i4_top;
    INT32               i4_right;
    INT32               i4_bottom;
    UINT32              ui4_max_w;
    UINT32              ui4_max_h;

    UINT32              ui4_min_w;
    UINT32              ui4_min_h;
#endif
} GFX_PLANE_CAPS_T;

typedef struct _GFX_REGION_DETACH_T
{
    GFX_HREGION_T   h_region;
    GFX_HRGNLIST_T  h_rgnlist;
} GFX_REGION_DETACH_T;

typedef struct _GFX_REGION_ATTR_T
{
    GFX_HREGION_T   h_region;

    UINT32          ui4_flag;

    UINT32          ui4_width;
    UINT32          ui4_height;

    GFX_FP_T        t_shrink_x;
    GFX_FP_T        t_shrink_y;

    UINT8           ui1_blend_option;
    UINT8           ui1_blend_level;

    BOOL            b_colorkey;
    UINT32          ui4_colorkey;

    UINT32          ui4_surf_offset_x;
    UINT32          ui4_surf_offset_y;
    BOOL             b_ycbcr709_en;
} GFX_REGION_ATTR_T;

typedef struct _GFX_REGION_3D_POS_T
{
    GFX_HREGION_T   h_region;
    UINT32          u4_StartX;
    UINT32          u4_ClipH;
    UINT32          u4_OutWidth;
}GFX_REGION_3D_POS_T;

typedef struct _GFX_ATTACH_LIST_T
{
    GFX_HRGNLIST_T  h_rgnlist;
    UINT16          ui2_count;
    GFX_HREGION_T   ah_region[1];
} GFX_ATTACH_LIST_T;

typedef struct _GFX_POS_T
{
    INT32       i4_x;
    INT32       i4_y;
} GFX_POS_T;

#if 1//CONFIG_DRV_3D_SUPPORT
typedef struct _GFX_HRGNLIST_3D_T
{
    GFX_HRGNLIST_T          r_left_rnglist; 
    GFX_HRGNLIST_T          r_right_rnglist;
} GFX_HRGNLIST_3D_T;

typedef struct _GFX_3D_FLIP_T
{
    GFX_HRGNLIST_3D_T          r_rgnlist; 
    BOOL                b_sync;
} GFX_3D_FLIP_T;

typedef struct _GFX_3D_PTS_FLIP_T
{
    GFX_HRGNLIST_3D_T          r_rgnlist; 
    UINT64              ui8_pts;                     
    BOOL                b_needchangeenabledisable;
    BOOL                b_planeenable;
    BOOL                b_sync;
    UINT64              ui8_tickets;
} GFX_3D_PTS_FLIP_T;

typedef struct _GFX_3D_PTS_OFFSET_SEQUENCE_ID_T
{
    UINT64  u8_playitem_starttime;    // 90kHz
    UINT8   u1_sequence_Id;
} GFX_3D_PTS_OFFSET_SEQUENCE_ID_T;


typedef struct _GFX_3D_FIXED_OFFSET_FLIP_T
{
    GFX_HRGNLIST_3D_T          r_rnglist; 
    BOOL                b_sync;
    BOOL                b_isVideoBB;
    INT32               i4_offsetvalue;
} GFX_3D_FIXED_OFFSET_FLIP_T;

typedef struct _GFX_3D_BDJ_PRESENTATION_T
{
    BOOL  b_isVideoBB;
    INT32   i4_offsetvalue; 

} GFX_3D_BDJ_PRESENTATION_T;

typedef struct _GFX_3D_PTS_ENTRIES_T
{
    GFX_HPALETTE_T       h_leftpalette;
     GFX_HPALETTE_T       h_rightpalette;
     UINT16              ui2_start;
     UINT16              ui2_count;
     GFX_COLOR_PACKED_T* __cross_space__ pui4_left_entries;
     GFX_COLOR_PACKED_T* __cross_space__ pui4_right_entries;
     UINT64              ui8_pts;
     UINT64              ui8_tickets;

} GFX_3D_PTS_ENTRIES_T;

typedef struct _GFX_3D_ENTRIES_T
{
    GFX_HPALETTE_T       h_leftpalette;
     GFX_HPALETTE_T       h_rightpalette;
     UINT16              ui2_start;
     UINT16              ui2_count;
     GFX_COLOR_PACKED_T* __cross_space__ pui4_left_entries;
     GFX_COLOR_PACKED_T* __cross_space__ pui4_right_entries;

} GFX_3D_ENTRIES_T;


typedef struct _GFX_3D_BDJ_SCALER_T
{
    UINT32 u4_numerator;
    UINT32 u4_denominator;
} GFX_3D_BDJ_SCALER_T;


#endif
/* as GFXSC_VERT_SEG_T */
typedef struct _GFX_SEG_INFO_T
{
    UINT32      ui4_src_pos;
    UINT32      ui4_dst_pos;

    UINT32      ui4_src_size;
    UINT32      ui4_dst_size;

    UINT32      ui4_reserved1;
    UINT32      ui4_reserved2;
} GFX_SEG_INFO_T;

typedef struct _GFX_ADV_SCALER_PARAM_T
{
    UINT16  ui2_flag;
    UINT16  ui2_func_enable;
    UINT32  ui4_src_w;
    UINT32  ui4_src_h;
    UINT32  ui4_dst_w;
    UINT32  ui4_dst_h;
    UINT8   ui1_buf_arrange;
    UINT8   ui1_color_mode;
    UINT8   ui1_filter_cnt;
    INT16   ai2_filter[1];
} GFX_ADV_SCALER_PARAM_T;

typedef struct _GFX_NO_ALPHA_AREA_T
{
    BOOL        b_enable;
    INT32       i4_x;
    INT32       i4_y;
    UINT32      ui4_w;
    UINT32      ui4_h;
} GFX_NO_ALPHA_AREA_T;

typedef struct _GFX_GPU_CAPS_T
{
    UINT32          ui4_gfx_op;
    UINT32          ui4_reserved;
}   GFX_GPU_CAPS_T;
 
typedef struct _GFX_FILL_T
{
   #if 1 //def USE_RESOURCE_MANAGER 
    INT32           u4TicketId;
    UINT32            u4Flag;   
   #endif
    VOID*     __cross_space__ pv_dst;
    VOID*     __cross_space__ pv_dst2;
    INT32           i4_dst_x;
    INT32           i4_dst_y;
    UINT32          ui4_dst_pitch;
    UINT32          ui4_dst_pitch2;
    GFX_COLORMODE_T e_dst_cm;
    BOOL           b_compressed;       

    UINT32          ui4_width;
    UINT32          ui4_height;
    UINT32          ui4_color;
    UINT32          ui4ModuleID;
} GFX_FILL_T;
 #if 1//def USE_RESOURCE_MANAGER        

#define COMMON_BITBLT_FIELDS        \
    INT32           u4TicketId;     \
    UINT32         u4Flag;       \
    VOID*       __cross_space__   pv_src;         \
    INT32           i4_src_x;       \
    INT32           i4_src_y;       \
    UINT32          ui4_src_pitch;  \
    GFX_COLORMODE_T e_src_cm;       \
    BOOL           b_src_compressed;       \
                                    \
    VOID*       __cross_space__    pv_dst;         \
    INT32           i4_dst_x;       \
    INT32           i4_dst_y;       \
    UINT32          ui4_dst_pitch;  \
    GFX_COLORMODE_T e_dst_cm;       \
    BOOL           b_dst_compressed;       \
                                    \
    UINT32          ui4_width;      \
    UINT32          ui4_height;     \
    GFX_YCBCR_FORMAT_T  e_ycbcr_format;     \
    VOID*           __cross_space__  pv_cbcr_or_palette;             \
    UINT32              ui4_cbcr_pitch;     \
    UINT8              ui1_ycbcr_alpha;  \
    GFX_BYTE_ALIGNED_T  e_byte_aligned;  \
    GFX_HPALETTE_T      h_palette;       \
    UINT32          ui4ModuleID;         \
    UINT32          u4_alCom_normal;       \
    UINT32          u4_src_picth_en;   
#else

#define COMMON_BITBLT_FIELDS        \
    BOOL            bSyncEn;       \
    VOID*       __cross_space__   pv_src;         \
    INT32           i4_src_x;       \
    INT32           i4_src_y;       \
    UINT32          ui4_src_pitch;  \
    GFX_COLORMODE_T e_src_cm;       \
    BOOL           b_src_compressed;       \
                                    \
    VOID*       __cross_space__    pv_dst;         \
    INT32           i4_dst_x;       \
    INT32           i4_dst_y;       \
    UINT32          ui4_dst_pitch;  \
    GFX_COLORMODE_T e_dst_cm;       \
    BOOL           b_dst_compressed;       \
                                    \
    UINT32          ui4_width;      \
    UINT32          ui4_height;

#endif  

#define GFX_3D_HOR_ROTATE   0

#if GFX_3D_HOR_ROTATE

#define GFX_3D_ROTATE_FRAME_COUNT    26

typedef enum _GFX_3D_HROT_DIRECTION_T
{
    GFX_3D_HROT_DIR_CLOCKWISE,
    GFX_3D_HROT_DIR_ANTICLOCKWISE,
    GFX_3D_HROT_DIR_MAX
} GFX_3D_HROT_DIRECTION_T;

typedef struct _GFX_3D_HOR_ROTATE_T
{
    COMMON_BITBLT_FIELDS

    UINT32 u4FrmNum;
    UINT32 u4FrmTotal;
    UINT32 u4BgColor;

    GFX_3D_HROT_DIRECTION_T u4Direction;
} GFX_3D_HOR_ROTATE_T;
#endif  // GFX_3D_HOR_ROTATE

typedef struct _GFX_BITBLT_BASE_T
{
    COMMON_BITBLT_FIELDS
} GFX_BITBLT_BASE_T;

typedef struct _GFX_BITBLT_T
{
    COMMON_BITBLT_FIELDS

    UINT8       ui1_alpha;
    
    UINT32 u4RollBackEn;
    UINT32 u4QulityMode; 
    UINT32 u4LineSeparate;
} GFX_BITBLT_T;

//add for 8530 emulation for mirror and flip msz00441 080604
typedef enum 
{
    E_BITBLT_SRC_MIRROR = 0,
    E_BITBLT_SRC_FLIP,
    E_BITBLT_DST_MIRROR,
    E_BITBLT_DST_FLIP,
    E_BITBLT_SRC_FLIPMIRROR,
    E_BITBLT_DST_FLIPMIRROR,
    E_BITBLT_NORMAL,
    E_BITBLT_MF_MAX
}EGFX_BITBLT_OPT_T;


typedef struct _GFX_BITBLT_MIRROR_FLIP_T
{
    COMMON_BITBLT_FIELDS

    UINT8       ui1_alpha;
    EGFX_BITBLT_OPT_T u4Opt;
    
} GFX_BITBLT_MIRROR_FLIP_T;


typedef GFX_BITBLT_T GFX_ALPHA_MUL_T;

typedef GFX_BITBLT_BASE_T GFX_ALPHAMAP_BITBLT_T;

typedef struct _GFX_STRETCH_BITBLT_T
{
    COMMON_BITBLT_FIELDS

    UINT32      ui4_dst_width;
    UINT32      ui4_dst_height;
    //UINT32*    pv_palette;
} GFX_STRETCH_BITBLT_T;

typedef struct _GFX_TRANSPARENT_BITBLT_T
{
    COMMON_BITBLT_FIELDS

#if 0
    UINT8       ui1_alpha;
#endif

    BOOL        b_inverse_masking;
    UINT32      ui4_color_space_min;
    UINT32      ui4_color_space_max;
} GFX_TRANSPARENT_BITBLT_T;

typedef struct _GFX_TRANSPARENT_FILL_T
{
    COMMON_BITBLT_FIELDS

#if 0
    UINT8       ui1_alpha;
#endif

    BOOL        b_inverse_masking;
    UINT32      ui4_color_space_min;
    UINT32      ui4_color_space_max;

    UINT32      ui4_fill_color;
} GFX_TRANSPARENT_FILL_T;

typedef GFX_BITBLT_T GFX_ALPHA_BLEND_T;

typedef struct _GFX_ALPHA_COMPOSITION_T
{
    COMMON_BITBLT_FIELDS

    UINT8           ui1_alpha;

    GFX_PD_RULE_T   e_rule;

    BOOL            b_rect_src_option;
    UINT32          ui4_color;
} GFX_ALPHA_COMPOSITION_T;

typedef struct _GFX_ROTATE_T
{
    COMMON_BITBLT_FIELDS

    GFX_ROTATE_FLAG_T e_direction;
} GFX_ROTATE_T;

typedef struct _GFX_YCBCR_TO_RGB
{
    COMMON_BITBLT_FIELDS

    VOID      *pv_cbcr;
    //UINT32              ui4_cbcr_pitch;
    UINT8               ui1_alpha;
    //GFX_YCBCR_FORMAT_T  e_ycbcr_format;
} GFX_YCBCR_TO_RGB_T;

typedef struct _GFX_PLTBLT_T
{
    COMMON_BITBLT_FIELDS

    //GFX_BYTE_ALIGNED_T  e_byte_aligned;
    //GFX_HPALETTE_T      h_palette;
    //UINT32*                  pv_palette;
} GFX_PLTBLT_T;

typedef struct _GFX_ROP_BITBLT_T
{
    COMMON_BITBLT_FIELDS

    GFX_ROP_TYPE_T      e_rop_type;
} GFX_ROP_BITBLT_T;

typedef struct _GFX_JPG_SCALE_T
{
    COMMON_BITBLT_FIELDS

    UINT32          ui4_dst_width;
    UINT32          ui4_dst_height;

    BOOL            b_do_segment;
    UINT32          ui4_vert_seg_length;
    UINT32          ui4_loop;

    /* output */
    UINT32          ui4_segment_count;
    GFX_SEG_INFO_T  __opaque__ *pt_vertical_info;
} GFX_JPG_SCALE_T;

typedef struct _GFX_REPLACE_COLOR_T
{
    COMMON_BITBLT_FIELDS

    UINT8           ui1_alpha;

    UINT32    __local_space__ *pui4_original_color;
    UINT32    __local_space__ *pui4_new_color;
    UINT8           ui1_color_num;
} GFX_REPLACE_COLOR_T;

typedef struct _GFX_PLA_SRC_ATT
{
	GFX_COLORMODE_T e_colormode;
	UINT32 ui4_width;
 	UINT32 ui4_height;
} GFX_PLA_SRC_ATT;    
typedef enum _GFX_OPTION_T
{
    GFX_OPTION_COMP     = 0,
    GFX_OPTION_XOR     = 1,
} GFX_OPTION_T;

typedef struct _GFX_ROTATE_STRETCH_ALPHACOMP_T
{
    COMMON_BITBLT_FIELDS
    UINT8           ui1_alpha_assigned;
    UINT8           ui1_ac_ar;
    GFX_PD_RULE_T   e_rule;
    BOOL             b_rect_src_option;
    UINT32          ui4_color;
    UINT32          ui4_dst_width;
    UINT32          ui4_dst_height;
    GFX_OPTION_T  e_option;
    UINT32          ui4_rot_op;
} GFX_ROTATE_STRETCH_ALPHACOMP_T;  

typedef struct _GFX_PREMULTIPLIED_CONVERT_T
{
    COMMON_BITBLT_FIELDS
    UINT32     ui4_convert_type;
    UINT8       ui1_alpha;
}GFX_PREMULTIPLIED_CONVERT_T;

typedef struct _GFX_REFLECT_T
{
    COMMON_BITBLT_FIELDS
    //UINT8       ui1_alpha;
    //EGFX_BITBLT_OPT_T u4Opt;
    INT32       i4StartAlpha;
    INT32       i4EndAlpha;
    //INT32       i4AlphaHeight;
    //UINT32      u4GradeFliiMode;
    //UINT32      ui4_color;
}GFX_REFLECT_T;

#define GFX_BLT_FLIP                 ((UINT32) 0x10000)
#define GFX_BLT_ROTATE_MASK          ((UINT32) 0xffff)
#define GFX_BLT_ROTATE_NONE          ((UINT32)      0)
#define GFX_BLT_ROTATE_CW90          ((UINT32)     90)
#define GFX_BLT_ROTATE_CW180         ((UINT32)    180)
#define GFX_BLT_ROTATE_CW270         ((UINT32)    270)
#define GFX_BLT_ROTATE_NONE_FLIP          ((UINT32) 0x10000)
#define GFX_BLT_ROTATE_CW90_FLIP            ((UINT32) 0x1005A)
#define GFX_BLT_ROTATE_CW180_FLIP           ((UINT32)0x100B4)
#define GFX_BLT_ROTATE_CW270_FLIP           ((UINT32)  0x1010E)



//add for new command queue msz00441  080805
typedef enum _GFX_APP_INSTANCEID_T
{   
    GFX_APP_DBJ  = 0,
    GFX_APP_UI = 1,
    GFX_APP_BDMW  = 2,
    GFX_APP_MAX  = 3 
} GFX_APP_INSTANCEID_T;

typedef enum _GFX_HW_INSTANCEID_T
{
    GFX_HW_INSTANCE_0=0,
    GFX_HW_INSTANCE_1=1,
    GFX_HW_INSTANCE_MAX=2
} GFX_HW_INSTANCEID_T;


#define  GFX_OFF_SCREEN  	FALSE
#define  GFX_ON_SCREEN  	TRUE

typedef enum _GFX_BUFF_CMD_TYPE_T
{
    GFX_BUFF_TYPE_FILL_RECT = 0,
    GFX_BUFF_TYPE_BITBLT,
    GFX_BUFF_TYPE_ALPHA_COMPOSITION,
    GFX_BUFF_TYPE_STRETCH_BITBLT,
    
    GFX_BUFF_TYPE_DRAW_LINE_H,
    GFX_BUFF_TYPE_DRAW_LINE_V,
    GFX_BUFF_TYPE_TRANSPARENT_BITBLT,
    GFX_BUFF_TYPE_TRANSPARENT_FILL,
    GFX_BUFF_TYPE_REPLACE_COLOR,
    GFX_BUFF_TYPE_ALPHA_BLENDING,
    GFX_BUFF_TYPE_EXT_ALPHA_MUL,
    GFX_BUFF_TYPE_ALPHA_BITBLT,  
    GFX_BUFF_TYPE_ALPHAMAP_BITBLT,
    GFX_BUFF_TYPE_COLOR_BITBLT,
    GFX_BUFF_TYPE_YCBCR_TO_RGB,     
    GFX_BUFF_TYPE_ADV_STRETCH_BITBLT,
    GFX_BUFF_TYPE_ROP_BITBLT,
    GFX_BUFF_TYPE_PALETTE_BITBLT,       
    GFX_BUFF_TYPE_ROTATE_90 ,
    GFX_BUFF_TYPE_IGPG_DECODE,
    GFX_BUFF_TYPE_FLUSH,
    GFX_BUFF_TYPE_STOP,
    
    GFX_BUFF_TYPE_FILL_COMPOSITION,
    //#ifdef GL_XOR
    GFX_BUFF_TYPE_STRETCH_COMPOSITION,
    GFX_BUFF_TYPE_ADV_STRETCH_COMPOSITION,
    GFX_BUFF_TYPE_ROTATE_STRETCH ,
    GFX_BUFF_TYPE_ROTATE_ADV_STRETCH,
    GFX_BUFF_TYPE_ROTATE_STRETCH_COMPOSITION,
    GFX_BUFF_TYPE_ROTATE_ADV_STRETCH_COMPOSITION,
    GFX_BUFF_TYPE_ROTATE_COMPOSITION,
    GFX_BUFF_TYPE_MIRROR_FLIP,
    GFX_BUFF_TYPE_Y2R_BITBLT,
    GFX_BUFF_TYPE_Y2R_COMPOSE,
    GFX_BUFF_TYPE_Y2R_XOR,
    GFX_BUFF_TYPE_Y2R_STRETCH,
    GFX_BUFF_TYPE_ROTATE_ONLY,
    GFX_BUFF_TYPE_YUV_STRETCH_COMPOSITION,
    GFX_BUFF_TYPE_YUV_ADV_STRETCH_COMPOSITION,
    GFX_BUFF_TYPE_YUV_ROTATE_STRETCH ,
    GFX_BUFF_TYPE_YUV_ROTATE_ADV_STRETCH,
    GFX_BUFF_TYPE_YUV_ROTATE_STRETCH_COMPOSITION,
    GFX_BUFF_TYPE_YUV_ROTATE_ADV_STRETCH_COMPOSITION,
    GFX_BUFF_TYPE_YUV_ROTATE_COMPOSITION,
    GFX_BUFF_TYPE_YUV_ROTATE,
    GFX_BUFF_TYPE_CLUT8_STRETCH_COMPOSITION,
    GFX_BUFF_TYPE_CLUT8_ADV_STRETCH_COMPOSITION,
    GFX_BUFF_TYPE_CLUT8_ROTATE_STRETCH ,
    GFX_BUFF_TYPE_CLUT8_ROTATE_ADV_STRETCH,
    GFX_BUFF_TYPE_CLUT8_ROTATE_STRETCH_COMPOSITION,
    GFX_BUFF_TYPE_CLUT8_ROTATE_ADV_STRETCH_COMPOSITION,
    GFX_BUFF_TYPE_CLUT8_ROTATE_COMPOSITION,
    GFX_BUFF_TYPE_CLUT8_ROTATE,
    GFX_BUFF_TYPE_FLIPGRADEFILL,
    //#endif

    #ifdef GFX_SUPPORT_DOUBLE_BUFFER
    GFX_BUFF_TYPE_FLIP,
    #endif

    #ifdef GFX_SUPPORT_3D_DOUBLE_BUFFER
    GFX_BUFF_TYPE_3D_FLIP,
    #endif

    #if GFX_3D_HOR_ROTATE
    GFX_BUFF_TYPE_3DHORROTATE,
    #endif
    
    GFX_BUFF_TYPE_MAX
} GFX_BUFF_CMD_TYPE_T;

typedef struct _GFX_EX_INFO_T
{
    GFX_APP_INSTANCEID_T e_app_id;
    GFX_HW_INSTANCEID_T  e_hw_id; 
    BOOL fg_OnScreen;
} GFX_EX_INFO_T;


#define OSD_PLANE_NUMS  5

typedef struct _GFX_FLIP_PLA_T
{
    BOOL fgPlaneNeedToFlip;
    UINT32 ui4Region;
}  GFX_FLIP_PLA_T;

typedef struct _GFX_FLIP_PLA_MULTI_T
{
    GFX_FLIP_PLA_T PlaneList[5] ;
}  GFX_FLIP_PLA_MULTI_T;

typedef struct _GFX_FLIP_PLA_MULTI_WITH_TIMER_T
{
    GFX_FLIP_PLA_T PlaneList[5] ;
    UINT32              ui4Timer;
}  GFX_FLIP_PLA_MULTI_WITH_TIMER_T;

typedef struct _GFX_EN_PLA_T
{
    BOOL  fgPlaneNeedToSet;  //
    BOOL  fgEnablePlane;        //flag : enable or disable 
}  GFX_EN_PLA_T;

#ifdef SUPPORT_FRAME_ACCURATE
typedef struct _GFX_EN_PLA_PTS_T
{
    UINT32  u4List;        //flag : enable or disable 
    UINT64 u8Pts;
    UINT64 u8Tickets;
    BOOL   needChangeEnableDisable;
    BOOL enable;

}  GFX_EN_PLA_PTS_T;

typedef struct _GFX_PTS_CALL_T
{
    DRV_CB_REG_INFO_T  rCallBack;        //flag : enable or disable 
    BOOL  fgNeedPtsCall;    
    UINT64 u8Pts;
    UINT64 u8Tickets;

}  GFX_PTS_CALL_T;

#endif

typedef struct
{
    VOID        __cross_space__    *pv_img_buf;        /* image data */
    VOID        __cross_space__    *pv_img_buf2;       /* image data2 */
    UINT32          ui4_img_size;       /* image size */
    VOID        __cross_space__    *pv_type_data;      /* palette data */

    VOID        __cross_space__    *pv_dst;            /* destination starting address */

    UINT32          ui4_dst_x;          /* x offset in the destination in pixels */
    UINT32          ui4_dst_y;          /* y offset in the destination in pixels */
    UINT32          ui4_dst_width;      /* expected output width in pixels */
    UINT32          ui4_dst_height;     /* expected output height in pixels */
    UINT32          ui4_dst_pitch;      /* pitch of the destination image */
    GFX_COLORMODE_T e_dst_cm;           /* destination color mode */
    
} RLE_DRV_DECODE_T;


typedef enum
{
    RLE_DRV_CMDBUF_PRIORITY_LOW = 1,       ///< Low priority,  
    RLE_DRV_CMDBUF_PRIORITY_MID = 3,       ///< Medium priority, 
    RLE_DRV_CMDBUF_PRIORITY_HIGH = 10,     ///< High priority,  
    RLE_DRV_CMDBUF_PRIORITY_MAX            ///< The max
} RLE_DRV_CMDBUF_PRIORITY;

//GFX Priority
typedef enum
{
    GFX_DRV_CMDBUF_PRIORITY_LOW = 1,       ///< Low priority, for normal image decoder driver
    GFX_DRV_CMDBUF_PRIORITY_MID = 3,       ///< Medium priority, for video decoder driver SD source using
    GFX_DRV_CMDBUF_PRIORITY_HIGH = 10,     ///< High priority, for video decoder driver HD source using
    GFX_DRV_CMDBUF_PRIORITY_HIGHEST = 15,     ///< Highest priority, for video decoder driver HD source using
    GFX_DRV_CMDBUF_PRIORITY_MAX            ///< The max
} GFX_DRV_CMDBUF_PRIORITY;

/// GFX  Flush Type
typedef enum
{
   GFX_DRV_FLUSH_NONE = 0,      ///< Not available.
   GFX_DRV_FORCE_FLUSH,         ///< force flush
    GFX_DRV_AUTO_FLUSH        ///< auto flush
}GFX_DRV_FLUSH_TYPE;
#define MT8530_GFX 
typedef enum _RLE_COLORMODE_T
{
    RLE_COLORMODE_AYCbCr_CLUT2  = 0,
    RLE_COLORMODE_AYCbCr_CLUT4  = 1,
    RLE_COLORMODE_AYCbCr_CLUT8  = 2,
    RLE_COLORMODE_CbYCrY_16     = 3,
    RLE_COLORMODE_YCbYCr_16     = 4,
    RLE_COLORMODE_AYCbCr_D8888  = 5,
    RLE_COLORMODE_ARGB_CLUT2    = 6,
    RLE_COLORMODE_ARGB_CLUT4    = 7,
    RLE_COLORMODE_ARGB_CLUT8    = 8,
    RLE_COLORMODE_RGB_D565      = 9,
    RLE_COLORMODE_ARGB_D1555    = 10,
    RLE_COLORMODE_ARGB_D4444    = 11,
    RLE_COLORMODE_ARGB_D8888    = 12,
    RLE_COLORMDOE_YUV_420_BLK   = 13,
    RLE_COLORMODE_YUV_420_RS    = 14,
    RLE_COLORMDOE_YUV_422_BLK   = 15,
    RLE_COLORMODE_YUV_422_RS    = 16,
    RLE_COLORMDOE_YUV_444_BLK   = 17,
    RLE_COLORMODE_YUV_444_RS    = 18
}RLE_COLORMODE_T;


/// GFX Cmd Buf State Machine
typedef enum
{
   GFX_CMDBUF_STATE_IDLE,               ///< Cmd buffer init state
   GFX_CMDBUF_STATE_ACTIVE,             ///< Cmd buffer active
   GFX_CMDBUF_STATE_WAITFLUSH,          ///< Cmd buffer wait flush
   GFX_CMDBUF_STATE_INFLUSH,            ///< Cmd buffer in flush
   GFX_CMDBUF_STATE_MAX                 ///< The max
} GFX_CMDBUF_STATE;

//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

typedef struct _GFX_LINUX_CMDBUF_T
{
    UINT32                  u4GfxCmdMaxCount;
    UINT32                  u4GfxCmdBufId;           ///< the GFXCmd Buf Id
    UINT32                  u4GfxPriority;           ///< priority
    UINT32                  u4GfxCurrPriority;       ///< current priority (priority will increase if not serviced)
    UINT32                  u4GfxHwInstId;           ///< GFX hardware instance id.
    UINT32                  u4GfxCmdBufState;         ///< Vg Cmd Buf state
    UINT32                  u4GfxCmdBufEvent;         ///< GFX Cmd Buf Event Handle
    UINT32                  u4WrPoint;
    UINT32                  u4RdPoint;
    UINT32                  u4GfxCmdBufCompress;
#if (defined(GFX_SUPPORT_DOUBLE_BUFFER) || defined(GFX_SUPPORT_3D_DOUBLE_BUFFER))
    BOOL                    fgWaitFlipDone;
    VOID*                   pvFlipEndBuf;
    VOID*                   pvFlipStartBuf;
    VOID*                   pvLastFlipBuf;
#endif
#ifdef GFX_SUPPORT_3D_DOUBLE_BUFFER
    VOID*                   pvFlipEndBufR;
    VOID*                   pvFlipStartBufR;
    VOID*                   pvLastFlipBufR;
#endif
} GFX_LINUX_CMDBUF_T;


#if (defined(GFX_SUPPORT_DOUBLE_BUFFER) || defined(GFX_SUPPORT_3D_DOUBLE_BUFFER))
extern BOOL         g_fgGfxHasPendingOnScreenCmd;
extern HANDLE_T     _h_Gfx_Flip_Sema;
#endif


typedef struct _GFX_DRV_CMDBUF_T
{
    GFX_LINUX_CMDBUF_T            *prGfxDrvCmdBuf;
    VOID                    *prPrevCmdBuf;          ///< previous Cmd Buf
    VOID                    *prNextCmdBuf;          ///< Next Cmd Buffer
    UINT32                  u4Private;
    UINT32                  ui4GfxCmdBufSema;          // /< GFX Cmd Buf Same
} GFX_DRV_CMDBUF_T;

typedef struct _GFX_FLUSH_TEST_T
{
    UINT32  ui4CmdBufId;
    UINT8   ui1ServiceHwId;
}GFX_FLUSH_TEST_T;

typedef struct _GFX_CMD_BUFF_T
{
    UINT32          ui4_gfx_ticketid;
    GFX_DRV_CMDBUF_PRIORITY e_cmdbuff_priority; 
} GFX_CMD_BUFF_T;

#if 0
typedef struct _RLE_REPLACE_COLOR_T
{
    COMMON_BITBLT_FIELDS

    UINT8           ui1_alpha;

    UINT32          *pui4_original_color;
    UINT32          *pui4_new_color;
    UINT8           ui1_color_num;
} RLE_REPLACE_COLOR_T;

typedef struct _RLE_BITBLT_T
{
    COMMON_BITBLT_FIELDS

    UINT8       ui1_alpha;
} RLE_BITBLT_T;

typedef struct _RLE_FILL_T
{
    VOID*           pv_dst;
    VOID*           pv_dst2;
    INT32           i4_dst_x;
    INT32           i4_dst_y;
    UINT32          ui4_dst_pitch;
    UINT32          ui4_dst_pitch2;
    RLE_COLORMODE_T e_dst_cm;
    BOOL           b_compressed;

    UINT32          ui4_width;
    UINT32          ui4_height;
    UINT32          ui4_color;
} RLE_FILL_T;


typedef enum
{
    RLE_IMG_ROTATE_NONE             = 0,    /* no rotation */
    RLE_IMG_ROTATE_CW_90            = 1,    /* clockwise  90 degrees  */
    RLE_IMG_ROTATE_CW_180           = 2,    /* clockwise 180 degrees  */
    RLE_IMG_ROTATE_CW_270           = 3,    /* clockwise 270 degrees  */
    RLE_IMG_ROTATE_NONE_WITH_FLIP   = 4,    /* no rotation, with flip */
    RLE_IMG_ROTATE_CW_90_WITH_FLIP  = 5,    /* clockwise  90 degrees, with flip */
    RLE_IMG_ROTATE_CW_180_WITH_FLIP = 6,    /* clockwise 180 degrees, with flip */
    RLE_IMG_ROTATE_CW_270_WITH_FLIP = 7     /* clockwise 270 degrees, with flip */
} RLE_IMG_ROTATE_T;


typedef struct  _RLE_IMG_DIRECT_DECODE_T
{	  
    VOID            *pv_img_buf;        /* image data */
    VOID            *pv_img_buf2;       /* image data2 */
    UINT32          ui4_img_size;       /* image size */
    VOID            *pv_type_data;      /* type data */

    UINT32          ui4_src_x;          /* x offset in the source image in pixels */
    UINT32          ui4_src_y;          /* y offset in the source image in pixels */
    UINT32          ui4_src_width;      /* width to be decoded in pixels */
    UINT32          ui4_src_height;     /* height to be decoded in pixels */

    VOID            *pv_dst;            /* destination starting address */
    VOID            *pv_dst2;           /* additional dst starting address */
    UINT32          ui4_dst_x;          /* x offset in the destination in pixels */
    UINT32          ui4_dst_y;          /* y offset in the destination in pixels */
    UINT32          ui4_dst_width;      /* expected output width in pixels */
    UINT32          ui4_dst_height;     /* expected output height in pixels */
    UINT32          ui4_dst_pitch;      /* pitch of the destination image */
    UINT32          ui4_dst2_pitch;     /* pitch of the additional destination image */
    RLE_COLORMODE_T e_dst_cm;           /* destination color mode */
    
    RLE_IMG_ROTATE_T    e_rotate;           /* rotation option */
}RLE_IMG_DIRECT_DECODE_T;
#endif
typedef struct _RLE_FILL_T
{
    INT32           u4TicketId;     \
    BOOL            bSyncEn;       \
    VOID*      __cross_space__     pv_dst;
    VOID*      __cross_space__     pv_dst2;
    INT32           i4_dst_x;
    INT32           i4_dst_y;
    UINT32          ui4_dst_pitch;
    UINT32          ui4_dst_pitch2;
    RLE_COLORMODE_T e_dst_cm;

    UINT32          ui4_width;
    UINT32          ui4_height;
    UINT32          ui4_color;
} RLE_FILL_T;



#define RLE_COMMON_BITBLT_FIELDS        \
    INT32           u4TicketId;     \
    BOOL            bSyncEn;       \
    VOID*      __cross_space__     pv_src;         \
    INT32           i4_src_x;       \
    INT32           i4_src_y;       \
    UINT32          ui4_src_pitch;  \
    RLE_COLORMODE_T e_src_cm;       \
                                    \
    VOID*      __cross_space__     pv_dst;         \
    INT32           i4_dst_x;       \
    INT32           i4_dst_y;       \
    UINT32          ui4_dst_pitch;  \
    RLE_COLORMODE_T e_dst_cm;       \
                                    \
    UINT32          ui4_width;      \
    UINT32          ui4_height;

typedef struct _RLE_REPLACE_COLOR_T
{
    RLE_COMMON_BITBLT_FIELDS

    UINT8           ui1_alpha;

    UINT32      __local_space__    *pui4_original_color;
    UINT32      __local_space__    *pui4_new_color;
    UINT8           ui1_color_num;
} RLE_REPLACE_COLOR_T;

typedef struct _RLE_BITBLT_T
{
    RLE_COMMON_BITBLT_FIELDS

    UINT8       ui1_alpha;
} RLE_BITBLT_T;

typedef struct _RLE_BITBLT_BASE_T
{
    RLE_COMMON_BITBLT_FIELDS
} RLE_BITBLT_BASE_T;


typedef struct _RLE_IMG_DIRECT_DECODE_T
{	  
    INT32           u4TicketId;   
    BOOL            bSyncEn;       
    VOID        __cross_space__    *pv_img_buf;        /* image data */
    VOID        __cross_space__    *pv_img_buf2;       /* image data2 */
    UINT32          ui4_img_size;       /* image size */
    VOID        __cross_space__    *pv_type_data;      /* type data */
		
    VOID        __cross_space__    *pv_dst;            /* destination starting address */

    UINT32          ui4_src_x;          /* x offset in the source image in pixels */
    UINT32          ui4_src_y;          /* y offset in the source image in pixels */
    UINT32          ui4_src_width;      /* width to be decoded in pixels */
    UINT32          ui4_src_height;     /* height to be decoded in pixels */

    UINT32          ui4_dst_x;          /* x offset in the destination in pixels */
    UINT32          ui4_dst_y;          /* y offset in the destination in pixels */
    UINT32          ui4_dst_width;      /* expected output width in pixels */
    UINT32          ui4_dst_height;     /* expected output height in pixels */
    UINT32          ui4_dst_pitch;      /* pitch of the destination image */
    UINT32          ui4_dst2_pitch;     /* pitch of the additional destination image */
    RLE_COLORMODE_T e_dst_cm;           /* destination color mode */    
    UINT32          ui4ModuleID;
#ifdef SUPPORT_FRAME_ACCURATE
    UINT64          ui8Tickets;
#endif
} RLE_IMG_DIRECT_DECODE_T;


#ifdef SUPPORT_FRAME_ACCURATE

typedef struct _RLE_CB_INFO_T
{
    UINT64         u8Tickets;
} RLE_CB_INFO_T;
#endif


#if 1//Rle_use new rm method
typedef struct _RLE_CMD_BUFF_T
{
    UINT32          ui4_rle_ticketid;
    RLE_DRV_CMDBUF_PRIORITY e_cmdbuff_priority; 
} RLE_CMD_BUFF_T;

#endif

//for GFX drv
#define GFX_SYNC_DRAW              (1<<0) //sync draw operation
#define GFX_DCACHEFLUSH            (1<<1) //need flush dcach
#define GFX_COMPRESSED_SURFACE     (1<<2) //WT surface
#define GFX_ON_SCREEN_SURFACE      (1<<3) //on screen draw operation

typedef struct _GFX_FLIP_T 
{
    UINT32            ui4_plane_id;
    GFX_HRGNLIST_T    rgnlist;
    UINT32            ui4_flip_id;
    BOOL              b_force;
} GFX_FLIP_T;


#ifdef GFX_SUPPORT_3D_DOUBLE_BUFFER
typedef struct _GFX_FLIP_3D_T 
{
    UINT32            ui4_plane_id;
    GFX_HRGNLIST_T    rgnlist_left;
    GFX_HRGNLIST_T    rgnlist_right;
    UINT32            ui4_flip_id;
    BOOL              b_force;
} GFX_FLIP_3D_T;
#endif

typedef struct _OSD_FLIP_CB_INFO_T
{
    UINT32          ui4_plane_id;
    UINT32          ui4_flip_id; //same as ui4_flip_id in GFX_FLIP_T
} OSD_FLIP_CB_INFO_T;

typedef VOID (*x_gfx_flip_cb_fct)(VOID* pvPriv, INT32 i4Result);    //osd call notify when flip done

#ifdef GFX_SUPPORT_SINGLE_BUFFER
typedef struct _GFX_SINGLEBUF_MODE_INFO_T
{
    BOOL                    bSingleBufMode;
    GFX_HRGNLIST_T          r_left_rnglist; 
    GFX_HRGNLIST_T          r_right_rnglist;
} GFX_SINGLEBUF_MODE_INFO_T;


 extern VOID _vGfxSetSingleBufStatus(BOOL bSingleBufMode, GFX_HRGNLIST_T prRgnListLeft, GFX_HRGNLIST_T prRgnListRight);
 extern VOID _vGfxSetSingleBufEvent(VOID);
#endif

typedef union _GFX_CMD_BUF_U
{   
#ifdef GFX_SUPPORT_DOUBLE_BUFFER
    GFX_FLIP_T                          rFlipCmd;
#endif
#ifdef GFX_SUPPORT_3D_DOUBLE_BUFFER
    GFX_FLIP_3D_T                    r3DFlipCmd;
#endif

    GFX_ALPHA_COMPOSITION_T             rAlphaCompositionCmd;
    GFX_FILL_T                          rFillCmd;
    GFX_BITBLT_T                        rBitbltCmd;
    GFX_TRANSPARENT_BITBLT_T            rTransparentBitbltCmd;
    GFX_TRANSPARENT_FILL_T              rTransparentFillCmd;
    GFX_REPLACE_COLOR_T                 rRplCmd;
    GFX_ALPHA_BLEND_T                   rAlphaBlendCmd;
    GFX_ALPHA_MUL_T                     rAlphaMulCmd;
    GFX_BITBLT_BASE_T                   rBitbltBaseCmd;
    GFX_STRETCH_BITBLT_T                rStretchBitbltCmd;
    GFX_ALPHAMAP_BITBLT_T               rAlphaMapBitbltCmd;
    GFX_YCBCR_TO_RGB_T                  rYc2RgbCmd;
    GFX_ROP_BITBLT_T                    rRopBitbltCmd;
    GFX_PLTBLT_T                        rPBitbltCmd;
    GFX_ROTATE_T                        rRotateCmd;
    
#ifdef GL_XOR
    GFX_ROTATE_STRETCH_ALPHACOMP_T      rRotateStretchAlphaCompCmd;
    GFX_BITBLT_MIRROR_FLIP_T            rBitbltMirrorFlipCmd;
    GFX_REFLECT_T                       rReflectCmd;
#endif

} GFX_CMD_BUF_U;

typedef struct _GFX_BUFF_CMD_ITEM_T
{
    //GFX_BUFF_CMD_TYPE_T e_Type; // command type
    UINT32              u4CmdType;
    //GFX_DRV_FLUSH_TYPE  eFlushType; // command type 
    UINT32              u4FlushType;
    //UINT32 u4TimeStamp; // (reserved) time stamp of the command issued
    //UINT32 u4EstDuration; // (reserved) estimated duration of this command

    //GFX_APP_INSTANCEID_T e_app_id; // (reserved) module identifier which issues this command
    //GFX_HW_INSTANCEID_T  e_hw_id; // (reserved)
    //BOOL fg_OnScreen; // (reserved) if the drawing is on on-screen surface

#if 0
    UINT32 u4CmdBuff[35]; // command data
#else
    UINT32 u4CmdBuff[(sizeof(GFX_CMD_BUF_U) + 3)/4];
#endif
} GFX_BUFF_CMD_ITEM_T;

typedef struct _GFX_BUF_INFO
{
    GFX_BUFF_CMD_ITEM_T                  *u4BufStart;             ///< buffer start address
    //GFX_BUFF_CMD_ITEM_T                  u4BufEnd;               ///< buffer end address
    UINT32                  u4RdPoint;              ///< buffer read point
    UINT32                  u4WrPoint;              ///< buffer write point
    UINT16                  u2CmdSize;
} GFX_BUF_FIFO;

typedef struct _GFX_CMDBUF_T
{
    UINT32                  u4GfxCmdBufId;           ///< the GFXCmd Buf Id
    UINT32                  u4GfxPriority;           ///< priority
    UINT32                  u4GfxCurrPriority;       ///< current priority (priority will increase if not serviced)
    UINT32                  u4GfxHwInstId;           ///< GFX hardware instance id.
    GFX_CMDBUF_STATE         eGfxCmdBufState;         ///< Vg Cmd Buf state
    HANDLE_T                hGfxCmdBufEvent;         ///< GFX Cmd Buf Event Handle
    HANDLE_T                hGfxCmdBufSema;          ///< GFX Cmd Buf Same
    HANDLE_T                hGfxCmdBufSema2;          ///< GFX Cmd Buf Same
    
    GFX_BUF_FIFO             rBufInfo;               ///< GFX Cmd Buffer Info
    struct _GFX_CMDBUF_T     *prPrevCmdBuf;          ///< previous Cmd Buf
    struct _GFX_CMDBUF_T     *prNextCmdBuf;          ///< Next Cmd Buffer
    UINT32                  u4Private;              ///< private data, used to verify if cmd buf is valid
    BOOL                    fg_compress; 
    UINT64                  ui8GfxOperationSize;
#if (defined(GFX_SUPPORT_DOUBLE_BUFFER) || defined(GFX_SUPPORT_3D_DOUBLE_BUFFER))
    BOOL                    fgWaitFlipDone;
    VOID*                   pvFlipEndBuf;
    VOID*                   pvFlipStartBuf;
    VOID*                   pvLastFlipBuf;
#endif
#ifdef GFX_SUPPORT_3D_DOUBLE_BUFFER
    VOID*                   pvFlipEndBufR;
    VOID*                   pvFlipStartBufR;
    VOID*                   pvLastFlipBufR;
#endif
} GFX_CMDBUF_T;

#ifdef __cplusplus
}
#endif

typedef struct _GFX_LOSSLESS_REGION_INFO_T
{
    UINT32  ui4_region_id;
    UINT32  ui4_src_y;
    UINT32  ui4_dst_y;
    UINT32  ui4_height;
} GFX_LOSSLESS_REGION_INFO_T;

#define  GFX_LOSSLESS_FIRST_REGION_ID     0x1<<0
#define  GFX_LOSSLESS_SECOND_REGION_ID    0x1<<1
#define  GFX_LOSSLESS_THIRD_REGION_ID     0x1<<2
#define  GFX_LOSSLESS_FOURTH_REGION_ID    0x1<<3
#define  GFX_LOSSLESS_FIFTH_REGION_ID     0x1<<4

#define  GFX_LOSSLESS_REGION_HEIGHT       216
#define  GFX_LOSSLESS_REGION_COUNT        5

#define  GFX_LOSSLESS_REGION_TYPE_NOCHANGE  0X1<<0
#define  GFX_LOSSLESS_REGION_TYPE_LOSSLESS  0X1<<1
#define  GFX_LOSSLESS_REGION_TYPE_WT        0X1<<2
#endif
