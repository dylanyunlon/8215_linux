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
#ifndef _VDP_HAL_H_
#define _VDP_HAL_H_

#ifndef __ARM2__
#include <linux/kernel.h>
#endif
#include "x_lint.h"
LINT_EXT_HEADER_BEGIN
#include "chip_ver.h"
#ifndef __ARM2__
#include <media/atc/drv_av_d.h>
#else
#include "drv_av_d.h"
#endif
LINT_EXT_HEADER_END
#include "vdp_hw.h"


typedef struct _VDP_REGION_T {
	__u32			X;
	__u32			Y;
	__u32			Width;
	__u32			Height;
} VDP_REGION_T;

typedef enum {
	VDP_OUT_480P = 0,
	VDP_OUT_576P,
	VDP_OUT_480_800,
	VDP_OUT_600_800,
	VDP_OUT_600_1024,
	VDP_OUT_720_1280,
	VDP_OUT_800_1280,
} VDP_OUT_MODE;

typedef struct {
	PMX_RESOLUTION_MODE_T u4Mode;
	__u32                u4PosX;
	__u32                u4PosY;
	__u32                u4PosEY;
} VDP_ACT_START_POS;

typedef union {
	PMX_HAL_DISP_MAIN_UNION_T *pDispMain;
	PMX_HAL_DISP_AUX_UNION_T  *pDispAux;
} PMX_DISP_REG_UNION_PTR_T;

#define VDO_VSCALE_STEP    (128)
#define VDO_VSCALE_SHIFT   (7)
#define FMT_HSCALE_STEP    (256)
#define FMT_HSCALE_SHIFT   (8)

#define VSYNC_PER_FRAME    (3000)
#define VSYNC_PER_FIELD    (1500)

#define VDP_DI_FRAME_MODE  (0)  /* default is frame mode for progressive*/
#define VDP_DI_FIELD_MODE  (1)
#define VDP_DI_HD_MODE     (3)
#define VDP_DI_MA4F_MODE   (4)
#define VDP_DI_PD_MODE     (5)
#define VDP_DI_MODE_MAX    (5)

#define IMG_RESZ_BUFF_SIZE  3

#define VDP_BUF_NUM       (4)
#define VDP_DUMP_IN       (0)
#define VDP_DUMP_OUT      (1)
#define VDP_DUMP_IO       (2)


typedef struct {
	__u32 WYMotion;
	__u32 WXComb;
	__u32 XZMotion;
	__u32 XYComb;
} VDP_MOT_COMB;


extern void vVdpHalReset(__u8 ucVdpId);
extern void vVdpHalInit(__u8 ucVdpId, bool fgHwReset);
extern void vVdpHalSetSrcSize(__u8 ucVdpId, __u32 Width, __u32 Height);
extern void vVdpHalSetSrcRegion(__u8 ucVdpId, __u32 X, __u32 Y, __u32 Width, __u32 Height);
extern void vVdpHalSetOutRegion(__u8 ucVdpId, __u32 X, __u32 Y, __u32 Width, __u32 Height);
extern void vVdpHalSetMode(__u8 ucVdpId, __u32 u4Mode);
extern void vVdpHalSetFifo(__u8 ucVdpId);
extern void vVdpHalSetYuv422(__u8 ucVdpId, bool fgYUV422);
extern void vVdpHalSetScanLine(__u8 ucVdpId, bool fgScLineEnable);
extern void vVdpHalSetHFilterMode(__u8 ucVdpId, bool fgFIRMode);
extern void vVdpHalSetYBufPtr(__u8 ucVdpId, __u32 u4AddrY, __u32 u4AddrC);
extern void vVdpHalSetDeintWXYZ(__u8 ucVdpId, const __u32 *pu4AddrY, const __u32 *pu4AddrC);
extern void vVdpHalSetFieldInfo(__u8 ucVdpId, bool fgTopFldFirst, bool fgFirstFld);
extern void vVdpHalSetDeintMode(__u8 ucVdpId, __u8 ucDiMode);
extern void vVdpHalSetPdDetect(__u8 ucVdpId, bool fgEnable);
extern void vVdpHalSetMergeZ(__u32 u4VDP, bool fgMergeZ);
extern void vVdpHalDisMergeZ(__u32 u4VDP);
extern void vVdpHalResume(__u8 ucVdpId);
extern void vVdpHalIsr(__u8 ucVdpId);
extern void vVdpHalGetMotionComb(__u32 u4VdpId, __u32 *prXYComb
	, __u32 *prWXComb, __u32 *prWYMotion, __u32 *prXZMotion);
extern void vVdpHalGetFieldInfo(__u32 u4VdpId, __u32 *prFldInfo);
#ifndef __ARM2__
typedef struct _VDP_BUF_IN_INFO_{
	__u32 u4YAddr[VDP_BUF_NUM];
	__u32 u4CAddr[VDP_BUF_NUM];
	__u32 u4Width;
	__u32 u4Height;
	__u32 u4YSize;
	__u32 u4CSize;
	__u32 flag;
} VDP_BUF_IN_INFO;

extern VDP_BUF_IN_INFO vdpInBuf;
extern VDP_BUF_IN_INFO vdpOutBuf;
extern void vVdpDumpTest(const unsigned char path[], const unsigned int dumpNum, const unsigned int io);
#endif
#endif  /* _VDP_HAL_H_*/



