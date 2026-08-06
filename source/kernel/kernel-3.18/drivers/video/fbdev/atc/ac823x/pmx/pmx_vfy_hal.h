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
#ifndef PMX_VERIFY_HAL_H_
#define PMX_VERIFY_HAL_H_

#include "types.h"
// Panel Type
#define PANNEL_TTL   0
#define PANNEL_LVDS  1
#define PANNEL_RSV   2

//Panel Size 
#define PANEL_SIZE_800_480  0
#define PANEL_SIZE_800_600  1
#define PANEL_SIZE_1024_600 2
#define PANEL_SIZE_1280_720 3
#define PANEL_SIZE_1280_800 4
#define PANEL_SIZE_1920_1080 5 

#define PANEL_TYPE_SELECT	PANNEL_TTL		// 0: TTL Panel, 1: LVDS Panel 2: others
#define PANEL_SIZE_SELECT       PANEL_SIZE_800_480  

#define REG_SETTING(setting_data_array) #setting_data_array, setting_data_array, sizeof(setting_data_array)/4

typedef struct {
  const char *szArrayName;
	__u32    *pu4RegSetting;
	SIZE_T  size;
} REG_SET_T;

typedef struct rPmxGoldenCrc
{
    unsigned int u4TestCaseId;
    unsigned int *pu4GoldenCrc;
}PMX_GOLDEN_CRC_T;

extern REG_SET_T _ArVdoVCoefSetting[];
extern REG_SET_T _ArVdoV4CoefSetting[];
extern REG_SET_T _ArVdoV8CoefSetting[];
extern REG_SET_T _ArVdoHCoefSetting[];
extern UINT8 u1HdmiVideoID[21];
extern unsigned int u4HdmiAviInfoReg[];
extern PMX_GOLDEN_CRC_T _rPmxGoldenCRC[];
extern unsigned int u4PmxGoldenCrcCnt;


#define FRONT_ALL_HD_MEM_EN 0
#define DISPFPGA_REALCHIP_EN 1
#define PMX_REALCHIP_EN 1
//picture mode
#define PMX_VFY_VDO_FLD	0
#define PMX_VFY_VDO_FRM	1
#define PMX_VFY_VDO_4MA	2
#define PMX_VFY_VDO_4MEMA   3
#define PMX_VFY_VDO_4FUSION 4
#define PMX_VFY_VDO_4FUSION_CS 5

//source format
#define PMX_VFY_VDO_SRC_420_MB	0
#define PMX_VFY_VDO_SRC_422_MB	1
#define PMX_VFY_VDO_SRC_420_SL	2
#define PMX_VFY_VDO_SRC_422_SL	3

//definition of ZOOM type
#define PMX_VFY_H_ZOOM_1X		0x1
#define PMX_VFY_H_ZOOM_2X		0x2
#define PMX_VFY_H_ZOOM_3X		0x3
#define PMX_VFY_H_ZOOM_4X		0x4
#define PMX_VFY_H_ZOOM_A2X	0x5
#define PMX_VFY_H_ZOOM_A3X	0x6
#define PMX_VFY_H_ZOOM_A4X	0x7
#define PMX_VFY_H_ZOOM_1D2X	0x8
#define PMX_VFY_H_ZOOM_1D3X	0x9
#define PMX_VFY_H_ZOOM_1D4X	0xA
#define PMX_VFY_H_ZOOM_A1D2X	0xB
#define PMX_VFY_H_ZOOM_A1D3X	0xC
#define PMX_VFY_H_ZOOM_A1D4X	0xD

#define PMX_VFY_V_ZOOM_1X		0x1
#define PMX_VFY_V_ZOOM_2X		0x2
#define PMX_VFY_V_ZOOM_3X		0x3
#define PMX_VFY_V_ZOOM_4X		0x4
#define PMX_VFY_V_ZOOM_A2X	0x5
#define PMX_VFY_V_ZOOM_A3X	0x6
#define PMX_VFY_V_ZOOM_A4X	0x7
#define PMX_VFY_V_ZOOM_1D2X	0x8
#define PMX_VFY_V_ZOOM_1D3X	0x9
#define PMX_VFY_V_ZOOM_1D4X	0xA
#define PMX_VFY_V_ZOOM_A1D2X	0xB
#define PMX_VFY_V_ZOOM_A1D3X	0xC
#define PMX_VFY_V_ZOOM_A1D4X	0xD

#define PMX_VFY_DEMO_BDR_0D8    0
#define PMX_VFY_DEMO_BDR_1D8    1
#define PMX_VFY_DEMO_BDR_2D8    2
#define PMX_VFY_DEMO_BDR_3D8    3
#define PMX_VFY_DEMO_BDR_4D8    4
#define PMX_VFY_DEMO_BDR_5D8    5
#define PMX_VFY_DEMO_BDR_6D8    6
#define PMX_VFY_DEMO_BDR_7D8    7
#define PMX_VFY_DEMO_BDR_8D8    8

//FPGADISP select dump data point
#define PMX_VFY_FMT        0
#define PMX_VFY_RGB2HDMI   1
#define PMX_VFY_RGB2HDMI_SUB  2
#define PMX_VFY_TVE  3

typedef struct
{
    int i4HBGN;
    int i4HEND;
    int i4HOBGN;
    int i4HOEND;
} PMX_VFY_NEW_HDSL_H_OFST_T;

extern void vPmxVerifyHalXfsInit(unsigned int u4VdoId,unsigned int u4PmxFmt,UCHAR ucTvType,UCHAR ucInterlace);
extern void vPmxVerifyPanelSizeSel(unsigned int u4VdoId,unsigned int u4PmxFmt,UCHAR ucTvType);
extern void vPmxVerifyHalSysInit(void);
extern void vPmxVerifyHalLoadSetting(REG_SET_T *prRetSet);
extern void vPmxVerifyHalLoadCavSetting(REG_SET_T *prRetSet, BOOL fgIndex);
extern void vPmxVerifyHalLoadCvbsSetting(REG_SET_T *prRetSet, BOOL fgIndex);
extern void vPmxVerifyHalVdoPtr(UCHAR ucVdoId, UINT64 u4YBuf, UINT64 u4CBuf);
extern void vPmxVerifyHalSclDRAMPtr(UCHAR ucVdoId);
extern void vPmxVerifyHalAVIFrame(UCHAR ucRes, UCHAR ucColor);
extern void vPmxVerifyHalSclFilter(UCHAR ucOn, unsigned int u4CoefIdx);
extern void vPmxVerifyHalDispFmtHFilter(UCHAR ucVdoId, UCHAR ucYC, UCHAR ucCoef);
extern void vPmxVerifyHalDispFmtPhaseSel(UCHAR ucVdoId, UCHAR ucYPhase, UCHAR ucCPhase);
extern void vPmxVerifyHalDispFmtPhaseSel2(UCHAR ucVdoId, UCHAR ucYPhase, UCHAR ucCPhase,bool bYEven,bool bCEven);

extern void vPmxVerifyHalDispFmtSDModeSel(UCHAR ucVdoId, UCHAR ucSDMode);
extern void vPmxVerifyHalMvdoVFilter(UCHAR ucVdoId, UCHAR ucV4Tap, UCHAR ucCoef);
extern void vPmxVerifyHalVdoFit(unsigned int u4In, unsigned int u4Out, UCHAR ucVdoId);
//[xzr] add ucScanline for video demo version
extern void vPmxVerifyHalVdoFit2(unsigned int u4In, unsigned int u4Out, UCHAR ucVdoId, UCHAR ucSrcType, UCHAR ucTvType, unsigned int ucScanline);
extern void vPmxVerifyHalEnablePMX(UCHAR ucVdoId, UCHAR ucOn);
extern void vPmxVerifyHalEnableColorBar(UCHAR ucVdoId,UCHAR ucOn);
extern void vPmxVerifyHalVdoXSkip(unsigned int u4XSkip, UCHAR ucVdoId);
extern void vPmxVerifyHalV4Tap(UCHAR uc4Tap, UCHAR ucCoef, UCHAR ucSrcHD, UCHAR ucVdoId);
extern void vPmxVerifyHalV8Tap(UCHAR uc8Tap, UCHAR ucCoef, UCHAR ucSuper, UCHAR ucSrcHD, UCHAR ucVdoId);
extern void vPmxVerifyHalDeIntMode(UCHAR ucVdoId, UCHAR ucMode);
extern void vPmxVerifyHalOSDMixRatio(UCHAR ucOsdId, unsigned int u4MixRatio);
extern void vPmxVerifyHalSrcFmt(UCHAR ucVdoId, UCHAR ucSrcFmt);
extern void vPmxVerifyHalHDownScaler(UCHAR ucOn, UCHAR ucVdoId, unsigned int u4DownSrcSel, unsigned int u4HScaleUp, unsigned int u4HScaleDiv);
extern void vPmxVerifyHalHDownScaler2(UCHAR ucOn, UCHAR ucVdoId,unsigned int u4HScaleUp, unsigned int u4HScaleDiv,unsigned int Mode);

#if 1 //(CONFIG_CHIP_VER_PMXVDO == CONFIG_CHIP_VER_MT8563)
extern void vPmxVerifyHalNewHDownScaler(UCHAR ucOn, UCHAR ucVdoId, unsigned int u4DownSrcSel, unsigned int u4HScaleUp, unsigned int u4HScaleDiv,
    BOOL fgNewScale, BOOL fgPreScaleUseLinear, BOOL fgPostScaleUseAcc);
#endif
extern void vPmxVerifyHalLumaKey(UCHAR ucVdoId, UCHAR ucOn, unsigned int u4LumaKeyVal);
extern void vPmxVerifyHalNonLinear(UCHAR ucVdoId, UCHAR ucDispFmt);
extern void vPmxVerifyHalShiftLine(UCHAR ucVdoId, UCHAR ucHD, UCHAR ucLine);
extern void vPmxVerifyHalDispClearChkSum(UCHAR ucVdoId);
extern void vPmxVerifyHalDispInitChkSum(UCHAR ucVdoId);
extern unsigned int u4PmxVerifyHalDispGetChkSum(UCHAR ucVdoId);
extern void vPmxVerifyHalSetDataSource(UCHAR sel_hdmi, UCHAR sel_cav, UCHAR sel_cvbs);
extern void vPmxVerifyHalSetClockSource(UCHAR sel_hdmi, UCHAR sel_cav, UCHAR sel_cvbs);
extern void vPmxVerifyHalSetScalerClkSet(UCHAR vclk, UCHAR hclk);
extern void PmxVerifyHalFixCVBS(void);
extern void vPmxVerifyHalFmt2Timing(unsigned int u4PmxMode);
extern void vPmxVerifyHalModeSel(unsigned int u4PmxFmtNo, unsigned int u4Rbg2HdmiNo);
extern void PmxVerifyHal480iExtra(UCHAR ucVdoId);
extern void PmxVerifyHal576iExtra(UCHAR ucVdoId);
extern void PmxVerifyHal480PSD144Mode(void);
extern void PmxVerifyHal1080iExtra(UCHAR ucVdoId);
extern void PmxVerifyHal480I3dTiming(UCHAR ucVdoId);
extern void PmxVerifyHal480P3dTiming(UCHAR ucVdoId);
extern void PmxVerifyHal576I3dTiming(UCHAR ucVdoId);
extern void PmxVerifyHal576P3dTiming(UCHAR ucVdoId);
extern void PmxVerifyHal720P60HZ3dTiming(UCHAR ucVdoId);
extern void PmxVerifyHal720P50HZ3dTiming(UCHAR ucVdoId);
extern void PmxVerifyHal1080I60HZ3dTiming(UCHAR ucVdoId);
extern void PmxVerifyHal1080I50HZ3dTiming(UCHAR ucVdoId);
extern void PmxVerifyHal1080P25HZ3dTiming(UCHAR ucVdoId);
extern void PmxVerifyHal1080P24HZ3dTiming(UCHAR ucVdoId);
extern void PmxVerifyHal1080P30HZ3dTiming(UCHAR ucVdoId);
extern void vPmxVerifyHalSetVHLnrMode(unsigned int u4VdoId, unsigned int u4VLnrOn, unsigned int u4HLnrOn);
extern void vPmxVerifyHalDering(unsigned int ucVdoId, unsigned int u4Enable, unsigned int u4ThredY, unsigned int u4ThredC, unsigned int u4TransY);
extern void vPmxVerifyHalCfgV4Coff(unsigned int u4VdoId, unsigned int u4CoefIdx);
extern void vPmxVerifyHalBuidinColor(unsigned int u4Mvdo, unsigned int u4Enable, unsigned int u4BIY, unsigned int u4BICb, unsigned int u4BICr);
extern void vPmxVerifyHalBoardColor(unsigned int u4Mvdo,unsigned int u4module, unsigned int u4Enable, unsigned int u4XWidth, unsigned int u4YWidth, unsigned int u4BDY, unsigned int u4BDCb, unsigned int u4BDCr);
extern void PmxVerifyHalDemoMode(unsigned int u4Mvdo, unsigned int u4Enable, unsigned int u4BdrLine, unsigned int u4LeftMode);
extern void PmxVerifyHalNewSDEn(unsigned int u4VdoId, unsigned int u4Enable, unsigned int u4Type);
extern void PmxVerifyHalColorConv(unsigned int u4VdoId, unsigned int u4Enable, unsigned int u4B4, unsigned int u4TgtMode);
extern void PmxVerifyHalPostYLevTest(void);
extern void PmxVerifyHalPostYLevBurstTest(void);
extern void PmxVerifyHalPostLClipTest(void);
extern void PmxVerifyHalPostLClipBurstTest(void);
extern void vPmxVerifyHalVdoRXSkip(unsigned int u4RXSkip, UCHAR ucVdoId);
extern void vPmxVerifyHalSecArea(unsigned int u4Mvdo, unsigned int u4Enable, unsigned int u4XShift);
void vPmxVerifyHalSetCsMode(UCHAR u4VdoId, unsigned int u4On);
void vPmxVerifyHalSetHSharp(unsigned int u4VdoId, unsigned int u4Tap8, unsigned int u4On);
void PmxVerifyHalPostColorConv(unsigned int u4VdoId, unsigned int u4Enable, unsigned int u4TgtMode);
void PmxVerifyHalFpgaDispEn(unsigned int u4On);
void PmxVerifyHalSelSrcPoint(unsigned int u4SrcFmt, unsigned int u4SelSrc, unsigned int u4DramBufPtr);
void PmxVerifyHalUsingStandardTiming(unsigned int u4PmxId,unsigned int u4SrcWidth,unsigned int u4SrcHight,unsigned int u4DstWidth,unsigned int u4DstHight,unsigned int u4adj1,unsigned int u4adj2,unsigned int u4adj3);
void PmxVerifyHalSetPrgsForOsd3d(unsigned int u4Prgs_for_osd_3d);
void PmxVerifyHalSetSrcSize (UCHAR ucVdpId, unsigned int u4Width, unsigned int u4Height);
void vPmxVerifyHalVdoFitEx(unsigned int u4PicW, unsigned int u4PicH, BOOL fgVFit, BOOL fgHFit, UCHAR ucVdoId);
void PmxVerifyHalVdo2PowerOnOff (BOOL fgOn);

#endif //VDP_VERIFY_HAL_H_


