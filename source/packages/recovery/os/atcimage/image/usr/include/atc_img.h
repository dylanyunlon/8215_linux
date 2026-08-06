/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008 MediaTek Inc.
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
*
* Filename:
* ---------
* file ImgCodec.h
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*  
*
* Author:
* -------
*   
*
*------------------------------------------------------------------------------
* $Revision: #1 $
* $Modtime:$
* $Log:$
*
*******************************************************************************/

#ifndef _ATC_IMG_H_
#define _ATC_IMG_H_

typedef void *HIMGDECINST;

typedef enum _IMGCODEC_ID_T
{
    IMGCODEC_ID_JPG,
    IMGCODEC_ID_PNG,
    IMGCODEC_ID_BMP,
    IMGCODEC_ID_GIF,
    IMGCODEC_ID_MAX
} IMGCODEC_ID_T;

typedef enum _IMGCODEC_PIXEL_FORMAT_T
{
    //Image Resizer's output pixel format
    IMG_FORMAT_RGB_565,
    IMG_FORMAT_ARGB_1555,
    IMG_FORMAT_ARGB_4444,
    IMG_FORMAT_ARGB_8888,
} IMGCODEC_PIXEL_FORMAT_T;

typedef struct _IMGCODEC_CFG_T
{
    unsigned char fgUseHWAlloc;
} IMGCODEC_CFG_T;

typedef struct _IMGSAMPLEINFO
{
    unsigned short         u2ImgWidth;
    unsigned short          u2ImgHeight;
    unsigned short          Xdensity;
    unsigned short          Ydensity;
    unsigned char           u1ThumbW;
    unsigned char           u1ThumbH;
    unsigned short          u2PerPixel;
    unsigned short          u2ThumbSz;
    /* jpeg thumbnail mode */
    unsigned char           *pu1JpegThumbnail;    /* point to JPEG data in APP0 stream */
    unsigned long          *pu4Palette;
} IMGCODEC_SAMPLE_INFO_T;

typedef struct _IMGCODEC_INST_PARAM_T
{
    unsigned long       u4SrcXOfst;
    unsigned long       u4SrcYOfst;
    unsigned long       u4SrcWidth;
    unsigned long       u4SrcHeight;
    unsigned long       u4DstXOfst;
    unsigned long       u4DstYOfst;
    unsigned long       u4DstWidth;
    unsigned long       u4DstHeight;
    unsigned long       u4Bpp;
    IMGCODEC_PIXEL_FORMAT_T eDstPixelFormat;
	//QIODevice    *pIStream;
	void*         pvSrcBuf;
	unsigned long        u4ImgSz;
	int           u4frmcount; //zt add
} IMGCODEC_INST_PARAM_T;

#define IMGCODEC_RTN_OK                 0
#define IMGCODEC_RTN_FAIL               1
#define IMGCODEC_RTN_ARG_ERR            2
#define IMGCODEC_RTN_NOT_SUPPORT        3
#define IMGCODEC_RTN_HW_DRV_LIMIT       4
#define IMGCODEC_RTN_IMG_CORRUPTED      5

#ifdef __cplusplus
extern "C" {
#endif

//MP3 Audio Decoder API
unsigned long      ImgDec_Init(void);
unsigned long      ImgDec_Deinit(void);

HIMGDECINST ImgDec_CreateInstance(IMGCODEC_ID_T eCodecID);
unsigned long      ImgDec_Release(HIMGDECINST hInst);

unsigned long      ImgDec_SetConfig(IMGCODEC_CFG_T *prcfg);
unsigned long      ImgDec_GetConfig(IMGCODEC_CFG_T *prcfg);

unsigned long      ImgDec_SetParam(HIMGDECINST hInst, IMGCODEC_INST_PARAM_T *prParam);
unsigned long      ImgDec_GetParam(HIMGDECINST hInst, IMGCODEC_INST_PARAM_T *prParam);

unsigned long      ImgDec_SetInputBuf(HIMGDECINST hInst, void *pvBuf, unsigned long u4BufSz);
void       *ImgDec_GetInputBuf(HIMGDECINST hInst, unsigned long *pu4BufSz);

unsigned long      ImgDec_SetInputDataLength(HIMGDECINST hInst, unsigned long u4DataLength);
unsigned long      ImgDec_GetInputDataLength(HIMGDECINST hInst);

unsigned long      ImgDec_SetOutputBuf(HIMGDECINST hInst, void *pvBuf, unsigned long u4BufSz);
void       *ImgDec_GetOutputBuf(HIMGDECINST hInst, unsigned long *pu4BufSz);
unsigned long   	ImgDec_GetOutputBufVA(HIMGDECINST hInst, void *pvBufVir,unsigned long *pu4BufSz);
unsigned long 		ImgDec_GetOutputBufPA(HIMGDECINST hInst, unsigned long *pu4BufSz);

unsigned long      ImgDec_GetOutputDataLength(HIMGDECINST hInst);
unsigned long      ImgDec_Start(HIMGDECINST hInst);
unsigned long      ImgDec_Stop(HIMGDECINST hInst);
unsigned long      ImgDec_GetImgInfo(HIMGDECINST hInst,unsigned short *u4Width,unsigned short *u4Height);
void        *ImgDec_GetNptcBuf(HIMGDECINST hInst, unsigned long *prBufAddr,unsigned long *pu4BufSz);

#ifdef __cplusplus
}
#endif

#endif //_ATC_IMG_H_
