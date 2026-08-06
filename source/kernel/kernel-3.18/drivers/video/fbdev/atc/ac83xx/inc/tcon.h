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
#ifndef _TCON_H
#define _TCON_H


#define SV_PQ_GAMMA_NONE     3   /*No Gamma*/
#define SV_PQ_GAMMA_LOW      2   /*fine tune Gamma curve gain, range: 0/1/2*/
#define SV_PQ_GAMMA_MID      1  /**/
#define SV_PQ_GAMMA_HIGH     0  /**/


#define	DEFAULT_GAMMA_SET(bLevel)		vPanelSetGamma(bLevel)

#define	WHITEENHANCE		(1)
#define	BLACKENHANCE		(2)
#define	RIGHT					(0)
#define	LEFT					(1)
#define	UP						(0)
#define	DOWN					(1)

#define IS_FPGA_VER  0

/*mtk71543*/
typedef struct _TCON_TIMING {
	bool   bAdjEnable;
	bool   bPolInv;
	__u32 u4Hstart;
	__u32 u4Hend;
	__u32 u4Vstart;
	__u32 u4Vend;
} TCON_TIMING;


typedef enum {

	LVDS_OUTPUT_MODE_6BITS,
	LVDS_OUTPUT_MODE_8BITS,
	LVDS_OUTPUT_MODE_AUTO,

} LVDS_OUTPUT_MODE_E;


typedef struct _LCD_TCON_ARGS_T {
	__u32       u4Clock;
	__u32      u4HTotal;
	__u32      u4VTotal;
	bool        bEnableTcon;
	TCON_TIMING rHsync;
	TCON_TIMING rVsync;
	TCON_TIMING rDE;
} LCD_TCON_ARGS_T;

void vTCONReset(bool fgEnable, bool fgFillTable, bool fgHwReset);
void vLoadCMMSetting(void);
void vPanelSet(void);
void vPanelInvert(__u8 bLR, __u8 bUD);
/*void vPanelSetGamma (__u8 bLevel);*/
void vPanelSetGamma(__u8 *pu4GammaData);
void TconGetGamma(__u8 *const pbReadTable);
void vTConGeneralPro(__u8 bType);
void vTConPanelBWLevel(__u8 bType);
void vTconSetDEMd(void);
void vPanelSetSCE(void);
void vTconSetSclDEMd(void);
void vTConPatternGnrtr(__u8 bPattenType);
void vTconSetSYNCMd(void);
void vTconSetSclSYNCMd(void);
void  vTconSetBrightness(__u32 u4Value);
void  vTconSetContrast(__u32 u4Value);
void vTconTimingInput(__s32 i4En, __s32 i4HStart, __s32 i4HEnd, __s32 i4VStart, __s32 i4VEnd);
void vTconSetSaturation(__u32 u4Value);
void vTconSetHue(__u32 u4Value);
void vTconSetYGain(__u32 u4Value);
void vTconSetUGain(__u32 u4Value);
void vTconSetVGain(__u32 u4Value);
void vTconSetGainYUV(__u32 gain_Y, __u32 gain_U, __u32 gain_V);
void vTconSetDefautGainYUV(void);
void vFpdTconInit(void);
void vFpdInit(void);
void vSetLVDSOutputMode(LVDS_OUTPUT_MODE_E eOutputMode);
void TconSetDither(bool enable, __u32 u4Input, __u32 u4Output);
void vSetLVDSTX(__u32 level1,__u32 level2);
void vTconSetTiming(__u16 u2LCDType, LCD_TCON_ARGS_T timing);
void DisplaySetDrvAbility(__u32 curt);
int TconGetContrast(void);
int TconGetBrightness(void);
int TconGetHue(void);
int TconGetSaturation(void);
int TconGetYGain(void);
int TconGetUGain(void);
int TconGetVGain(void);
void vTconSupend(void);
void vTconResume(void);
int TconGetDither(void);
void LvdsSscConfig(unsigned int dir, unsigned int freq, unsigned int range);

#endif /* _TCON_H*/



