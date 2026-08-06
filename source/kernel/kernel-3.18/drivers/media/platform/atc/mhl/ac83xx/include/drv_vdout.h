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

#ifndef _DRV_VDO_H_
#define _DRV_VDO_H_

#include "drv_av_d.h"
#include "chip_ver.h"
#include "drv_config.h"

#define SUPPORT_CCIR_INPUT        1
#define SUPPORT_3D_WORK_AROUND    0
#define SUPPORT_NON_VIDEO_TIMING  1
#define SUPPORT_HDMI_RX_INPUT     1



typedef enum {
	CCIR_PIN_NON_USED = 0,
	CCIR_PIN_YCMIX_8BIT,
	CCIR_PIN_YCMIX_10BIT,
	CCIR_PIN_YCMIX_12BIT,
	CCIR_PIN_YC422_16BIT,
	CCIR_PIN_YC422_20BIT,
	CCIR_PIN_YC422_24BIT,
	CCIR_PIN_YC444_24BIT,
	CCIR_PIN_YC444_30BIT,
	CCIR_PIN_YC444_36BIT,

} CCIR_PIN_MUX_SETTING_E;

typedef enum {
	OSD_CLK_SOURCE_MAIN = 0,
	OSD_CLK_SOURCE_SCL,
	OSD_CLK_SOURCE_27M,
	OSD_CLK_SOURCE_P2I,
	OSD_CLK_SOURCE_DGI,
} OSD_CLK_SOURCE_T;

#if 1/*((CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8560) || (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8561) || \
							(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8563))*/
typedef enum {
	MIX_ALL_B_601_709 = 0,
	MIX_ALL_A_601_709,
	MIX_PLN1_PLN2,
	MIX_B_UI,
	ONLY_PLN1,
	ONLY_PLN2,

} COO_ENTRY_SEL_E;

typedef enum {
	COO_RESOLUTION_FEATURE_OFF = 0,
	COO_RESOLUTION_FEATURE_0,
	COO_RESOLUTION_FEATURE_1,
	COO_RESOLUTION_FEATURE_2,

} COO_FEATURE_RES_CTRL_E;

typedef enum {
	COO_FEATURE_C_OFF = 0,
	COO_FEATURE_C_ON,

} COO_FEATURE_C_CTRL_E;

typedef enum {
	COO_FEATURE_D_OFF = 0,
	COO_FEATURE_D_ON_1,
	COO_FEATURE_D_ON_2,

} COO_FEATURE_D_CTRL_E;

typedef enum {
	COO_FEATURE_ALL_OFF = 0,
	COO_FEATURE_ALL_ON,

} COO_FEATURE_ALL_CTRL_E;

typedef struct _COO_CONFIG_T {
	BOOL                      fgIs3DSrc;
	UCHAR                     ucFmt;
	COO_FEATURE_RES_CTRL_E    eFeatureRes;
	COO_FEATURE_C_CTRL_E      eFeatureC;
	COO_FEATURE_D_CTRL_E      eFeatureD;
	COO_FEATURE_ALL_CTRL_E    eFeatureAll;

} COO_CONFIG_T;

#endif

#if 0/* ((CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)) */

typedef enum {
	MIX_ALL_B_601_709 = 0,
	MIX_ALL_A_601_709,
	MIX_PLN1_PLN2,
	MIX_B_UI,
	ONLY_PLN1,
	ONLY_PLN2,

} DOVE_ENTRY_SEL_E;

typedef struct _DOVE_CONFIG_T {
	BOOL                      fgIs3DSrc;
	UCHAR                     ucFmt;
	DOVE_OUTPUT_DEPTH_T       eDeepColor;

} DOVE_CONFIG_T;

#endif

#define fgInputIs444(ucFmt) ((ucFmt == CCIR_PIN_YC444_24BIT) || (ucFmt == CCIR_PIN_YC444_30BIT) || \
							(ucFmt == CCIR_PIN_YC444_36BIT))

#define fgIs3DVideo(ucFmt) ((ucFmt == RES_3D_1080P23HZ) || (ucFmt == RES_3D_1080P24HZ) || \
							(ucFmt == RES_3D_1080P25HZ) || (ucFmt == RES_3D_1080P29HZ) || \
							(ucFmt == RES_3D_1080P30HZ) || \
							(ucFmt == RES_3D_720P60HZ) || (ucFmt == RES_3D_720P50HZ) || \
							(ucFmt == RES_3D_720P30HZ) || (ucFmt == RES_3D_720P25HZ) || \
							(ucFmt == RES_3D_1080I60HZ) || (ucFmt == RES_3D_1080I50HZ) || \
							(ucFmt == RES_3D_1080I30HZ) || (ucFmt == RES_3D_1080I25HZ) || \
							(ucFmt == RES_3D_480P60HZ) || (ucFmt == RES_3D_576P50HZ) || \
							(ucFmt == RES_3D_480I60HZ) || (ucFmt == RES_3D_576I50HZ) || \
							(ucFmt == RES_3D_480I30HZ) || (ucFmt == RES_3D_576I25HZ))

#define fgIsProg3DVideo(ucFmt) ((ucFmt == RES_3D_1080P23HZ) || (ucFmt == RES_3D_1080P24HZ) || \
							(ucFmt == RES_3D_1080P25HZ) || (ucFmt == RES_3D_1080P29HZ) || \
							(ucFmt == RES_3D_1080P30HZ) || \
							(ucFmt == RES_3D_720P60HZ) || (ucFmt == RES_3D_720P50HZ) || \
							(ucFmt == RES_3D_720P30HZ) || (ucFmt == RES_3D_720P25HZ) || \
							(ucFmt == RES_3D_480P60HZ) || (ucFmt == RES_3D_576P50HZ))

#define fgIsHDVideo(ucFmt) ((ucFmt == RES_1080P60HZ) || (ucFmt == RES_1080P50HZ) || \
							(ucFmt == RES_1080I60HZ) || (ucFmt == RES_1080I50HZ) || \
							(ucFmt == RES_1080P24HZ) || (ucFmt == RES_1080P23_976HZ) || \
							(ucFmt == RES_1080P30HZ) || (ucFmt == RES_1080P25HZ) || \
							(ucFmt == RES_1080P29_97HZ) || (ucFmt == RES_720P50HZ) || \
							(ucFmt == RES_720P60HZ) || \
							(ucFmt == RES_3D_1080P23HZ) || (ucFmt == RES_3D_1080P24HZ) || \
							(ucFmt == RES_3D_1080P25HZ) || (ucFmt == RES_3D_1080P29HZ) || \
							(ucFmt == RES_3D_1080P30HZ) || \
							(ucFmt == RES_3D_720P60HZ) || (ucFmt == RES_3D_720P50HZ) || \
							(ucFmt == RES_3D_720P30HZ) || (ucFmt == RES_3D_720P25HZ) || \
							(ucFmt == RES_3D_1080I60HZ) || (ucFmt == RES_3D_1080I50HZ) || \
							(ucFmt == RES_3D_1080I30HZ) || (ucFmt == RES_3D_1080I25HZ))

#define fgIsInterlaceVideo(ucFmt) ((ucFmt == RES_480I) || (ucFmt == RES_576I) || \
								(ucFmt == RES_1080I60HZ) || (ucFmt == RES_1080I50HZ))

#define fgIsVideoClk27M(ucFmt) ((ucFmt == RES_480I) || (ucFmt == RES_576I) || \
								(ucFmt == RES_480P) || (ucFmt == RES_576P))

#define fgIsVideoClk74M(ucFmt) ((ucFmt == RES_720P60HZ) || (ucFmt == RES_720P50HZ) || \
							(ucFmt == RES_1080I60HZ) || (ucFmt == RES_1080I50HZ) || \
							(ucFmt == RES_1080P23_976HZ) || (ucFmt == RES_1080P24HZ))

#define fgIsVideoClk148M(ucFmt) ((ucFmt == RES_1080P60HZ) || (ucFmt == RES_1080P50HZ) || \
					(ucFmt == RES_3D_1080P23HZ) || (ucFmt == RES_3D_1080P24HZ) || \
					(ucFmt == RES_3D_1080P25HZ) || (ucFmt == RES_3D_1080P29HZ) || \
					(ucFmt == RES_3D_1080P30HZ) || \
					(ucFmt == RES_3D_720P60HZ) || (ucFmt == RES_3D_720P50HZ) || \
					(ucFmt == RES_3D_720P30HZ) || (ucFmt == RES_3D_720P25HZ) || \
					(ucFmt == RES_3D_1080I60HZ) || (ucFmt == RES_3D_1080I50HZ) || \
					(ucFmt == RES_3D_1080I30HZ) || (ucFmt == RES_3D_1080I25HZ))

#define fgIsVideo4K2K(ucFmt) ((ucFmt == RES_2160P_23_976HZ) || (ucFmt == RES_2160P_24HZ) || \
					(ucFmt == RES_2160P_25HZ) || (ucFmt == RES_2160P_29_97HZ) || \
					(ucFmt == RES_2160P_30HZ) || (ucFmt == RES_2161P_24HZ))

void vSetVdoutDataClk(BOOL fgIsMainVideoHD, BOOL fgCavDownSampling, BOOL fgCavP2IMode, UINT32 u4HDRes);
void vSetTveCavVdout(BOOL fgIsScaleDownMode, BOOL fgCavDownSampling, BOOL fgCavP2IMode, UINT32 u4HDRes);
void vSetHdmiVdout(BOOL fgIsScaleDownMode, UINT32 u4HDRes);
void vUnSet3DMaster(void);

void vSetOSD5Src(UINT32 u4OSD5Src);
void vSetOSD5Path(UINT32 u4OSD5Sel, UINT32 e4ResMode);

void vSet3DHdmiVdout(HDMI_RESOLUTION_T u4HDRes);
void vSet2D480IHdmiVdout(void);
void vSetTveCvbsVdout(BOOL fgIsScaleDownMode, UINT32 u4HDRes);
void vSetVdoutOnOff(BOOL fgON);
void vSetCcirOutBeforeMixer(BOOL fgEnable);
void vSetCcirOut(CCIR_DRV_INFO_T rInfo, UCHAR ucFmt, CCIR_SRC_TYPE_E eSrc);
void vSetCcirIn(CCIR_DRV_INFO_T rInfo, UCHAR ucFmt);
void vSetCcirInOff(UCHAR ucFmt);
void vSetCcirInPinMux(UINT8 u1Type, UINT8 u1Mode);
void vSetCcirOutPinMux(UINT8 u1Type, UINT8 u1Mode);
void vDisableCcirOut(void);
void vDisableCcirIn(UCHAR ucFmt);
void vSetHdmiRx601DigitalIn(UCHAR ucFmt);
void vCCIR_Init(void);
void vSetOsdClkSrc(UINT8 u1OsdPlane, OSD_CLK_SOURCE_T u1ClkSelect);
void vCCIR_UnInit(void);
#if (SUPPORT_3D_WORK_AROUND)
void vSetCcirMVC(UCHAR ucFmt);
#endif
#if (SUPPORT_CCIR_INPUT)
void vSetHDMIRxCcirIn(CCIR_IN_DRV_INFO_T rInfo, UCHAR ucFmt);
#endif
UCHAR ucGetCCIRRes(void);
UINT32 u4GetP2iTvField(void);
void vP2IReset(void);
UINT32 u4GetCCIRFldCnt(void);
void vResetCCIRFldCnt(UCHAR ucCnt);
void vSetCCIRVSyncDelayForward(void);
#endif

#if 0/*(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8560) || (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8561) || \
							(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8563)*/
void vConfigCoo(COO_CONFIG_T rConfg);
void vCooMuxCtrl(UCHAR ucFmt);
void vCooWrite(UINT32 u4Addr, UINT32 u4Data);
BOOL fgIsCooEnable(void);
#if 0/*(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8563)*/
void vCOO_Pwr_Wap(BOOL fgEnable);
#endif
#endif
#if 0/*CONFIG_DRV_DOVE_SUPPORT*/
void vConfigDove(DOVE_CONFIG_T rConfg, BOOL fghdmi1);
#endif

#if 0/*CONFIG_DUALTX_DRV_EN*/
void vSet2D480IHdmi2Vdout(void);
void vSet3DHdmi2Vdout(HDMI_RESOLUTION_T u4HDRes);
void vUnSet3DMaster2(void);
void vSetHdmi2Vdout(BOOL fgIsScaleDownMode, UINT32 u4HDRes);
void vSetFmtToHdmi(FMTTOHDMI_MODE_T uFmt2Hdmi);
void vSetFmt2Clk(HDMI_RESOLUTION_T u4HDRes, FMTTOHDMI_MODE_T uFmt2Hdmi);
void vUnSet4k2k(void);
#endif
