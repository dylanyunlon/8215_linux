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
#ifndef __ARM2__
#include "x_common.h"
#include <linux/types.h>
#include <linux/fs.h>
#else
#include "x_types.h"
#endif

#include "windev.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

#define IMGRESZ_ALPHA_BLENDING 1

#define USE_IMGRESZ_DRV_DLL    0
/*added by mtk68119 for ioctl commands*/
#define FILE_DEVICE_IMGRESZ 0x0000BA21
#define SET_PARAM CTL_CODE(FILE_DEVICE_IMGRESZ, 0x0101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SCALE_FIRE CTL_CODE(FILE_DEVICE_IMGRESZ, 0x0103, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define SCALE_STOP CTL_CODE(FILE_DEVICE_IMGRESZ, 0x0105, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IMGRESZ_GET_TICKET CTL_CODE(FILE_DEVICE_IMGRESZ, 0x0107, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IMGRESZ_SET_PRORITY CTL_CODE(FILE_DEVICE_IMGRESZ, 0x0109, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IMGRESZ_SET_SCALEMODE CTL_CODE(FILE_DEVICE_IMGRESZ, 0x010B, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IMGRESZ_SET_LOCK CTL_CODE(FILE_DEVICE_IMGRESZ, 0x010C, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IMGRESZ_SET_SRCBUF CTL_CODE(FILE_DEVICE_IMGRESZ, 0x010D, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IMGRESZ_SET_DSTBUF CTL_CODE(FILE_DEVICE_IMGRESZ, 0x010E, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IMGRESZ_SET_RMINFO CTL_CODE(FILE_DEVICE_IMGRESZ, 0x010F, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IMGRESZ_SET_PARTICALBUF CTL_CODE(FILE_DEVICE_IMGRESZ, 0x0110, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IMGRESZ_DO_SCALE CTL_CODE(FILE_DEVICE_IMGRESZ, 0x0111, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IMGRESZ_STOP_SCALE CTL_CODE(FILE_DEVICE_IMGRESZ, 0x0112, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IMGRESZ_RELEASE_TICKET CTL_CODE(FILE_DEVICE_IMGRESZ, 0x0113, METHOD_BUFFERED, FILE_ANY_ACCESS)

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
    bool                        fgInterlaced;   ///< Interlaced picture.
    bool                        fgTopField;     ///< Interlaced picture top field exist. (For interlaced video only)
    bool                        fgBottomField;  ///< Interlaced picture bottom field exist. (For interlaced video only)
    IMGRESZ_DRV_COMPONENT_FACTOR_T  rCompFactor;    ///< Used when eSrcColorMode == IMGRESZ_INPUT_COL_MD_JPG_DEF.
    uintptr_t                      u4ColorPalletSa; ///< Color Pallet.
    bool                        fgWTEnable;     ///< Wavelet transform compression.
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
    bool                        fgWTEnable;     ///< Wavelet transform compression.    
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
    bool                        fgBlendBeforeResz; /// < True: Blending before resize. False: Blending after resize.
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
    bool                        fgFirstRow;     ///< The First row.
    bool                        fgLastRow;      ///< The Last row.
} IMGRESZ_DRV_PARTIAL_INFO_T;


/// Image Resizer Jpg Info
typedef struct _IMGRESZ_DRV_JPEG_INFO_T
{
    bool                        fgExistY;       ///< Exist Y component
    bool                        fgExistCb;      ///< Exist Cb component
    bool                        fgExistCr;      ///< Exist Cr component
    bool                        fgPreload;      ///< Progressive preload mode
    __u32                      u4HwId;         ///< Indicate image resizer hardware ID for picture mode
} IMGRESZ_DRV_JPEG_INFO_T;

/// Imgage Resizer RM video related info
typedef struct _IMGRESZ_DRV_RM_INFO_T
{
    bool                        fgRPRMode;     ///< PRP mode or not for rm video
    bool                        fgRacingMode;  ///< Racing mode or not
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
	bool  						SrcIsVirtual;
	IMGRESZ_DRV_SRC_COLOR_MODE  SrcColorMode;
	__u32 						SrcWidth;
	__u32 						SrcHeight;
	__u32						SrcBufWidth;
	__u32						SrcBufHeight;
	__u32 						SrcXoff;
	__u32 						SrcYoff;
	uintptr_t						DstBuf[2];
	bool  						DstIsVirtual;
	IMGRESZ_DRV_DST_COLOR_MODE  DstColorMode;
	__u32 						DstWidth;
	__u32 						DstHeight;
	__u32						DstBufWidth;
	__u32						DstBufHeight;
	__u32 						DstXoff;
	__u32 						DstYoff;	
}IMGRESZ_MW_PARAM;


extern bool ImgreszSetParam(struct file * filp,
	IMGRESZ_MW_PARAM * pParam
);

extern bool ImgreszScaleFire(struct file * filp
);

extern bool ImgreszStopScale(struct file * filp
);
/// Release image resizer by ticket.
/// If you do not use image resizer any more, release it so others can use it.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_ReleaseTicket(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket
);


/// Set the priority that using image resizer.
/// Image resizer can be used by multi-instances, so higher priority is more likely to be served.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_SetPriority(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket,
    IMGRESZ_DRV_SCALE_PRIORITY  eScalePriority
);


/// Set lock or unlock image resizer.
/// If you lock image resizer, it can serve only you.
/// Lock can promise high performance, but also prevent other to using it.
/// It suggests that lock should be used only for some specific case.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_SetLock(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket,
    bool fgLock
);


/// Get image resizer state.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_GetState(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket,
    IMGRESZ_DRV_SCALE_STATE_T *pImgReszState
);


/// Set image resizer scaling mode.
/// Set scaling mode detail in IMGRESZ_DRV_SCALE_MODE.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_SetScaleMode(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket,
    IMGRESZ_DRV_SCALE_MODE eScaleMode
);

///Set mmu table
extern __s32 i4ImgResz_Drv_SetMMUTable(
	IMGRESZ_DRV_TICKET_T *pImgReszTicket,
	uintptr_t u4tableaddr
);

/// Set image resizer luma key.
/// Set luma key information u1LumaKey and fgEnableLumaKey.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.

__s32 i4ImgResz_Drv_SetLumaKey(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket,
    __u8 u1LumaKey, 
    bool fgEnableLumaKey
);
    

/// Set image resizer source buffer information.
/// Set source buffer information detail in IMGRESZ_DRV_SRC_BUF_INFO_T.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_SetSrcBufInfo(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket,
    IMGRESZ_DRV_SRC_BUF_INFO_T *pSrcBufInfo
);


/// Set image resizer destination buffer information.
/// Set destination buffer information detail in IMGRESZ_DRV_DST_BUF_INFO_T.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_SetDstBufInfo(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket,
    IMGRESZ_DRV_DST_BUF_INFO_T *pDstBufInfo
);


/// Set image resizer blending buffer information.
/// Set blending buffer information detail in IMGRESZ_DRV_BLD_BUF_INFO_T.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_SetBldBufInfo(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket,
    IMGRESZ_DRV_BLD_BUF_INFO_T *pBldBufInfo
);


/// Set image resizer partial buffer information.
/// If scaling mode is partial mode, set the partial buffer info.
/// Set partial buffer information detail in IMGRESZ_DRV_PARTIAL_INFO_T.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_SetPartialBufInfo(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket,
    IMGRESZ_DRV_PARTIAL_INFO_T *pPartialBufInfo
);


/// Set image resizer jpg information.
/// For jpg file use
/// Set jpg information detail in IMGRESZ_DRV_JPEG_INFO_T.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_SetJpegInfo(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket,
    IMGRESZ_DRV_JPEG_INFO_T *prJpegInfo
);

/// Set image resizer rm video information.
/// For rm video use
/// Set rm video scale information detail in IMGRESZ_DRV_RM_INFO_T.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.

extern __s32 i4ImgResz_Drv_SetRmInfo(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket, 
    IMGRESZ_DRV_RM_INFO_T *prRmInfo
);


/// Trigget image resizer to do scale.
/// Be sure all info is set before do scale.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_DoScale(
    IMGRESZ_DRV_TICKET_T *pImgReszTicket,
    IMGRESZ_DRV_DO_SCALE_T *pDoScale
);


/// To stop scale while doing scale.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_StopScale(IMGRESZ_DRV_TICKET_T *pImgReszTicket);


/// Register a callback function which will be called when scale finish.
/// You can do send event in this callback function to notify the task that trigger doing scale.
/// Do register callback function before triggering to do scale.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_RegFinishNotifyCallback(IMGRESZ_DRV_TICKET_T *pImgReszTicket,IMGRESZ_DRV_NOTIFY_CB_REG_T *prNotifyCallbackReg);


/// Unregister the callback function that is registered.
/// Before release a ticket, remember to do unregister.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_UnregFinishNotifyCallback(IMGRESZ_DRV_TICKET_T *pImgReszTicket,IMGRESZ_DRV_NOTIFY_CB_REG_T *prNotifyCallbackReg);


/// Init image resizer driver.
/// Call this function by system initialization function when system boot up.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_Init(void);
extern __s32 TS_DirectScale(__u32 u4HwId, bool fg8Tap, VOID *prSrcImgInfo, VOID *prDestImgInfo);

extern __s32 YUV420BlkToARGB8888(__u32 u4HwId, VOID  * prSrcImgInfo,
                     VOID  *prDestImgInfo);
extern __s32 TS_DirectScale2(__u32 u4HwId, VOID  * prSrcImgInfo,
                      VOID  *prDestImgInfo);
extern void vYUV420_Block_TO_ARGB8888(void *lpInBuffer);

extern void vYUV420_Block_TO_NV12(void * lpInBuffer1);

/// Uninit image resizer driver.
/// Call this function by system uninitialization function when system shut down.
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.
extern __s32 i4ImgResz_Drv_Uninit(void);

/// Uninit image resizer driver.
/// Call this function by system uninitialization function when system shut down,for linux use
/// \return If return value < 0, it's failed. Please reference drv_imgresz_errcode.h.

extern __s32 i4ImgResz_DrvUninit(__u32 u4Case);


typedef struct _IMGRESZ_LUMAKEY_T
{
	__u8 u1LumaKey;
	bool fgEnableLumaKey;

} IMGRESZ_LUMAKEY_T;


#define IMGRESZ_DEV_NAME             L"GRZ1:"

#define IOCTL_GRZ_GET_TICKET            \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GRZ_RELEASE_TICKET            \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS)  
    
#define IOCTL_GRZ_GET_STATE            \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)  

#define IOCTL_GRZ_SET_PRIORITY            \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS) 

#define IOCTL_GRZ_SET_LOCK          \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x904, METHOD_BUFFERED, FILE_ANY_ACCESS)
   
#define IOCTL_GRZ_SET_SCALEMODE            \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x905, METHOD_BUFFERED, FILE_ANY_ACCESS)  

#define IOCTL_GRZ_SET_LUMAKEY            \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x906, METHOD_BUFFERED, FILE_ANY_ACCESS)  
    
#define IOCTL_GRZ_SET_SRCBUFINFO            \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x907, METHOD_BUFFERED, FILE_ANY_ACCESS)  

#define IOCTL_GRZ_SET_DSTBUFINFO            \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x908, METHOD_BUFFERED, FILE_ANY_ACCESS)  

#define IOCTL_GRZ_SET_BLDBUFINFO            \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x909, METHOD_BUFFERED, FILE_ANY_ACCESS)  

#define IOCTL_GRZ_SET_PARTIALBUFINFO            \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x910, METHOD_BUFFERED, FILE_ANY_ACCESS)
    
#define IOCTL_GRZ_SET_JPEGINFO	\
		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x911, METHOD_BUFFERED, FILE_ANY_ACCESS)
		
#define IOCTL_GRZ_SET_RMINFO	\
		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x912, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GRZ_DOSCALE	\
		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x913, METHOD_BUFFERED, FILE_ANY_ACCESS)
		
#define IOCTL_GRZ_STOPSCALE	\
		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x914, METHOD_BUFFERED, FILE_ANY_ACCESS)
	
#define IOCTL_GRZ_REGFINISHNOTIFY	\
		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x915, METHOD_BUFFERED, FILE_ANY_ACCESS)
		
#define IOCTL_GRZ_UNREGFINISHNOTIFY	\
		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x916, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    IMGRESZ_DRV_SCALE_PRIORITY ePriority;
} IMGRESZ_DRV_SET_SCALE_PRIORITY_T;

typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    bool fgLock;
} IMGRESZ_DRV_SET_LOCK_T;

typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    IMGRESZ_DRV_SCALE_MODE eMode;
} IMGRESZ_DRV_SET_SCALEMODE_T;

typedef struct
{
    IMGRESZ_DRV_TICKET_T rImgReszTicket;
    __u8 u1LumaKey;
    bool  fgEnableLumaKey;
} IMGRESZ_DRV_SET_LUMAKEY_T;

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

#define VIRT_TO_BUS(vaddr)		(((uintptr_t)__pa(vaddr)) - 0x100000000L)
#define BUS_TO_VIRT(baddr)		__va(((uintptr_t)baddr + 0x100000000L))

struct Linebuf {
unsigned long base;
unsigned long size;
};
#endif // __IMGRESZ_DRV_DRV_H
