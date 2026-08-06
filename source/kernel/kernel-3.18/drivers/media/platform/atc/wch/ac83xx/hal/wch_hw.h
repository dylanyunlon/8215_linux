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

#ifndef _WCH_HW_H_
#define _WCH_HW_H_

#include "x_typedef.h"
#include "x_hal_io.h"
#include "x_hal_ic.h"

#ifndef __ARM2__
#include <linux/types.h>
extern void __iomem *wch0_sysreg_base;
extern void __iomem *wch1_sysreg_base;
extern unsigned int wch0irq;
extern unsigned int wch1irq;
extern struct clk *clk_ac8317_wch0;
extern struct clk *clk_ac8317_wch1;
extern struct pinctrl *pinctrl_wch;
#endif
extern void ac83xx_mask_ack_bim_irq(uint32_t irq);

#define WCH_HAL_REG_NUM                   (0x60/4)
/* for arm2 begin*/
#define IO_BASE_VA  0xFD000000
#define WCH1_HAL_REG    (IO_BASE_VA + 0x42300)
#define WCH2_HAL_REG    (IO_BASE_VA + 0x42500)
/* for arm2 end*/

typedef struct _WCH_HAL_FIELD_T {
	/* DWORD - 000 */
	u32      fgWchOn                           : 1;
	u32      fgCKeepIn                         : 1; /* timing control 0-program timing
	    						 (TVD, DGI, YPbPr, VGA and Display);
	    						 1-source timing (hdmi)*/
	u32      fgCFreeOn                         : 1; /* shadow control 0-need set reg_touch bit;
	    						 1-update reg at edge of vsync/field*/
	u32      fgRegTouch                        : 1; /* shadow control if c free on = 0, write 0->1
	    						 to update register at next edge of vsync/field*/

	u32      fgSramCtl                         : 1; /* control line buffer need set 1 */
	u32      fgDataSel                         : 1; /* 0-top field first; 1-any field first */
	u32      fgExtEAV                          : 1; /* 0-external eav disable; 1-external eav enable */
	u32      fgSramPp                          : 1; /* 0-may rw same address; 1-avoid rw same address */

	u32      reserved1                         : 4;

	u32      fgFldInv                          : 1; /* 0-normal field; 1-field inverse */
	u32      reserved40                        : 1;
	u32      fgLineAddrEn                      : 1; /* write enable */
	u32      fgHsynInv                         : 1;

	u32      u4SrcSel                          : 3;
	u32      fgVsynInv                         : 1;

	u32      fgProgSel                         : 1; /* 0-interlace; 1-progressive */
	u32      fgInputFmt                        : 1; /* 0-BT601/656; 1-420/422 */
	u32      reserved39                        : 3;
	u32      fgOutputFmt                       : 1; /* 0-420; 1-422*/
	u32      fgClrFrmFlg                       : 1; /* 0-default; 1-clear frame flag*/
	u32      fgIntrMode                        : 1; /* 0-progressive interrupt; 1-interlace interrupt*/

	u32      fgOutFldSet                       : 1; /* 0-disable field set mode; 1-enable,
	    						progressive set 1 and interlace set 0*/
	u32      fgOutFldIn                        : 1; /* 0-default; 1-field flag in field set mode*/
	u32      fgInFldSet                        : 1; /* 0-disable field set mode; 1-enable, always 0
	    						(3360 progressive set 1 and interlace set 0)*/
	u32      fgInFldIn                         : 1; /* 0-default; 1-field flag in field set mode*/

	/* DWORD - 004 */
	u32      fgAddrCtl                         : 1; /* 0-next vsync; 1-next interrupt*/
	u32      reserved38                        : 1;
	u32      fgInVdoFmt                        : 1; /* 0-BT656/YUV422; 1-BT601/YUV444*/
	u32      reserved2                         : 1;

	u32      u4Threshold                       : 2;
	u32      u4Changereq                       : 1;/*When fifo empty&line end FSM change
	    						 from request to last....0:disable;1:enable*/
	u32      fgVFlip                           : 1;

	u32      fgHFlip                           : 1;
	u32      u4FieldSel                        : 2; /* 01-V active refresh; 10-V sync refresh*/
	u32      reserved37                        : 6;
	u32      fgHEndSel                         : 1; /* 0-counter base; 1-Hsync base*/
	u32      fgHStartSel                       : 1; /* 0-counter base; 1-Hsync base*/
	u32      fgYCSwap                          : 1;

	u32      fgUVSwap                          : 1;
	u32      u4DeSel                           : 2;
	u32      fgAdjDE                           : 1; /* 0-normal; 1-DE always 1*/

	u32      u4DelayV                          : 3;
	u32      u4DelayU                          : 3;
	u32      u4DelayY                          : 2;

	/* DWORD - 008 */
	u32      u4Y0Addr                          : 28;
	u32      reserved3                         : 4;

	/* DWORD - 00C */
	u32      u4ActLine                         : 12;
	u32      reserved4                         : 20;

	/* DWORD - 010 */
	u32      u4C0Addr                          : 28;
	u32      reserved5                         : 4;

	/* DWORD - 014 */
	u32      u4DbgDepth                        : 28;
	u32      reserved6                         : 2;
	u32      fgDbgEndClr                       : 1;
	u32      fgDbgMode                         : 1;

	/* DWORD - 018 */
	u32      u4HStartPxl                       : 11;
	u32      reserved7                         : 5;
	u32      u4HpStartPxl                      : 11; /* Not used*/
	u32      reserved8                         : 5;

	/* DWORD - 01C */
	u32      reserved41                         : 10;
	u32      u4DisReqTimeOut                   : 1;/*this bit for handling WCH request
	    						memory bus timeout....0:disable;1:enable*/
	u32      reserved9                         : 21;

	/* DWORD - 020 */
	u32     reserved10                         : 32;

	/* DWORD - 024 */
	u32      u4TopLine                         : 11;
	u32      reserved11                        : 5;
	u32      u4BotLine                         : 11;
	u32      reserved12                        : 5;

	/* DWORD - 028 */
	u32      u4CFldNum                         : 12;
	u32      reserved13                        : 4;
	u32      u4YFldNum                         : 12;
	u32      reserved14                        : 4;

	/* DWORD - 02C */
	u32     reserved15                         : 32;

	/* DWORD - 030 */
	u32      reserved16                        : 30;
	u32      u4FsmRst                          : 2;

	/* DWORD - 034 */
	u32      u4HCnt                            : 11; /* Not used*/
	u32      reserved17                        : 5;
	u32      u4HActCnt                         : 11;
	u32      reserved18                        : 5;

	/* DWORD - 038 */
	u32      u4BGYHEndDW                       : 9;
	u32      u4BGCHEndDW                       : 9;
	u32      u4HSizeDW                         : 9;
	u32      reserved19                        : 5;

	/* DWORD - 03C */
	u32      u4FldEndSel                       : 3;
	u32      fgVsyncRst                        : 1; /* need set 1 to reset*/

	u32      reserved20                        : 1;
	u32      u4IntrEn                          : 3;

	u32      fgVsyncEn                         : 1; /* need set 1 to reset*/
	u32      reserved21                        : 1;
	u32      fgLStartEn                        : 1;
	u32      reserved22                        : 1;

	u32      fgSramCsEn                        : 1;
	u32      fgTstBar                          : 1;
	u32      u4Option                          : 10;

	u32      u4BT601FldDet                     : 8;

	/* DWORD - 040 */
	u32     reserved23                         : 32;

	/* DWORD - 044 */
	u32      u4InbufState                      : 4;
	u32      reserved                          : 4;

	u32      u4OutctlState                     : 6;
	u32      reserved24                        : 2;

	u32      u4AvState                         : 4;
	u32      reserved25                        : 4;

	u32      fgInHsync                         : 1;
	u32      fgInVsync                         : 1;
	u32      fgInFld                           : 1; /* 0-top; 1-bottom*/
	u32      fgOutFrame                        : 1;

	u32      reserved26                        : 1;
	u32      fgHCycCnt                         : 1;
	u32      reserved27                        : 1;
	u32      fgSrcType                         : 1; /* 0-NTSC; 1-PAL*/

	/* DWORD - 048 */
	u32      u4OutCCnt                         : 5;
	u32      reserved28                        : 2;
	u32      u4OutYCnt                         : 5;

	u32      reserved36                        : 2;
	u32      fgOutFld                          : 1; /* 0-top; 1-bottom*/
	u32      reserved29                        : 3;
	u32      fgFrame                           : 1;
	u32      fgSrcType1                        : 1; /* 0-NTSC; 1-PAL*/

	u32      reserved30                        : 11;
	u32      fgDbgEnd                          : 1;

	/* DWORD - 04C */
	u32      fgWrReq                           : 1;
	u32      fgWrLast                          : 1;
	u32      fgWrReq1                          : 1;
	u32      fgWrLast1                         : 1;

	u32      fgCReq                            : 1;
	u32      fgYReq                            : 1;
	u32      reserved35                        : 26;

	/* DWORD - 050 */
	u32      u4YAddr                           : 27;
	u32      fgVDispEnd                        : 1;

	u32      fgHDispEnd                        : 1;
	u32      reserved31                        : 3;

	/* DWORD - 054 */
	u32      u4BwPro                           : 2;
	u32      reserved32                        : 2;

	u32      fgYPbPrCtl                        : 1;
	u32      fgHdmiCtl                         : 1;
	u32      reserved33                        : 26;

	/* DWORD - 058 */
	u32      u4ProtAddr                        : 25;
	u32      reserved34                        : 5;
	u32      fgProtEn                          : 1;
	u32      fgProtClr                         : 1;

	/* DWORD - 05C */
	u32      u4CAddr                           : 27;
	u32      reserved42			      : 4;
	u32      fgOverWrFlg                       : 1;
} WCH_HAL_FIELD_T;

typedef union _WCH_HAL_UNION_T {
	u32            au4Reg[WCH_HAL_REG_NUM];
	WCH_HAL_FIELD_T   rField;
} WCH_HAL_UNION_T;

#define WCH_MIX_DVP_REG                     (0x1F040)
 #define DVP_OUT_ENABLE                     (1 << 5)

#define WCH_MIX_REG                         (0x1F030)
 #define WRITE_BACK_DVD                     (0)
 #define WRITE_BACK_FMTR                    (1)
 #define WRITE_BACK_FMTF                    (2)
 #define WRITE_BACK_MIX                     (3)

 #define WCH1_SEL_HDMI                      (0 << 4)
 #define WCH1_SEL_YPBPR                     (1 << 4)
 #define WCH1_SEL_TVD                       (2 << 4)
 #define WCH1_SEL_DGI                       (3 << 4)

 #define WCH1_SEL_WRITE_BACK                (0 << 7)
 #define WCH1_SEL_VDOIN                     (1 << 7)

 #define WCH2_SEL_HDMI                      (0 << 8)
 #define WCH2_SEL_YPBPR                     (1 << 8)
 #define WCH2_SEL_TVD                       (2 << 8)
 #define WCH2_SEL_DGI                       (3 << 8)

 #define WCH2_SERVO_DEBUG                   (1 << 10)

 #define WCH2_SEL_WRITE_BACK                (0 << 11)
 #define WCH2_SEL_VDOIN                     (1 << 11)

 #define WRITE_BACK_MASK                    (0x3)
 #define WCH1_VDOIN_MASK                    (0x30)
 #define WCH1_SEL_MASK                      (0xB0)
 #define WCH2_VDOIN_MASK                    (0x300)
 #define WCH2_SEL_MASK                      (0xF00)

#define WCH_CLK_SEL_REG                     (0x364)
 #define WCH1_CLK_SEL_WRTIEBACK             (0)
 #define WCH1_CLK_SEL_VDO                   (1)

 #define WCH2_CLK_SEL_WRTIEBACK             (0 << 1)
 #define WCH2_CLK_SEL_VDO                   (1 << 1)

 #define WCH1_CLK_SEL_HDMI                  (0 << 4)
 #define WCH1_CLK_SEL_YPBPR                 (1 << 4)
 #define WCH1_CLK_SEL_TVD                   (2 << 4)
 #define WCH1_CLK_SEL_DGI                   (3 << 4)

 #define WCH2_CLK_SEL_HDMI                  (0 << 8)
 #define WCH2_CLK_SEL_YPBPR                 (1 << 8)
 #define WCH2_CLK_SEL_TVD                   (2 << 8)
 #define WCH2_CLK_SEL_DGI                   (3 << 8)

 #define WCH2_CLK_SERVO_DEBUG               (1 << 12)

 #define WCH_DGI_CLK_INVERT                 (1 << 16)
 #define WCH_DGI_ENABLE                     (1 << 17)

 #define WCH1_CLK_SEL_MASK                  (0x31)
 #define WCH2_CLK_SEL_MASK                  (0x1302)
 #define WCH_DGI_CLK_MASK                   (0x30000)

#define WCH_CK_REG                          (0xB4)
 #define WCH1_CLK_EN                        (1 << 8)
 #define WCH2_CLK_EN                        (1 << 10)

#define WCH_RST_REG                         (0xD0)
 #define WCH1_RST_EN                        (1 << 8)
 #define WCH2_RST_EN                        (1 << 10)

#define WCH_PINMUX_REG                      (0x5C)  /* bit 4, 10, 12, 14 set 1 for DGI source, and others set 0*/
 #define WCH_DGI_PINMUX                     ((0x1 << 4)|(0x1<<10)|(0x1<<14)|(0x1<<12))
 #define WCH_DGI_MASK                       ((0x3 << 4)|(0x3<<10)|(0x3<<14)|(0x3<<12))

#define WCH_REG_READ(offset)			    IO_READ32(IO_BASE_VA, (offset))
#define WCH_REG_WRITE(offset, value)	    IO_WRITE32(IO_BASE_VA, (offset), (value))
#define WCH_REG_WRITE_MSK(offset, value, mask) \
		IO_WRITE32(IO_BASE_VA, (offset), ((IO_READ32(IO_BASE_VA, (offset)) & (~mask)) | ((value) & (mask))))

#endif  /* _WCH_HW_H_*/

