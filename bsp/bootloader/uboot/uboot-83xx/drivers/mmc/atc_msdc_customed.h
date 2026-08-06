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

#ifndef _ATC_MSDC_CUSTOMED_H_
#define _ATC_MSDC_CUSTOMED_H_


#define ATC_MSDC_HOST_NUM			(3)

//---------------------------------------------------------------------------
// Macro definitions
//---------------------------------------------------------------------------
#define MSDC_CD_PIN_EN      (1 << 0)  /* card detection pin is wired   */
#define MSDC_WP_PIN_EN      (1 << 1)  /* write protection pin is wired */
#define MSDC_RST_PIN_EN     (1 << 2)  /* emmc reset pin is wired       */
#define MSDC_SDIO_IRQ       (1 << 3)  /* use internal sdio irq (bus)   */
#define MSDC_EXT_SDIO_IRQ   (1 << 4)  /* use external sdio irq         */
#define MSDC_REMOVABLE      (1 << 5)  /* removable slot                */
#define MSDC_SYS_SUSPEND    (1 << 6)  /* suspended by system           */
#define MSDC_HIGHSPEED      (1 << 7)  /* high-speed mode support       */
#define MSDC_UHS1           (1 << 8)  /* uhs-1 mode support            */
#define MSDC_DDR            (1 << 9)  /* ddr mode support              */
#define MSDC_INTERNAL_CLK   (1 << 11)  /* Force Internal clock */
#define MSDC_GPIO_EINT_CD   (1 << 12)  /* Use GPIO as eint for card detect */
#define MSDC_SD_NEED_POWER  (1 << 31) /* for Yecon board, need SD power always on!! or cannot recognize the sd card*/

#define MSDC_SMPL_RISING    (0)
#define MSDC_SMPL_FALLING   (1)

#define MSDC_CMD_PIN        (0)
#define MSDC_DAT_PIN        (1)
#define MSDC_CD_PIN         (2)
#define MSDC_WP_PIN         (3)
#define MSDC_RST_PIN        (4)

#define MSDC_DATA1_INT      (1)

enum {
    MSDC_CLKSRC_200MHZ = 0,
	MSDC_CLKSRC_196MHZ = 1,
	MSDC_CLKSRC_189MHZ = 2,
	MSDC_CLKSRC_162MHZ = 3,
	MSDC_CLKSRC_147MHZ = 4,
	MSDC_CLKSRC_135MHZ = 5,
	MSDC_CLKSRC_108MHZ = 6,
	MSDC_CLKSRC_27MHZ = 7,
	MSDC_CLKSRC_AUTO = 8
};

#define MSDC_BOOT_EN 		(1)
#define MSDC_CD_HIGH 		(1)
#define MSDC_CD_LOW  		(0)

enum {
    MSDC_EMMC = 0,
    MSDC_SD   = 1,
    MSDC_SDIO = 2
};

//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------
#define MSDC_RESISTOR_10K			(1)
#define MSDC_RESISTOR_50K			(2)

struct msdc_rw_param {
	unsigned int   read_dat_latch_ck_sel;
	unsigned int   read_ckgen_delay_sel;
	unsigned int   read_sample_edge;
	unsigned int   read_pad_delay;

	unsigned int   write_dat_latch_ck_sel;
	unsigned int   write_ckgen_delay_sel;
	unsigned int   write_sample_edge;
	unsigned int   write_pad_delay;
	unsigned int   write_internal_delay;
};


struct msdc_host {
    unsigned char  clk_src;          		/* host clock source */
	unsigned long  max_clock;				/* host max work clock */
    unsigned char  cmd_edge;         		/* command latch edge */
    unsigned char  rdata_edge;        		/* read data latch edge */
	unsigned char  wdata_edge;        		/* write data latch edge */
    unsigned char  clk_drv;          		/* clock pad driving */
    unsigned char  cmd_drv;          		/* command pad driving */
    unsigned char  dat_drv;          		/* data pad driving */
    unsigned char  slew_rate;
    unsigned char  resistor_clk_line;		/* resistor for clk line */
    unsigned char  resistor_cmddat_line;	/* resistor for cmd and data line */
    unsigned long  flags;            		/* hardware capability flags */
    unsigned long  data_pins;        		/* data pins */
    unsigned long  data_offset;      		/* data address offset */
	
    unsigned char  ddlsel;    				/* data line delay line fine tune selecion */
	unsigned char  rdsplsel;  				/* read: data line rising or falling latch fine tune selection */
	unsigned char  wdsplsel;  				/* write: data line rising or falling latch fine tune selection */

	unsigned char  datwrddly; 				/* write; range: 0~31 */
	unsigned char  cmdrrddly; 				/* cmd; range: 0~31 */
	unsigned char  cmdrddly; 				/* cmd; range: 0~31 */
	
	unsigned int  each_dat_line_read_rxdly0;
	unsigned int  each_dat_line_read_rxdly1;
	unsigned int  each_dat_line_write_rxdly0;
	
	unsigned int   read_pre_setting_en;
	unsigned int   read_dat_latch_ck_sel;
	unsigned int   read_ckgen_delay_sel;
	unsigned int   read_sample_edge;
	unsigned int   read_pad_delay;
	unsigned int   ddr_read_dat_latch_ck_sel;
	unsigned int   ddr_read_ckgen_delay_sel;

	unsigned int   write_pre_setting_en;
	unsigned int   write_dat_latch_ck_sel;
	unsigned int   write_ckgen_delay_sel;
	unsigned int   write_sample_edge;
	unsigned int   write_pad_delay;
	unsigned int   write_internal_delay;
	unsigned int   ddr_write_dat_latch_ck_sel;
	unsigned int   ddr_write_ckgen_delay_sel;

	unsigned long  host_function;	 		/* define host function */
	unsigned long  boot;			 		/* define boot host */ 
	unsigned long  cd_level;		 		/* card detection level */	
};


// MSDC Devices
extern struct msdc_host atc_msdc0_hw;
extern struct msdc_host atc_msdc1_hw;
extern struct msdc_host atc_msdc2_hw;

extern struct msdc_host* atc_msdc_dev[];
extern struct msdc_rw_param*atc_msdc0_param[];

#endif // _ATC_MSDC_CUSTOMED_H_

