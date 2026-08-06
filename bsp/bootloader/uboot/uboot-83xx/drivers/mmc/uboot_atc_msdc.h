/*
 *  linux/drivers/mmc/uboot_atc_msdc.h - Secure Digital Host Controller Interface driver
 *
 *  Copyright (C) 2009 Autochips, All Rights Reserved.
 *
 */
#ifndef __MTK_SD_H__
#define __MTK_SD_H__

#include "atc_msdc_hw.h"


#define DRIVER_NAME "[MMC]"
#define DRIVER_VERSION "0.10"

#define BUGMAIL "Autochips.Inc"



// #define SD_LOG_ENABLE 1

/**
* Basic Type definition
*/

#define UINT32 uint32_t
#define UINT uint32_t
#define UINT16 unsigned short
#define UINT8 unsigned char

#define INT32 int32_t
#define INT16 signed short
#define INT8 signed char

#define BOOL  int32_t

#define ASSERT(xxx) 

#define VOID void

#define x_thread_delay(x)  udelay(x*1000);

#define Printf  printk

//#define HalFlushInvalidateDCache(xxx)  flush_cache_all();

//#define VERIFY(xxx) 

typedef struct _NFIType
{
   UINT16   pageSize;   
   UINT16   spareSize;
   UINT16   addressCycle;   
   UINT16   pageShift;
} NFI_MENU;

typedef struct _BOOTLHeader_
{
   char ID1[12];
   char version[4];
   UINT32 length;
   UINT32 startAddr;
   UINT32 checksum;
   char ID2[8];
   NFI_MENU  NFIinfo;
   UINT16 pagesPerBlock;   
   UINT16  totalBlocks;
   UINT16  blockShift;
   UINT16  linkAddr[6];   
   UINT16  lastBlock;
} BOOTL_HEADER;


#define UNUSED(x)               (void)x

#define x_sema_lock(x,y)  0

/**
* Host definition
*/

#define CMD_DEBUG_LENGTH  10

/**
* LOG function
*/

#define SD_LOG_ENABLE 			(1)
#define MMC_CMD_TUNE            (0)

#if SD_LOG_ENABLE

#define SD_LOG_ERROR			((u32)1 << 0)
#define SD_LOG_CARD				((u32)1 << 1)
#define SD_LOG_REQ				((u32)1 << 2)
#define SD_LOG_CLK				((u32)1 << 3)
#define SD_LOG_RETRY			((u32)1 << 4)

#define SD_LOG_INIT				((u32)1 << 5)
#define SD_LOG_FUNC				((u32)1 << 6)
#define SD_LOG_FSM				((u32)1 << 7)
#define SD_LOG_IRQ				((u32)1 << 8)
#define SD_LOG_RSPERR			((u32)1 << 9)
#define SD_LOG_IOCMD			((u32)1 << 10)
#define SD_LOG_RW				((u32)1 << 11)
#define SD_LOG_BUFFER			((u32)1 << 12)
#define SD_LOG_DBG				((u32)1 << 13)
#define SD_LOG_INFO				((u32)1 << 15)
#define SD_LOG_RSP				((u32)1 << 16)
#define SD_LOG_TUNE				((u32)1 << 17)

static unsigned int msdc_log_mask = SD_LOG_ERROR | SD_LOG_DBG;

#define SD_LOG(mask, format, x...) \
if (msdc_log_mask & (mask)){	\
	if (mmc){	\
		printf("[MSDC%d]: <%s> <Line %u> --> "format" <--\r\n", mmc->host_id, __FUNCTION__, __LINE__, ## x);	\
	}	\
	else {	\
		printf("[MSDC]: <%s> <Line %u> --> "format" <--\r\n", __FUNCTION__, __LINE__, ## x);	\
	}	\
}

#define MSDC_LOG(mask, format, x...) \
if (msdc_log_mask & (mask)){	\
	if (mmc){	\
		printf("[MSDC%d]: <%s> <Line %u> --> "format" <--\r\n", mmc->host_id, __FUNCTION__, __LINE__, ## x);	\
	}	\
	else {	\
		printf("[MSDC]: <%s> <Line %u> --> "format" <--\r\n", __FUNCTION__, __LINE__, ## x);	\
	}	\
}

#else
#define SD_LOG(n, f, x...)
#define MSDC_LOG(n, f, x...)
#endif

#define CMD_RETRIES        				(5)
#define CMD_TIMEOUT        				(0x3FFFFFFF)    /* 500ms */

#define MMC_ERR_NONE				(0)
#define MMC_ERR_TIMEOUT     			(1)
#define MMC_ERR_BADCRC      			(2)
#define MMC_ERR_FIFO       			(3)
#define MMC_ERR_FAILED      			(4)
#define MMC_ERR_INVALID     			(5)
#define MMC_ERR_WP_VIOLATION			(6)
#define	MMC_ERR_ENOMEDIUM				(123)		/* No medium found */
#define PINMUX_LEVEL_GPIO_END_FLAG      (0xFF)
#define SD1_CD_PIN            (71)
#define SD2_CD_PIN            (72)
enum {
    RESP_NONE = 0,
    RESP_R1,
    RESP_R2,
    RESP_R3,
    RESP_R4,
    RESP_R5,
    RESP_R6,
    RESP_R7,
    RESP_R1B
};

// For boot partition ops
#define BOOT_CONFIG_BOOTMODE_NORM       (0x00<<2)
#define BOOT_CONFIG_BOOTMODE_ALTE       (0x01<<2)
#define BOOT_CONFIG_ACK_WITHOUT         (0x00<<7)
#define BOOT_CONFIG_ACK_WITH            (0x01<<7)


// According to selected clock source, reference to function: msdc_select_clock_source()
static u32 msdc_host_clk_freq[3] = {200 * 1000 * 1000,
							 108 * 1000 * 1000,
							 108 * 1000 * 1000};

// =====  Clock Select  ===== 
#define MSDC_EMMC_ENABLE_DDR52		(0)
// OR
#define MSDC_ENABLE_HS200			(1)
#define EMMC_ENTER_HS200_TIMING		(1)		// Use CMD6 to write EXT_CSD, make eMMC select HS200 trming
#if MMC_CMD_TUNE
#define MSDC_ENABLE_READ_PRE_SET	(0)	
#define MSDC_ENABLE_WRITE_PRE_SET	(0)	
#else
#define MSDC_ENABLE_READ_PRE_SET	(1)	
#define MSDC_ENABLE_WRITE_PRE_SET	(1)	
#endif
#define MSDC_CLK_200MHZ				(200 * 1000 * 1000)
#define MSDC_CLK_100MHZ				(100 * 1000 * 1000)


#if MSDC_ENABLE_HS200
#define EMMC_WORK_CLOCK				(200 * 1000 * 1000)
#else
#define EMMC_WORK_CLOCK				(108 * 1000 * 1000)
#endif

#define DEF_WORK_CLOCK				(27 * 1000 * 1000)	// Default eMMC/SD work clock

#define MSDC_INIT_CLOCK				(400 * 1000)	// 400KHz
#define EMMC_BOOT_OP_CLOCK			(54  * 1000 * 1000)
#define SD_WORK_CLOCK				(54  * 1000 * 1000)
#define EMMC_DDR_WORK_CLOCK			(54  * 1000 * 1000)


// Function Select
#define MSDC_WRITE_TUNE_FULL_RANGE	(0)

#define MSDC_FORCE_CLOCK_TUNE		(0)		// Force to reduce clock and not try to tune delay parameters
#define MSDC_AUTOCMD23_EN           (1)
#if (MSDC_AUTOCMD23_EN == 1)
#define MSDC_AUTOCMD12_EN			(0)
#else
#define MSDC_AUTOCMD12_EN			(1)
#endif
#define MAX_TRANSFER_BLOCK			(8192)//(1024) 	//Max Transfer Block count

#define ETT_MAX_TRANSFER_BLOCK		(1024) 	// ETT Max Transfer Block count


#define MSDC_RW_TUNE_CONUT_EN		(0)  	// After 'MSDC_RW_TUNE_COUNT' times tuning, if still failed, we make it jump to reduce clock tune
#define MSDC_RW_TUNE_COUNT			(10)

#define TUNE_EACH_DATA_LINE			(0)


//************************************************************//
//
// MSDC Reset Setting
//
//************************************************************//
// Register
#define MSDC_RESET_REG				(IO_BASE + 0x00C4UL)

// SW Reset bit offset
#define MSDC0_MODULE_SW_RESET			(1 << 19) 
#define MSDC1_MODULE_SW_RESET			(1 << 20)
#define MSDC2_MODULE_SW_RESET			(1 << 21)

// HW Reset bit offset
#define MSDC0_MODULE_HW_RESET			(1 << 16) 
#define MSDC1_MODULE_HW_RESET			(1 << 17)
#define MSDC2_MODULE_HW_RESET			(1 << 18)

// Value
#define MSDC_MODULE_RESET_ENABLE		(0)
#define MSDC_MODULE_RESET_DISABLE		(1)

#define MSDC_MODULE_SW_RESET(val) 				\
do {    										\
	u32 bit_offset = 0;							\
	if (mmc->host_id == 0)						\
		bit_offset = MSDC0_MODULE_SW_RESET;		\
	else if (mmc->host_id == 1)					\
		bit_offset = MSDC1_MODULE_SW_RESET;		\
	else if (mmc->host_id == 2)					\
		bit_offset = MSDC2_MODULE_SW_RESET;		\
	MSDC_SET_FIELD(MSDC_RESET_REG, bit_offset, val );\
} while(0)

#define MSDC_MODULE_HW_RESET(val) 				\
do {    										\
	u32 bit_offset = 0;							\
	if (mmc->host_id == 0)						\
		bit_offset = MSDC0_MODULE_HW_RESET;		\
	else if (mmc->host_id == 1)					\
		bit_offset = MSDC1_MODULE_HW_RESET;		\
	else if (mmc->host_id == 2)					\
		bit_offset = MSDC2_MODULE_HW_RESET;		\
	MSDC_SET_FIELD(MSDC_RESET_REG, bit_offset, val );\
} while(0)



//************************************************************//
//
// MSDC Clock Setting
//
//************************************************************//
// AC8317 Clock Source.  IO_BASE (0xF0000000) 
#define MSDC0_CLK_SEL				(IO_BASE + 0x0014UL) // SD01_AP_SEL [15:12] 
#define MSDC1_CLK_SEL				(IO_BASE + 0x0014UL) // SD11_AP_SEL [19:16]
#define MSDC2_CLK_SEL				(IO_BASE + 0x0008UL) // SD21_AP_SEL [27:24]

#define MSDC0_CLK_SEL_OFFSET		(12) // SD01_AP_SEL [15:12]
#define MSDC1_CLK_SEL_OFFSET		(16) // SD11_AP_SEL [19:16]
#define MSDC2_CLK_SEL_OFFSET		(24) // SD21_AP_SEL [27:24]

#define MSDC_CLK_SEL_MASK			(0x0F)

#define MSDC_CLK_SEL_27MHZ			(0x00)	// 27M
#define MSDC_CLK_SEL_MSDCPLL_D2		(0x01)	// 196M
#define MSDC_CLK_SEL_ARMPLL2_D2		(0x02)	// 202M
#define MSDC_CLK_SEL_SYSPLL_D4		(0x03)	// 162M 
#define MSDC_CLK_SEL_USBPLL_D4		(0x04)	// 120M
#define MSDC_CLK_SEL_SYSPLL_D6		(0x05)	// 108M
#define MSDC_CLK_SEL_SYSPLL_D12		(0x06)	// 54M
#define MSDC_CLK_SEL_USBPLL_D10		(0x07)	// 48M
#define MSDC_CLK_SEL_DMPLL_D2		(0x08)	// 189M
#define MSDC_CLK_SEL_APLL2_D2		(0x09)	// 147M
#define MSDC_CLK_SEL_APLL2_D3		(0x0A)	// 98M
#define MSDC_CLK_SEL_APLL1_D2		(0x0B)	// 135M
#define MSDC_CLK_SEL_MSDCPLL_D3		(0x0C)	// 135M
#define MSDC_CLK_SEL_MSDCPLL_D4		(0x0D)	// 99M


#define MSDC_SELECT_CLK_SRC(clk_src) 			\
do {    										\
	u32 clk_reg = 0, bit_offset = 0;			\
	if (mmc->host_id == 0){						\
		bit_offset = MSDC0_CLK_SEL_OFFSET;		\
		clk_reg = MSDC0_CLK_SEL; }				\
	else if (mmc->host_id == 1){				\
		bit_offset = MSDC1_CLK_SEL_OFFSET;		\
		clk_reg = MSDC1_CLK_SEL; }				\
	else if (mmc->host_id == 2){				\
		bit_offset = MSDC2_CLK_SEL_OFFSET;		\
		clk_reg = MSDC2_CLK_SEL; }				\
	if (clk_reg)								\
		MSDC_SET_FIELD(clk_reg, (MSDC_CLK_SEL_MASK << bit_offset), clk_src );\
} while(0)


// AC8317 HClock Source, for module internal use. IO_BASE (0xF0000000) has been added by SD_SET_FIELD
#define MSDC_HCLK_SEL				(IO_BASE + 0x0014UL) // SD20_AP_SEL [11:9], SD10_AP_SEL [8:6], SD00_AP_SEL [5:3]
	
#define MSDC0_HCLK_SEL_OFFSET		(3) // SD00_AP_SEL [5:3]
#define MSDC1_HCLK_SEL_OFFSET		(6) // SD10_AP_SEL [8:6]
#define MSDC2_HCLK_SEL_OFFSET		(9) // SD20_AP_SEL [11:9]
	
#define MSDC_HCLK_SEL_MASK			(0x07)
	
#define MSDC_HCLK_SEL_27MHZ			(0x00)	// 27M
#define MSDC_HCLK_SEL_APLL2_D3		(0x01)	// 98M
#define MSDC_HCLK_SEL_USBPLL_D6		(0x02)	// 80M
#define MSDC_HCLK_SEL_SYSPLL_D9		(0x03)	// 72M
#define MSDC_HCLK_SEL_USBPLL_D8		(0x04)	// 60M
#define MSDC_HCLK_SEL_SYSPLL_D12	(0x05)	// 54M
#define MSDC_HCLK_SEL_USBPLL_D10	(0x06)	// 48M
#define MSDC_HCLK_SEL_SYSPLL_D18	(0x07)	// 36M

	
#define MSDC_SELECT_HCLK_SRC(hclk_src) 				\
	do {											\
		u32 bit_offset = 0;							\
		if (mmc->host_id == 0){ 						\
			bit_offset = MSDC0_HCLK_SEL_OFFSET; }	\
		else if (mmc->host_id == 1){					\
			bit_offset = MSDC1_HCLK_SEL_OFFSET; }	\
		else if (mmc->host_id == 2){					\
			bit_offset = MSDC2_HCLK_SEL_OFFSET;	}	\
		MSDC_SET_FIELD(MSDC_HCLK_SEL, (MSDC_HCLK_SEL_MASK << bit_offset), hclk_src);\
	} while(0)

//************************************************************//
//
//	PAD Setting Config
//
//************************************************************//

// For 4bit mode, only use for nand bootup
#define PAD_CFG_SD0_4BIT_CLK		(IO_BASE + 0x000002C0)
#define PAD_CFG_SD0_4BIT_CMD		(IO_BASE + 0x000002C4)
#define PAD_CFG_SD0_4BIT_DAT0		(IO_BASE + 0x000002C8)
#define PAD_CFG_SD0_4BIT_DAT1		(IO_BASE + 0x000002CC)
#define PAD_CFG_SD0_4BIT_DAT2		(IO_BASE + 0x000002D0)
#define PAD_CFG_SD0_4BIT_DAT3		(IO_BASE + 0x000002D4)
#define PAD_CFG_SD0_4BIT_RST		(IO_BASE + 0x00000334)
		
		
#define PAD_CFG_SD1_CLK				(IO_BASE + 0x000002D8)
#define PAD_CFG_SD1_CMD				(IO_BASE + 0x000002DC)
#define PAD_CFG_SD1_DAT0			(IO_BASE + 0x000002E0)
#define PAD_CFG_SD1_DAT1			(IO_BASE + 0x000002E4)
#define PAD_CFG_SD1_DAT2			(IO_BASE + 0x000002E8)
#define PAD_CFG_SD1_DAT3			(IO_BASE + 0x000002EC)
#define PAD_CFG_SD1_RST				(IO_BASE + 0x0000033C)
		
		
#define PAD_CFG_SD2_CLK				(IO_BASE + 0x000002F0)
#define PAD_CFG_SD2_CMD				(IO_BASE + 0x000002F4)
#define PAD_CFG_SD2_DAT0			(IO_BASE + 0x000002F8)
#define PAD_CFG_SD2_DAT1			(IO_BASE + 0x000002FC)
#define PAD_CFG_SD2_DAT2			(IO_BASE + 0x00000300)
#define PAD_CFG_SD2_DAT3			(IO_BASE + 0x00000304)
#define PAD_CFG_SD2_RST				(IO_BASE + 0x00000340)
		
		
#define PAD_CFG_SD0_CLK				(IO_BASE + 0x0000030C)
#define PAD_CFG_SD0_CMD				(IO_BASE + 0x00000310)
#define PAD_CFG_SD0_DAT0			(IO_BASE + 0x00000314)
#define PAD_CFG_SD0_DAT1			(IO_BASE + 0x00000318)
#define PAD_CFG_SD0_DAT2			(IO_BASE + 0x0000031C)
#define PAD_CFG_SD0_DAT3			(IO_BASE + 0x00000320)
#define PAD_CFG_SD0_DAT4			(IO_BASE + 0x00000324)
#define PAD_CFG_SD0_DAT5			(IO_BASE + 0x00000328)
#define PAD_CFG_SD0_DAT6			(IO_BASE + 0x0000032C)
#define PAD_CFG_SD0_DAT7			(IO_BASE + 0x00000330)
#define PAD_CFG_SD0_RST				(IO_BASE + 0x00000338)
		
// Bit mask and bit offset
#define PAD_CFG_TDSEL_MASK			(0x0F << 23)
#define PAD_CFG_RDSEL_MASK			(0xFF << 15)
#define PAD_CFG_SMT_MASK			(0x01 << 14)
#define PAD_CFG_RESISTOR_MASK		(0x03 << 12)
#define PAD_CFG_PUPD_MASK			(0x01 << 11)
#define PAD_CFG_IES_MASK			(0x01 << 10)
#define PAD_CFG_DRV_MASK			(0x3F << 4)
#define PAD_CFG_SR_MASK				(0x0F << 0)

// Config register value
#define MSDC_SMT_ENABLE				(1)
#define MSDC_SMT_DISABLE			(0)
#define MSDC_PULL_UP				(0)
#define MSDC_PULL_DOWN				(1)
#define MSDC_RESISTOR_10K			(1)
#define MSDC_RESISTOR_50K			(2)
		
		
/* pad_msdc_cfg36 0x00000350*/
#define MSDC_PAD_RST_RXDLY				(IO_BASE + 0x00000350)
	#define SD1_DATA_PINS_AS_SD0_HIGH_4DATA		(0x01 << 0)  /* SD1's data used for SD0 high 4bit data*/
	#define MSDC_PAD_RXDLY_P0_4BIT_RST_MASK		(0x1F << 4)
	#define MSDC_PAD_RXDLY_P0_RST_MASK			(0x1F << 9)
	#define MSDC_PAD_RXDLY_P1_RST_MASK			(0x1F << 14)
	#define MSDC_PAD_RXDLY_P2_RST_MASK			(0x1F << 19)
		
		
// Pad MSDC Function Select
#define MSDC_PAD_FUNC_SELECT				(IO_BASE + 0x00000308)
	#define MSDC_PAD_FUNC_SD2_RST_GPIO_CTL			(0x01UL << 31)
	#define MSDC_PAD_FUNC_SD1_RST_GPIO_CTL			(0x01 << 30)
	#define MSDC_PAD_FUNC_SD0_8BIT_RST_GPIO_CTL		(0x01 << 29)
	#define MSDC_PAD_FUNC_SD0_RST_GPIO_CTL			(0x01 << 28)
	#define MSDC_PAD_FUNC_SD0_8BIT_GPIO_CTL			(0x3FF << 18)
	#define MSDC_PAD_FUNC_SD2_GPIO_CTL				(0x3F << 12)
	#define MSDC_PAD_FUNC_SD1_GPIO_CTL				(0x3F << 6)
	#define MSDC_PAD_FUNC_SD0_GPIO_CTL				(0x3F << 0)
		
#define MSDC_PAD_MISC_CTL					(IO_BASE + 0x0000094)
	#define MSDC_PAD_MISC_SD2_DRAM_AGENT		(0x01 << 14)
	#define MSDC_PAD_MISC_SD1_DRAM_AGENT		(0x01 << 13)
	#define MSDC_PAD_MISC_SD0_DRAM_AGENT		(0x01 << 12)
		
		
// For switch IO Voltage between 3.3V and 1.8V
#define MSDC_SW_GPIO_ENABLE_OUTPUT  		(IO_BASE + 0x00000080)  //gpio0
	#define SD_V33_18_SW0_ENABLE  				(0x01 << 18)
	#define SD_V33_18_SW1_ENABLE  				(0x01 << 19)
	#define SD_V33_18_SW2_ENABLE  				(0x01 << 20)
			
#define MSDC_SW_GPIO_OUTPUT_VALUE 			(IO_BASE + 0x000000EC)
	#define SD_V33_18_SW0_VALUE  				(0x01 << 18)
	#define SD_V33_18_SW1_VALUE  				(0x01 << 19)
	#define SD_V33_18_SW2_VALUE  				(0x01 << 20)
		
//======================================================================================================
		
#define SD_CMD_BIT                      (1 << 7)
#define SD_CMD_APP_BIT                  (1 << 8)
#define SD_CMD_AUTO_BIT                 (1 << 9)

#define DAT_TIMEOUT_100MS				(100000000)
#define DAT_TIMEOUT_200MS				(200000000)
#define DAT_TIMEOUT_500MS				(500000000)


#define mdelay(n)	udelay((n)*1000)

#define DEFAULT_RETRY_NUM		(3)
#define DEFAULT_RETRY_COUNT		(1000)	

#define MSDC_RETRY(expr, retry, cnt) \
    do { \
        u32 t = cnt; \
        u32 r = retry; \
        u32 c = cnt; \
        while (r) { \
            if (!(expr)) break; \
            if (c-- == 0) { \
                r--; udelay(200); c = t; \
            } \
        } \
    } while(0)

#define MSDC_RETRY_DEF(expr) MSDC_RETRY(expr, DEFAULT_RETRY_NUM, DEFAULT_RETRY_COUNT)

#define MSDC_RESET() \
    do { \
        MSDC_SETBIT(MSDC_CFG, MSDC_CFG_RST); \
        MSDC_RETRY(MSDC_READ32(MSDC_CFG) & MSDC_CFG_RST, 5, 1000); \
    } while(0)

#define MSDC_CLR_INT() \
    do { \
        volatile u32 val = MSDC_READ32(MSDC_INT); \
        MSDC_WRITE32(MSDC_INT, val); \
        if (MSDC_READ32(MSDC_INT)) { \
            SD_LOG(SD_LOG_ERROR, "[ASSERT] MSDC_INT is NOT clear"); \
        } \
    } while(0)
        
#define MSDC_CLR_FIFO() \
    do { \
        MSDC_SETBIT(MSDC_FIFOCS, MSDC_FIFOCS_CLR); \
        MSDC_RETRY(MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_CLR, 5, 1000); \
    } while(0)

#define MSDC_TXFIFOCNT() \
		((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_TXCNT) >> 16)
#define MSDC_RXFIFOCNT() \
		((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_RXCNT) >> 0)

#define MSDC_DMA_ON()   	MSDC_CLRBIT(MSDC_CFG, MSDC_CFG_PIO)
#define MSDC_DMA_OFF()  	MSDC_SETBIT(MSDC_CFG, MSDC_CFG_PIO)
#define MSDC_START_DMA()	MSDC_SETBIT(MSDC_DMA_CTRL, MSDC_DMA_CTRL_START)
#define MSDC_DMA_STOP() \
    do { \
        MSDC_SETBIT(MSDC_DMA_CTRL, MSDC_DMA_CTRL_STOP); \
        MSDC_RETRY(MSDC_READ32(MSDC_DMA_CFG) & MSDC_DMA_CFG_STS, 5, 1000); \
    } while(0)


#define SDC_IS_BUSY()	    (MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY)
#define SDC_IS_CMD_BUSY()	(MSDC_READ32(SDC_STS) & SDC_STS_CMDBUSY)
    
#define SDC_SEND_CMD(cmd,arg) \
    do { \
        MSDC_WRITE32(SDC_ARG, (arg)); \
        MSDC_WRITE32(SDC_CMD, (cmd)); \
    } while(0)

#define MSDC_WAIT_COND_TMO(cond, timeout)	\
	do {									\
		u32 to = timeout; 					\
		while (1) {							\
			if (cond) break;				\
			if (timeout > 0){				\
				timeout--;					\
				udelay(1);					\
			}								\
			else {							\
				SD_LOG(SD_LOG_ERROR, "Timeout in %s (line %d), timeout: %dusec", __FUNCTION__, __LINE__, to);\
				break;						\
			}								\
		}									\
	} while(0);
	

#define MSDC_CARD_DETECTION_ON()  MSDC_SETBIT(MSDC_PS, MSDC_PS_CDEN)
#define MSDC_CARD_DETECTION_OFF() MSDC_CLRBIT(MSDC_PS, MSDC_PS_CDEN)	

//
// Clock gate register
//
#define MSDC_CKEN_GATE_REG			(IO_BASE + 0x00A8)
	#define MSDC0_CKEN_GATE				(0x01 << 16)
	#define MSDC1_CKEN_GATE				(0x01 << 17)
	#define MSDC2_CKEN_GATE				(0x01 << 18)
	#define GATE_ENABLE_CLOCK				(1)
	#define GATE_DISABLE_CLOCK				(0)
		
#define MSDC_CLOCK_GATE(host, enable)			\
	do {										\
		u32 bit_offset = 0; 					\
		if (host->host_id == 0){ 				\
			bit_offset = MSDC0_CKEN_GATE; } 	\
		else if (host->host_id == 1){			\
			bit_offset = MSDC1_CKEN_GATE; } 	\
		else if (host->host_id == 2){			\
			bit_offset = MSDC2_CKEN_GATE; } 	\
		MSDC_SET_FIELD(MSDC_CKEN_GATE_REG, bit_offset, enable);	\
		if (enable == GATE_ENABLE_CLOCK)								\
			MSDC_RETRY_DEF(!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));	\
	} while(0)
	
// Real shut down clock then resume it.	
#define RESET_MSDC_CLOCK_GATE(host)					\
	do {											\
		MSDC_CLOCK_GATE(host, GATE_DISABLE_CLOCK);	\
		mdelay(10);									\
		MSDC_CLOCK_GATE(host, GATE_ENABLE_CLOCK);	\
	} while(0)

#if 1

#define MSDC_RESET_HW() 	\
		MSDC_RESET(); 		\
		MSDC_CLR_FIFO(); 	\
		MSDC_CLR_INT(); 

#else

#define MSDC_RESET_HW() 			\
		RESET_MSDC_CLOCK_GATE(mmc);	\
		MSDC_RESET(); 				\
		MSDC_CLR_FIFO(); 			\
		MSDC_CLR_INT(); 

#endif

// export 
extern unsigned int current_in_ett_mode;




#endif
