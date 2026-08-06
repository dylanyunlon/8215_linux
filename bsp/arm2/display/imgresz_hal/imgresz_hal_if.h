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

#ifndef _HAL_IMGRESZ_IF_H_
#define _HAL_IMGRESZ_IF_H_

#include "drv_config.h"
#include "drv_def.h"
#include "chip_ver.h"
#include "sys_config.h"
#include "types.h"
#include "x_assert.h"

typedef unsigned int __u32;
typedef unsigned char __u8;
typedef unsigned int uintptr_t;
typedef unsigned int u32;
typedef int __s32;
typedef int s32;
typedef unsigned char u8;

//typedef BOOL bool;


#ifdef __cplusplus
extern "C" {
#endif

/*! \name Image Resizer HAL Interface
* @{
*/
#if CONFIG_DRV_FPGA_BOARD
#define IMGRESZ_HAL_EMU
#endif

#define IMGRESZ_IO_MMU_TEST    0

#define IMGRESZ_SUPPORT_RESET_DEST_BUFFER 0

typedef struct {

  __u32 u4BufVA;
  __u32 u4BufPA;
} ADDR_INFO_T, *PADDR_INFO_T;


typedef enum {
    IMGRESZ_HAL_RESIZE_MODE_FRAME,                  ///< Resize from whole frame to whole frame
    IMGRESZ_HAL_RESIZE_MODE_PARTIAL                 ///< Resize from partial frame to partial frame
} IMGRESZ_HAL_RESIZE_MODE_T;


typedef enum {
    IMGRESZ_HAL_RESAMPLE_METHOD_BILINEAR,           ///< Bi-linear resample method
    IMGRESZ_HAL_RESAMPLE_METHOD_4_TAP,              ///< 4-tap resample method
    IMGRESZ_HAL_RESAMPLE_METHOD_8_TAP               ///< 8-tap resample method
} IMGRESZ_HAL_RESAMPLE_METHOD_T;


typedef enum {
    IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER=1,     ///< Y, C buffer supporting YUV422/YUV420, Block/RasterScan mode, Address swap/No swap.
    IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_CB_CR_BUFFER, ///< Y, Cb, Cr buffer.
    IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_INDEX_BUFFER,   ///< Color index buffer with color pallet.
    IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_ARGB_BUFFER,    ///< ARGB color buffer.
    IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_AYUV_BUFFER     ///< AYUV color buffer.
} IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_T;


typedef enum {
    IMGRESZ_HAL_IMG_YUV_FORMAT_420,                 ///< YUV 420
    IMGRESZ_HAL_IMG_YUV_FORMAT_422,                 ///< YUV 422
    IMGRESZ_HAL_IMG_YUV_FORMAT_444                  ///< YUV 444
} IMGRESZ_HAL_IMG_YUV_FORMAT_T;


typedef enum {
    IMGRESZ_HAL_INDEX_BUFFER_FORMAT_2BPP,           ///< 2 bits per pixel index format.
    IMGRESZ_HAL_INDEX_BUFFER_FORMAT_4BPP,           ///< 4 bits per pixel index format.
    IMGRESZ_HAL_INDEX_BUFFER_FORMAT_8BPP            ///< 8 bits per pixel index format.
} IMGRESZ_HAL_INDEX_BUFFER_FORMAT_T;


typedef enum {
    IMGRESZ_HAL_ARGB_BUFFER_FORMAT_0565,            ///< Bit number of (A,R,G,B) = (0,5,6,5)
    IMGRESZ_HAL_ARGB_BUFFER_FORMAT_1555,            ///< Bit number of (A,R,G,B) = (1,5,5,5)
    IMGRESZ_HAL_ARGB_BUFFER_FORMAT_4444,            ///< Bit number of (A,R,G,B) = (4,4,4,4)
    IMGRESZ_HAL_ARGB_BUFFER_FORMAT_8888,            ///< Bit number of (A,R,G,B) = (8,8,8,8)
} IMGRESZ_HAL_ARGB_BUFFER_FORMAT_T;


typedef struct {
    __u8 u1A;                                      ///< Alfa blanding.
    __u8 u1R;                                      ///< Color Red.
    __u8 u1G;                                      ///< Color Green.
    __u8 u1B;                                      ///< Color Blue.
} IMGRESZ_HAL_ARGB_COLOR_T;


typedef struct {
    IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_T eBufferMainFormat;      ///< Buffer main format
    BOOL fgJpg;
    BOOL fgProgressiveFrame;                                  ///< If this is progressive frame or interlace field.
    BOOL fgTopField;                                          ///< When interlace field, if this is top field or bottom field.
    IMGRESZ_HAL_IMG_YUV_FORMAT_T eYUVFormat;                  ///< (Only for Y_C_BUFFER) YUV format.
    BOOL fgBlockMode;                                         ///< (Only for Y_C_BUFFER) True for Block Mode. False for RasterScan Mode.
    BOOL fgAddrSwap;                                          ///< (Only for Y_C_BUFFER) Address swap.
    __u32 u4HSampleFactor[3];                                ///< (Only for Y_CB_CR_BUFFER) Horizontal Sample Facotr of component 0,1,2
    __u32 u4VSampleFactor[3];                                ///< (Only for Y_CB_CR_BUFFER) Vertical Sample Facotr of component 0,1,2
    IMGRESZ_HAL_INDEX_BUFFER_FORMAT_T eIndexBufferFormat;     ///< (Only for INDEX_BUFFER) Index Buffer Format.
    IMGRESZ_HAL_ARGB_COLOR_T *prColorPallet;                  ///< (Only for INDEX_BUFFER) Color pallet.
    IMGRESZ_HAL_ARGB_BUFFER_FORMAT_T eARGBBufferFormat;       ///< (Only for ARGB_BUFFER) ARGB Buffer Format.
    BOOL fgWT;                                                ///< (Only for ARGB_BUFFER,AYUV_BUFFER) WT compression                                             ///< (Only for ARGB_BUFFER,AYUV_BUFFER) WT compression
} IMGRESZ_HAL_IMG_BUF_FORMAT_T;


typedef struct {
    IMGRESZ_HAL_IMG_BUF_FORMAT_T rBufferFormat;      ///< Buffer format
    uintptr_t u4BufSA;   
   uintptr_t u4BufSA1;                                 ///< Used as Y buffer start address of Y_C_BUFFER and Y_CB_CR_BUFFER, or the start address of INDEX_BUFFER, ARGB_BUFFER, and AYUV buffer.
    uintptr_t u4BufSA2;                                 ///< Used as C buffer start address of Y_C BIFFER, or Cb buffer start address of Y_CB_CR_BUFFER.
    uintptr_t u4BufSA3;                                 ///< Used as Cr buffer start address of Y_CB_CR_BUFFER.
    __u32 u4BufWidth;                               ///< Buffer width
    __u32 u4BufHeight;                              ///< Buffer height
    __u32 u4YBufWidth;                               ///< Y Buffer width
    __u32 u4CBufWidth;                               ///< C Buffer width
    __u32 u4ImgXOff;                                ///< Image left margin X offset to buffer left margin.
    __u32 u4ImgYOff;                                ///< Image top margin Y offset to buffer top margin.
    __u32 u4ClipImgXOff;                                ///< Clip Image left margin X offset to buffer left margin.
    __u32 u4ClipImgYOff;                                ///<Clip Image top margin Y offset to buffer top margin.
    __u32 u4ImgWidth;                               ///< Image width.
    __u32 u4ClipImgHeight;                              ///< Image height.
    __u32 u4ClipImgWidth;                               ///<Clip Image width.
    __u32 u4SrcClipYHeight;
    __u32 u4SrcClipYWidth;
    __u32 u4SrcClipCHeight;
    __u32 u4SrcClipCWidth;
    BOOL   fgClipen;
    __u32 u4ImgHeight;                              ///< Image height.
    __u32 u1Alpha;                                     ///< Image alpha value. (Only used for blending buffer)
    BOOL fgPreloadBuf;                                ///PreloadBuf load  file sucess flag       
    __u32 u4AlphaScalingType;                   /// alpha scaling type choose
    BOOL fgBilinearBoundary;                       /// If fgBilinearBoundary is 1, bilinear mode for alpha change in 4 & 8-tap filters
    BOOL fgOnlyDistinquishAlpha;                 ///If fgOnlyDistinquishAlpha is 1, only distinguish alpha = 0.
    BOOL gracefully;                                    /// gracefully reset flag ,if gracefully is 1,gracefully reset start.
    __u32 u4LumaKeyScalingType;   /// LumaKeyScalingType 0 ,1 ,2  
    BOOL fgYUVMode;                       /// fgYUVMode flag   
    BYTE *u4CurFileNum;
    __u32 u4Curcase;                ////CRC Checksum current number
    __u32 Checksumdata;          ////CRC Checksum data
    BOOL Checksu_flag;              ////CRC Checksum flag
} IMGRESZ_HAL_IMG_INFO_T;


typedef struct {
    BOOL fgFirstRowBuf;                              ///< If this partial row buffer is the first row buffer.
    BOOL fgLastRowBuf;                               ///< If this partial row buffer is the last row buffer.
    __u32 u4RowBufHeight;                           ///< If this is 0, determine the height by Jpeg vertical sample factor.
   uintptr_t u4CurRowBufSA1;                           ///< Current row buffer start address of component 1.
    uintptr_t u4CurRowBufSA2;                           ///< Current row buffer start address of component 2.
    uintptr_t u4CurRowBufSA3;                           ///< Current row buffer start address of component 3.
   uintptr_t u4PrevRowBufSA1;                          ///< Previous row buffer start address of component 1 when current is not first row.
    uintptr_t u4PrevRowBufSA2;                          ///< Previous row buffer start address of component 2 when current is not first row.
   uintptr_t u4PrevRowBufSA3;                          ///< Previous row buffer start address of component 3 when current is not first row.
} IMGRESZ_HAL_PARTIAL_BUF_INFO_T;


typedef struct {
    BOOL fgPictureMode;                              ///< If this is picture mode.
    BOOL fgPreloadMode;                              ///< If this is progressive preload mode.
    BOOL fgYExist;                                   ///< If Y component exist
    BOOL fgCbExist;                                  ///< If Cb component exist
    BOOL fgCrExist;                                  ///< If Cr component exist
} IMGRESZ_HAL_JPEG_INFO_T;


typedef struct {
    BOOL fgRPRMode;                                        ///< If this is RM mode.
    BOOL fgRPRRacingModeEnable;                 ///< If this is Tracing mode enable.
} IMGRESZ_HAL_RM_INFO_T;

typedef struct {
    __u32 u4SrcCntY;                                ///< Source Count Y
    __u32 u4SrcCntCb;                               ///< Source Count Cb
    __u32 u4SrcCntCr;                               ///< Source Count Cr
    __u32 u4VOffsetY;                               ///< Vertical offset Y
    __u32 u4VOffsetCb;                              ///< Verfical offset Cb
    __u32 u4VOffsetCr;                              ///< Verfical offset Cr
    __u32 u4VNextCY;                                ///< Vertical next C Y
    __u32 u4VNextCCb;                               ///< Verfical next C Cb
    __u32 u4VNextCCr;                               ///< Verfical next C Cr
    } IMGRESZ_HAL_HW_STATUS_T;


/// Notify callback function
typedef __s32 (*IMGRESZ_HAL_NOTIFY)(__s32 i4State,void *pvPrivData);
typedef struct
{
  IMGRESZ_HAL_NOTIFY pvCallBackFunc;
  void *pvPrivData;
} IMGRESZ_HAL_NOTIFY_CB_REG_T;

extern void ImgrGetHwRegAddress(void);

/// Initialize Image Resizer HAL when boot up
/// \return If return value < 0, it's failed. Please reference imgresz_hal_errcode.h.
__s32 i4ImgResz_HAL_Boot_Init( void);

void i4ImgResz_HAL_Set_Interupt_Enable(void);

/// Uninitialize Image Resizer HAL when boot down
/// \return If return value < 0, it's failed. Please reference imgresz_hal_errcode.h.
__s32 i4ImgResz_HAL_Boot_Uninit( void);
void vImgreszSetMMU(__u32 u4ImgResizerID, BOOL fg2way);


/// Initialize Image Resizer HAL
/// \return If return value < 0, it's failed. Please reference imgresz_hal_errcode.h.
__s32 i4ImgResz_HAL_Init(
    __u32 u4ImgResizerID,                               ///< [IN] Image Resizer hardware ID
    uintptr_t u4tableaddr
);


/// Uninitialize Image Resizer HAL
/// \return If return value < 0, it's failed. Please reference imgresz_hal_errcode.h.
__s32 i4ImgResz_HAL_Uninit(
    __u32 u4ImgResizerID                               ///< [IN] Image Resizer hardware ID
);

__s32 i4ImgResz_Gracefully_Reset(
    __u32 u4ImgReszID                               ///< [IN] Image Resizer hardware ID
);
/// Set Image Resizer HAL resize mode
/// \return If return value < 0, it's failed. Please reference imgresz_hal_errcode.h.
__s32 i4ImgResz_HAL_Set_Resize_Mode(
    __u32 u4ImgResizerID,                              ///< [IN] Image Resizer hardware ID
    IMGRESZ_HAL_RESIZE_MODE_T eResizeMode               ///< [IN] Resize mode
);


/// Set Image Resizer HAL resample method
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Set_Resample_Method(
    __u32 u4ImgResizerID,                              ///< [IN] Image Resizer hardware ID
    IMGRESZ_HAL_RESAMPLE_METHOD_T eHResampleMethod,     ///< [IN] Horizontal Resample method
    IMGRESZ_HAL_RESAMPLE_METHOD_T eVResampleMethod      ///< [IN] Vertical Resample method
);


/// Set Image Resizer HAL source image info.
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Set_Source_Image_Info(
    __u32 u4ImgResizerID,                              ///< [IN] Image Resizer hardware ID
    IMGRESZ_HAL_IMG_INFO_T *prSrcImgInfo                ///< [IN] Source image infomation.
);


/// Set Image Resizer HAL destination image info.
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Set_Destination_Image_Info(
    __u32 u4ImgResizerID,                              ///< [IN] Image Resizer hardware ID
    IMGRESZ_HAL_IMG_INFO_T *prDestImgInfo               ///< [IN] Destination image infomation.
);


/// Set Image Resizer HAL blending image info.
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Set_Blending_Image_Info(
    __u32 u4ImgReszID,                              ///< [IN] Image Resizer hardware ID
    IMGRESZ_HAL_IMG_INFO_T *prBldImgInfo             ///< [IN] Blending image infomation.
);
 void vHwImgReszPrinterRegister(__u32 u4ImgReszID);


/// Set Image Resizer HAL partial mode information
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Set_Partial_Mode_Info(
    __u32 u4ImgReszID,                              ///< [IN] Image Resizer hardware ID
    IMGRESZ_HAL_PARTIAL_BUF_INFO_T *prSrcRowBufInfo, ///< [IN] Source row buffer infomation.
   uintptr_t u4TempBufSa                               ///< [IN] The start address of temporary buffer for partial mode.
                                                     ///<      The size of temp buffer is destination image width * 1 bytes.
);


/// Set Image Resizer HAL Jpeg information
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Set_Jpeg_Info(
    __u32 u4ImgReszID,                              ///< [IN] Image Resizer hardware ID
    IMGRESZ_HAL_JPEG_INFO_T *prJpegInfo              ///< [IN] Jpeg information.
);


/// Set Image Resizer HAL RM(RPR) information
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Set_RM_Info(
    __u32 u4ImgReszID,                              ///< [IN] Image Resizer hardware ID
    IMGRESZ_HAL_RM_INFO_T *prRMInfo              ///< [IN] rm information.
);

/// Set Image Resizer HAL Luma key information
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Set_LumaKey(
    __u32 u4ImgReszID,                              ///< [IN] Image Resizer hardware ID
    BOOL fgEnable,                                   ///< [IN] Luma key enable flag
    __u8 u1LumaKey                                  ///< [IN] Luma key value
);

__s32 i4ImgResz_HAL_Set_Scale1to1(
    __u32 u4ImgReszID,                              ///< [IN] Image Resizer hardware ID
    BOOL fgEnable                                   ///< [IN] scale1:1 enable flag                            
);

__s32 i4ImgResz_HAL_Set_Scale4to1(
    __u32 u4ImgReszID,                              ///< [IN] Image Resizer hardware ID
    BOOL fgEnable                                   ///< [IN] scale4:1 enable flag                            
);


/// Image Resizer HAL do resize.
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Resize(
    __u32 u4ImgResizerID                               ///< [IN] Image Resizer hardware ID
);




/// Image Resizer HAL Get resize status.
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Get_Resize_Status(
    __u32 u4ImgResizerID                               ///< [IN] Image Resizer hardware ID
);


/// Image Resizer HAL Get HW status.
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Get_HW_Status(
    __u32 u4ImgReszID,                              ///< [IN] Image Resizer hardware ID
    IMGRESZ_HAL_HW_STATUS_T *prHwStatus              ///< [OUT] Hardware status
);


/// Image Resizer HAL Set HW status.
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Set_HW_Status(
    __u32 u4ImgReszID,                              ///< [IN] Image Resizer hardware ID
    IMGRESZ_HAL_HW_STATUS_T *prHwStatus              ///< [IN] Hardware status
);


/// Image Resizer HAL register notify callback function.
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Reg_Notify_Callback(
    __u32 u4ImgReszID,                              ///< [IN] Image Resizer hardware ID
    IMGRESZ_HAL_NOTIFY_CB_REG_T *prNofifyCallback    ///< ]IN] Nofity callback function
);

#if 0

/// Image Resizer HAL unregister notify callback function.
/// \return If return value < 0, it's failed. Please reference hal_img_resizer_errcode.h.
__s32 i4ImgResz_HAL_Unreg_Notify_Callback(
    __u32 u4ImgReszID,                              ///< [IN] Image Resizer hardware ID
    IMGRESZ_HAL_NOTIFY_CB_REG_T *prNofifyCallback    ///< ]IN] Nofity callback function
);
#endif

//
/*! @} */


#ifdef __cplusplus
}
#endif

#endif //#ifndef _HAL_VDEC_MPEG_IF_H_

