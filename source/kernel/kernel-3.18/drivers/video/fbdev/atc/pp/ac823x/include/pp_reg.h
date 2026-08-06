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
/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2017-02-27
 */
#ifndef _PP_REG_H_
#define _PP_REG_H_
#define PP_HAL_WRITE32(_reg_, _val_)   		(*(volatile unsigned int*)((_reg_) + _IO_BASE_) = (_val_))
#define PP_HAL_READ32(_reg_)           		(*(volatile unsigned int*)((_reg_) + _IO_BASE_))
#define PP_WRITE32MSK(_reg_, _val_, _mask_)  PP_HAL_WRITE32(_reg_, (PP_HAL_READ32(_reg_) & (~(_mask_))) | ((_val_) & (_mask_)))

#if 1//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)
#define TDPROC_00 (PP_BASE + 0x000)	//0x5B000
    #define TDSHARP_GAIN1          0xFF << 24		//Fld(8, 24, AC_FULLB3) //31:24
    #define TDSHARP_LIMIT_POS1     0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_LIMIT_NEG1     0xFF << 8		//Fld(8, 8, AC_FULLB1) //15:8
    #define TDSHARP_CORING1        0xFF << 0		//Fld(8, 0, AC_FULLB0) //7:0
    
#define TDPROC_01 (PP_BASE + 0x004)	//0x5B004
    #define TDSHARP_CLIP_THPOS1    0xFF << 24		//Fld(8, 24, AC_FULLB3) //31:24
    #define TDSHARP_CLIP_THNEG1    0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_ATTENUATE_SEL1 0x03 << 8		//Fld(2, 8, AC_MSKB1) //10:8
    #define TDSHARP_CLIP_EN1       0x03 << 6		//Fld(2, 6, AC_MSKB0) //7:6
    #define TDSHARP_CLIP_BAND_SEL1 0x07 << 0		//Fld(3, 0, AC_MSKB0) // 2:0
    
#define TDPROC_06 (PP_BASE + 0x018)	//0x5B018
    #define TDSHARP_GAIN4          0xFF << 24		//Fld(8, 24, AC_FULLB3) //31:24
    #define TDSHARP_LIMIT_POS4     0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_LIMIT_NEG4     0xFF << 8		//Fld(8, 8, AC_FULLB1) //15:8
    #define TDSHARP_CORING4        0xFF << 0		//Fld(8, 0, AC_FULLB0) //7:0
    
#define TDPROC_07 (PP_BASE + 0x01C)	//0x5B01C
    #define TDSHARP_CLIP_THPOS4    0xFF << 24		//Fld(8, 24, AC_FULLB3) //31:24
    #define TDSHARP_CLIP_THNEG4    0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_ATTENUATE_SEL4 0x03 << 8		//Fld(2, 8, AC_MSKB1) //10:8
    #define TDSHARP_CLIP_EN4       0x03 << 6		//Fld(2, 6, AC_MSKB0) //7:6
    #define TDSHARP_CLIP_BAND_SEL4 0x07 << 0		//Fld(3, 0, AC_MSKB0) //2:0
    
#define TDPROC_08 (PP_BASE + 0x020)	//0x5B020
    #define TDSHARP_GAIN5          0xFF << 24		//Fld(8, 24, AC_FULLB3) //31:24
    #define TDSHARP_LIMIT_POS5     0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_LIMIT_NEG5     0xFF << 8		//Fld(8, 8, AC_FULLB1) //15:8
    #define TDSHARP_CORING5        0xFF << 0		//Fld(8, 0, AC_FULLB0) //7:0
    
#define TDPROC_09 (PP_BASE + 0x024)	//0x5B024
    #define TDSHARP_CLIP_THPOS5    0xFF << 24		//Fld(8, 24, AC_FULLB3) //31:24
    #define TDSHARP_CLIP_THNEG5    0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_ATTENUATE_SEL5 0x03 << 8		//Fld(2, 8, AC_MSKB1) //10:8
    #define TDSHARP_CLIP_EN5       0x03 << 6		//Fld(2, 6, AC_MSKB0) //7:6
    #define TDSHARP_CLIP_BAND_SEL5 0x07 << 0		//Fld(3, 0, AC_MSKB0) //2:0
    
#define TDPROC_0A (PP_BASE + 0x028)	//0x5B028
    #define TDSHARP_GAIN6          0xFF << 24		//Fld(8, 24, AC_FULLB3) //31:24
    #define TDSHARP_LIMIT_POS6     0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_LIMIT_NEG6     0xFF << 8		//Fld(8, 8, AC_FULLB1) //15:8
    #define TDSHARP_CORING6        0xFF << 0		//Fld(8, 0, AC_FULLB0) //7:0
    
#define TDPROC_0B (PP_BASE + 0x02C)	//0x5B02C
    #define TDSHARP_CLIP_THPOS6    0xFF << 24		//Fld(8, 24, AC_FULLB3) //31:24
    #define TDSHARP_CLIP_THNEG6    0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_ATTENUATE_SEL6 0x03 << 8		//Fld(2, 8, AC_MSKB1) //10:8
    #define TDSHARP_CLIP_EN6       0x03 << 6		//Fld(2, 6, AC_MSKB0) //7:6
    #define TDSHARP_CLIP_BAND_SEL6 0x07 << 0		//Fld(3, 0, AC_MSKB0) //2:0
    
#define TDPROC_0E (PP_BASE + 0x038)	//0x5B038
    #define TDSHARP_GAIN8          0xFF << 24		//Fld(8, 24, AC_FULLB3) //31:24
    #define TDSHARP_LIMIT_POS8     0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_LIMIT_NEG8     0xFF << 8		//Fld(8, 8, AC_FULLB1) //15:8
    #define TDSHARP_CORING8        0xFF << 0		//Fld(8, 0, AC_FULLB0) //7:0
    
#define TDPROC_0F (PP_BASE + 0x03C)	//0x5B03C
    #define TDSHARP_CLIP_THPOS8    0xFF << 24		//Fld(8, 24, AC_FULLB3) //31:24
    #define TDSHARP_CLIP_THNEG8    0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_ATTENUATE_SEL8 0x03 << 8		//Fld(2, 8, AC_MSKB1) //10:8
    #define TDSHARP_CLIP_EN8       0x03 << 6		//Fld(2, 6, AC_MSKB0) //7:6
    #define TDSHARP_CLIP_BAND_SEL8 0x07 << 0		//Fld(3, 0, AC_MSKB0) //2:0
    
#define TDPROC_10 (PP_BASE + 0x040)	//0x5B040
    #define TDSHARP_EN             0x01 << 31		//Fld(1, 31, AC_MSKB3) //31
    #define TDSHARP_RGB            0x01 << 30		//Fld(1, 30, AC_MSKB3) //30
    #define TDSHARP_HMASK          0x01 << 29		//Fld(1, 29, AC_MSKB3) //29
    #define TDSHARP_VMASK          0x01 << 28		//Fld(1, 28, AC_MSKB3) //28
    #define TDSHARP_LIMIT_POS_ALL  0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_LIMIT_NEG_ALL  0xFF << 8		//Fld(8, 8, AC_FULLB1) //15:8
    
#define TDPROC_11 (PP_BASE + 0x044)	//0x5B044
    #define TDSHARP_HEND           0xFFF << 12	//Fld(12, 12, AC_MSKW21) //23:12
    #define TDSHARP_LSTART         0xFFF << 0		//Fld(12, 0, AC_MSKW10) //11:0
    
#define TDPROC_12 (PP_BASE + 0x048)	//0x5B048
    #define TDSHARP_SPLIT_SIDE     0x01 << 15		//Fld(1, 15, AC_MSKB1) //15
    #define TDSHARP_SPLIT          0xFFF << 0		//Fld(12, 0, AC_MSKW10) //11:0
    
#define TDPROC_13 (PP_BASE + 0x04C)	//0x5B04C
    #define TDSHARP_HSX            0x01 << 26		//Fld(1, 26, AC_MSKB3) //26
    #define TDSHARP_NRM            0x01 << 25		//Fld(1, 25, AC_MSKB3) //25
    #define TDSHARP_VSX            0x01 << 24		//Fld(1, 24, AC_MSKB3) //24
    #define TDSHARP_SFT            0x03 << 20		//Fld(2, 20, AC_MSKB2) //21:20
    #define TDSHARP_PREC9          0x03 << 16		//Fld(2, 16, AC_MSKB2) //17:16
    #define TDSHARP_PREC8          0x03 << 14		//Fld(2, 14, AC_MSKB1) //15:14
    #define TDSHARP_PREC7          0x03 << 12		//Fld(2, 12, AC_MSKB1) //13:12
    #define TDSHARP_PREC6          0x03 << 10		//Fld(2, 10, AC_MSKB1) //11:10
    #define TDSHARP_PREC5          0x03 << 8		//Fld(2, 8, AC_MSKB1) //9:8
    #define TDSHARP_PREC4          0x03 << 6		//Fld(2, 6, AC_MSKB0) //7:6
    #define TDSHARP_PREC3          0x03 << 4		//Fld(2, 4, AC_MSKB0) //5:4
    #define TDSHARP_PREC2          0x03 << 2		//Fld(2, 2, AC_MSKB0) //3:2
    #define TDSHARP_PREC1          0x03 << 0		//Fld(2, 0, AC_MSKB0) //1:0

#define TDPROC_14 (PP_BASE + 0x050)	//0x5B050
    #define TDSHARP_GAIN9          0xFF << 24		//Fld(8, 24, AC_FULLB3) //31:24
    #define TDSHARP_LIMIT_POS9     0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_LIMIT_NEG9     0xFF << 8		//Fld(8, 8, AC_FULLB1) //15:8
    #define TDSHARP_CORING9        0xFF << 0		//Fld(8, 0, AC_FULLB0) //7:0

#define TDPROC_15 (PP_BASE + 0x054)	//0x5B054
    #define TDSHARP_CLIP_THPOS9    0xFF << 24		//Fld(8, 24, AC_FULLB3) //31:24
    #define TDSHARP_CLIP_THNEG9    0xFF << 16		//Fld(8, 16, AC_FULLB2) //23:16
    #define TDSHARP_ATTENUATE_SEL9 0x03 << 8		//Fld(2, 8, AC_MSKB1) //10:8
    #define TDSHARP_CLIP_EN9       0x03 << 6		//Fld(2, 6, AC_MSKB0) //7:6
    #define TDSHARP_MASK_SP        0x01 << 3		//Fld(1, 3, AC_MSKB0) //3:3
    #define TDSHARP_CLIP_BAND_SEL9 0x07 << 0		//Fld(3, 0, AC_MSKB0) //2:0  


/*-----------------------------------------------------------------------------
 *  EDGEP ?
 *-----------------------------------------------------------------------------*/
#define TDPROC_20 (PP_BASE + 0x080)	//0x5B080
    #define SAW_REMOP2             0x01 << 18		//Fld(1, 18, AC_MSKB2) // 18:18 
    #define SAW_REMOP1             0x01 << 17		//Fld(1, 17, AC_MSKB2) // 17:17
    #define EDGE_OP7               0x01 << 7		//Fld(1, 7, AC_MSKB0) // 7:7
    #define EDGE_OP6               0x01 << 6		//Fld(1, 6, AC_MSKB0) // 6:6
    #define EDGE_OP5               0x01 << 5		//Fld(1, 5, AC_MSKB0) // 5:5
    #define EDGE_OP4               0x01 << 4		//Fld(1, 4, AC_MSKB0) // 4:4
    #define EDGE_OP3               0x01 << 3		//Fld(1, 3, AC_MSKB0) // 3:3
    #define EDGE_OP2               0x01 << 2		//Fld(1, 2, AC_MSKB0) // 2:2
    #define EDGE_OP1               0x01 << 1		//Fld(1, 1, AC_MSKB0) // 1:1
    #define EDGEP_EN               0x01 << 0		//Fld(1, 0, AC_MSKB0) // 0:0

#define TDPROC_21 (PP_BASE + 0x084)	//0x5B084
    #define EDGEP_TH1              0xFF << 24		//Fld(8, 24, AC_FULLB3) // 0:0
    #define EDGEP_TH2              0xFF << 16		//Fld(8, 16, AC_FULLB2) // 0:0
    #define EDGEP_TH3              0xFF << 8		//Fld(8, 8, AC_FULLB1) // 0:0
    #define EDGEP_TH4              0xFF << 0		//Fld(8, 0, AC_FULLB0) // 0:0

#define TDPROC_22 (PP_BASE + 0x088)	//0x5B088
    #define EDGEP_TH5              0xFF << 24		//Fld(8, 24, AC_FULLB3) // 0:0
    #define EDGEP_TH6              0xFF << 16		//Fld(8, 16, AC_FULLB2) // 0:0
    #define EDGEP_TH7              0xFF << 8		//Fld(8, 8, AC_FULLB1) // 0:0
    #define EDGEP_TH8              0xFF << 0		//Fld(8, 0, AC_FULLB0) // 0:0

#define TDPROC_23 (PP_BASE + 0x08C)	//0x5B08C
    #define EDGEP_TH9              0xFF << 24		//Fld(8, 24, AC_FULLB3) // 0:0
    #define EDGEP_TH10             0xFF << 16		//Fld(8, 16, AC_FULLB2) // 0:0
    #define EDGEP_TH11             0xFF << 8		//Fld(8, 8, AC_FULLB1) // 0:0
    #define EDGEP_TH12             0xFF << 0		//Fld(8, 0, AC_FULLB0) // 0:0

#define TDPROC_24 (PP_BASE + 0x090)	//0x5B090
    #define EDGEP_TH13             0xFF << 24		//Fld(8, 24, AC_FULLB3) // 0:0
    #define EDGEP_TH14             0xFF << 16		//Fld(8, 16, AC_FULLB2) // 0:0
    #define EDGEP_TH15             0xFF << 8		//Fld(8, 8, AC_FULLB1) // 0:0
    #define EDGEP_TH16             0xFF << 0		//Fld(8, 0, AC_FULLB0) // 0:0

#define TDPROC_25 (PP_BASE + 0x094)	//0x5B094
    #define EDGEP_TH17             0xFF << 24		//Fld(8, 24, AC_FULLB3) // 0:0

#define TDPROC_26 (PP_BASE + 0x098)	//0x5B098
    #define SAW_REM_TH1            0xFF << 24		//Fld(8, 24, AC_FULLB3) // 0:0
    #define SAW_REM_TH2            0xFF << 16		//Fld(8, 16, AC_FULLB2) // 0:0
    #define SAW_REM_TH3            0xFF << 8		//Fld(8, 8, AC_FULLB1) // 0:0
    #define EDGEP_AMODE            0x03 << 0		//Fld(2, 0, AC_FULLB0) // 0:0

#define TDPROC_27 (PP_BASE + 0x09C)	//0x5B09C
    #define NM_GAIN                0xFF << 24		//Fld(8, 24, AC_FULLB3) // 0:0
    #define NM_OFST                0xFF << 16		//Fld(8, 16, AC_FULLB2) // 0:0
    #define TG_GAIN                0xFF << 8		//Fld(8, 8, AC_FULLB1) // 0:0
    #define TG_OFST                0xFF << 0		//Fld(8, 0, AC_FULLB0) // 0:0
    
#endif

/*-----------------------------------------------------------------------------
 *  ECTI
 *-----------------------------------------------------------------------------*/
#if 1//zxk (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8530)||(CONFIG_DRV_ENABLE_SIMP_POST_PROC == 1)
#define ECTI_00 (PP_BASE + 0x100)	//0x5B100
    #define ECTI_SPK               0x01 << 31	//Fld(1, 31, AC_MSKB3) //31
    #define ECTI_HUE_TIE           0x01 << 29	//Fld(1, 29, AC_MSKB3) //29
    #define ECTI_INK_ENA           0x01 << 28	//Fld(1, 28, AC_MSKB3) //28
    #define ECTI_STB_BYPASS        0x01 << 26	//Fld(1, 26, AC_MSKB3) //26
    #define ECTI_FLAT_BYPASS       0x01 << 25	//Fld(1, 25, AC_MSKB3) //25
    #define ECTI_ADPT_BYPASS       0x01 << 24	//Fld(1, 24, AC_MSKB3) //24
    #define ECTI_FLAT_SEL          0xFF << 16	//Fld(8, 16, AC_FULLB2) //23:16
    #define ECTI_FLAT_OFST         0xFF << 8	//Fld(8, 8, AC_FULLB1) //15:8
    #define ECTI_T_SEL             0x07 << 4	//Fld(3, 4, AC_MSKB0) //6:4
    #define ECTI_ADPT_SEL          0x0F << 0	//Fld(4, 0, AC_MSKB0) //3:0
    
#define ECTI_01 (PP_BASE + 0x104)	//0x5B104
    #define ECTI_STB_SEL           0x1FF << 12	//Fld(9, 12, AC_MSKW21) //20:12
    #define ECTI_STB_OFST          0x3FF << 0		//Fld(10, 0, AC_MSKW10) //9:0
    
#define ECTI_02 (PP_BASE + 0x108)	//0x5B108
    #define ECTI_ENA               0x01 << 28	//Fld(1, 28, AC_MSKB3) //28
    #define ECTI_FLAT_SZ           0x07 << 20	//Fld(3, 20, AC_MSKB2) //22:20
    #define ECTI_FLAT_FIX          0x01 << 16	//Fld(1, 16, AC_MSKB2) //16
    #define ECTI_LPF_SEL           0x03 << 12	//Fld(2, 12, AC_MSKB1) //13:12
    #define ECTI_ADPT_LPF          0x03 << 8	//Fld(2, 8, AC_MSKB1) //9:8
    
#define ECTI_03 (PP_BASE + 0x10C)	//0x5B10C
    #define WINDOW_X_END           0x7FF << 16	//Fld(11, 16, AC_MSKW32) //26:16 //[SSWu?]
    #define ECTI_VMASK             0x01 << 12		//Fld(1, 12, AC_MSKB1) //12
    #define ECTI_PRT_ENA           0x01 << 8		//Fld(1, 8, AC_MSKB1) //8
    #define ECTI_SGN_PRT           0x01 << 4		//Fld(1, 4, AC_MSKB0) //4
    #define ECTI_ADPT_OFST         0x07 << 0		//Fld(3, 0, AC_MSKB0) //2:0
    
#define ECTI_04 (PP_BASE + 0x110)	//0x5B110
    #define WINDOW_Y_END           0x7FF << 16	//Fld(11, 16, AC_MSKW32) //26:16 //[SSWu?]
    #define ECTI_LMT_ENA           0x01 << 12		//Fld(1, 12, AC_MSKB1) //12
    #define ECTI_LMT               0x3FF << 0		//Fld(10, 0, AC_MSKW10) //9:0

///?
#define HB_LTI_01 0x5B184
    #define HB_LTI_EN              0x01 << 0	//Fld(1, 0, AC_MSKB0) // 0:0
		
#define HB_LTI_02 0x5B188
    #define HB_V_SHIFT             0xFF << 24	//Fld(8, 24, AC_FULLB3) // 0:0
    #define HB_H_SHIFT             0xFF << 16	//Fld(8, 16, AC_FULLB2) // 0:0
    #define HB_V_GAIN              0xFF << 8	//Fld(8, 8, AC_FULLB1) // 0:0
    #define HB_H_GAIN              0xFF << 0	//Fld(8, 0, AC_FULLB0) // 0:0
#endif

#endif
