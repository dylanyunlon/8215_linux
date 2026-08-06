/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of Autochips Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE") RECEIVED
 *     FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE AUTOCHIPS SOFTWARE RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION,
 *     TO REVISE OR REPLACE THE AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#include "atc_msdc_customed.h"
#include "atc_msdc_config.h"


//---------------------------------------------------------------------------
// ATC MSDC Devices definitions
//---------------------------------------------------------------------------
/* Default parameters */
struct msdc_rw_param atc_msdc0_rw_param0 = {
	.read_dat_latch_ck_sel	= 0,
	.read_ckgen_delay_sel	= 13,
	.read_sample_edge		= 0,
	.read_pad_delay 		= 15,

	.write_dat_latch_ck_sel = 0,
	.write_ckgen_delay_sel	= 13,
	.write_sample_edge		= 0,
	.write_pad_delay		= 0,
	.write_internal_delay	= 15,
};

/* Toshiba eMMC on EVB Board, Special */
struct msdc_rw_param atc_msdc0_rw_param1 = {
	.read_dat_latch_ck_sel	= 0,
	.read_ckgen_delay_sel	= 9,
	.read_sample_edge		= 0,
	.read_pad_delay			= 16,

	.write_dat_latch_ck_sel	= 0,
	.write_ckgen_delay_sel	= 9,
	.write_sample_edge		= 0,
	.write_pad_delay		= 3,
	.write_internal_delay	= 15,
};

/* rw parameters for special eMMC, use special parameters */
struct msdc_rw_param* atc_msdc0_param[2] = {
	&atc_msdc0_rw_param0,
	&atc_msdc0_rw_param1,
};


struct msdc_host atc_msdc0_hw = {
    .clk_src        = MSDC_CLKSRC_135MHZ, //DMSDC0_CLK_SRC,
    .max_clock		= DMSDC0_MAXCLK_MHZ,
    .cmd_edge       = MSDC_SMPL_FALLING,
    .rdata_edge     = MSDC_SMPL_FALLING,
    .wdata_edge     = MSDC_SMPL_FALLING,
    .clk_drv        = 0x2F,
    .cmd_drv        = 0x2F,
    .dat_drv        = 0x2F,
    .slew_rate		= 0x0,
    .resistor_clk_line = MSDC_RESISTOR_50K,
    .resistor_cmddat_line = MSDC_RESISTOR_10K,
    .data_pins      = 8,
    .data_offset    = 0,
    .flags          = MSDC_SYS_SUSPEND | MSDC_HIGHSPEED,
	.datwrddly		= 0,
	.cmdrrddly		= 0,
	.cmdrddly		= 0,

	.each_dat_line_read_rxdly0 = 0,
	.each_dat_line_read_rxdly1 = 0,

	.write_pre_setting_en	= 1,
	.read_pre_setting_en	= 1,

/*******************************/
/*****          rw param          *******/
	.read_dat_latch_ck_sel	= 0,
	.read_ckgen_delay_sel	= 13,
	.read_sample_edge		= 0,
	.read_pad_delay 		= 15,

	.write_dat_latch_ck_sel = 0,
	.write_ckgen_delay_sel	= 13,
	.write_sample_edge		= 0,
	.write_pad_delay		= 0,
	.write_internal_delay	= 15,
/*******************************/

	.ddr_read_dat_latch_ck_sel	= 5,
	.ddr_read_ckgen_delay_sel	= 15,
	
	.ddr_write_dat_latch_ck_sel = 5,
	.ddr_write_ckgen_delay_sel	= 15,
	
    .host_function	= MSDC_EMMC,
    .boot			= 0,
    .cd_level		= MSDC_CD_LOW,
};

struct msdc_host atc_msdc1_hw = {
    .clk_src        = DMSDC1_CLK_SRC,		// value from platform_config.h
	.max_clock		= DMSDC1_MAXCLK_MHZ,	// value from platform_config.h
    .cmd_edge       = MSDC_SMPL_FALLING, 
    .rdata_edge     = MSDC_SMPL_FALLING,
    .wdata_edge     = MSDC_SMPL_FALLING,

    #if 0
    .clk_drv        = 0x05,
    .cmd_drv        = 0x05,
    .dat_drv        = 0x05,
	#else
	.clk_drv        = 0x07,
    .cmd_drv        = 0x0C,
    .dat_drv        = 0x0C,
    #endif
	
    .slew_rate		= 0x00,
    .resistor_clk_line = MSDC_RESISTOR_50K,
    .resistor_cmddat_line = MSDC_RESISTOR_10K,
    .data_pins      = 4,
    .data_offset    = 0,
    .flags          = MSDC_SYS_SUSPEND | MSDC_HIGHSPEED | MSDC_GPIO_EINT_CD | MSDC_REMOVABLE, 
	.datwrddly		= 0,
	.cmdrrddly		= 0,
	.cmdrddly		= 0,

	.read_pre_setting_en	= 1,
	.read_dat_latch_ck_sel	= 0,
	.read_ckgen_delay_sel 	= 12,
	.read_sample_edge 		= 1,
	.read_pad_delay			= 12,

	.write_pre_setting_en	= 1,
	.write_dat_latch_ck_sel	= 0,
	.write_ckgen_delay_sel	= 12,
	.write_sample_edge		= 1,
	.write_pad_delay		= 12,
	.write_internal_delay 	= 12,

    .host_function	= MSDC_SD,
    .boot			= 0,
    .cd_level		= MSDC_CD_LOW,
};

struct msdc_host atc_msdc2_hw = {
    .clk_src        = DMSDC2_CLK_SRC,		// value from platform_config.h
	.max_clock		= DMSDC2_MAXCLK_MHZ,	// value from platform_config.h
    .cmd_edge       = MSDC_SMPL_FALLING,
    .rdata_edge     = MSDC_SMPL_FALLING,
    .wdata_edge     = MSDC_SMPL_FALLING,
    
    #if 0
    .clk_drv        = 0x05,
    .cmd_drv        = 0x05,
    .dat_drv        = 0x05,
	#else
	.clk_drv        = 0x07,
    .cmd_drv        = 0x0C,
    .dat_drv        = 0x0C,
	#endif
	
	.slew_rate		= 0x00,
    .resistor_clk_line = MSDC_RESISTOR_50K,
    .resistor_cmddat_line = MSDC_RESISTOR_10K,
    .data_pins      = 4,
    .data_offset    = 0,
    .flags          = MSDC_SYS_SUSPEND | MSDC_HIGHSPEED | MSDC_GPIO_EINT_CD | MSDC_REMOVABLE,  // MSDC_REMOVABLE | MSDC_UHS1 |MSDC_DDR
	.datwrddly		= 0,
	.cmdrrddly		= 0,
	.cmdrddly		= 0,

	.read_pre_setting_en	= 1,
	.read_dat_latch_ck_sel	= 0,
	.read_ckgen_delay_sel	= 12,
	.read_sample_edge		= 1,
	.read_pad_delay			= 12,

	.write_pre_setting_en	= 1,
	.write_dat_latch_ck_sel	= 0,
	.write_ckgen_delay_sel	= 12,
	.write_sample_edge		= 1,
	.write_pad_delay		= 12,
	.write_internal_delay	= 12,
	
    .host_function	= MSDC_SD,
    .boot			= 0,
    .cd_level		= MSDC_CD_LOW,
};

struct msdc_host* atc_msdc_dev[ATC_MSDC_HOST_NUM] = {
	&atc_msdc0_hw,
	&atc_msdc1_hw,
	&atc_msdc2_hw
};


