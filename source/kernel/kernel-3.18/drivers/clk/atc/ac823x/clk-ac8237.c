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

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/clkdev.h>
#include "../clk-mux.h"
#include "../clk-pll.h"
#include "../clk-gate.h"
#include "../clk-divider.h"
#include "clk-ac8237-pll.h"
#include <dt-bindings/clock/ac8237-clk.h>

/*
 * platform clocks
 */

/* ROOT */
#define clk_null		"clk_null"
#define clk27m			"clk27m"
#define clk32k			"clk32k"

/* PLL */
#define armpll			"armpll"
#define armpll2			"armpll2"
#define msdcpll			"msdcpll"
#define mmpll			"mmpll"
#define apll1			"apll1"
#define apll2			"apll2"
#define apll3			"apll3"
#define syspll			"syspll"
#define usbpll			"usbpll"
#define adpll_324m		"adpll_324m"
#define adpll_108m		"adpll_108m"
#define adpll_648m		"adpll_648m"
#define hadds2			"hadds2"
#define vpll_ttl		"vpll_ttl"
#define aclk			"aclk"
#define g3dpll			"g3dpll"

/* FACTOR */
#define dmpll_ck		"dmpll_ck"
#define dmpll_d2		"dmpll_d2"
#define dmpll_d3		"dmpll_d3"
#define dmpll_d4		"dmpll_d4"
#define dmpll_d6		"dmpll_d6"
#define dmpll_d8		"dmpll_d8"
#define dmpll_d10		"dmpll_d10"
#define armpll_ck		"armpll_ck"
#define armpll_d2		"armpll_d2"
#define armpll_d3		"armpll_d3"
#define armpll_d4		"armpll_d4"
#define armpll_d6		"armpll_d6"
#define armpll_d8		"armpll_d8"
#define armpll_d16		"armpll_d16"
#define apll1_ck		"apll1_ck"
#define apll1_d2		"apll1_d2"
#define apll1_d4		"apll1_d4"
#define apll2_ck		"apll2_ck"
#define apll2_d2		"apll2_d2"
#define apll2_d3		"apll2_d3"
#define apll2_d4		"apll2_d4"
#define apll3_ck		"apll3_ck"
#define syspll_ck		"syspll_ck"
#define syspll_d2		"syspll_d2"
#define syspll_d3		"syspll_d3"
#define syspll_d4		"syspll_d4"
#define syspll_d6		"syspll_d6"
#define syspll_d8		"syspll_d8"
#define syspll_d9		"syspll_d9"
#define syspll_d10		"syspll_d10"
#define syspll_d12		"syspll_d12"
#define syspll_d16		"syspll_d16"
#define syspll_d18		"syspll_d18"
#define syspll_d20		"syspll_d20"
#define syspll_d36		"syspll_d36"
#define clk27m_ck		"clk27m_ck"
#define clk27m_d2		"clk27m_d2"
#define clk27m_d4		"clk27m_d4"
#define clk27m_d8		"clk27m_d8"
#define clk27m_d16		"clk27m_d16"
#define clk27m_d512		"clk27m_d512"
#define clk27m_d1024	"clk27m_d1024"
#define clk27m_d2048	"clk27m_d2048"
#define usbpll_ck		"usbpll_ck"
#define usbpll_d2		"usbpll_d2"
#define usbpll_d4		"usbpll_d4"
#define usbpll_d6		"usbpll_d6"
#define usbpll_d8		"usbpll_d8"
#define usbpll_d10		"usbpll_d10"
#define usbpll_d15		"usbpll_d15"
#define usbpll_d20		"usbpll_d20"
#define armpll2_ck		"armpll2_ck"
#define armpll2_d2		"armpll2_d2"
#define armpll2_d4		"armpll2_d4"
#define adpll_324m_ck	"adpll_324m_ck"
#define adpll_324m_d2	"adpll_324m_d2"
#define adpll_108m_ck	"adpll_108m_ck"
#define adpll_108m_d2	"adpll_108m_d2"
#define adpll_108m_d3	"adpll_108m_d3"
#define adpll_648m_ck	"adpll_648m_ck"
#define rtc_ck			"rtc_ck"
#define msdcpll_ck		"msdcpll_ck"
#define msdcpll_d2		"msdcpll_d2"
#define msdcpll_d3		"msdcpll_d3"
#define msdcpll_d4		"msdcpll_d4"
#define vpll_ttl_ck		"vpll_ttl_ck"
#define vpll_ttl_d3		"vpll_ttl_d3"
#define vpll_ttl_d4		"vpll_ttl_d4"
#define vpll_ttl_d6		"vpll_ttl_d6"
#define vpll_ttl_d9		"vpll_ttl_d9"
#define vpll_ttl_d12	"vpll_ttl_d12"
#define vpll_ttl_d18	"vpll_ttl_d18"
#define aclk_k1			"aclk_k1"
#define aclk_k2			"aclk_k2"
#define aclk_k3			"aclk_k3"
#define aclk_k4			"aclk_k4"
#define aclk_k5			"aclk_k5"
#define aclk_k6			"aclk_k6"
#define aclk_k7			"aclk_k7"
#define aclk_k8			"aclk_k8"
#define aclk_k9			"aclk_k9"
#define aclk_k10		"aclk_k10"
#define aclk_k11		"aclk_k11"
#define aclk_k12		"aclk_k12"
#define aclk_k13		"aclk_k13"
#define aclk_k14		"aclk_k14"
#define hadds2_ck		"hadds2_ck"
#define g3dpll_ck		"g3dpll_ck"

/* DIVIDER */
#define riscclk_ratio		"riscclk_ratio"
#define axiclk_ratio		"axiclk_ratio"
#define aud_k1_ratio		"aud_k1_ratio"
#define aud_k2_ratio		"aud_k2_ratio"
#define aud_k3_ratio		"aud_k3_ratio"
#define aud_k4_ratio		"aud_k4_ratio"
#define aud_k5_ratio		"aud_k5_ratio"
#define aud_k6_ratio		"aud_k6_ratio"
#define aud_k7_ratio		"aud_k7_ratio"
#define aud_k8_ratio		"aud_k8_ratio"
#define aud_k9_ratio		"aud_k9_ratio"
#define aud_k10_ratio		"aud_k10_ratio"
#define aud_k11_ratio		"aud_k11_ratio"
#define aud_k12_ratio		"aud_k12_ratio"
#define aud_k13_ratio		"aud_k13_ratio"
#define aud_k14_ratio		"aud_k14_ratio"
#define aud_a1_ratio		"aud_a1_ratio"
#define aud_a2_ratio		"aud_a2_ratio"
#define aud_a3_ratio		"aud_a3_ratio"

/* TOP */
#define dram_slow_sel	"dram_slow_sel"
#define sd31_sel		"sd31_sel"
#define rfi1_sel		"rfi1_sel"
#define rfi2_sel		"rfi2_sel"
#define rfi3_sel		"rfi3_sel"
#define rfi4_sel		"rfi4_sel"
#define rfi5_sel		"rfi5_sel"
#define rfi6_sel		"rfi6_sel"
#define sd21_sel		"sd21_sel"
#define adsp_sel		"adsp_sel"
#define sd30_sel		"sd30_sel"
#define demux_sel		"demux_sel"
#define rsic_sel		"rsic_sel"
#define vdo_sel			"vdo_sel"
#define tp_sel			"tp_sel"
#define tp_f32_sel		"tp_f32_sel"
#define rfi_sel			"rfi_sel"
#define vdec1_sel		"vdec1_sel"
#define bclk_sel		"bclk_sel"
#define resz1_sel		"resz1_sel"
#define flash_sel		"flash_sel"
#define resz_sel		"resz_sel"
#define jpeg_sel		"jpeg_sel"
#define vdec_sel		"vdec_sel"
#define spm_sel			"spm_sel"
#define axim_sel		"axim_sel"
#define dram_sel		"dram_sel"
#define osd_sel			"osd_sel"
#define usb_27m_sel		"usb_27m_sel"
#define graph_sel		"graph_sel"
#define sd00_sel		"sd00_sel"
#define sd10_sel		"sd10_sel"
#define sd20_sel		"sd20_sel"
#define sd01_sel		"sd01_sel"
#define sd11_sel		"sd11_sel"
#define fpd_sel			"fpd_sel"
#define g3d_sel			"g3d_sel"
#define aud_sel			"aud_sel"
#define aud2_sel		"aud2_sel"
#define cpu1_sel		"cpu1_sel"
#define cpu2_sel		"cpu2_sel"
#define mphon_sel		"mphon_sel"
#define ssusb_xhci_sel	"ssusb_xhci_sel"
#define ssusb_sel		"ssusb_sel"
#define arm_aud_sel		"arm_aud_sel"
#define clk_nr_sel		"clk_nr_sel"
#define bt_mic_aud_sel	"bt_mic_aud_sel"
#define therm_slow_sel	"therm_slow_sel"
#define therm_sel		"therm_sel"
#define nflash_sel		"nflash_sel"
#define deg_sel			"deg_sel"
#define duty_sel		"duty_sel"
#define audio_k1_sel	"audio_k1_sel"
#define audio_k2_sel	"audio_k2_sel"
#define audio_k3_sel	"audio_k3_sel"
#define audio_k4_sel	"audio_k4_sel"
#define audio_k5_sel	"audio_k5_sel"
#define audio_k6_sel	"audio_k6_sel"
#define audio_k7_sel	"audio_k7_sel"
#define audio_k8_sel	"audio_k8_sel"
#define audio_k9_sel	"audio_k9_sel"
#define audio_k10_sel	"audio_k10_sel"
#define audio_k11_sel	"audio_k11_sel"
#define audio_k12_sel	"audio_k12_sel"
#define audio_k13_sel	"audio_k13_sel"
#define audio_k14_sel	"audio_k14_sel"
#define audio_a1_sel	"audio_a1_sel"
#define audio_a2_sel	"audio_a2_sel"
#define audio_a3_sel	"audio_a3_sel"
#define pll_test_sel	"pll_test_sel"
#define aud_adc_sel		"aud_adc_sel"
#define aud_pwm_sel		"aud_pwm_sel"
#define mlin2_sel		"mlin2_sel"
#define aud_mph_sel		"aud_mph_sel"
#define mlin_sel		"mlin_sel"
#define ts0_sel			"ts0_sel"
#define ts1_sel			"ts1_sel"
#define png_sel			"png_sel"
#define spi_moto_sel	"spi_moto_sel"
#define tvd_mbist_sel	"tvd_mbist_sel"
#define sramif_sel		"sramif_sel"
#define mclk_div2_sel	"mclk_div2_sel"
#define da_apllck_sel	"da_apllck_sel"
#define da_apllck1_sel	"da_apllck1_sel"
#define lvds_sel		"lvds_sel"
#define twds_sel		"twds_sel"
#define pwm0_sel		"pwm0_sel"
#define pwm1_sel		"pwm1_sel"
#define pwm2_sel		"pwm2_sel"
#define pwm3_sel		"pwm3_sel"
#define sifm0_sel		"sifm0_sel"
#define sifm1_sel		"sifm1_sel"
#define sifs0_sel		"sifs0_sel"
#define sifs1_sel		"sifs1_sel"
#define osd1_sel		"osd1_sel"
#define osd2_sel		"osd2_sel"
#define osd3_sel		"osd3_sel"
#define osd4_sel		"osd4_sel"
#define osd_main_sel	"osd_main_sel"
#define osd_aux_sel		"osd_aux_sel"
#define vdt_ft_sel		"vdt_ft_sel"
#define aud_a1_tst_sel		"aud_a1_tst_sel"
#define aud_a2_tst_sel		"aud_a2_tst_sel"

/* other */
#define aclk_in				"aclk_in"
#define i2s_out2_mclk		"i2s_out2_mclk"
#define mphon_in			"mphon_in"
#define syspll_d1500		"syspll_d1500"
#define asim_ck				"asim_ck"
#define pre_usb_ck			"pre_usb_ck"
#define ad_ttl_ck_d3		"ad_ttl_ck_d3"
#define ad_ttl_ck_d4		"ad_ttl_ck_d4"
#define ad_ttl_ck_d6		"ad_ttl_ck_d6"
#define ad_ttl_ck_d9		"ad_ttl_ck_d9"
#define ad_ttl_ck_d12		"ad_ttl_ck_d12"
#define ad_ttl_ck_d18		"ad_ttl_ck_d18"
#define i2s_out0_mclk_int_in		"i2s_out0_mclk_int_in"
#define i2s_out1_mclk_int_in		"i2s_out1_mclk_int_in"
#define sd_108m_ck			"sd_108m_ck"
#define panel_ck			"panel_ck"
#define normal_ck			"normal_ck"
#define ack_in				"ack_in"
#define spmclk_in			"spmclk_in"
#define ttl_ck				"ttl_ck"
#define lvds_dpix_ck		"lvds_dpix_ck"
#define twds_dpix_ck		"twds_dpix_ck"
#define spmck_in			"spmck_in"
#define spmck2_in			"spmck2_in"
#define demux_ck			"demux_ck"
#define ext_ts0_clk			"ext_ts0_clk"
#define ts_out				"ts_out"
#define ext_ts1_clk			"ext_ts1_clk"
#define cvsb_adc_ck			"cvsb_adc_ck"
#define ack_k6				"ack_k6"
#define ack_k8				"ack_k8"
#define ack_k9				"ack_k9"
#define bt_mclk_in			"bt_mclk_in"
#define test_in_0			"test_in_0"
#define aclk_a1				"aclk_a1"
#define aclk_a2				"aclk_a2"

/* VDEC */
#define vdec_full					"vdec_full"

/* IMG DRAMC */
#define img_dramc_gfx				"img_dramc_gfx"
#define img_dramc_dmarb				"img_dramc_dmarb"
#define img_dramc_png				"img_dramc_png"
#define img_dramc_gif				"img_dramc_gif"
#define img_dramc_img_resz			"img_dramc_img_resz"
#define img_dramc_osd_resz			"img_dramc_osd_resz"
#define img_dramc_jpgdec			"img_dramc_jpgdec"
#define img_dramc_demux				"img_dramc_demux"
#define img_dramc_demux_ts0			"img_dramc_demux_ts0"
#define img_dramc_demux_ts1			"img_dramc_demux_ts1"
#define img_dramc_demux_27m			"img_dramc_demux_27m"
#define img_dramc_sub_img_resz		"img_dramc_sub_img_resz"
#define img_dramc_sub_osd_resz		"img_dramc_sub_osd_resz"
#define img_dramc_usb_port0			"img_dramc_usb_port0"
#define img_dramc_usb_port1			"img_dramc_usb_port1"
#define img_dramc_therm				"img_dramc_therm"
#define img_dramc_therm_slow		"img_dramc_therm_slow"
#define img_dramc_irt_dma_wrapper	"img_dramc_irt_dma_wrapper"
#define img_dramc_ssusb_xhci		"img_dramc_ssusb_xhci"
#define img_dramc_ssusb				"img_dramc_ssusb"
#define img_dramc_nfi				"img_dramc_nfi"
#define img_dramc_arm9				"img_dramc_arm9"

/* AUDIO */
#define audio_peri_b00				"audio_peri_b00"
#define audio_peri_b01				"audio_peri_b01"
#define audio_peri_b02				"audio_peri_b02"
#define audio_peri_b03				"audio_peri_b03"
#define audio_peri_b04				"audio_peri_b04"
#define audio_peri_b05				"audio_peri_b05"
#define audio_peri_b06				"audio_peri_b06"
#define audio_peri_b07				"audio_peri_b07"
#define audio_peri_b08				"audio_peri_b08"
#define audio_peri_b09				"audio_peri_b09"
#define audio_peri_b10				"audio_peri_b10"
#define audio_peri_b11				"audio_peri_b11"
#define audio_peri_b12				"audio_peri_b12"
#define audio_peri_b13				"audio_peri_b13"
#define audio_peri_b14				"audio_peri_b14"
#define audio_peri_rfi_top			"audio_peri_rfi_top"
#define audio_peri_msdc_pd0			"audio_peri_msdc_pd0"
#define audio_peri_msdc_pd1			"audio_peri_msdc_pd1"
#define audio_peri_msdc_pd2			"audio_peri_msdc_pd2"
#define audio_peri_msdc_sw0			"audio_peri_msdc_sw0"
#define audio_peri_msdc_sw1			"audio_peri_msdc_sw1"
#define audio_peri_msdc_sw2			"audio_peri_msdc_sw2"
#define audio_peri_spi_moto1		"audio_peri_spi_moto1"
#define audio_peri_spi_moto2		"audio_peri_spi_moto2"
#define audio_peri_pwmx6_0			"audio_peri_pwmx6_0"
#define audio_peri_pwmx6_1			"audio_peri_pwmx6_1"
#define audio_peri_pwmx6_2			"audio_peri_pwmx6_2"
#define audio_peri_pwmx6_3			"audio_peri_pwmx6_3"
#define audio_peri_sifm0			"audio_peri_sifm0"
#define audio_peri_sifm1			"audio_peri_sifm1"
#define audio_peri_sifs0			"audio_peri_sifs0"
#define audio_peri_sifs1			"audio_peri_sifs1"

/* MISC */
#define misc_msdc3					"misc_msdc3"
#define misc_msdc3_sresetb			"misc_msdc3_sresetb"
#define misc_vdec1					"misc_vdec1"
#define misc_vdec2					"misc_vdec2"
#define misc_mfg_top_pwr_wrap		"misc_mfg_top_pwr_wrap"

/* LVDS */
#define lvds_clk					"lvds_clk"
#define lvds_tp_top0				"lvds_tp_top0"
#define lvds_tp_top1				"lvds_tp_top1"
#define lvds_tp_top2				"lvds_tp_top2"
#define lvds_rfi_top1				"lvds_rfi_top1"
#define lvds_rfi_top2				"lvds_rfi_top2"
#define lvds_rfi_top3				"lvds_rfi_top3"
#define lvds_rfi_top4				"lvds_rfi_top4"
#define lvds_rfi_top5				"lvds_rfi_top5"
#define lvds_rfi_top6				"lvds_rfi_top6"

/* VDOUT */
#define vdout_scler					"vdout_scler"
#define vdout_tvd1					"vdout_tvd1"
#define vdout_tvd2					"vdout_tvd2"
#define vdout_osd					"vdout_osd"
#define vdout_osd_r					"vdout_osd_r"
#define vdout_fpd					"vdout_fpd"
#define vdout_fmt_vdo_f				"vdout_fmt_vdo_f"
#define vdout_fmt_vdo_r				"vdout_fmt_vdo_r"
#define vdout_write_chanel			"vdout_write_chanel"
#define vdout_frame_lock			"vdout_frame_lock"
#define vdout_write_chanel2			"vdout_write_chanel2"
#define vdout_vga					"vdout_vga"
#define vdout_ypbpr					"vdout_ypbpr"
#define vdout_hdmi					"vdout_hdmi"
#define vdout_tve					"vdout_tve"
#define vdout_dvd_mix_2ap			"vdout_dvd_mix_2ap"
#define vdout_osd1					"vdout_osd1"
#define vdout_osd2					"vdout_osd2"
#define vdout_osd3					"vdout_osd3"
#define vdout_osd4					"vdout_osd4"
#define vdout_osd5					"vdout_osd5"
#define vdout_osd_r_2				"vdout_osd_r_2"
#define vdout_osd_r_3				"vdout_osd_r_3"
#define vdout_scler_tg				"vdout_scler_tg"
#define vdout_lcproc_vdo			"vdout_lcproc_vdo"
#define vdout_wr_channel_tvd4		"vdout_wr_channel_tvd4"
#define vdout_lcproc_vdo_r			"vdout_lcproc_vdo_r"
#define vdout_wr_channel_hdmirx		"vdout_wr_channel_hdmirx"
#define vdout_wr_channel_dgi		"vdout_wr_channel_dgi"
#define vdout_wr_channel_vga		"vdout_wr_channel_vga"
#define vdout_wr_channel_vdout		"vdout_wr_channel_vdout"

/* DIVIDER */

struct atc_divider {
	int id;
	const char *name;
	const char *parent_name;
	unsigned int reg;
	unsigned int shift;
	unsigned int width;
};

#define DIVIDER(_id, _name, _parent, _reg, _shift, _width) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.reg = _reg,			\
		.shift = _shift,		\
		.width = _width,		\
	}

static struct atc_divider sub_divs[] __initdata = {
	DIVIDER(TOP_RISCCLK_RATIO, riscclk_ratio, clk27m, 0x04, 8, 3),
	DIVIDER(TOP_AXICLK_RATIO, axiclk_ratio, clk27m, 0x04, 0, 3),
	DIVIDER(TOP_AUD_K1_RATIO, aud_k1_ratio, clk27m, 0x1C, 0, 8),
	DIVIDER(TOP_AUD_K2_RATIO, aud_k2_ratio, clk27m, 0x1C, 8, 8),
	DIVIDER(TOP_AUD_K3_RATIO, aud_k3_ratio, clk27m, 0x1C, 16, 8),
	DIVIDER(TOP_AUD_K4_RATIO, aud_k4_ratio, clk27m, 0x1C, 24, 8),
	DIVIDER(TOP_AUD_K5_RATIO, aud_k5_ratio, clk27m, 0x20, 0, 8),
	DIVIDER(TOP_AUD_K6_RATIO, aud_k6_ratio, clk27m, 0x20, 8, 8),
	DIVIDER(TOP_AUD_K7_RATIO, aud_k7_ratio, clk27m, 0x20, 16, 8),
	DIVIDER(TOP_AUD_K8_RATIO, aud_k8_ratio, clk27m, 0x20, 24, 8),
	DIVIDER(TOP_AUD_K9_RATIO, aud_k9_ratio, clk27m, 0x24, 0, 12),
	DIVIDER(TOP_AUD_K10_RATIO, aud_k10_ratio, clk27m, 0x0C, 16, 8),
	DIVIDER(TOP_AUD_K11_RATIO, aud_k11_ratio, clk27m, 0x24, 16, 8),
	DIVIDER(TOP_AUD_K12_RATIO, aud_k12_ratio, clk27m, 0x24, 24, 8),
	DIVIDER(TOP_AUD_K13_RATIO, aud_k13_ratio, clk27m, 0x28, 0, 8),
	DIVIDER(TOP_AUD_K14_RATIO, aud_k14_ratio, clk27m, 0x28, 8, 8),
	DIVIDER(TOP_AUD_A1_RATIO, aud_a1_ratio, clk27m, 0x24, 12, 1),
	DIVIDER(TOP_AUD_A2_RATIO, aud_a2_ratio, clk27m, 0x24, 13, 1),
	DIVIDER(TOP_AUD_A3_RATIO, aud_a3_ratio, clk27m, 0x24, 14, 1),
};

static void __init init_clk_divider(void __iomem *top_base, struct clk_onecell_data *clk_data)
{
	int i;
	struct clk *clk;
	int ret = 0;

	for (i = 0; i < ARRAY_SIZE(sub_divs); i++) {
		struct atc_divider *divider = &sub_divs[i];

		clk = atc_clk_register_divider(divider->name,
					       divider->parent_name,
					       top_base + divider->reg,
					       divider->shift,
					       divider->width);

		if (IS_ERR(clk)) {
			pr_err("Failed to register clk %s: %ld\n", divider->name, PTR_ERR(clk));
			continue;
		}

		ret = clk_register_clkdev(clk, divider->name, NULL);

		if (ret) {
			pr_err("Failed to register clkdev %s\n", divider->name);
		}

		if (clk_data) {
			clk_data->clks[divider->id] = clk;
		}
	}
}


/* FACTOR */

struct atc_fixed_factor {
	int id;
	const char *name;
	const char *parent_name;
	int mult;
	int div;
};

#define FACTOR(_id, _name, _parent, _mult, _div) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.mult = _mult,				\
		.div = _div,				\
	}

static void __init init_factors(struct atc_fixed_factor *clks, int num,
				struct clk_onecell_data *clk_data)
{
	int i;
	struct clk *clk;
	int ret = 0;

	for (i = 0; i < num; i++) {
		struct atc_fixed_factor *ff = &clks[i];

		clk = clk_register_fixed_factor(NULL, ff->name, ff->parent_name,
						0, ff->mult, ff->div);

		if (IS_ERR(clk)) {
			pr_err("Failed to register clk %s: %ld\n",
			       ff->name, PTR_ERR(clk));
			continue;
		}

		ret = clk_register_clkdev(clk, ff->name, NULL);

		if (ret) {
			pr_err("Failed to register clkdev %s\n", ff->name);
		}

		if (clk_data) {
			clk_data->clks[ff->id] = clk;
		}
	}
}

static struct atc_fixed_factor top_divs[] __initdata = {
	FACTOR(TOP_DMPLL_CK, dmpll_ck, mmpll, 1, 1),
	FACTOR(TOP_DMPLL_D2, dmpll_d2, mmpll, 1, 2),
	FACTOR(TOP_DMPLL_D3, dmpll_d3, mmpll, 1, 3),
	FACTOR(TOP_DMPLL_D4, dmpll_d4, mmpll, 1, 4),
	FACTOR(TOP_DMPLL_D6, dmpll_d6, mmpll, 1, 6),
	FACTOR(TOP_DMPLL_D8, dmpll_d8, mmpll, 1, 8),
	FACTOR(TOP_DMPLL_D10, dmpll_d10, mmpll, 1, 10),
	FACTOR(TOP_ARMPLL_CK, armpll_ck, armpll, 1, 1),
	FACTOR(TOP_ARMPLL_D2, armpll_d2, armpll, 1, 2),
	FACTOR(TOP_ARMPLL_D3, armpll_d3, armpll, 1, 3),
	FACTOR(TOP_ARMPLL_D4, armpll_d4, armpll, 1, 4),
	FACTOR(TOP_ARMPLL_D6, armpll_d6, armpll, 1, 6),
	FACTOR(TOP_ARMPLL_D8, armpll_d8, armpll, 1, 8),
	FACTOR(TOP_ARMPLL_D16, armpll_d16, armpll, 1, 16),
	FACTOR(TOP_APLL1_CK, apll1_ck, apll1, 1, 1),
	FACTOR(TOP_APLL1_D2, apll1_d2, apll1, 1, 2),
	FACTOR(TOP_APLL1_D4, apll1_d4, apll1, 1, 4),
	FACTOR(TOP_APLL2_CK, apll2_ck, apll2, 1, 1),
	FACTOR(TOP_APLL2_D2, apll2_d2, apll2, 1, 2),
	FACTOR(TOP_APLL2_D3, apll2_d3, apll2, 1, 3),
	FACTOR(TOP_APLL2_D4, apll2_d4, apll2, 1, 4),
	FACTOR(TOP_APLL3_CK, apll3_ck, apll3, 1, 1),
	FACTOR(TOP_SYSPLL_CK, syspll_ck, syspll, 1, 1),
	FACTOR(TOP_SYSPLL_D2, syspll_d2, syspll, 1, 2),
	FACTOR(TOP_SYSPLL_D3, syspll_d3, syspll, 1, 3),
	FACTOR(TOP_SYSPLL_D4, syspll_d4, syspll, 1, 4),
	FACTOR(TOP_SYSPLL_D6, syspll_d6, syspll, 1, 6),
	FACTOR(TOP_SYSPLL_D8, syspll_d8, syspll, 1, 8),
	FACTOR(TOP_SYSPLL_D9, syspll_d9, syspll, 1, 9),
	FACTOR(TOP_SYSPLL_D10, syspll_d10, syspll, 1, 10),
	FACTOR(TOP_SYSPLL_D12, syspll_d12, syspll, 1, 12),
	FACTOR(TOP_SYSPLL_D16, syspll_d16, syspll, 1, 16),
	FACTOR(TOP_SYSPLL_D18, syspll_d18, syspll, 1, 18),
	FACTOR(TOP_SYSPLL_D20, syspll_d20, syspll, 1, 20),
	FACTOR(TOP_SYSPLL_D36, syspll_d36, syspll, 1, 36),
	FACTOR(TOP_CLK27M_CK, clk27m_ck, clk27m, 1, 1),
	FACTOR(TOP_CLK27M_D2, clk27m_d2, clk27m, 1, 2),
	FACTOR(TOP_CLK27M_D4, clk27m_d4, clk27m, 1, 4),
	FACTOR(TOP_CLK27M_D8, clk27m_d8, clk27m, 1, 8),
	FACTOR(TOP_CLK27M_D16, clk27m_d16, clk27m, 1, 16),
	FACTOR(TOP_USBPLL_CK, usbpll_ck, usbpll, 1, 1),
	FACTOR(TOP_USBPLL_D2, usbpll_d2, usbpll, 1, 2),
	FACTOR(TOP_USBPLL_D4, usbpll_d4, usbpll, 1, 4),
	FACTOR(TOP_USBPLL_D6, usbpll_d6, usbpll, 1, 6),
	FACTOR(TOP_USBPLL_D8, usbpll_d8, usbpll, 1, 8),
	FACTOR(TOP_USBPLL_D10, usbpll_d10, usbpll, 1, 10),
	FACTOR(TOP_USBPLL_D15, usbpll_d15, usbpll, 1, 15),
	FACTOR(TOP_USBPLL_D20, usbpll_d20, usbpll, 1, 20),
	FACTOR(TOP_ARMPLL2_CK, armpll2_ck, armpll2, 1, 1),
	FACTOR(TOP_ARMPLL2_D2, armpll2_d2, armpll2, 1, 2),
	FACTOR(TOP_ARMPLL2_D4, armpll2_d4, armpll2, 1, 4),
	FACTOR(TOP_ADPLL_324M_CK, adpll_324m_ck, adpll_324m, 1, 1),
	FACTOR(TOP_ADPLL_324M_D2, adpll_324m_d2, adpll_324m, 1, 2),
	FACTOR(TOP_ADPLL_108M_CK, adpll_108m_ck, adpll_108m, 1, 1),
	FACTOR(TOP_ADPLL_108M_D2, adpll_108m_d2, adpll_108m, 1, 2),
	FACTOR(TOP_ADPLL_108M_D3, adpll_108m_d3, adpll_108m, 1, 3),
	FACTOR(TOP_ADPLL_648M_CK, adpll_648m_ck, adpll_648m, 1, 1),
	FACTOR(TOP_RTC_CK, rtc_ck, clk32k, 1, 1),
	FACTOR(TOP_VPLL_TTL_CK, vpll_ttl_ck, vpll_ttl, 1, 1),
	FACTOR(TOP_VPLL_TTL_D3, vpll_ttl_d3, vpll_ttl, 1, 3),
	FACTOR(TOP_VPLL_TTL_D4, vpll_ttl_d4, vpll_ttl, 1, 4),
	FACTOR(TOP_VPLL_TTL_D6, vpll_ttl_d6, vpll_ttl, 1, 6),
	FACTOR(TOP_VPLL_TTL_D9, vpll_ttl_d9, vpll_ttl, 1, 9),
	FACTOR(TOP_VPLL_TTL_D12, vpll_ttl_d12, vpll_ttl, 1, 12),
	FACTOR(TOP_VPLL_TTL_D18, vpll_ttl_d18, vpll_ttl, 1, 18),
	FACTOR(TOP_MSDCPLL_CK, msdcpll_ck, msdcpll, 1, 1),
	FACTOR(TOP_MSDCPLL_D2, msdcpll_d2, msdcpll, 1, 2),
	FACTOR(TOP_MSDCPLL_D3, msdcpll_d3, msdcpll, 1, 3),
	FACTOR(TOP_MSDCPLL_D4, msdcpll_d4, msdcpll, 1, 4),
	FACTOR(TOP_ACLK_K1, aclk_k1, aclk, 1, 1),
	FACTOR(TOP_ACLK_K2, aclk_k2, aclk, 1, 2),
	FACTOR(TOP_ACLK_K3, aclk_k3, aclk, 1, 3),
	FACTOR(TOP_ACLK_K4, aclk_k4, aclk, 1, 4),
	FACTOR(TOP_ACLK_K5, aclk_k5, aclk, 1, 5),
	FACTOR(TOP_ACLK_K6, aclk_k6, aclk, 1, 6),
	FACTOR(TOP_ACLK_K7, aclk_k7, aclk, 1, 7),
	FACTOR(TOP_ACLK_K8, aclk_k8, aclk, 1, 8),
	FACTOR(TOP_ACLK_K9, aclk_k9, aclk, 1, 9),
	FACTOR(TOP_ACLK_K10, aclk_k10, aclk, 1, 10),
	FACTOR(TOP_ACLK_K11, aclk_k11, aclk, 1, 11),
	FACTOR(TOP_ACLK_K12, aclk_k12, aclk, 1, 12),
	FACTOR(TOP_ACLK_K13, aclk_k13, aclk, 1, 13),
	FACTOR(TOP_ACLK_K14, aclk_k14, aclk, 1, 14),
	FACTOR(TOP_ACLK_IN, aclk_in, aclk, 1, 1),
	FACTOR(TOP_I2S_OUT2_MCLK, i2s_out2_mclk, clk27m, 1, 1),
	FACTOR(TOP_MPHON_IN, mphon_in, clk27m, 1, 1),
	FACTOR(TOP_SYSPLL_D1500, syspll_d1500, clk27m, 1, 1),
	FACTOR(TOP_ASIM_CK, asim_ck, clk27m, 1, 1),
	FACTOR(TOP_PRE_USB_CK, pre_usb_ck, clk27m, 1, 1),
	FACTOR(TOP_AD_TTL_CK_D3, ad_ttl_ck_d3, clk27m, 1, 1),
	FACTOR(TOP_AD_TTL_CK_D4, ad_ttl_ck_d4, clk27m, 1, 1),
	FACTOR(TOP_AD_TTL_CK_D6, ad_ttl_ck_d6, clk27m, 1, 1),
	FACTOR(TOP_AD_TTL_CK_D9, ad_ttl_ck_d9, clk27m, 1, 1),
	FACTOR(TOP_AD_TTL_CK_D12, ad_ttl_ck_d12, clk27m, 1, 1),
	FACTOR(TOP_AD_TTL_CK_D18, ad_ttl_ck_d18, clk27m, 1, 1),
	FACTOR(TOP_I2S_OUT0_MCLK_INT_IN, i2s_out0_mclk_int_in, clk27m, 1, 1),
	FACTOR(TOP_I2S_OUT1_MCLK_INT_IN, i2s_out1_mclk_int_in, clk27m, 1, 1),
	FACTOR(TOP_SD_108M_CK, sd_108m_ck, clk27m, 1, 1),
	FACTOR(TOP_PANEL_CK, panel_ck, clk27m, 1, 1),
	FACTOR(TOP_NORMAL_CK, normal_ck, clk27m, 1, 1),
	FACTOR(TOP_ACK_IN, ack_in, clk27m, 1, 1),
	FACTOR(TOP_SPMCLK_IN, spmclk_in, clk27m, 1, 1),
	FACTOR(TOP_TTL_CK, ttl_ck, clk27m, 1, 1),
	FACTOR(TOP_LVDS_DPIX_CK, lvds_dpix_ck, clk27m, 1, 1),
	FACTOR(TOP_TWDS_DPIX_CK, twds_dpix_ck, clk27m, 1, 1),
	FACTOR(TOP_SPMCK_IN, spmck_in, clk27m, 1, 1),
	FACTOR(TOP_SPMCK2_IN, spmck2_in, clk27m, 1, 1),
	FACTOR(TOP_DEMUX_CK, demux_ck, clk27m, 1, 1),
	FACTOR(TOP_EXT_TS0_CLK, ext_ts0_clk, clk27m, 1, 1),
	FACTOR(TOP_TS_OUT, ts_out, clk27m, 1, 1),
	FACTOR(TOP_EXT_TS1_CLK, ext_ts1_clk, clk27m, 1, 1),
	FACTOR(TOP_CVSB_ADC_CK, cvsb_adc_ck, clk27m, 1, 1),
	FACTOR(TOP_ACK_K6, ack_k6, clk27m, 1, 1),
	FACTOR(TOP_ACK_K8, ack_k8, clk27m, 1, 1),
	FACTOR(TOP_ACK_K9, ack_k9, clk27m, 1, 1),
	FACTOR(TOP_BT_MCLK_IN, bt_mclk_in, clk27m, 1, 1),
	FACTOR(TOP_TEST_IN_0, test_in_0, clk27m, 1, 1),
	FACTOR(TOP_ACLK_A1, aclk_a1, clk27m, 1, 1),
	FACTOR(TOP_ACLK_A2, aclk_a2, clk27m, 1, 1),
	FACTOR(TOP_G3DPLL_CK, g3dpll_ck, g3dpll, 1, 1),
	FACTOR(TOP_CLK27M_D512, clk27m_d512, clk27m, 1, 512),
	FACTOR(TOP_CLK27M_D1024, clk27m_d1024, clk27m, 1, 1024),
	FACTOR(TOP_CLK27M_D2048, clk27m_d2048, clk27m, 1, 2048),
};

static void __init init_clk_top_div(struct clk_onecell_data *clk_data)
{
	init_factors(top_divs, ARRAY_SIZE(top_divs), clk_data);
}


/* MUX */

static const char *dram_slow_parents[] __initconst = {
	clk27m_ck,
	dmpll_d2,
	apll1_ck,
	syspll_d3,
	syspll_d4,
	adpll_108m_ck,
	dmpll_d3,
	dmpll_d4
};

static const char *sd31_parents[] __initconst = {
	clk27m_ck,
	msdcpll_d2,
	armpll2_d2,
	syspll_d4,
	usbpll_d4,
	syspll_d6,
	syspll_d12,
	usbpll_d10,
	dmpll_d2,
	apll2_d2,
	apll2_d3,
	apll1_d2,
	msdcpll_d3,
	msdcpll_d4
};

static const char *rfi1_parents[] __initconst = {
	clk27m_ck,
	syspll_d20,
	apll3_ck,
	usbpll_d15
};

static const char *rfi2_parents[] __initconst = {
	clk27m_ck,
	syspll_d20,
	apll3_ck,
	usbpll_d15
};

static const char *rfi3_parents[] __initconst = {
	clk27m_ck,
	syspll_d20,
	apll3_ck,
	usbpll_d15
};

static const char *rfi4_parents[] __initconst = {
	clk27m_ck,
	syspll_d20,
	apll3_ck,
	usbpll_d15
};

static const char *rfi5_parents[] __initconst = {
	clk27m_ck,
	syspll_d20,
	apll3_ck,
	usbpll_d15
};

#if 0
static const char *rfi6_parents[] __initconst = {
	clk27m_ck,
	syspll_d20,
	apll3_ck,
	usbpll_d15
};
#endif

static const char *sd21_parents[] __initconst = {
	clk27m_ck,
	msdcpll_d2,
	armpll2_d2,
	syspll_d4,
	usbpll_d4,
	syspll_d6,
	syspll_d12,
	usbpll_d10,
	dmpll_d2,
	apll2_d2,
	apll2_d4,
	apll1_d2,
	syspll_d8,
	msdcpll_d4
};

static const char *adsp_parents[] __initconst = {
	clk27m_ck,
	syspll_d3,
	armpll2_ck,
	usbpll_d2,
	apll2_ck,
	dmpll_ck,
	syspll_d2,
	syspll_d4
};

static const char *sd30_parents[] __initconst = {
	clk27m_ck,
	apll2_d3,
	usbpll_d6,
	syspll_d9,
	usbpll_d8,
	syspll_d12,
	usbpll_d10,
	syspll_d18
};

static const char *demux_parents[] __initconst = {
	clk27m_ck,
	apll2_d2,
	armpll_d4,
	syspll_d6,
	apll1_d2,
	syspll_d3,
	syspll_d4,
	dmpll_d2
};

// zplee
static const char *rsic_parents[] __initconst = {
	syspll_d12,
	clk27m_ck
};

static const char *vdo_parents[] __initconst = {
	syspll_d12,
	clk27m_ck
};

static const char *tp_parents[] __initconst = {
	syspll_d12,
	clk27m_ck,
	mphon_in
};

static const char *tp_f32_parents[] __initconst = {
	syspll_d1500,
	rtc_ck
};

static const char *rfi_parents[] __initconst = {
	clk27m_ck,
	syspll_d20,
	apll3_ck,
	usbpll_d15
};

static const char *vdec1_parents[] __initconst = {
	clk27m_ck,
	syspll_d2,
	apll2_ck,
	apll1_ck,
	usbpll_d2,
	syspll_d3,
	dmpll_d2,
	syspll_d4
};

static const char *bclk_parents[] __initconst = {
	clk27m_ck,
	syspll_d3,
	syspll_d4,
	syspll_d6,
	syspll_d9,
	armpll2_d2
};

static const char *resz1_parents[] __initconst = {
	clk27m_ck,
	apll1_ck,
	apll2_d2,
	dmpll_ck,
	syspll_d3,
	syspll_d2,
	usbpll_d2,
	apll2_ck
};

static const char *flash_parents[] __initconst = {
	clk27m_d8,
	clk27m_d4,
	clk27m_d2,
	clk27m_ck,
	syspll_d9,
	syspll_d12,
	syspll_d16,
	syspll_d18
};

static const char *resz_parents[] __initconst = {
	clk27m_ck,
	apll1_ck,
	apll2_d2,
	dmpll_ck,
	syspll_d3,
	syspll_d2,
	usbpll_d2,
	apll2_ck
};

static const char *jpeg_parents[] __initconst = {
	clk27m_ck,
	apll1_ck,
	apll2_d2,
	syspll_d6,
	syspll_d3,
	syspll_d2,
	usbpll_d2,
	apll2_ck
};

static const char *vdec_parents[] __initconst = {
	clk27m_ck,
	syspll_d2,
	apll2_ck,
	apll1_ck,
	usbpll_d2,
	syspll_d3,
	dmpll_d2,
	syspll_d4
};

static const char *spm_parents[] __initconst = {
	clk27m_ck,
	syspll_d6,
	dmpll_d2
};

static const char *axim_parents[] __initconst = {
	clk27m_ck,
	syspll_d2,
	syspll_d3,
	syspll_d6,
	armpll_d2
};

static const char *dram_parents[] __initconst = {
	asim_ck,
	dmpll_ck
};

static const char *osd_parents[] __initconst = {
	clk27m_ck,
	syspll_d10,
	syspll_d9,
	syspll_d8,
	syspll_d6,
	apll2_d2,
	syspll_d4,
	syspll_d3
};

static const char *usb_27m_parents[] __initconst = {
	pre_usb_ck,
	clk27m_ck
};

static const char *graph_parents[] __initconst = {
	clk27m_ck,
	apll1_ck,
	syspll_d2,
	syspll_d3,
	usbpll_d2,
	apll2_ck,
	dmpll_ck
};

static const char *sd00_parents[] __initconst = {
	clk27m_ck,
	apll2_d3,
	usbpll_d6,
	syspll_d9,
	usbpll_d8,
	syspll_d12,
	usbpll_d10,
	syspll_d18
};

static const char *sd10_parents[] __initconst = {
	clk27m_ck,
	apll2_d3,
	usbpll_d6,
	syspll_d9,
	usbpll_d8,
	syspll_d12,
	usbpll_d10,
	syspll_d18
};

static const char *sd20_parents[] __initconst = {
	clk27m_ck,
	apll2_d3,
	usbpll_d6,
	syspll_d9,
	usbpll_d8,
	syspll_d12,
	usbpll_d10,
	syspll_d18
};

static const char *sd01_parents[] __initconst = {
	clk27m_ck,
	msdcpll_ck,
	armpll2_d2,
	syspll_d4,
	usbpll_d4,
	syspll_d6,
	syspll_d12,
	usbpll_d10,
	dmpll_d2,
	apll2_d2,
	apll2_d3,
	apll1_d2,
	msdcpll_d3,
	msdcpll_d4
};

static const char *sd11_parents[] __initconst = {
	clk27m_ck,
	msdcpll_d2,
	armpll2_d2,
	syspll_d4,
	usbpll_d4,
	syspll_d6,
	syspll_d12,
	usbpll_d10,
	dmpll_d2,
	apll2_d2,
	apll2_d3,
	apll1_d2,
	msdcpll_d3,
	msdcpll_d4
};

#if 0
static const char *fpd_parents[] __initconst = {
	clk27m_ck,
	ad_ttl_ck_d3,
	ad_ttl_ck_d4,
	ad_ttl_ck_d6,
	ad_ttl_ck_d9,
	ad_ttl_ck_d12,
	ad_ttl_ck_d18,
	mphon_in
};
#endif

static const char *g3d_parents[] __initconst = {
	clk27m_ck,
	g3dpll_ck,
	syspll_d2,
	apll2_ck,
	apll1_ck,
	syspll_d3,
	syspll_d4,
	syspll_d6
};

static const char *aud_parents[] __initconst = {
	clk27m_ck,
	aclk_k2,
	i2s_out0_mclk_int_in,
	i2s_out1_mclk_int_in
};

static const char *aud2_parents[] __initconst = {
	clk27m_ck,
	aclk_k4,
	i2s_out0_mclk_int_in,
	i2s_out1_mclk_int_in
};

static const char *cpu1_parents[] __initconst = {
	clk27m_ck,
	armpll_ck,
	syspll_ck,
	dmpll_ck
};

static const char *cpu2_parents[] __initconst = {
	clk27m_ck,
	armpll2_ck,
	syspll_d2,
	armpll_d2
};

static const char *mphon_parents[] __initconst = {
	clk27m_ck,
	ack_k6,
	mphon_in,
	spmclk_in
};

static const char *ssusb_xhci_parents[] __initconst = {
	clk27m_ck,
	syspll_d6,
	usbpll_d4,
	syspll_d9
};

static const char *ssusb_parents[] __initconst = {
	clk27m_ck,
	syspll_d6,
	usbpll_d4,
	syspll_d9
};

static const char *arm_aud_parents[] __initconst = {
	clk27m_ck,
	ack_k8,
	aclk_in,
	mphon_in
};

static const char *clk_nr_parents[] __initconst = {
	clk27m_ck,
	syspll_d10,
	syspll_d8,
	syspll_d6,
	apll2_d2,
	syspll_d4,
	syspll_d12,
	syspll_d16
};

static const char *bt_mic_aud_parents[] __initconst = {
	clk27m_ck,
	ack_k9,
	bt_mclk_in,
	mphon_in
};

static const char *therm_slow_parents[] __initconst = {
	clk27m_ck,
	clk27m_d512,
	clk27m_d1024,
	clk27m_d2048
};

static const char *therm_parents[] __initconst = {
	clk27m_ck,
	usbpll_d8,
	syspll_d12,
	syspll_d16
};

static const char *nflash_parents[] __initconst = {
	clk27m_ck,
	apll2_d2,
	apll1_d2,
	syspll_d4,
	armpll_d4,
	syspll_d6
};

static const char *deg_parents[] __initconst = {
	clk27m_ck,
	clk27m_d16,
	clk27m_d4,
	clk27m_d2
};

static const char *duty_parents[] __initconst = {
	clk27m_ck,
	clk27m_d2,
	clk27m_d4,
	clk27m_d8
};

static const char *audio_k1_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k2_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k3_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k4_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k5_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k6_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k7_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k8_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k9_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k10_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k11_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k12_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k13_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_k14_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_a1_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_a2_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *audio_a3_parents[] __initconst = {
	apll1_ck,
	apll2_ck,
	hadds2_ck,
	syspll_d2
};

static const char *pll_test_parents[] __initconst = {
	normal_ck,
	test_in_0
};

static const char *aud_adc_parents[] __initconst = {
	clk27m_ck,
	aclk_k10,
	aclk_k5,
	mphon_in
};

static const char *aud_pwm_parents[] __initconst = {
	clk27m_ck,
	aclk_k11,
	aclk_k5,
	mphon_in
};

static const char *mlin2_parents[] __initconst = {
	clk27m_ck,
	aclk_k12,
	spmck_in,
	spmck2_in
};

static const char *aud_mph_parents[] __initconst = {
	clk27m_ck,
	aclk_k13,
	ack_in,
	spmck_in
};

static const char *mlin_parents[] __initconst = {
	clk27m_ck,
	aclk_k3,
	spmck2_in,
	spmck_in
};

static const char *ts0_parents[] __initconst = {
	demux_ck,
	ext_ts0_clk,
	ts_out
};

static const char *ts1_parents[] __initconst = {
	demux_ck,
	ext_ts1_clk,
	ts_out
};

static const char *png_parents[] __initconst = {
	clk27m_ck,
	apll1_ck,
	apll2_d2,
	syspll_d6,
	syspll_d3,
	syspll_d2,
	usbpll_d2,
	apll2_ck
};

static const char *spi_moto_parents[] __initconst = {
	clk27m_ck,
	syspll_d3,
	syspll_d6,
	apll2_d2
};

static const char *tvd_mbist_parents[] __initconst = {
	cvsb_adc_ck,
	syspll_d12
};

static const char *sramif_parents[] __initconst = {
	clk27m_ck,
	syspll_d2,
	apll2_ck,
	apll1_ck,
	usbpll_d2,
	syspll_d3,
	syspll_d4,
	syspll_d6
};

static const char *mclk_div2_parents[] __initconst = {
	dmpll_ck,
	dmpll_d2
};

static const char *da_apllck_parents[] __initconst = {
	clk27m_ck,
	ack_in,
	i2s_out2_mclk,
	mphon_in,
	spmclk_in
};

static const char *da_apllck1_parents[] __initconst = {
	clk27m_ck,
	ack_in,
	i2s_out2_mclk,
	mphon_in,
	spmclk_in
};

static const char *lvds_parents[] __initconst = {
	ttl_ck,
	lvds_dpix_ck
};

static const char *twds_parents[] __initconst = {
	ttl_ck,
	twds_dpix_ck
};

static const char *pwm0_parents[] __initconst = {
	clk27m_ck,
	syspll_d3,
	syspll_d6,
	apll2_ck,
	syspll_d2,
	armpll_d2,
	dmpll_d2,
	apll1_ck
};

static const char *pwm1_parents[] __initconst = {
	clk27m_ck,
	syspll_d3,
	syspll_d6,
	apll2_ck,
	syspll_d2,
	armpll_d2,
	dmpll_d2,
	apll1_ck
};

static const char *pwm2_parents[] __initconst = {
	clk27m_ck,
	syspll_d3,
	syspll_d6,
	apll2_ck,
	syspll_d2,
	armpll_d2,
	dmpll_d2,
	apll1_ck
};

static const char *pwm3_parents[] __initconst = {
	clk27m_ck,
	syspll_d3,
	syspll_d6,
	apll2_ck,
	syspll_d2,
	armpll_d2,
	dmpll_d2,
	apll1_ck
};

static const char *sifm0_parents[] __initconst = {
	clk27m_ck,
	syspll_d18,
	syspll_d9,
	usbpll_d4,
	usbpll_d2
};

static const char *sifm1_parents[] __initconst = {
	clk27m_ck,
	syspll_d18,
	syspll_d9,
	usbpll_d4,
	usbpll_d2
};

static const char *sifs0_parents[] __initconst = {
	clk27m_ck,
	syspll_d18,
	syspll_d9,
	usbpll_d4,
	usbpll_d2
};

static const char *sifs1_parents[] __initconst = {
	clk27m_ck,
	syspll_d18,
	syspll_d9,
	usbpll_d4,
	usbpll_d2
};

static const char *osd1_parents[] __initconst = {
	sd_108m_ck,
	panel_ck
};

static const char *osd2_parents[] __initconst = {
	sd_108m_ck,
	panel_ck
};

static const char *osd3_parents[] __initconst = {
	sd_108m_ck,
	panel_ck
};

static const char *osd4_parents[] __initconst = {
	sd_108m_ck,
	panel_ck
};

static const char *osd_main_parents[] __initconst = {
	sd_108m_ck,
	panel_ck
};

static const char *osd_aux_parents[] __initconst = {
	sd_108m_ck,
	panel_ck
};

static const char *vdt_ft_parents[] __initconst = {
	normal_ck,
	clk27m_ck
};

static const char *aud_a1_tst_parents[] __initconst = {
	aclk_a1,
	mphon_in
};

static const char *aud_a2_tst_parents[] __initconst = {
	aclk_a2,
	mphon_in
};

struct atc_mux {
	int id;
	const char *name;
	uint32_t reg;
	int shift;
	int width;
	int gate;
	const char **parent_names;
	int num_parents;
};

#define MUX(_id, _name, _parents, _reg, _shift, _width, _gate) {	\
		.id = _id,						\
		.name = _name,						\
		.reg = _reg,						\
		.shift = _shift,					\
		.width = _width,					\
		.gate = _gate,						\
		.parent_names = (const char **)_parents,		\
		.num_parents = ARRAY_SIZE(_parents),			\
	}

static struct atc_mux top_muxes[] __initdata = {
	MUX(TOP_MUX_CLK_DRAM_SLOW, dram_slow_sel, dram_slow_parents, 0x0008, 0, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SD31, sd31_sel, sd31_parents, 0x0008, 4, 4, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_RFI1, rfi1_sel, rfi1_parents, 0x0008, 12, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_RFI2, rfi2_sel, rfi2_parents, 0x0008, 14, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_RFI3, rfi3_sel, rfi3_parents, 0x0008, 16, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_RFI4, rfi4_sel, rfi4_parents, 0x0008, 18, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_RFI5, rfi5_sel, rfi5_parents, 0x0008, 20, 2, INVALID_MUX_GATE_BIT),
	//MUX(TOP_MUX_CLK_RFI6, rfi6_sel, rfi6_parents, 0x0008, 22, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SD21, sd21_sel, sd21_parents, 0x0008, 24, 4, INVALID_MUX_GATE_BIT),

	MUX(TOP_MUX_CLK_ADSP, adsp_sel, adsp_parents, 0x000C, 0, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SD30, sd30_sel, sd30_parents, 0x000C, 3, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_DEMUX, demux_sel, demux_parents, 0x000C, 6, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_RSIC, rsic_sel, rsic_parents, 0x000C, 10, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_VDO, vdo_sel, vdo_parents, 0x000C, 12, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_TP, tp_sel, tp_parents, 0x000C, 13, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_TP_F32, tp_f32_sel, tp_f32_parents, 0x000C, 15, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_RFI, rfi_sel, rfi_parents, 0x000C, 27, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_VDEC1, vdec1_sel, vdec1_parents, 0x000C, 29, 3, INVALID_MUX_GATE_BIT),

	MUX(TOP_MUX_CLK_BCLK, bclk_sel, bclk_parents, 0x0010, 0, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_RESZ1, resz1_sel, resz1_parents, 0x0010, 3, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_FLASH, flash_sel, flash_parents, 0x0010, 6, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_RESZ, resz_sel, resz_parents, 0x0010, 12, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_JPEG, jpeg_sel, jpeg_parents, 0x0010, 15, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_VDEC, vdec_sel, vdec_parents, 0x0010, 18, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SPM, spm_sel, spm_parents, 0x0010, 21, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AXIM, axim_sel, axim_parents, 0x0010, 24, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_DRAM, dram_sel, dram_parents, 0x0010, 27, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_OSD, osd_sel, osd_parents, 0x0010, 28, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_USB_27M, usb_27m_sel, usb_27m_parents, 0x0010, 31, 1, INVALID_MUX_GATE_BIT),

	MUX(TOP_MUX_CLK_GRAPH, graph_sel, graph_parents, 0x0014, 0, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SD00, sd00_sel, sd00_parents, 0x0014, 3, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SD10, sd10_sel, sd10_parents, 0x0014, 6, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SD20, sd20_sel, sd20_parents, 0x0014, 9, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SD01, sd01_sel, sd01_parents, 0x0014, 12, 4, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SD11, sd11_sel, sd11_parents, 0x0014, 16, 4, INVALID_MUX_GATE_BIT),
	//MUX(TOP_MUX_CLK_FPD, fpd_sel, fpd_parents, 0x0014, 21, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_G3D, g3d_sel, g3d_parents, 0x0014, 24, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUD, aud_sel, aud_parents, 0x0014, 28, 2, INVALID_MUX_GATE_BIT),

	MUX(TOP_MUX_CLK_AUD2, aud2_sel, aud2_parents, 0x0018, 0, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_CPU1, cpu1_sel, cpu1_parents, 0x0018, 2, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_CPU2, cpu2_sel, cpu2_parents, 0x0018, 4, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_MPHON, mphon_sel, mphon_parents, 0x0018, 6, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SSUSB_XHCI, ssusb_xhci_sel, ssusb_xhci_parents, 0x0018, 8, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SSUSB, ssusb_sel, ssusb_parents, 0x0018, 10, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_ARM_AUD, arm_aud_sel, arm_aud_parents, 0x0018, 12, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_CLK_NR, clk_nr_sel, clk_nr_parents, 0x0018, 14, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_BT_MIC_AUD, bt_mic_aud_sel, bt_mic_aud_parents, 0x0018, 18, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_THERM_SLOW, therm_slow_sel, therm_slow_parents, 0x0018, 20, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_THERM, therm_sel, therm_parents, 0x0018, 22, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_NFLASH, nflash_sel, nflash_parents, 0x0018, 24, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_DEG, deg_sel, deg_parents, 0x0018, 28, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_DUTY, duty_sel, duty_parents, 0x0018, 30, 2, INVALID_MUX_GATE_BIT),

	MUX(TOP_MUX_CLK_AUDIO_K1, audio_k1_sel, audio_k1_parents, 0x0028, 16, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_K2, audio_k2_sel, audio_k2_parents, 0x0028, 18, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_K3, audio_k3_sel, audio_k3_parents, 0x0028, 20, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_K4, audio_k4_sel, audio_k4_parents, 0x0028, 22, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_K5, audio_k5_sel, audio_k5_parents, 0x0028, 24, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_K6, audio_k6_sel, audio_k6_parents, 0x0028, 26, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_K7, audio_k7_sel, audio_k7_parents, 0x0028, 28, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_K8, audio_k8_sel, audio_k8_parents, 0x0028, 30, 2, INVALID_MUX_GATE_BIT),

	MUX(TOP_MUX_CLK_AUDIO_K9, audio_k9_sel, audio_k9_parents, 0x002C, 0, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_K10, audio_k10_sel, audio_k10_parents, 0x002C, 2, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_K11, audio_k11_sel, audio_k11_parents, 0x002C, 4, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_K12, audio_k12_sel, audio_k12_parents, 0x002C, 6, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_K13, audio_k13_sel, audio_k13_parents, 0x002C, 8, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_K14, audio_k14_sel, audio_k14_parents, 0x002C, 10, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_A1, audio_a1_sel, audio_a1_parents, 0x002C, 12, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_A2, audio_a2_sel, audio_a2_parents, 0x002C, 14, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUDIO_A3, audio_a3_sel, audio_a3_parents, 0x002C, 16, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_PLL_TEST, pll_test_sel, pll_test_parents, 0x002C, 19, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUD_ADC, aud_adc_sel, aud_adc_parents, 0x002C, 21, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUD_PWM, aud_pwm_sel, aud_pwm_parents, 0x002C, 23, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_MLIN2, mlin2_sel, mlin2_parents, 0x002C, 25, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUD_MPH, aud_mph_sel, aud_mph_parents, 0x002C, 27, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_MLIN, mlin_sel, mlin_parents, 0x002C, 29, 2, INVALID_MUX_GATE_BIT),

	MUX(TOP_MUX_CLK_TS0, ts0_sel, ts0_parents, 0x0030, 0, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_TS1, ts1_sel, ts1_parents, 0x0030, 2, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_PNG, png_sel, png_parents, 0x0030, 5, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SPI_MOTO, spi_moto_sel, spi_moto_parents, 0x0030, 8, 2, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_TVD_MBIST, tvd_mbist_sel, tvd_mbist_parents, 0x0030, 10, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SRAMIF, sramif_sel, sramif_parents, 0x0030, 12, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_MCLK_DIV2, mclk_div2_sel, mclk_div2_parents, 0x0030, 16, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_DA_APLLCK, da_apllck_sel, da_apllck_parents, 0x0030, 18, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_DA_APLL1CK, da_apllck1_sel, da_apllck1_parents, 0x0030, 21, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUD_A1_TST, aud_a1_tst_sel, aud_a1_tst_parents, 0x0030, 27, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_AUD_A2_TST, aud_a2_tst_sel, aud_a2_tst_parents, 0x0030, 28, 1, INVALID_MUX_GATE_BIT),

	MUX(TOP_MUX_CLK_LVDS, lvds_sel, lvds_parents, 0x0034, 3, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_TWDS, twds_sel, twds_parents, 0x0034, 4, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_PWM0, pwm0_sel, pwm0_parents, 0x0034, 6, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_PWM1, pwm1_sel, pwm1_parents, 0x0034, 9, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_PWM2, pwm2_sel, pwm2_parents, 0x0034, 13, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_PWM3, pwm3_sel, pwm3_parents, 0x0034, 16, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SIFM0, sifm0_sel, sifm0_parents, 0x0034, 19, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SIFM1, sifm1_sel, sifm1_parents, 0x0034, 22, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SIFS0, sifs0_sel, sifs0_parents, 0x0034, 25, 3, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_SIFS1, sifs1_sel, sifs1_parents, 0x0034, 28, 3, INVALID_MUX_GATE_BIT),

	MUX(TOP_MUX_CLK_OSD1_P, osd1_sel, osd1_parents, 0x00D4, 16, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_OSD2_P, osd2_sel, osd2_parents, 0x00D4, 17, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_OSD3_P, osd3_sel, osd3_parents, 0x00D4, 18, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_OSD4_P, osd4_sel, osd4_parents, 0x00D4, 19, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_OSD_MAIN_P, osd_main_sel, osd_main_parents, 0x00D4, 23, 1, INVALID_MUX_GATE_BIT),
	MUX(TOP_MUX_CLK_OSD_AUX_P, osd_aux_sel, osd_aux_parents, 0x00D4, 24, 1, INVALID_MUX_GATE_BIT),

	MUX(TOP_MUX_CLK_VDT_FT, vdt_ft_sel, vdt_ft_parents, 0x00D8, 31, 1, INVALID_MUX_GATE_BIT),
};

static void __init init_clk_topselect(void __iomem *top_base, struct clk_onecell_data *clk_data)
{
	int i;
	struct clk *clk;
	int ret = 0;

	for (i = 0; i < ARRAY_SIZE(top_muxes); i++) {
		struct atc_mux *mux = &top_muxes[i];

		clk = atc_clk_register_mux(mux->name,
					   mux->parent_names, mux->num_parents,
					   top_base + mux->reg, mux->shift, mux->width, mux->gate);

		if (IS_ERR(clk)) {
			pr_err("Failed to register clk %s: %ld\n",
			       mux->name, PTR_ERR(clk));
			continue;
		}

		ret = clk_register_clkdev(clk, mux->name, NULL);

		if (ret) {
			pr_err("Failed to register clkdev %s\n", mux->name);
		}

		if (clk_data) {
			clk_data->clks[mux->id] = clk;
		}
	}
}


/* PLL */
struct atc_pll {
	int id;
	const char *name;
	const char *parent_name;
	uint32_t reg;
	uint32_t pwr_reg;
	uint32_t en_mask;
	unsigned int flags;
	const struct clk_ops *ops;
};

#define PLL(_id, _name, _parent, _reg, _pwr_reg, _en_mask, _flags, _ops) { \
		.id = _id,						\
		.name = _name,						\
		.parent_name = _parent,					\
		.reg = _reg,						\
		.pwr_reg = _pwr_reg,					\
		.en_mask = _en_mask,					\
		.flags = _flags,					\
		.ops = _ops,						\
	}

static struct atc_pll clk_plls[] __initdata = {
	PLL(APMIXED_ARMPLL, armpll, clk27m, 0x0200, 0x020C, 0x00000001, HAVE_PLL_HP, &atc_clk_arm_pll_ops),
	PLL(APMIXED_ARMPLL2, armpll2, clk27m, 0x0200, 0x020C, 0x00000001, HAVE_PLL_HP, &atc_clk_arm_pll_ops),
	PLL(APMIXED_SYSPLL, syspll, clk27m, 0x0210, 0x021C, 0xF0000101, HAVE_PLL_HP, &atc_clk_sdm_pll_ops),
	PLL(APMIXED_MSDCPLL, msdcpll, clk27m, 0x0240, 0x024C, 0x00000001, HAVE_PLL_HP, &atc_clk_sdm_pll_ops),
	PLL(APMIXED_MMPLL, mmpll, clk27m, 0x0220, 0x022C, 0xFC000001, HAVE_PLL_HP, &atc_clk_univ_pll_ops),
	PLL(APMIXED_USBPLL, usbpll, clk27m, 0x0220, 0x022C, 0xFC000001, HAVE_PLL_HP, &atc_clk_univ_pll_ops),
	PLL(APMIXED_APLL1, apll1, clk27m, 0x0270, 0x0280, 0x00000001, HAVE_PLL_HP, &atc_clk_aud_pll_ops),
	PLL(APMIXED_APLL2, apll2, clk27m, 0x0284, 0x0294, 0x00000001, HAVE_PLL_HP, &atc_clk_aud_pll_ops),
	PLL(APMIXED_APLL3, apll3, clk27m, 0x0284, 0x0294, 0x00000001, HAVE_PLL_HP, &atc_clk_aud_pll_ops),
	PLL(APMIXED_ADPLL_324M, adpll_324m, clk27m, 0x0230, 0x023C, 0x00000001, HAVE_PLL_HP, &atc_clk_mm_pll_ops),
	PLL(APMIXED_ADPLL_108M, adpll_108m, clk27m, 0x0250, 0x025C, 0x00000001, HAVE_PLL_HP, &atc_clk_sdm_pll_ops),
	PLL(APMIXED_ADPLL_648M, adpll_648m, clk27m, 0x0260, 0x026C, 0x00000001, HAVE_PLL_HP, &atc_clk_sdm_pll_ops),
	PLL(APMIXED_VPLL_TTL, vpll_ttl, clk27m, 0x0250, 0x025C, 0x00000001, HAVE_PLL_HP, &atc_clk_sdm_pll_ops),
	PLL(APMIXED_ACLK, aclk, clk27m, 0x0260, 0x026C, 0x00000001, HAVE_PLL_HP, &atc_clk_sdm_pll_ops),
	PLL(APMIXED_G3DPLL, g3dpll, clk27m, 0x0260, 0x026C, 0x00000001, HAVE_PLL_HP, &atc_clk_sdm_pll_ops),
};

static void __init init_clk_apmixedsys(void __iomem *apmixed_base,
				       struct clk_onecell_data *clk_data)
{
	int i;
	struct clk *clk;
	int ret = 0;

	for (i = 0; i < ARRAY_SIZE(clk_plls); i++) {
		struct atc_pll *pll = &clk_plls[i];

		clk = atc_clk_register_pll(pll->name, pll->parent_name,
					   apmixed_base + pll->reg,
					   apmixed_base + pll->pwr_reg,
					   pll->en_mask, pll->flags, pll->ops);

		if (IS_ERR(clk)) {
			pr_err("Failed to register clk %s: %ld\n",
			       pll->name, PTR_ERR(clk));
			continue;
		}

		ret = clk_register_clkdev(clk, pll->name, NULL);

		if (ret) {
			pr_err("Failed to register clkdev %s\n", pll->name);
		}

		if (clk_data) {
			clk_data->clks[pll->id] = clk;
		}
	}
}


/* GATE */
struct atc_gate_regs {
	unsigned int onoff_ofs;
	unsigned int rst_ofs;
};

struct atc_gate {
	unsigned int id;
	const char *name;
	const char *parent_name;
	struct atc_gate_regs *regs;
	unsigned int onoff_shift;
	unsigned int onoff_flags;
	unsigned int rst_shift;
	unsigned int rst_flags;
};

#define GATE(_id, _name, _parent, _regs, _onoffshift, _onoffflags, _rstshift, _rstflags) {	\
		.id = _id,					\
		.name = _name,					\
		.parent_name = _parent,				\
		.regs = &_regs,					\
		.onoff_shift = _onoffshift,				\
		.onoff_flags = _onoffflags,				\
		.rst_shift = _rstshift,				\
		.rst_flags = _rstflags,				\
	}

static void __init init_clk_gates(
	void __iomem *reg_base,
	struct atc_gate *clks, int num,
	struct clk_onecell_data *clk_data)
{
	int i;
	struct clk *clk;
	int ret = 0;

	for (i = 0; i < num; i++) {
		struct atc_gate *gate = &clks[i];

		clk = atc_clk_register_gate(gate->name, gate->parent_name,
					    reg_base + gate->regs->onoff_ofs,
					    reg_base + gate->regs->rst_ofs,
					    gate->onoff_shift, gate->onoff_flags,
					    gate->rst_shift, gate->rst_flags);

		if (IS_ERR(clk)) {
			pr_err("Failed to register clk %s: %ld\n",
			       gate->name, PTR_ERR(clk));
			continue;
		}

		ret = clk_register_clkdev(clk, gate->name, NULL);

		if (ret) {
			pr_err("Failed to register clkdev %s\n", gate->name);
		}

		if (clk_data) {
			clk_data->clks[gate->id] = clk;
		}

	}
}

/* gate vdec */
static struct atc_gate_regs vdec_cg_regs = {
	.onoff_ofs = 0x009C,
	.rst_ofs = 0x00B8,
};

static struct atc_gate vdec_clks[] __initdata = {
	GATE(VDEC_FULL_CLK, vdec_full, vdec_sel, vdec_cg_regs, 0, 1, 0, 1),
};

static void __init init_clk_vdec_sys(void __iomem *vdec_base,
				     struct clk_onecell_data *clk_data)
{
	init_clk_gates(vdec_base, vdec_clks, ARRAY_SIZE(vdec_clks),	clk_data);
}

/* gate img dramc */
static struct atc_gate_regs img_dramc_cg_regs = {
	.onoff_ofs = 0x00A0,
	.rst_ofs = 0x00BC,
};

static struct atc_gate img_dramc_clks[] __initdata = {
	GATE(IMG_DRAMC_CLK_GFX, img_dramc_gfx, graph_sel, img_dramc_cg_regs, 0, 1, 0, 1),
	GATE(IMG_DRAMC_CLK_DMARB, img_dramc_dmarb, dram_sel, img_dramc_cg_regs, 1, 1, 1, 1),
	GATE(IMG_DRAMC_CLK_PNG, img_dramc_png, png_sel, img_dramc_cg_regs, 2, 1, 2, 1),
	GATE(IMG_DRAMC_CLK_GIF, img_dramc_gif, png_sel, img_dramc_cg_regs, 3, 1, 3, 1),
	GATE(IMG_DRAMC_CLK_IMG_RESZ, img_dramc_img_resz, resz_sel, img_dramc_cg_regs, 4, 1, 4, 1),
	GATE(IMG_DRAMC_CLK_OSD_RESZ, img_dramc_osd_resz, osd_sel, img_dramc_cg_regs, 5, 1, 5, 1),
	GATE(IMG_DRAMC_CLK_JPGDEC, img_dramc_jpgdec, jpeg_sel, img_dramc_cg_regs, 6, 1, 6, 1),
	GATE(IMG_DRAMC_CLK_DEMUX, img_dramc_demux, demux_sel, img_dramc_cg_regs, 7, 0, 7, 1),
	GATE(IMG_DRAMC_CLK_DEMUX_TS0, img_dramc_demux_ts0, ts0_sel, img_dramc_cg_regs, 8, 0, 8, 1),
	GATE(IMG_DRAMC_CLK_DEMUX_TS1, img_dramc_demux_ts1, ts1_sel, img_dramc_cg_regs, 9, 0, 9, 1),
	GATE(IMG_DRAMC_CLK_DEMUX_27M, img_dramc_demux_27m, demux_sel, img_dramc_cg_regs, 11, 0, 11, 1),
	GATE(IMG_DRAMC_CLK_SUB_IMG_RESZ, img_dramc_sub_img_resz, resz_sel, img_dramc_cg_regs, 12, 1, 12, 1),
	GATE(IMG_DRAMC_CLK_SUB_OSD_RESZ, img_dramc_sub_osd_resz, osd_sel, img_dramc_cg_regs, 13, 1, 13, 1),
	GATE(IMG_DRAMC_CLK_USB_PORT0, img_dramc_usb_port0, usb_27m_sel, img_dramc_cg_regs, 16, 1, 16, 1),
	GATE(IMG_DRAMC_CLK_USB_PORT1, img_dramc_usb_port1, usb_27m_sel, img_dramc_cg_regs, 17, 1, 17, 1),
	GATE(IMG_DRAMC_CLK_THERM, img_dramc_therm, therm_sel, img_dramc_cg_regs, 18, 1, 18, 1),
	GATE(IMG_DRAMC_CLK_THERM_SLOW, img_dramc_therm_slow, therm_slow_sel, img_dramc_cg_regs, 19, 1, 19, 1),
	GATE(IMG_DRAMC_CLK_IRT_DMA_WRAPPER, img_dramc_irt_dma_wrapper, demux_sel, img_dramc_cg_regs, 20, 1, 20, 1),
	GATE(IMG_DRAMC_CLK_SSUSB_XHCI, img_dramc_ssusb_xhci, ssusb_xhci_sel, img_dramc_cg_regs, 21, 1, 21, 1),
	GATE(IMG_DRAMC_CLK_SSUSB, img_dramc_ssusb, ssusb_sel, img_dramc_cg_regs, 22, 1, 22, 1),
	GATE(IMG_DRAMC_CLK_NFI, img_dramc_nfi, nflash_sel, img_dramc_cg_regs, 23, 1, 23, 1),
	GATE(IMG_DRAMC_CLK_ARM9, img_dramc_arm9, armpll2, img_dramc_cg_regs, 24, 1, 24, 0),
};

static void __init init_clk_img_dramc_sys(void __iomem *img_dramc_base,
					  struct clk_onecell_data *clk_data)
{
	init_clk_gates(img_dramc_base, img_dramc_clks, ARRAY_SIZE(img_dramc_clks),
		       clk_data);
}

/* gate audio */
static struct atc_gate_regs audio_cg_regs = {
	.onoff_ofs = 0x00A8,
	.rst_ofs = 0x00C4,
};

static struct atc_gate audio_peri_clks[] __initdata = {
	GATE(AUDIO_PERI_CLK_B00, audio_peri_b00, aud_sel, audio_cg_regs, 0, 1, 0, 1),
	GATE(AUDIO_PERI_CLK_B01, audio_peri_b01, aud_sel, audio_cg_regs, 1, 1, 1, 1),
	GATE(AUDIO_PERI_CLK_B02, audio_peri_b02, aud_sel, audio_cg_regs, 2, 1, 2, 1),
	GATE(AUDIO_PERI_CLK_B03, audio_peri_b03, aud_sel, audio_cg_regs, 3, 1, 3, 1),
	GATE(AUDIO_PERI_CLK_B04, audio_peri_b04, aud_sel, audio_cg_regs, 4, 1, 4, 1),
	GATE(AUDIO_PERI_CLK_B05, audio_peri_b05, aud_sel, audio_cg_regs, 5, 1, 5, 1),
	GATE(AUDIO_PERI_CLK_B06, audio_peri_b06, aud_sel, audio_cg_regs, 6, 1, 6, 1),
	GATE(AUDIO_PERI_CLK_B07, audio_peri_b07, aud_sel, audio_cg_regs, 7, 1, 7, 1),
	GATE(AUDIO_PERI_CLK_B08, audio_peri_b08, aud_sel, audio_cg_regs, 8, 1, 8, 1),
	GATE(AUDIO_PERI_CLK_B09, audio_peri_b09, aud_sel, audio_cg_regs, 9, 1, 9, 1),
	GATE(AUDIO_PERI_CLK_B10, audio_peri_b10, aud_sel, audio_cg_regs, 10, 1, 10, 1),
	GATE(AUDIO_PERI_CLK_B11, audio_peri_b11, aud_sel, audio_cg_regs, 11, 1, 11, 1),
	GATE(AUDIO_PERI_CLK_B12, audio_peri_b12, aud_sel, audio_cg_regs, 12, 1, 12, 1),
	GATE(AUDIO_PERI_CLK_B13, audio_peri_b13, aud_sel, audio_cg_regs, 13, 1, 13, 1),
	GATE(AUDIO_PERI_CLK_B14, audio_peri_b14, aud_sel, audio_cg_regs, 14, 1, 14, 1),
	GATE(AUDIO_PERI_CLK_RFI_TOP, audio_peri_rfi_top, rfi_sel, audio_cg_regs, 15, 1, 15, 1),
	GATE(AUDIO_PERI_CLK_MSDC_PD0, audio_peri_msdc_pd0, aud_sel, audio_cg_regs, 16, 1, 16, 1),
	GATE(AUDIO_PERI_CLK_MSDC_PD1, audio_peri_msdc_pd1, aud_sel, audio_cg_regs, 17, 1, 17, 1),
	GATE(AUDIO_PERI_CLK_MSDC_PD2, audio_peri_msdc_pd2, aud_sel, audio_cg_regs, 18, 1, 18, 1),
	GATE(AUDIO_PERI_CLK_MSDC_SW0, audio_peri_msdc_sw0, aud_sel, audio_cg_regs, INVALID_MUX_GATE_BIT, 0, 19, 1),
	GATE(AUDIO_PERI_CLK_MSDC_SW1, audio_peri_msdc_sw1, aud_sel, audio_cg_regs, INVALID_MUX_GATE_BIT, 0, 20, 1),
	GATE(AUDIO_PERI_CLK_MSDC_SW2, audio_peri_msdc_sw2, aud_sel, audio_cg_regs, INVALID_MUX_GATE_BIT, 0, 21, 1),
	GATE(AUDIO_PERI_CLK_SPI_MOTO1, audio_peri_spi_moto1, aud_sel, audio_cg_regs, 22, 1, 22, 1),
	GATE(AUDIO_PERI_CLK_SPI_MOTO2, audio_peri_spi_moto2, aud_sel, audio_cg_regs, 23, 1, 23, 1),
	GATE(AUDIO_PERI_CLK_PWMX6_0, audio_peri_pwmx6_0, aud_pwm_sel, audio_cg_regs, 24, 1, 24, 1),
	GATE(AUDIO_PERI_CLK_PWMX6_1, audio_peri_pwmx6_1, aud_pwm_sel, audio_cg_regs, 25, 1, 25, 1),
	GATE(AUDIO_PERI_CLK_PWMX6_2, audio_peri_pwmx6_2, aud_pwm_sel, audio_cg_regs, 26, 1, 26, 1),
	GATE(AUDIO_PERI_CLK_PWMX6_3, audio_peri_pwmx6_3, aud_pwm_sel, audio_cg_regs, 27, 1, 27, 1),
	GATE(AUDIO_PERI_CLK_SIFM0, audio_peri_sifm0, sifm0_sel, audio_cg_regs, 28, 1, 28, 1),
	GATE(AUDIO_PERI_CLK_SIFM1, audio_peri_sifm1, sifm1_sel, audio_cg_regs, 29, 1, 29, 1),
	GATE(AUDIO_PERI_CLK_SIFS0, audio_peri_sifs0, sifs0_sel, audio_cg_regs, 30, 1, 30, 1),
	GATE(AUDIO_PERI_CLK_SIFS1, audio_peri_sifs1, sifs1_sel, audio_cg_regs, 31, 1, 31, 1),
};

static void __init init_clk_audio_peri_sys(void __iomem *audio_peri_base,
					   struct clk_onecell_data *clk_data)
{
	init_clk_gates(audio_peri_base, audio_peri_clks, ARRAY_SIZE(audio_peri_clks), clk_data);
}

/* gate misc */
static struct atc_gate_regs misc_cg_regs = {
	.onoff_ofs = 0x00AC,
	.rst_ofs = 0x00C8,
};

static struct atc_gate misc_clks[] __initdata = {
	GATE(MISC_CLK_MSDC3, misc_msdc3, dmpll_ck, misc_cg_regs, 0, 1, 0, 1),
	GATE(MISC_CLK_MSDC3_SRESETB, misc_msdc3_sresetb, dmpll_ck, misc_cg_regs, INVALID_MUX_GATE_BIT, 0, 1, 1),
	GATE(MISC_CLK_VDEC1, misc_vdec1, vdec_sel, misc_cg_regs, 2, 1, 2, 1),
	GATE(MISC_CLK_VDEC2, misc_vdec2, vdec1_sel, misc_cg_regs, 3, 1, 3, 1),
	GATE(MISC_CLK_MFG_TOP_PWR_WRAP, misc_mfg_top_pwr_wrap, clk27m_ck, misc_cg_regs, 31, 1, 31, 1),
};

static void __init init_clk_misc_sys(void __iomem *misc_base, struct clk_onecell_data *clk_data)
{
	init_clk_gates(misc_base, misc_clks, ARRAY_SIZE(misc_clks), clk_data);
}

/* gate lvds */
static struct atc_gate_regs lvds_cg_regs = {
	.onoff_ofs = 0x00B0,
	.rst_ofs = 0x00CC,
};

static struct atc_gate lvds_clks[] __initdata = {
	GATE(LVDS_CLK_LVDS, lvds_clk, lvds_sel, lvds_cg_regs, 0, 1, 0, 0),
	GATE(LVDS_CLK_TP_TOP0, lvds_tp_top0, tp_sel, lvds_cg_regs, 1, 1, 1, 0),
	GATE(LVDS_CLK_TP_TOP1, lvds_tp_top1, tp_sel, lvds_cg_regs, 2, 1, 2, 0),
	GATE(LVDS_CLK_TP_TOP2, lvds_tp_top2, tp_sel, lvds_cg_regs, 3, 1, 3, 0),
	GATE(LVDS_CLK_RFI_TOP1, lvds_rfi_top1, rfi1_sel, lvds_cg_regs, 4, 1, INVALID_MUX_GATE_BIT, 0),
	GATE(LVDS_CLK_RFI_TOP2, lvds_rfi_top2, rfi2_sel, lvds_cg_regs, 5, 1, INVALID_MUX_GATE_BIT, 0),
	GATE(LVDS_CLK_RFI_TOP3, lvds_rfi_top3, rfi3_sel, lvds_cg_regs, 6, 1, INVALID_MUX_GATE_BIT, 0),
	GATE(LVDS_CLK_RFI_TOP4, lvds_rfi_top4, rfi4_sel, lvds_cg_regs, 7, 1, INVALID_MUX_GATE_BIT, 0),
	GATE(LVDS_CLK_RFI_TOP5, lvds_rfi_top5, rfi5_sel, lvds_cg_regs, 8, 1, INVALID_MUX_GATE_BIT, 0),
	GATE(LVDS_CLK_RFI_TOP6, lvds_rfi_top6, rfi6_sel, lvds_cg_regs, 9, 1, INVALID_MUX_GATE_BIT, 0),
};

static void __init init_clk_lvds_sys(void __iomem *lvds_base, struct clk_onecell_data *clk_data)
{
	init_clk_gates(lvds_base, lvds_clks, ARRAY_SIZE(lvds_clks), clk_data);
}


/* gate vout */
static struct atc_gate_regs vdout_cg_regs = {
	.onoff_ofs = 0x00B4,
	.rst_ofs = 0x00D0,
};

static struct atc_gate vdout_clks[] __initdata = {
	GATE(VDOUT_CLK_SCLER, vdout_scler, vdt_ft_sel, vdout_cg_regs, 0, 1, 0, 1),
	GATE(VDOUT_CLK_TVD1, vdout_tvd1, vdt_ft_sel, vdout_cg_regs, 1, 1, 1, 1),
	GATE(VDOUT_CLK_TVD2, vdout_tvd2, vdt_ft_sel, vdout_cg_regs, 2, 1, 1, 1),
	GATE(VDOUT_CLK_OSD, vdout_osd, vdt_ft_sel, vdout_cg_regs, 3, 1, 3, 1),
	GATE(VDOUT_CLK_OSD_R, vdout_osd_r, vdt_ft_sel, vdout_cg_regs, 4, 1, 4, 1),
	GATE(VDOUT_CLK_FPD, vdout_fpd, vdt_ft_sel, vdout_cg_regs, 5, 1, 5, 1),
	GATE(VDOUT_CLK_FMT_VDO_F, vdout_fmt_vdo_f, vdt_ft_sel, vdout_cg_regs, 6, 1, 6, 1),
	GATE(VDOUT_CLK_FMT_VDO_R, vdout_fmt_vdo_r, vdt_ft_sel, vdout_cg_regs, 7, 1, 7, 1),
	GATE(VDOUT_CLK_WRITE_CHANEL, vdout_write_chanel, vdt_ft_sel, vdout_cg_regs, 8, 1, 8, 1),
	GATE(VDOUT_CLK_FRAME_LOCK, vdout_frame_lock, vdt_ft_sel, vdout_cg_regs, 9, 1, 9, 1),
	GATE(VDOUT_CLK_WRITE_CHANEL2, vdout_write_chanel2, vdt_ft_sel, vdout_cg_regs, 10, 1, 10, 1),
	GATE(VDOUT_CLK_VGA, vdout_vga, vdt_ft_sel, vdout_cg_regs, 11, 1, 11, 1),
	GATE(VDOUT_CLK_YPBPR, vdout_ypbpr, vdt_ft_sel, vdout_cg_regs, 13, 1, 13, 1),
	GATE(VDOUT_CLK_HDMI, vdout_hdmi, vdt_ft_sel, vdout_cg_regs, 14, 1, 14, 1),
	GATE(VDOUT_CLK_TVE, vdout_tve, vdt_ft_sel, vdout_cg_regs, 15, 1, 15, 1),
	GATE(VDOUT_CLK_DVD_MIX_2AP, vdout_dvd_mix_2ap, vdt_ft_sel, vdout_cg_regs, 16, 1, 16, 1),
	GATE(VDOUT_CLK_OSD1, vdout_osd1, vdt_ft_sel, vdout_cg_regs, 17, 1, INVALID_MUX_GATE_BIT, 0),
	GATE(VDOUT_CLK_OSD2, vdout_osd2, vdt_ft_sel, vdout_cg_regs, 18, 1, INVALID_MUX_GATE_BIT, 0),
	GATE(VDOUT_CLK_OSD3, vdout_osd3, vdt_ft_sel, vdout_cg_regs, 19, 1, INVALID_MUX_GATE_BIT, 0),
	GATE(VDOUT_CLK_OSD4, vdout_osd4, vdt_ft_sel, vdout_cg_regs, 20, 1, INVALID_MUX_GATE_BIT, 0),
	GATE(VDOUT_CLK_OSD5, vdout_osd5, vdt_ft_sel, vdout_cg_regs, 21, 1, INVALID_MUX_GATE_BIT, 0),
	GATE(VDOUT_CLK_OSD_R_2, vdout_osd_r_2, vdt_ft_sel, vdout_cg_regs, 22, 1, INVALID_MUX_GATE_BIT, 0),
	GATE(VDOUT_CLK_OSD_R_3, vdout_osd_r_3, vdt_ft_sel, vdout_cg_regs, 23, 1, INVALID_MUX_GATE_BIT, 0),
	GATE(VDOUT_CLK_SCLER_TG, vdout_scler_tg, vdt_ft_sel, vdout_cg_regs, 24, 1, 24, 1),
	GATE(VDOUT_CLK_LCPROC_VDO, vdout_lcproc_vdo, vdt_ft_sel, vdout_cg_regs, 25, 1, 25, 1),
	GATE(VDOUT_CLK_WR_CHANNEL_TVD4, vdout_wr_channel_tvd4, vdt_ft_sel, vdout_cg_regs, 26, 1, 26, 1),
	GATE(VDOUT_CLK_LCPROC_VDO_R, vdout_lcproc_vdo_r, vdt_ft_sel, vdout_cg_regs, 27, 1, 27, 1),
	GATE(VDOUT_CLK_WR_CHANNEL_HDMIRX, vdout_wr_channel_hdmirx, vdt_ft_sel, vdout_cg_regs, 28, 1, 28, 1),
	GATE(VDOUT_CLK_WR_CHANNEL_DGI, vdout_wr_channel_dgi, vdt_ft_sel, vdout_cg_regs, 29, 1, 29, 1),
	GATE(VDOUT_CLK_WR_CHANNEL_VGA, vdout_wr_channel_vga, vdt_ft_sel, vdout_cg_regs, 30, 1, 30, 1),
	GATE(VDOUT_CLK_WR_CHANNEL_VDOUT, vdout_wr_channel_vdout, vdt_ft_sel, vdout_cg_regs, 31, 1, 31, 1),
};

static void __init init_clk_vdout_sys(void __iomem *vdout_base, struct clk_onecell_data *clk_data)
{
	init_clk_gates(vdout_base, vdout_clks, ARRAY_SIZE(vdout_clks), clk_data);
}


/*
 * device tree support
 */

static struct clk_onecell_data *alloc_clk_data(unsigned int clk_num)
{
	int i;
	struct clk_onecell_data *clk_data;

	clk_data = kzalloc(sizeof(clk_data), GFP_KERNEL);

	if (!clk_data) {
		return NULL;
	}

	clk_data->clks = kcalloc(clk_num, sizeof(struct clk *), GFP_KERNEL);

	if (!clk_data->clks) {
		kfree(clk_data);
		return NULL;
	}

	clk_data->clk_num = clk_num;

	for (i = 0; i < clk_num; ++i) {
		clk_data->clks[i] = ERR_PTR(-ENOENT);
	}

	return clk_data;
}

static void __iomem *get_reg(struct device_node *np, int index)
{
#if DUMMY_REG_TEST
	return kzalloc(PAGE_SIZE, GFP_KERNEL);
#else
	return of_iomap(np, index);
#endif
}

static void __init ac8237_topckgen_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	void __iomem *base;
	int r;

	base = get_reg(node, 0);

	if (!base) {
		pr_err("ioremap topckgen failed\n");
		return;
	}

	clk_data = alloc_clk_data(TOP_NR_CLK);

	if (clk_data == NULL) {
		pr_err("alloc topckgen clock failed\n");
		return;
	}

	/*init_clk_root_alias(clk_data);*/
	init_clk_top_div(clk_data);
	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r) {
		pr_err("could not register clock provide\n");
	}
}
CLK_OF_DECLARE(atc_topckgen, "atc,ac8237-topckgen", ac8237_topckgen_init);

static void __init ac8237_topselect_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	void __iomem *base;
	int r;

	base = get_reg(node, 0);

	if (!base) {
		pr_err("ioremap topselect failed\n");
		return;
	}

	clk_data = alloc_clk_data(TOP_MUX_NR_CLK);

	if (clk_data == NULL) {
		pr_err("alloc topselect clock failed\n");
		return;
	}

	init_clk_topselect(base, clk_data);
	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r) {
		pr_err("could not register clock provide\n");
	}
}
CLK_OF_DECLARE(atc_topselect, "atc,ac8237-topselect", ac8237_topselect_init);


static void __init ac8237_divider_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	void __iomem *base;
	int r;

	base = get_reg(node, 0);

	if (!base) {
		pr_err("ioremap divider failed\n");
		return;
	}

	clk_data = alloc_clk_data(TOP_DIVIDER_NR_CLK);

	if (clk_data == NULL) {
		pr_err("alloc divider clock failed\n");
		return;
	}

	init_clk_divider(base, clk_data);
	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r) {
		pr_err("could not register clock provide\n");
	}
}
CLK_OF_DECLARE(atc_sub_divider, "atc,ac8237-divider", ac8237_divider_init);

static void __init ac8237_apmixedsys_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	void __iomem *base;
	int r;

	base = get_reg(node, 0);

	if (!base) {
		pr_err("ioremap apmixedsys failed\n");
		return;
	}


	clk_data = alloc_clk_data(APMIXED_NR_CLK);

	if (clk_data == NULL) {
		pr_err("alloc apmixedsys clock failed\n");
		return;
	}

	init_clk_apmixedsys(base, clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r) {
		pr_err("could not register clock provide\n");
	}
}
CLK_OF_DECLARE(atc_apmixedsys, "atc,ac8237-apmixedsys", ac8237_apmixedsys_init);


static void __init atc_vdecsys_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	void __iomem *base;
	int r;

	base = get_reg(node, 0);

	if (!base) {
		pr_err("ioremap vdecsys failed\n");
		return;
	}

	clk_data = alloc_clk_data(VDEC_NR_CLK);

	if (clk_data == NULL) {
		pr_err("alloc vdecsys clock failed\n");
		return;
	}

	init_clk_vdec_sys(base, clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r) {
		pr_err("could not register clock provide\n");
	}
}
CLK_OF_DECLARE(atc_vdecsys, "atc,ac8237-vdecsys", atc_vdecsys_init);

static void __init ac8237_img_dramc_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	void __iomem *base;
	int r;

	base = get_reg(node, 0);

	if (!base) {
		pr_err("ioremap img dramc failed\n");
		return;
	}

	clk_data = alloc_clk_data(IMG_DRAMC_NR_CLK);

	if (clk_data == NULL) {
		pr_err("alloc img dramc clock failed\n");
		return;
	}

	init_clk_img_dramc_sys(base, clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r) {
		pr_err("could not register clock provide\n");
	}
}
CLK_OF_DECLARE(atc_infrasys, "atc,ac8237-imgdramcsys", ac8237_img_dramc_init);

static void __init atc_audioperisys_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	void __iomem *base;
	int r;

	base = get_reg(node, 0);

	if (!base) {
		pr_err("ioremap audioperisys failed\n");
		return;
	}

	clk_data = alloc_clk_data(AUDIO_PERI_NR_CLK);

	if (clk_data == NULL) {
		pr_err("alloc audioperisys clock failed\n");
		return;
	}

	init_clk_audio_peri_sys(base, clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r) {
		pr_err("could not register clock provide\n");
	}
}
CLK_OF_DECLARE(atc_audioperisys, "atc,ac8237-audioperisys", atc_audioperisys_init);

static void __init atc_miscsys_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	void __iomem *base;
	int r;

	base = get_reg(node, 0);

	if (!base) {
		pr_err("ioremap miscsys failed\n");
		return;
	}

	clk_data = alloc_clk_data(MISC_NR_CLK);

	if (clk_data == NULL) {
		pr_err("alloc miscsys clock failed\n");
		return;
	}

	init_clk_misc_sys(base, clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r) {
		pr_err("could not register clock provide\n");
	}
}
CLK_OF_DECLARE(atc_miscsys, "atc,ac8237-miscsys", atc_miscsys_init);

static void __init atc_lvdssys_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	void __iomem *base;
	int r;

	base = get_reg(node, 0);

	if (!base) {
		pr_err("ioremap lvdssys failed\n");
		return;
	}

	clk_data = alloc_clk_data(LVDS_NR_CLK);

	if (clk_data == NULL) {
		pr_err("alloc lvdssys clock failed\n");
		return;
	}

	init_clk_lvds_sys(base, clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r) {
		pr_err("could not register clock provide\n");
	}
}
CLK_OF_DECLARE(atc_lvdssys, "atc,ac8237-lvdssys", atc_lvdssys_init);

static void __init atc_vdoutsys_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	void __iomem *base;
	int r;

	base = get_reg(node, 0);

	if (!base) {
		pr_err("ioremap vdoutsys failed\n");
		return;
	}

	clk_data = alloc_clk_data(VDOUT_NR_CLK);

	if (clk_data == NULL) {
		pr_err("alloc vdoutsys clock failed\n");
		return;
	}

	init_clk_vdout_sys(base, clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r) {
		pr_err("could not register clock provide\n");
	}
}
CLK_OF_DECLARE(atc_vdoutsys, "atc,ac8237-vdoutsys", atc_vdoutsys_init);


