/*
 *  linux/drivers/mmc/uboot_atc_msdc.c - Secure Digital Host Controller Interface driver
 *
 *  Copyright (C) 2009 Autochips.Inc, All Rights Reserved.
 *
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <hwconfig.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/x_typedef.h>
#include <asm/arch/ac83xx_basic.h>
#include "atc_msdc_config.h"

#include "uboot_atc_msdc.h"
#include "atc_msdc_type.h"
#include "atc_msdc_reg.h"
#include "atc_msdc_hw.h"

#include <gpio.h>
#include <ac83xx_gpio_pinmux.h>

DECLARE_GLOBAL_DATA_PTR;

static int msdc_initialize(bd_t *bis);
int msdc_mmc_inited = 0;
int msdc_mmc_reinit_id = 0;

//===========================================================
// Extern mmc  interfaces 
extern int mmc_send_stopcmd(struct mmc *mmc);
extern int mmc_force_reinit(struct mmc *mmc);
extern int mmc_send_status(struct mmc *mmc, u32 *status);
extern int mmc_disable_dump(struct mmc *mmc, unsigned int flag);
//===========================================================

unsigned int current_in_ett_mode = 0;

static unsigned int msdc_base_address[] = {	ATC_MSDC0_BASE,
                                         	ATC_MSDC1_BASE,
                                         	ATC_MSDC2_BASE};
#define MAX_CLOCK_SRC_TYPE		(8)
// Clock source value for clock source select
static u32 clock_freq[] = { 200 * 1000 * 1000, 
							196 * 1000 * 1000, 
							189 * 1000 * 1000, 
							162 * 1000 * 1000, 
							147 * 1000 * 1000, 
							135 * 1000 * 1000, 
							108 * 1000 * 1000, 
							27  * 1000 * 1000}; 

//===========================================================
// Local function defines for fixing build warning 
int msdc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data);
int msdc_set_clock(struct mmc *mmc);
int msdc_wait_rsp(struct mmc *mmc, struct mmc_cmd *cmd);
void msdc_abort_handler(struct mmc *mmc, int abort_card);
int msdc_select_clock_source(struct mmc *mmc);
int msdc_command_tune (struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data);
//===========================================================

int msdc_get_err_from_card_status(struct mmc *mmc)
{
	u32 status;

	int err = mmc_send_status(mmc, &status);
	if(err == MMC_ERR_NONE) {
		if(status & R1_WP_VIOLATION)
			err = MMC_ERR_WP_VIOLATION;
	}
	return err;
}

int msdc_dump_register(struct mmc *mmc)
{
	UINT base = mmc->base_address;
	
	SD_LOG(SD_LOG_DBG, "[00] MSDC_CFG    = 0x%.8X", MSDC_READ32(MSDC_CFG));
	SD_LOG(SD_LOG_DBG, "[04] MSDC_IOCON  = 0x%.8X", MSDC_READ32(MSDC_IOCON));
	SD_LOG(SD_LOG_DBG, "[08] MSDC_PS     = 0x%.8X", MSDC_READ32(MSDC_PS));
	SD_LOG(SD_LOG_DBG, "[0C] MSDC_INT    = 0x%.8X", MSDC_READ32(MSDC_INT));
	SD_LOG(SD_LOG_DBG, "[10] MSDC_INTEN  = 0x%.8X", MSDC_READ32(MSDC_INTEN));
	SD_LOG(SD_LOG_DBG, "[14] MSDC_FIFOCS = 0x%.8X", MSDC_READ32(MSDC_FIFOCS));
	SD_LOG(SD_LOG_DBG, "[18] MSDC_TXDATA = 0x%.8X", MSDC_READ32(MSDC_TXDATA));
	SD_LOG(SD_LOG_DBG, "[1C] MSDC_RXDATA = 0x%.8X", MSDC_READ32(MSDC_RXDATA));
	SD_LOG(SD_LOG_DBG, "[30] SDC_CFG     = 0x%.8X", MSDC_READ32(SDC_CFG));
	SD_LOG(SD_LOG_DBG, "[34] SDC_CMD     = 0x%.8X", MSDC_READ32(SDC_CMD));
	SD_LOG(SD_LOG_DBG, "[38] SDC_ARG     = 0x%.8X", MSDC_READ32(SDC_ARG));
	SD_LOG(SD_LOG_DBG, "[3C] SDC_STS     = 0x%.8X", MSDC_READ32(SDC_STS));	
	SD_LOG(SD_LOG_DBG, "[50] SDC_BLK_NUM = 0x%.8X", MSDC_READ32(SDC_BLK_NUM));
	SD_LOG(SD_LOG_DBG, "[90] DMA_SA      = 0x%.8X", MSDC_READ32(MSDC_DMA_SA));
	SD_LOG(SD_LOG_DBG, "[94] DMA_CA      = 0x%.8X", MSDC_READ32(MSDC_DMA_CA));
	SD_LOG(SD_LOG_DBG, "[98] DMA_CTRL    = 0x%.8X", MSDC_READ32(MSDC_DMA_CTRL));
	SD_LOG(SD_LOG_DBG, "[9C] DMA_CFG     = 0x%.8X", MSDC_READ32(MSDC_DMA_CFG));
	SD_LOG(SD_LOG_DBG, "[A8] DMA_LEN     = 0x%.8X", MSDC_READ32(MSDC_DMA_LEN));
	SD_LOG(SD_LOG_DBG, "[B0] PATCH_BIT0  = 0x%.8X", MSDC_READ32(MSDC_PATCH_BIT0));
	SD_LOG(SD_LOG_DBG, "[B4] PATCH_BIT1  = 0x%.8X", MSDC_READ32(MSDC_PATCH_BIT1));
	SD_LOG(SD_LOG_DBG, "[EC] PAD_TUNE     = 0x%.8X", MSDC_READ32(MSDC_PAD_TUNE));
	SD_LOG(SD_LOG_DBG, "[F0] DAT_RD_DLY0  = 0x%.8X", MSDC_READ32(MSDC_DAT_RDDLY0));
	SD_LOG(SD_LOG_DBG, "[F4] DAT_RD_DLY1  = 0x%.8X", MSDC_READ32(MSDC_DAT_RDDLY1));

	return MMC_ERR_NONE;
}


void msdc_dump_card_status(u32 card_status)
{
    static char *state[] = {
        "Idle",			/* 0 */
        "Ready",		/* 1 */
        "Ident",		/* 2 */
        "Stby",			/* 3 */
        "Tran",			/* 4 */
        "Data",			/* 5 */
        "Rcv",			/* 6 */
        "Prg",			/* 7 */
        "Dis",			/* 8 */
        "Reserved",		/* 9 */
        "Reserved",		/* 10 */
        "Reserved",		/* 11 */
        "Reserved",		/* 12 */
        "Reserved",		/* 13 */
        "Reserved",		/* 14 */
        "I/O mode",		/* 15 */
    };
    if (card_status & R1_OUT_OF_RANGE)
        printf("\t[CARD_STATUS] Out of Range\r\n");
    if (card_status & R1_ADDRESS_ERROR)
        printf("\t[CARD_STATUS] Address Error\r\n");
    if (card_status & R1_BLOCK_LEN_ERROR)
        printf("\t[CARD_STATUS] Block Len Error\r\n");
    if (card_status & R1_ERASE_SEQ_ERROR)
        printf("\t[CARD_STATUS] Erase Seq Error\r\n");
    if (card_status & R1_ERASE_PARAM)
        printf("\t[CARD_STATUS] Erase Param\r\n");
    if (card_status & R1_WP_VIOLATION)
        printf("\t[CARD_STATUS] WP Violation\r\n");
    if (card_status & R1_CARD_IS_LOCKED)
        printf("\t[CARD_STATUS] Card is Locked\r\n");
    if (card_status & R1_LOCK_UNLOCK_FAILED)
        printf("\t[CARD_STATUS] Lock/Unlock Failed\r\n");
    if (card_status & R1_COM_CRC_ERROR)
        printf("\t[CARD_STATUS] Command CRC Error\r\n");
    if (card_status & R1_ILLEGAL_COMMAND)
        printf("\t[CARD_STATUS] Illegal Command\r\n");
    if (card_status & R1_CARD_ECC_FAILED)
        printf("\t[CARD_STATUS] Card ECC Failed\r\n");
    if (card_status & R1_CC_ERROR)
        printf("\t[CARD_STATUS] CC Error\r\n");
    if (card_status & R1_ERROR)
        printf("\t[CARD_STATUS] Error\r\n");
    if (card_status & R1_UNDERRUN)
        printf("\t[CARD_STATUS] Underrun\r\n");
    if (card_status & R1_OVERRUN)
        printf("\t[CARD_STATUS] Overrun\r\n");
    if (card_status & R1_CID_CSD_OVERWRITE)
        printf("\t[CARD_STATUS] CID/CSD Overwrite\r\n");
    if (card_status & R1_WP_ERASE_SKIP)
        printf("\t[CARD_STATUS] WP Eraser Skip\r\n");
    if (card_status & R1_CARD_ECC_DISABLED)
        printf("\t[CARD_STATUS] Card ECC Disabled\r\n");
    if (card_status & R1_ERASE_RESET)
        printf("\t[CARD_STATUS] Erase Reset\r\n");
    if (card_status & R1_READY_FOR_DATA)
        printf("\t[CARD_STATUS] Ready for Data\r\n");
    if (card_status & R1_SWITCH_ERROR)
        printf("\t[CARD_STATUS] Switch error\r\n");
    if (card_status & R1_EXCEPTION_EVENT)
        printf("\t[CARD_STATUS] Exception event happens\r\n");
    if (card_status & R1_APP_CMD)
        printf("\t[CARD_STATUS] App Command\r\n");

    printf("\t[CARD_STATUS] '%s' State(0x%08X)\r\n", 
    state[R1_CURRENT_STATE(card_status)], card_status);
}


void msdc_dump_ocr_reg(u32 resp)
{
    if (resp & (1 << 7))
        printf("\t[OCR] Low Voltage Range\r\n");
    if (resp & (1 << 15))
        printf("\t[OCR] 2.7-2.8 volt\r\n");
    if (resp & (1 << 16))
        printf("\t[OCR] 2.8-2.9 volt\r\n");
    if (resp & (1 << 17))
        printf("\t[OCR] 2.9-3.0 volt\r\n");
    if (resp & (1 << 18))
        printf("\t[OCR] 3.0-3.1 volt\r\n");
    if (resp & (1 << 19))
        printf("\t[OCR] 3.1-3.2 volt\r\n");
    if (resp & (1 << 20))
        printf("\t[OCR] 3.2-3.3 volt\r\n");
    if (resp & (1 << 21))
        printf("\t[OCR] 3.3-3.4 volt\r\n");
    if (resp & (1 << 22))
        printf("\t[OCR] 3.4-3.5 volt\r\n");
    if (resp & (1 << 23))
        printf("\t[OCR] 3.5-3.6 volt\r\n");
    if (resp & (1 << 24))
        printf("\t[OCR] Switching to 1.8V Accepted (S18A)\r\n");
    if (resp & (1 << 30))
        printf("\t[OCR] Card Capacity Status (CCS)\r\n");
    if (resp & (1UL << 31))
        printf("\t[OCR] Card Power Up Status (Idle)\r\n");
    else
        printf("\t[OCR] Card Power Up Status (Busy)\r\n");
}


void msdc_dump_rca_resp(u32 resp)
{
    u32 card_status = (((resp >> 15) & 0x1) << 23) |
                      (((resp >> 14) & 0x1) << 22) |
                      (((resp >> 13) & 0x1) << 19) |
                        (resp & 0x1fff);

    printf("\t[RCA] 0x%x\r\n", resp >> 16);
    msdc_dump_card_status(card_status);
}

void msdc_set_timeout(struct mmc *mmc, u32 ns, u32 clks)
{
    u32 base = mmc->base_address;
    u32 timeout, clk_ns;

    clk_ns  = 1000000000UL / mmc->real_clock; 	/* time length (ns) of each clock cycle */ 
    timeout = ns / clk_ns + clks; 				/* make timeout value from time to clock cycle number */
    timeout = timeout >> 20; 					/* in 1048576 sclk cycle unit */
    timeout = timeout > 1 ? timeout - 1 : 0;	/* convert to register value */
    timeout = timeout > 255 ? 255 : timeout;

	MSDC_SET_FIELD(SDC_CFG, SDC_CFG_DTOC, timeout);

    SD_LOG(SD_LOG_INFO, "Set read data timeout: %dns %dclks -> %d x 65536 cycles",
        ns, clks, timeout + 1);
}

int msdc_get_cd(int dev_num)
{
	if(dev_num==0){ 
		return 	0;
	}
	if(dev_num==1){ 
		return 	gpio_get_value(SD1_CD_PIN);
	}
	else if(dev_num==2){ 
		return  gpio_get_value(SD2_CD_PIN);
	}
	else{
		printf("error port %d \n",dev_num);
		return 0;
	}
}
#if 1
UINT msdc_dma_transfer(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	UINT datsta;
	UINT status = MMC_ERR_NONE;
	UINT32 dma_retry = 0x4FFFFF;//0x3FFFFFF; // Here retry times need large for dma transfer done
	UINT32 dwLength = data->blocks * data->blocksize;
	void *dst = data->dest;
	UINT base = mmc->base_address;	

	MSDC_DMA_ON();
#if MSDC_AUTOCMD23_EN
	// the INT_CMDRDY occurs after INT_ACMDRDY .and must be clear in here
	if (mmc->card_type == MMC_TYPE_MMC)
		MSDC_WRITE32(MSDC_INT,MSDC_INT_CMDRDY);	
#endif
	// Double check interrupt status
	datsta = MSDC_READ32(MSDC_INT);
	if (datsta != 0)
	{
		SD_LOG(SD_LOG_ERROR, "Before Start DMA =====> 0x%08X", datsta);
		// Clear Interrupts
		MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));
	}
	
	MSDC_WRITE32(MSDC_INTEN, MSDC_INT_DXFER_DONE | MSDC_INT_XFER_COMPL | MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_CSTA );

	MSDC_WRITE32(MSDC_DMA_SA, (UINT32)(dst));
	MSDC_WRITE32(MSDC_DMA_LEN, dwLength);

	// Double check Data Len
	if (MSDC_READ32(MSDC_DMA_LEN) != (MSDC_READ32(SDC_BLK_NUM) * 512))
	{
		SD_LOG(SD_LOG_ERROR, "===============> MSDC_DMA_LEN = 0x%08X, SDC_BLK_NUM = 0x%08X <===============", MSDC_READ32(MSDC_DMA_LEN), MSDC_READ32(SDC_BLK_NUM));
	}
	
	MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_LASTBUF, 1);
	MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 0); // Basic DMA
	MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, MSDC_DMA_BST_64);
	
	//msdc_dump_register(mmc);
	MSDC_START_DMA();

	while(dma_retry){
		datsta = MSDC_READ32(MSDC_INT);

	   	if(datsta & MSDC_INT_DATTMO){
			#ifndef CONFIG_MSDC_ETT
			SD_LOG(SD_LOG_ERROR, "CMD%d Data Timeout: data_len = %d blocks" , cmd->opcode, data->blocks);
			#endif
			status = MMC_ERR_TIMEOUT;
			break;
		}

		if(datsta & MSDC_INT_DATCRCERR){
			//SD_LOG(SD_LOG_ERROR, "Data CRC Error");
			status = MMC_ERR_BADCRC;
			break;
		}

		if(datsta & MSDC_INT_XFER_COMPL){
			//SD_LOG(SD_LOG_ERROR, "=====> MSDC_INT_XFER_COMPL");
			status = MMC_ERR_NONE;
			break;
		}

		if(datsta & MSDC_INT_DXFER_DONE){
			//SD_LOG(SD_LOG_ERROR, "=====> MSDC_INT_DXFER_DONE");
			//break;
		}

		if (datsta & MSDC_INT_ACMDCRCERR)
		{
			status = MMC_ERR_BADCRC;
			break;
		}

		if (datsta & MSDC_INT_ACMDRDY)
		{
			SD_LOG(SD_LOG_ERROR, "=====> ACMD DONE");
		}

		if (datsta != 0)
		{
			if (datsta & MSDC_INT_ACMDCRCERR) // Autocmd CRC ERR
			{
				status = MMC_ERR_BADCRC;
				break;
			}
			else if (datsta & MSDC_INT_ACMDTMO) // Autocmd Timeout
			{
				status = MMC_ERR_TIMEOUT;
				break;
			}
			
			SD_LOG(SD_LOG_ERROR, "=====> 0x%08X", datsta);
		}
		dma_retry--;
	}

	if (dma_retry == 0)
	{
		SD_LOG(SD_LOG_ERROR, "FATAL ERROR!!! Data DMA transfer is not really completed. <CMD%d, BLK %d>", cmd->opcode, data->blocks);
		status = MMC_ERR_TIMEOUT;
		msdc_dump_register(mmc);
		RESET_MSDC_CLOCK_GATE(mmc);
	}

	// Stop DMA and wait it was completed
	MSDC_DMA_STOP();
	// Clear Interrupts
	MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));
	MSDC_WRITE32(MSDC_INTEN, 0);

	MSDC_DMA_OFF();

	MSDC_WRITE32(MSDC_DMA_LEN, 0);
	//MSDC_WRITE32(SDC_BLK_NUM, 0); 
	
	// Check FIFO Status
	if (status == MMC_ERR_BADCRC)
	{
		MSDC_CLR_FIFO();//SD_LOG(SD_LOG_ERROR, "TXCNT=%d, RXCNT=%d", MSDC_TXFIFOCNT(), MSDC_RXFIFOCNT());
	}

	return status;
}

#else

UINT msdc_dma_transfer(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	UINT datsta;
	UINT status = MMC_ERR_NONE;
	UINT32 dma_retry = 0x3FFFFF; // Here retry times need large for dma transfer done
	int retry = 30;
	int count = 1000;
	UINT dwLength = data->blocks * data->blocksize;
	void *dst = data->dest;
	UINT base = mmc->base_address;

	msdc_set_timeout(mmc, 100000000, 0);

	MSDC_DMA_ON();
	MSDC_WRITE32(MSDC_INTEN, MSDC_INT_XFER_COMPL | MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_CSTA );

	MSDC_WRITE32(MSDC_DMA_SA, (UINT32)(dst));
	MSDC_WRITE32(MSDC_DMA_LEN, dwLength);
	MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_LASTBUF, 1);
	MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, MSDC_DMA_BST_64);
	
	//msdc_dump_register(mmc);
	MSDC_SETBIT(MSDC_DMA_CTRL, MSDC_DMA_CTRL_START);

	while(1){
		datsta = MSDC_READ32(MSDC_INT);

	   	if(datsta & MSDC_INT_DATTMO){
			SD_LOG(SD_LOG_ERROR, "Data Timeout");
			status = MMC_ERR_TIMEOUT;
			break;
		}

		if(datsta & MSDC_INT_DATCRCERR){
			SD_LOG(SD_LOG_ERROR, "Data CRC Error");
			status = MMC_ERR_BADCRC;
			break;
		}

		if(datsta & MSDC_INT_XFER_COMPL){
			break;
		}

		mdelay(1);
		//dma_retry--;
	}

	if (dma_retry == 0)
	{
		SD_LOG(SD_LOG_ERROR, "Data DMA transfer is not really completed");
		status = MMC_ERR_TIMEOUT;
	}

	// Stop DMA and wait it was completed
	MSDC_SETBIT(MSDC_DMA_CTRL, MSDC_DMA_CTRL_STOP);
	MSDC_RETRY((MSDC_READ32(MSDC_DMA_CFG) & MSDC_DMA_CFG_STS), retry, count);
	// Clear Interrupts
	MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));
	MSDC_WRITE32(MSDC_INTEN, 0);

	MSDC_DMA_OFF();

	return status;
}

#endif

UINT32 msdc_hw_reset_whole_module(struct mmc *mmc)
{
	SD_LOG(SD_LOG_ERROR, "hw reset whole module for msdc%d", mmc->host_id);
	
	MSDC_MODULE_HW_RESET(MSDC_MODULE_RESET_ENABLE);
	MSDC_MODULE_HW_RESET(MSDC_MODULE_RESET_DISABLE);
	
	return MMC_ERR_NONE;
}

UINT32 msdc_sw_reset_whole_module(struct mmc *mmc)
{
	SD_LOG(SD_LOG_ERROR, "sw reset whole module for msdc%d", mmc->host_id);

	MSDC_MODULE_SW_RESET(MSDC_MODULE_RESET_ENABLE);
	MSDC_MODULE_SW_RESET(MSDC_MODULE_RESET_DISABLE);
	
	return MMC_ERR_NONE;
}

UINT32 msdc_set_pad_params(struct mmc* mmc, u32 clk_drv, u32 cmd_drv, u32 dat_drv, u32 resistor, u32 slew_rate)
{	
	printf("=====> msdc%d change pin pad config params <=====\n", mmc->host_id);
	printf("\t clk_drv  : 0x%02X --> 0x%02X (0~3F)\n", mmc->host_hw->clk_drv, clk_drv);
	printf("\t cmd_drv  : 0x%02X --> 0x%02X (0-3F)\n", mmc->host_hw->cmd_drv, cmd_drv);
	printf("\t dat_drv  : 0x%02X --> 0x%02X (0~3F)\n", mmc->host_hw->dat_drv, dat_drv);
	printf("\t cmd and data line resistor : 0x%02X --> 0x%02X (0~1)\n", mmc->host_hw->resistor_cmddat_line, resistor);
	printf("\t slew_rate: 0x%02X --> 0x%02X (0~F)\n",  mmc->host_hw->slew_rate, slew_rate);

	mmc->host_hw->clk_drv	= clk_drv % 64;
	mmc->host_hw->cmd_drv	= cmd_drv % 64;
	mmc->host_hw->dat_drv	= dat_drv % 64;
	mmc->host_hw->resistor_cmddat_line = resistor % 2;
	mmc->host_hw->slew_rate	= slew_rate % 16;

	msdc_pad_init(mmc, 0);
	
	return MMC_ERR_NONE;
}

UINT32 msdc_get_pad_params(struct mmc* mmc)
{	
	printf("=====> msdc%d pin pad config params <=====\n", mmc->host_id);
	printf("\t clk_drv  : 0x%02X (0~3F)\n", mmc->host_hw->clk_drv);
	printf("\t cmd_drv  : 0x%02X (0-3F)\n", mmc->host_hw->cmd_drv);
	printf("\t dat_drv  : 0x%02X (0~3F)\n", mmc->host_hw->dat_drv);
	printf("\t clk resistor : 0x%02X (0~1)\n", mmc->host_hw->resistor_clk_line);
	printf("\t cmd data resistor : 0x%02X (0~1)\n", mmc->host_hw->resistor_cmddat_line);
	printf("\t slew_rate: 0x%02X (0~F)\n", mmc->host_hw->slew_rate);
	
	return MMC_ERR_NONE;
}


UINT32 msdc_host_hw_read_setting_en(struct mmc *mmc, u32 enable)
{
	mmc->host_hw->read_pre_setting_en = enable;
	printf("=====> msdc%d read pre_setting enable = %d <=====\n", mmc->host_id, mmc->host_hw->read_pre_setting_en); 
	return MMC_ERR_NONE;
}

UINT32 msdc_host_hw_write_setting_en(struct mmc *mmc, u32 enable)
{
	mmc->host_hw->write_pre_setting_en = enable;
	printf("=====> msdc%d write pre_setting enable = %d <=====\n", mmc->host_id, mmc->host_hw->write_pre_setting_en);
	return MMC_ERR_NONE;
}

UINT32 msdc_get_read_pre_setting_params(struct mmc *mmc, u32 ddr)
{
	printf("==========> get msdc%d read pre_setting <==========\n", mmc->host_id);
	
	if(ddr)
	{
		printf("\t DDR read pre_setting: %s\n", mmc->host_hw->read_pre_setting_en ? "enable":"disable");
		printf("\t ck_sel:               %d\n", mmc->host_hw->ddr_read_dat_latch_ck_sel);
		printf("\t ckgen_delay:          %d\n", mmc->host_hw->ddr_read_ckgen_delay_sel);
	}
	else
	{
		printf("\t SDR read pre_setting: %s\n", mmc->host_hw->read_pre_setting_en ? "enable":"disable");
		printf("\t ck_sel:               %d\n", mmc->host_hw->read_dat_latch_ck_sel);
		printf("\t ckgen_delay:          %d\n", mmc->host_hw->read_ckgen_delay_sel);
		printf("\t pad_delay:            %d\n", mmc->host_hw->read_pad_delay);
		printf("\t sampe_edge:           %d\n", mmc->host_hw->read_sample_edge);
	}
	
	printf("===================================================\n\n");
	return MMC_ERR_NONE;
}

UINT32 msdc_set_read_pre_setting_params(struct mmc *mmc, u32 ddr, u32 ck_sel, u32 ckgen_delay, u32 pad_delay, u32 sample_edge)
{
	printf("==========> set msdc%d read pre_setting <==========\n", mmc->host_id);
	
	if(ddr)
	{
		printf("\t DDR read pre_setting: %s\n", mmc->host_hw->read_pre_setting_en ? "enable":"disable");
		printf("\t ck_sel:               %d --> %d\n", mmc->host_hw->ddr_read_dat_latch_ck_sel, ck_sel % 8);
		printf("\t ckgen_delay:          %d --> %d\n", mmc->host_hw->ddr_read_ckgen_delay_sel, ckgen_delay % 32);
		mmc->host_hw->ddr_read_dat_latch_ck_sel	= ck_sel % 8;
		mmc->host_hw->ddr_read_ckgen_delay_sel 	= ckgen_delay % 32;
	}
	else
	{
		printf("\t SDR read pre_setting: %s\n", mmc->host_hw->read_pre_setting_en ? "enable":"disable");
		printf("\t ck_sel:               %d --> %d\n", mmc->host_hw->read_dat_latch_ck_sel, ck_sel % 8);
		printf("\t ckgen_delay:          %d --> %d\n", mmc->host_hw->read_ckgen_delay_sel, ckgen_delay % 32);
		printf("\t pad_delay:            %d --> %d\n", mmc->host_hw->read_pad_delay, pad_delay % 32);
		printf("\t sampe_edge:           %d --> %d\n", mmc->host_hw->read_sample_edge, sample_edge % 2);

		mmc->host_hw->read_dat_latch_ck_sel	= ck_sel % 8;
		mmc->host_hw->read_ckgen_delay_sel 	= ckgen_delay % 32;
		mmc->host_hw->read_pad_delay 		= pad_delay % 32;
		mmc->host_hw->read_sample_edge 		= sample_edge % 2;
	}
	
	printf("====================================================\n\n");
	return MMC_ERR_NONE;
}

UINT32 msdc_get_write_pre_setting_params(struct mmc *mmc, u32 ddr)
{
	printf("==========> get msdc%d write pre_setting <==========\n", mmc->host_id);
	
	if(ddr)
	{
		printf("\t DDR write pre_setting: %s\n", mmc->host_hw->write_pre_setting_en ? "enable":"disable");
		printf("\t ck_sel:                %d\n", mmc->host_hw->ddr_write_dat_latch_ck_sel);
		printf("\t ckgen_delay:           %d\n", mmc->host_hw->ddr_write_ckgen_delay_sel);
	}
	else
	{
		printf("\t SDR write pre_setting: %s\n", mmc->host_hw->write_pre_setting_en ? "enable":"disable");
		printf("\t ck_sel:                %d\n", mmc->host_hw->write_dat_latch_ck_sel);
		printf("\t ckgen_delay:           %d\n", mmc->host_hw->write_ckgen_delay_sel);
		printf("\t pad_delay:             %d\n", mmc->host_hw->write_pad_delay);
		printf("\t internal_delay:        %d\n", mmc->host_hw->write_internal_delay);
		printf("\t sampe_edge:            %d\n", mmc->host_hw->write_sample_edge);
	}
	
	printf("====================================================\n\n");
	return MMC_ERR_NONE;
}

UINT32 msdc_set_write_pre_setting_params(struct mmc *mmc, u32 ddr, u32 ck_sel, u32 ckgen_delay, u32 pad_delay, u32 internal_delay, u32 sample_edge)
{
	printf("==========> set msdc%d write pre_setting <==========\n", mmc->host_id);
	
	if(ddr)
	{
		printf("\t DDR write pre_setting: %s\n", mmc->host_hw->write_pre_setting_en ? "enable":"disable");
		printf("\t ck_sel:                %d --> %d\n", mmc->host_hw->ddr_write_dat_latch_ck_sel, ck_sel % 8);
		printf("\t ckgen_delay:           %d --> %d\n", mmc->host_hw->ddr_write_ckgen_delay_sel, ckgen_delay % 32);
		mmc->host_hw->ddr_write_dat_latch_ck_sel	= ck_sel % 8;
		mmc->host_hw->ddr_write_ckgen_delay_sel 	= ckgen_delay % 32;
	}
	else
	{
		printf("\t SDR write pre_setting: %s\n", mmc->host_hw->write_pre_setting_en ? "enable":"disable");
		printf("\t ck_sel:                %d --> %d\n", mmc->host_hw->read_dat_latch_ck_sel, ck_sel % 8);
		printf("\t ckgen_delay:           %d --> %d\n", mmc->host_hw->read_ckgen_delay_sel, ckgen_delay % 32);
		printf("\t pad_delay:             %d --> %d\n", mmc->host_hw->read_pad_delay, pad_delay % 32);
		printf("\t internal_delay:        %d --> %d\n", mmc->host_hw->read_pad_delay, internal_delay % 32);
		printf("\t sampe_edge:            %d --> %d\n", mmc->host_hw->read_sample_edge, sample_edge % 2);

		mmc->host_hw->write_dat_latch_ck_sel	= ck_sel % 8;
		mmc->host_hw->write_ckgen_delay_sel 	= ckgen_delay % 32;
		mmc->host_hw->write_pad_delay 			= pad_delay % 32;
		mmc->host_hw->write_internal_delay		= internal_delay % 32;
		mmc->host_hw->write_sample_edge 		= sample_edge % 2;
	}
	
	printf("=====================================================\n\n");
	return MMC_ERR_NONE;
}


static UINT32 msdc_hs_read_pre(struct mmc *mmc)
{
	int ret = MMC_ERR_NONE;
	UINT32 base = mmc->base_address;

	// If in tuning process, we not change tune parameters, return directly
	if ( mmc->host_hw->read_pre_setting_en && (!mmc->in_tuning_process))
	{
		if (!mmc->cur_ddr_mode)
		{
		#if TUNE_EACH_DATA_LINE
		//set all data line use same delay cycle 
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);
			//set all data line use same sample edge	
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 0);

			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, mmc->host_hw->ddr_read_dat_latch_ck_sel);
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, mmc->host_hw->ddr_read_ckgen_delay_sel);
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, mmc->host_hw->read_sample_edge);
			MSDC_WRITE32(MSDC_DAT_RDDLY0, mmc->host_hw->each_dat_line_read_rxdly0);
			MSDC_WRITE32(MSDC_DAT_RDDLY1, mmc->host_hw->each_dat_line_read_rxdly1);
		#else
			//set all data line use same delay cycle 
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 0);
			//set all data line use same sample edge	
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 0);

			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, mmc->host_hw->read_dat_latch_ck_sel);
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, mmc->host_hw->read_ckgen_delay_sel);
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, mmc->host_hw->read_sample_edge);
			MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, mmc->host_hw->read_pad_delay);
		#endif
		}
		else
		{
			//set all data line use same delay cycle 
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 0);
			//set all data line use same sample edge	
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 0);

			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, mmc->host_hw->ddr_read_dat_latch_ck_sel);
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, mmc->host_hw->ddr_read_ckgen_delay_sel);
		}
	}
	
	return ret;
}

static UINT32 msdc_hs_write_pre(struct mmc *mmc)
{
	int ret = MMC_ERR_NONE;
	UINT32 base = mmc->base_address;

	// If in tuning process, we not change tune parameters, return directly
	if ( mmc->host_hw->write_pre_setting_en && (!mmc->in_tuning_process))
	{
		if (!mmc->cur_ddr_mode)
		{
		#if TUNE_EACH_DATA_LINE
			/* set each data line has its own delay */
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);
			MSDC_WRITE32(MSDC_DAT_RDDLY0, mmc->host_hw->each_dat_line_write_rxdly0);
		
		#else
			// different data line use different sample edge 
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 1);

			//different data line use different delay cycle 
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, mmc->host_hw->write_dat_latch_ck_sel);
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, mmc->host_hw->write_ckgen_delay_sel);
			MSDC_SET_FIELD(MSDC_DAT_RDDLY0, MSDC_DAT_RDDLY0_D0, mmc->host_hw->write_pad_delay);
			MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, mmc->host_hw->write_internal_delay);
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_WD0_SMPL, mmc->host_hw->write_sample_edge);
		#endif
		}
		else
		{
			// different data line use different sample edge 
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 1);

			//different data line use different delay cycle 
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, mmc->host_hw->ddr_write_dat_latch_ck_sel);
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, mmc->host_hw->ddr_write_ckgen_delay_sel);
		}
	}
	
	return ret;
}


UINT msdc_read_fifo(struct mmc *mmc, struct mmc_data *data)
{
    UINT datsta;
    UINT status = MMC_ERR_NONE;
    UINT base;
    UINT dwLength = data->blocks * data->blocksize;
    void *dst = data->dest;
	UINT retry = 0xFFFF;
	
    base = mmc->base_address;

	SD_LOG(SD_LOG_INFO, "read size = %d", dwLength);
	
	MSDC_DMA_OFF();

	// set to 4bytes access mode
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RISCSZ, MSDC_IOCON_RISCSZ_4BYTE);
	
	MSDC_WRITE32(MSDC_INTEN, MSDC_INT_XFER_COMPL | MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_CSTA );
    
	while(dwLength > 0)
	{
		datsta = MSDC_READ32(MSDC_INT);
        
		if (datsta & MSDC_INT_DATTMO) {
			SD_LOG(SD_LOG_INFO | SD_LOG_ERROR, "Data Timeout");
			status = MMC_ERR_TIMEOUT;
			break;
		}

		if (datsta & MSDC_INT_DATCRCERR) {
		    SD_LOG(SD_LOG_INFO |SD_LOG_ERROR, "Data CRC Error");
			status = MMC_ERR_BADCRC;
			break;
		}

		while((MSDC_RXFIFOCNT() >= 4) && (dwLength > 0)){ 
			*((unsigned int *)dst) = MSDC_READ32(MSDC_RXDATA);
			dst = (char*)dst + 4;
			dwLength -= 4;
		}
	}

	//while(!(MSDC_READ32(MSDC_INT) & MSDC_INT_XFER_COMPL) && dwLength){
	while(!(MSDC_READ32(MSDC_INT) & MSDC_INT_XFER_COMPL)){
        SD_LOG(SD_LOG_INFO, "data transfer is not end.");
		udelay(1000);
		retry--;
		if (retry == 0)
		{
			SD_LOG(SD_LOG_ERROR, "Final can not recevie data transfer complete interrupt in PIO mode. left data size = %d.", dwLength);
			status = MMC_ERR_TIMEOUT;
			break;
		}
    }
	
	MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));

	return status;	
}

int  msdc_write_fifo(struct mmc *mmc, struct mmc_data *data)
{
    UINT dwLength;
    UINT dwFIFOCnt;
    UINT datsta;
    UINT status = MMC_ERR_NONE;
    UINT base;
    void *pBuffer = data->src;
	UINT retry = 0xFFFF;

    base = mmc->base_address;
    dwLength = data->blocks * data->blocksize;
	
	// set to 4bytes access mode
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RISCSZ, MSDC_IOCON_RISCSZ_4BYTE);

	MSDC_DMA_OFF();
    
	while(dwLength > 0)
	{
		dwFIFOCnt = MSDC_TXFIFOCNT();

		if(dwFIFOCnt < 128){
			MSDC_WRITE32(MSDC_TXDATA, *((unsigned int *)pBuffer));
			dwLength -= sizeof(unsigned int);
			pBuffer += sizeof(unsigned int);
		}
	}

	while (retry)
	{
		datsta = MSDC_READ32(MSDC_INT);
		
		if(datsta & MSDC_INT_DATTMO){
			SD_LOG(SD_LOG_ERROR, "Data Timeout");
			status = MMC_ERR_TIMEOUT;
			break;
		}

		if(datsta & MSDC_INT_DATCRCERR){
			SD_LOG(SD_LOG_ERROR, "Data CRC Error");
			status = MMC_ERR_BADCRC;
			break;
		}
		
		if(datsta & MSDC_INT_XFER_COMPL){
	        status = MMC_ERR_NONE;
			break;
		}

		udelay(1000);
		retry--;
	}

	if (retry == 0)
	{
		SD_LOG(SD_LOG_ERROR, "Final can not recevie data transfer complete interrupt in PIO mode.");
		status = MMC_ERR_TIMEOUT;
	}
	
    //Clear the interrupt.
	MSDC_WRITE32(MSDC_INT, datsta);
    
	return status;  
}

//-------------------------------------------------------------------------
/** wait_card_not_busy
 *  Wait command line ready. Use in commad write operation.
 *  @param VOID.
 *  @retval  MMC_ERR_NONE  Success.
 *  @retval  Others  Fail.
 */
//-------------------------------------------------------------------------
static INT32 wait_card_not_busy(struct mmc *mmc)
{
	INT32 ret = 0;
	UINT32 u4SDStatus;
    UINT32 base = mmc->base_address;;
	INT32 timeout = 50000; //5000 * 100us = 500ms
	//INT32 timeout = 0x3FFFF; 

	do{
		u4SDStatus =  MSDC_READ32(SDC_STS);
		udelay(100);
		timeout --;
		if (timeout <= 0){
			ret = MMC_ERR_TIMEOUT;
			//msdc_dump_register(mmc);
			if (current_in_ett_mode == 0)
			{
				SD_LOG(SD_LOG_ERROR, "wait card not busy timeout, slot%d, status = 0x%.8X, PS = 0x%.8X", mmc->block_dev.dev, u4SDStatus, MSDC_READ32(MSDC_PS));
				msdc_dump_register(mmc);
				//mmc_force_reinit(mmc);
				MSDC_RESET();
			}
			break;
		}
	}while(u4SDStatus & (SDC_STS_SDCBUSY | SDC_STS_CMDBUSY));

	return ret;
}

static UINT32 msdc_send_command(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	UINT32 rawcmd = (cmd->opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT));
    UINT32 base = mmc->base_address;

	switch(cmd->rsptype){
    	case MMC_RSP_NONE:
    		rawcmd |= SDC_CMD_RSPTYPE_NO;
    		break;

    	case MMC_RSP_R1:	
    	//case MMC_RSP_R5:				
    	//case MMC_RSP_R6:
    	//case MMC_RSP_R7:
    		rawcmd |= SDC_CMD_RSPTYPE_R1;
    		break;

    	case MMC_RSP_R1b:
    		rawcmd |= SDC_CMD_RSPTYPE_R1B;
    		break;
			
    	case MMC_RSP_R2:
    		rawcmd |= SDC_CMD_RSPTYPE_R2;
    		break;

    	case MMC_RSP_R3:
    		rawcmd |= SDC_CMD_RSPTYPE_R3;
    		break;	
			
    	default:
    		break;
	}

	if (data) {
        if(data->blocks > 1)
        {
        	rawcmd |= SDC_CMD_DTYPE_MULTI_BLK;

			#if MSDC_AUTOCMD12_EN
			// Enable Auto command 12
			rawcmd |= SDC_CMD_AUTOCMD_CMD12;
			#endif
			#if MSDC_AUTOCMD23_EN
			// Enable Auto command 23 only for eMMC
			if (mmc->card_type == MMC_TYPE_MMC)
				rawcmd |= SDC_CMD_AUTOCMD_CMD23;
			else
			rawcmd |= SDC_CMD_AUTOCMD_CMD12;
			#endif
		}
		else
			rawcmd |= SDC_CMD_DTYPE_SINGLE_BLK;
		
        if (data->flags & MMC_DATA_WRITE)
		{
			rawcmd |= (SDC_CMD_WRITE | (data->blocksize << SDC_CMD_LEN_SHIFT));
			
			#if MSDC_ENABLE_WRITE_PRE_SET
			msdc_hs_write_pre(mmc);
			#endif
        }
        else if (data->flags & MMC_DATA_READ)
		{
            rawcmd |= (SDC_CMD_READ | (data->blocksize << SDC_CMD_LEN_SHIFT));
			
			#if MSDC_ENABLE_READ_PRE_SET
			msdc_hs_read_pre(mmc);
			#endif
        }
        MSDC_WRITE32(SDC_BLK_NUM,  data->blocks);
        MSDC_CLRBIT(SDC_CFG, SDC_CFG_SDIO);        
	}

	//if ((cmd->opcode == SD_CMD_APP_SEND_SCR) || (cmd->opcode == SD_CMD_SWITCH_FUNC)){
	if (cmd->opcode == SD_CMD_APP_SEND_SCR){
        rawcmd |= 1 << 11;
    }

    if (cmd->opcode == MMC_CMD_STOP_TRANSMISSION){
        rawcmd |= SDC_CMD_STOP;
    } 

	SDC_SEND_CMD(rawcmd, cmd->arg);
	
	return MMC_ERR_NONE;    
}


//#define MSDC_FORCE_PIO		(1)  // Force to use PIO as transfer mode
int msdc_send_data(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	int ret = MMC_ERR_NONE;
	UINT data_length = data->blocks * data->blocksize;
	
	if ((MMC_DATA_READ == data->flags) && (NULL != data->dest))
	{
		#ifndef MSDC_FORCE_PIO
		if(data_length >= 512)
		{
			msdc_set_timeout(mmc, DAT_TIMEOUT_500MS, 0);
	 		ret = msdc_dma_transfer(mmc, cmd, data);
		}
		else
		{
			msdc_set_timeout(mmc, DAT_TIMEOUT_500MS, 0);
			ret = msdc_read_fifo(mmc, data);
		}

		#else
		
		msdc_set_timeout(mmc, DAT_TIMEOUT_500MS, 0);
		ret = msdc_read_fifo(mmc, data);

		#endif
	}
	else if((MMC_DATA_WRITE == data->flags) && (NULL != data->src))
	{
		#ifndef MSDC_FORCE_PIO
		if(data_length >= 512)
		{
			msdc_set_timeout(mmc, DAT_TIMEOUT_500MS, 0);
	 		ret = msdc_dma_transfer(mmc, cmd, data);
		}
		else
		{
			msdc_set_timeout(mmc, DAT_TIMEOUT_500MS, 0);
			ret = msdc_write_fifo(mmc, data);
		}

		#else
		
		msdc_set_timeout(mmc, DAT_TIMEOUT_500MS, 0);
		ret = msdc_write_fifo(mmc, data);

		#endif
	}
OUT:
	return ret;
}

// ========================================================================
// ETT Releated Functions (Begin)
// ========================================================================
#ifdef CONFIG_MSDC_ETT

#define ETT_RET_OK					(0)
#define ETT_RET_DAT_CRC_ERR			(1)
#define ETT_RET_DAT_TIMEOUT			(2)
#define ETT_RET_CMD_CRC_ERR			(3)
#define ETT_RET_CMD_TIMEOUT			(4)

static unsigned int msdc_ett_ret[2][32];		// Store ett result
static unsigned int ett_success_count = 0;
static unsigned int ett_fail_count = 0;

static int msdc_ett_read_result_print(unsigned int ck_sel, unsigned int ckgen)
{
	int i = 0, j = 0;
	char stat_buf[2][256] = {"\0", "\0"};
	char buf[8];
	int success[2] = {0, 0};
	int fail[2] = {0, 0};

	for (j = 0; j < 2; j++)
	{
		for(i = 0; i < 32; i++)
		{
			if (msdc_ett_ret[j][i] == 0)
			{
				success[j]++;
				ett_success_count++;
			}
			else
			{
				fail[j]++;
				ett_fail_count++;
			}
			sprintf(buf, " %d", msdc_ett_ret[j][i]);
			strcat(stat_buf[j], buf);
		}
	}

	printf("ck_sel<%02d>, ckgen<%02d> | S<%02d> F<%02d>:", ck_sel, ckgen, success[0], fail[0]);
	printf("\t %s \t| S<%02d> F<%02d>:\t %s            \n", stat_buf[0], success[1], fail[1], stat_buf[1]);
	
	return MMC_ERR_NONE;
}


static int msdc_ett_write_result_print(unsigned int ck_sel, unsigned int ckgen, unsigned int pad_delay)
{
	int i = 0, j = 0;
	char stat_buf[2][256] = {"\0", "\0"};
	char buf[8];
	int success[2] = {0, 0};
	int fail[2] = {0, 0};

	for (j = 0; j < 2; j++)
	{
		for(i = 0; i < 32; i++)
		{
			if (msdc_ett_ret[j][i] == 0)
			{
				success[j]++;
				ett_success_count++;
			}
			else
			{
				fail[j]++;
				ett_fail_count++;
			}
			sprintf(buf, " %d", msdc_ett_ret[j][i]);
			strcat(stat_buf[j], buf);
		}
	}

	printf("ck_sel<%02d>, ckgen<%02d> pad_delay<%02d> | S<%02d> F<%02d>:", ck_sel, ckgen, pad_delay, success[0], fail[0]);
	printf("\t %s \t| S<%02d> F<%02d>:\t %s            \n", stat_buf[0], success[1], fail[1], stat_buf[1]);
	
	return MMC_ERR_NONE;
}

static int msdc_ett_ddr_read_result_print(unsigned int ck_sel)
{
	int i = 0;
	char stat_buf[256] = {"\0"};
	char buf[8];
	int success[2] = {0, 0};
	int fail[2] = {0, 0};

	for(i = 0; i < 32; i++)
	{
		if (msdc_ett_ret[0][i] == 0)
		{
			success[0]++;
			ett_success_count++;
		}
		else
		{
			fail[0]++;
			ett_fail_count++;
		}
		sprintf(buf, " %d", msdc_ett_ret[0][i]);
		strcat(stat_buf, buf);
	}
	
	printf("ck_sel<%02d> | S<%02d> F<%02d>:\t %s\n", ck_sel, success[0], fail[0], stat_buf);
	
	return MMC_ERR_NONE;
}


static int msdc_ett_ddr_write_result_print(unsigned int ck_sel)
{
	int i = 0;
	char stat_buf[256] = {"\0"};
	char buf[8];
	int success[2] = {0, 0};
	int fail[2] = {0, 0};

	for(i = 0; i < 32; i++)
	{
		if (msdc_ett_ret[0][i] == 0)
		{
			success[0]++;
			ett_success_count++;
		}
		else
		{
			fail[0]++;
			ett_fail_count++;
		}
		sprintf(buf, " %d", msdc_ett_ret[0][i]);
		strcat(stat_buf, buf);
	}
	
	printf("ck_sel<%02d> | S<%02d> F<%02d>:\t %s\n", ck_sel, success[0], fail[0], stat_buf);
	
	return MMC_ERR_NONE;
}

int msdc_ett_read (struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	unsigned int  base = mmc->base_address;

	unsigned int  ddr = mmc->cur_ddr_mode;
	
	unsigned int  sample_edge;
	unsigned int  pad_delay;

	unsigned int  ckgen;
	unsigned int  ck_sel;

	unsigned int  result = -1;
	unsigned int  dcrc = 0;
	unsigned int  card_status = 0;
	unsigned int retry = 1000;

	current_in_ett_mode = 1;
	ett_success_count = 0;
	ett_fail_count = 0;

	mmc_disable_dump(mmc, 1);

start_read_tune:

	SD_LOG(SD_LOG_TUNE, "enter msdc_read_tune");

	//set all data line use same delay cycle 
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 0);

	//set all data line use same sample edge	
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 0);

	if(ddr) //========================   DDR  ============================
	{
		for(ck_sel = 0; ck_sel < 8; ck_sel++)
		{
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
		
			for(ckgen = 0; ckgen < 32; ckgen++)
			{
				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);

				#if 1
				mmc_send_stopcmd(mmc);
				#else
				mmc_force_reinit(mmc);
				#endif
				MSDC_RESET_HW();

				//Check Card Status						
				do{
					udelay(1000);
					mmc_send_status(mmc, &card_status);
				}while (R1_CURRENT_STATE(card_status) != 4);
				
				// config the command and send it
			    result = msdc_send_command(mmc, cmd, data);
				if (result)
				{
					return result;
				}
			    
			     // Wait for the request to complete.
			    result = msdc_wait_rsp(mmc, cmd);
				if (result == MMC_ERR_BADCRC)
				{
					msdc_ett_ret[0][ckgen] = ETT_RET_CMD_CRC_ERR;
					continue;
				}
				else if (result == MMC_ERR_TIMEOUT)
				{
					msdc_ett_ret[0][ckgen] = ETT_RET_CMD_TIMEOUT;
					continue;
				}
						
				// Re-Read Data
				result = msdc_send_data(mmc, cmd, data);
				MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS|SDC_DCRC_STS_NEG, dcrc);
				
				if((result == MMC_ERR_NONE) && dcrc == 0)
				{
					msdc_ett_ret[0][ckgen] = ETT_RET_OK;
				}
				else if (result == MMC_ERR_BADCRC)
				{
					msdc_ett_ret[0][ckgen] = ETT_RET_DAT_CRC_ERR;
				}
				else if (result == MMC_ERR_TIMEOUT)
				{
					msdc_ett_ret[0][ckgen] = ETT_RET_DAT_TIMEOUT;
				}						
			}

			msdc_ett_ddr_read_result_print(ck_sel);
		}

	}
	else //========================   SDR  ============================
	{
		for(ck_sel = 0; ck_sel < 8; ck_sel++)
		{
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
		
			for(ckgen = 0; ckgen < 32; ckgen++)
			{
				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);
				//SD_LOG(SD_LOG_ERROR, "------------------------------------ Change Ckgen ---------------------------------------");
				for(sample_edge = 0; sample_edge < 2; sample_edge++)
				{
					MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, sample_edge);
				
					for(pad_delay = 0; pad_delay < 32; pad_delay++)
					{
						MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, pad_delay);

						mmc_send_stopcmd(mmc);
						MSDC_RESET_HW();

						//Check Card Status
						//SD_LOG(SD_LOG_ERROR, " Check Card Status ");
						retry = 1000;
						do{
							udelay(1000);
							mmc_send_status(mmc, &card_status);
							//SD_LOG(SD_LOG_ERROR, " Card Status: %d ", R1_CURRENT_STATE(card_status));
							retry--;
							if (retry == 0)
							{
								//SD_LOG(SD_LOG_ERROR, " Force Re-init ");
								mmc_force_reinit(mmc);
								break;
							}
						}while (R1_CURRENT_STATE(card_status) != 4);

						// Config the command and send it
						//SD_LOG(SD_LOG_ERROR, " Send Command ");
						result = msdc_send_command(mmc, cmd, data);
						//SD_LOG(SD_LOG_ERROR, " Wait Responese ");
						// Wait for the request to complete.
						result = msdc_wait_rsp(mmc, cmd);
						if (result == MMC_ERR_BADCRC)
						{
							msdc_ett_ret[sample_edge][pad_delay] = ETT_RET_CMD_CRC_ERR;
							continue;
						}
						else if (result == MMC_ERR_TIMEOUT)
						{
							msdc_ett_ret[sample_edge][pad_delay] = ETT_RET_CMD_TIMEOUT;
							continue;
						}

						// Read Data
						//SD_LOG(SD_LOG_ERROR, " Send Data ");
						result = msdc_send_data(mmc, cmd, data);
						
						MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS, dcrc);
						//SD_LOG(SD_LOG_ERROR, " Get Result ");
						if((result == MMC_ERR_NONE) && dcrc == 0)
						{
							msdc_ett_ret[sample_edge][pad_delay] = ETT_RET_OK;
						}
						else if (result == MMC_ERR_BADCRC)
						{
							msdc_ett_ret[sample_edge][pad_delay] = ETT_RET_DAT_CRC_ERR;
						}
						else if (result == MMC_ERR_TIMEOUT)
						{
							msdc_ett_ret[sample_edge][pad_delay] = ETT_RET_DAT_TIMEOUT;
						}
					}

					if (sample_edge)
						msdc_ett_read_result_print(ck_sel, ckgen);
					
				}
			}
		}
	}
	mmc_disable_dump(mmc, 0);
	SD_LOG(SD_LOG_ERROR, "************************** MSDC%d Read ETT Completed S<%d> F<%d> *****************************", mmc->host_id, ett_success_count, ett_fail_count);
	
done:
	SD_LOG(SD_LOG_TUNE, "exit msdc_read_tune");
	return MMC_ERR_NONE;

}

int msdc_ett_write (struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	unsigned int  base = mmc->base_address;
	unsigned int  ddr = mmc->cur_ddr_mode;

	unsigned int  pad_delay;
	unsigned int  internal_delay;
	unsigned int  sample_edge		= 0;
	unsigned int  current_sample	= 0;
	unsigned int  ckgen 	= 0;
	unsigned int  ck_sel 	= 0;

	unsigned int  result = -1;
	unsigned int  card_status = 0;

	unsigned int retry = 1000;

	current_in_ett_mode = 1;
	ett_success_count = 0; 
	ett_fail_count = 0;

start_write_tune:

	SD_LOG(SD_LOG_TUNE, "enter %s", __FUNCTION__);
	
	// different data line use different sample edge 
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 1);

	//different data line use different delay cycle 
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

	if(ddr) //========================   DDR  ============================
	{
		for(ck_sel = 0; ck_sel < 8; ck_sel++)
		{
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
		
			for(ckgen = 0; ckgen < 32; ckgen++)
			{
				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ck_sel);
				
				#if 1
				mmc_send_stopcmd(mmc);
				#else
				mmc_force_reinit(mmc);
				#endif
				MSDC_RESET_HW();

				//Check Card Status							
				do{
					udelay(1000);
					mmc_send_status(mmc, &card_status);
				}while (R1_CURRENT_STATE(card_status) != 4);
				
				// config the command and send it
			    result = msdc_send_command(mmc, cmd, data);
			    
			     // Wait for the request to complete.
			    result = msdc_wait_rsp(mmc, cmd);
				if (result == MMC_ERR_BADCRC)
				{
					msdc_ett_ret[0][ckgen] = ETT_RET_CMD_CRC_ERR;
					continue;
				}
				else if (result == MMC_ERR_TIMEOUT)
				{
					msdc_ett_ret[0][ckgen] = ETT_RET_CMD_TIMEOUT;
					continue;
				}

				// Re-send Data
				result = msdc_send_data(mmc, cmd, data);
				
				if(result == MMC_ERR_NONE)
				{
					msdc_ett_ret[0][ckgen] = ETT_RET_OK;
				}
				else if (result == MMC_ERR_BADCRC)
				{
					msdc_ett_ret[0][ckgen] = ETT_RET_DAT_CRC_ERR;
				}
				else if (result == MMC_ERR_TIMEOUT)
				{
					msdc_ett_ret[0][ckgen] = ETT_RET_DAT_TIMEOUT;
				}										
			}

			msdc_ett_ddr_write_result_print(ck_sel);
		}

	}
	else //========================   SDR  ============================
	{
		#if (MSDC_WRITE_TUNE_FULL_RANGE)
		for(ck_sel = 0; ck_sel < 8; ck_sel++)
		{
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
		
			for(ckgen = 0; ckgen < 32; ckgen++)
			{
				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);
		#else
		// Use same ck_sel and ckgen delay with read ops
		ck_sel = mmc->host_hw->read_dat_latch_ck_sel;
		ckgen = mmc->host_hw->read_ckgen_delay_sel;
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);
		#endif
				for(pad_delay = 0; pad_delay < 32; pad_delay++)
				{
					MSDC_SET_FIELD(MSDC_DAT_RDDLY0, MSDC_DAT_RDDLY0_D0, pad_delay);
				
					for(sample_edge = 0; sample_edge < 2; sample_edge++)
					{
						MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_WD0_SMPL, sample_edge);
						
						for(internal_delay = 0; internal_delay < 32; internal_delay++)
						{
							MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, internal_delay);
							
							#if 1
							mmc_send_stopcmd(mmc);
							#else
							mmc_force_reinit(mmc);
							#endif
							MSDC_RESET_HW();

							//Check Card Status	
							retry = 1000;
							do{
								udelay(1000);
								mmc_send_status(mmc, &card_status);
								retry--;
								if (retry == 0)
								{
									mmc_force_reinit(mmc);
									break;
								}
							}while (R1_CURRENT_STATE(card_status) != 4);
							
							// config the command and send it
						    result = msdc_send_command(mmc, cmd, data);
						
						     // Wait for the request to complete.
						    result = msdc_wait_rsp(mmc, cmd);
							if (result == MMC_ERR_BADCRC)
							{
								msdc_ett_ret[sample_edge][internal_delay] = ETT_RET_CMD_CRC_ERR;
								continue;
							}
							else if (result == MMC_ERR_TIMEOUT)
							{
								msdc_ett_ret[sample_edge][internal_delay] = ETT_RET_CMD_TIMEOUT;
								continue;
							}

							// Re-send Data
							result = msdc_send_data(mmc, cmd, data);
											
							if(result == MMC_ERR_NONE)
							{
								msdc_ett_ret[sample_edge][internal_delay] = ETT_RET_OK;
							}
							else if (result == MMC_ERR_BADCRC)
							{
								msdc_ett_ret[sample_edge][internal_delay] = ETT_RET_DAT_CRC_ERR;
							}
							else if (result == MMC_ERR_TIMEOUT)
							{
								msdc_ett_ret[sample_edge][internal_delay] = ETT_RET_DAT_TIMEOUT;
								mmc_force_reinit(mmc); //Timeout will cause eMMC stop respond all commands
							}
						}

						if (sample_edge)
							msdc_ett_write_result_print(ck_sel, ckgen, pad_delay);
					}
				}
		#if (MSDC_WRITE_TUNE_FULL_RANGE)
			}
		}
		#endif
	}
	
	SD_LOG(SD_LOG_ERROR, "************************** MSDC%d Write ETT Completed S<%d> F<%d> *****************************", mmc->host_id, ett_success_count, ett_fail_count);
done:

	SD_LOG(SD_LOG_TUNE, "exit %s", __FUNCTION__);
	return MMC_ERR_NONE;
}


int msdc_ett_command(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	unsigned int base = mmc->base_address;
	signed int result = -1;
	unsigned int orgi_pad_delay;
	unsigned int orgi_internal_delay;
	unsigned int orgi_sample_edge;
	unsigned int current_sample_edge;
	unsigned int pad_delay;
	unsigned int internal_delay;
	unsigned int sample_edge;
	unsigned int ckgen;
	unsigned int ck_sel;
	unsigned int  ck_sel_start = 0;
	unsigned int drv = 6;
	unsigned int card_status;
	unsigned int  pad_delay_step = 1;

	
start_cmd_tune:

	SD_LOG(SD_LOG_TUNE, "enter %s", __FUNCTION__);
	
	MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, orgi_pad_delay);
	MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, orgi_internal_delay);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, orgi_sample_edge);

	for(ck_sel = ck_sel_start; ck_sel < 8; ck_sel++)
	{
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
	
		for(ckgen = 0; ckgen < 32; ckgen++)
		{
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);

			for(internal_delay = 0; internal_delay < 32; internal_delay++)
			{
				MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, internal_delay);
				
				for(pad_delay = 0; pad_delay < 32; pad_delay += pad_delay_step)
				{
					MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, pad_delay);

					for(sample_edge = 0; sample_edge < 2; sample_edge++)
					{
						current_sample_edge = (sample_edge + orgi_sample_edge) % 2;
						MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, current_sample_edge);

						#if 1
						mmc_send_stopcmd(mmc);
						#else
						mmc_force_reinit(mmc);
						#endif
						MSDC_RESET_HW();

						//Check Card Status	
						//mmc_send_status(mmc, &card_status);
						//msdc_dump_card_status(card_status);
						do{
							udelay(1000);
							mmc_send_status(mmc, &card_status);
						}while (R1_CURRENT_STATE(card_status) != 4);
						
						
						result = msdc_send_command(mmc, cmd, NULL);
						if (result != MMC_ERR_NONE)
							continue;
						
						result = msdc_wait_rsp(mmc, cmd);		

						if(result == MMC_ERR_TIMEOUT)// if cmd timeout check card status
						{
							mmc_send_status(mmc, &card_status);
							if ((R1_CURRENT_STATE(card_status)) != 4)	
							{
								if (mmc_send_stopcmd(mmc))
								{
									SD_LOG(SD_LOG_ERROR, "mmc send stop cmd fail");
								}
							}
						}
						
						if(result == MMC_ERR_NONE)
						{
							SD_LOG(SD_LOG_ERROR, "TUNE_CMD<%d> [%s] ck_sel<%d> ckgen<%d> current_sample_edge<%d> internal_delay<%d> pad_delay<%d>", 
										  cmd->opcode, (result == MMC_ERR_NONE ? "PASS" : "FAIL"), ck_sel,ckgen,current_sample_edge, internal_delay, pad_delay);
							goto done; 
						}
					}
				}
			}
		}
	}
	
	SD_LOG(SD_LOG_ERROR, "************* Command Tune FAILED ****************");
done:
	SD_LOG(SD_LOG_TUNE, "exit %s", __FUNCTION__);
	return result;

}

#endif //CONFIG_MSDC_ETT
// ========================================================================
// ETT Releated Functions (End)
// ========================================================================



static int msdc_data_tune_clock (struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	int result = MMC_ERR_NONE;
	unsigned int base = mmc->base_address;
	unsigned int tmp_cmd;
	
	if (mmc->tune_clock_times < 2)
	{
		mmc_send_stopcmd(mmc);
		MSDC_RESET_HW();
		mmc->clock = mmc->clock >> 1;
		mmc->tune_clock_times++;
		msdc_set_clock(mmc);

		// config the command and send it
	    result = msdc_send_command(mmc, cmd, data);
		if (result)
		{
			return result;
		}
	    
	     // Wait for the request to complete.
	    result = msdc_wait_rsp(mmc, cmd);
		if((result==MMC_ERR_TIMEOUT)&&(msdc_get_cd(mmc->host_id))){
			printf("no sd%d card 1 exist \n",mmc->host_id);
			return -MMC_ERR_ENOMEDIUM;
		}
		if (result == MMC_ERR_BADCRC)
		{
			tmp_cmd = MSDC_READ32(SDC_CMD);

	        /* check if data is used by the command or not */
	        if (tmp_cmd & 0x1800) {
	            msdc_abort_handler(mmc, 1);
	        }
			
			if(cmd->opcode != MMC_CMD_STOP_TRANSMISSION)
			{
				result = msdc_command_tune(mmc, cmd, NULL);
			}
		}

		// Re-Read Data
		result = msdc_send_data(mmc, cmd, data);
	}
	else {
		result = MMC_ERR_FAILED;
	}
	
	return result;
}

static int msdc_save_read_tune_params(struct mmc *mmc, unsigned int ddr_mode, 
												    unsigned int read_dat_latch_ck_sel,
													unsigned int read_ckgen_delay_sel,
													unsigned int read_sample_edge,
													unsigned int read_pad_delay0,
													unsigned int read_pad_delay1,
													unsigned int tune_all_dat_lines)
{
	if (tune_all_dat_lines)
	{
		if (!ddr_mode)
		{
			mmc->host_hw->read_dat_latch_ck_sel = read_dat_latch_ck_sel;
			mmc->host_hw->read_ckgen_delay_sel	= read_ckgen_delay_sel;
			mmc->host_hw->read_sample_edge		= read_sample_edge;
			mmc->host_hw->read_pad_delay		= read_pad_delay0;
		}
		else
		{
			mmc->host_hw->ddr_read_dat_latch_ck_sel = read_dat_latch_ck_sel;
			mmc->host_hw->ddr_read_ckgen_delay_sel	= read_ckgen_delay_sel;
		}
	}
	else
	{
		mmc->host_hw->read_dat_latch_ck_sel = read_dat_latch_ck_sel;
		mmc->host_hw->read_ckgen_delay_sel	= read_ckgen_delay_sel;
		mmc->host_hw->read_sample_edge		= read_sample_edge;
		mmc->host_hw->each_dat_line_read_rxdly0	= read_pad_delay0;
		mmc->host_hw->each_dat_line_read_rxdly1	= read_pad_delay1;
	}
	return MMC_ERR_NONE;
}


static int msdc_save_write_tune_params(struct mmc *mmc, unsigned int ddr_mode, 
													unsigned int write_dat_latch_ck_sel,
													unsigned int write_ckgen_delay_sel,
													unsigned int write_sample_edge,
													unsigned int write_pad_delay,
													unsigned int write_internal_delay,
													unsigned int tune_all_dat_lines)
{
	if (tune_all_dat_lines)
	{
		if (!ddr_mode)
		{
			mmc->host_hw->write_dat_latch_ck_sel	= write_dat_latch_ck_sel;
			mmc->host_hw->write_ckgen_delay_sel		= write_ckgen_delay_sel;
			mmc->host_hw->write_sample_edge			= write_sample_edge;
			mmc->host_hw->write_pad_delay			= write_pad_delay;
			mmc->host_hw->write_internal_delay		= write_internal_delay;
		}
		else
		{
			mmc->host_hw->ddr_write_dat_latch_ck_sel	= write_dat_latch_ck_sel;
			mmc->host_hw->ddr_write_ckgen_delay_sel		= write_ckgen_delay_sel;
		}
	}
	else
	{
		mmc->host_hw->each_dat_line_write_rxdly0	= write_internal_delay;
	}
	return MMC_ERR_NONE;
}

static int msdc_read_tune_each_dat_line(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	u32 base = mmc->base_address;
	u32 sel = 0;
	u32 ddr = 0;
	u32 dcrc;
	u32 clkmode = 0;
	u32 cur_rxdly0, cur_rxdly1;
	u32 cur_dsmpl = 0, orig_dsmpl;
	u32 cur_dsel = 0, orgi_ckgen;
	u32 cur_dl_cksel = 0, orig_dl_cksel;
	u32 cur_dat0 = 0, cur_dat1 = 0, cur_dat2 = 0, cur_dat3 = 0,
		cur_dat4 = 0, cur_dat5 = 0, cur_dat6 = 0, cur_dat7 = 0;
	u32 orig_dat0, orig_dat1, orig_dat2, orig_dat3, orig_dat4, orig_dat5, orig_dat6, orig_dat7;
	int result = 0;
	u32 loop = 1;
	u32 ck_sel, ckgen, sample_edge, current_sample;
	u32 tmp_cmd = 0, card_status = 0;
	
	#if MSDC_RW_TUNE_CONUT_EN
	unsigned int  tune_count = MSDC_RW_TUNE_COUNT;
	#endif

	/* get CKMOD: 00->divider mode, 01->no divider mode, 02 -> DDR mode */
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, clkmode);
	ddr = (clkmode == 2) ? 1 : 0;

	/* get current CKBUF in CKGEN Delay, 32 stages */
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, orgi_ckgen);

	/* get current Data Latch Delay, 8 stages */
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);

	/* get current Read Data Sample Edge, rising edge or falling edge */
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, orig_dsmpl);

	/* set each data line has its own delay selection */
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

	/********************************************************************************************/
	for(ck_sel = orig_dl_cksel; ck_sel < 8; ck_sel++)
	{
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);

		for(ckgen = orgi_ckgen; ckgen < 32; ckgen++)
		{
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);

			for(sample_edge = 0; sample_edge < 2; sample_edge++)
			{
				current_sample = (sample_edge + orig_dsmpl) % 2;
				MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, current_sample);

				loop = 1;

				while(loop) {

					#if MSDC_RW_TUNE_CONUT_EN
					if (tune_count == 0)
					{
						goto read_clock_tune;
					}
					#endif

					mmc_send_stopcmd(mmc);
					MSDC_RESET_HW();

					// Check Card Status
					do{
						udelay(1000);
						result=mmc_send_status(mmc, &card_status);
						if(result&&(msdc_get_cd(mmc->host_id))){
							printf("no sd%d card 14 exist \n",mmc->host_id);
							return -MMC_ERR_ENOMEDIUM;
							}
						if (R1_CURRENT_STATE(card_status) == 0)
							mmc_force_reinit(mmc);
					}while (R1_CURRENT_STATE(card_status) != 4);

					// Config the command and send it
					result = msdc_send_command(mmc, cmd, data);

					// Wait for the request to complete.
					result = msdc_wait_rsp(mmc, cmd);
					if((result==MMC_ERR_TIMEOUT)&&(msdc_get_cd(mmc->host_id))){
						printf("no sd%d card 2 exist \n",mmc->host_id);
						return -MMC_ERR_ENOMEDIUM;
					}
					if (result == MMC_ERR_BADCRC)
					{
						tmp_cmd = MSDC_READ32(SDC_CMD);

						/* check if data is used by the command or not */
						if (tmp_cmd & 0x1800) {
							msdc_abort_handler(mmc, 1);
						}

						if(cmd->opcode != MMC_CMD_STOP_TRANSMISSION)
						{
							result = msdc_command_tune(mmc, cmd, NULL);
						}
					}
					else if (result == MMC_ERR_TIMEOUT)
					{
						// Need Comfirm
						mmc_force_reinit(mmc);
						result = msdc_command_tune(mmc, cmd, NULL);
					}

					// Re-Read Data
					result = msdc_send_data(mmc, cmd, data);

					/* get value of Data CRC Status */
					MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS | SDC_DCRC_STS_NEG, dcrc);

					/* if it is not DDR mode, mask unconcerned bits */
					if (!ddr) {
						dcrc &= ~SDC_DCRC_STS_NEG;
					}

					MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS, dcrc);

					if((result == MMC_ERR_NONE) && dcrc == 0)
					{
						MSDC_LOG(SD_LOG_ERROR, "TUNE_READ<PASS>: dcrc<0x%x> ck_sel<%d> ckgen<%d> sample_edge<%d>",
								dcrc,ck_sel,ckgen,current_sample);

						// Save tune params for next time used
						msdc_save_read_tune_params(mmc, ddr, ck_sel, ckgen, current_sample, cur_rxdly0, cur_rxdly1, 0);

						goto done;
					}
					else
					{
						/* get current data line delay */
						cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
						cur_rxdly1 = MSDC_READ32(MSDC_DAT_RDDLY1);

						orig_dat0 = (cur_rxdly0 >> 24) & 0x1F;
						orig_dat1 = (cur_rxdly0 >> 16) & 0x1F;
						orig_dat2 = (cur_rxdly0 >> 8) & 0x1F;
						orig_dat3 = (cur_rxdly0 >> 0) & 0x1F;
						orig_dat4 = (cur_rxdly1 >> 24) & 0x1F;
						orig_dat5 = (cur_rxdly1 >> 16) & 0x1F;
						orig_dat6 = (cur_rxdly1 >> 8) & 0x1F;
						orig_dat7 = (cur_rxdly1 >> 0) & 0x1F;
						if (ddr) 
						{
							/* DDR mode, if data line occur CRC error, delay = delay + 1; */
							cur_dat0 = (dcrc & (1 << 0)
									|| dcrc & (1 << 8)) ? (orig_dat0 + 1) : orig_dat0;
							cur_dat1 = (dcrc & (1 << 1)
									|| dcrc & (1 << 9)) ? (orig_dat1 + 1) : orig_dat1;
							cur_dat2 = (dcrc & (1 << 2)
									|| dcrc & (1 << 10)) ? (orig_dat2 + 1) : orig_dat2;
							cur_dat3 = (dcrc & (1 << 3)
									|| dcrc & (1 << 11)) ? (orig_dat3 + 1) : orig_dat3;
							cur_dat4 = (dcrc & (1 << 4)
									|| dcrc & (1 << 12)) ? (orig_dat4 + 1) : orig_dat4;
							cur_dat5 = (dcrc & (1 << 5)
									|| dcrc & (1 << 13)) ? (orig_dat5 + 1) : orig_dat5;
							cur_dat6 = (dcrc & (1 << 6)
									|| dcrc & (1 << 14)) ? (orig_dat6 + 1) : orig_dat6;
							cur_dat7 = (dcrc & (1 << 7)
									|| dcrc & (1 << 15)) ? (orig_dat7 + 1) : orig_dat7;
						}
						else
						{
							/* SDR mode, if data line occur CRC error, delay = delay + 1; */
							cur_dat0 = (dcrc & (1 << 0)) ? (orig_dat0 + 1) : orig_dat0;
							cur_dat1 = (dcrc & (1 << 1)) ? (orig_dat1 + 1) : orig_dat1;
							cur_dat2 = (dcrc & (1 << 2)) ? (orig_dat2 + 1) : orig_dat2;
							cur_dat3 = (dcrc & (1 << 3)) ? (orig_dat3 + 1) : orig_dat3;
							cur_dat4 = (dcrc & (1 << 4)) ? (orig_dat4 + 1) : orig_dat4;
							cur_dat5 = (dcrc & (1 << 5)) ? (orig_dat5 + 1) : orig_dat5;
							cur_dat6 = (dcrc & (1 << 6)) ? (orig_dat6 + 1) : orig_dat6;
							cur_dat7 = (dcrc & (1 << 7)) ? (orig_dat7 + 1) : orig_dat7;
						}

						/* If one data line delay is reach to MAX delay */
						if ((cur_dat0 >= 32) || (cur_dat1 >= 32) || (cur_dat2 >= 32) || (cur_dat3 >= 32) ||
							(cur_dat4 >= 32) || (cur_dat5 >= 32) || (cur_dat6 >= 32) || (cur_dat7 >= 32)) {
							/* Reset data line delay to 0 */
							MSDC_WRITE32(MSDC_DAT_RDDLY0, 0);
							MSDC_WRITE32(MSDC_DAT_RDDLY1, 0);
							// Break this loop for changing each delay
							loop = 0;
							continue;
						}

						/* Write back new delay setting */
						cur_rxdly0 = ((cur_dat0 & 0x1F) << 24) | ((cur_dat1 & 0x1F) << 16) |
								 ((cur_dat2 & 0x1F) << 8) | ((cur_dat3 & 0x1F) << 0);
						cur_rxdly1 = ((cur_dat4 & 0x1F) << 24) | ((cur_dat5 & 0x1F) << 16) |
								 ((cur_dat6 & 0x1F) << 8) | ((cur_dat7 & 0x1F) << 0);

						MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);
						MSDC_WRITE32(MSDC_DAT_RDDLY1, cur_rxdly1);
					}

					#if MSDC_RW_TUNE_CONUT_EN
					tune_count--;
					#endif
				}
			}
		}
	}
	
	/********************************************************************************************/

read_clock_tune:

	#if MSDC_RW_TUNE_CONUT_EN
	if (!tune_count && (result != MMC_ERR_NONE))
	{
		// Reduce work clock and try
		result = msdc_data_tune_clock(mmc, cmd, data);
	}
	#endif


done:
	MSDC_LOG(SD_LOG_TUNE, "exit %s", __FUNCTION__);
	mmc->in_tuning_process = 0;
	return result;
}

static int msdc_read_tune_all_dat_lines(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	unsigned int  base = mmc->base_address;

	unsigned int  ddr = mmc->cur_ddr_mode;

	unsigned int  orgi_ckgen;
	unsigned int  orgi_sample_edge;
	unsigned int  orgi_pad_delay;
	unsigned int  current_sample;
	
	unsigned int  sample_edge;
	unsigned int  pad_delay;

	unsigned int  ckgen;
	unsigned int  ck_sel;
	unsigned int  ck_sel_start = 0;

	unsigned int  result = -1;
	unsigned int  dcrc = 0;
	unsigned int  tmp_cmd = 0;
	unsigned int  card_status = 0;
	unsigned int  pad_delay_step = 1;
	
	#if MSDC_RW_TUNE_CONUT_EN
	unsigned int  tune_count = MSDC_RW_TUNE_COUNT;
	#endif
	
	// Select start Ckgen_Sel
	if (mmc_card_sd(mmc))
	{
		ck_sel_start = 1;
	}
	else
	{
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel_start);
	}

	if (mmc->clock >= MSDC_CLK_100MHZ)
	{
		pad_delay_step = 2;
	}
	else
	{
		pad_delay_step = 3;
	}

	mmc->in_tuning_process = 1;
	
start_read_tune:

	SD_LOG(SD_LOG_TUNE, "enter %s", __FUNCTION__);
	
	//obtain orginal values
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, orgi_ckgen);
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, orgi_sample_edge);
	MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, orgi_pad_delay);

	//set all data line use same delay cycle 
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 0);

	//set all data line use same sample edge	
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 0);

	if(ddr) //========================   DDR  ============================
	{
		for(ck_sel = ck_sel_start; ck_sel < 8; ck_sel++)
		{
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
		
			for(ckgen = orgi_ckgen; ckgen < 32; ckgen++)
			{
				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);

				#if MSDC_RW_TUNE_CONUT_EN
				if (tune_count == 0)
				{
					goto read_clock_tune;
				}
				#endif
				
				#if 1
				mmc_send_stopcmd(mmc);
				#else
				mmc_force_reinit(mmc);
				#endif
				MSDC_RESET_HW();

				//Check Card Status 					
				do{
					udelay(1000);
					result=mmc_send_status(mmc, &card_status);
					if(result&&(msdc_get_cd(mmc->host_id))){
						printf("no sd%d card 15 exist \n",mmc->host_id);
						return -MMC_ERR_ENOMEDIUM;
						}
				}while (R1_CURRENT_STATE(card_status) != 4);

				// config the command and send it
			    result = msdc_send_command(mmc, cmd, data);
				if (result)
				{
					return result;
				}
			    
			     // Wait for the request to complete.
			    result = msdc_wait_rsp(mmc, cmd);
				if((result==MMC_ERR_TIMEOUT)&&(msdc_get_cd(mmc->host_id))){
					printf("no sd%d card 3 exist \n",mmc->host_id);
					return -MMC_ERR_ENOMEDIUM;
				} 
				if (result == MMC_ERR_BADCRC)
				{
					tmp_cmd = MSDC_READ32(SDC_CMD);

			        /* check if data is used by the command or not */
			        if (tmp_cmd & 0x1800) {
			            msdc_abort_handler(mmc, 1);
			        }
					
					if(cmd->opcode != MMC_CMD_STOP_TRANSMISSION)
					{
						result = msdc_command_tune(mmc, cmd, NULL);
					}
				}
						
				// Re-Read Data
				result = msdc_send_data(mmc, cmd, data);

				// Read each data line state for read operation 
				MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS|SDC_DCRC_STS_NEG, dcrc);
				
				if((result == MMC_ERR_NONE) && dcrc == 0)
				{
					SD_LOG(SD_LOG_ERROR, "TUNE_READ<%s> dcrc<0x%x> ck_sel<0x%x> ckgen<0x%x>",
							(result == MMC_ERR_NONE && dcrc == 0) ? "PASS" : "FAIL",dcrc,ck_sel,ckgen);

					// Save tune params for next time used
					msdc_save_read_tune_params(mmc, ddr, ck_sel, ckgen, 0, 0, 0, 1);
					
					goto done;
				}
				else
				{
					// Going On
					//SD_LOG(SD_LOG_ERROR, "TUNE_READ(2): result<0x%x>", result);	
				}

				#if MSDC_RW_TUNE_CONUT_EN
				tune_count--;
				#endif
						
			}
		
		}

	}
	else //========================   SDR  ============================
	{
		for(ck_sel = ck_sel_start; ck_sel < 8; ck_sel++)
		{
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
		
			for(ckgen = orgi_ckgen; ckgen < 32; ckgen++)
			{
				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);
				
				for(sample_edge = 0; sample_edge < 2; sample_edge++)
				{
					current_sample = (sample_edge + orgi_sample_edge) % 2;
					MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, current_sample);
				
					for(pad_delay = orgi_pad_delay; pad_delay < 32; pad_delay += pad_delay_step)
					{
						MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, pad_delay);

						#if MSDC_RW_TUNE_CONUT_EN
						if (tune_count == 0)
						{
							goto read_clock_tune;
						}
						#endif

						#if 1
						mmc_send_stopcmd(mmc);
						#else
						mmc_force_reinit(mmc);
						#endif
						MSDC_RESET_HW();

						//Check Card Status						
						do{
							udelay(1000);
							result =mmc_send_status(mmc, &card_status);
							if(result&&(msdc_get_cd(mmc->host_id))){
								printf("no sd%d card 13 exist \n",mmc->host_id);
								return -MMC_ERR_ENOMEDIUM;
								}
							if (R1_CURRENT_STATE(card_status) == 0)
								mmc_force_reinit(mmc);
						}while (R1_CURRENT_STATE(card_status) != 4);

						// config the command and send it
						result = msdc_send_command(mmc, cmd, data);
						if (result)
						{
							return result;
						}
						
						 // Wait for the request to complete.
						result = msdc_wait_rsp(mmc, cmd);
						if((result==MMC_ERR_TIMEOUT)&&(msdc_get_cd(mmc->host_id))){
							printf("no sd%d card 4 exist \n",mmc->host_id);
							return -MMC_ERR_ENOMEDIUM;
						}
						if (result == MMC_ERR_BADCRC)
						{
							tmp_cmd = MSDC_READ32(SDC_CMD);

							/* check if data is used by the command or not */
							if (tmp_cmd & 0x1800) {
								msdc_abort_handler(mmc, 1);
							}
							
							if(cmd->opcode != MMC_CMD_STOP_TRANSMISSION)
							{
								result = msdc_command_tune(mmc, cmd, NULL);
							}
						}
						else if (result == MMC_ERR_TIMEOUT)
						{
							// Need Comfirm
							mmc_force_reinit(mmc);
							result = msdc_command_tune(mmc, cmd, NULL);
						}

						// Re-Read Data
						result = msdc_send_data(mmc, cmd, data);
						
						MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS, dcrc);
						
						if((result == MMC_ERR_NONE) && dcrc == 0)
						{
							SD_LOG(SD_LOG_ERROR, "TUNE_READ<%s> dcrc<0x%x> ck_sel<%d> ckgen<%d> sample_edge<%d> pad_delay<%d>",
									(result == MMC_ERR_NONE && dcrc == 0) ? "PASS" : "FAIL", dcrc,ck_sel,ckgen,current_sample,pad_delay);

							// Save tune params for next time used
							msdc_save_read_tune_params(mmc, ddr, ck_sel, ckgen, current_sample, pad_delay, 0, 1);

							goto done;
						}
						else
						{
							// Going On
							//SD_LOG(SD_LOG_ERROR, "TUNE_READ(2): result<0x%x>", result);	
						}

						#if MSDC_RW_TUNE_CONUT_EN
						tune_count--;
						#endif
						
					}
				}
			}
		}
	}

	SD_LOG(SD_LOG_ERROR, "************* MSDC Read Tune FAILED ****************");

read_clock_tune:

	#if MSDC_RW_TUNE_CONUT_EN
	if (!tune_count && (result != MMC_ERR_NONE))
	{
		// Reduce work clock and try
		result = msdc_data_tune_clock(mmc, cmd, data);
	}
	#endif
	
done:
	SD_LOG(SD_LOG_TUNE, "exit %s", __FUNCTION__);
	mmc->in_tuning_process = 0;
	return result;

}

static int msdc_read_tune (struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	int ret;
	#if TUNE_EACH_DATA_LINE
		ret=msdc_read_tune_each_dat_line(mmc, cmd, data);
	#else
		ret=msdc_read_tune_all_dat_lines(mmc, cmd, data);
	#endif
	return ret;
}

static int msdc_write_tune_each_dat_line(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	u32 base = mmc->base_address;
	u32 cur_dsmpl = 0, orig_dsmpl;
	u32 cur_rxdly0 = 0;
	u32 orig_dat0, orig_dat1, orig_dat2, orig_dat3;
	u32 cur_dat0 = 0, cur_dat1 = 0, cur_dat2 = 0, cur_dat3 = 0;
	u32 cur_d_cntr = 0, orig_d_cntr;
	int result = 0;
	int clkmode = 0;
	int loop = 1;
	unsigned int  tmp_cmd = 0;
	unsigned int  card_status = 0;
	
	#if MSDC_RW_TUNE_CONUT_EN
	unsigned int  tune_count = MSDC_RW_TUNE_COUNT;
	#endif

	/* get CKMOD: 00->divider mode, 01->no divider mode, 02 -> DDR mode */
	MSDC_GET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, clkmode);

	/* MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, orig_wrrdly); */

	/* get current write data sample edge,  rising edge or falling edge */
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, orig_dsmpl);

	/* get current write data turn around period */
	MSDC_GET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, orig_d_cntr);

	/* set each data line has its own delay */
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

	for (cur_d_cntr = orig_d_cntr; cur_d_cntr < 8; cur_d_cntr++)
	{
		cur_d_cntr = (orig_d_cntr + 1);
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, cur_d_cntr % 8);

		for(cur_dsmpl = orig_dsmpl; cur_dsmpl < 2; cur_dsmpl++)
		{
			/* change write data sample edge */
			cur_dsmpl = (orig_dsmpl + 1);
			MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, cur_dsmpl % 2);

			loop = 1;
			while (loop) {
				cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
				orig_dat0 = (cur_rxdly0 >> 24) & 0x1F;
				orig_dat1 = (cur_rxdly0 >> 16) & 0x1F;
				orig_dat2 = (cur_rxdly0 >> 8) & 0x1F;
				orig_dat3 = (cur_rxdly0 >> 0) & 0x1F;

				cur_dat0 = (orig_dat0 + 1);	/* only adjust bit-1 for crc */
				cur_dat1 = orig_dat1;
				cur_dat2 = orig_dat2;
				cur_dat3 = orig_dat3;

				if (cur_dat0 >= 32)
				{
					loop = 0;
					continue;
				}
				cur_rxdly0 = ((cur_dat0 & 0x1F) << 24) | ((cur_dat1 & 0x1F) << 16) |
					     ((cur_dat2 & 0x1F) << 8) | ((cur_dat3 & 0x1F) << 0);
				MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);

				mmc_send_stopcmd(mmc);
				MSDC_RESET_HW();

				//Check Card Status
				do{
					udelay(1000);
					result=mmc_send_status(mmc, &card_status);
					if(result&&(msdc_get_cd(mmc->host_id))){
						printf("no sd%d card 16 exist \n",mmc->host_id);
						return -MMC_ERR_ENOMEDIUM;
						}
				}while (R1_CURRENT_STATE(card_status) != 4);
				
				// config the command and send it
				result = msdc_send_command(mmc, cmd, data);

				// Wait for the request to complete.
				result = msdc_wait_rsp(mmc, cmd);
				if((result==MMC_ERR_TIMEOUT)&&(msdc_get_cd(mmc->host_id))){
					printf("no sd%d card 5 exist \n",mmc->host_id);
					return -MMC_ERR_ENOMEDIUM;
				}
				if (result == MMC_ERR_BADCRC)
				{
					tmp_cmd = MSDC_READ32(SDC_CMD);

				/* check if data is used by the command or not */
					if (tmp_cmd & 0x1800) {
						msdc_abort_handler(mmc, 1);
					}

					if(cmd->opcode != MMC_CMD_STOP_TRANSMISSION)
					{
						result = msdc_command_tune(mmc, cmd, NULL);
					}
				}
				else if (result == MMC_ERR_TIMEOUT)
				{
					// Need Comfirm
					mmc_force_reinit(mmc);
					result = msdc_command_tune(mmc, cmd, NULL);
				}

				// Re-send Data
				result = msdc_send_data(mmc, cmd, data);
				if(result == MMC_ERR_NONE)
				{
					MSDC_LOG(SD_LOG_ERROR, "TUNE_WRITE<PASS> cur_rxdly0<%d>", cur_rxdly0);

					// Save tune params for next time used
					msdc_save_write_tune_params(mmc, 0, 0, 0, 0, 0, cur_rxdly0, 0);

					goto done;
				}

				#if MSDC_RW_TUNE_CONUT_EN
				tune_count--;
				#endif
			}
		}
	}

write_clock_tune:
	#if MSDC_RW_TUNE_CONUT_EN
		if (!tune_count && (result != MMC_ERR_NONE))
		{
			// Reduce work clock and try
			result = msdc_data_tune_clock(mmc, cmd, data);
		}
	#endif
		
done:
	
	SD_LOG(SD_LOG_TUNE, "exit %s", __FUNCTION__);
	mmc->in_tuning_process = 0;
	return result;

}

static int msdc_write_tune_all_dat_lines(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	unsigned int  base = mmc->base_address;
	unsigned int  ddr = mmc->cur_ddr_mode;
	unsigned int  orgi_ckgen;
	unsigned int  orgi_pad_delay;
	unsigned int  orgi_internal_delay;
	unsigned int  orgi_sample_dege;
	unsigned int  pad_delay;
	unsigned int  internal_delay;
	unsigned int  sample_edge;
	unsigned int  current_sample;
	unsigned int  ckgen;
	unsigned int  ck_sel;
	unsigned int  ck_sel_start = 0;

	unsigned int  result = -1;
	unsigned int  tmp_cmd = 0;
	unsigned int  card_status = 0;
	unsigned int  pad_delay_step = 1;

	#if MSDC_RW_TUNE_CONUT_EN
	unsigned int  tune_count = MSDC_RW_TUNE_COUNT;
	#endif

	// Select start Ckgen_Sel
	if (mmc_card_sd(mmc))
	{
		ck_sel_start = 1;
	}
	else
	{
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel_start);
	}

	if (mmc->clock >= MSDC_CLK_100MHZ)
	{
		pad_delay_step = 2;
	}
	else
	{
		pad_delay_step = 3;
	}
		
	mmc->in_tuning_process = 1;

start_write_tune:

	SD_LOG(SD_LOG_TUNE, "enter %s", __FUNCTION__);
	
	//only tune data0 
	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, orgi_ckgen);
	MSDC_GET_FIELD(MSDC_DAT_RDDLY0, MSDC_DAT_RDDLY0_D0, orgi_pad_delay);
	MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, orgi_internal_delay);
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_WD0_SMPL, orgi_sample_dege);

	// different data line use different sample edge 
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 1);

	//different data line use different delay cycle 
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

	if(ddr) //========================   DDR  ============================
	{
		for(ck_sel = ck_sel_start; ck_sel < 8; ck_sel++)
		{
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
		
			for(ckgen = orgi_ckgen; ckgen < 32; ckgen++)
			{
				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ck_sel);

				#if MSDC_RW_TUNE_CONUT_EN
				if (tune_count == 0)
				{
					goto write_clock_tune;
				}
				#endif
				
				#if 1
				mmc_send_stopcmd(mmc);
				#else
				mmc_force_reinit(mmc);
				#endif
				MSDC_RESET_HW();

				//Check Card Status							
				do{
					udelay(1000);
					result=mmc_send_status(mmc, &card_status);
					if(result&&(msdc_get_cd(mmc->host_id))){
						printf("no sd%d card 17 exist \n",mmc->host_id);
						return -MMC_ERR_ENOMEDIUM;
						}
				}while (R1_CURRENT_STATE(card_status) != 4);
				
				// config the command and send it
			    result = msdc_send_command(mmc, cmd, data);
				if (result)
				{
					return result;
				}
			    
			     // Wait for the request to complete.
			    result = msdc_wait_rsp(mmc, cmd);
				 if((result==MMC_ERR_TIMEOUT)&&(msdc_get_cd(mmc->host_id))){
					 printf("no sd%d card 6 exist \n",mmc->host_id);
					 return -MMC_ERR_ENOMEDIUM;
				 }
				 
				if (result == MMC_ERR_BADCRC)
				{
					tmp_cmd = MSDC_READ32(SDC_CMD);

			        /* check if data is used by the command or not */
			        if (tmp_cmd & 0x1800) {
			            msdc_abort_handler(mmc, 1);
			        }
					
					if(cmd->opcode != MMC_CMD_STOP_TRANSMISSION)
					{
						result = msdc_command_tune(mmc, cmd, NULL);
					}
				}
				else if (result == MMC_ERR_TIMEOUT)
				{
					// Need Comfirm
					mmc_force_reinit(mmc);
					result = msdc_command_tune(mmc, cmd, NULL);
				}

				// Re-send Data
				result = msdc_send_data(mmc, cmd, data);
				

				if(result == MMC_ERR_NONE)
				{
					SD_LOG(SD_LOG_ERROR, "TUNE_WRITE<%s> ck_sel<%d> ckgen<%d>", 
								  (result == MMC_ERR_NONE ? "PASS" : "FAIL"), ck_sel,ckgen);
					// Save tune params for next time used
					msdc_save_write_tune_params(mmc, ddr, ck_sel, ckgen, 0, 0, 0, 1);
					goto done;
				}
				else
				{
					// Going On
					//SD_LOG(SD_LOG_ERROR, "TUNE_WRITE(2): result<0x%x>", result);
				}		

				#if MSDC_RW_TUNE_CONUT_EN
				tune_count--;
				#endif
				
			}
		}

	}
	else //========================   SDR  ============================
	{
		for(ck_sel = ck_sel_start; ck_sel < 8; ck_sel++)
		{
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
		
			for(ckgen = orgi_ckgen; ckgen < 32; ckgen++)
			{
				MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);

				for(pad_delay = 0; pad_delay < 32; pad_delay += pad_delay_step)
				{
					MSDC_SET_FIELD(MSDC_DAT_RDDLY0, MSDC_DAT_RDDLY0_D0, pad_delay);
				
					for(internal_delay = 0; internal_delay < 32; internal_delay ++)
					{
						MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, internal_delay);

						for(sample_edge = 0; sample_edge < 2; sample_edge++)
						{
							current_sample = (sample_edge+orgi_sample_dege)%2;
							MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_WD0_SMPL, current_sample);

							#if MSDC_RW_TUNE_CONUT_EN
							if (tune_count == 0)
							{
								goto write_clock_tune;
							}
							#endif

							#if 1
							mmc_send_stopcmd(mmc);
							#else
							mmc_force_reinit(mmc);
							#endif
							MSDC_RESET_HW();

							//Check Card Status							
							do{
								udelay(1000);
								result=mmc_send_status(mmc, &card_status);
								if(result&&(msdc_get_cd(mmc->host_id))){
									printf("no sd%d card 18 exist \n",mmc->host_id);
									return -MMC_ERR_ENOMEDIUM;
									}
							}while (R1_CURRENT_STATE(card_status) != 4);
							
							// config the command and send it
						    result = msdc_send_command(mmc, cmd, data);
							if (result)
							{
								return result;
							}
						    
						     // Wait for the request to complete.
						    result = msdc_wait_rsp(mmc, cmd);
							if((result==MMC_ERR_TIMEOUT)&&(msdc_get_cd(mmc->host_id))){
								printf("no sd%d card 7 exist \n",mmc->host_id);
								return -MMC_ERR_ENOMEDIUM;
							}
							if (result == MMC_ERR_BADCRC)
							{
								tmp_cmd = MSDC_READ32(SDC_CMD);

						        /* check if data is used by the command or not */
						        if (tmp_cmd & 0x1800) {
						            msdc_abort_handler(mmc, 1);
						        }
								
								if(cmd->opcode != MMC_CMD_STOP_TRANSMISSION)
								{
									result = msdc_command_tune(mmc, cmd, NULL);
								}
							}
							else if (result == MMC_ERR_TIMEOUT)
							{
								// Need Comfirm
								mmc_force_reinit(mmc);
								result = msdc_command_tune(mmc, cmd, NULL);
							}

							// Re-send Data
							result = msdc_send_data(mmc, cmd, data);
											
							if(result == MMC_ERR_NONE)
							{
								SD_LOG(SD_LOG_ERROR, "TUNE_WRITE<%s> ck_sel<%d> ckgen<%d> DSPL<%d> pad_delay<%d> internal_delay<%d>", 
											  (result == MMC_ERR_NONE ? "PASS" : "FAIL"), ck_sel, ckgen, current_sample, pad_delay, internal_delay);

								// Save tune params for next time used
								msdc_save_write_tune_params(mmc, ddr, ck_sel, ckgen, current_sample, pad_delay, internal_delay, 1);

								goto done;
							}
							else
							{
								// Going On
								//SD_LOG(SD_LOG_ERROR, "TUNE_WRITE(2): result<0x%x>", result);
							}

							#if MSDC_RW_TUNE_CONUT_EN
							tune_count--;
							#endif
							
						}
					}
				}
			}
		}
	}
	
	SD_LOG(SD_LOG_ERROR, "************* Write Tune FAILED ****************");

write_clock_tune:
	#if MSDC_RW_TUNE_CONUT_EN
	if (!tune_count && (result != MMC_ERR_NONE))
	{
		// Reduce work clock and try
		result = msdc_data_tune_clock(mmc, cmd, data);
	}
	#endif
	
done:

	SD_LOG(SD_LOG_TUNE, "exit %s", __FUNCTION__);
	mmc->in_tuning_process = 0;
	return result;
}

static int msdc_write_tune (struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	int ret;
	#if TUNE_EACH_DATA_LINE
		ret=msdc_write_tune_each_dat_line(mmc, cmd, data);
	#else
		ret=msdc_write_tune_all_dat_lines(mmc, cmd, data);
	#endif
	return ret;
}

int msdc_command_tune (struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	unsigned int base = mmc->base_address;
	signed int result = -1;
	unsigned int orgi_ckgen;
	unsigned int orgi_pad_delay;
	unsigned int orgi_internal_delay;
	unsigned int orgi_sample_edge;
	unsigned int current_sample_edge;
	unsigned int pad_delay;
	unsigned int internal_delay;
	unsigned int sample_edge;
	unsigned int ckgen;
	unsigned int ck_sel;
	unsigned int  ck_sel_start = 0;
	unsigned int card_status;
	unsigned int  pad_delay_step = 1;
	unsigned int i = 0;

	// Select start Ckgen_Sel
	if (mmc_card_sd(mmc))
	{
		ck_sel_start = 1;
	}
	else
	{
		MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel_start);
	}

	if (mmc->clock >= MSDC_CLK_100MHZ)
	{
		pad_delay_step = 2;
	}
	else
	{
		pad_delay_step = 3;
	}
	
start_cmd_tune:

	SD_LOG(SD_LOG_TUNE, "enter %s", __FUNCTION__);

	MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, orgi_ckgen);
	MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, orgi_pad_delay);
	MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, orgi_internal_delay);
	MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, orgi_sample_edge);

	for(ck_sel = ck_sel_start; ck_sel < 8; ck_sel++)
	{
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, ck_sel);
	
		for(ckgen = orgi_ckgen; ckgen < 32; ckgen++)
		{
			MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, ckgen);

			for(internal_delay = orgi_internal_delay; internal_delay < 32; internal_delay++)
			{
				MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, internal_delay);
				
				for(pad_delay = orgi_pad_delay; pad_delay < 32; pad_delay += pad_delay_step)
				{
					MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, pad_delay);

					for(sample_edge = 0; sample_edge < 2; sample_edge++)
					{
						current_sample_edge = (sample_edge + orgi_sample_edge) % 2;
						MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, current_sample_edge);

						#if 1
						mmc_send_stopcmd(mmc);
						#else
						mmc_force_reinit(mmc);
						#endif
						MSDC_RESET_HW();

						//Check Card Status
						do{
							udelay(1000);
							result=mmc_send_status(mmc, &card_status);
							if(result&&(msdc_get_cd(mmc->host_id))){
								printf("no sd%d card 19 exist \n",mmc->host_id);
								return -MMC_ERR_ENOMEDIUM;
							}
							i++;
							if (!(i % 5))
							{
								//msdc_dump_register(mmc);
								// If card still in wrong status, we force to re_init it.
								msdc_force_reinit(mmc);
								mmc_force_reinit(mmc);
								break;
							}
						}while (R1_CURRENT_STATE(card_status) != 4);

						result = msdc_send_command(mmc, cmd, NULL);
						if (result != MMC_ERR_NONE)
							continue;

						result = msdc_wait_rsp(mmc, cmd);
						if((result==MMC_ERR_TIMEOUT)&&(msdc_get_cd(mmc->host_id))){
							printf("no sd%d card 8 exist \n",mmc->host_id);
							return -MMC_ERR_ENOMEDIUM;
						}

						if(result == MMC_ERR_TIMEOUT)// if cmd timeout check card status
						{
							mmc_send_status(mmc, &card_status);
							if ((R1_CURRENT_STATE(card_status)) != 4)
							{
								if (mmc_send_stopcmd(mmc))
								{
									SD_LOG(SD_LOG_ERROR, "mmc send stop cmd fail");
								}
							}
						}

						if(result == MMC_ERR_NONE)
						{
							if (cmd->opcode != MMC_CMD_SEND_STATUS)
								SD_LOG(SD_LOG_ERROR, "TUNE_CMD<%d> [%s] ck_sel<%d> ckgen<%d> current_sample_edge<%d> internal_delay<%d> pad_delay<%d>", 
										  cmd->opcode, (result == MMC_ERR_NONE ? "PASS" : "FAIL"), ck_sel,ckgen,current_sample_edge, internal_delay, pad_delay);
							goto done; 
						}
					}
				}
			}
		}
	}
	
	SD_LOG(SD_LOG_ERROR, "************* Command Tune FAILED ****************");
done:
	SD_LOG(SD_LOG_TUNE, "exit %s", __FUNCTION__);
	return result;

}


u32 msdc_intr_wait(struct mmc *mmc, u32 intrs)
{
    u32 base = mmc->base_address;
    u32 sts;

    /* warning that interrupts are not enabled */
    //WARN_ON((MSDC_READ32(MSDC_INTEN) & intrs) != intrs);
	   
	//u32 tmo = 0x3FFFFF;
	u32 tmo = 0x5FFFF;
	MSDC_WAIT_COND_TMO(((sts = MSDC_READ32(MSDC_INT)) & intrs), tmo);
	
	if (tmo == 0) 
	{
	    SD_LOG(SD_LOG_ERROR, "WARNNING ==> Wait INT timeout");
	    MSDC_RESET();
	}

    //MSG(INT, "[SD%d] INT(0x%x)\r\n", mmc->id, sts);

	// Clear Interrupt
    MSDC_WRITE32(MSDC_INT, (sts & intrs));

    if (~intrs & sts) 
	{
        SD_LOG(SD_LOG_ERROR, "<CHECKME> Unexpected INT(0x%x)", ~intrs & sts);
    }
    return sts;
}

int msdc_wait_rsp(struct mmc *mmc, struct mmc_cmd *cmd)
{
    u32 base   = mmc->base_address;
    u32 rsptyp = cmd->rsptype;
    u32 status;
    u32 opcode = (cmd->opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT));
    u32 error = MMC_ERR_NONE;
	
    u32 wints = MSDC_INT_CMDTMO | MSDC_INT_CMDRDY | MSDC_INT_RSPCRCERR |
        MSDC_INT_ACMDRDY | MSDC_INT_ACMDCRCERR | MSDC_INT_ACMDTMO |
        MSDC_INT_ACMD19_DONE;

    //if (cmd->opcode == MMC_CMD_GO_IRQ_STATE)
    //    wints |= MSDC_INT_MMCIRQ;

    status = msdc_intr_wait(mmc, wints);

    if (status == 0) 
	{
        error = MMC_ERR_TIMEOUT;
        goto end;
    }

    if ((status & MSDC_INT_CMDRDY) || (status & MSDC_INT_ACMDRDY) || (status & MSDC_INT_ACMD19_DONE)) 
    {
        switch (rsptyp) 
		{
		    case MMC_RSP_NONE:
		        SD_LOG(SD_LOG_RSP, "CMD(%d): RSP(%d)", opcode, rsptyp);
		        break;
				
		    case MMC_RSP_R2:
				cmd->resp[0]  = MSDC_READ32(SDC_RESP3);  
            	cmd->resp[1]  = MSDC_READ32(SDC_RESP2);  
            	cmd->resp[2]  = MSDC_READ32(SDC_RESP1);  
            	cmd->resp[3]  = MSDC_READ32(SDC_RESP0);
				
		        SD_LOG(SD_LOG_RSP, "CMD(%d): RSP(%d) = 0x%x 0x%x 0x%x 0x%x", 
		            opcode, cmd->rsptype, cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);         
		        break;
			
		    default: /* Response types 1, 3, 4, 5, 6, 7(1b) */
		        if ((status & MSDC_INT_ACMDRDY) || (status & MSDC_INT_ACMD19_DONE))
		            cmd->resp[0] = MSDC_READ32(SDC_ACMD_RESP);
		        else
		            cmd->resp[0] = MSDC_READ32(SDC_RESP0);
		        SD_LOG(SD_LOG_RSP, "CMD(%d): RSP(%d) = 0x%x AUTO(%d)", opcode, 
		            cmd->rsptype, cmd->resp[0], 
		            ((status & MSDC_INT_ACMDRDY) || (status & MSDC_INT_ACMD19_DONE)) ? 1 : 0);
		        break;
		}
    } 
	else if ((status & MSDC_INT_RSPCRCERR) || (status & MSDC_INT_ACMDCRCERR)) 
    {
        error = MMC_ERR_BADCRC;
        SD_LOG(SD_LOG_RSPERR, "CMD(%d): RSP(%d) ERR(BADCRC)", opcode, cmd->rsptype);
    } 
	else if ((status & MSDC_INT_CMDTMO) || (status & MSDC_INT_ACMDTMO)) 
    {
        error = MMC_ERR_TIMEOUT;
        SD_LOG(SD_LOG_RSPERR, "CMD(%d): RSP(%d) ERR(CMDTO) AUTO(%d)", 
            opcode, cmd->rsptype, status & MSDC_INT_ACMDTMO ? 1: 0);
    } 
	else 
    {
        error = MMC_ERR_INVALID;
        SD_LOG(SD_LOG_RSPERR, "CMD(%d): RSP(%d) ERR(INVALID), Status:%x", opcode, cmd->rsptype, status);        
    }

end:


#if 0    
    if ((error == MMC_ERR_NONE) && (MSG_EVT_MASK & MSG_EVT_RSP)){
        switch(cmd->rsptype) {
        case RESP_R1:
        case RESP_R1B:
            msdc_dump_card_status(cmd->resp[0]);
            break;
        case RESP_R3:
            msdc_dump_ocr_reg(cmd->resp[0]);
            break;
        case RESP_R5:
            msdc_dump_io_resp(cmd->resp[0]);
            break;
        case RESP_R6:
            msdc_dump_rca_resp(cmd->resp[0]);
            break;
        }
    }
#endif

    cmd->error = error;
 
    return error;
}


void msdc_abort(struct mmc *mmc)
{
    u32 base = mmc->base_address;
	u32 card_status = 0;
	
	//mmc_send_status(mmc, &card_status);
	//msdc_dump_card_status(card_status);

    SD_LOG(SD_LOG_ERROR, "Abort: MSDC_FIFOCS=0x%08X MSDC_PS=0x%08X SDC_STS=0x%08X", 
        				MSDC_READ32(MSDC_FIFOCS), MSDC_READ32(MSDC_PS), MSDC_READ32(SDC_STS));

    /* reset controller */
    MSDC_RESET();

    /* clear fifo */
    MSDC_CLR_FIFO();

    /* make sure txfifo and rxfifo are empty */
    if (MSDC_TXFIFOCNT() != 0 || MSDC_RXFIFOCNT() != 0) {
        SD_LOG(SD_LOG_ERROR, "Abort: TXFIFO(%d), RXFIFO(%d) != 0\r\n",
            			MSDC_TXFIFOCNT(), MSDC_RXFIFOCNT());
    }

    /* clear all interrupts */
    MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));
}

int msdc_is_fake_error(struct mmc *mmc)
{
	u32 card_status = 0;

	return 0;

	mmc_send_status(mmc, &card_status);
	card_status = R1_CURRENT_STATE(card_status);

	if ((card_status == 5) || (card_status == 6))
	{
		if (current_in_ett_mode == 0)
			SD_LOG(SD_LOG_ERROR, "crc error is a fake error, current card state: %d", card_status);
		return 1;
	}

	return 0;
	
}

void msdc_abort_handler(struct mmc *mmc, int abort_card)
{
    u32 base = mmc->base_address;
    struct mmc_cmd stop;

	msdc_abort(mmc);

	// Send STOP command
    if (abort_card)  {
        stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
        stop.rsptype = RESP_R1B;
        stop.arg     = 0;
        stop.retries = CMD_RETRIES;
        stop.timeout = CMD_TIMEOUT;
        msdc_send_cmd(mmc, &stop, NULL);
        msdc_wait_rsp(mmc, &stop);
    }

	
}

// Interface Function for mmc 
int msdc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
    INT32 ret;
	UINT32 tmp_cmd = 0;
	UINT32 card_status = 0;
    UINT32 base = mmc->base_address;
	
restart:
	msdc_set_timeout(mmc, DAT_TIMEOUT_100MS, 0);
    
	ret = wait_card_not_busy(mmc);
	if (ret != MMC_ERR_NONE)
	{
		return ret;
	}

	// Clear Interrupts before send command, make sure that command env is clear.
	MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));
	
    // config the command and send it
    ret = msdc_send_command(mmc, cmd, data);
	if (ret)
	{
		return ret;
	}
    
     // Wait for the request to complete.
    ret = msdc_wait_rsp(mmc, cmd);

	if((ret==MMC_ERR_TIMEOUT)&&(msdc_get_cd(mmc->host_id))){
		printf("no sd%d card 10 exist \n",mmc->host_id);
		return -MMC_ERR_ENOMEDIUM;
	}
	//=====================
	if ((cmd->opcode == MMC_CMD_READ_SINGLE_BLOCK) || (cmd->opcode == MMC_CMD_READ_MULTIPLE_BLOCK))
	{
		//mmc_send_status(mmc, &card_status);
		//msdc_dump_card_status(card_status);
	}
	//=====================
	if (ret == MMC_ERR_BADCRC)
	{
		if (!msdc_is_fake_error(mmc))
		{
			tmp_cmd = MSDC_READ32(SDC_CMD);

	        /* check if data is used by the command or not */
	        if (tmp_cmd & 0x1800) {
	            msdc_abort_handler(mmc, 1);
	        }

			//#ifdef CONFIG_MSDC_ETT
			if ((cmd->opcode != MMC_CMD_STOP_TRANSMISSION) && (cmd->opcode != MMC_CMD_SEND_STATUS))
			//#else
			//if (cmd->opcode != MMC_CMD_STOP_TRANSMISSION)
			//#endif
			{
				ret = msdc_command_tune(mmc, cmd, NULL);
			}
		}
	}
	else if (ret == MMC_ERR_TIMEOUT)
	{
		if((mmc->host_inited == 0)
			&& (mmc->host_id == 0)
			&& (cmd->opcode == MMC_CMD_SEND_EXT_CSD)
			&& (mmc->clock <= MSDC_INIT_CLOCK)) {

			SD_LOG(SD_LOG_ERROR, "msdc_wait_rsp TIMEOUT on CMD%d", cmd->opcode);
			return ret;
		}
		else if ((cmd->opcode == MMC_CMD_READ_SINGLE_BLOCK) ||
			(cmd->opcode == MMC_CMD_READ_MULTIPLE_BLOCK) ||
			(cmd->opcode == MMC_CMD_WRITE_SINGLE_BLOCK) ||
			(cmd->opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK))
		{
			/*mmc_send_status(mmc, &card_status);
			if((R1_CURRENT_STATE(card_status))!= 4)	
			{
				if(mmc_send_stopcmd(mmc))
				{
					
				}
			}*/
			ret = msdc_command_tune(mmc, cmd, NULL);
		}
	}



	/*if ((cmd->opcode == MMC_CMD_READ_SINGLE_BLOCK) ||
			(cmd->opcode == MMC_CMD_READ_MULTIPLE_BLOCK) ||
			(cmd->opcode == MMC_CMD_WRITE_SINGLE_BLOCK) ||
			(cmd->opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK))
	{
		if (data == NULL)
			SD_LOG(SD_LOG_ERROR, "XXXXXXXX data Err XXXXXXXXX");
	}*/

    //read/write data if is a read/write cmd 
    if ((ret == MMC_ERR_NONE) && data)
	{
		ret = msdc_send_data(mmc, cmd, data);
		if(ret&&(msdc_get_cd(mmc->host_id))){
			printf("no sd%d card 11 exist \n",mmc->host_id);
			return -MMC_ERR_ENOMEDIUM;
		}
		if((ret == MMC_ERR_TIMEOUT) && (mmc->host_inited == 0)
			&& (mmc->host_id == 0) && (cmd->opcode == MMC_CMD_SEND_EXT_CSD) && (mmc->clock <= MSDC_INIT_CLOCK)) {

			SD_LOG(SD_LOG_ERROR, "msdc_send_data TIMEOUT on CMD%d\n", cmd->opcode);
			return ret;
		}
		else if ((ret == MMC_ERR_BADCRC) || (ret == MMC_ERR_TIMEOUT))
		{
			SD_LOG(SD_LOG_ERROR, "Start Data Tuning: <CMD%d, BLK %d>",cmd->opcode, data->blocks);

		    if((mmc->host_id != 0) || (ret != MMC_ERR_TIMEOUT)) {
			#if (MSDC_FORCE_CLOCK_TUNE == 0)
			if (data->flags & MMC_DATA_READ) {
				ret = msdc_read_tune(mmc, cmd, data);
			}
			else {
				ret = msdc_write_tune(mmc, cmd, data);
			}
			#else
			ret = msdc_data_tune_clock(mmc, cmd, data);
			#endif
			if(ret&&(msdc_get_cd(mmc->host_id))){
				printf("no sd%d card 20 exist \n",mmc->host_id);
				return -MMC_ERR_ENOMEDIUM;
				}
		    } else {//meet emmc TIMEOUT
			    SD_LOG(SD_LOG_ERROR, "cmd%d timeout, force reinit\n", cmd->opcode);
			    msdc_force_reinit(mmc);
			    ret = mmc_force_reinit(mmc);
			    if(ret) {
				    SD_LOG(SD_LOG_ERROR, "cmd%d tuning: reinit fail\n", cmd->opcode);
			    }
			    else {
				    goto restart;
			    }
		    }

			SD_LOG(SD_LOG_ERROR, "End Data Tuning: <CMD%d, BLK %d>",cmd->opcode, data->blocks);
		}
    }
    
    return MMC_ERR_NONE;
}

void msdc_force_reinit(struct mmc *mmc)
{
	int ret;
	unsigned int base = mmc->base_address;

	msdc_mmc_reinit_id = mmc->host_id;
	MSDC_CLOCK_GATE(mmc, GATE_DISABLE_CLOCK);
	msdc_sw_reset_whole_module(mmc);
	ret = msdc_initialize(gd->bd);
}

int msdc_set_clock(struct mmc *mmc)
{
	int ret = MMC_ERR_NONE;
	unsigned int clock_rate;
    unsigned expFreq = 0;
    int divider;
    unsigned base = mmc->base_address;

	// Set the clock speed
    clock_rate = mmc->clock;

	// Special Setting for HS200
	if (mmc->clock > MSDC_CLK_100MHZ)
	{
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_WRDAT_CRCS, 2);
		MSDC_SET_FIELD(MSDC_PATCH_BIT1, MSDC_PATCH_BIT1_CMD_RSP, 2);
	}

	if (mmc->ddr_mode) // DDR mode
	{
		// Set to DDR mode
		MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, 0x02);

		for (divider = 0; divider <= 0xFF; divider++) {
			if (0 == divider) {
				expFreq = msdc_host_clk_freq[mmc->host_id] / 2;
			}
			else {
				expFreq = msdc_host_clk_freq[mmc->host_id] / 4 / divider;
			}

			if (expFreq <= clock_rate) {
				MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKDIV, divider);

				// Wait until clock is stable
				MSDC_RETRY_DEF(!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));
			    			
				SD_LOG(SD_LOG_ERROR, "[DDR]request clock is %dKHz, clock has been set to %dKHz", clock_rate/1000, expFreq/1000);
	        	break;
		    }
    	}
	}
	else  // SDR mode
	{
		if (clock_rate >= msdc_host_clk_freq[mmc->host_id])
		{
			expFreq = msdc_host_clk_freq[mmc->host_id];
			MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, 0x01);
			MSDC_RETRY_DEF(!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));
			SD_LOG(SD_LOG_ERROR, "request clock is %dKHz, clock has been set to %dKHz", clock_rate/1000, msdc_host_clk_freq[mmc->host_id]/1000);
		}
		else
		{
			// Divider mode
			MSDC_CLRBIT(MSDC_CFG, MSDC_CFG_CKMOD);
			
			for (divider = 0; divider <= 0xFF; divider++) {
				if (0 == divider) {
					expFreq = msdc_host_clk_freq[mmc->host_id] / 2;
				}
				else {
					expFreq = msdc_host_clk_freq[mmc->host_id] / 4 / divider;
				}

				if (expFreq <= clock_rate) {
					MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKDIV, divider);

					// Wait until clock is stable
					MSDC_RETRY_DEF(!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));
				    			
					SD_LOG(SD_LOG_ERROR, "request clock is %dKHz, clock has been set to %dKHz", clock_rate/1000, expFreq/1000);
		        	break;
			    }
	    	}
		}
	}

	// Save current clock setting
	mmc->cur_ddr_mode = mmc->ddr_mode;
	mmc->real_clock = expFreq;
	mmc->cur_clock = mmc->clock;
	
	return ret;
}

static int msdc_set_buswidth(struct mmc *mmc)
{
	u32 base = mmc->base_address;
	
	if (1 == mmc->bus_width) {
		MSDC_SET_FIELD(SDC_CFG, SDC_CFG_BUSWIDTH, 0x00);
	}
	else if (4 == mmc->bus_width) {
		MSDC_SET_FIELD(SDC_CFG, SDC_CFG_BUSWIDTH, 0x01);
	}
	else if (8 == mmc->bus_width){
		MSDC_SET_FIELD(SDC_CFG, SDC_CFG_BUSWIDTH, 0x02);
	}
	else{
		SD_LOG(SD_LOG_ERROR, "error bus width value: %d", mmc->bus_width);
	}
	//SD_LOG(SD_LOG_ERROR, "select bus width: %dbit mode", mmc->bus_width);
	
	return MMC_ERR_NONE;
}


static int msdc_set_ios(struct mmc *mmc)
{
	// set bus width
	msdc_set_buswidth(mmc);

	// Clock not change, return directly
	if ((mmc->cur_clock == mmc->clock) && (mmc->cur_ddr_mode == mmc->ddr_mode))
	{
		return 0;
	}
	
    // Set the clock speed
    msdc_set_clock(mmc);	
	
    return 0;
}


// When trap pin is selected to nand boot, SD0 is working in 4 bit mode for SD card, not emmc.
int msdc_pad_init(struct mmc *mmc, u32 nand_boot)
{
	u32 msdc_clk_drv 	= mmc->host_hw->clk_drv;
	u32 msdc_cmd_drv 	= mmc->host_hw->cmd_drv;
	u32 msdc_dat_drv 	= mmc->host_hw->dat_drv;
	u32 msdc_clk_resistor 	= mmc->host_hw->resistor_clk_line;
	u32 msdc_resistor = mmc->host_hw->resistor_cmddat_line;
	u32 msdc_slew_rate  = mmc->host_hw->slew_rate;	

	if (mmc->host_id == 0)
	{
		// ================= eMMC boot, for SD0 Setting ================
		if (!nand_boot)
		{
			// CLK Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_RESISTOR_MASK, msdc_clk_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_PUPD_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_DRV_MASK, msdc_clk_drv); 
			MSDC_SET_FIELD(PAD_CFG_SD0_CLK, PAD_CFG_SR_MASK, msdc_slew_rate);

			// CMD Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_RESISTOR_MASK, msdc_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_DRV_MASK, msdc_cmd_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_CMD, PAD_CFG_SR_MASK, msdc_slew_rate);

			// DAT0 Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_RESISTOR_MASK, msdc_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT0, PAD_CFG_SR_MASK, msdc_slew_rate);

			// DAT1 Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_RESISTOR_MASK, msdc_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT1, PAD_CFG_SR_MASK, msdc_slew_rate);

			// DAT2 Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_RESISTOR_MASK, msdc_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT2, PAD_CFG_SR_MASK, msdc_slew_rate);

			// DAT3 Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_RESISTOR_MASK, msdc_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT3, PAD_CFG_SR_MASK, msdc_slew_rate);

			// DAT4 Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_RESISTOR_MASK, msdc_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT4, PAD_CFG_SR_MASK, msdc_slew_rate);

			// DAT5 Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_RESISTOR_MASK, msdc_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT5, PAD_CFG_SR_MASK, msdc_slew_rate);

			// DAT6 Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_RESISTOR_MASK, msdc_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT6, PAD_CFG_SR_MASK, msdc_slew_rate);

			// DAT7 Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_RESISTOR_MASK, msdc_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_DAT7, PAD_CFG_SR_MASK, msdc_slew_rate);

			// RST Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_RESISTOR_MASK, msdc_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_PUPD_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_RST, PAD_CFG_SR_MASK, msdc_slew_rate);
		}
		// ================= Nand boot, for SD0 Setting ================
		else 
		{ 
			// CLK Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CLK, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CLK, PAD_CFG_RESISTOR_MASK, msdc_clk_resistor);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CLK, PAD_CFG_PUPD_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CLK, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CLK, PAD_CFG_DRV_MASK, msdc_clk_drv); 
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CLK, PAD_CFG_SR_MASK, msdc_slew_rate);

			// CMD Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CMD, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CMD, PAD_CFG_RESISTOR_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CMD, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CMD, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CMD, PAD_CFG_DRV_MASK, msdc_cmd_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_CMD, PAD_CFG_SR_MASK, msdc_slew_rate);

			// DAT0 Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT0, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT0, PAD_CFG_RESISTOR_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT0, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT0, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT0, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT0, PAD_CFG_SR_MASK, msdc_slew_rate);

			// DAT1 Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT1, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT1, PAD_CFG_RESISTOR_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT1, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT1, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT1, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT1, PAD_CFG_SR_MASK, msdc_slew_rate);

			// DAT2 Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT2, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT2, PAD_CFG_RESISTOR_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT2, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT2, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT2, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT2, PAD_CFG_SR_MASK, msdc_slew_rate);

			// DAT3 Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT3, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT3, PAD_CFG_RESISTOR_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT3, PAD_CFG_PUPD_MASK, 0);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT3, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT3, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_DAT3, PAD_CFG_SR_MASK, msdc_slew_rate);

			// RST Pad
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_RST, PAD_CFG_SMT_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_RST, PAD_CFG_RESISTOR_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_RST, PAD_CFG_PUPD_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_RST, PAD_CFG_IES_MASK, 1);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_RST, PAD_CFG_DRV_MASK, msdc_dat_drv);
			MSDC_SET_FIELD(PAD_CFG_SD0_4BIT_RST, PAD_CFG_SR_MASK, msdc_slew_rate);
		}
	}
	else if (mmc->host_id == 1)
	{
		// CLK Pad
		MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_RESISTOR_MASK, msdc_clk_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_PUPD_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_DRV_MASK, msdc_clk_drv); 
		MSDC_SET_FIELD(PAD_CFG_SD1_CLK, PAD_CFG_SR_MASK, msdc_slew_rate);

		// CMD Pad
		MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_RESISTOR_MASK, msdc_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_DRV_MASK, msdc_cmd_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_CMD, PAD_CFG_SR_MASK, msdc_slew_rate);

		// DAT0 Pad
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_RESISTOR_MASK, msdc_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_DRV_MASK, msdc_dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT0, PAD_CFG_SR_MASK, msdc_slew_rate);

		// DAT1 Pad
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_RESISTOR_MASK, msdc_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_DRV_MASK, msdc_dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT1, PAD_CFG_SR_MASK, msdc_slew_rate);

		// DAT2 Pad
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_RESISTOR_MASK, msdc_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_DRV_MASK, msdc_dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT2, PAD_CFG_SR_MASK, msdc_slew_rate);

		// DAT3 Pad
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_RESISTOR_MASK, msdc_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_DRV_MASK, msdc_dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_DAT3, PAD_CFG_SR_MASK, msdc_slew_rate);

		// RST Pad
		MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_RESISTOR_MASK, msdc_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_PUPD_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_DRV_MASK, msdc_dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD1_RST, PAD_CFG_SR_MASK, msdc_slew_rate);
	}
	else if (mmc->host_id == 2)
	{
		// CLK Pad
		MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_RESISTOR_MASK, msdc_clk_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_PUPD_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_DRV_MASK, msdc_clk_drv); 
		MSDC_SET_FIELD(PAD_CFG_SD2_CLK, PAD_CFG_SR_MASK, msdc_slew_rate);

		// CMD Pad
		MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_RESISTOR_MASK, msdc_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_DRV_MASK, msdc_cmd_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_CMD, PAD_CFG_SR_MASK, msdc_slew_rate);

		// DAT0 Pad
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_RESISTOR_MASK, msdc_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_DRV_MASK, msdc_dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT0, PAD_CFG_SR_MASK, msdc_slew_rate);

		// DAT1 Pad
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_RESISTOR_MASK, msdc_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_DRV_MASK, msdc_dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT1, PAD_CFG_SR_MASK, msdc_slew_rate);

		// DAT2 Pad
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_RESISTOR_MASK, msdc_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_DRV_MASK, msdc_dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT2, PAD_CFG_SR_MASK, msdc_slew_rate);

		// DAT3 Pad
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_RESISTOR_MASK, msdc_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_PUPD_MASK, 0);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_DRV_MASK, msdc_dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_DAT3, PAD_CFG_SR_MASK, msdc_slew_rate);

		// RST Pad
		MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_SMT_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_RESISTOR_MASK, msdc_resistor);
		MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_PUPD_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_IES_MASK, 1);
		MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_DRV_MASK, msdc_dat_drv);
		MSDC_SET_FIELD(PAD_CFG_SD2_RST, PAD_CFG_SR_MASK, msdc_slew_rate);
	}
	else
	{
		// Wrong!!!
	}
	return MMC_ERR_NONE;
}

static int msdc_pad_multi_func(struct mmc *mmc, u32 nand_boot)
{
	if (!mmc)
		return 1;

	if (mmc->host_id == 0)
	{
		// Set RST pin as GPIO, for swtich voltage between 3.3V and 1.8V, default 3.3V. (EVB)
		MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD0_8BIT_RST_GPIO_CTL, 0); // 0 - RST be used as MSDC Func
		//MSDC_SET_FIELD(MSDC_SW_GPIO_ENABLE_OUTPUT, SD_V33_18_SW0_ENABLE, 1);
		//MSDC_SET_FIELD(MSDC_SW_GPIO_OUTPUT_VALUE, SD_V33_18_SW0_VALUE, 1);
		gpio_direction_output(PIN_114_SD_V33_18_SW0, 1);
			
		// ================= eMMC boot, for SD0 Setting ================
		if (!nand_boot)
		{
			// Set SD0 pins work as MSDC mode
			MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD0_8BIT_GPIO_CTL, 0);
		}
		// ================== Nand boot, for SD0 Setting ================
		else
		{
			MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD0_RST_GPIO_CTL, 1);
			MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD0_GPIO_CTL, 0);	// Make SD0 work as SD
			MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD0_8BIT_GPIO_CTL, 0x3FF); // Disable emmc 8bit
		}
	}
	else if (mmc->host_id == 1)
	{
		MSDC_SET_FIELD(MSDC_PAD_RST_RXDLY, SD1_DATA_PINS_AS_SD0_HIGH_4DATA, 0); // Port1's pins was used for port1
		MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD1_GPIO_CTL, 0);

		// Set RST pin as GPIO, for swtich voltage between 3.3V and 1.8V, default 3.3V. (EVB)
		MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD1_RST_GPIO_CTL, 1);
		//MSDC_SET_FIELD(MSDC_SW_GPIO_ENABLE_OUTPUT, SD_V33_18_SW1_ENABLE, 1);
		//MSDC_SET_FIELD(MSDC_SW_GPIO_OUTPUT_VALUE, SD_V33_18_SW1_VALUE, 0);
		gpio_direction_output(PIN_115_SD_V33_18_SW1, 0);
		/*config sd1 detect gpio pin*/
		GPIO_MultiFun_Set(SD1_CD_PIN,PINMUX_LEVEL_GPIO_END_FLAG);
		gpio_direction_input(SD1_CD_PIN);
	}
	else if (mmc->host_id == 2)
	{
		MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD2_GPIO_CTL, 0);
		// Set RST pin as GPIO, for swtich voltage between 3.3V and 1.8V, default 3.3V. (EVB)
		MSDC_SET_FIELD(MSDC_PAD_FUNC_SELECT, MSDC_PAD_FUNC_SD2_RST_GPIO_CTL, 1);
		//MSDC_SET_FIELD(MSDC_SW_GPIO_ENABLE_OUTPUT, SD_V33_18_SW2_ENABLE, 1);
		//MSDC_SET_FIELD(MSDC_SW_GPIO_OUTPUT_VALUE, SD_V33_18_SW2_VALUE, 0);
		gpio_direction_output(PIN_116_SD_V33_18_SW2, 0);
		/*config sd2 detect gpio pin*/
		GPIO_MultiFun_Set(SD2_CD_PIN,PINMUX_LEVEL_GPIO_END_FLAG);
		gpio_direction_input(SD2_CD_PIN);
	}
	else
	{
		// Wrong!!!
	}
	
	return MMC_ERR_NONE;
}


/* initialize the registers of sd */
static int msdc_register_init(struct mmc *mmc)
{
	unsigned int base;
    base =  mmc->base_address;

	#if 1
	/* reset tuning parameter */
	MSDC_WRITE32(MSDC_PAD_TUNE, 0x0000000);
	MSDC_WRITE32(MSDC_DAT_RDDLY0, 0x00000000);
	MSDC_WRITE32(MSDC_DAT_RDDLY1, 0x00000000);
	MSDC_WRITE32(MSDC_IOCON, 0x00000000);
	MSDC_WRITE32(MSDC_PATCH_BIT0, 0x003C0007);  // bit3 should set to 0, or the DDR mode will fail
	MSDC_SET_FIELD(MSDC_PATCH_BIT1, 0x1<<16, 0);  // close single burst type, or 8bytes burst size will fail in DMA mode
	#endif

    MSDC_SETBIT(MSDC_CFG, MSDC_CFG_RST);

    //wait msdc reset complete
    while(0 != ((MSDC_READ32(MSDC_CFG)) & MSDC_CFG_RST))
        udelay(1000);

    // set clock mode
    MSDC_CLRBIT(MSDC_CFG, MSDC_CFG_CKMOD);

    //set SD/MMC Mode
    MSDC_SETBIT(MSDC_CFG, MSDC_CFG_SDMMC);

    // Set Data Timeout Counter
	//MSDC_SET_FIELD(SDC_CFG, SDC_CFG_DTOC, 0xFF);

    //Disable SDIO
    MSDC_CLRBIT(SDC_CFG, SDC_CFG_SDIO);

    // Set default RISC_SIZE for DWRD pio mode
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RISCSZ, MSDC_IOCON_RISCSZ_4BYTE);

	// Set clock free running mode
	//MSDC_SETBIT(MSDC_CFG, MSDC_CFG_CKPDN);

	// Set clock to no-free running mode
	MSDC_CLRBIT(MSDC_CFG, MSDC_CFG_CKPDN);

	// Disable MSDC internal CD 
	MSDC_CARD_DETECTION_OFF();

    // Clear Interrupt status
    MSDC_WRITE32(MSDC_INT, 0x0001F7FB);

	// Set R1B Busy delay
    MSDC_SETBIT(MSDC_PATCH_BIT0, MSDC_PATCH_BIT_BUSYDLY);

	// Set write CRC error and timeout detect
    MSDC_SETBIT(MSDC_PATCH_BIT0, MSDC_PATCH_BIT_DECRCTMO);

	MSDC_WRITE32(MSDC_PATCH_BIT1, 0xFFFE0009); //0xFFFF00C9

	// For eMMC, Tune Parameters
	if (mmc->host_id == 0)
	{
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, 0);
		MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_DLY_SEL, 0);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, 0);
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_WD0_SMPL, 0);
		MSDC_SET_FIELD(MSDC_DAT_RDDLY0, MSDC_DAT_RDDLY0_D0, 8);
	}
	
    SD_LOG(SD_LOG_INIT, "init msdc register, base = 0x%X\r\n", base);
	
    return MMC_ERR_NONE;
}

// for each host controller init 
static int msdc_init(struct mmc *mmc)
{    
    // set default bus width & clock
    mmc->bus_width = 1;
    mmc->clock = MSDC_INIT_CLOCK;
	mmc->ddr_mode = 0;
    
    // Initialize msdc hardware
    return msdc_register_init(mmc);
}

int msdc_select_clock_source_auto(struct mmc *mmc)
{
	u32 nearest_clock_freq = 0;
	u32 nearest_clock_src  = 0;
	u32 i = 0;
	u32 divider = 0;
	u32 temp_freq = 0;
	u32 expected_freq = mmc->host_hw->max_clock * 1000 * 1000;
	
	// Double check clock source
	if (mmc->host_hw->clk_src != MSDC_CLKSRC_AUTO)
		return MMC_ERR_NONE;

	// Compare max_clock with fastest clock source
	if (expected_freq >= clock_freq[0])
	{
		mmc->host_hw->clk_src = MSDC_CLKSRC_200MHZ;
		mmc->host_hw->max_clock = 200;	// 200M
		msdc_select_clock_source(mmc);
	}
	else
	{
		// Loop for finding out the nearest clock and clock source
		for (i = 0; i < MAX_CLOCK_SRC_TYPE; i++)
		{
			if (expected_freq >= clock_freq[i])
			{
				temp_freq = clock_freq[i];
			}
			else
			{
				// Divider mode			
				for (divider = 0; divider <= 0xFF; divider++) 
				{
					if (0 == divider) 
					{
						temp_freq = clock_freq[i] / 2;
					}
					else 
					{
						temp_freq = clock_freq[i] / 4 / divider;
					}

					if (temp_freq <= expected_freq) 
					{
			        	break;
				    }
		    	}
			}

			// Compare with previous nearest clock, update clock freq and clock source
			if (nearest_clock_freq == 0)
			{
				nearest_clock_freq = temp_freq;
				nearest_clock_src = i;
			}
			else
			{
				if ((expected_freq - nearest_clock_freq) > (expected_freq - temp_freq))
				{
					nearest_clock_freq = temp_freq;
					nearest_clock_src = i;
				}
			}
		}

		// Select clock
		mmc->host_hw->clk_src = nearest_clock_src;
		msdc_select_clock_source(mmc);
	}
	return MMC_ERR_NONE;
}

int msdc_select_clock_source(struct mmc *mmc)
{	
	switch (mmc->host_hw->clk_src)
	{
	case MSDC_CLKSRC_200MHZ:
		msdc_host_clk_freq[mmc->host_id] = 200 * 1000 * 1000;
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_ARMPLL2_D2);	// 202MHz
		break;

	case MSDC_CLKSRC_196MHZ:
		msdc_host_clk_freq[mmc->host_id] = 196 * 1000 * 1000;
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_MSDCPLL_D2);	// 196MHz
		break;

	case MSDC_CLKSRC_189MHZ:
		msdc_host_clk_freq[mmc->host_id] = 189 * 1000 * 1000;
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_DMPLL_D2);		// 189MHz
		break;
		
	case MSDC_CLKSRC_162MHZ:
		msdc_host_clk_freq[mmc->host_id] = 162 * 1000 * 1000;
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_SYSPLL_D4);
		break;

	case MSDC_CLKSRC_147MHZ:
		msdc_host_clk_freq[mmc->host_id] = 147 * 1000 * 1000;
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_APLL2_D2);
		break;
		
	case MSDC_CLKSRC_135MHZ:
		msdc_host_clk_freq[mmc->host_id] = 135 * 1000 * 1000;
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_MSDCPLL_D3);
		break;
		
	case MSDC_CLKSRC_108MHZ:
		msdc_host_clk_freq[mmc->host_id] = 108 * 1000 * 1000;
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_SYSPLL_D6);
		break;

	case MSDC_CLKSRC_27MHZ:
		msdc_host_clk_freq[mmc->host_id] = 27 * 1000 * 1000;
		MSDC_SELECT_CLK_SRC(MSDC_CLK_SEL_27MHZ);
		break;

	case MSDC_CLKSRC_AUTO:
		msdc_select_clock_source_auto(mmc);
		break;
		
	default:
		SD_LOG(SD_LOG_ERROR, "error source clock value");
	}

	#ifdef CONFIG_MSDC_ETT
	SD_LOG(SD_LOG_ERROR, "Clock_Source = %d, Max_Clock = %dMHz", mmc->host_hw->clk_src, mmc->host_hw->max_clock);
	#endif
	
	return MMC_ERR_NONE;
}


int msdc_change_clock_source(struct mmc *mmc, u32 clock)
{
	u32 org_clock = msdc_host_clk_freq[mmc->host_id];
	
	mmc->host_hw->clk_src = clock; // Clock source update
	msdc_select_clock_source(mmc);
	
	printf("=====> msdc%d clock source change from %dKHz to %dKHz <=====\n", mmc->host_id, org_clock/1000, msdc_host_clk_freq[mmc->host_id]/1000); 

	// Update max and min clock threshold
	mmc->f_min = msdc_host_clk_freq[mmc->host_id] /(4*255);
    mmc->f_max = msdc_host_clk_freq[mmc->host_id]; 
	return MMC_ERR_NONE;
}

int msdc_show_clock_source(struct mmc *mmc)
{
	printf("=====> msdc%d clock source %dKHz <=====\n", mmc->host_id, msdc_host_clk_freq[mmc->host_id]/1000); 
	return MMC_ERR_NONE;
}

// allocate host controller and register mmc host to mmc 'core'
static int msdc_initialize(bd_t *bis)
{
    struct mmc *mmc;
	unsigned base = 0;
    
   if(msdc_mmc_inited)
	mmc = find_mmc_device(msdc_mmc_reinit_id);
   else
	mmc = malloc(sizeof(struct mmc));
    if (NULL == mmc){
        SD_LOG(SD_LOG_BUFFER | SD_LOG_ERROR, "malloc mmc struct error");
        return E_ALLOC_BUFFER_ERROR;
	}

   if(msdc_mmc_inited)
	mmc_unregister(mmc);
         
    memset(mmc, 0, sizeof(struct mmc));

    sprintf(mmc->name, DRIVER_NAME);
    
    mmc->send_cmd 	= msdc_send_cmd;
    mmc->set_ios 	= msdc_set_ios;
    mmc->init 		= msdc_init;

	mmc->card_type	= MMC_TYPE_UNKNOWN;

    mmc->voltages |= (MMC_VDD_26_27 | MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30| MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36);

    mmc_register(mmc);
    
    SD_LOG(SD_LOG_INIT, "SD card is %d", mmc->block_dev.dev);

	mmc->host_id = mmc->block_dev.dev;

	// get device attributes
	mmc->host_hw = atc_msdc_dev[mmc->host_id];
    
    // get msdc host base address after mmc register
    mmc->base_address = msdc_base_address[mmc->block_dev.dev];
	base = mmc->base_address;

	// set work default clock and  clock mode
	#ifdef CONFIG_MSDC_ETT
		mmc->switch_hs200 = 0;
		mmc->work_clock_freq = DEF_WORK_CLOCK;
		mmc->work_clock_mode = 0; // SDR
	#else
		if (mmc->host_id == 0) { // Slot 0 for eMMC 
			mmc->switch_hs200 = 1;
			#if MSDC_ENABLE_HS200
				mmc->work_clock_freq = EMMC_WORK_CLOCK;
				mmc->work_clock_mode = 0; // SDR
			#else
				#if MSDC_EMMC_ENABLE_DDR52
					mmc->work_clock_freq = EMMC_DDR_WORK_CLOCK;
					mmc->work_clock_mode = 1; // DDR
				#else
					mmc->work_clock_freq = DEF_WORK_CLOCK;
					mmc->work_clock_mode = 0; // SDR
				#endif
			#endif
		}
		else {	// Slot 1 & Slot 2 for SD Card
			mmc->switch_hs200 = 0;
			mmc->work_clock_freq = SD_WORK_CLOCK;
			mmc->work_clock_mode = 0; // SDR
		}
	#endif

	// select clock source and set clock freq threshold
	msdc_select_clock_source(mmc);
    mmc->f_max = mmc->host_hw->max_clock * 1000 * 1000;
	mmc->f_min = mmc->f_max / (4*255);

	// Select HClock and enable clock output
	MSDC_SELECT_HCLK_SRC(MSDC_HCLK_SEL_SYSPLL_D9);
	//MSDC_SELECT_HCLK_SRC(MSDC_HCLK_SEL_APLL2_D3);
	MSDC_CLOCK_GATE(mmc, GATE_ENABLE_CLOCK);
	
	// Pad init
	//if (mmc->block_dev.dev == 0)
	{
		msdc_pad_init(mmc, 0); 
		msdc_pad_multi_func(mmc, 0);
	}
	
    SD_LOG(SD_LOG_INIT, "SD %d initialize, base is 0x%x", mmc->block_dev.dev, msdc_base_address[mmc->block_dev.dev]);

    return MMC_ERR_NONE;
}

// export the function for mmc 'core', start init all host controller
int msdc_mmc_init(bd_t *bis)
{
    unsigned int uloop;
    unsigned int sd_num = ATC_MSDC_HOST_NUM;
    int ret = 0;

    /* we should initialize every sd*/
    for (uloop = 0; uloop < sd_num; uloop++)
    {
        ret = msdc_initialize(bis);
    }
	
	msdc_mmc_inited = 1;
	return ret;
}

#if (MMC_CMD_TUNE)
#define CMD_TUNE_SIZE 128

struct msdc_delay_phase {
	u8 maxlen;
	u8 start;
	u8 final_phase;
};
#define PAD_DELAY_MAX (32)
static u32 test_delay_bit(u32 delay, u32 bit)
{
	bit %= PAD_DELAY_MAX;
	return delay & (1 << bit);
}
static int get_delay_len(u32 delay, u32 start_bit)
{
	int i;

	for (i = 0; i < (PAD_DELAY_MAX - start_bit); i++) {
		if (test_delay_bit(delay, start_bit + i) == 0)
			return i;
	}
	return PAD_DELAY_MAX - start_bit;
}

static struct msdc_delay_phase get_best_delay(struct mmc *mmc, u32 delay)
{
	int start = 0, len = 0;
	int start_final = 0, len_final = 0;
	u8 final_phase = 0xff;
	struct msdc_delay_phase delay_phase = { 0, };

	if (delay == 0) {
		printf("msdc%d phase error: [map:%x]\n", mmc->host_id, delay);
		delay_phase.final_phase = final_phase;
		return delay_phase;
	}

	while (start < PAD_DELAY_MAX) {
		len = get_delay_len(delay, start);
		if (len_final < len) {
			start_final = start;
			len_final = len;
		}
		start += len ? len : 1;
		if (len >= 12 && start_final < 4)
			break;
	}

	/* The rule is that to find the smallest delay cell */
	if (start_final == 0)
		final_phase = (start_final + len_final / 3) % PAD_DELAY_MAX;
	else
		final_phase = (start_final + len_final / 2) % PAD_DELAY_MAX;
		printf("msdc%d: phase: [map:%x] [maxlen:%d] [final:%d]\n", mmc->host_id,
				delay, len_final, final_phase);

	delay_phase.maxlen = len_final;
	delay_phase.start = start_final;
	delay_phase.final_phase = final_phase;
	return delay_phase;
}
static int  msdc_abort_data(struct mmc *mmc)
{
	u32 card_status = 0;
    u32 state = 0;
	int err;
	
	//Check Card Status 					
	do{
		err = mmc_send_status(mmc, &card_status);
         
        if (err != MMC_ERR_NONE) {
            printf("[Err handle][%s:%d]cmd13 fail\n", __func__, __LINE__);
            goto out;
        }
        
        state = R1_CURRENT_STATE(card_status);

        printf("check card state<%d>\n", state);
        if (state == 5 || state == 6) {
                printf("state<%d> need cmd12 to stop\n", state);

                err=mmc_send_stopcmd(mmc);
 
                if (err != MMC_ERR_NONE) {
                    printf("[Err handle][%s:%d]cmd12 fail\n", __func__, __LINE__);
                    goto out;
                }   
        } else if (state == 7) {  // busy in programing
            printf("state<%d> card is busy\n", state);
            mdelay(100);
        } else if (state != 4) {
            printf("state<%d> ??? \n", state);
            goto out;
        }
		udelay(100);
	}while (state != 4);
	return 0;
out:
    printf("[SD%d] data abort failed\n",mmc->host_id);
    return 1;

}

UINT msdc_rd_fifo(struct mmc *mmc, struct mmc_data *data)
{
    UINT datsta;
    UINT status = MMC_ERR_NONE;
    UINT base;
    UINT dwLength = data->blocks * data->blocksize;
    void *dst = data->dest;
	UINT retry = 50;
	
    base = mmc->base_address;
	
	MSDC_DMA_OFF();

	// set to 4bytes access mode
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RISCSZ, MSDC_IOCON_RISCSZ_4BYTE);
	
	MSDC_WRITE32(MSDC_INTEN, MSDC_INT_XFER_COMPL | MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_CSTA );
    
	while(dwLength > 0)
	{
		datsta = MSDC_READ32(MSDC_INT);
        
		if (datsta & MSDC_INT_DATTMO) {
			SD_LOG(SD_LOG_INFO | SD_LOG_ERROR, "Data Timeout");
			status = MMC_ERR_TIMEOUT;
			break;
		}

		if (datsta & MSDC_INT_DATCRCERR) {
		    SD_LOG(SD_LOG_INFO |SD_LOG_ERROR, "Data CRC Error");
			status = MMC_ERR_BADCRC;
			break;
		}

		while((MSDC_RXFIFOCNT() >= 4) && (dwLength > 0)){ 
			*((unsigned int *)dst) = MSDC_READ32(MSDC_RXDATA);
			dst = (char*)dst + 4;
			dwLength -= 4;
		}
	}

	//while(!(MSDC_READ32(MSDC_INT) & MSDC_INT_XFER_COMPL) && dwLength){
	while(!(MSDC_READ32(MSDC_INT) & MSDC_INT_XFER_COMPL)){
        SD_LOG(SD_LOG_INFO, "data transfer is not end.");
		udelay(1000);
		retry--;
		if (retry == 0)
		{
			SD_LOG(SD_LOG_ERROR, "SW timeout left data size = %d.", dwLength);
			status = MMC_ERR_TIMEOUT;
			break;
		}
    }
	
	MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));

	return status;	
}

static int msdc_send_tuning(struct mmc *mmc, u32 opcode, int *cmd_error)
{	
    INT32 ret;
	UINT32 tmp_cmd = 0;
	UINT32 card_status = 0;
    UINT32 base = mmc->base_address;
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;
	char buf[CMD_TUNE_SIZE];

	/* Get the Card Status Register */
	cmd.opcode 	= opcode;
	cmd.rsptype = MMC_RSP_R1;
	cmd.arg 	= 0;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

	data.dest = buf;
	data.blocks = 1;
	data.blocksize = CMD_TUNE_SIZE;
	data.flags = MMC_DATA_READ;
	
	msdc_set_timeout(mmc, DAT_TIMEOUT_100MS, 0);
    
	ret = wait_card_not_busy(mmc);
	*cmd_error=ret;
	if (ret != MMC_ERR_NONE)
		return ret;

	// Clear Interrupts before send command, make sure that command env is clear.
	MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));
	
    // config the command and send it
    ret = msdc_send_command(mmc, &cmd, &data);
	*cmd_error=ret;
	if (ret)
		return ret;
    
     // Wait for the request to complete.
    ret = msdc_wait_rsp(mmc, &cmd);
	if((ret==MMC_ERR_TIMEOUT)&&(msdc_get_cd(mmc->host_id))){
		printf("no sd%d card 12 exist \n",mmc->host_id);
		return -MMC_ERR_ENOMEDIUM;
	}
	*cmd_error=ret;
	if (ret){
		msdc_abort(mmc);
		return ret;
	}
    //read/write data if is a read/write cmd 
	//ret = msdc_send_data(mmc, &cmd, &data);
	msdc_set_timeout(mmc, DAT_TIMEOUT_500MS, 0);
	ret = msdc_rd_fifo(mmc, &data);
    if (ret){
		msdc_abort_data(mmc);
		return ret;
    }
    return MMC_ERR_NONE;
}

static int msdc_tune_response(struct mmc *mmc, u32 opcode)
{
	u32 rise_delay = 0, fall_delay = 0;
	struct msdc_delay_phase final_rise_delay, final_fall_delay = { 0,};
	struct msdc_delay_phase internal_delay_phase;
	u8 final_delay, final_maxlen;
	u32 internal_delay = 0;
	int cmd_err;
	int i;
	u32 base = mmc->base_address;

#if 0
	if (mmc->ios.timing == MMC_TIMING_MMC_HS200 ||
	    mmc->ios.timing == MMC_TIMING_UHS_SDR104)
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, host->hs200_cmd_int_delay);
#else
	MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, 0);
#endif
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 0);
	for(i=0; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, i);
		msdc_send_tuning(mmc, opcode, &cmd_err);
		if(!cmd_err)
			rise_delay |= (1 << i);
	}
	final_rise_delay = get_best_delay(mmc, rise_delay);
	if (final_rise_delay.maxlen >= 12 || (final_rise_delay.start == 0 && final_rise_delay.maxlen >= 4))
		goto skip_fall;
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 1);
	for (i = 0; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, i);
		msdc_send_tuning(mmc, opcode, &cmd_err);
		if (!cmd_err)
			fall_delay |= (1 << i);
	}
	final_fall_delay = get_best_delay(mmc, fall_delay);

skip_fall:
	final_maxlen = max(final_rise_delay.maxlen, final_fall_delay.maxlen);
	if (final_maxlen == final_rise_delay.maxlen) {
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 0);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, final_rise_delay.final_phase);
		final_delay = final_rise_delay.final_phase;
	} else {
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, 1);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRDLY, final_fall_delay.final_phase);
		final_delay = final_fall_delay.final_phase;
	}

	for (i = 0; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, i);
		msdc_send_tuning(mmc, opcode, &cmd_err);
		if (!cmd_err)
			internal_delay |= (1 << i);
	}
	printf("[%d]: Final internal delay: 0x%x\n", mmc->host_id, internal_delay);
	internal_delay_phase = get_best_delay(mmc, internal_delay);
	MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, internal_delay_phase.final_phase);
done:
	printf("[%d]: Final cmd pad delay: %x\n", mmc->host_id, final_delay);
	return final_delay == 0xff ? -5 : 0;
}

static int msdc_tune_data(struct mmc *mmc, u32 opcode)
{

	u32 rise_delay = 0, fall_delay = 0;
	struct msdc_delay_phase final_rise_delay, final_fall_delay = { 0, };
	u8 final_delay, final_maxlen;
	int i, ret,cmd_err;
	u32 base = mmc->base_address;
	//set all data line use same delay cycle 
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 0);
	//set all data line use same sample edge	
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RDSPLSEL, 0);

	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, 0);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, 0);
	for (i = 0 ; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, i);
		ret = msdc_send_tuning(mmc, opcode, &cmd_err);
		if (!ret)
			rise_delay |= (1 << i);
	}
	final_rise_delay = get_best_delay(mmc, rise_delay);
	if (final_rise_delay.maxlen >= 12 ||
	    (final_rise_delay.start == 0 && final_rise_delay.maxlen >= 4))
		goto skip_fall;

	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, 1);
	MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, 1);
	for (i = 0; i < PAD_DELAY_MAX; i++) {
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, i);
		ret = msdc_send_tuning(mmc, opcode, &cmd_err);
		if (!ret)
			fall_delay |= (1 << i);
	}
	final_fall_delay = get_best_delay(mmc, fall_delay);

skip_fall:
	final_maxlen = max(final_rise_delay.maxlen, final_fall_delay.maxlen);
	if (final_maxlen == final_rise_delay.maxlen) {
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, 0);
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, 0);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY,
			      final_rise_delay.final_phase);
		final_delay = final_rise_delay.final_phase;
	} else {
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, 1);
		MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_W_DSPL, 1);
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY,
			      final_fall_delay.final_phase);
		final_delay = final_fall_delay.final_phase;
	}
#if 0
	if (mmc->ios.timing == MMC_TIMING_MMC_HS200 ||
	    mmc->ios.timing == MMC_TIMING_UHS_SDR104)
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY,
			      host->hs200_write_int_delay);
#else
		MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY,0);
#endif
	printf("[%d] Final data pad delay: %x\n", mmc->host_id, final_delay);
	return final_delay == 0xff ? -5 : 0;
}
static unsigned int boot_time_ms(void)
{
	volatile unsigned int time = 0;
	
	/***
	* Register F000814C, which was triggered by BootROM, 
	* start with 0xFFFFFFFF, end with 0x00000000,
	* decrease with every 27M crystal oscillation.
	*/
	time = (0xFFFFFFFF - (*((volatile uint32_t*)(0xF000814C)))) / 27000;
	return time;
}

int msdc_execute_tuning(struct mmc *mmc, u32 opcode)
{
	int ret;
	unsigned long long time1, time2;
	u32 base = mmc->base_address;

	time1 = boot_time_ms();
	printf("start msdc CMD%d tuning \n",opcode);
	ret = msdc_tune_response(mmc, opcode);
	if (ret == -5) {
		printf("[%d] tune response fail!\n", mmc->host_id);
		goto out;
	}

	ret = msdc_tune_data(mmc, opcode);
	if (ret == -5)
		printf("[%d] tune data fail!\n", mmc->host_id);

	 /* reset controller */
    MSDC_RESET();

    /* clear fifo */
    MSDC_CLR_FIFO();
	
	time2 = boot_time_ms();
	printf("[%d] tune takes %lld ms\n", mmc->host_id, (time2-time1));

out:
	return ret;
}

#endif

