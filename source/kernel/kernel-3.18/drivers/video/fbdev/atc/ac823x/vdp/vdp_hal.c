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

/*****************************************************************************
*  Video Plane: Interface
*****************************************************************************/

#ifndef _VDP_HAL_C_
#define _VDP_HAL_C_

#ifndef __ARM2__
#include <linux/module.h>
#include <media/atc/ac823x/pmx_hal.h>
#include <media/atc/display_inc.h>
#include "windows.h"
#include "x_debug.h"
#else
#include "x_types.h"
#include "assert.h"
#include "pmx_hal.h"
#include "display_inc.h"
#endif
#include "vdp_hw.h"
#include "vdp_hal.h"
#include "log.h"
#include "pmx_hw.h"
#include "x_lint.h"
#include "x_hal_ic.h"
#include "drv_common.h"
#include "drv_config.h"
#include "chip_ver.h"
#include "x_bim.h"
#include "x_os.h"
#include "x_assert.h"

/******************************************************************************
* Local variable
******************************************************************************/
#define DEINTERLACE_DEFAULT_START_ADDRESS       0x50
#define DEINTERLACE_DEFAULT_SIZE                28

__u32 _u4Tm070ddhg;

__u32 deinterlace_default[] = { /* start from 0x70042150*/
        0x00010000, 0x000007ff, 0x10000000, 0x10000000, 
        0x00000000, 0x80018700, 0x00000c0f, 0x05110000, 
        0x00001000, 0x000001f1, 0x00010037, 0x08000000, 
        0x0006ba80, 0x00091a00, 0x4a0186a0, 0x1b011940, 
        0x0000c350, 0x00012003, 0x3202a02a, 0x1600c002, 
        0x6401c01c, 0x32051004, 0x01436004, 0x02002604, 
        0x0332323c, 0x00320814, 0x00320432, 0x00005537, 
};

HAL_VDO_UNION_T _rVdp1SwReg;
HAL_VDO_UNION_T _rVdp2SwReg;
__u8 _rVdp1RegMode[VDP_HAL_VDO_REG_NUM];
__u8 _rVdp2RegMode[VDP_HAL_VDO_REG_NUM];


HAL_VDO_APQ_UNION_T _rVdp1ApqSwReg;
HAL_VDO_APQ_UNION_T _rVdp2ApqSwReg;
__u8 _rVdp1ApqRegMode[HAL_VDO_APQ_REG_NUM];
__u8 _rVdp2ApqRegMode[HAL_VDO_APQ_REG_NUM];

static VDP_REGION_T _rVdp1SrcRegion = { 0, 0, 2, 1 };
static VDP_REGION_T _rVdp2SrcRegion = { 0, 0, 2, 1 };
static VDP_REGION_T _rVdp1OutRegion = { 0, 0, 2, 1 };
static VDP_REGION_T _rVdp2OutRegion = { 0, 0, 2, 1 };

static bool _fgVdp1ResetInVSync = FALSE;
static bool _fgVdp2ResetInVSync = FALSE;

static VDP_ACT_START_POS _tVdp1ActPos_TM[] = { /*for TM070ddhg*/
        {RES_480P,           0x068, 0x035, 0x035},/*verified*/
        {RES_576P,           0x074, 0x030, 0x030},
        {RES_480P_800,       0x066, 0x02C, 0x02C},
        {RES_600P_800,       0x064, 0x02C, 0x02C},
        {RES_600P_1024,      0x09d, 0x024, 0x024},/*verified*/
        {RES_768P_1024,      0x063, 0x01A, 0x01A},
        {RES_720P_1280,      0x050, 0x013, 0x013},
        {RES_800P_1280,      0x090, 0x01C, 0x01C},
};

static VDP_ACT_START_POS _tVdp1ActPos[] = {
        {RES_480P,           0x068, 0x02C, 0x02C},
        {RES_576P,           0x074, 0x030, 0x030},
        {RES_480P_800,       0x066, 0x02C, 0x02C},
        {RES_600P_800,       0x064, 0x02C, 0x02C},
        {RES_600P_1024,      0x105, 0x018, 0x018},
        {RES_768P_1024,      0x063, 0x01A, 0x01A},
        {RES_720P_1280,      0x050, 0x013, 0x013},
        {RES_800P_1280,      0x090, 0x01C, 0x01C},
};

static VDP_ACT_START_POS _tVdp2ActPos[] = {
        {RES_480P,          0x06D, 0x02C, 0x02C},
        {RES_576P,          0x079, 0x032, 0x032},
};

void vVdpHalReset(__u8 ucVdpId)
{
        if (ucVdpId == VDP_1) {
                _fgVdp1ResetInVSync = TRUE;
        } else {
                _fgVdp2ResetInVSync = TRUE;
        }
}

void vVdpHalInit(__u8 ucVdpId, bool fgHwReset)
{
        volatile __u32 *prVdpDiRegs = NULL;
        volatile HAL_VDO_UNION_T *prVdpHwRegs = NULL;
        __u32 u4Idx;

        GET_VDP_HW_PTR(ucVdpId, prVdpHwRegs);

        #if 0
        if (fgHwReset) {
                prVdpDiRegs = (ucVdpId == VDP_1) ? (__u32 *)(vdof_reg + DEINTERLACE_DEFAULT_START_ADDRESS) :
                              (__u32 *)(vdor_reg + DEINTERLACE_DEFAULT_START_ADDRESS);

                for (u4Idx = 0; u4Idx < DEINTERLACE_DEFAULT_SIZE; u4Idx++) {
                        *prVdpDiRegs++ = deinterlace_default[u4Idx];
                }

                prVdpHwRegs->rField.u4VDORST = 0xFF;
                prVdpHwRegs->rField.u4VDORST = 0;
        }
        #endif
        
        if (ucVdpId == VDP_1) {
                for (u4Idx = 0; u4Idx < VDP_HAL_VDO_REG_NUM; u4Idx++) {
                        _rVdp1SwReg.au4Reg[u4Idx] = prVdpHwRegs->au4Reg[u4Idx];
                }
        } else {
                for (u4Idx = 0; u4Idx < VDP_HAL_VDO_REG_NUM; u4Idx++) {
                        _rVdp2SwReg.au4Reg[u4Idx] = prVdpHwRegs->au4Reg[u4Idx];
                }
        }
}
EXPORT_SYMBOL(vVdpHalInit);

void vVdpHalSetSrcSize(__u8 ucVdpId, __u32 Width, __u32 Height)
{
        HAL_VDO_UNION_T *prVdpSwReg;
        __u8 *prVdpRegMode;
        __u8 *prPmxDispRegMode;
        PMX_DISP_REG_UNION_PTR_T prPmxDispReg;

        GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);
        GET_PMX_DISP_PTR(ucVdpId, prPmxDispReg, prPmxDispRegMode);

        prVdpSwReg->rField.u4Hblock = ((Width + 7) >> 3);
        prVdpSwReg->rField.u4Pic_Height = Height;

        if (Width > 720) {
                prVdpSwReg->rField.u4DW_NEED_HD = ((Width + 3) >> 2);
                prVdpSwReg->rField.fgHD_EN = 1;
        } else {
                prVdpSwReg->rField.u4DW_Need = ((Width + 3) >> 2);
                prVdpSwReg->rField.fgHD_EN = 0;
        }

        prPmxDispReg.pDispMain->rField.u2PXLLEN = Width;
        VDO_LOG(VDO_LOG_LVL_REGRW, "vVdpHalSetSrcSize: ucVdpId %d, 0x10 %x\r\n"
                , (int)ucVdpId, (unsigned int)prVdpSwReg->au4Reg[(0x10 / 4)]);
        
        prVdpRegMode[(0x10 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prVdpRegMode[(0xE0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0x9C / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalSetSrcRegion(__u8 ucVdpId, __u32 X, __u32 Y, __u32 Width, __u32 Height)
{
        if (ucVdpId == VDP_1) {
                _rVdp1SrcRegion.X = X;
                _rVdp1SrcRegion.Y = Y;
                _rVdp1SrcRegion.Width = Width;
                _rVdp1SrcRegion.Height = Height;
        }

        if (ucVdpId == VDP_2) {
                _rVdp2SrcRegion.X = X;
                _rVdp2SrcRegion.Y = Y;
                _rVdp2SrcRegion.Width = Width;
                _rVdp2SrcRegion.Height = Height;
        }
}

void vVdpHalSetOutRegion(__u8 ucVdpId, __u32 X, __u32 Y, __u32 Width, __u32 Height)
{
        if (ucVdpId == VDP_1) {
                _rVdp1OutRegion.X = X;
                _rVdp1OutRegion.Y = Y;
                _rVdp1OutRegion.Width = Width;
                _rVdp1OutRegion.Height = Height;
        }

        if (ucVdpId == VDP_2) {
                _rVdp2OutRegion.X = X;
                _rVdp2OutRegion.Y = Y;
                _rVdp2OutRegion.Width = Width;
                _rVdp2OutRegion.Height = Height;
        }
}

void vVdpHalSetMode(__u8 ucVdpId, __u32 u4Mode)
{
        HAL_VDO_UNION_T *prVdpSwReg;
        __u8 *prVdpRegMode;
        PMX_DISP_REG_UNION_PTR_T prPmxDispReg;
        __u8 *prPmxDispRegMode;
        VDP_REGION_T *prSrcRegion;
        VDP_REGION_T *prOutRegion;
        __u32 u4Cnt, u4PosX = 0, u4PosY = 0, u4PosEY = 0;
        __u32 u4VdpOutWidth, u4VdpOutHeight, u4HFactor;
        unsigned int SrcWidth, OutWidth;
        unsigned int u4HFactor_inverse = 0, adj_hact_vdo = 0, adj_hact_hd = 0;
        
        GET_VDP_PTR_REGION(ucVdpId, prVdpSwReg, prVdpRegMode, prSrcRegion, prOutRegion);
        GET_PMX_DISP_PTR(ucVdpId, prPmxDispReg, prPmxDispRegMode);


        if (ucVdpId == VDP_1) {
                for (u4Cnt = 0; u4Cnt < sizeof(_tVdp1ActPos) / sizeof(_tVdp1ActPos[0]); u4Cnt++) {
                        if (_tVdp1ActPos[u4Cnt].u4Mode == u4Mode) {
                                u4PosX = _tVdp1ActPos[u4Cnt].u4PosX;
                                u4PosY = _tVdp1ActPos[u4Cnt].u4PosY;
                                u4PosEY = _tVdp1ActPos[u4Cnt].u4PosEY;
                                break;
                        }
                }
        } else {
                for (u4Cnt = 0; u4Cnt < sizeof(_tVdp2ActPos) / sizeof(_tVdp2ActPos[0]); u4Cnt++) {
                        if (_tVdp2ActPos[u4Cnt].u4Mode == u4Mode) {
                                u4PosX = _tVdp2ActPos[u4Cnt].u4PosX;
                                u4PosY = _tVdp2ActPos[u4Cnt].u4PosY;
                                u4PosEY = _tVdp2ActPos[u4Cnt].u4PosEY;
                                break;
                        }
                }
        }

        VDO_LOG(VDO_LOG_LVL_REGRW, "vVdpHalSetMode: ucVdpId %d, u4PosX %d, u4PosY %d, u4PosEY %d u4Cnt %d\r\n",
                (int)ucVdpId, (int)u4PosX, (int)u4PosY, (int)u4PosEY, (int)u4Cnt);

        switch (u4Mode) {
        case RES_600P_1024:
        case RES_600P_1024_50HZ:
                u4VdpOutWidth = 1024;
                u4VdpOutHeight = 600;
                break;

        default:
                u4VdpOutWidth = 720;
                u4VdpOutHeight = 480;
        }

        prVdpSwReg->rField.fgVdoEn = 1;
        prVdpSwReg->rField.u4P_Skip = prSrcRegion->X & ~0x1;

        prVdpSwReg->rField.u4Total_Mbr = 0x1E;
        prVdpSwReg->rField.u4Max_Mbr = 0x1D;

        prVdpSwReg->rField.fgSP_LD = 1;

        //u4Vscale indicates the vertical scale factor to display a picture.The scale ratio is 4096/u4Vscale in interlace mode,
        //or 2048/u4Vscale in progressive mode.
        prVdpSwReg->rField.u4Vscale = (2048 * (prSrcRegion->Height - 1) + (prOutRegion->Height - 1) / 2) /
                (prOutRegion->Height - 1);

        VDO_LOG(VDO_LOG_LVL_REGRW, "vVdpHalSetMode: ucVdpId %d, u4VdpOutWidth %d, u4VdpOutHeight %d\r\n"
                , (int)ucVdpId, (int)u4VdpOutWidth, (int)u4VdpOutHeight);
        VDO_LOG(VDO_LOG_LVL_REGRW, "vVdpHalSetMode: ucVdpId %d, _u4LCDWidth %d, _u4LCDHeight %d\r\n"
                , (int)ucVdpId, (int)_u4LCDWidth, (int)_u4LCDHeight);
        VDO_LOG(VDO_LOG_LVL_REGRW, "vVdpHalSetMode: ucVdpId %d, prOutRegion->Width %d, prOutRegion->Height %d\r\n"
                , (int)ucVdpId, (int)prOutRegion->Width, (int)prOutRegion->Height);
        VDO_LOG(VDO_LOG_LVL_REGRW, "vVdpHalSetMode: ucVdpId %d, prSrcRegion->Width %d, prSrcRegion->Height %d\r\n"
                , (int)ucVdpId, (int)prSrcRegion->Width, (int)prSrcRegion->Height);

        if (prSrcRegion->Width > 720 && prVdpSwReg->rField.fgYFrm) {
                /*Set start line and start sub-line*/
                prVdpSwReg->au4Reg[0x20 / 4] = 0x07fa07f9;
                prVdpSwReg->au4Reg[0x24 / 4] = 0x07fd07fc;
                prVdpSwReg->au4Reg[0x28 / 4] = 0x00000000;
                prVdpSwReg->au4Reg[0x2C / 4] = 0x20602060;
                prVdpSwReg->au4Reg[0x50 / 4] = 0x07fa07f9;
                prVdpSwReg->au4Reg[0x54 / 4] = 0x07fd07fc;
        } else {
                if (prPmxDispReg.pDispMain->rField.fgPRGS) {
                        /*Set start line and start sub-line*/
                        unsigned int u4YTTPos, u4YBTPos, u4YTBPos, u4YBBPos;
                        unsigned int u4CTTPos, u4CBTPos, u4CTBPos, u4CBBPos;
                        unsigned int u4YSLTTini, u4YSLBTini;
                        unsigned int u4YSSLTTini, u4YSSLBTini;
                        unsigned int u4CSLTTini, u4CSLBTini;
                        unsigned int u4CSSLTTini, u4CSSLBTini;
                        bool fgYUV422 = prVdpSwReg->rField.fgYUV422;
                        unsigned int u4VScaleHW = prVdpSwReg->rField.u4Vscale;
                        unsigned int SrcY = prSrcRegion->Y;
                        // fld without filter
                        u4YSLTTini = 0x7FA;
                        u4YSSLTTini = 0;
                        u4YSLBTini = 0x7F9;
                        u4YSSLBTini = 0x80;
                        u4CSLTTini = fgYUV422? u4YSLTTini : 0x7FC;
                        u4CSSLTTini = fgYUV422? u4YSSLTTini : 0xE0;
                        u4CSLBTini = fgYUV422? u4YSLBTini : 0x7FD;
                        u4CSSLBTini = fgYUV422? u4YSSLBTini : 0x60;
      
                        u4YTTPos = SrcY * 128 + u4YSSLTTini;
                        u4YBTPos = SrcY * 128 + u4YSSLBTini;
                        u4YTBPos = u4YTTPos + ((u4VScaleHW) / 16);
                        u4YBBPos = u4YBTPos + ((u4VScaleHW) / 16);

                        prVdpSwReg->rField.u4YSLTT = u4YSLTTini + ((u4YTTPos / 256) * 2);
                        prVdpSwReg->rField.u4YSSLTT = (u4YTTPos % 256);
                        prVdpSwReg->rField.u4YSLBT = u4YSLBTini + ((u4YBTPos / 256) * 2);
                        prVdpSwReg->rField.u4YSSLBT = (u4YBTPos % 256);

                        prVdpSwReg->rField.u4YSLTB = u4YSLTTini + ((u4YTBPos / 256) * 2);
                        prVdpSwReg->rField.u4YSSLTB = (u4YTBPos % 256);
                        prVdpSwReg->rField.u4YSLBB = u4YSLBTini + ((u4YBBPos / 256) * 2);
                        prVdpSwReg->rField.u4YSSLBB = (u4YBBPos % 256);

                        if (fgYUV422)            //422
                        {
                                u4CTTPos = (SrcY * 128) + u4CSSLTTini;
                                u4CBTPos = (SrcY * 128) + u4CSSLBTini;
                                u4CTBPos = u4CTTPos + ((u4VScaleHW) / 16);
                                u4CBBPos = u4CBTPos + ((u4VScaleHW) / 16);
                        }
                        else
                        {                           //420
                                u4CTTPos = (SrcY * 128 / 2) + u4CSSLTTini;
                                u4CBTPos = (SrcY * 128 / 2) + u4CSSLBTini;
                                u4CTBPos = u4CTTPos + ((u4VScaleHW) / 32);
                                u4CBBPos = u4CBTPos + ((u4VScaleHW) / 32);
                        }
                        prVdpSwReg->rField.u4CSLTT = u4CSLTTini + ((u4CTTPos / 256) * 2);
                        prVdpSwReg->rField.u4CSSLTT = (u4CTTPos % 256);
                        prVdpSwReg->rField.u4CSLBT = u4CSLBTini + ((u4CBTPos / 256) * 2);
                        prVdpSwReg->rField.u4CSSLBT = (u4CBTPos % 256);

                        prVdpSwReg->rField.u4CSLTB = u4CSLTTini + ((u4CTBPos / 256) * 2);
                        prVdpSwReg->rField.u4CSSLTB = (u4CTBPos % 256);
                        prVdpSwReg->rField.u4CSLBB = u4CSLBTini + ((u4CBBPos / 256) * 2);
                        prVdpSwReg->rField.u4CSSLBB = (u4CBBPos % 256);
                } else {
                        /*Set start line and start sub-line for interlace mode*/
                }
        }

        //calclate HSFACTOR
        SrcWidth = prSrcRegion->Width;
        OutWidth = prOutRegion->Width;
        
        if (SrcWidth > OutWidth) {
                prVdpSwReg->rField.fgXHalf = 1;
                SrcWidth >>= 1;
        } else {
                prVdpSwReg->rField.fgXHalf = 0;
        }
        u4HFactor = (HSCALE_UNIT * (SrcWidth - 1) + (OutWidth - 1) / 2) / (OutWidth - 1);

        prPmxDispReg.pDispMain->rField.u2HACTBGN = u4PosX + prOutRegion->X;
        prPmxDispReg.pDispMain->rField.u2HACTEND = u4PosX + prOutRegion->X + prOutRegion->Width - 1;
        prPmxDispReg.pDispMain->rField.u2VOACTBGN = u4PosY + prOutRegion->Y;
        prPmxDispReg.pDispMain->rField.u2VOACTEND = u4PosY + prOutRegion->Y + prOutRegion->Height - 1;
        prPmxDispReg.pDispMain->rField.u2VEACTBGN = u4PosEY + prOutRegion->Y;
        prPmxDispReg.pDispMain->rField.u2VEACTEND = u4PosEY + prOutRegion->Y + prOutRegion->Height - 1;

        prPmxDispReg.pDispMain->rField.fgHSON = 1;

        //setting fmt 6c~7c, c0, e8, ec,c0
        prPmxDispReg.pDispMain->au4Reg[(0x6C / 4)] = 0x10000820; //hard code--why?
        prPmxDispReg.pDispMain->rField.u2HDWN_HBGN = u4PosX + 5; //70
        prPmxDispReg.pDispMain->rField.u2HDWN_HEND = prPmxDispReg.pDispMain->rField.u2HDWN_HBGN + u4VdpOutWidth; //70
        prPmxDispReg.pDispMain->rField.u2HDWN_VOBGN = u4PosY; //74
        prPmxDispReg.pDispMain->rField.u2HDWN_VOEND = prPmxDispReg.pDispMain->rField.u2HDWN_VOBGN + u4VdpOutHeight + 1;
        prPmxDispReg.pDispMain->rField.u2HDWN_VEBGN = prPmxDispReg.pDispMain->rField.u2HDWN_VOBGN; //78
        prPmxDispReg.pDispMain->rField.u2HDWN_VEEND = prPmxDispReg.pDispMain->rField.u2HDWN_VOEND; //78
        //hardcode for 1024*600 panel
        prPmxDispReg.pDispMain->au4Reg[(0x7C / 4)] = 0x01402940;  //hard code--why?
#if 0//move to arm2 for pp&vcp
        prPmxDispReg.pDispMain->au4Reg[(0xE8 / 4)] = 0x00040a7c;  //h/v sync delay
        prPmxDispReg.pDispMain->au4Reg[(0xEC / 4)] = 0x40020536;
#endif
        prPmxDispReg.pDispMain->au4Reg[(0xC0 / 4)] = 0x00b104b0; //hard code--why?

        if (0 == ucVdpId) {
                if (u4HFactor > 0x1000) {
                        prPmxDispReg.pDispMain->rField.u2HSFACTOR = HSCALE_UNIT;
                        prPmxDispReg.pDispMain->rField.fgHSON = 1;
                        prPmxDispReg.pDispMain->au4Reg[(0x6C / 4)] = ((u4HFactor << 16)| 0x821);

                        //use acc.
                        u4HFactor_inverse = (u4HFactor * 1000) / 0x1000;
                        u4HFactor_inverse = (0x1000 * 1000) / u4HFactor_inverse;
                        prPmxDispReg.pDispMain->rField.fgDownScaleMode = 1;
                        prPmxDispReg.pDispMain->rField.u4ScalerFactor = u4HFactor_inverse;
                        
                        adj_hact_vdo = prPmxDispReg.pDispMain->rField.u2HACTBGN;
                        adj_hact_hd = prPmxDispReg.pDispMain->rField.u2HDWN_HBGN;

                        adj_hact_vdo = SrcWidth +adj_hact_vdo +1;
                        adj_hact_hd = SrcWidth + adj_hact_hd +1;

                        prPmxDispReg.pDispMain->rField.u2HACTEND = adj_hact_vdo;
                        prPmxDispReg.pDispMain->rField.u2HDWN_HEND = adj_hact_hd;
                } else {
                        prPmxDispReg.pDispMain->rField.u2HSFACTOR = u4HFactor;
                        //prVdpSwReg->rField.fgXHalf = 0;                       
                        prPmxDispReg.pDispMain->rField.fgHSON = 1;

                        if (u4HFactor == 0x1000) { //fixme;
                                //use acc.
                                u4HFactor_inverse = (u4HFactor * 1000) / 0x1000;
                                u4HFactor_inverse = (0x1000 * 1000) / u4HFactor_inverse;
                                prPmxDispReg.pDispMain->rField.fgDownScaleMode = 1;
                                prPmxDispReg.pDispMain->rField.u4ScalerFactor = u4HFactor_inverse;
                        }
                }
        }//todo rear:vdpid == 1
        //setting fmt 48 : H Filter
        //todo


        if (prPmxDispReg.pDispMain->rField.u2HSFACTOR == HSCALE_UNIT)        // 1:1 scaler, turn on linear filter
        {
                prPmxDispReg.pDispMain->rField.fgHSLR = 1; // use linear filter
                prPmxDispReg.pDispMain->rField.fgHD_C_FIR_EN = 0; // use linear filter
        }
        else
        {
                prPmxDispReg.pDispMain->rField.fgHSLR = 0; // use fir
                prPmxDispReg.pDispMain->rField.fgHD_C_FIR_EN = 1; // use fir
        }

        prPmxDispRegMode[(0x6C / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0x70 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0x74 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0x78 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0x7C / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0xA0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0xA4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0xA8 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0xB0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0xC0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0xCC / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0xE4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
#if 0//move to arm2 for pp&vcp
        prPmxDispRegMode[(0xE8 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prPmxDispRegMode[(0xEC / 4)] |= PMX_HAL_REG_MODE_WRITE;    
#endif
        prVdpRegMode[(0x14 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prVdpRegMode[(0x18 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prVdpRegMode[(0x1C / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prVdpRegMode[(0x20 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prVdpRegMode[(0x24 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prVdpRegMode[(0x28 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prVdpRegMode[(0x2C / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prVdpRegMode[(0x34 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prVdpRegMode[(0x50 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prVdpRegMode[(0x54 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        prVdpRegMode[(0x64 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalSetFifo(__u8 ucVdpId)
{
        HAL_VDO_UNION_T *prVdpSwReg;
        __u8 *prVdpRegMode;

        GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

        if (prVdpSwReg->rField.fgHD_EN) {
                prVdpSwReg->au4Reg[0x34 / 4] = 0x3cc83c3c;
        } else {
                prVdpSwReg->au4Reg[0x34 / 4] = 0x20D82020;
        }

        prVdpRegMode[(0x34 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalSetYuv422(__u8 ucVdpId, bool fgYUV422)
{
        HAL_VDO_UNION_T *prVdpSwReg;
        __u8 *prVdpRegMode;

        GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

        prVdpSwReg->rField.fgYUV422 = fgYUV422;
        prVdpRegMode[(0x30 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalSetScanLine(__u8 ucVdpId, bool fgScLineEnable)
{
        HAL_VDO_UNION_T *prVdpSwReg;
        __u8 *prVdpRegMode;

        GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

        prVdpSwReg->rField.fgSLE = fgScLineEnable;
        prVdpRegMode[(0xE0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalSetHFilterMode(__u8 ucVdpId, bool fgFIRMode)
{
        
}

void vVdpHalSetYBufPtr(__u8 ucVdpId, __u32 u4AddrY, __u32 u4AddrC)
{
        if ((u4AddrY & 0x1FF) || (u4AddrC & 0x1FF)) {
                VDO_LOG(VDO_LOG_LVL_ERR, "vVdpHalSetVdoPtr: address not align, u4AddrY = 0x%x, u4AddrC = 0x%x\r\n",
                        (unsigned int)u4AddrY, (unsigned int)u4AddrC);
                ASSERT(0);
        } else {
                HAL_VDO_UNION_T *prVdpSwReg;
                __u8 *prVdpRegMode;

                GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

                prVdpSwReg->rField.u4YAddrY = u4AddrY >> 2;
                prVdpSwReg->rField.u4XAddrY = u4AddrY >> 2;
                prVdpSwReg->rField.u4WAddrY = u4AddrY >> 2;
                prVdpSwReg->rField.u4ZAddrY = u4AddrY >> 2;
                prVdpSwReg->rField.u4PTR_AF_Y = u4AddrY >> 2;

                prVdpSwReg->rField.u4YAddrC = u4AddrC >> 2;
                prVdpSwReg->rField.u4XAddrC = u4AddrC >> 2;
                prVdpSwReg->rField.u4ZAddrC = u4AddrC >> 2;
                prVdpRegMode[(0x00 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x08 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x80 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x84 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0xEC / 4)] |= PMX_HAL_REG_MODE_WRITE;
                
                prVdpRegMode[(0x04 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x0C / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0xFC / 4)] |= PMX_HAL_REG_MODE_WRITE;
        }
}

void vVdpHalSetDeintWXYZ(__u8 ucVdpId, const __u32 *pu4AddrY, const __u32 *pu4AddrC)
{
        if ((pu4AddrY) && (pu4AddrC)) {
                HAL_VDO_UNION_T *prVdpSwReg;
                __u8 *prVdpRegMode;

                GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

                prVdpSwReg->rField.u4WAddrY = (__u32)(pu4AddrY[0] >> 2);
                prVdpSwReg->rField.u4XAddrY = (__u32)(pu4AddrY[1] >> 2);
                prVdpSwReg->rField.u4YAddrY = (__u32)(pu4AddrY[2] >> 2);
                prVdpSwReg->rField.u4ZAddrY = (__u32)(pu4AddrY[3] >> 2);

                prVdpSwReg->rField.u4XAddrC = (__u32)(pu4AddrC[1] >> 2);
                prVdpSwReg->rField.u4YAddrC = (__u32)(pu4AddrC[2] >> 2);
                prVdpSwReg->rField.u4ZAddrC = (__u32)(pu4AddrC[3] >> 2);


                prVdpRegMode[(0x00 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x04 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x08 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x0C / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x80 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x84 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0xFC / 4)] |= PMX_HAL_REG_MODE_WRITE;
        } else {
                ASSERT(0);
        }
}

void vVdpHalSetFieldInfo(__u8 ucVdpId, bool fgTopFldFirst, bool fgFirstFld)
{
        HAL_VDO_UNION_T *prVdpSwReg;
        __u8 *prVdpRegMode;

        GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);

        if (fgTopFldFirst) {
                prVdpSwReg->rField.fgPFld = !fgFirstFld;      /*Field Setting 0 for top, 1 for bottom.*/
        } else {
                prVdpSwReg->rField.fgPFld = fgFirstFld;
        }

        prVdpRegMode[(0x30 / 4)] |= PMX_HAL_REG_MODE_WRITE;
}

void vVdpHalGetFieldInfo(__u32 u4VdpId, __u32 *prFldInfo)
{
        
}

unsigned int MOTION_THRESHOLD = 0x10;
static unsigned int COMB_THRESHOLD = 0x03;
static unsigned int CT_THRESHOLD = 0x20;

void vVdpHalSetDeintMode(__u8 ucVdpId, __u8 ucDiMode)
{
        HAL_VDO_UNION_T *prVdpSwReg;
        __u8 *prVdpRegMode;
        HAL_VDO_APQ_UNION_T *prVdpApqSwReg;
        __u8 *prVdpApqRegMode;
        
        GET_VDP_PTR(ucVdpId, prVdpSwReg, prVdpRegMode);
        GET_VDP_APQ_PTR(ucVdpId, prVdpApqSwReg, prVdpApqRegMode);
        //0x1C 24-31bit = 0x10, enable auto_reset? //xzr
        //prVdpSwReg->rField.u4TST = 0x10;
        //prVdpRegMode[(0x1C / 4)] |= PMX_HAL_REG_MODE_WRITE;

        //0x38 bit u4CTHRD 24-30bit Comb threshold
        prVdpSwReg->rField.u4CTHRD = 0x5;
        //prVdpRegMode[(0x38 / 4)] |= PMX_HAL_REG_MODE_WRITE;

        if (ucDiMode == VDP_DI_MA4F_MODE) {
                #if 1
                prVdpApqSwReg->rField.u4blend_Exp_Off_Sel  = 1;                 //Apq 0xF4
                prVdpApqSwReg->rField.u4Jaggy_smooth_enable = 0;                //Apq 0xF4
                prVdpApqSwReg->rField.u4Pre_Fetch_Line_Still_Dis = 0;           //Apq 0xF4

                prVdpSwReg->rField.fgPD32 = 0;  //verify is 1?                                           //0x1C
                //force every pxl as motion point in motion regions
                prVdpSwReg->rField.fgF_CMB = 0;                                 //0x1C

                prVdpSwReg->rField.fgYFrm = 0;                                  //0x30
                prVdpSwReg->rField.fgCFrm = 0;                                  //0x30
                // skip the first line to avoid chroma motion crash.
                prVdpSwReg->rField.fgNF5S_1ST = 1;                              //0x30

                prVdpSwReg->rField.u4CT_THRD = CT_THRESHOLD;                    //0x38
                prVdpSwReg->rField.u4MTHRD = MOTION_THRESHOLD;                  //0x38

                prVdpApqSwReg->rField.u4SelNewMd = 0;                           //0x4C
                prVdpApqSwReg->rField.u4SelNewPixDif = 0;
                prVdpApqSwReg->rField.u4EnVEdgeMag = 0;
                prVdpApqSwReg->rField.u4SelBigVEdge = 0;
                prVdpApqSwReg->rField.u4FrameVEdge = 0;
                prVdpApqSwReg->rField.u4MagSawOnly = 0;
                prVdpApqSwReg->rField.u4NoSpMoveChk = 0;
                prVdpApqSwReg->rField.u4UseUpDownClamp = 0;
                prVdpApqSwReg->rField.u4ClampNoMagnify = 0;

                prVdpSwReg->rField.fgEHF = 1;   //disable edge horizontal filter                   //0x70

                prVdpSwReg->rField.fgCSP_DIS = 0; // 20080617 by YL                              //0x78
                prVdpSwReg->rField.fgBE_4LINE_FORCE = 0;                        //0x78
                prVdpSwReg->rField.fgBE_4LINE_DIS = 0;                          //0x78
                prVdpSwReg->rField.fgINTRA_EDGEPMODE = 0;                       //0x78
                prVdpSwReg->rField.fgAYS4L_DIS = 0; // 0x42478[21]                               //0x78

                prVdpSwReg->rField.u4Video_Opt20 = 1; //?                                               //0x7C
                prVdpSwReg->rField.u4Video_Opt21 = 1; //?                                               //0x7C

                prVdpSwReg->rField.fgMA4F = 1;                                  //0x88
                prVdpSwReg->rField.u4FDIFF_CTRL = 5;                            //0x88

                prVdpSwReg->rField.u4MA_Video_Mode5 = 0;                        //0x8C
                prVdpSwReg->rField.u4MA_Video_Mode1 = 1;                        //0x8C

                prVdpSwReg->rField.u4MA_HW_Option3 = 1;                         //0x90

                prVdpSwReg->rField.u4EDGE_P_TH = 0x7c;                          //0xB0
                prVdpSwReg->rField.u4MA_EDGE_MODE0 = 1;                         //0xB0
                prVdpSwReg->rField.u4MA_EDGE_MODE1 = 1;                         //0xB0

                prVdpSwReg->rField.fgBLEND_EXP_OFF = 0;                         //0xC0
                prVdpSwReg->rField.fgGET5F = 0;                                 //0xC0
                prVdpSwReg->rField.fgMA5F = 0;                                  //0xC0
                prVdpSwReg->rField.fgWA_NA24   = 1;                             //0xC0

                prVdpSwReg->rField.u4PD_COMB_TH = COMB_THRESHOLD;               //0xC4
                prVdpSwReg->rField.fgVDO_32_PD_EN = 0; //verify is 1?                              //0xC4
                prVdpSwReg->rField.fgVDO_32_PD_MODE = 0;                        //0xC4

                //Chroma 3-field motion detection
                prVdpSwReg->rField.fgCRM_3FMD = 0; //verify is 1?                                     //0xD0
                prVdpSwReg->rField.fgC_VT_BLEND = 1; //sd                                              //0xD0
                prVdpSwReg->rField.fgC_INTER_X    = 0;                          //0xD0
                
                prVdpSwReg->rField.fgF_PRGS = 1;                                //0xE0
                prVdpSwReg->rField.fgF_L_SEL = 1; //First/Last fetching selection              //0xE0

                //skip config fusion
                #else
                prVdpSwReg->rField.fgMA4F = 1;
                //prVdpSwReg->rField.fgMDDI_CLK_ON = 1;

                prVdpSwReg->rField.u4Video_Opt20 = 1;
                prVdpSwReg->rField.u4Video_Opt21 = 1;

                prVdpSwReg->au4Reg[0xE0 / 4] |= 0xA0000000;

                prVdpSwReg->rField.fgYFrm = 0;
                prVdpSwReg->rField.fgCFrm = 0;
                #endif

                #if 1
                prVdpRegMode[(0x1C / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x30 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x38 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x4C / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x70 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x78 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x7C / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x88 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x8C / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x90 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0xB0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0xC0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0xC4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0xD0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0xE0 / 4)] |= PMX_HAL_REG_MODE_WRITE;

                prVdpApqRegMode[(0xF4 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                #else
                prVdpRegMode[(0x30 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x7C / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x88 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0xE0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                #endif
        } else {
                prVdpSwReg->rField.fgMA4F = 0;
                //prVdpSwReg->rField.fgMDDI_CLK_ON = 0;

                prVdpSwReg->rField.u4Video_Opt20 = 0;
                prVdpSwReg->rField.u4Video_Opt21 = 0;

                prVdpSwReg->au4Reg[0xE0 / 4] &= 0x5FFFFFFF;

                if (ucDiMode == VDP_DI_FRAME_MODE) {
                        prVdpSwReg->rField.fgYFrm = 1;
                        prVdpSwReg->rField.fgCFrm = 1;
                } else {
                        prVdpSwReg->rField.fgYFrm = 0;
                        prVdpSwReg->rField.fgCFrm = 0;
                        prVdpSwReg->rField.fgPFld = 0;
                }

                prVdpRegMode[(0x30 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x7C / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0x88 / 4)] |= PMX_HAL_REG_MODE_WRITE;
                prVdpRegMode[(0xE0 / 4)] |= PMX_HAL_REG_MODE_WRITE;
        }

}

void vVdpHalSetPdDetect(__u8 ucVdpId, bool fgEnable)
{
        
}

void vVdpHalSetMergeZ(__u32 u4VDP, bool fgMergeZ)
{
        
}


void vVdpHalDisMergeZ(__u32 u4VDP)
{
        
}

void vVdpHalResume(__u8 ucVdpId)
{
        
}

void vVdpHalIsr(__u8 ucVdpId)
{
        HAL_VDO_UNION_T *prVdpSwReg;
        volatile HAL_VDO_UNION_T *prVdpHwReg;
        __u8 *pVdpRegMode;
        __u32 u4RegIdx;

        GET_VDP_PTR(ucVdpId, prVdpSwReg, pVdpRegMode);
        GET_VDP_HW_PTR(ucVdpId, prVdpHwReg);

        /* update plane mixer display register at vsync*/
        for (u4RegIdx = 0; u4RegIdx < VDP_HAL_VDO_REG_NUM; u4RegIdx++) {
                if (pVdpRegMode[u4RegIdx] & PMX_HAL_REG_MODE_WRITE) {
                        prVdpHwReg->au4Reg[u4RegIdx] = prVdpSwReg->au4Reg[u4RegIdx];
                        pVdpRegMode[u4RegIdx] &= ~PMX_HAL_REG_MODE_WRITE;
                }

                if (pVdpRegMode[u4RegIdx] & PMX_HAL_REG_MODE_READ) {
                        prVdpSwReg->au4Reg[u4RegIdx] = prVdpHwReg->au4Reg[u4RegIdx];
                }
        }

        if (prVdpHwReg->rField.fgMA4F == 1) {
                /* chroma detect*/
                if ((prVdpHwReg->rField.u4M_CNT_XZ < 0xFF) && (prVdpHwReg->rField.u4FLD_WY_MOTION < 0xFF)) {
                        /* turn off chroma detection*/
                        prVdpHwReg->rField.fgCRM_3FMD = 0;
                        prVdpHwReg->rField.fgCRM_FDIFF = 0;
                        prVdpHwReg->rField.fgCRM_EXP_OFF = 0;
                } else {
                        prVdpHwReg->rField.fgCRM_3FMD = 1;
                        prVdpHwReg->rField.fgCRM_FDIFF = 1;
                        prVdpHwReg->rField.fgCRM_EXP_OFF = 1;
                }
        } else {
                prVdpHwReg->rField.fgCRM_3FMD = 0;
                prVdpHwReg->rField.fgCRM_FDIFF = 0;
                prVdpHwReg->rField.fgCRM_EXP_OFF = 0;
        }

        if (prVdpHwReg->rField.fgDISP3 == 1) { /* Workaround HW MergeZ issue*/
                prVdpHwReg->rField.u4XAddrC = prVdpHwReg->rField.u4ZAddrC;
        }

        if (_fgVdp1ResetInVSync && (ucVdpId == VDP_1)) {
                prVdpHwReg->rField.u4VDORST = 0xFF;
                prVdpHwReg->rField.u4VDORST = 0;

                _fgVdp1ResetInVSync = FALSE;
        }

        if (_fgVdp2ResetInVSync && (ucVdpId == VDP_2)) {
                prVdpHwReg->rField.u4VDORST = 0xFF;
                prVdpHwReg->rField.u4VDORST = 0;

                _fgVdp2ResetInVSync = FALSE;
        }
}

void vVdpHalGetMotionComb(__u32 u4VdpId, __u32 *prXYComb, __u32 *prWXComb, __u32 *prWYMotion, __u32 *prXZMotion)
{
        
}
#endif



