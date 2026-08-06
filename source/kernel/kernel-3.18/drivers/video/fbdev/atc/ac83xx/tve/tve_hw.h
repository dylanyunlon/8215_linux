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

#ifndef _TVE_HW_H_
#define _TVE_HW_H_

#include "x_hal_ic.h"
#include "pmx_hw.h"

#define TVE_BAK0_HAL_REG             (IO_BASE_VA + 0x2000)
#define TVE_BAK1_HAL_REG             (IO_BASE_VA + 0x2100)
#define TVE_BAK2_HAL_REG             (IO_BASE_VA + 0x2180)
#define TVE_BAK0_HAL_REG_NUM         (0x80/4)
#define TVE_BAK1_HAL_REG_NUM         (0x20/4)
#define TVE_BAK2_HAL_REG_NUM         (0x1C/4)

typedef struct _TVE_BAK0_HAL_FIELD_T {
	/* DWORD - 000 */
	__u32      u4Y2HBgn                          : 6;
	__u32      u4C2HBgn                          : 6;
	__u32      u4UVSwap                          : 1;
	__u32      u4Enc2YUV                         : 1;
	__u32      u4LineShift                       : 2;
	__u32      u4SyncLevel                       : 8;
	__u32      u4BlankRGB                        : 6;
	__u32      u4EncRst                          : 2;

	/* DWORD - 004 */
	__u32      u4EncOff                          : 1;
	__u32      u4CBOn                            : 1;
	__u32      u4Setup                           : 1;
	__u32      u4CavSetup                        : 1;
	__u32      u4CbCrLpfOn                       : 1;
	__u32      u4UVLpfOn                         : 1;
	__u32      u4COnly                           : 1;
	__u32      u4YLpfOn                          : 1;
	__u32      u4YDelay                          : 2;
	__u32      u4SYDelay                         : 2;
	__u32      u4YGDelay                         : 2;
	__u32      u4CupsOn                          : 1;
	__u32      u4CAVupsOn                        : 1;
	__u32      u4OutMode0                        : 2;
	__u32      u4OutMode1                        : 2;
	__u32      u4SyncOnG                         : 1;
	__u32      u4VGA                             : 1;
	__u32      u4DirectSel                       : 1;
	__u32      u4CavWssOff                       : 1;
	__u32      u4SposdSel                        : 1;
	__u32      u4SlewOff                         : 1;
	__u32      u4YUV7Video                       : 1;
	__u32      u4FULW                            : 1;
	__u32      u4CBType                          : 1;
	__u32      u4HVSynVideo                      : 1;
	__u32      u4Blacker                         : 1;
	__u32      u4Syn700_300Z                     : 1;

	/* DWORD - 008 */
	__u32      u4CCData                          : 16;
	__u32      u4CCMode                          : 3;
	__u32                                        : 13;

	/* DWORD - 00C */
	__u32      u4YScale                          : 8;
	__u32      u4Y2Scale                         : 8;
	__u32      u4BlankLevel                      : 4;
	__u32      u4BlankLevel2                     : 4;
	__u32      u4YScale8                         : 1;
	__u32      u4Y2Scale8                        : 1;
	__u32      u4P2IInvIne                       : 1;
	__u32      u4ExtGainEn                       : 1;
	__u32      u4UV2GainEn                       : 1;
	__u32      u4SY2Delay                        : 3;

	/* DWORD - 010 */
	__u32      u4CBScale                         : 32;

	/* DWORD - 014 */
	__u32      u4CRScale                         : 32;

	/* DWORD - 018 */
	__u32      u4UGain0                          : 8;
	__u32      u4UGain1                          : 8;
	__u32      u4UGain2                          : 8;
	__u32      u4UGain3                          : 8;

	/* DWORD - 01C */
	__u32      u4VGain0                          : 8;
	__u32      u4VGain1                          : 8;
	__u32      u4VGain2                          : 8;
	__u32      u4VGain3                          : 8;

	/* DWORD - 020 */
	__u32      u4Gamma0                          : 12;
	__u32                                        : 11;
	__u32      u4Osd888                          : 1;
	__u32      u4BrightLevel                     : 8;

	/* DWORD - 024 */
	__u32      u4Gamma1                          : 12;
	__u32                                        : 4;
	__u32      u4Gamma2                          : 12;
	__u32                                        : 4;

	/* DWORD - 028 */
	__u32      u4Gamma3                          : 12;
	__u32                                        : 4;
	__u32      u4Gamma4                          : 12;
	__u32                                        : 4;

	/* DWORD - 02C */
	__u32      u4Gamma5                          : 12;
	__u32                                        : 4;
	__u32      u4Gamma6                          : 12;
	__u32                                        : 4;

	/* DWORD - 030 */
	__u32      u4Gamma7                          : 12;
	__u32                                        : 4;
	__u32      u4Gamma8                          : 12;
	__u32                                        : 4;

	/* DWORD - 034 */
	__u32      u4DARB                            : 2;
	__u32      u4DAG                             : 2;
	__u32      u4DAX                             : 2;
	__u32      u4DACKPRGB                        : 1;
	__u32      u4DetIdFix                        : 1;
	__u32      u4Vcod                            : 2;
	__u32      u4DETLEN                          : 2;
	__u32      u4PDET                            : 1;
	__u32      u4TrimsetRisc                     : 1;
	__u32      u4DACKP                           : 1;
	__u32      u4VrefOff                         : 1;
	__u32      u4PDBCD                           : 4;
	__u32      u4PDCD                            : 4;
	__u32      u4PDBIAS                          : 4;
	__u32      u4TRIMVAL                         : 4;

	/* DWORD - 038 */
	__u32      u4BurstLevel                      : 8;
	__u32      u4SCH                             : 7;
	__u32      u4PAL60                           : 1;
	__u32      u4PanelVertEn                     : 1;
	__u32      u4FixHSync                        : 2;
	__u32      u4HSyncAdiEn                      : 1;
	__u32      u4HSyncDlySel                     : 2;
	__u32      u4HSyncWidthAdj                   : 2;
	__u32      u4DgoSel                          : 2;
	__u32      u4LpfRound                        : 1;
	__u32      u4DgoLpfEn                        : 1;
	__u32      u4YxcCkOff                        : 1;
	__u32      u4RgbCkOff                        : 1;
	__u32      u4Osd2GaiinEn                     : 1;
	__u32      u4DA12bit                         : 1;

	/* DWORD - 03C */
	__u32      u4HdSyn100                        : 11;
	__u32      u4Enc2UvDly                       : 2;
	__u32      u4SCDlyNew                        : 1;
	__u32      u4SYDlyNew                        : 1;
	__u32      u4Color709                        : 1;
	__u32      u4HdVgaSynPxl                     : 12;
	__u32      u4CbSel                           : 2;
	__u32      u4CrSel                           : 2;

	/* DWORD - 040 */
	__u32      u4WSDataP                         : 20;
	__u32      u4WSOnP                           : 1;
	__u32      u4WssLinePipeOffP                 : 1;
	__u32                                        : 2;
	__u32      u4WSLVLP                          : 8;

	/* DWORD - 044 */
	__u32      u4WSDataI                         : 20;
	__u32      u4WSOnI                           : 1;
	__u32      u4WssLinePipeOffI                 : 1;
	__u32                                        : 2;
	__u32      u4WSLVLI                          : 8;

	/* DWORD - 048 */
	__u32      u4PanelEndPxl                     : 12;
	__u32      u4ChangeEn                        : 1;
	__u32      u4PanelSel                        : 1;
	__u32      u4PanelOutEn                      : 1;
	__u32      u4PanelEn                         : 1;
	__u32      u4PanelBgnPxl                     : 12;
	__u32      u4Garmma2En                       : 1;
	__u32      u4BorrowClkInv                    : 1;
	__u32                                        : 2;

	/* DWORD - 04C */
	__u32      u4TeltextAddr                     : 26;
	__u32      u4TeltextCtrl                     : 6;

	/* DWORD - 050 */
	__u32      u4TtEndHcnt                       : 11;
	__u32                                        : 4;
	__u32      u4Busy                            : 1;
	__u32      u4TtBgnHcnt                       : 11;
	__u32                                        : 5;

	/* DWORD - 054 */
	__u32      u4TtEndLine                       : 5;
	__u32                                        : 3;
	__u32      u4TtBgnLine                       : 5;
	__u32                                        : 3;
	__u32      u4TtBufWidth                      : 9;
	__u32                                        : 3;
	__u32      u4TtAddrNum                       : 4;

	/* DWORD - 058 */
	__u32      u4TtP1                            : 9;
	__u32                                        : 7;
	__u32      u4TtP2                            : 11;
	__u32                                        : 5;

	/* DWORD - 05C */
	__u32      u4Ttsyn                           : 8;
	__u32                                        : 8;
	__u32      u4TtCodeEnd                       : 9;
	__u32                                        : 7;

	/* DWORD - 060 */
	__u32      u4ApSel                           : 1;  /* 0 -source from DVD; 1 - from AP*/
	__u32      u4DvdOsdSel                       : 1;  /* 0 -share ap spu as DVD osd; 1 - not share*/
	__u32      u4EncMixSpSel                     : 1;  /* 0 - for fmt; 1 - for tve*/
	__u32      u4EncMixSpSlef                    : 1;
	__u32      u4EncMixOsdSel                    : 1;  /* 0 - for fmt; 1 - for tve*/
	__u32      u4EncMixOsdSlef                   : 1;
	__u32      u4TveMixUi                        : 1;
	__u32      u4ApUiSel                         : 1;
	__u32                                        : 24;

	/* DWORD - 064 */
	__u32      u4CbCrGain                        : 32;

	/* DWORD - 068 */
	__u32      u4UGain4                          : 8;
	__u32      u4UGain5                          : 8;
	__u32      u4VGain4                          : 8;
	__u32      u4VGain5                          : 8;

	/* DWORD - 06C */
	__u32      u4TopThrd                         : 8;
	__u32      u4BotThrd                         : 9;
	__u32                                        : 3;
	__u32      u4Y2hend                          : 6;
	__u32      u4C2hend                          : 6;

	/* DWORD - 070 */
	__u32      u4DispEndPxl                      : 12;
	__u32                                        : 3;
	__u32      u4NewDinSel                       : 1;
	__u32      u4DispBgnPxl                      : 12;
	__u32                                        : 4;

	/* DWORD - 074 */
	__u32      u4BurstEnd                        : 8;
	__u32      u4BurstStart                      : 8;
	__u32      u4MVBurstStart                    : 8;
	__u32      u4AdvMVBurstStart                 : 8;

	/* DWORD - 078 */
	__u32      u4DAPinSel                        : 8;
	__u32                                        : 8;
	__u32      u4ChromaGain                      : 8;
	__u32      u4ChromaGainEn                    : 1;
	__u32      u4DacoutAbtEn                     : 1;
	__u32                                        : 2;
	__u32      u4DABOff                          : 1;
	__u32      u4DACOff                          : 1;
	__u32      u4DABgrefOff                      : 1;
	__u32      u4TestCompOff                     : 1;

	/* DWORD - 07C */
	__u32      u4VdacRev                         : 8;
	__u32      u4DAPlugDet                       : 5;
	__u32                                        : 18;
	__u32      u4TvdDirect                       : 1;
} TVE_BAK0_HAL_FIELD_T;

typedef union _TVE_BAK0_HAL_UNION_T {
	__u32            au4Reg[TVE_BAK0_HAL_REG_NUM];
	TVE_BAK0_HAL_FIELD_T   rField;
} TVE_BAK0_HAL_UNION_T;

#define TVE_BAK1_HAL_REG_NUM         (0x20/4)

typedef struct _TVE_BAK1_HAL_FIELD_T {
	/* DWORD - 000 */
	__u32      u4YLPF0                           : 6;
	__u32                                        : 2;
	__u32      u4YLPF1                           : 6;
	__u32                                        : 2;
	__u32      u4YLPF2                           : 6;
	__u32                                        : 2;
	__u32      u4YLPF3                           : 6;
	__u32                                        : 2;

	/* DWORD - 004 */
	__u32      u4YLPF4                           : 6;
	__u32                                        : 2;
	__u32      u4YLPF5                           : 6;
	__u32                                        : 2;
	__u32      u4YLPF6                           : 6;
	__u32                                        : 2;
	__u32      u4YLPF7                           : 6;
	__u32                                        : 2;

	/* DWORD - 008 */
	__u32      u4YLPF8                           : 7;
	__u32                                        : 1;
	__u32      u4YLPF9                           : 7;
	__u32                                        : 1;
	__u32      u4YLPF10                          : 7;
	__u32                                        : 1;
	__u32      u4YLPF11                          : 7;
	__u32                                        : 1;

	/* DWORD - 00C */
	__u32      u4YLPF12                          : 7;
	__u32                                        : 1;
	__u32      u4YLPF13                          : 7;
	__u32                                        : 1;
	__u32      u4YLPF14                          : 7;
	__u32                                        : 1;
	__u32      u4YLPF15                          : 7;
	__u32                                        : 1;

	/* DWORD - 010 */
	__u32      u4YLPF16                          : 8;
	__u32      u4YLPF17                          : 8;
	__u32      u4YLPF18                          : 8;
	__u32      u4YLPF19                          : 8;

	/* DWORD - 014 */
	__u32      u4C1LPF0                          : 7;
	__u32                                        : 1;
	__u32      u4C1LPF1                          : 7;
	__u32                                        : 1;
	__u32      u4C1LPF2                          : 7;
	__u32                                        : 1;
	__u32      u4C1LPF3                          : 7;
	__u32                                        : 1;

	/* DWORD - 018 */
	__u32      u4C1LPF4                          : 8;
	__u32      u4C2LPF0                          : 7;
	__u32                                        : 1;
	__u32      u4C2LPF1                          : 7;
	__u32                                        : 1;
	__u32      u4C2LPF2                          : 7;
	__u32                                        : 1;

	/* DWORD - 01C */
	__u32      u4C2LPF3                          : 7;
	__u32                                        : 1;
	__u32      u4C2LPF4                          : 8;
	__u32                                        : 16;
} TVE_BAK1_HAL_FIELD_T;

typedef union _TVE_BAK1_HAL_UNION_T {
	__u32                 au4Reg[TVE_BAK1_HAL_REG_NUM];
	TVE_BAK1_HAL_FIELD_T   rField;
} TVE_BAK1_HAL_UNION_T;

typedef struct _TVE_BAK2_HAL_FIELD_T {
	/* DWORD - 080 */
	__u32      u4N0                              : 8;
	__u32      u4N1                              : 6;
	__u32                                        : 2;
	__u32      u4N2                              : 6;
	__u32      u4N2_6                            : 1;
	__u32                                        : 1;
	__u32      u4N3                              : 6;
	__u32                                        : 2;

	/* DWORD - 084 */
	__u32      u4N4                              : 6;
	__u32                                        : 2;
	__u32      u4N5                              : 3;
	__u32                                        : 1;
	__u32      u4N6                              : 3;
	__u32                                        : 1;
	__u32      u4N7                              : 2;
	__u32                                        : 6;
	__u32      u4N8                              : 6;
	__u32                                        : 2;

	/* DWORD - 088 */
	__u32      u4N9                              : 6;
	__u32                                        : 2;
	__u32      u4N10                             : 6;
	__u32                                        : 2;
	__u32      u4YLPF11                          : 15;
	__u32                                        : 1;

	/* DWORD - 08C */
	__u32      u4N12                             : 15;
	__u32                                        : 1;
	__u32      u4N13                             : 8;
	__u32      u4N14                             : 8;

	/* DWORD - 090 */
	__u32      u4N15                             : 8;
	__u32      u4N16                             : 1;
	__u32      fgFixN17                          : 1;
	__u32      fgFixN18                          : 1;
	__u32      u4FixN19                          : 1;
	__u32      u4N17                             : 4;
	__u32      u4N18                             : 4;
	__u32      u4N19                             : 4;
	__u32      u4N20                             : 3;
	__u32                                        : 5;

	/* DWORD - 094 */
	__u32      u4N21                             : 10;
	__u32                                        : 6;
	__u32      u4N22                             : 1;
	__u32                                        : 11;
	__u32      fgEncMvOff                        : 1;
	__u32      fgCavMvOff                        : 1;
	__u32      fgMUVSw                           : 1;
	__u32                                        : 1;

	/* DWORD - 098 */
	__u32      u4N0P                             : 8;
	__u32      u4PorchLvl                        : 8;
	__u32      u4AgcLvl                          : 8;
	__u32      fgPal319                          : 1;
	__u32      fgBrst                            : 1;
	__u32      u4MfSel                           : 1;
	__u32                                        : 5;
} TVE_BAK2_HAL_FIELD_T;

typedef union _TVE_BAK2_HAL_UNION_T {
	__u32                 au4Reg[TVE_BAK2_HAL_REG_NUM];
	TVE_BAK2_HAL_FIELD_T   rField;
} TVE_BAK2_HAL_UNION_T;

#define MV_AGCLVL_NTSC  (0x99 << 16)
#define MV_AGCLVL_PAL   (0x9d << 16)
#define MV_BPLVL_NTSC   (0x87 << 8)
#define MV_BPLVL_PAL    (0x8a << 8)
#define MV_AGC_BP_MASK  (0xffff << 8)
#define GET_PMX_MIX_PTR(reg, mode) \
	do { \
		reg = &_rPmxHalMixSwReg; \
		mode = _rPmxMixRegMode; \
	} while (0)

#endif /* _TVE_HW_H_*/




