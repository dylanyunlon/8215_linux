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

#ifndef _X_IMG_DEC_H_
#define _X_IMG_DEC_H_


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include "x_os.h"
#include "x_gfx.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "x_drv_cb.h"
/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
/* notification states */
typedef enum
{
	IMG_NFY_FILL_BUF,
	IMG_NFY_FINISHED,
	IMG_NFY_ERROR,
	IMG_NFY_STOP_DONE,
	IMG_NFY_PRS_MARKER,
	IMG_NFY_ALREADY_STOPPED
}IMG_NFY_STATE_T;

/* rotation information */
typedef enum
{
	IMG_ROTATE_NONE             = 0,    /* no rotation */
	IMG_ROTATE_CW_90            = 1,    /* clockwise  90 degrees  */
	IMG_ROTATE_CW_180           = 2,    /* clockwise 180 degrees  */
	IMG_ROTATE_CW_270           = 3,    /* clockwise 270 degrees  */
	IMG_ROTATE_NONE_WITH_FLIP   = 4,    /* no rotation, with flip */
	IMG_ROTATE_CW_90_WITH_FLIP  = 5,    /* clockwise  90 degrees, with flip */
	IMG_ROTATE_CW_180_WITH_FLIP = 6,    /* clockwise 180 degrees, with flip */
	IMG_ROTATE_CW_270_WITH_FLIP = 7     /* clockwise 270 degrees, with flip */
} IMG_ROTATE_T;

/* decode quality factor */
typedef enum
{
	IMG_QUALITY_FACTOR_NORMAL = 0,
	IMG_QUALITY_FACTOR_LOW,
	IMG_QUALITY_FACTOR_FAST
} IMG_QUALITY_FACTOR_T;

typedef enum
{
	IMG_JPG_DECODE_NORMAL = 0,
	IMG_JPG_DECODE_SAME_PIC,
} IMG_JPG_DECODE_FLAG_E;


/* data passed with IMG_SET_TYPE_BUF_FILLED */
typedef struct
{
	__u32          u4WrSize;           /* Img Buffer Write Size */
	bool            fgEOI;              /* Indicates whether encounter the end of the file */
} IMG_BUF_FILLED_T;

typedef struct 
{
	uintptr_t         u4RdAddr;
	uintptr_t         u4FileRdOfst;
} IMG_NFY_PARSE_INFO_T;

typedef struct
{
	IMG_NFY_PARSE_INFO_T rParseInfo;
	__u32         u4WrSize;
	IMG_NFY_STATE_T eState;
	HANDLE         hInst;
	int           transfer_num_parse; 
	bool          thread_parse;
} IMG_NFY_DATA_T;

typedef struct _IMG_BUF_MAPPING_INFO_T
{
	void *pvNAVirtual;
	void *pvCallVirtual;
} IMG_BUF_MAPPING_INFO_T;

typedef struct _IMG_BUF_INFO_T
{
	void   *pvVirtual;
	__u32 u4Size;
	__u64 u4PhyAdr;
	IMG_BUF_MAPPING_INFO_T rMapping;
	bool fgSelfAlloc;
} IMG_BUF_INFO_T;

typedef struct _IMG_BUF_T
{
	void  *pvVirtual;
	__u64 u4PhyAdr;
	__u32 u4Size;
} IMG_BUF_T;

typedef enum _IMG_TYPE_T
{
	IMG_TYPE_NONE = -1,
	IMG_TYPE_JPG  = 0,
	IMG_TYPE_PNG,
	IMG_TYPE_GIF,
	IMG_TYPE_BMP,
	IMG_TYPE_MAX
} IMG_TYPE_T;

/* callback function */
typedef void (*IMG_NFY_FCT_T)
(
	IMG_TYPE_T      eImgType,
	void            *pv_tag,
	void            *pv_data,           /* data passed with this notification */
	IMG_NFY_STATE_T e_state
);           /* notification state */

/* data passed with IMG_SET_TYPE_FRM_START */
typedef struct _IMG_DEC_PARAM_T
{
	__u32                  u4FrameIdx;       /* frame index */
	__u32                  u4srcx;           /* x offset in the source image in pixels */
	__u32                  u4srcy;           /* y offset in the source image in pixels */
	__u32                  u4srcwidth;       /* width to be decoded in pixels */
	__u32                  u4srcheight;      /* height to be decoded in pixels */
	__u32                  u4dstx;           /* x offset in the destination in pixels */
	__u32                  u4dsty;           /* y offset in the destination in pixels */
	__u32                  u4dstwidth;       /* expected output width in pixels */
	__u32                  u4dstheight;      /* expected output height in pixels */
	__u32                  u4dstpitch;       /* pitch of the destination image */

	__u32                  u4imgsize;        /* image size          */
	IMG_BUF_T               rdstbuf;            /* destination starting address */
	IMG_BUF_T               rdstYbuf;  // YUVBLK
	IMG_BUF_T               rdstCbuf;   //yuvblk
	IMG_BUF_T               rdst2buf;
	IMG_BUF_T               rhdrbuf;         /* Image file header info used in decode, for different image type, it is different */
	IMG_BUF_T               rnfybuf;         /* Image file header info used in decode, for different image type, it is different */
	GFX_COLORMODE_T         edstcm;           /* destination color mode */
	IMG_QUALITY_FACTOR_T    equality;          /* quality factor */ //[20081014] BDP00013841
	IMG_JPG_DECODE_FLAG_E   ejpgflag;         /* IMG_JPG_DECODE_FLAG_E */

	HANDLE                   hInst;
	bool                    bSwScale;          /* SW scale */
	bool                    bcompressed;       /* PNG WT*/ 
    
} IMG_DEC_PARAM_T;

typedef struct  _IMG_DEC_PREPARE_T
{
	__u32          u4ImgSize;
	IMG_BUF_T       rSrcBuf;
	IMG_BUF_T       rHdrBuf;
	//mtk94038 modify NfyBuf malloc in driver
#ifdef __linux__
	IMG_BUF_T		rNfyBuf;
#endif
	HANDLE_T        hTrsDataEvt;
	HANDLE_T        hDecFinishEvt;
} IMG_DEC_PREPARE_T;

typedef struct _IMG_DECODE_T
{
	__u64 u4RdAddr;
	__u64 u4WrAddr;
	__u32 u4FileRdOfst;
	bool   fgEOI;
} IMG_DECODE_T;

typedef enum
{
	IMG_DEC_FINISH_OK,
	IMG_DEC_FINISH_STOP,
	IMG_DEC_FINISH_ERR,
	IMG_DEC_FINISH_NOT
}IMG_DEC_FINISH_STATE_T;


/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* _X_IMG_DEC_H_ */

