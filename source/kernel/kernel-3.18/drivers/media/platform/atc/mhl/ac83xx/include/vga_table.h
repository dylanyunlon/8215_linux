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

#ifndef _VGA_TABLE_H_
#define _VGA_TABLE_H_

/* #include "general.h" */
#define SUPPORT_VGA_USERMODE 1

#define CO(_PRO_INT, _DIV_MUL, _ADCPLLDIV, _CLKIN_MCODE) \
	((_PRO_INT << 6) | ((_DIV_MUL >> 1) << 5) | ((_ADCPLLDIV >> 1) << 4) | _CLKIN_MCODE)
#define Displaymode_delay 0 /* Modify for Disaplymode delay */

#ifndef _VGAMODE_STRUCT_
#define _VGAMODE_STRUCT_
typedef struct VGAMODE {    /*  14 bytes */
	UINT16 IHF; /*  Horizontal Frequency for timing search */
	UINT8 IVF; /*  Vertical Frequency for timing search */
	UINT16 ICLK; /*  Pixel Frequency */
	UINT16 IHTOTAL; /*  H Total */
	UINT16 IVTOTAL; /*  V Total */
	UINT16 IPH_SYNCW; /*  H Sync Width */
	UINT16 IPH_WID; /*  H Resolution */
	UINT16 IPH_BP; /*  H Back Porch */
	UINT16 IPV_STA; /*  V Back Porch + Sync Width */
	UINT16 IPV_LEN; /*  V Resolution */
	UINT16 COMBINE;
	/* UINT32 CLKIN_CWCODE; // ?? */
}  VGAMODE ; /*  using __attribute__((packed)) make armcc --gnu  internal fault ? */
#endif


extern const UINT8 bHdtvTimings;
extern const UINT8 bVgaTimings;
extern const UINT8 bUserHDMITimings;
extern const UINT8 bAllHDMITimings;

extern const UINT8 bUserVgaTimingBegin;

#if SUPPORT_VGA_USERMODE
#define  USERMODE_TIMING 8
typedef struct VGA_USRMODE {
	UINT16 vlen: 11; /* reference STA12 */
	UINT16 hsync_wvar: 4;
	UINT16 vpol: 1; /* reference STA12 */
	UINT16 hlen: 11; /* reference STA12 */
	UINT16 hlen_var: 4;
	UINT16 hpol: 1; /* reference STA12 */
	UINT8 hsync_w;
	UINT8 isCVTRB: 1;
	UINT8 unused: 3;
	UINT8 forceXY: 2;   /* force XY resolution*/
	UINT8 override: 1; /* override preset timing */
		UINT8 id: 1;    /*inverse to change return mode number */
	} VGA_USRMODE;

	typedef struct VGA_USRMODE_EXT {
	UINT16  h_res;  /* orig input h resolotion */
} VGA_USRMODE_EXT;

extern VGA_USRMODE rVgaUsrEEP[USERMODE_TIMING];     /* both  EEP & RAM */
extern VGA_USRMODE_EXT rVgaUsrExt[USERMODE_TIMING]; /* only on RAM */
#endif


#ifdef CC_UP8032_ATV
extern VGAMODE code VGATIMING_TABLE[];
#else
extern VGAMODE  VGATIMING_TABLE[];
#endif




/* VGA Mode */
extern UINT16 Get_HDMIMODE_IHF(UINT8 mode);
extern UINT8 Get_HDMIMODE_IVF(UINT8 mode);
extern UINT16 Get_HDMIMODE_ICLK(UINT8 mode);
extern UINT16 Get_HDMIMODE_IHTOTAL(UINT8 mode);
extern UINT16 Get_HDMIMODE_IVTOTAL(UINT8 mode);
extern UINT16 Get_HDMIMODE_IPH_STA(UINT8 mode);
extern UINT16 Get_HDMIMODE_IPH_SYNCW(UINT8 mode);
extern UINT16 Get_HDMIMODE_IPH_WID(UINT8 mode);
extern UINT16 Get_HDMIMODE_IPH_BP(UINT8 mode);
extern UINT8 Get_HDMIMODE_IPV_STA(UINT8 mode);
extern UINT16 Get_HDMIMODE_IPV_LEN(UINT8 mode);
extern UINT16 Get_HDMIMODE_COMBINE(UINT8 mode);
extern UINT8 Get_HDMIMODE_OverSample(UINT8 mode);


/*#define Get_VGAMODE_OverSample(bMode) (Get_VGAMODE_COMBINE(bMode)&0x01) //oversample */
#define Get_HDMIMODE_HSyncWidthChk(bMode) ((Get_HDMIMODE_COMBINE(bMode)>>1)&0x01)
#define Get_HDMIMODE_AmbiguousH(bMode) ((Get_HDMIMODE_COMBINE(bMode)>>2)&0x01)
#define Get_HDMIMODE_VPol(bMode) ((Get_HDMIMODE_COMBINE(bMode)>>3)&0x01)
#define Get_HDMIMODE_HPol(bMode) ((Get_HDMIMODE_COMBINE(bMode)>>4)&0x01)
#define Get_HDMIMODE_VSyncWidthChk(bMode) ((Get_HDMIMODE_COMBINE(bMode)>>5)&0x01)
#define Get_HDMIMODE_INTERLACE(bMode) ((Get_HDMIMODE_COMBINE(bMode)>>6)&0x01)
#define Get_HDMIMODE_PolChk(bMode) ((Get_HDMIMODE_COMBINE(bMode)>>7)&0x01)
#define Get_HDMIMODE_SupportVideo(bMode) ((Get_HDMIMODE_COMBINE(bMode)>>8)&0x01)
#define Get_HDMIMODE_VgaDisabled(bMode) ((Get_HDMIMODE_COMBINE(bMode)>>9)&0x01)
#define Get_HDMIMODE_YpbprDisabled(bMode) ((Get_HDMIMODE_COMBINE(bMode)>>10)&0x01)


#endif
