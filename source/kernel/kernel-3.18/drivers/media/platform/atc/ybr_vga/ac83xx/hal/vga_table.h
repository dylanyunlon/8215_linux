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

#ifndef VGA_TABLE_H_
#define VGA_TABLE_H_

#include "ybr_vga_util.h"
#include <linux/types.h>

/***Macro Define***/
#define SUPPORT_VGA_USERMODE  0
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#define PIX_CLK_LIMIT 743 //74.3MHz
#else
#define PIX_CLK_LIMIT 1485 //148.5MHz
#endif
//#define Get_VGAMODE_OverSample(bMode) (Get_VGAMODE_COMBINE(bMode)&0x01) //oversample
#define Get_VGAMODE_HSyncWidthChk(bMode) ((Get_VGAMODE_COMBINE(bMode)>>1)&(u16)0x01)
#define Get_VGAMODE_AmbiguousH(bMode) ((Get_VGAMODE_COMBINE(bMode)>>2)&(u16)0x01)
#define Get_VGAMODE_VPol(bMode) ((Get_VGAMODE_COMBINE(bMode)>>3)&(u16)0x01)
#define Get_VGAMODE_HPol(bMode) ((Get_VGAMODE_COMBINE(bMode)>>4)&(u16)0x01)
#define Get_VGAMODE_VSyncWidthChk(bMode) ((Get_VGAMODE_COMBINE(bMode)>>5)&(u16)0x01)
#define Get_VGAMODE_INTERLACE(bMode) ((Get_VGAMODE_COMBINE(bMode)>>6)&(u16)0x01)
#define Get_VGAMODE_PolChk(bMode) ((Get_VGAMODE_COMBINE(bMode)>>7)&(u16)0x01)
#define Get_HDMIMODE_SupportVideo(bMode) ((Get_VGAMODE_COMBINE(bMode)>>8)&(u16)0x01)
#define Get_VGAMODE_VgaDisabled(bMode) ((Get_VGAMODE_COMBINE(bMode)>>9)&(u16)0x01)
#define Get_VGAMODE_YpbprDisabled(bMode) ((Get_VGAMODE_COMBINE(bMode)>>10)&(u16)0x01)

/***Struct Define***/
typedef struct VGAMODE      // 14 u8s
{
    u16 IHF; // Horizontal Frequency for timing search
    u8 IVF; // Vertical Frequency for timing search
    u16 ICLK; // Pixel Frequency
    u16 IHTOTAL; // H Total
    u16 IVTOTAL; // V Total 
    u16 IPH_SYNCW; // H Sync Width
    u16 IPH_WID; // H Resolution
    u16 IPH_BP; // H Back Porch
    u16 IPV_STA; // V Back Porch + Sync Width
    u16 IPV_LEN; // V Resolution
    u16 COMBINE; // ??
    //u32 CLKIN_CWCODE; // ??
}  VGAMODE ; // using __attribute__((packed)) make armcc --gnu  internal fault ?

#if SUPPORT_VGA_USERMODE
#define  USERMODE_TIMING 8
typedef struct VGA_USRMODE  
{
    u16 vlen:11; /* reference STA12 */
    u16 hsync_wvar:4;
    u16 vpol:1;  /* reference STA12 */
    u16 hlen:11; /* reference STA12 */
    u16 hlen_var:4;
    u16 hpol:1;  /* reference STA12 */
    u8 hsync_w;  
    u8 isCVTRB:1;
    u8 unused:3;
    u8 forceXY:2;    /* force XY resolution*/
    u8 override:1; /* override preset timing */
    u8 id:1;     /*inverse to change return mode number */
} VGA_USRMODE;

typedef struct VGA_USRMODE_EXT
{
    u16  h_res;  //orig input h resolotion
} VGA_USRMODE_EXT;

extern VGA_USRMODE rVgaUsrEEP[USERMODE_TIMING];     //both  EEP & RAM
extern VGA_USRMODE_EXT rVgaUsrExt[USERMODE_TIMING]; //only on RAM
#endif

/***Function Declaration***/
extern u16 Get_VGAMODE_IHF(u8 mode) ;
extern u8 Get_VGAMODE_IVF(u8 mode) ;
extern u16 Get_VGAMODE_ICLK(u8 mode) ;
extern u16 Get_VGAMODE_IHTOTAL(u8 mode) ;
extern u16 Get_VGAMODE_IVTOTAL(u8 mode) ;
extern u16 Get_VGAMODE_IPH_STA(u8 mode) ;
extern u16 Get_VGAMODE_IPH_SYNCW(u8 mode) ;
extern u16 Get_VGAMODE_IPH_WID(u8 mode) ;
extern u16 Get_VGAMODE_IPH_BP(u8 mode) ;
extern u8 Get_VGAMODE_IPV_STA(u8 mode) ;
extern u16 Get_VGAMODE_IPV_LEN(u8 mode) ;
extern u16 Get_VGAMODE_COMBINE(u8 mode) ;
extern u8 Get_VGAMODE_OverSample(u8 mode) ;

/***Extern Variable Declaration***/
extern VGAMODE  VGATIMING_TABLE[];
extern const u8 bHdtvTimings;
extern const u8 bVgaTimings;
extern const u8 bUserVgaTimings;
extern const u8 bAllTimings;
extern const u8 bUserVgaTimingBegin;

#endif


