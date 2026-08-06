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

#include <mach/hardware.h>
#include <mach/io.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <asm/mach/time.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/mach/arch.h>
#include <mach/board.h>
#include <linux/mtd/partitions.h>
#include <mtd/mtd-abi.h>
#include <linux/slab.h>
#include <linux/sys_soc.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/of_irq.h>
#include <linux/mmc/atc_msdc_config.h>

#include <linux/types.h>
#include <mach/chip_ver.h>
#include <linux/module.h>
#include <linux/spi/spi.h>

#if defined(CONFIG_CACHE_L2X0)
#include <asm/hardware/cache-l2x0.h>
#endif

//#include <asm/hardware/gic.h>
#include <linux/irqchip/arm-gic.h>
#include <mach/nand.h>
//#include <mach/ac83xx_memory.h>
#include <mach/ac83xx_system.h>
/*
 * IRQ handler for the timer
 */

#include <mach/gpio.h>
#include <mach/ac83xx_gpio_pinmux.h>
#include <linux/of.h>
#include <mach/irqs.h>
#include <linux/interrupt.h>
static struct map_desc ac83xx_io_desc[] __initdata =
{
    {AC83XX_IO_VA, __phys_to_pfn(AC83XX_IO_PA), AC83XX_IO_SIZE, MT_DEVICE},
    {AC83XX_GIC_VA, __phys_to_pfn(AC83XX_GIC_PA), AC83XX_GIC_SIZE, MT_DEVICE},
    //{RESV_MEM_IO_VIRT, __phys_to_pfn(RESV_MEM_IO_PHYS), RESV_MEM_IO_SIZE , MT_DEVICE},
    //{AC83XX_SRAM_VA, __phys_to_pfn(AC83XX_SRAM_PA), AC83XX_SRAM_SIZE , MT_MEMORY_SO},
	//{0xff000000, __phys_to_pfn(0x0), 0x1000 , MT_DEVICE},
    //{0xff100000, __phys_to_pfn(0xc0000000), 0x1000 , MT_DEVICE},
};

void __init ac83xx_map_io(void)
{
	printk("\n============ac83xx_map_io=======\n");
    iotable_init(ac83xx_io_desc, ARRAY_SIZE(ac83xx_io_desc));
}

void mtk_wcn_cmb_sdio_request_eirq(sdio_irq_handler_t irq_handler, void *data)
{
	printk("\n============         mtk_wcn_cmb_sdio_request_eirq            =======\n");
}
EXPORT_SYMBOL(mtk_wcn_cmb_sdio_request_eirq);



// =====================================================================
// MSDC Devices
// =====================================================================

struct msdc_hw msdc0_hw = {
    .clk_src        = DMSDC0_CLK_SRC,
    .clk_max     = DMSDC0_MAXCLK_MHZ,
    .cmd_edge       = DMSDC0_CMD_EDGE,
    .rdata_edge     = DMSDC0_RDATA_EDGE,
    .wdata_edge     = DMSDC0_WDATA_EDGE,
    .clk_drv        = DMSDC0_CLK_DRV_33,
    .cmd_drv        = DMSDC0_CMD_DRV_33,
    .dat_drv        = DMSDC0_DAT_DRV_33,
    .slew_rate		= DMSDC0_SLEW_RATE,
	.clk_resistor	= DMSDC0_CLK_RESISTOR,
	.cmd_resistor	= DMSDC0_CMD_RESISTOR,
	.dat_resistor	= DMSDC0_DAT_RESISTOR,
    .clk_drv_sd_18  = DMSDC0_CLK_DRV_18,               /* sdr104 mode */
    .cmd_drv_sd_18  = DMSDC0_CMD_DRV_18,
    .dat_drv_sd_18  = DMSDC0_DAT_DRV_18,
    .clk_drv_sd_18_sdr50    = DMSDC0_CLK_DRV_18_SDR50,       /* sdr50 mode */
    .cmd_drv_sd_18_sdr50    = DMSDC0_CMD_DRV_18_SDR50,
    .dat_drv_sd_18_sdr50    = DMSDC0_DAT_DRV_18_SDR50,
    .clk_drv_sd_18_ddr50    = DMSDC0_CLK_DRV_18_DDR50,       /* ddr50 mode */
    .cmd_drv_sd_18_ddr50    = DMSDC0_CMD_DRV_18_DDR50,
    .dat_drv_sd_18_ddr50    = DMSDC0_DAT_DRV_18_DDR50,
    .data_pins      = DMSDC0_DAT_PINS,
    .data_offset    = 0,
    .tuning_method	= DMSDC0_TUNING_METHOD,
    .flags          = MSDC_SYS_SUSPEND | MSDC_HIGHSPEED | MSDC_EMMC_HS200,  // MSDC_DDR
    .dat0rddly      = DMSDC0_DAT0_RDDLY,
    .dat1rddly      = DMSDC0_DAT1_RDDLY,
    .dat2rddly      = DMSDC0_DAT2_RDDLY,
    .dat3rddly      = DMSDC0_DAT3_RDDLY,
    .dat4rddly      = DMSDC0_DAT4_RDDLY,
    .dat5rddly      = DMSDC0_DAT5_RDDLY,
    .dat6rddly      = DMSDC0_DAT6_RDDLY,
    .dat7rddly      = DMSDC0_DAT7_RDDLY,
    .datwrddly      = DMSDC0_DAT_WRDDLY,
    .cmdrrddly      = DMSDC0_CMD_RRDDLY,
    .cmdrddly       = DMSDC0_CMD_RDDLY,

	.read_pre_setting_en		= DMSDC0_READ_PRE_SETTING_EN,
	.read_dat_latch_ck_sel		= DMSDC0_READ_DAT_LATCH_CK_SEL,
	.read_ckgen_delay_sel		= DMSDC0_READ_CKGEN_DELAY_SEL,
	.read_sample_edge			= DMSDC0_READ_SAMPL_EDGE,
	.read_pad_delay				= DMSDC0_READ_PAD_DELAY,

	.ddr_read_dat_latch_ck_sel	= DMSDC0_DDR_READ_DAT_LATCH_CK_SEL,
	.ddr_read_ckgen_delay_sel	= DMSDC0_DDR_READ_CKGEN_DELAY_SEL,

	.read_patch_bit1_wrdat_crcs	= 2,
	.read_patch_bit1_cmdrsp_ta	= 2,

	.read_rxdly0				= 0, //0x0b0c0f0e,
	.read_rxdly1				= 0, //0x10120c0e,

	.write_pre_setting_en		= DMSDC0_WRITE_PRE_SETTING_EN,
	.write_dat_latch_ck_sel		= DMSDC0_WRITE_DAT_LATCH_CK_SEL,
	.write_ckgen_delay_sel		= DMSDC0_WRITE_CKGEN_DELAY_SEL,
	.write_sample_edge			= DMSDC0_WRITE_SAMPLE_EDGE,
	.write_pad_delay			= DMSDC0_WRITE_PAD_DELAY,
	.write_internal_delay		= DMSDC0_WRITE_INTERNAL_DELAY,

	.ddr_write_dat_latch_ck_sel	= DMSDC0_DDR_WRITE_DAT_LATCH_CK_SEL,
	.ddr_write_ckgen_delay_sel	= DMSDC0_DDR_WRITE_CKGEN_DELAY_SEL,

	.write_patch_bit1_wrdat_crcs= 2,
	.write_patch_bit1_cmdrsp_ta	= 2,
	
	.write_rxdly0				= 0,
	
    .host_function  = DMSDC0_HOST_FUNCTION,
    .boot           = DMSDC0_BOOT_FLAG,
    .cd_level       = MSDC_CD_LOW,
    .cd_method		= DMSDC0_CD_METHOD,
};

struct msdc_hw msdc1_hw = {
    .clk_src        = DMSDC1_CLK_SRC,
    .clk_max     	= DMSDC1_MAXCLK_MHZ,
    .cmd_edge       = DMSDC1_CMD_EDGE, // MSDC_SMPL_RISING,
    .rdata_edge     = DMSDC1_RDATA_EDGE,
    .wdata_edge     = DMSDC1_WDATA_EDGE,
    .clk_drv        = DMSDC1_CLK_DRV_33,
    .cmd_drv        = DMSDC1_CMD_DRV_33,
    .dat_drv        = DMSDC1_DAT_DRV_33,
    .slew_rate		= DMSDC1_SLEW_RATE,
	.clk_resistor	= DMSDC1_CLK_RESISTOR,
	.cmd_resistor	= DMSDC1_CMD_RESISTOR,
	.dat_resistor	= DMSDC1_DAT_RESISTOR,
    .clk_drv_sd_18  = DMSDC1_CLK_DRV_18,               /* sdr104 mode */
    .cmd_drv_sd_18  = DMSDC1_CMD_DRV_18,
    .dat_drv_sd_18  = DMSDC1_DAT_DRV_18,
    .clk_drv_sd_18_sdr50    = DMSDC1_CLK_DRV_18_SDR50,       /* sdr50 mode */
    .cmd_drv_sd_18_sdr50    = DMSDC1_CMD_DRV_18_SDR50,
    .dat_drv_sd_18_sdr50    = DMSDC1_DAT_DRV_18_SDR50,
    .clk_drv_sd_18_ddr50    = DMSDC1_CLK_DRV_18_DDR50,       /* ddr50 mode */
    .cmd_drv_sd_18_ddr50    = DMSDC1_CMD_DRV_18_DDR50,
    .dat_drv_sd_18_ddr50    = DMSDC1_DAT_DRV_18_DDR50,
    .data_pins      = DMSDC1_DAT_PINS,
    .data_offset    = 0,
    .tuning_method	= DMSDC1_TUNING_METHOD,
    //.flags          = MSDC_SYS_SUSPEND | MSDC_HIGHSPEED | MSDC_GPIO_EINT_CD | MSDC_REMOVABLE,  // MSDC_UHS1 |MSDC_DDR
    .flags          = MSDC_SYS_SUSPEND | MSDC_HIGHSPEED | MSDC_REMOVABLE,  
    .dat0rddly      = DMSDC1_DAT0_RDDLY,
    .dat1rddly      = DMSDC1_DAT1_RDDLY,
    .dat2rddly      = DMSDC1_DAT2_RDDLY,
    .dat3rddly      = DMSDC1_DAT3_RDDLY,
    .dat4rddly      = DMSDC1_DAT4_RDDLY,
    .dat5rddly      = DMSDC1_DAT5_RDDLY,
    .dat6rddly      = DMSDC1_DAT6_RDDLY,
    .dat7rddly      = DMSDC1_DAT7_RDDLY,
    .datwrddly      = DMSDC1_DAT_WRDDLY,
    .cmdrrddly      = DMSDC1_CMD_RRDDLY,
    .cmdrddly       = DMSDC1_CMD_RDDLY,

	.read_pre_setting_en		= DMSDC1_READ_PRE_SETTING_EN,
	.read_dat_latch_ck_sel		= DMSDC1_READ_DAT_LATCH_CK_SEL,
	.read_ckgen_delay_sel		= DMSDC1_READ_CKGEN_DELAY_SEL,
	.read_sample_edge			= DMSDC1_READ_SAMPL_EDGE,
	.read_pad_delay				= DMSDC1_READ_PAD_DELAY,

	.ddr_read_dat_latch_ck_sel	= DMSDC1_DDR_READ_DAT_LATCH_CK_SEL,
	.ddr_read_ckgen_delay_sel	= DMSDC1_DDR_READ_CKGEN_DELAY_SEL,

	.read_patch_bit1_wrdat_crcs	= 1,
	.read_patch_bit1_cmdrsp_ta	= 1,

	.read_rxdly0				= 0,
	.read_rxdly1				= 0,

	.write_pre_setting_en		= DMSDC1_WRITE_PRE_SETTING_EN,
	.write_dat_latch_ck_sel		= DMSDC1_WRITE_DAT_LATCH_CK_SEL,
	.write_ckgen_delay_sel		= DMSDC1_WRITE_CKGEN_DELAY_SEL,
	.write_sample_edge			= DMSDC1_WRITE_SAMPLE_EDGE,
	.write_pad_delay			= DMSDC1_WRITE_PAD_DELAY,
	.write_internal_delay		= DMSDC1_WRITE_INTERNAL_DELAY,

	.ddr_write_dat_latch_ck_sel	= DMSDC1_DDR_WRITE_DAT_LATCH_CK_SEL,
	.ddr_write_ckgen_delay_sel	= DMSDC1_DDR_WRITE_CKGEN_DELAY_SEL,

	.write_patch_bit1_wrdat_crcs= 1,
	.write_patch_bit1_cmdrsp_ta	= 1,

	.write_rxdly0				= 0,
	
    .host_function  = DMSDC1_HOST_FUNCTION,
    .boot           = DMSDC1_BOOT_FLAG,
    .cd_level       = MSDC_CD_LOW,
    .cd_method		= DMSDC1_CD_METHOD,
};

struct msdc_hw msdc2_hw = {
    .clk_src        = DMSDC2_CLK_SRC,
    .clk_max     = DMSDC2_MAXCLK_MHZ,
    .cmd_edge       = DMSDC2_CMD_EDGE, // MSDC_SMPL_RISING,
    .rdata_edge     = DMSDC2_RDATA_EDGE,
    .wdata_edge     = DMSDC2_WDATA_EDGE,
    .clk_drv        = DMSDC2_CLK_DRV_33,
    .cmd_drv        = DMSDC2_CMD_DRV_33,
    .dat_drv        = DMSDC2_DAT_DRV_33,
    .slew_rate		= DMSDC2_SLEW_RATE,
	.clk_resistor	= DMSDC2_CLK_RESISTOR,
	.cmd_resistor	= DMSDC2_CMD_RESISTOR,
	.dat_resistor	= DMSDC2_DAT_RESISTOR,
    .clk_drv_sd_18  = DMSDC2_CLK_DRV_18,               /* sdr104 mode */
    .cmd_drv_sd_18  = DMSDC2_CMD_DRV_18,
    .dat_drv_sd_18  = DMSDC2_DAT_DRV_18,
    .clk_drv_sd_18_sdr50    = DMSDC2_CLK_DRV_18_SDR50,       /* sdr50 mode */
    .cmd_drv_sd_18_sdr50    = DMSDC2_CMD_DRV_18_SDR50,
    .dat_drv_sd_18_sdr50    = DMSDC2_DAT_DRV_18_SDR50,
    .clk_drv_sd_18_ddr50    = DMSDC2_CLK_DRV_18_DDR50,       /* ddr50 mode */
    .cmd_drv_sd_18_ddr50    = DMSDC2_CMD_DRV_18_DDR50,
    .dat_drv_sd_18_ddr50    = DMSDC2_DAT_DRV_18_DDR50,
    .data_pins      = DMSDC2_DAT_PINS,
    .data_offset    = 0,
    .tuning_method	= DMSDC2_TUNING_METHOD,
    .flags          = MSDC_SYS_SUSPEND | MSDC_HIGHSPEED | MSDC_GPIO_EINT_CD | MSDC_REMOVABLE,  // MSDC_UHS1 |MSDC_DDR
    .dat0rddly      = DMSDC2_DAT0_RDDLY,
    .dat1rddly      = DMSDC2_DAT1_RDDLY,
    .dat2rddly      = DMSDC2_DAT2_RDDLY,
    .dat3rddly      = DMSDC2_DAT3_RDDLY,
    .dat4rddly      = DMSDC2_DAT4_RDDLY,
    .dat5rddly      = DMSDC2_DAT5_RDDLY,
    .dat6rddly      = DMSDC2_DAT6_RDDLY,
    .dat7rddly      = DMSDC2_DAT7_RDDLY,
    .datwrddly      = DMSDC2_DAT_WRDDLY,
    .cmdrrddly      = DMSDC2_CMD_RRDDLY,
    .cmdrddly       = DMSDC2_CMD_RDDLY,
    
    .read_pre_setting_en		= DMSDC2_READ_PRE_SETTING_EN,
	.read_dat_latch_ck_sel		= DMSDC2_READ_DAT_LATCH_CK_SEL,
	.read_ckgen_delay_sel		= DMSDC2_READ_CKGEN_DELAY_SEL,
	.read_sample_edge			= DMSDC2_READ_SAMPL_EDGE,
	.read_pad_delay				= DMSDC2_READ_PAD_DELAY,

	.ddr_read_dat_latch_ck_sel	= DMSDC2_DDR_READ_DAT_LATCH_CK_SEL,
	.ddr_read_ckgen_delay_sel	= DMSDC2_DDR_READ_CKGEN_DELAY_SEL,

	.read_patch_bit1_wrdat_crcs	= 1,
	.read_patch_bit1_cmdrsp_ta	= 1,

	.read_rxdly0				= 0,
	.read_rxdly1				= 0,

	.write_pre_setting_en		= DMSDC2_WRITE_PRE_SETTING_EN,
	.write_dat_latch_ck_sel		= DMSDC2_WRITE_DAT_LATCH_CK_SEL,
	.write_ckgen_delay_sel		= DMSDC2_WRITE_CKGEN_DELAY_SEL,
	.write_sample_edge			= DMSDC2_WRITE_SAMPLE_EDGE,
	.write_pad_delay			= DMSDC2_WRITE_PAD_DELAY,
	.write_internal_delay		= DMSDC2_WRITE_INTERNAL_DELAY,

	.ddr_write_dat_latch_ck_sel	= DMSDC2_DDR_WRITE_DAT_LATCH_CK_SEL,
	.ddr_write_ckgen_delay_sel	= DMSDC2_DDR_WRITE_CKGEN_DELAY_SEL,

	.write_patch_bit1_wrdat_crcs= 1,
	.write_patch_bit1_cmdrsp_ta	= 1,

	.write_rxdly0				= 0,
	
    .host_function  = DMSDC2_HOST_FUNCTION,
    .boot           = DMSDC2_BOOT_FLAG,
    .cd_level       = MSDC_CD_LOW,
    .cd_method		= DMSDC2_CD_METHOD,
};

#define IO_BASE_PHYS                        0xF0000000
#define ATC33XX_MSDC0_BASE                  (IO_BASE_PHYS + 0x0B000)
#define ATC33XX_MSDC1_BASE                  (IO_BASE_PHYS + 0x21000)//21000
#define ATC33XX_MSDC2_BASE                  (IO_BASE_PHYS + 0x0A000)

#define ATC83XX_SPI0_BASE                   (0xFD000000 + 0x23400)
#define ATC83XX_SPI1_BASE                   (0xFD000000 + 0x23800)

#define VECTOR_ATC33XX_MSDC0                187
#define VECTOR_ATC33XX_MSDC1                188
#define VECTOR_ATC33XX_MSDC2                189
#define ECO_VER                             0x108 // (0x104 +4) , The Last Register

#define GPIO_MSDC0_DECTECT                  (70)
#define GPIO_MSDC1_DECTECT                  (71)
#define GPIO_MSDC2_DECTECT                  (72)

#define VECTOR_MSDC0_DECTECT                (161)
#define VECTOR_MSDC1_DECTECT                (162)
#define VECTOR_MSDC2_DECTECT                (163)

#define GPIO_MSDC0_SW_V33_18                (114)
#define GPIO_MSDC1_SW_V33_18                (115)
#define GPIO_MSDC2_SW_V33_18                (116)


static struct resource atc_resource_msdc0[] = {
    {
        .start  = ATC33XX_MSDC0_BASE,
        .end    = ATC33XX_MSDC0_BASE + ECO_VER,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = VECTOR_ATC33XX_MSDC0,
        .end    = GPIO_MSDC0_DECTECT,
        .flags  = IORESOURCE_IRQ,
    },
    {
        .start  = VECTOR_MSDC0_DECTECT,
        .end    = GPIO_MSDC0_SW_V33_18,
        .flags  = IORESOURCE_BUS,
    },
};


static struct resource atc_resource_msdc1[] = {
    {
        .start  = ATC33XX_MSDC1_BASE,           // Module IO Base Start
        .end    = ATC33XX_MSDC1_BASE + ECO_VER, // Module IO Base End
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = VECTOR_ATC33XX_MSDC1,         // Module Interrupt Number
        .end    = GPIO_MSDC1_DECTECT,           // GPIO for detect card plug in/out
        .flags  = IORESOURCE_IRQ,
    },
    {
        .start  = VECTOR_MSDC1_DECTECT,         // Interrupt number for detect card plug in/out
        .end    = GPIO_MSDC1_SW_V33_18,         // GPIO for switch V3.3 / V1.8
        .flags  = IORESOURCE_BUS,
    },
};

static struct resource atc_resource_msdc2[] = {
    {
        .start  = ATC33XX_MSDC2_BASE,
        .end    = ATC33XX_MSDC2_BASE + ECO_VER,
        .flags  = IORESOURCE_MEM,
    },
    {
        .start  = VECTOR_ATC33XX_MSDC2,
        .end    = GPIO_MSDC2_DECTECT,
        .flags  = IORESOURCE_IRQ,
    },
    {
        .start  = VECTOR_MSDC2_DECTECT,
        .end    = GPIO_MSDC2_SW_V33_18,
        .flags  = IORESOURCE_BUS,
    },
};

struct platform_device atc83xx_device_msdc0 = {
        .name           = "atc-msdc",
        .id             = 0,
        .num_resources  = ARRAY_SIZE(atc_resource_msdc0),
        .resource       = atc_resource_msdc0,
        .dev = {
            .platform_data = &msdc0_hw,
        },
};

struct platform_device atc83xx_device_msdc1 = {
        .name           = "atc-msdc",
        .id             = 1,
        .num_resources  = ARRAY_SIZE(atc_resource_msdc1),
        .resource       = atc_resource_msdc1,
        .dev = {
            .platform_data = &msdc1_hw,
        },
};

struct platform_device atc83xx_device_msdc2 = {
        .name           = "atc-msdc",
        .id             = 2,
        .num_resources  = ARRAY_SIZE(atc_resource_msdc2),
        .resource       = atc_resource_msdc2,
        .dev = {
            .platform_data = &msdc2_hw,
        },
};


// =====================================================================

//
// I2C Devices
//
struct platform_device atc83xx_device_i2c0 = {
    .name = "ac83xx_i2c",
    .id   = 0,
};

struct platform_device atc83xx_device_i2c1 = {
    .name = "ac83xx_i2c",
    .id   = 1,
};
//
//SPI Devices
//
static struct spi_board_info ac83xx_spi0_board[] = {
     [0] = {
              .modalias = "spidev",
              .bus_num = 0,
              .chip_select = 0,
              .max_speed_hz = 500*1000,
     },
};

static struct spi_board_info ac83xx_spi1_board[] = {
     [0] = {
              .modalias = "spidev",
              .bus_num = 1,
              .chip_select = 0,
              .max_speed_hz = 500*1000,
     },
};

static struct resource atc83xx_resource_spi0[] = {
    {
        .start  = ATC83XX_SPI0_BASE,
        .end    = ATC83XX_SPI0_BASE + 0x24,
        .flags  = IORESOURCE_MEM,
    },
};

static struct resource atc83xx_resource_spi1[] = {
    {
        .start  = ATC83XX_SPI1_BASE,
        .end    = ATC83XX_SPI1_BASE + 0x24,
        .flags  = IORESOURCE_MEM,
    },
};

struct platform_device atc83xx_device_spi0 = {
    .name = "ac83xx-spi",
    .id   = 0,
    .num_resources  = ARRAY_SIZE(atc83xx_resource_spi0),
    .resource       = atc83xx_resource_spi0,
};

struct platform_device atc83xx_device_spi1 = {
       .name = "ac83xx-spi",
       .id   = 1,
       .num_resources  = ARRAY_SIZE(atc83xx_resource_spi1),
       .resource       = atc83xx_resource_spi1,
};
#if 0

// =====================================================================

static struct mtd_partition ac83xx_nand_partitions[] = {
    {
        /* UBL (a few copies) plus U-Boot */
        .name       = "boot",
        .offset     = 0,
        .size       = SZ_4M,
        .mask_flags = MTD_WRITEABLE, /* force read-only */
    }, {
        /* U-Boot environment */
        .name       = "env",
        .offset     = MTDPART_OFS_APPEND,
        .size       = SZ_2M,
        .mask_flags = 0,
    }, {
        .name       = "arm2",
        .offset     = MTDPART_OFS_APPEND,
        .size       = SZ_2M,
        .mask_flags = 0,
    }, {
        .name       = "logo",
        .offset     = MTDPART_OFS_APPEND,
        .size       = SZ_4M,
        .mask_flags = 0,
    }, {
        .name       = "kernel",
        .offset     = MTDPART_OFS_APPEND,
        .size       = SZ_4M,
        .mask_flags = 0,
    }, {
        .name       = "rootfs",
        .offset     = MTDPART_OFS_APPEND,
        .size       = SZ_2M,
        .mask_flags = 0,
    }, {
        .name       = "system",
        .offset     = MTDPART_OFS_APPEND,
        .size       = (SZ_256M+SZ_32M+SZ_8M+SZ_4M),
        .mask_flags = 0,
    }, {
        .name       = "data",
        .offset     = MTDPART_OFS_APPEND,
        .size       = (SZ_256M+SZ_32M+SZ_8M+SZ_4M),
        .mask_flags = 0,
    }, {
        .name       = "cache",
        .offset     = MTDPART_OFS_APPEND,
        .size       = (SZ_64M+SZ_32M+SZ_4M),
        .mask_flags = 0,
    }, {
        .name       = "swap",
        .offset     = MTDPART_OFS_APPEND,
        .size       = (SZ_256M+SZ_32M+SZ_8M+SZ_4M),
        .mask_flags = 0,
    }, {
        .name       = "recovery",
        .offset     = MTDPART_OFS_APPEND,
        .size       = SZ_4M,
        .mask_flags = 0,
    }, {
        .name       = "misc",
        .offset     = MTDPART_OFS_APPEND,
        .size       = SZ_2M,
        .mask_flags = 0,
    }, {
        .name       = "backup",
        .offset     = MTDPART_OFS_APPEND,
        .size       = (SZ_32M+SZ_16M+SZ_2M),
        .mask_flags = 0,
    }, {
        .name       = "metazone",
        .offset     = MTDPART_OFS_APPEND,
        .size       = SZ_4M,
        .mask_flags = 0,
    }, {
        .name       = "dvp",
        .offset     = MTDPART_OFS_APPEND,
        .size       = SZ_4M,
        .mask_flags = 0,
    }, {
        .name       = "data4write",
        .offset     = MTDPART_OFS_APPEND,
        .size       = (SZ_32M+SZ_16M+SZ_2M),
        .mask_flags = 0,
    }, {
        .name       = "usrdata",
        .offset     = MTDPART_OFS_APPEND,
        .size       = MTDPART_SIZ_FULL,
        .mask_flags = 0,
    }
    /* two blocks with bad block table (and mirror) at the end */
};

static struct mtk_nand_pdata ac83xx_nand_data = {
    .partitions         = ac83xx_nand_partitions,
    .nr_partitions      = ARRAY_SIZE(ac83xx_nand_partitions),
};



struct platform_device ac83xx_device_nand = {
    .name = "ac83xx_nand",
    .id   = 0,
    .dev  = {
        .platform_data  = &ac83xx_nand_data,
    },

};
#endif

struct platform_device ac83xx_device_gpio = {
    .name = "ac83xx_gpio",
    .id   = 0,
    //.num_resources  = 202,
    //.resource       = NULL
};

#if 0

struct platform_device ac83xx_device_rtc = {
    .name = "ac83xx_rtc",
    .id   = 0,
};
#endif

static struct platform_device * ac83xx_devices[] __initdata={
    //&ac83xx_device_nand,
    //&ac83xx_device_rtc,
    //&ac83xx_device_gpio,
#if 0
    &atc83xx_device_msdc0,
    &atc83xx_device_msdc1,
    &atc83xx_device_msdc2,
#endif
    //&atc83xx_device_i2c0,
    //&atc83xx_device_i2c1,
    //&atc83xx_device_spi0,
    //&atc83xx_device_spi1,
};


#if 1
typedef struct {
    uint32_t u4AfifoRPtr;
    uint32_t u4AfifoWPtr;
    uint32_t u4AfifoSA;
    uint32_t u4AfifoEA;
} POSINFO_T;

typedef struct {
    POSINFO_T posInfo[4];
} AFIFO_POSINFO_T;

AFIFO_POSINFO_T rAFifo_PosInfo;
AFIFO_POSINFO_T *m_afifo_info;

EXPORT_SYMBOL(m_afifo_info);

#endif

static void __init ac83xx_dt_init(void)
{
    //struct soc_device_attribute *soc_dev_attr;
    //struct soc_device *soc_dev;
    struct device *parent = NULL;
    
#if 1
    memset(&rAFifo_PosInfo, 0, sizeof(rAFifo_PosInfo));
    m_afifo_info = &rAFifo_PosInfo;
    
    platform_add_devices(ac83xx_devices,ARRAY_SIZE(ac83xx_devices));
    spi_register_board_info(ac83xx_spi0_board,ARRAY_SIZE(ac83xx_spi0_board));
    spi_register_board_info(ac83xx_spi1_board,ARRAY_SIZE(ac83xx_spi1_board));
#endif
    /*
     * Finished with the static registrations now; fill in the missing
     * devices
     */

    of_platform_populate(NULL, of_default_bus_match_table, NULL, parent);
}

#if 0
static struct of_device_id irq_match[] __initdata = {
    { .compatible = "arm,cortex-a15-gic", .data = gic_of_init, },
    { }
};
#endif
extern void ac83xx_mask_ack_bim_irq_gic(struct irq_data *data);

/*
static void __init ac83xx_init_irq(void)
{
    //gic_arch_extn.irq_eoi = ac83xx_mask_ack_bim_irq_gic;
    //of_irq_init(irq_match);
}
*/

extern struct sys_timer ac83xx_timer;

#ifdef CONFIG_OF

static const char *ac83xx_dt_board_compat[] __initdata = {
    "Autochips,ac83xx",
    NULL
};

DT_MACHINE_START(AC83XX, "Autochips ac83xx")
    .map_io         = ac83xx_map_io,
    .init_machine   = ac83xx_dt_init,
//    .handle_irq     = gic_handle_irq,
//    .init_irq       = ac83xx_init_irq,
//    .timer          = &ac83xx_timer,
    .dt_compat      = ac83xx_dt_board_compat,
    .restart        = arch_reset
MACHINE_END

#endif

