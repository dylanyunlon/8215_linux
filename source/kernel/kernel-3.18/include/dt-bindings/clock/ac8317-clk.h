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

#ifndef _DT_BINDINGS_CLK_AC8317_H
#define _DT_BINDINGS_CLK_AC8317_H


/*  &apmixedsys  */
/*  include  PLL clock */

/* ---- PLL clock ---- */
#define APMIXED_ARMPLL			1
#define APMIXED_ARMPLL2			2
#define APMIXED_SYSPLL			3
#define APMIXED_MSDCPLL			4
#define APMIXED_MMPLL			5
#define APMIXED_USBPLL			6
#define APMIXED_APLL1			7
#define APMIXED_APLL2			8
#define APMIXED_APLL3			9
#define APMIXED_ADPLL_324M		10
#define APMIXED_ADPLL_108M		11
#define APMIXED_ADPLL_648M		12
#define APMIXED_VPLL_TTL		13
#define APMIXED_ACLK			14
#define APMIXED_NR_CLK			15



/*  &topckgen  */
/*  include FACTOR  clock */

/* ---- FACTOR clock ---- */
#define TOP_DMPLL_CK			1  // MEMPLL
#define TOP_DMPLL_D2			2
#define TOP_DMPLL_D3			3
#define TOP_DMPLL_D4			4
#define TOP_DMPLL_D6			5
#define TOP_DMPLL_D8			6
#define TOP_DMPLL_D10			7
#define TOP_ARMPLL_CK			8  // ARMPLL
#define TOP_ARMPLL_D2			9
#define TOP_ARMPLL_D3			10
#define TOP_ARMPLL_D4			11
#define TOP_ARMPLL_D6			12
#define TOP_ARMPLL_D8			13
#define TOP_ARMPLL_D16			14
#define TOP_APLL1_CK			15  // APLL1
#define TOP_APLL1_D2			16
#define TOP_APLL1_D4			17
#define TOP_APLL2_CK			18  // APLL2
#define TOP_APLL2_D2			19
#define TOP_APLL2_D3			20
#define TOP_APLL2_D4			21
#define TOP_APLL3_CK			22  // APLL3
#define TOP_SYSPLL_CK			23  // SYSPLL
#define TOP_SYSPLL_D2			24
#define TOP_SYSPLL_D3			25
#define TOP_SYSPLL_D4			26
#define TOP_SYSPLL_D6			27
#define TOP_SYSPLL_D8			28
#define TOP_SYSPLL_D9			29
#define TOP_SYSPLL_D10			30
#define TOP_SYSPLL_D12			31
#define TOP_SYSPLL_D16			32
#define TOP_SYSPLL_D18			33
#define TOP_SYSPLL_D20			34
#define TOP_SYSPLL_D36			35
#define TOP_CLK27M_CK			36 // CLK27M
#define TOP_CLK27M_D2			37
#define TOP_CLK27M_D4			38
#define TOP_CLK27M_D8			39
#define TOP_CLK27M_D16			40
#define TOP_USBPLL_CK			41  // USBPLL
#define TOP_USBPLL_D2			42
#define TOP_USBPLL_D4			43
#define TOP_USBPLL_D6			44
#define TOP_USBPLL_D8			45
#define TOP_USBPLL_D10			46
#define TOP_USBPLL_D15			47
#define TOP_USBPLL_D20			48
#define TOP_ARMPLL2_CK			49  // ARMPLL2
#define TOP_ARMPLL2_D2			50
#define TOP_ARMPLL2_D4			51
#define TOP_ADPLL_324M_CK		52  // ADPLL_324M
#define TOP_ADPLL_324M_D2		53
#define TOP_ADPLL_108M_CK		54  // ADPLL_108M
#define TOP_ADPLL_108M_D2		55
#define TOP_ADPLL_108M_D3		56
#define TOP_ADPLL_648M_CK		57  // ADPLL_648M
#define TOP_RTC_CK			58  // RTC, 32k
#define TOP_VPLL_TTL_CK			59  // VPLL_TTL
#define TOP_VPLL_TTL_D3			60  
#define TOP_VPLL_TTL_D4			61
#define TOP_VPLL_TTL_D6			62
#define TOP_VPLL_TTL_D9			63
#define TOP_VPLL_TTL_D12		64
#define TOP_VPLL_TTL_D18		65
#define TOP_MSDCPLL_CK			66  //  MSDCPLL
#define TOP_MSDCPLL_D2			67
#define TOP_MSDCPLL_D3			68
#define TOP_MSDCPLL_D4			69
#define TOP_ACLK_K1			70
#define TOP_ACLK_K2			71
#define TOP_ACLK_K3			72
#define TOP_ACLK_K4			73
#define TOP_ACLK_K5			74
#define TOP_ACLK_K6			75
#define TOP_ACLK_K7			76
#define TOP_ACLK_K8			77
#define TOP_ACLK_K9			78
#define TOP_ACLK_K10			79
#define TOP_ACLK_K11			80
#define TOP_ACLK_K12			81
#define TOP_ACLK_K13			82
#define TOP_ACLK_K14			83
#define TOP_ACLK_IN			84
#define TOP_I2S_OUT2_MCLK		85
#define TOP_MPHON_IN			86
#define TOP_SYSPLL_D1500		87
#define TOP_ASIM_CK			88
#define TOP_PRE_USB_CK			89
#define TOP_AD_TTL_CK_D3		90
#define TOP_AD_TTL_CK_D4		91
#define TOP_AD_TTL_CK_D6		92
#define TOP_AD_TTL_CK_D9		93
#define TOP_AD_TTL_CK_D12		94
#define TOP_AD_TTL_CK_D18		95
#define TOP_I2S_OUT0_MCLK_INT_IN	96
#define TOP_I2S_OUT1_MCLK_INT_IN	97
#define TOP_SD_108M_CK			98
#define TOP_PANEL_CK			99
#define TOP_NORMAL_CK			100
#define TOP_ACK_IN			101
#define TOP_SPMCLK_IN			102
#define TOP_TTL_CK			103
#define TOP_LVDS_DPIX_CK		104
#define TOP_TWDS_DPIX_CK		105
#define TOP_SPMCK_IN			106
#define TOP_SPMCK2_IN			107
#define TOP_DEMUX_CK			108
#define TOP_EXT_TS0_CLK			109
#define TOP_TS_OUT			110
#define TOP_EXT_TS1_CLK			111
#define TOP_CVSB_ADC_CK			112
#define TOP_ACK_K6			113
#define TOP_ACK_K8			114
#define TOP_ACK_K9			115
#define TOP_BT_MCLK_IN			116
#define TOP_TEST_IN_0			117
#define TOP_ACLK_A1			118
#define TOP_ACLK_A2			119
//#define TOP_CLK_CLK27M			118
//#define TOP_CLK_CLK32K			119
#define TOP_NR_CLK			120


/*  &topselect  */
/*  include MUX clock */

/* ---- MUX clock ---- */
#define TOP_MUX_CLK_DRAM_SLOW		1
#define TOP_MUX_CLK_AUD_DVD		2
#define TOP_MUX_CLK_RFI1		3
#define TOP_MUX_CLK_RFI2		4
#define TOP_MUX_CLK_RFI3		5
#define TOP_MUX_CLK_RFI4		6
#define TOP_MUX_CLK_RFI5		7
#define TOP_MUX_CLK_RFI6		8
#define TOP_MUX_CLK_SD21		9
#define TOP_MUX_CLK_ADSP		10
#define TOP_MUX_CLK_DEMUX		11
#define TOP_MUX_CLK_RSIC		12
#define TOP_MUX_CLK_VDO			13
#define TOP_MUX_CLK_TP			14
#define TOP_MUX_CLK_TP_F32		15
#define TOP_MUX_CLK_RFI			16
#define TOP_MUX_CLK_BCLK		17
#define TOP_MUX_CLK_FLASH		18
#define TOP_MUX_CLK_RESZ		19
#define TOP_MUX_CLK_JPEG		20
#define TOP_MUX_CLK_VDEC		21
#define TOP_MUX_CLK_SPM			22
#define TOP_MUX_CLK_AXIM		23
#define TOP_MUX_CLK_DRAM		24
#define TOP_MUX_CLK_OSD			25
#define TOP_MUX_CLK_USB_27M		26
#define TOP_MUX_CLK_GRAPH		27
#define TOP_MUX_CLK_SD00		28
#define TOP_MUX_CLK_SD10		29
#define TOP_MUX_CLK_SD20		30
#define TOP_MUX_CLK_SD01		31
#define TOP_MUX_CLK_SD11		32
#define TOP_MUX_CLK_FPD			33
#define TOP_MUX_CLK_G3D			34
#define TOP_MUX_CLK_AUD			35
#define TOP_MUX_CLK_AUD2		36
#define TOP_MUX_CLK_CPU1		37
#define TOP_MUX_CLK_CPU2		38
#define TOP_MUX_CLK_MPHON		39
#define TOP_MUX_CLK_ARM_AUD		40
#define TOP_MUX_CLK_BT_MIC_AUD		41
#define TOP_MUX_CLK_NFLASH		42
#define TOP_MUX_CLK_DEG			43
#define TOP_MUX_CLK_DUTY		44
#define TOP_MUX_CLK_AUDIO_K1		45
#define TOP_MUX_CLK_AUDIO_K2		46
#define TOP_MUX_CLK_AUDIO_K3		47
#define TOP_MUX_CLK_AUDIO_K4		48
#define TOP_MUX_CLK_AUDIO_K5		49
#define TOP_MUX_CLK_AUDIO_K6		50
#define TOP_MUX_CLK_AUDIO_K7		51
#define TOP_MUX_CLK_AUDIO_K8		52
#define TOP_MUX_CLK_AUDIO_K9		53
#define TOP_MUX_CLK_AUDIO_K10		54
#define TOP_MUX_CLK_AUDIO_K11		55
#define TOP_MUX_CLK_AUDIO_K12		56
#define TOP_MUX_CLK_AUDIO_K13		57
#define TOP_MUX_CLK_AUDIO_K14		58
#define TOP_MUX_CLK_AUDIO_A1		59
#define TOP_MUX_CLK_AUDIO_A2		60
#define TOP_MUX_CLK_AUDIO_A3		61
#define TOP_MUX_CLK_PLL_TEST		62
#define TOP_MUX_CLK_AUD_ADC		63
#define TOP_MUX_CLK_AUD_PWM		64
#define TOP_MUX_CLK_MLIN2		65
#define TOP_MUX_CLK_AUD_MPH		66
#define TOP_MUX_CLK_MLIN		67
#define TOP_MUX_CLK_TS0			68
#define TOP_MUX_CLK_TS1			69
#define TOP_MUX_CLK_PNG			70
#define TOP_MUX_CLK_SPI_MOTO		71
#define TOP_MUX_CLK_TVD_MBIST		72
#define TOP_MUX_CLK_SRAMIF		73
#define TOP_MUX_CLK_MCLK_DIV2		74
#define TOP_MUX_CLK_DA_APLLCK		75
#define TOP_MUX_CLK_DA_APLL1CK		76
#define TOP_MUX_CLK_LVDS		77
#define TOP_MUX_CLK_TWDS		78
#define TOP_MUX_CLK_PWM0		79
#define TOP_MUX_CLK_PWM1		80
#define TOP_MUX_CLK_PWM2		81
#define TOP_MUX_CLK_PWM3		82
#define TOP_MUX_CLK_SIFM0		83
#define TOP_MUX_CLK_SIFM1		84
#define TOP_MUX_CLK_SIFS0		85
#define TOP_MUX_CLK_SIFS1		86
#define TOP_MUX_CLK_OSD1_P		87
#define TOP_MUX_CLK_OSD2_P		88
#define TOP_MUX_CLK_OSD3_P		89
#define TOP_MUX_CLK_OSD4_P		90
#define TOP_MUX_CLK_OSD_MAIN_P		91
#define TOP_MUX_CLK_OSD_AUX_P		92
#define TOP_MUX_CLK_VDT_FT		93
#define TOP_MUX_CLK_AUD_A1_TST	94
#define	TOP_MUX_CLK_AUD_A2_TST	95
#define TOP_MUX_NR_CLK			96


/* ---- DIVIDER clock  ----*/
/*  &subdivider  */

#define TOP_RISCCLK_RATIO		1
#define TOP_AXICLK_RATIO			2
#define TOP_AUD_K1_RATIO			3
#define TOP_AUD_K2_RATIO			4
#define TOP_AUD_K3_RATIO			5
#define TOP_AUD_K4_RATIO			6
#define TOP_AUD_K5_RATIO			7
#define TOP_AUD_K6_RATIO			8
#define TOP_AUD_K7_RATIO			9
#define TOP_AUD_K8_RATIO			10
#define TOP_AUD_K9_RATIO			11
#define TOP_AUD_K10_RATIO			12
#define TOP_AUD_K11_RATIO			13
#define TOP_AUD_K12_RATIO			14
#define TOP_AUD_K13_RATIO			15
#define TOP_AUD_K14_RATIO			16
#define TOP_AUD_A1_RATIO			17
#define TOP_AUD_A2_RATIO			18
#define TOP_AUD_A3_RATIO			19
#define TOP_DIVIDER_NR_CLK			20


/* ---- GATE clock ---- */
/* all below are GATE clock */

/*  &vdecsys  */
#define VDEC_FULL_CLK			1  // 9C, B8
#define VDEC_NR_CLK			2

/*  &imgdramcsys  */
#define IMG_DRAMC_CLK_GFX		1
#define IMG_DRAMC_CLK_DMARB		2
#define IMG_DRAMC_CLK_PNG		3
#define IMG_DRAMC_CLK_GIF		4
#define IMG_DRAMC_CLK_IMG_RESZ		5
#define IMG_DRAMC_CLK_OSD_RESZ		6
#define IMG_DRAMC_CLK_JPGDEC		7
#define IMG_DRAMC_CLK_DEMUX		8
#define IMG_DRAMC_CLK_DEMUX_TS0		9
#define IMG_DRAMC_CLK_DEMUX_TS1		10
#define IMG_DRAMC_CLK_DEMUX_27M		11
#define IMG_DRAMC_CLK_NFI		12
#define IMG_DRAMC_CLK_USB		13
#define IMG_DRAMC_CLK_IRT_DMA_WRAPPER	14
#define IMG_DRAMC_CLK_ARM9		15
#define IMG_DRAMC_NR_CLK		16

/*  &audioperisys  */
#define AUDIO_PERI_CLK_B00		1  // A8, C4
#define AUDIO_PERI_CLK_B01		2
#define AUDIO_PERI_CLK_B02		3
#define AUDIO_PERI_CLK_B03		4
#define AUDIO_PERI_CLK_B04		5
#define AUDIO_PERI_CLK_B05		6
#define AUDIO_PERI_CLK_B06		7
#define AUDIO_PERI_CLK_B07		8
#define AUDIO_PERI_CLK_B08		9
#define AUDIO_PERI_CLK_B09		10
#define AUDIO_PERI_CLK_B10		11
#define AUDIO_PERI_CLK_B11		12
#define AUDIO_PERI_CLK_B12		13
#define AUDIO_PERI_CLK_B13		14
#define AUDIO_PERI_CLK_B14		15
#define AUDIO_PERI_CLK_RFI_TOP		16
#define AUDIO_PERI_CLK_MSDC_PD0		17
#define AUDIO_PERI_CLK_MSDC_PD1		18
#define AUDIO_PERI_CLK_MSDC_PD2		19
#define AUDIO_PERI_CLK_MSDC_SW0		20
#define AUDIO_PERI_CLK_MSDC_SW1		21
#define AUDIO_PERI_CLK_MSDC_SW2		22
#define AUDIO_PERI_CLK_SPI_MOTO1	23
#define AUDIO_PERI_CLK_SPI_MOTO2	24
#define AUDIO_PERI_CLK_PWMX6_0		25
#define AUDIO_PERI_CLK_PWMX6_1		26
#define AUDIO_PERI_CLK_PWMX6_2		27
#define AUDIO_PERI_CLK_PWMX6_3		28
#define AUDIO_PERI_CLK_SIFM0		29
#define AUDIO_PERI_CLK_SIFM1		30
#define AUDIO_PERI_CLK_SIFS0		31
#define AUDIO_PERI_CLK_SIFS1		32
#define AUDIO_PERI_NR_CLK		33

/*  &dvdsys  */
#define DVD_CLK_MVDO			1  // AC, C8
#define DVD_CLK_DGO			2
#define DVD_CLK_OSD			3
#define DVD_CLK_GRA			4
#define DVD_CLK_BIM			5
#define DVD_CLK_TURBO32			6
#define DVD_CLK_VDEC			7
#define DVD_CLK_PARSER			8
#define DVD_CLK_RAMBUF			9
#define DVD_CLK_PT110			10
#define DVD_CLK_RS232			11
#define DVD_CLK_CDDVD			12
#define DVD_CLK_AUDIO			13
#define DVD_CLK_SERVO_MISC		14
#define DVD_CLK_RAMBUF_APCTRL_TBUS3	15
#define DVD_CLK_RAMBUF_APCTRL_TBUS4	16
#define DVD_CLK_RAMBUF_APCTRL_TBUS5	17
#define DVD_CLK_MFG_TOP_PWR_WRAP	18
#define DVD_NR_CLK			19


/*  &lvdssys  */
#define LVDS_CLK_LVDS			1  // B0, CC
#define LVDS_CLK_TP_TOP0		2
#define LVDS_CLK_TP_TOP1		3
#define LVDS_CLK_TP_TOP2		4
#define LVDS_CLK_RFI_TOP1		5
#define LVDS_CLK_RFI_TOP2		6
#define LVDS_CLK_RFI_TOP3		7
#define LVDS_CLK_RFI_TOP4		8
#define LVDS_CLK_RFI_TOP5		9
#define LVDS_CLK_RFI_TOP6		10
#define LVDS_NR_CLK			11

/*  &vdoutsys  */
#define VDOUT_CLK_SCLER			1  // B4, D0
#define VDOUT_CLK_TVD1			2
#define VDOUT_CLK_TVD2			3
#define VDOUT_CLK_OSD			4
#define VDOUT_CLK_OSD_R			5
#define VDOUT_CLK_FPD			6
#define VDOUT_CLK_FMT_VDO_F		7
#define VDOUT_CLK_FMT_VDO_R		8
#define VDOUT_CLK_WRITE_CHANEL		9
#define VDOUT_CLK_FRAME_LOCK		10
#define VDOUT_CLK_WRITE_CHANEL2		11
#define VDOUT_CLK_VGA			12
#define VDOUT_CLK_YPBPR			13
#define VDOUT_CLK_HDMI			14
#define VDOUT_CLK_TVE			15
#define VDOUT_CLK_DVD_MIX_2AP		16
#define VDOUT_CLK_OSD1			17
#define VDOUT_CLK_OSD2			18
#define VDOUT_CLK_OSD3			19
#define VDOUT_CLK_OSD4			20
#define VDOUT_CLK_OSD5          	21
#define VDOUT_CLK_OSD_R_2		22
#define VDOUT_CLK_OSD_R_3		23
#define VDOUT_CLK_SCLER_TG          	24
#define VDOUT_CLK_LCPROC_VDO          	25
#define VDOUT_NR_CLK			26

#endif				/* _DT_BINDINGS_CLK_AC8317_H */
