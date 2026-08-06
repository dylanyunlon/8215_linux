#ifndef _MSDC_IO_H_
#define _MSDC_IO_H_

/**************************************************************/
/* Section 1: Device Tree                                     */
/**************************************************************/
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
extern const struct of_device_id msdc_of_ids[];
extern struct msdc_hw *p_msdc_hw[];
extern unsigned int cd_gpio;
extern struct device_node *eint_node;

extern void __iomem *gpio_base;
extern void __iomem *msdc0_io_cfg_base;
extern void __iomem *msdc1_io_cfg_base;
extern void __iomem *msdc2_io_cfg_base;
extern void __iomem *msdc3_io_cfg_base;

extern void __iomem *infracfg_ao_reg_base;
/*
extern void __iomem *infracfg_reg_base;
*/
extern void __iomem *pericfg_reg_base;
/*
extern void __iomem *emi_reg_base;
extern void __iomem *toprgu_reg_base;
*/
extern void __iomem *apmixed_reg_base;
extern void __iomem *topckgen_reg_base;


int msdc_dt_init(struct platform_device *pdev, struct mmc_host *mmc,
		unsigned int *cd_irq);
void card_detect_gpio_config(struct msdc_host *host);
void msdc_gpio_config_pinmux(struct msdc_host *host);

#if 1
#define TST_CFG3  ((volatile u64 *)(base+0x000001A0))
#define TST_CFG1  ((volatile u64 *)(base+0x00000198))
#define AP_REG2   ((volatile u64 *)(base+0x00000014))
#define misc_ctrl2 ((volatile u64 *)(base+0x00000298))
#define PAD_MUX0   ((volatile u64 *)(base+0x00000054))
#define PAD_MUX1   ((volatile u64 *)(base+0x00000058))
#define PAD_MUX2   ((volatile u64 *)(base+0x0000005C))
#define PAD_MUX3   ((volatile u64 *)(base+0x00000060))
#define PAD_MUX4   ((volatile u64 *)(base+0x00000064))
#define PAD_MUX5   ((volatile u64 *)(base+0x00000068))
#define PAD_MUX6   ((volatile u64 *)(base+0x0000006C))
#define PAD_MUX7 ((volatile u64 *)(base+0x00000070))


//#define IO_BASE						0xFd000000
#define dma_channel_ctrl ((volatile u64 *)(base+0x0000801C))
	#define MSDC_CHA_SEL_CTRL      (0x1 << 1)

#define misc_ctrl ((volatile u64 *)(base+0x00000094))
	#define MSDC0_MISC_CTRL      (0x1 << 12)
	#define MSDC1_MISC_CTRL      (0x1 << 13)
	#define MSDC2_MISC_CTRL      (0x1 << 14)
	#define MSDC3_MISC_CTRL      (0x1 << 11)
#define MSDC_DMA_AGENT5 (0)
#define MSDC_DMA_AGENT6 (1)
#define MSDC_DMA_AGENT16 (2)


#define REG_PLLGP_CFG28 ((volatile u64 *)(base+0x000005f0))
	#define RG_MSDCPLL_FBSEL        (0x3 << 0)
	#define RG_MSDCPLL_CKTROL       (0x3 << 2)
	#define RG_MSDCPLL_POSDIV       (0x3 << 4)
	#define RG_MSDCPLL_PROEDIV      (0x3 << 6)
	#define RG_MSDCPLL_FBDIV        (0x7F << 8)
	#define RG_MSDCPLL_PWD          (0x1 << 15)

#define MSDC_CLK_CHOOSE0 ((volatile u64 *)(base+0x00000014))
#define MSDC_CLK_CHOOSE1 ((volatile u64 *)(base +0x00000008))
#define MSDC_CLK_CHOOSE2 ((volatile u64 *)(base +0x00000010))
#define MSDC_CLK_CHOOSE3 ((volatile u64 *)(base +0x0000000C))

	//AHB clock
	#define SD00_AP_SEL      (0x7<<3)
	#define SD10_AP_SEL      (0x7<<6)
	#define SD20_AP_SEL      (0x7<<9)
	#define SD30_AP_SEL      (0x7 << 3)
	//src clock
	#define SD01_AP_SEL      (0xf<<12)
	#define SD11_AP_SEL      (0xf<<16)
	#define SD21_AP_SEL      (0xf<<24)
	#define SD31_AP_SEL      (0xf << 4)

#define clkgate_cfg3 ((volatile u64 *)(base +0x000000A8))
	#define MSDC_0_CKEN      (0x1<<16)
	#define MSDC_1_CKEN      (0x1<<17)
	#define MSDC_2_CKEN      (0x1<<18)

#define clkgate_cfg4 ((volatile u64 *)(base +0x000000AC))
	#define MSDC_3_CKEN      (0x1 << 0)

#define GATE_ENABLE_CLOCK				(1)
#define GATE_DISABLE_CLOCK				(0)

#define misc_control ((volatile u64 *)(base +0x00000094))
	#define msdc0_gpio_mode_sel      (0x1<<9)
	#define msdc1_gpio_mode_sel      (0x1<<10)
	#define msdc2_gpio_mode_sel      (0x1<<11)


#define sync_reset_cfg3 ((volatile u64 *)(base +0x000000C4))
	#define MSDC_0_PDRST      (0x1<<16)
	#define MSDC_1_PDRST      (0x1<<17)
	#define MSDC_2_PDRST      (0x1<<18)
	#define MSDC_0_SWRST      (0x1<<19)
	#define MSDC_1_SWRST      (0x1<<20)
	#define MSDC_2_SWRST      (0x1<<21)
	
#define pd_reset_en_cfg3 ((volatile u64 *)(base +0x000000C8))
	#define MSDC_3_SWRST      (0x1 << 1)
	#define MSDC_3_SWRST_EN	  (0x1 << 0)

#define PAD_EN0  ((volatile u64 *)(base +0x00000074))
	#define MT6630_PMU_EN_CFG (0x1 << 31)
#define PAD_EN1 ((volatile u64 *)(base +0x00000078))
	#define MT6630_SYS_RST_CFG (0x1 << 10)
#define GPIO_OUT0 ((volatile u64 *)(base +0x000000E0))
#define GPIO_OUT1 ((volatile u64 *)(base +0x000000E4))

#define GPIOEN3   ((volatile u64 *)(base +0x00000080))  //sd0 clock
	#define pad_emmc_reset_output_en   (0x1<<1)



#define GPIOOUT3   ((volatile u64 *)(base +0x000000EC))  //sd0 clock
	#define pad_emmc_reset_output_value   (0x1<<1)



#define pad_msdc_cfg0   ((volatile u64 *)(base +0x000002c0))  //sd0 clock
	#define pad_sd0_clk_tdsel   (0xf<<23)
	#define pad_sd0_clk_rdsel	(0xff<<15)
	#define pad_sd0_clk_smit	(0x1<<14)
	#define pad_sd0_clk_r		(0x3<<12)
	#define pad_sd0_clk_pupd	(0x1<<11)
	#define pad_sd0_clk_ies		(0x1<<10)
	#define pad_sd0_clk_drv		(0x3f<<4)
	#define pad_sd0_clk_sr 		(0xf<<0)

#define pad_msdc_cfg1   ((volatile u64 *)(base +0x000002c4)) //sd0 cmd
	#define pad_sd0_cmd_tdsel   (0xf<<23)
	#define pad_sd0_cmd_rdsel	(0xff<<15)
	#define pad_sd0_cmd_smit	(0x1<<14)
	#define pad_sd0_cmd_r		(0x3<<12)
	#define pad_sd0_cmd_pupd	(0x1<<11)
	#define pad_sd0_cmd_ies		(0x1<<10)
	#define pad_sd0_cmd_drv		(0x3f<<4)
	#define pad_sd0_cmd_sr 		(0xf<<0)

#define pad_msdc_cfg2   ((volatile u64 *)(base +0x000002c8)) //sd0 data0
	#define pad_sd0_dat0_tdsel   (0xf<<23)
	#define pad_sd0_dat0_rdsel	 (0xff<<15)
	#define pad_sd0_dat0_smit	 (0x1<<14)
	#define pad_sd0_dat0_r		 (0x3<<12)
	#define pad_sd0_dat0_pupd	 (0x1<<11)
	#define pad_sd0_dat0_ies	 (0x1<<10)
	#define pad_sd0_dat0_drv	 (0x3f<<4)
	#define pad_sd0_dat0_sr 	 (0xf<<0)

#define pad_msdc_cfg3   ((volatile u64 *)(base +0x000002cc)) //sd0 data1
	#define pad_sd0_dat1_tdsel   (0xf<<23)
	#define pad_sd0_dat1_rdsel	 (0xff<<15)
	#define pad_sd0_dat1_smit	 (0x1<<14)
	#define pad_sd0_dat1_r		 (0x3<<12)
	#define pad_sd0_dat1_pupd	 (0x1<<11)
	#define pad_sd0_dat1_ies	 (0x1<<10)
	#define pad_sd0_dat1_drv	 (0x3f<<4)
	#define pad_sd0_dat1_sr 	 (0xf<<0)

#define pad_msdc_cfg4   ((volatile u64 *)(base +0x000002d0)) //sd0 data2
	#define pad_sd0_dat2_tdsel   (0xf<<23)
	#define pad_sd0_dat2_rdsel	 (0xff<<15)
	#define pad_sd0_dat2_smit	 (0x1<<14)
	#define pad_sd0_dat2_r		 (0x3<<12)
	#define pad_sd0_dat2_pupd	 (0x1<<11)
	#define pad_sd0_dat2_ies	 (0x1<<10)
	#define pad_sd0_dat2_drv	 (0x3f<<4)
	#define pad_sd0_dat2_sr 	 (0xf<<0)

#define pad_msdc_cfg5   ((volatile u64 *)(base +0x000002d4)) //sd0 data3
	#define pad_sd0_dat3_tdsel   (0xf<<23)
	#define pad_sd0_dat3_rdsel	 (0xff<<15)
	#define pad_sd0_dat3_smit	 (0x1<<14)
	#define pad_sd0_dat3_r		 (0x3<<12)
	#define pad_sd0_dat3_pupd	 (0x1<<11)
	#define pad_sd0_dat3_ies	 (0x1<<10)
	#define pad_sd0_dat3_drv	 (0x3f<<4)
	#define pad_sd0_dat3_sr 	 (0xf<<0)



#define pad_msdc_cfg6   ((volatile u64 *)(base +0x000002d8))  //sd1 clock
	#define pad_sd1_clk_tdsel   (0xf<<23)
	#define pad_sd1_clk_rdsel	(0xff<<15)
	#define pad_sd1_clk_smit	(0x1<<14)
	#define pad_sd1_clk_r		(0x3<<12)
	#define pad_sd1_clk_pupd	(0x1<<11)
	#define pad_sd1_clk_ies		(0x1<<10)
	#define pad_sd1_clk_drv		(0x3f<<4)
	#define pad_sd1_clk_sr 		(0xf<<0)

#define pad_msdc_cfg7   ((volatile u64 *)(base +0x000002dc)) //sd1 cmd
	#define pad_sd1_cmd_tdsel   (0xf<<23)
	#define pad_sd1_cmd_rdsel	(0xff<<15)
	#define pad_sd1_cmd_smit	(0x1<<14)
	#define pad_sd1_cmd_r		(0x3<<12)
	#define pad_sd1_cmd_pupd	(0x1<<11)
	#define pad_sd1_cmd_ies		(0x1<<10)
	#define pad_sd1_cmd_drv		(0x3f<<4)
	#define pad_sd1_cmd_sr 		(0xf<<0)

#define pad_msdc_cfg8   ((volatile u64 *)(base +0x000002e0)) //sd1 data0
	#define pad_sd1_dat0_tdsel   (0xf<<23)
	#define pad_sd1_dat0_rdsel	 (0xff<<15)
	#define pad_sd1_dat0_smit	 (0x1<<14)
	#define pad_sd1_dat0_r		 (0x3<<12)
	#define pad_sd1_dat0_pupd	 (0x1<<11)
	#define pad_sd1_dat0_ies	 (0x1<<10)
	#define pad_sd1_dat0_drv	 (0x3f<<4)
	#define pad_sd1_dat0_sr 	 (0xf<<0)

#define pad_msdc_cfg9   ((volatile u64 *)(base +0x000002e4)) //sd1 data1
	#define pad_sd1_dat1_tdsel   (0xf<<23)
	#define pad_sd1_dat1_rdsel	 (0xff<<15)
	#define pad_sd1_dat1_smit	 (0x1<<14)
	#define pad_sd1_dat1_r		 (0x3<<12)
	#define pad_sd1_dat1_pupd	 (0x1<<11)
	#define pad_sd1_dat1_ies	 (0x1<<10)
	#define pad_sd1_dat1_drv	 (0x3f<<4)
	#define pad_sd1_dat1_sr 	 (0xf<<0)

#define pad_msdc_cfg10   ((volatile u64 *)(base +0x000002e8)) //sd1 data2
	#define pad_sd1_dat2_tdsel   (0xf<<23)
	#define pad_sd1_dat2_rdsel	 (0xff<<15)
	#define pad_sd1_dat2_smit	 (0x1<<14)
	#define pad_sd1_dat2_r		 (0x3<<12)
	#define pad_sd1_dat2_pupd	 (0x1<<11)
	#define pad_sd1_dat2_ies	 (0x1<<10)
	#define pad_sd1_dat2_drv	 (0x3f<<4)
	#define pad_sd1_dat2_sr 	 (0xf<<0)

#define pad_msdc_cfg11   ((volatile u64 *)(base +0x000002ec)) //sd1 data3
	#define pad_sd1_dat3_tdsel   (0xf<<23)
	#define pad_sd1_dat3_rdsel	 (0xff<<15)
	#define pad_sd1_dat3_smit	 (0x1<<14)
	#define pad_sd1_dat3_r		 (0x3<<12)
	#define pad_sd1_dat3_pupd	 (0x1<<11)
	#define pad_sd1_dat3_ies	 (0x1<<10)
	#define pad_sd1_dat3_drv	 (0x3f<<4)
	#define pad_sd1_dat3_sr 	 (0xf<<0)


#define pad_msdc_cfg12   ((volatile u64 *)(base +0x000002f0))  //sd2 clock
	#define pad_sd2_clk_tdsel   (0xf<<23)
	#define pad_sd2_clk_rdsel	(0xff<<15)
	#define pad_sd2_clk_smit	(0x1<<14)
	#define pad_sd2_clk_r		(0x3<<12)
	#define pad_sd2_clk_pupd	(0x1<<11)
	#define pad_sd2_clk_ies		(0x1<<10)
	#define pad_sd2_clk_drv		(0x3f<<4)
	#define pad_sd2_clk_sr 		(0xf<<0)

#define pad_msdc_cfg13   ((volatile u64 *)(base +0x000002f4)) //sd2 cmd
	#define pad_sd2_cmd_tdsel   (0xf<<23)
	#define pad_sd2_cmd_rdsel	(0xff<<15)
	#define pad_sd2_cmd_smit	(0x1<<14)
	#define pad_sd2_cmd_r		(0x3<<12)
	#define pad_sd2_cmd_pupd	(0x1<<11)
	#define pad_sd2_cmd_ies		(0x1<<10)
	#define pad_sd2_cmd_drv		(0x3f<<4)
	#define pad_sd2_cmd_sr 		(0xf<<0)

#define pad_msdc_cfg14   ((volatile u64 *)(base +0x000002f8)) //sd2 data0
	#define pad_sd2_dat0_tdsel   (0xf<<23)
	#define pad_sd2_dat0_rdsel	 (0xff<<15)
	#define pad_sd2_dat0_smit	 (0x1<<14)
	#define pad_sd2_dat0_r		 (0x3<<12)
	#define pad_sd2_dat0_pupd	 (0x1<<11)
	#define pad_sd2_dat0_ies	 (0x1<<10)
	#define pad_sd2_dat0_drv	 (0x3f<<4)
	#define pad_sd2_dat0_sr 	 (0xf<<0)

#define pad_msdc_cfg15   ((volatile u64 *)(base +0x000002fc)) //sd2 data1
	#define pad_sd2_dat1_tdsel   (0xf<<23)
	#define pad_sd2_dat1_rdsel	 (0xff<<15)
	#define pad_sd2_dat1_smit	 (0x1<<14)
	#define pad_sd2_dat1_r		 (0x3<<12)
	#define pad_sd2_dat1_pupd	 (0x1<<11)
	#define pad_sd2_dat1_ies	 (0x1<<10)
	#define pad_sd2_dat1_drv	 (0x3f<<4)
	#define pad_sd2_dat1_sr 	 (0xf<<0)

#define pad_msdc_cfg16   ((volatile u64 *)(base +0x00000300)) //sd2 data2
	#define pad_sd2_dat2_tdsel   (0xf<<23)
	#define pad_sd2_dat2_rdsel	 (0xff<<15)
	#define pad_sd2_dat2_smit	 (0x1<<14)
	#define pad_sd2_dat2_r		 (0x3<<12)
	#define pad_sd2_dat2_pupd	 (0x1<<11)
	#define pad_sd2_dat2_ies	 (0x1<<10)
	#define pad_sd2_dat2_drv	 (0x3f<<4)
	#define pad_sd2_dat2_sr 	 (0xf<<0)

#define pad_msdc_cfg17   ((volatile u64 *)(base +0x00000304)) //sd2 data3
	#define pad_sd2_dat3_tdsel   (0xf<<23)
	#define pad_sd2_dat3_rdsel	 (0xff<<15)
	#define pad_sd2_dat3_smit	 (0x1<<14)
	#define pad_sd2_dat3_r		 (0x3<<12)
	#define pad_sd2_dat3_pupd	 (0x1<<11)
	#define pad_sd2_dat3_ies	 (0x1<<10)
	#define pad_sd2_dat3_drv	 (0x3f<<4)
	#define pad_sd2_dat3_sr 	 (0xf<<0)

#define pad_msdc_cfg18	 ((volatile u64 *)(base +0x00000308))  //pin control
	#define pad_sd2_reset_gpio_ctl      (0x1<<31)
	#define pad_sd1_reset_gpio_ctl      (0x1<<30)
	#define pad_sd_8bit_reset_gpio_ctl  (0x1<<29)
	#define pad_sd0_reset_gpio_ctl      (0x1<<28)
	#define pad_sd2_gpio_ctl_9_0      	(0x3ff<<18)
	#define pad_sd2_gpio_ctl_5_0      	(0x3f<<12)
	#define pad_sd1_gpio_ctl_5_0      	(0x3f<<6)
	#define pad_sd0_gpio_ctl_5_0      	(0x3f<<0)


#define pad_msdc_cfg19   ((volatile u64 *)(base +0x0000030c))  //sd0 8bit clock
	#define pad_sd0_8bit_clk_tdsel   (0xf<<23)
	#define pad_sd0_8bit_clk_rdsel	(0xff<<15)
	#define pad_sd0_8bit_clk_smit	(0x1<<14)
	#define pad_sd0_8bit_clk_r		(0x3<<12)
	#define pad_sd0_8bit_clk_pupd	(0x1<<11)
	#define pad_sd0_8bit_clk_ies		(0x1<<10)
	#define pad_sd0_8bit_clk_drv		(0x3f<<4)
	#define pad_sd0_8bit_clk_sr 		(0xf<<0)

#define pad_msdc_cfg20   ((volatile u64 *)(base +0x00000310)) //sd0 8bit  cmd
	#define pad_sd0_8bit_cmd_tdsel   (0xf<<23)
	#define pad_sd0_8bit_cmd_rdsel	(0xff<<15)
	#define pad_sd0_8bit_cmd_smit	(0x1<<14)
	#define pad_sd0_8bit_cmd_r		(0x3<<12)
	#define pad_sd0_8bit_cmd_pupd	(0x1<<11)
	#define pad_sd0_8bit_cmd_ies		(0x1<<10)
	#define pad_sd0_8bit_cmd_drv		(0x3f<<4)
	#define pad_sd0_8bit_cmd_sr 		(0xf<<0)

#define pad_msdc_cfg21   ((volatile u64 *)(base +0x00000314)) //sd0 8bit  data0
	#define pad_sd0_8bit_dat0_tdsel   (0xf<<23)
	#define pad_sd0_8bit_dat0_rdsel	 (0xff<<15)
	#define pad_sd0_8bit_dat0_smit	 (0x1<<14)
	#define pad_sd0_8bit_dat0_r		 (0x3<<12)
	#define pad_sd0_8bit_dat0_pupd	 (0x1<<11)
	#define pad_sd0_8bit_dat0_ies	 (0x1<<10)
	#define pad_sd0_8bit_dat0_drv	 (0x3f<<4)
	#define pad_sd0_8bit_dat0_sr 	 (0xf<<0)

#define pad_msdc_cfg22   ((volatile u64 *)(base +0x00000318)) //sd0 8bit data1
	#define pad_sd0_8bit_dat1_tdsel   (0xf<<23)
	#define pad_sd0_8bit_dat1_rdsel	 (0xff<<15)
	#define pad_sd0_8bit_dat1_smit	 (0x1<<14)
	#define pad_sd0_8bit_dat1_r		 (0x3<<12)
	#define pad_sd0_8bit_dat1_pupd	 (0x1<<11)
	#define pad_sd0_8bit_dat1_ies	 (0x1<<10)
	#define pad_sd0_8bit_dat1_drv	 (0x3f<<4)
	#define pad_sd0_8bit_dat1_sr 	 (0xf<<0)

#define pad_msdc_cfg23   ((volatile u64 *)(base +0x0000031c)) //sd0 8bit  data2
	#define pad_sd0_8bit_dat2_tdsel   (0xf<<23)
	#define pad_sd0_8bit_dat2_rdsel	 (0xff<<15)
	#define pad_sd0_8bit_dat2_smit	 (0x1<<14)
	#define pad_sd0_8bit_dat2_r		 (0x3<<12)
	#define pad_sd0_8bit_dat2_pupd	 (0x1<<11)
	#define pad_sd0_8bit_dat2_ies	 (0x1<<10)
	#define pad_sd0_8bit_dat2_drv	 (0x3f<<4)
	#define pad_sd0_8bit_dat2_sr 	 (0xf<<0)

#define pad_msdc_cfg24   ((volatile u64 *)(base +0x00000320)) //sd0 8bit  data3
	#define pad_sd0_8bit_dat3_tdsel   (0xf<<23)
	#define pad_sd0_8bit_dat3_rdsel	 (0xff<<15)
	#define pad_sd0_8bit_dat3_smit	 (0x1<<14)
	#define pad_sd0_8bit_dat3_r		 (0x3<<12)
	#define pad_sd0_8bit_dat3_pupd	 (0x1<<11)
	#define pad_sd0_8bit_dat3_ies	 (0x1<<10)
	#define pad_sd0_8bit_dat3_drv	 (0x3f<<4)
	#define pad_sd0_8bit_dat3_sr 	 (0xf<<0)

#define pad_msdc_cfg25   ((volatile u64 *)(base +0x00000324)) //sd0 8bit  data4
	#define pad_sd0_8bit_dat0_tdsel   (0xf<<23)
	#define pad_sd0_8bit_dat0_rdsel	 (0xff<<15)
	#define pad_sd0_8bit_dat0_smit	 (0x1<<14)
	#define pad_sd0_8bit_dat0_r		 (0x3<<12)
	#define pad_sd0_8bit_dat0_pupd	 (0x1<<11)
	#define pad_sd0_8bit_dat0_ies	 (0x1<<10)
	#define pad_sd0_8bit_dat0_drv	 (0x3f<<4)
	#define pad_sd0_8bit_dat0_sr 	 (0xf<<0)

#define pad_msdc_cfg26   ((volatile u64 *)(base +0x00000328)) //sd0 8bit data5
	#define pad_sd0_8bit_dat1_tdsel   (0xf<<23)
	#define pad_sd0_8bit_dat1_rdsel	 (0xff<<15)
	#define pad_sd0_8bit_dat1_smit	 (0x1<<14)
	#define pad_sd0_8bit_dat1_r		 (0x3<<12)
	#define pad_sd0_8bit_dat1_pupd	 (0x1<<11)
	#define pad_sd0_8bit_dat1_ies	 (0x1<<10)
	#define pad_sd0_8bit_dat1_drv	 (0x3f<<4)
	#define pad_sd0_8bit_dat1_sr 	 (0xf<<0)

#define pad_msdc_cfg27   ((volatile u64 *)(base +0x0000032c)) //sd0 8bit  data6
	#define pad_sd0_8bit_dat2_tdsel   (0xf<<23)
	#define pad_sd0_8bit_dat2_rdsel	 (0xff<<15)
	#define pad_sd0_8bit_dat2_smit	 (0x1<<14)
	#define pad_sd0_8bit_dat2_r		 (0x3<<12)
	#define pad_sd0_8bit_dat2_pupd	 (0x1<<11)
	#define pad_sd0_8bit_dat2_ies	 (0x1<<10)
	#define pad_sd0_8bit_dat2_drv	 (0x3f<<4)
	#define pad_sd0_8bit_dat2_sr 	 (0xf<<0)

#define pad_msdc_cfg28   ((volatile u64 *)(base +0x00000330)) //sd0 8bit  data7
	#define pad_sd0_8bit_dat3_tdsel   (0xf<<23)
	#define pad_sd0_8bit_dat3_rdsel	 (0xff<<15)
	#define pad_sd0_8bit_dat3_smit	 (0x1<<14)
	#define pad_sd0_8bit_dat3_r		 (0x3<<12)
	#define pad_sd0_8bit_dat3_pupd	 (0x1<<11)
	#define pad_sd0_8bit_dat3_ies	 (0x1<<10)
	#define pad_sd0_8bit_dat3_drv	 (0x3f<<4)
	#define pad_sd0_8bit_dat3_sr 	 (0xf<<0)



#define pad_msdc_cfg29   ((volatile u64 *)(base +0x00000334)) //sd0 rst
	#define pad_sd0_rst_tdsel   (0xf<<23)
	#define pad_sd0_rst_rdsel	 (0xff<<15)
	#define pad_sd0_rst_smit	 (0x1<<14)
	#define pad_sd0_rst_r		 (0x3<<12)
	#define pad_sd0_rst_pupd	 (0x1<<11)
	#define pad_sd0_rst_ies	 (0x1<<10)
	#define pad_sd0_rst_drv	 (0x3f<<4)
	#define pad_sd0_rst_sr 	 (0xf<<0)

#define pad_msdc_cfg30   ((volatile u64 *)(base +0x00000338)) //sd0 8bit  rst
	#define pad_sd0_8bit_rst_tdsel   (0xf<<23)
	#define pad_sd0_8bit_rst_rdsel	 (0xff<<15)
	#define pad_sd0_8bit_rst_smit	 (0x1<<14)
	#define pad_sd0_8bit_rst_r		 (0x3<<12)
	#define pad_sd0_8bit_rst_pupd	 (0x1<<11)
	#define pad_sd0_8bit_rst_ies	 (0x1<<10)
	#define pad_sd0_8bit_rst_drv	 (0x3f<<4)
	#define pad_sd0_8bit_rst_sr 	 (0xf<<0)

#define pad_msdc_cfg31   ((volatile u64 *)(base +0x0000033c)) //sd1 rst
	#define pad_sd1_rst_tdsel   (0xf<<23)
	#define pad_sd1_rst_rdsel	 (0xff<<15)
	#define pad_sd1_rst_smit	 (0x1<<14)
	#define pad_sd1_rst_r		 (0x3<<12)
	#define pad_sd1_rst_pupd	 (0x1<<11)
	#define pad_sd1_rst_ies	 (0x1<<10)
	#define pad_sd1_rst_drv	 (0x3f<<4)
	#define pad_sd1_rst_sr 	 (0xf<<0)


#define pad_msdc_cfg32   ((volatile u64 *)(base +0x00000340)) //sd2 rst
	#define pad_sd2_rst_tdsel   (0xf<<23)
	#define pad_sd2_rst_rdsel	 (0xff<<15)
	#define pad_sd2_rst_smit	 (0x1<<14)
	#define pad_sd2_rst_r		 (0x3<<12)
	#define pad_sd2_rst_pupd	 (0x1<<11)
	#define pad_sd2_rst_ies	 (0x1<<10)
	#define pad_sd2_rst_drv	 (0x3f<<4)
	#define pad_sd2_rst_sr 	 (0xf<<0)

#define pad_msdc_cfg36 ((volatile u64 *)(base +0x00000350))
	#define pad_msdc_cfg36_8_4      (0x1<<0)  //sd1's data used for sd0 high 4bit data
	#define pad_sd0_rst_rxdly_ctrl  (0x1f<<4)
	#define pad_sd0_8bit_rst_rxdly_ctrl  (0x1f<<9)
	#define pad_sd1_rst_rxdly_ctrl  (0x1f<<14)
	#define pad_sd2_rst_rxdly_ctrl  (0x1f<<19)


	#define pad_tdsel	(0xf<<23)
	#define pad_rdsel	(0xff<<15)
	#define pad_smit	(0x1<<14)
	#define pad_r		(0x3<<12)
	#define pad_pupd	(0x1<<11)
	#define pad_ies 	(0x1<<10)
	#define pad_drv 	(0x3f<<4)
	#define pad_sr		(0xf<<0)



#define pad_msdc_cfg36 ((volatile u64 *)(base +0x00000350))
	#define pad_msdc_cfg36_8_4      (0x1<<0)  //sd1's data used for sd0 high 4bit data
	#define pad_sd0_rst_rxdly_ctrl  (0x1f<<4)
	#define pad_sd0_8bit_rst_rxdly_ctrl  (0x1f<<9)
	#define pad_sd1_rst_rxdly_ctrl  (0x1f<<14)
	#define pad_sd2_rst_rxdly_ctrl  (0x1f<<19)


	#define pad_tdsel	(0xf<<23)
	#define pad_rdsel	(0xff<<15)
	#define pad_smit	(0x1<<14)
	#define pad_r		(0x3<<12)
	#define pad_pupd	(0x1<<11)
	#define pad_ies 	(0x1<<10)
	#define pad_drv 	(0x3f<<4)
	#define pad_sr		(0xf<<0)


#define gpio_enable_output  ((volatile u64 *)(base +0x00000080))  //gpio0
	#define sd_v33_18_sw0_enable  (0x1<<18)
	#define sd_v33_18_sw1_enable  (0x1<<19)
	#define sd_v33_18_sw2_enable  (0x1<<20)


#define gpio_output_value_fix   ((volatile u64 *)(base +0x000000Ec))
	#define sd_v33_18_sw0_value  (0x1<<18)
	#define sd_v33_18_sw1_value  (0x1<<19)
	#define sd_v33_18_sw2_value  (0x1<<20)

#endif
void msdc_sd_power_switch(struct msdc_host *host, u32 on);
void msdc_sdio_power_switch(struct msdc_host *host, u32 on);

void msdc_emmc_power(struct msdc_host *host, u32 on);
void msdc_sd_power(struct msdc_host *host, u32 on);
void msdc_sdio_power(struct msdc_host *host, u32 on);
void msdc_dump_ldo_sts(struct msdc_host *host);


extern u32 g_msdc0_io;
extern u32 g_msdc0_flash;
extern u32 g_msdc1_io;
extern u32 g_msdc1_flash;
extern u32 g_msdc2_io;
extern u32 g_msdc2_flash;
extern u32 g_msdc3_io;
extern u32 g_msdc3_flash;

/**************************************************************/
/* Section 3: Clock                                           */
/**************************************************************/
#define PLLCLK_6_65M  6650000
#define PLLCLK_13_5M  13500000
#define PLLCLK_27M  27000000
#define PLLCLK_36M  36000000
#define PLLCLK_48M  48000000
#define PLLCLK_50M  50000000
#define PLLCLK_54M  54000000
#define PLLCLK_60M  60000000
#define PLLCLK_72M  72000000
#define PLLCLK_74M  74000000
#define PLLCLK_80M  80000000
#define PLLCLK_81M  81000000
#define PLLCLK_98M  98000000
#define PLLCLK_100M  100000000
#define PLLCLK_108M  108000000
#define PLLCLK_120M  120000000
#define PLLCLK_135M  135000000
#define PLLCLK_147M  147000000
#define PLLCLK_162M  162000000
#define PLLCLK_189M  189000000
#define PLLCLK_200M  200000000
#define PLLCLK_202M  202000000
#define PLLCLK_270M  270000000
#define PLLCLK_294M  294000000
#define PLLCLK_324M 324000000
#define PLLCLK_400M 400000000


void msdc_dump_clock_sts(struct msdc_host *host);
/* MSDCPLL register offset */
#define MSDCPLL_CON0_OFFSET             (0x250)
#define MSDCPLL_CON1_OFFSET             (0x254)
#define MSDCPLL_CON2_OFFSET             (0x258)
#define MSDCPLL_PWR_CON0_OFFSET         (0x25c)
/* Clock config register offset */
#define MSDC_CLK_CFG_3_OFFSET           (0x070)

#define MSDC_PERI_PDN_SET0_OFFSET       (0x0008)
#define MSDC_PERI_PDN_CLR0_OFFSET       (0x0010)
#define MSDC_PERI_PDN_STA0_OFFSET       (0x0018)

extern u32 hclks_msdc50_0[];
extern u32 hclks_msdc30_1[];
extern u32 hclks_msdc30_2[];
extern u32 hclks_msdc50[];
extern u32 hclks_msdc30[];

#define msdc_get_hclks(id) \
	((id == 0) ? hclks_msdc50 : \
	    hclks_msdc30)

#define msdc_clk_enable(host) clk_enable(host->clock_control)
#define msdc_clk_disable(host) clk_disable(host->clock_control)
int msdc_get_ccf_clk_pointer(struct platform_device *pdev,
		struct msdc_host *host);


/**************************************************************/
/* Section 4: GPIO and Pad                                    */
/**************************************************************/
/*******************************************************************************
 *PINMUX and GPIO definition
 ******************************************************************************/
#define MSDC_PIN_PULL_NONE      (0)
#define MSDC_PIN_PULL_DOWN      (1)
#define MSDC_PIN_PULL_UP        (2)
#define MSDC_PIN_KEEP           (3)
/* add pull down/up mode define */
#define MSDC_GPIO_PULL_UP       (0)
#define MSDC_GPIO_PULL_DOWN     (1)
/*--------------------------------------------------------------------------*/
/* MSDC0~1 GPIO and IO Pad Configuration Base                               */
/*--------------------------------------------------------------------------*/
#define MSDC_GPIO_BASE		gpio_base
#define MSDC0_IO_PAD_BASE	(io_cfg_b_base)
#define MSDC1_IO_PAD_BASE	(io_cfg_b_base)
/*--------------------------------------------------------------------------*/
/* MSDC GPIO Related Register                                               */
/*--------------------------------------------------------------------------*/
#define MSDC0_GPIO_MODE14	(MSDC_GPIO_BASE   +  0x3e0)
#define MSDC0_GPIO_MODE15	(MSDC_GPIO_BASE   +  0x3f0)
#define MSDC1_GPIO_MODE16	(MSDC_GPIO_BASE   +  0x400)

#define MSDC0_GPIO_SMT_ADDR	(MSDC0_IO_PAD_BASE + 0x060)
#define MSDC0_GPIO_IES_ADDR	(MSDC0_IO_PAD_BASE + 0x020)
#define MSDC0_GPIO_PUPD_ADDR	(MSDC0_IO_PAD_BASE + 0x100)
#define MSDC0_GPIO_R0_ADDR	(MSDC0_IO_PAD_BASE + 0x110)
#define MSDC0_GPIO_R1_ADDR	(MSDC0_IO_PAD_BASE + 0x120)
#define MSDC0_GPIO_SR_ADDR	(MSDC0_IO_PAD_BASE + 0x030)
#define MSDC0_GPIO_TDSEL_ADDR	(MSDC0_IO_PAD_BASE + 0x0d0)
#define MSDC0_GPIO_RDSEL_ADDR	(MSDC0_IO_PAD_BASE + 0x080)
#define MSDC0_GPIO_DRV_ADDR	(MSDC0_IO_PAD_BASE + 0x1a0)
/* msdc1 smt is in IO_CFG_R register map */
#define MSDC1_GPIO_SMT_ADDR	(io_cfg_r_base     + 0x030)
#define MSDC1_GPIO_IES_ADDR	(MSDC1_IO_PAD_BASE + 0x020)
#define MSDC1_GPIO_PUPD_ADDR	(MSDC1_IO_PAD_BASE + 0x100)
#define MSDC1_GPIO_R0_ADDR	(MSDC1_IO_PAD_BASE + 0x110)
#define MSDC1_GPIO_R1_ADDR	(MSDC1_IO_PAD_BASE + 0x120)
#define MSDC1_GPIO_SR_ADDR	(MSDC1_IO_PAD_BASE + 0x030)
#define MSDC1_GPIO_TDSEL_ADDR	(MSDC1_IO_PAD_BASE + 0x0c0)
#define MSDC1_GPIO_RDSEL_ADDR	(MSDC1_IO_PAD_BASE + 0x0a0)
#define MSDC1_GPIO_DRV_ADDR	(MSDC1_IO_PAD_BASE + 0x1b0)
/*--------------------------------------------------------------------------*/
/* MSDC GPIO Related Register Mask                                               */
/*--------------------------------------------------------------------------*/
/* MSDC0_GPIO_MODE14, 001b is msdc mode*/
#define MSDC0_MODE_DAT5_MASK            (0xf << 28)
#define MSDC0_MODE_DAT4_MASK            (0xf << 24)
#define MSDC0_MODE_DAT3_MASK            (0xf << 20)
#define MSDC0_MODE_DAT2_MASK            (0xf << 16)
#define MSDC0_MODE_DAT1_MASK            (0xf << 12)
#define MSDC0_MODE_DAT0_MASK            (0xf << 8)
/* MSDC0_GPIO_MODE15, 001b is msdc mode */
#define MSDC0_MODE_RSTB_MASK            (0xf << 20)
#define MSDC0_MODE_DSL_MASK             (0xf << 16)
#define MSDC0_MODE_CLK_MASK             (0xf << 12)
#define MSDC0_MODE_CMD_MASK             (0xf << 8)
#define MSDC0_MODE_DAT7_MASK            (0xf << 4)
#define MSDC0_MODE_DAT6_MASK            (0xf << 0)
/* MSDC1_GPIO_MODE16, 0001b is msdc mode */
#define MSDC1_MODE_CMD_MASK             (0xf << 4)
#define MSDC1_MODE_DAT0_MASK            (0xf << 8)
#define MSDC1_MODE_DAT1_MASK            (0xf << 12)
#define MSDC1_MODE_DAT2_MASK            (0xf << 16)
#define MSDC1_MODE_DAT3_MASK            (0xf << 20)
#define MSDC1_MODE_CLK_MASK             (0xf << 24)

/* MSDC0 SMT mask*/
#define MSDC0_SMT_DAT3_0_MASK            (0x1  <<  0)
#define MSDC0_SMT_DAT7_4_MASK            (0x1  <<  1)
#define MSDC0_SMT_CMD_DSL_RSTB_MASK      (0x1  <<  2)
#define MSDC0_SMT_CLK_MASK               (0x1  <<  3)
#define MSDC0_SMT_ALL_MASK               (0xf <<  0)
/* MSDC1 SMT mask. is in IO_CFG_R register map*/
#define MSDC1_SMT_CMD_MASK             (0x1 << 0)
#define MSDC1_SMT_DAT_MASK             (0x1 << 1)
#define MSDC1_SMT_CLK_MASK             (0x1 << 2)
#define MSDC1_SMT_ALL_MASK             (0x7 << 0)

/* MSDC0 IES mask*/
#define MSDC0_IES_DAT0_MASK              (0x1  <<  0)
#define MSDC0_IES_DAT1_MASK              (0x1  <<  1)
#define MSDC0_IES_DAT2_MASK              (0x1  <<  2)
#define MSDC0_IES_DAT3_MASK              (0x1  <<  3)
#define MSDC0_IES_DAT4_MASK              (0x1  <<  4)
#define MSDC0_IES_DAT5_MASK              (0x1  <<  5)
#define MSDC0_IES_DAT6_MASK              (0x1  <<  6)
#define MSDC0_IES_DAT7_MASK              (0x1  <<  7)
#define MSDC0_IES_CMD_MASK               (0x1  <<  8)
#define MSDC0_IES_CLK_MASK               (0x1  <<  9)
#define MSDC0_IES_DSL_MASK               (0x1  <<  10)
#define MSDC0_IES_RSTB_MASK              (0x1  <<  11)
#define MSDC0_IES_ALL_MASK               (0xfff << 0)
/* MSDC1 IES mask*/
#define MSDC1_IES_CMD_MASK             (0x1 << 25)
#define MSDC1_IES_DAT0_MASK            (0x1 << 26)
#define MSDC1_IES_DAT1_MASK            (0x1 << 27)
#define MSDC1_IES_DAT2_MASK            (0x1 << 28)
#define MSDC1_IES_DAT3_MASK            (0x1 << 29)
#define MSDC1_IES_CLK_MASK             (0x1 << 30)
#define MSDC1_IES_ALL_MASK             (0x3f << 25)

/* MSDC0 PUPD mask*/
#define MSDC0_PUPD_DAT0_MASK            (0x1  << 6)
#define MSDC0_PUPD_DAT1_MASK            (0x1  << 7)
#define MSDC0_PUPD_DAT2_MASK            (0x1  << 8)
#define MSDC0_PUPD_DAT3_MASK            (0x1  << 9)
#define MSDC0_PUPD_DAT4_MASK            (0x1  << 10)
#define MSDC0_PUPD_DAT5_MASK            (0x1  << 11)
#define MSDC0_PUPD_DAT6_MASK            (0x1  << 12)
#define MSDC0_PUPD_DAT7_MASK            (0x1  << 13)
#define MSDC0_PUPD_CMD_MASK             (0x1  << 14)
#define MSDC0_PUPD_CLK_MASK             (0x1  << 15)
#define MSDC0_PUPD_DSL_MASK             (0x1  << 16)
#define MSDC0_PUPD_RSTB_MASK            (0x1  << 17)
#define MSDC0_PUPD_DAT_MASK             (0xff << 6)
#define MSDC0_PUPD_CMD_DAT_MASK         (0x1FF << 6)
#define MSDC0_PUPD_CLK_DSL_MASK         (0x3  << 15)
#define MSDC0_PUPD_ALL_MASK             (0xfff << 6)
/* MSDC0 R0 mask*/
#define MSDC0_R0_DAT0_MASK            (0x1  << 6)
#define MSDC0_R0_DAT1_MASK            (0x1  << 7)
#define MSDC0_R0_DAT2_MASK            (0x1  << 8)
#define MSDC0_R0_DAT3_MASK            (0x1  << 9)
#define MSDC0_R0_DAT4_MASK            (0x1  << 10)
#define MSDC0_R0_DAT5_MASK            (0x1  << 11)
#define MSDC0_R0_DAT6_MASK            (0x1  << 12)
#define MSDC0_R0_DAT7_MASK            (0x1  << 13)
#define MSDC0_R0_CMD_MASK             (0x1  << 14)
#define MSDC0_R0_CLK_MASK             (0x1  << 15)
#define MSDC0_R0_DSL_MASK             (0x1  << 16)
#define MSDC0_R0_RSTB_MASK            (0x1  << 17)
#define MSDC0_R0_DAT_MASK             (0xff << 6)
#define MSDC0_R0_CMD_DAT_MASK         (0x1FF << 6)
#define MSDC0_R0_CLK_DSL_MASK         (0x3  << 15)
#define MSDC0_R0_ALL_MASK             (0xfff << 6)
/* MSDC0 R1 mask*/
#define MSDC0_R1_DAT0_MASK            (0x1  << 6)
#define MSDC0_R1_DAT1_MASK            (0x1  << 7)
#define MSDC0_R1_DAT2_MASK            (0x1  << 8)
#define MSDC0_R1_DAT3_MASK            (0x1  << 9)
#define MSDC0_R1_DAT4_MASK            (0x1  << 10)
#define MSDC0_R1_DAT5_MASK            (0x1  << 11)
#define MSDC0_R1_DAT6_MASK            (0x1  << 12)
#define MSDC0_R1_DAT7_MASK            (0x1  << 13)
#define MSDC0_R1_CMD_MASK             (0x1  << 14)
#define MSDC0_R1_CLK_MASK             (0x1  << 15)
#define MSDC0_R1_DSL_MASK             (0x1  << 16)
#define MSDC0_R1_RSTB_MASK            (0x1  << 17)
#define MSDC0_R1_DAT_MASK             (0xff << 6)
#define MSDC0_R1_CMD_DAT_MASK         (0x1FF << 6)
#define MSDC0_R1_CLK_DSL_MASK         (0x3  << 15)
#define MSDC0_R1_ALL_MASK             (0xfff << 6)
/* MSDC1 PUPD mask*/
#define MSDC1_PUPD_CMD_MASK             (0x1  << 18)
#define MSDC1_PUPD_DAT0_MASK            (0x1  << 19)
#define MSDC1_PUPD_DAT1_MASK            (0x1  << 20)
#define MSDC1_PUPD_DAT2_MASK            (0x1  << 21)
#define MSDC1_PUPD_DAT3_MASK            (0x1  << 22)
#define MSDC1_PUPD_CMD_DAT_MASK         (0x1f << 18)
#define MSDC1_PUPD_CLK_MASK             (0x1  << 23)
#define MSDC1_PUPD_ALL_MASK             (0x3f << 18)
/* MSDC1 R0 mask*/
#define MSDC1_R0_CMD_MASK             (0x1  << 18)
#define MSDC1_R0_DAT0_MASK            (0x1  << 19)
#define MSDC1_R0_DAT1_MASK            (0x1  << 20)
#define MSDC1_R0_DAT2_MASK            (0x1  << 21)
#define MSDC1_R0_DAT3_MASK            (0x1  << 22)
#define MSDC1_R0_CMD_DAT_MASK         (0x1f << 18)
#define MSDC1_R0_CLK_MASK             (0x1  << 23)
#define MSDC1_R0_ALL_MASK             (0x3f << 18)
/* MSDC1 R1 mask*/
#define MSDC1_R1_CMD_MASK             (0x1  << 18)
#define MSDC1_R1_DAT0_MASK            (0x1  << 19)
#define MSDC1_R1_DAT1_MASK            (0x1  << 20)
#define MSDC1_R1_DAT2_MASK            (0x1  << 21)
#define MSDC1_R1_DAT3_MASK            (0x1  << 22)
#define MSDC1_R1_CMD_DAT_MASK         (0x1f << 18)
#define MSDC1_R1_CLK_MASK             (0x1  << 23)
#define MSDC1_R1_ALL_MASK             (0x3f << 18)

/* MSDC0 SR mask*/
#define MSDC0_SR_DAT3_0_MASK            (0x1  << 15)
#define MSDC0_SR_DAT7_4_MASK            (0x1  << 16)
#define MSDC0_SR_CMD_DSL_RSTB_MASK      (0x1  << 17)
#define MSDC0_SR_CLK_MASK               (0x1  << 18)
#define MSDC0_SR_ALL_MASK               (0xf  << 15)
/* MSDC1 SR mask*/
#define MSDC1_SR_CMD_MASK               (0x1 << 29)
#define MSDC1_SR_DAT_MASK               (0x1 << 30)
#define MSDC1_SR_CLK_MASK               (0x1 << 31)

/* MSDC0 TDSEL mask*/
#define MSDC0_TDSEL_DAT3_0_MASK         (0xf  <<  0)
#define MSDC0_TDSEL_DAT7_4_MASK         (0xf  <<  4)
#define MSDC0_TDSEL_CMD_DSL_RSTB_MASK   (0xf  <<  8)
#define MSDC0_TDSEL_CLK_MASK            (0xf  <<  12)
#define MSDC0_TDSEL_ALL_MASK            (0xffff << 0)
/* MSDC1 TDSEL mask*/
#define MSDC1_TDSEL_CMD_MASK            (0xf << 16)
#define MSDC1_TDSEL_DAT_MASK            (0xf << 20)
#define MSDC1_TDSEL_CLK_MASK            (0xf << 24)
#define MSDC1_TDSEL_ALL_MASK            (0xfff << 16)

/* MSDC0 RDSEL mask*/
#define MSDC0_RDSEL_DAT3_0_MASK         (0x3f <<  0)
#define MSDC0_RDSEL_DAT7_4_MASK         (0x3f <<  6)
#define MSDC0_RDSEL_CLK_CMD_DSL_RSTB_MASK  (0x3f <<  12)
#define MSDC0_RDSEL_ALL_MASK            (0x3ffff << 0)
/* MSDC1 RDSEL mask*/
#define MSDC1_RDSEL_CMD_MASK            (0x3f << 8)
#define MSDC1_RDSEL_DAT_MASK            (0x3f << 14)
#define MSDC1_RDSEL_CLK_MASK            (0x3f << 20)
#define MSDC1_RDSEL_ALL_MASK            (0x3ffff << 8)

/* MSDC0 DRV mask*/
#define MSDC0_DRV_DAT3_0_MASK           (0x7  <<  0)
#define MSDC0_DRV_DAT7_4_MASK           (0x7  <<  3)
#define MSDC0_DRV_DAT_MASK              (0x3f <<  0)
#define MSDC0_DRV_CMD_DSL_RSTB_MASK     (0x7  <<  6)
#define MSDC0_DRV_CLK_MASK              (0x7  <<  9)
#define MSDC0_DRV_ALL_MASK              (0xfff << 0)
/* MSDC1 DRV mask*/
#define MSDC1_DRV_CMD_MASK            (0x7 << 21)
#define MSDC1_DRV_DAT_MASK            (0x7 << 24)
#define MSDC1_DRV_CLK_MASK            (0x7 << 27)
#define MSDC1_DRV_ALL_MASK            (0x1ff << 21)
/* MSDC1 BIAS Tune mask */
#define MSDC1_BIAS_MASK               (0x1f << 16)

void msdc_set_driving_by_id(u32 id, struct msdc_hw *hw, bool sd_18);
void msdc_set_driving(struct msdc_host *host, struct msdc_hw *hw, bool sd_18);
void msdc_get_driving_by_id(u32 id, struct msdc_hw *hw);
void msdc_set_ies_by_id(u32 id, int set_ies);
void msdc_set_sr_by_id(u32 id, int clk, int cmd, int dat);
void msdc_set_smt_by_id(u32 id, int set_smt);
void msdc_set_tdsel_by_id(u32 id, bool sleep, bool sd_18);
void msdc_set_rdsel_by_id(u32 id, bool sleep, bool sd_18);
void msdc_set_tdsel_dbg_by_id(u32 id, u32 value);
void msdc_set_rdsel_dbg_by_id(u32 id, u32 value);
void msdc_get_tdsel_dbg_by_id(u32 id, u32 *value);
void msdc_get_rdsel_dbg_by_id(u32 id, u32 *value);
void msdc_dump_padctl_by_id(u32 id);
void msdc_pin_config_by_id(u32 id, u32 mode);


#define msdc_get_driving(host, hw) \
	msdc_get_driving_by_id(host->id, hw)

#define msdc_set_ies(host, set_ies) \
	msdc_set_ies_by_id(host->id, set_ies)

#define msdc_set_sr(host, clk, cmd, dat) \
	msdc_set_sr_by_id(host->id, clk, cmd, dat)

#define msdc_set_smt(host, set_smt) \
	msdc_set_smt_by_id(host->id, set_smt)

#define msdc_set_tdsel(host, sleep, sd_18) \
	msdc_set_tdsel_by_id(host->id, sleep, sd_18)

#define msdc_set_rdsel(host, sleep, sd_18) \
	msdc_set_rdsel_by_id(host->id, sleep, sd_18)

#define msdc_set_tdsel_dbg(host, value) \
	msdc_set_tdsel_dbg_by_id(host->id, value)

#define msdc_set_rdsel_dbg(host, value) \
	msdc_set_rdsel_dbg_by_id(host->id, value)

#define msdc_get_tdsel_dbg(host, value) \
	msdc_get_tdsel_dbg_by_id(host->id, value)

#define msdc_get_rdsel_dbg(host, value) \
	msdc_get_rdsel_dbg_by_id(host->id, value)

#define msdc_dump_padctl(host) \
	msdc_dump_padctl_by_id(host->id)

#define msdc_pin_config(host, mode) \
	msdc_pin_config_by_id(host->id, mode)

/**************************************************************/
/* Section 5: MISC                                            */
/**************************************************************/
void dump_axi_bus_info(void);
void dump_emi_info(void);
void msdc_polling_axi_status(int line, int dead);

#define  MSDC_V33_18_SW0  (114)
#define  MSDC_V33_18_SW1  (115)
#define  MSDC_V33_18_SW2  (116)


#endif /* end of _MSDC_IO_H_ */
