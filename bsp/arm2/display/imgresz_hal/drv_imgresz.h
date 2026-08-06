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

#ifndef __IMGRESZ_DRV_DRV_H
#define __IMGRESZ_DRV_DRV_H

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "x_types.h"
#include "x_assert.h"
#include "imgresz_hal_if.h"


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

#define IMGRESZ_ALPHA_BLENDING 1

#define USE_IMGRESZ_DRV_DLL    0
/*added by mtk68119 for ioctl commands*/

/// Image Resizer Scale Mode
typedef enum
{
    IMGRESZ_DRV_NONE_SCALE,             ///< Not assign scale mode yet.
    IMGRESZ_DRV_FRAME_SCALE,            ///< Frame mode, scale whole frame at once.
    IMGRESZ_DRV_PARTIAL_SCALE,          ///< Partial mode decode, scale seperate partial of one frame.
    IMGRESZ_DRV_JPEG_PIC_SCALE,         ///< Jpeg picture mode decode
    IMGRESZ_DRV_SCALE_MODE_MAX          ///< The max
} IMGRESZ_DRV_SCALE_MODE;


/// Image Resizer Scale Priority
typedef enum
{
    IMGRESZ_DRV_PRIORITY_LOW = 1,       ///< Low priority, for normal image decoder driver
    IMGRESZ_DRV_PRIORITY_MID = 3,       ///< Medium priority, for video decoder driver SD source using
    IMGRESZ_DRV_PRIORITY_HIGH = 10,     ///< High priority, for video decoder driver HD source using
    IMGRESZ_DRV_PRIORITY_MAX            ///< The max
} IMGRESZ_DRV_SCALE_PRIORITY;


/// Image Resizer State Machine
typedef enum
{
    IMGRESZ_DRV_STATE_NONE,             ///< Instance is released.
    IMGRESZ_DRV_STATE_IDLE,             ///< Instance is lock but not trigger to do scale yet,
                                        ///< or instance has finish scale, but not release yet.
    IMGRESZ_DRV_STATE_START,            ///< Instance has trigger do scale, but not be serviced yet.
    IMGRESZ_DRV_STATE_SCALING,          ///< Instance is serviced to do scale.
    IMGRESZ_DRV_STATE_WAIT_STOP,        ///< When scaling and receive stop command, state change to wait stop. If stop, state change to IDLE.
    IMGRESZ_DRV_STATE_MAX               ///< The max
} IMGRESZ_DRV_STATE;


/// Image Resizer Input(Source Buffer) Color Mode
typedef enum
{
    IMGRESZ_DRV_INPUT_COL_MD_NONE = 0,      ///< Not available.
    IMGRESZ_DRV_INPUT_COL_MD_JPG_DEF,       ///< Video mode, only use in Jpeg partial and picture mode.
    IMGRESZ_DRV_INPUT_COL_MD_420_BLK,       ///< Video mode, YUV420 block mode.
    IMGRESZ_DRV_INPUT_COL_MD_422_BLK,       ///< Video mode, YUV422 block mode.
    IMGRESZ_DRV_INPUT_COL_MD_420_RS,        ///< Video mode, YUV420 raster scan mode.
    IMGRESZ_DRV_INPUT_COL_MD_422_RS,        ///< Video mode, YUV422 raster scan mode.
    IMGRESZ_DRV_INPUT_COL_MD_AYUV,          ///< Video mode, AYUV 8888 mode.
    IMGRESZ_DRV_INPUT_COL_MD_2BPP_IDX,      ///< Graph mode, 2 bits per pixel index mode.
    IMGRESZ_DRV_INPUT_COL_MD_4BPP_IDX,      ///< Graph mode, 4 bits per pixel index mode.
    IMGRESZ_DRV_INPUT_COL_MD_8BPP_IDX,      ///< Graph mode, 8 bits per pixel index mode.
    IMGRESZ_DRV_INPUT_COL_MD_ARGB_1555,     ///< Graph mode, ARGB 1555 mode.
    IMGRESZ_DRV_INPUT_COL_MD_RGB_565,       ///< Graph mode, RGB 565 mode.
    IMGRESZ_DRV_INPUT_COL_MD_ARGB_4444,     ///< Graph mode, ARGB 4444 mode.
    IMGRESZ_DRV_INPUT_COL_MD_ARGB_8888      ///< Graph mode, ARGB 8888 mode.
} IMGRESZ_DRV_SRC_COLOR_MODE;


/// Image Resizer Output(Destination Buffer) Color Mode
typedef enum
{
    IMGRESZ_DRV_OUTPUT_COL_MD_NONE = 0,     ///< Not available.
    IMGRESZ_DRV_OUTPUT_COL_MD_420_BLK,      ///< Video mode, YUV420 block mode
    IMGRESZ_DRV_OUTPUT_COL_MD_422_BLK,      ///< Video mode, YUV422 block mode
    IMGRESZ_DRV_OUTPUT_COL_MD_420_RS,       ///< Video mode, YUV420 raster scan mode
    IMGRESZ_DRV_OUTPUT_COL_MD_422_RS,       ///< Video mode, YUV422 raster scan mode
    IMGRESZ_DRV_OUTPUT_COL_MD_AYUV,         ///< Video mode, AYUV 8888 mode.
    IMGRESZ_DRV_OUTPUT_COL_MD_ARGB_1555,    ///< Graph mode, ARGB 1555 mode.
    IMGRESZ_DRV_OUTPUT_COL_MD_RGB_565,      ///< Graph mode, RGB 565 mode.
    IMGRESZ_DRV_OUTPUT_COL_MD_ARGB_4444,    ///< Graph mode, ARGB 4444 mode.
    IMGRESZ_DRV_OUTPUT_COL_MD_ARGB_8888     ///< Graph mode, ARGB 8888 mode.
} IMGRESZ_DRV_DST_COLOR_MODE;


/// Image Resizer Output(Destination Buffer) Color Mode
typedef enum
{
    IMGRESZ_DRV_BLENDING_COL_MD_SAME = 0,   ///< The same as source buffer or destination buffer
} IMGRESZ_DRV_BLD_COLOR_MODE;

typedef enum
{
    IMGRESZ_MODULE_JPEC,
    IMGRESZ_MUDULE_DISP,
    IMGRESZ_MODULE_MAX
} IMGRESZ_MODULE_T;

/// Image Resizer Specific Component Factor, only for jpeg picture scale using
typedef struct _IMGRESZ_DRV_COMPONENT_FACTOR_T
{
    __u32                      u4Components;       ///< The number of components in jpeg
    __u8                       u1YCompFactorH;     ///< Horizontal factor of Y component.
    __u8                       u1YCompFactorV;     ///< Vertical factor of Y component.
    __u8                       u1CbCompFactorH;    ///< Horizontal factor of Cb component.
    __u8                       u1CbCompFactorV;    ///< Vertical factor of Cb component.
    __u8                       u1CrCompFactorH;    ///< Horizontal factor of Cr component.
    __u8                       u1CrCompFactorV;    ///< Vertical factor of Cr component.
} IMGRESZ_DRV_COMPONENT_FACTOR_T;


/// Image Resizer Source Buffer Information
typedef struct _IMGRESZ_DRV_SRC_BUF_INFO_T
{
    IMGRESZ_DRV_SRC_COLOR_MODE  eSrcColorMode;  ///< The color mode of input source buffer.
    uintptr_t                    u4YBufAddr;     ///< In video mode, it means the Y buffer.
                                                ///< In graph mode, it means the graph buffer.
    uintptr_t                    u4CbBufAddr;    ///< Only used in video and jpeg mode.
    uintptr_t                    u4CrBufAddr;    ///< Only used in jpeg partial and picture mode.
    __u32                      u4BufWidth;     ///< The buffer width (pitch).
    __u32                      u4BufHeight;    ///< The buffer height.
    __u32                      u4PicPosX;      ///< The picture position X.
    __u32                      u4PicPosY;      ///< The picture position Y.
    __u32                      u4PicWidth;     ///< The picture width.
    __u32                      u4PicHeight;    ///< The picture height.
    BOOL                        fgInterlaced;   ///< Interlaced picture.
    BOOL                        fgTopField;     ///< Interlaced picture top field exist. (For interlaced video only)
    BOOL                        fgBottomField;  ///< Interlaced picture bottom field exist. (For interlaced video only)
    IMGRESZ_DRV_COMPONENT_FACTOR_T  rCompFactor;    ///< Used when eSrcColorMode == IMGRESZ_INPUT_COL_MD_JPG_DEF.
    uintptr_t                      u4ColorPalletSa; ///< Color Pallet.
    BOOL                        fgWTEnable;     ///< Wavelet transform compression.
} IMGRESZ_DRV_SRC_BUF_INFO_T;


/// Image Resizer Destination Buffer Information
typedef struct _IMGRESZ_DRV_DST_BUF_INFO_T
{
    IMGRESZ_DRV_DST_COLOR_MODE  eDstColorMode;  ///< The color mode of output destination buffer.
     uintptr_t                       u4YBufAddr;     ///< In video mode, it means the Y buffer.
                                                ///< In graph mode, it means the graph buffer.
     uintptr_t                       u4CBufAddr;     ///< Only used in video mode.
    __u32                      u4BufWidth;     ///< The buffer width (pitch).
    __u32                      u4BufHeight;    ///< The buffer height.
    __u32                      u4PicPosX;      ///< The picture position X.
    __u32                      u4PicPosY;      ///< The picture position Y.
    __u32                      u4PicWidth;     ///< The picture width.
    __u32                      u4PicHeight;    ///< The picture height.
    BOOL                        fgWTEnable;     ///< Wavelet transform compression.    
} IMGRESZ_DRV_DST_BUF_INFO_T;


/// Image Resizer Blending Buffer Information
typedef struct _IMGRESZ_DRV_BLD_BUF_INFO_T
{
    IMGRESZ_DRV_BLD_COLOR_MODE  eBldColorMode;  ///< The color mode of output destination buffer.
     uintptr_t                       u4YBufAddr;     ///< In video mode, it means the Y buffer.
                                                ///< In graph mode, it means the graph buffer.
     uintptr_t                       u4CbBufAddr;    ///< Only used in video and jpeg mode.
     uintptr_t                      u4CrBufAddr;    ///< Only used in jpeg partial and picture mode.
    __u32                      u4BufWidth;     ///< The buffer width (pitch).
    __u32                      u4BufHeight;    ///< The buffer height.
    __u32                      u4PicPosX;      ///< The picture position X.
    __u32                      u4PicPosY;      ///< The picture position Y.
    __u32                      u4PicWidth;     ///< The picture width.
    __u32                      u4PicHeight;    ///< The picture height.
    __u8                       u1Alpha;        ///< Alpha value. Alpha * blending buffer.
    BOOL                        fgBlendBeforeResz; /// < True: Blending before resize. False: Blending after resize.
} IMGRESZ_DRV_BLD_BUF_INFO_T;


/// Image Resizer Partial Buffer Info
typedef struct _IMGRESZ_DRV_PARTIAL_INFO_T
{
    uintptr_t                      u4YBufAddr;     ///< In graph mode, it means the graph buffer.
     uintptr_t                       u4CbBufAddr;    ///< Only used in jpeg mode.
     uintptr_t                      u4CrBufAddr;    ///< Only used in jpeg mode.
    __u32                      u4YBufLine;     ///< Y buffer line number (if eSrcColorMode != IMGRESZ_INPUT_COL_MD_JPG_DEF).
    __u32                      u4CbBufLine;    ///< Cb buffer line number (if eSrcColorMode != IMGRESZ_INPUT_COL_MD_JPG_DEF).
    __u32                      u4CrBufLine;    ///< Cr buffer line number (if eSrcColorMode != IMGRESZ_INPUT_COL_MD_JPG_DEF).
    BOOL                        fgFirstRow;     ///< The First row.
    BOOL                        fgLastRow;      ///< The Last row.
} IMGRESZ_DRV_PARTIAL_INFO_T;


/// Image Resizer Jpg Info
typedef struct _IMGRESZ_DRV_JPEG_INFO_T
{
    BOOL                        fgExistY;       ///< Exist Y component
    BOOL                        fgExistCb;      ///< Exist Cb component
    BOOL                        fgExistCr;      ///< Exist Cr component
    BOOL                        fgPreload;      ///< Progressive preload mode
    __u32                      u4HwId;         ///< Indicate image resizer hardware ID for picture mode
} IMGRESZ_DRV_JPEG_INFO_T;

/// Imgage Resizer RM video related info
typedef struct _IMGRESZ_DRV_RM_INFO_T
{
    BOOL                        fgRPRMode;     ///< PRP mode or not for rm video
    BOOL                        fgRacingMode;  ///< Racing mode or not
} IMGRESZ_DRV_RM_INFO_T;


/// Image Resizer Do Scale Structure
typedef struct _IMGRESZ_DRV_DO_SCALE_T
{
	__u32 u4Unused;
} IMGRESZ_DRV_DO_SCALE_T;


/// Image Resizer Scaling State
typedef struct _IMGRESZ_DRV_SCALE_STATE_T
{
    IMGRESZ_DRV_STATE           eState;             ///< The Scale State
} IMGRESZ_DRV_SCALE_STATE_T;


/// Image Resizer Ticket
typedef struct _IMGRESZ_DRV_TICKET_T
{
    __u32                      u4Ticket;           ///< The ticket
} IMGRESZ_DRV_TICKET_T;


/// Notify callback function
typedef __s32 (*IMGRESZ_DRV_NOTIFY)(__s32 i4State,void *pvPrivData);
typedef struct
{
  IMGRESZ_DRV_NOTIFY pvCallBackFunc;
  void *pvPrivData;
} IMGRESZ_DRV_NOTIFY_CB_REG_T;


/// Get a image resizer ticket.
/// You must get a ticket first and then you can control image resizer by the ticket.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
#if USE_IMGRESZ_DRV_DLL
extern __s32 i4ImgResz_Drv_GetTicket(
    IMGRESZ_MODULE_T eModule, 
    IMGRESZ_DRV_TICKET_T *pImgReszTicket);
#else
extern __s32 i4ImgResz_Drv_GetTicket(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket);
#endif

/*added by mtk68119 for ioctl commands*/
typedef struct{
	uintptr_t 						SrcBuf[3];
	BOOL  						SrcIsVirtual;
	IMGRESZ_DRV_SRC_COLOR_MODE  SrcColorMode;
	__u32 						SrcWidth;
	__u32 						SrcHeight;
	__u32						SrcBufWidth;
	__u32						SrcBufHeight;
	__u32 						SrcXoff;
	__u32 						SrcYoff;
	uintptr_t						DstBuf[2];
	BOOL  						DstIsVirtual;
	IMGRESZ_DRV_DST_COLOR_MODE  DstColorMode;
	__u32 						DstWidth;
	__u32 						DstHeight;
	__u32						DstBufWidth;
	__u32						DstBufHeight;
	__u32 						DstXoff;
	__u32 						DstYoff;	
}IMGRESZ_MW_PARAM;



/// Release image resizer by ticket.
/// If you do not use image resizer any more, release it so others can use it.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.





typedef struct _IMGRESZ_LUMAKEY_T
{
	__u8 u1LumaKey;
	BOOL fgEnableLumaKey;

} IMGRESZ_LUMAKEY_T;





typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    IMGRESZ_DRV_SCALE_PRIORITY ePriority;
} IMGRESZ_DRV_SET_SCALE_PRIORITY_T;

typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    BOOL fgLock;
} IMGRESZ_DRV_SET_LOCK_T;

typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    IMGRESZ_DRV_SCALE_MODE eMode;
} IMGRESZ_DRV_SET_SCALEMODE_T;



typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    IMGRESZ_DRV_DST_BUF_INFO_T rBufInfo;
} IMGRESZ_DRV_SET_DSTBUFINFO_T;

typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    IMGRESZ_DRV_SRC_BUF_INFO_T rBufInfo;
} IMGRESZ_DRV_SET_SRCBUFINFO_T;

typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    IMGRESZ_DRV_BLD_BUF_INFO_T rBufInfo;
} IMGRESZ_DRV_SET_BLDBUFINFO_T;

typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    IMGRESZ_DRV_PARTIAL_INFO_T rBufInfo;
} IMGRESZ_DRV_SET_PARTIALBUFINFO_T;

typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    IMGRESZ_DRV_JPEG_INFO_T rJPEGInfo;
} IMGRESZ_DRV_SET_JPEGINFO_T;

typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    IMGRESZ_DRV_RM_INFO_T rRmInfo;
} IMGRESZ_DRV_SET_RMINFO_T;

typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    IMGRESZ_DRV_DO_SCALE_T rDoScale;
} IMGRESZ_DRV_SET_DOSCALE_T;

///\example imgresz_cmd.c

struct Linebuf {
unsigned long base;
unsigned long size;
};
#endif // __IMGRESZ_DRV_DRV_H
