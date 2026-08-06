#ifndef _MSDC_H_
#define _MSDC_H_

#include "targetConfig.h"
#include "x_printf.h"
#include "x_util.h"
#include "Timer.h"

#define MSG_LVL_INFO		1
#define MSG_LVL_DEBUG       2



/* SDC_CMD */
/* Standard SD 2.0 Commands                        Type    Arguments            Response    */
/* Class 0 */
#define CMD0_GO_IDLE_STATE                0    /*    bc                                        */
#define CMD1_MMC_SEND_OP_COND                1    /*    bcr        [23:0] OCR            R3            */
#define CMD2_ALL_SEND_CID                2    /*    bcr                            R2            */
#define CMD3_SEND_RELATIVE_ADDR            3    /*    bcr                            R6            */
#define CMD4_SET_DSR                    4    /*    bc        [31:16] DSR                        */
#define CMD6_MMC_SET_BUS_WIDTH                6    /*    ac        [1:0] Bus width        R1            */
#define CMD7_SELECT_CARD                7    /*    ac        [31:16] RCA            R1b            */
#define CMD8_SEND_IF_COND                8    /*    bcr        [11:8] VHS            R7            */

#define CMD8_SEND_EXT_CSD               8
#define CMD9_SEND_CSD                    9    /*    ac        [31:16] RCA            R2            */
#define CMD10_SEND_CID                    10    /*    ac        [31:16] RCA            R2            */
#define CMD12_STOP_TRANSMISSION            12    /*    ac                            R1b            */
#define CMD13_SEND_STATUS                13    /*    ac        [31:16] RCA            R1b            */
#define CMD15_GO_INACTIVE_STATE            15    /*    ac        [31:16] RCA                        */

/* Class 2 */
#define CMD16_SET_BLOCKLEN                16    /*    ac        [31:0] blk len        R1            */
#define CMD17_READ_SINGLE_BLOCK            17    /*    adtc    [31:0] data addr.    R1            */
#define CMD18_READ_MULTIPLE_BLOCK        18    /*    adtc    [31:0] data addr.    R1            */

/* Class 4 */
#define    CMD24_WRITE_BLOCK                24    /*    adtc    [31:0] data addr.    R1            */
#define CMD25_WRITE_MULTIPLE_BLOCK        25    /*    adtc    [31:0] data addr.    R1            */
#define CMD27_PROGRAM_CSD                27    /*    adtc                        R1            */

/* Class 6 */
#define CMD28_SET_WRITE_PROT            28    /*    ac        [31:0] data addr.    R1b            */
#define CMD29_CLR_WRITE_PROT            29    /*    ac        [31:0] data addr.    R1b            */
#define CMD30_SEND_WRITE_PROT            30    /*    adtc    [31:0] prot addr.    R1            */

/* Class 5 */
#define CMD32_ERASE_WR_BLK_START        32    /*    ac        [31:0] data addr.    R1            */
#define CMD33_ERASE_WR_BLK_END            33    /*    ac        [31:0] data addr.    R1            */
#define CMD38_ERASE                        38    /*    ac                            R1b            */

/* Class 7 */
#define CMD42_LOCK_UNLOCK                42    /*    ac        [31:0] Reserved        R1            */

/* Class 8 */
#define CMD55_APP_CMD                    55    /*    ac        [31:16] RCA            R1            */
#define CMD56_GEN_CMD                    56    /*    adtc    [0] RD/WR            R1            */

/* Application Specific Cmds */
#define ACMD6_SET_BUS_WIDTH                6    /*    ac        [1:0] Bus width        R1            */
#define ACMD13_SD_STATUS                13    /*    adtc                        R1            */
#define ACMD22_SEND_NUM_WR_BLOCKS        22    /*    adtc                        R1            */
#define ACMD23_SET_WR_BLK_ERASE_COUNT    23    /*    ac        [22:0] Blk num.        R1            */
#define ACMD41_SD_SEND_OP_COND            41    /*    bcr        [23:0] OCR            R3            */
#define ACMD42_SET_CLR_CARD_DETECT        42    /*    ac        [0] Set cd            R1            */
#define ACMD51_SEND_SCR                   51    /*    adtc                        R1            */


/* IO Card Commands */
#define CMD5_IO_SEND_OP_COND            5    /*    ac        [24:0] OCR            R4            */
#define CMD52_IO_RW_DIRECT                52    /*    ac                            R5            */
#define CMD53_IO_RW_EXTENDED            53    /*    ac                            R5            */


// Card Spec.
#define CMD_RSPTYPE_NO		0
#define CMD_RSPTYPE_R1		1
#define CMD_RSPTYPE_R2		2
#define CMD_RSPTYPE_R3		3
#define CMD_RSPTYPE_R4		4
#define CMD_RSPTYPE_R5		5
#define CMD_RSPTYPE_R6		6
#define CMD_RSPTYPE_R7		7
#define CMD_RSPTYPE_R1B		8



#define COM_ACMD6_SET_BUS_WIDTH              (ACMD6_SET_BUS_WIDTH <<16)|(CMD_RSPTYPE_R1)
#define COM_CMD0_GO_IDLE_STATE              (CMD0_GO_IDLE_STATE  << 16)|(CMD_RSPTYPE_NO)
#define MMC_CMD1_SEND_OP_COND               (CMD1_MMC_SEND_OP_COND << 16)|(CMD_RSPTYPE_R3)
#define COM_CMD2_ALL_SEND_CID               (CMD2_ALL_SEND_CID << 16)|(CMD_RSPTYPE_R2)
#define SD_CMD3_SEND_RELATIVE_ADDR          (CMD3_SEND_RELATIVE_ADDR  << 16)|(CMD_RSPTYPE_R6)
#define MMC_CMD3_SET_RELATIVE_ADDR          (CMD3_SEND_RELATIVE_ADDR << 16)|(CMD_RSPTYPE_R1)
#define SD_CMD8_SEND_IF_COND                (CMD8_SEND_IF_COND  << 16)|(CMD_RSPTYPE_R7)
#define SD_ACMD41_SD_SEND_OP_COND           (ACMD41_SD_SEND_OP_COND  << 16)|(CMD_RSPTYPE_R3)
#define COM_CMD55_APP_CMD                   (CMD55_APP_CMD  << 16)|(CMD_RSPTYPE_R1)
#define COM_CMD9_SEND_CSD                   (CMD9_SEND_CSD<< 16)|(CMD_RSPTYPE_R2)
#define COM_CMD7_SELECT_CARD                (CMD7_SELECT_CARD << 16)|(CMD_RSPTYPE_R1)
#define COM_CMD18_READ_MULTIPLE_BLOCK       (CMD18_READ_MULTIPLE_BLOCK << 16)|(CMD_RSPTYPE_R1)
#define COM_CMD25_WRITE_MULTIPLE_BLOCK       (CMD25_WRITE_MULTIPLE_BLOCK << 16)|(CMD_RSPTYPE_R1)
#define COM_CMD12_STOP_TRANSMISSION         (CMD12_STOP_TRANSMISSION << 16)|(CMD_RSPTYPE_R1)
#define COM_CMD16_SET_BLOCKLEN              (CMD16_SET_BLOCKLEN<< 16)|(CMD_RSPTYPE_R1)
#define MMC_CMD8_SEND_EXT_CSD               (CMD8_SEND_EXT_CSD<<16)|(CMD_RSPTYPE_R1)
#define COM_CMD6_SWITCH						(CMD6_MMC_SET_BUS_WIDTH<<16)|(CMD_RSPTYPE_R1B)

#define CMD6_ARGS(access,index,value)		(((access&0x3)<<24)|((index&0xff)<<16)|((value&0xff)<<8))


#define CMD8_ARG_VHS_2V7_3V6			(0x1 << 8)
#define CMD8_ARG_CK_PAT					0xAA

#define GET_CMD_TYPE(cmd)               ((cmd >> 16)&0xFFFF)
#define GET_RESPONSE_TYPE(cmd)          (cmd & 0xFFFF)

#define ACMD41_ARG_HCS				(1 << 30)
#define ACMD41_ARG_OCS_2V7_2V8			(1 << 15)
#define ACMD41_ARG_OCS_2V8_2V9			(1 << 16)
#define ACMD41_ARG_OCS_2V9_3V0			(1 << 17)
#define ACMD41_ARG_OCS_3V0_3V1			(1 << 18)
#define ACMD41_ARG_OCS_3V1_3V2			(1 << 19)
#define ACMD41_ARG_OCS_3V2_3V3			(1 << 20)
#define ACMD41_ARG_OCS_3V3_3V4			(1 << 21)
#define ACMD41_ARG_OCS_3V4_3V5			(1 << 22)
#define ACMD41_ARG_OCS_3V5_3V6			(1 << 23)
#define ACMD41_ARG_OCS_2V7_3V6			(0x1FF << 15)
#define ACMD41_ARG_OCS_MASK			(0x1FF << 15)


#define MMC_OCR_1V7_1V95                (1 << 7)
#define MMC_OCR_2V7_3V6                 (0x1FF << 15)
#define MMC_OCR_BYTE_MODE               (0 << 29)
#define MMC_OCR_SECTOR_MODE             (2 << 29)
#define MMC_OCR_NOTBUSY                 (1UL << 31)


#define ACMD41_R3_NOTBUSY		(1UL << 31)
#define ACMD41_R3_SDHC			(1 << 30)
#define ACMD41_R3_OCR_MASK		(0xFFFFFF)
#define ACMD41_R3_S18A			(1 << 24)

// EXT_CSD
#define EXT_CSD_ACCESS_MODE_COMMANDSET		(0)
#define EXT_CSD_ACCESS_MODE_SETBITS		(1)
#define EXT_CSD_ACCESS_MODE_CLEARBITS		(2)
#define EXT_CSD_ACCESS_MODE_WRITEBYTE		(3)

#define EXT_CSD_179_BOOTPARTITION_MASK		(7)
#define EXT_CSD_179_USER_PARTITION		(0)
#define EXT_CSD_179_BOOTPARTITION_BOOT1		(1)
#define EXT_CSD_179_BOOTPARTITION_BOOT2		(2)

#define EXT_CSD_183_BUS_WIDTH_MASK		(3)
#define EXT_CSD_183_BUS_WIDTH_1			(0)	/* Card is in 1 bit mode */
#define EXT_CSD_183_BUS_WIDTH_4			(1)	/* Card is in 4 bit mode */
#define EXT_CSD_183_BUS_WIDTH_8			(2)	/* Card is in 8 bit mode */




//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

#define MSDC_CARDTYPE_UNKNOW		0
#define MSDC_CARDTYPE_SD20LATER		1
#define MSDC_CARDTYPE_SD20LATER_SDSC	2
#define MSDC_CARDTYPE_SD20LATER_SDHC	3
#define MSDC_CARDTYPE_SD1X_SDSC		4
#define MSDC_CARDTYPE_EMMC		5
#define MSDC_CARDTYPE_SDIO		6
#define MSDC_CARDTYPE_UNSTABLE		7
#define MSDC_CARDTYPE_MMC_HV		8
#define MSDC_CARDTYPE_MMC_DV_EMMC	9

//#define ENABLE_DUMP_CARD_TYPE
#ifdef ENABLE_DUMP_CARD_TYPE
UINT32 Dump_Card_Type(UINT32 ch, UINT32 pos);
#else
#define Dump_Card_Type(a, b)
#endif

UINT32 MSDC_Is_eMMC_Card(UINT32 ch);

#define MSDC_LASTERROR_IDENTIFY		(0x01 << 16)
#define MSDC_LASTERROR_CARDTYPE		(0x02 << 16)
#define MSDC_LASTERROR_STATECHANGE	(0x03 << 16)
#define MSDC_LASTERROR_READBLOCK	(0x04 << 16)
#define MSDC_LASTERROR_SETBLKLEN	(0x05 << 16)
#define MSDC_LASTERROR_READEXTCSD	(0x06 << 16)
#define MSDC_LASTERROR_WRITEBLOCK	(0x07 << 16)
#define MSDC_LASTERROR_WRITEEXTCSD	(0x08 << 16)
#define MSDC_LASTERROR_EMMCBOOTMODE	(0x09 << 16)
#define MSDC_LASTERROR_EMMCBOOT2	(0x0A << 16)
#define MSDC_LASTERROR_EMMCREAD		(0x0B << 16)
#define MSDC_LASTERROR_SETEXTCSD	(0x0C << 16)



#define MSDC_LASTERROR_OK           (0x0)
#define MSDC_LASTERROR_ERRORS       (0x1000)
#define MSDC_LASTERROR_ERRTYPE_MASK	(0xFFFFUL << 16)

#define MSDC_CARD_STATE_IDLE        0
#define MSDC_CARD_STATE_READY       1
#define MSDC_CARD_STATE_IDENT       2
#define MSDC_CARD_STATE_STBY        3
#define MSDC_CARD_STATE_TRAN        4
#define MSDC_CARD_STATE_RCV         5
#define MSDC_CARD_STATE_PRG         6
#define MSDC_CARD_STATE_DIS         7
#define MSDC_CARD_STATE_BTST        8
#define MSDC_CARD_STATE_SLP         9

#define IO_BASE_ADDR            (unsigned int) (0xF0000000)

// Pad Driver Strength
#define WRITEMEM(Address, Value) (*(volatile unsigned int *)(Address)) = Value
#define READMEM(Address)         (*(volatile unsigned int *)(Address))
#define MASKMEM(Address,Value,Mask)		WRITEMEM(Address,((READMEM(Address)&(~(Mask)))|Value))


#define BIT_PIN_DRV(v)		(v << 4)
#define BIT_PIN_DRV_MASK	(0x3F << 4)

#define PAD_DRV_MIN		BIT_PIN_DRV(5)
#define PAD_DRV_MAX		BIT_PIN_DRV(7)


#define PAD_CLK			(1 << 0)
#define PAD_CMD			(1 << 1)
#define PAD_DATA0		(1 << 2)
#define PAD_ALL			(PAD_CLK|PAD_CMD|PAD_DATA0)

#define REG_PAD_MSDC_CFG0        (IO_BASE_ADDR + 0x2C0)      //msdc0_clk
#define REG_PAD_MSDC_CFG1        (IO_BASE_ADDR + 0x2C4) 		//msdc0_cmd
#define REG_PAD_MSDC_CFG2        (IO_BASE_ADDR + 0x2C8)		//msdc0_dat0

#define REG_PAD_MSDC_CFG6        (IO_BASE_ADDR + 0x2D8)		//msdc1_clk
#define REG_PAD_MSDC_CFG7        (IO_BASE_ADDR + 0x2DC)		//msdc1_cmd
#define REG_PAD_MSDC_CFG8        (IO_BASE_ADDR + 0x2E0)		//msdc1_dat0

#define REG_PAD_MSDC_CFG12        (IO_BASE_ADDR + 0x2F0)		//msdc2_clk
#define REG_PAD_MSDC_CFG13        (IO_BASE_ADDR + 0x2F4)		//msdc2_cmd
#define REG_PAD_MSDC_CFG14        (IO_BASE_ADDR + 0x2F8)		//msdc2_dat0

#define REG_PAD_MSDC_CFG19        (IO_BASE_ADDR + 0x30C)		//msdc0_8b_clk
#define REG_PAD_MSDC_CFG20        (IO_BASE_ADDR + 0x310)		//msdc0_8b_cmd
#define REG_PAD_MSDC_CFG21        (IO_BASE_ADDR + 0x314)		//msdc0_8b_dat0
#define REG_PAD_MSDC_CFG22        (IO_BASE_ADDR + 0x318)		//msdc0_8b_dat1
#define REG_PAD_MSDC_CFG23        (IO_BASE_ADDR + 0x31C)		//msdc0_8b_dat2
#define REG_PAD_MSDC_CFG24        (IO_BASE_ADDR + 0x320)		//msdc0_8b_dat3
#define REG_PAD_MSDC_CFG25        (IO_BASE_ADDR + 0x324)		//msdc0_8b_dat4
#define REG_PAD_MSDC_CFG26        (IO_BASE_ADDR + 0x328)		//msdc0_8b_dat5
#define REG_PAD_MSDC_CFG27        (IO_BASE_ADDR + 0x32C)		//msdc0_8b_dat6
#define REG_PAD_MSDC_CFG28        (IO_BASE_ADDR + 0x330)		//msdc0_8b_dat7


#define MSDC_CMD_NORESPONSE	1
#define MSDC_CMD_OK		2
#define MSDC_CMD_ERROR		3
#define MSDC_CMD_BUSY		4

#define MSDC_INT_NOTHING        0
#define MSDC_INT_NORMAL		1
#define MSDC_INT_UNEXPECT	2
#define MSDC_INT_TIMEOUT	3



#define MSDC_VECTOR               (60) //(VECTOR_FLASHCARD)

#define MSDC_CH_NUM               (unsigned int) (1)
#define MSDC_CH_OFFSET            (unsigned int) (0x00008000)

#define MSDC_CH1				  0x0B000
#define MSDC_CH2				  0x21000
#define MSDC_CH3				  0x0A000

#define MSDC_CH_INDEX(ch)		  MSDC_GetPortIndex(ch)

#define MSDC_CLR_FIFO(ch) \
    do { \
        MSDC_SETBIT(MSDC_FIFOCS(ch), MSDC_FIFOCS_CLR); \
        MSDC_RETRY(MSDC_READ32(MSDC_FIFOCS(ch)) & MSDC_FIFOCS_CLR, 5, 1000); \
    } while(0)

#define MSDC_RETRY(expr, retry, cnt) \
    do { \
        UINT32 t = cnt; \
        UINT32 r = retry; \
        UINT32 c = cnt; \
        while (r) { \
            if (!(expr)) break; \
            if (c-- == 0) { \
                r--; TIM_DelayUS(200); c = t; \
            } \
        } \
    } while(0)

#define MSDC_DMA_ON(ch)   	MSDC_CLRBIT(MSDC_CFG(ch), MSDC_CFG_PIO_MODE)
#define MSDC_DMA_OFF(ch)  	MSDC_SETBIT(MSDC_CFG(ch), MSDC_CFG_PIO_MODE)
#define MSDC_START_DMA(ch)	MSDC_SETBIT(DMA_CTRL(ch), MSDC_CFG_SD)
#define MSDC_DMA_STOP(ch) \
    do { \
        MSDC_SETBIT(DMA_CTRL(ch), DMA_CTRL_STOP); \
        MSDC_RETRY(MSDC_READ32(DMA_CFG(ch)) & DMA_CFG_DMA_STATUS, 5, 1000); \
    } while(0)

#define MSDC_CFG(ch)              (ch + 0x00)
#define MSDC_IOCON(ch)            (ch + 0x04)
#define MSDC_PS(ch)               (ch + 0x08)
#define MSDC_INT(ch)              (ch + 0x0C)
#define MSDC_INTEN(ch)            (ch + 0x10)
#define MSDC_FIFOCS(ch)           (ch + 0x14)
#define MSDC_TXDATA(ch)           (ch + 0x18)
#define MSDC_RXDATA(ch)           (ch + 0x1C)
#define SDC_CFG(ch)               (ch + 0x30)
#define SDC_CMD(ch)               (ch + 0x34)
#define SDC_ARG(ch)               (ch + 0x38)
#define SDC_STS(ch)               (ch + 0x3C)
#define SDC_RESP0(ch)             (ch + 0x40)
#define SDC_RESP1(ch)             (ch + 0x44)
#define SDC_RESP2(ch)             (ch + 0x48)
#define SDC_RESP3(ch)             (ch + 0x4C)
#define SDC_BLK_NUM(ch)           (ch + 0x50)
#define SDC_CSTS(ch)              (ch + 0x58)
#define SDC_CSTS_EN(ch)           (ch + 0x5C)
#define SDC_DATCRC_STS(ch)        (ch + 0x60)
#define EMMC_CFG0(ch)             (ch + 0x70)
#define EMMC_CFG1(ch)             (ch + 0x74)
#define EMMC_STS(ch)              (ch + 0x78)
#define EMMC_IOCON(ch)            (ch + 0x7C)
#define ACMD_RESP(ch)             (ch + 0x80)
#define ACMD19_TRG(ch)            (ch + 0x84)
#define ACMD19_STS(ch)            (ch + 0x88)
#define DMA_SA(ch)                (ch + 0x90)
#define DMA_CA(ch)                (ch + 0x94)
#define DMA_CTRL(ch)              (ch + 0x98)
#define DMA_CFG(ch)               (ch + 0x9C)
#define DBG_SEL(ch)               (ch + 0xA0)
#define DBG_OUT(ch)               (ch + 0xA4)
#define DMA_LEN(ch)               (ch + 0xA8)
#define PATCH_BIT(ch)             (ch + 0xB0)
#define SD20_PAD_CTL0(ch)         (ch + 0xE0)
#define SD20_PAD_CTL1(ch)         (ch + 0xE4)
#define SD20_PAD_CTL2(ch)         (ch + 0xE8)
#define GPIO_DBG_OUT(ch)          (ch + 0xEB)
#define PAD_TUNE(ch)              (ch + 0xEC)
#define HW_DBG(ch)                (ch + 0xF8)
#define VERSION(ch)               (ch + 0xFC)

/* MSDC_CFG */
#define MSDC_CFG_SD                     (0x01 << 0)
#define MSDC_CFG_CK_EN                  (0x01 << 1)
#define MSDC_CFG_RST                    (0x01 << 2)
#define MSDC_CFG_PIO_MODE               (0x01 << 3)
#define MSDC_CFG_BUS_VOL_START          (0x01 << 5)
#define MSDC_CFG_BUS_VOL_PASS           (0x01 << 6)
#define MSDC_CFG_CARD_CK_STABLE         (0x01 << 7)
#define MSDC_CFG_CK_DIV_SHIFT           (8)
#define MSDC_CFG_CK_MODE_DIVIDER        (0x00 << 16)
#define MSDC_CFG_CK_MODE_DIRECT         (0x01 << 16)
#define MSDC_CFG_CK_MODE_DDR            (0x02 << 16)
#define MSDC_CFG_CK_MODE_MASK		(0x03 << 16)
#define MSDC_CFG_CK_DIV_MASK            (0xFF << 8)

/* MSDC_IOCON */
#define MSDC_IOCON_RISC_SIZE_MASK           (0x03 << 24)
#define MSDC_IOCON_RISC_SIZE_BYTE           (0x00 << 24)
#define MSDC_IOCON_RISC_SIZE_WORD          (0x01 << 24)
#define MSDC_IOCON_RISC_SIZE_DWRD           (0x02 << 24)

/* MSDC_INT */
#define INT_MMC_IRQ                     (0x01 << 0)
#define INT_MSDC_CDSC                   (0x01 << 1)
#define INT_SD_AUTOCMD_RDY              (0x01 << 3)
#define INT_SD_AUTOCMD_TO               (0x01 << 4)
#define INT_SD_AUTOCMD_RESP_CRCERR      (0x01 << 5)
#define INT_DMA_Q_EMPTY                 (0x01 << 6)
#define INT_SD_SDIOIRQ                  (0x01 << 7)
#define INT_SD_CMDRDY                   (0x01 << 8)
#define INT_SD_CMDTO                    (0x01 << 9)
#define INT_SD_RESP_CRCERR              (0x01 << 10)
#define INT_SD_CSTA                     (0x01 << 11)
#define INT_SD_XFER_COMPLETE            (0x01 << 12)
#define INT_DMA_XFER_DONE               (0x01 << 13)
#define INT_SD_DATTO                    (0x01 << 14)
#define INT_SD_DATA_CRCERR              (0x01 << 15)
#define INT_MS_RDY                      (0x01 << 24)
#define INT_MS_SIF                      (0x01 << 25)
#define INT_MS_TOER                     (0x01 << 26)
#define INT_MS_CRCERR                   (0x01 << 27)
#define INT_MS_CED                      (0x01 << 28)
#define INT_MS_ERR                      (0x01 << 29)
#define INT_MS_BREQ                     (0x01 << 30)
#define INT_CMDNK                       (0x01 << 31)

/* SDC_CFG */
#define SDC_CFG_BW_SHIFT                (16)
#define SDC_CFG_SDIO                    (0x01 << 19)
#define SDC_CFG_INTAT_BLK_GAP           (0x01 << 21)
#define SDC_CFG_DTOC_SHIFT              (24)

/* EMMC_CFG0 */
#define EMMC_BOOT_SUPPORT               (1 << 15)
#define EMMC_BOOT_SUPPORT_MASK          (1 << 15)
#define EMMC_BOOT_WAIT_EXIT_DELAY(value)      ((value & 0x7) << 12)
#define EMMC_BOOT_MODE_PULL_LOW         (0 << 2)
#define EMMC_BOOT_MODE_RESET_CMD        (1 << 2)
#define EMMC_BOOT_STOP                  (1 << 1)
#define EMMC_BOOT_STOP_MASK		(1 << 1)

#define EMMC_BOOT_START                 (1 << 0)
#define EMMC_BOOT_START_MASK		(1 << 0)

#define EMMC_BOOT_ACK_TOC(value)         ((value&0xFFF)<< 20)
#define EMMC_BOOT_DAT_TOC(value)         (value & 0xFFFFF)

/* EMMC_STS */
#define EMMC_BOOT_DAT_RECV		(1 << 6)
#define EMMC_BOOT_ACK_RECV              (1 << 5)
#define EMMC_BOOT_UP_STATE              (1 << 4)
#define EMMC_BOOT_ACK_TO                (1 << 3)
#define EMMC_BOOT_DAT_TO                (1 << 2)
#define EMMC_BOOT_ACK_ERR               (1 << 1)
#define EMMC_BOOT_CRC_ERR               (1 << 0)

/* EMMC_IOCON */
#define EMMC_BOOT_RST                   (1)

/* SDC_CMD */
#define SDC_CMD_BREAK                   (0x01 << 6)

#define SDC_CMD_RSPTYPE_NO              (0x00 << 7)
#define SDC_CMD_RSPTYPE_R1R5R6R7        (0x01 << 7)
#define SDC_CMD_RSPTYPE_R2              (0x02 << 7)
#define SDC_CMD_RSPTYPE_R3              (0x03 << 7)
#define SDC_CMD_RSPTYPE_R4              (0x04 << 7)

#define SDC_CMD_RSPTYPE_R1R5R6R7        (0x01 << 7)
#define SDC_CMD_RSPTYPE_R1B             (0x07 << 7)
#define SDC_CMD_RSPTYPE_MASK		(0x07 << 7)
#define SDC_CMD_CMD_MASK                (0x3F << 0)

#define DTYPE_NONE                      (0x00 << 11)
#define DTYPE_SINGLE_BLK                (0x01 << 11)
#define DTYPE_MULTI_BLK                 (0x02 << 11)
#define DTYPE_STREAM                    (0x03 << 11)
#define SDC_CMD_READ                    (0x00 << 13)
#define SDC_CMD_WRITE                   (0x01 << 13)
#define SDC_CMD_STOP                    (0x01 << 14)
#define SDC_CMD_GO_IRQ                  (0x01 << 15)
#define SDC_CMD_LEN_SHIFT               (16)
#define SDC_CMD_AUTO_CMD_NONE           (0x0 << 28)
#define SDC_CMD_AUTO_CMD12              (0x1 << 28)
#define SDC_CMD_ATUO_CMD23              (0x2 << 28)
#define SDC_CMD_AUTO_CMD19              (0x3 << 28)
#define SDC_CMD_VOL_SWITCH              (0x1 << 30)

/* MSDC_FIFOCS */
#define MSDC_FIFO_LEN                   (128)
#define MSDC_FIFOCS_TXFIFOCNT_SHIFT     (16)
#define MSDC_FIFOCS_RXFIFOCNT_SHIFT     (0)
#define MSDC_FIFOCS_FIFOCNT_MASK      (0x000000FF)
#define MSDC_FIFOCS_CLR                 (1UL << 31)
#define MSDC_FIFOCS_CLR_MASK            (1UL << 31)


/* SDC_CFG */
#define SDC_SDIO_EN                 (0x01<<19)
#define SDC_BUSWIDTH                (0x03<<16)
#define SDC_DTOC                    (0xFF000000)

/* SDC_STS */
#define SDC_STS_SDCBUSY                 (0x01<<0x0)
#define SDC_STS_CMDBUSY                 (0x01<<0x1)

/* DMA_CTRL */
#define DMA_CTRL_BST_SHIFT              (12)
//#define DMA_CTRL_BST_8                  (3 << DMA_CTRL_BST_SHIFT)
//#define DMA_CTRL_BST_16                 (4 << DMA_CTRL_BST_SHIFT)
//#define DMA_CTRL_BST_32                 (5 << DMA_CTRL_BST_SHIFT)
//#define DMA_CTRL_BST_64                 (6 << DMA_CTRL_BST_SHIFT)
#define DMA_CTRL_START                  (0x01 << 0)
#define DMA_CTRL_STOP                   (0x01 << 1)
#define DMA_CTRL_RESUME                 (0x01 << 2)
#define DMA_CTRL_LAST_BUF               (0x01 << 10)
#define DMA_CTRL_XFER_SIZE_SHIFT        (16)
#define DMA_CTRL_DESC_MODE              (0x01 << 8)
#define DMA_CTRL_BRUSTSZ                (0x7  << 12)

#define MSDC_DMA_BST_8                  (3)
#define MSDC_DMA_BST_16                 (4)
#define MSDC_DMA_BST_32                 (5)
#define MSDC_DMA_BST_64                 (6)

/* DMA_CFG */
#define DMA_CFG_DMA_STATUS              (0x01 << 0)
#define DMA_CFG_CHKSUM                  (0x01 << 1)
#define DMA_CFG_BD_CS_ERR               (0x01 << 4)
#define DMA_CFG_GPD_CS_ERR              (0x01 << 5)

/* PATCH_BIT */
#define R1B_DELAY_CYCLE              (0x0F << 18)

//************************************************************//
//
//	PAD Setting Config
//
//************************************************************//

// For 4bit mode, only use for nand bootup
//#define PAD_CFG_SD0_4BIT_CLK		(0x02C0)
//#define PAD_CFG_SD0_4BIT_CMD		(0x02C4)
//#define PAD_CFG_SD0_4BIT_DAT0		(0x02C8)
//#define PAD_CFG_SD0_4BIT_DAT1		(0x02CC)
//#define PAD_CFG_SD0_4BIT_DAT2		(0x02D0)
//#define PAD_CFG_SD0_4BIT_DAT3		(0x02D4)
//#define PAD_CFG_SD0_4BIT_RST		(0x0334)


//#define PAD_CFG_SD1_CLK				(0x02D8)
//#define PAD_CFG_SD1_CMD				(0x02DC)
//#define PAD_CFG_SD1_DAT0			(0x02E0)
//#define PAD_CFG_SD1_DAT1			(0x02E4)
//#define PAD_CFG_SD1_DAT2			(0x02E8)
//#define PAD_CFG_SD1_DAT3			(0x02EC)
//#define PAD_CFG_SD1_RST				(0x033C)


#define PAD_CFG_SD2_CLK				(0x02F0)
#define PAD_CFG_SD2_CMD				(0x02F4)
#define PAD_CFG_SD2_DAT0			(0x02F8)
#define PAD_CFG_SD2_DAT1			(0x02FC)
#define PAD_CFG_SD2_DAT2			(0x0300)
#define PAD_CFG_SD2_DAT3			(0x0304)
//#define PAD_CFG_SD2_RST				(0x0340)


#define PAD_CFG_SD0_CLK				(0x030C)
#define PAD_CFG_SD0_CMD				(0x0310)
#define PAD_CFG_SD0_DAT0			(0x0314)
#define PAD_CFG_SD0_DAT1			(0x0318)
#define PAD_CFG_SD0_DAT2			(0x031C)
#define PAD_CFG_SD0_DAT3			(0x0320)
#define PAD_CFG_SD0_DAT4			(0x0324)
#define PAD_CFG_SD0_DAT5			(0x0328)
#define PAD_CFG_SD0_DAT6			(0x032C)
#define PAD_CFG_SD0_DAT7			(0x0330)
//#define PAD_CFG_SD0_RST				(0x0338)

// Bit mask and bit offset
#define PAD_CFG_TDSEL_MASK			(0x0F << 23)
#define PAD_CFG_RDSEL_MASK			(0xFF << 15)
#define PAD_CFG_SMT_MASK			(0x01 << 14)
#define PAD_CFG_RESISTOR_MASK			(0x03 << 12)
#define PAD_CFG_PUPD_MASK			(0x01 << 11)
#define PAD_CFG_IES_MASK			(0x01 << 10)
#define PAD_CFG_DRV_MASK			(0x3F << 4)
#define PAD_CFG_SR_MASK				(0x0F << 0)

#define PAD_CFG_TDSEL_SHFIT			(23)
#define PAD_CFG_RDSEL_SHFIT			(15)
#define PAD_CFG_SMT_SHFIT			(14)
#define PAD_CFG_RESISTOR_SHFIT			(12)
#define PAD_CFG_PUPD_SHFIT			(11)
#define PAD_CFG_IES_SHFIT			(10)
#define PAD_CFG_DRV_SHFIT			(4)
#define PAD_CFG_SR_SHFIT			(0)

#define HAL_READ32(_reg_)           (*((volatile UINT32*)(_reg_)))
#define HAL_WRITE32(_reg_, _val_)   (*((volatile UINT32*)(_reg_)) = (_val_))


#define MSDC_WRITE32(reg,val)		  	HAL_WRITE32((IO_BASE_ADDR+reg),val)
#define MSDC_READ32(reg)				HAL_READ32((IO_BASE_ADDR+reg))

#define MSDC_SETBIT(reg, dBit)        MSDC_WRITE32(reg, MSDC_READ32(reg) | (dBit))
#define MSDC_CLRBIT(reg, dBit)        MSDC_WRITE32(reg, MSDC_READ32(reg) & (~(dBit)))

#define MSDC_MASK32(reg,value,mask)	  MSDC_WRITE32(reg, (MSDC_READ32(reg) & (~(mask)))|value)

#if 1
#define MSDC_SET_FIELD(addr, field, val) MSDC_WRITE32(addr, (MSDC_READ32(addr) & (~(field)))| val)

static inline unsigned int uffs(unsigned int x)
{
    unsigned int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        x >>= 1;
        r += 1;
    }
    return r;
}

#define MSDC_SET_VAL(addr, field, val) \
    do {	\
        volatile unsigned int tv = MSDC_READ32(addr);	\
        tv &= ~(field); \
        tv |= ((val) << (uffs((unsigned int)field) - 1)); \
        MSDC_WRITE32(addr, tv); \
    } while(0)
#else //MARKED_UART_ISSUE

static inline unsigned int uffs(unsigned int x)
{
    unsigned int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        x >>= 1;
        r += 1;
    }
    return r;
}

#define MSDC_SET_FIELD(addr, field, val) \
    do {	\
        volatile unsigned int tv = MSDC_READ32(addr);	\
        tv &= ~(field); \
        tv |= ((val) << (uffs((unsigned int)field) - 1)); \
        MSDC_WRITE32(addr, tv); \
    } while(0)
#endif

#define MSDC_CARD_DEFAULT_BLOCK_LEN         512
#define MSDC_CARD_SDHC_FIX_BLOCK_LEN        512
#define MSDC_CARD_MMC_TOTAL_EXT_CSD_SIZE    512

#define CARD_STATUS_BUSY		(1 << 16)
#define CARD_STATUS_CARDRETURNSTATUS_MASK   (0xFFFF)
#define CARD_STATUS_READYFORDATA    (1 << 8)

#define CARD_BLOCKLEN_SHIFTBIT_MASK      (0xFFFF)

#define CARD_OPS_SECTOR_ADDRESS     (1 << 0)

// For timeout support
#define BIM_REG_OFFSET                      (0x8000)
#define T0VAL_REG  							(0x14c)


#define u4ReadBIM(u2Reg)  \
          *(volatile UINT32 *)(IO_BASE_ADDR + BIM_REG_OFFSET + (u2Reg))


typedef struct _MSDC_CARD_T_
{
	UINT32 cardType;
	UINT32 cardOCR;
	UINT32 cardStatus;
	UINT32 cardRCA;
    UINT32 cardDeviceSize;
    UINT32 cardMaxSpeed;
    UINT32 cardMaxBlockLen;
    UINT32 cardBlockLen;
    UINT32 cardOps;
}MSDC_CARD_T;


extern void MSDC_Init(UINT32 ch);
extern UINT32 MSDC_Identify_Card(UINT32 ch);
extern UINT32 MSDC_StateChange(UINT32 ch,UINT32 cardState);
extern UINT32 MSDC_ReadBlock_PIO(UINT32 ch, UINT32 startAddr,UINT32 *pBuf,UINT32 dataSize);
extern UINT32 MSDC_SetBlockLength(UINT32 ch,UINT32 blkLen);
extern UINT32  MSDC_SetBusWidth(UINT32 ch, UINT32 u4BusWidth);


// EMMC boot partition OPS
extern UINT32 MSDC_EMMC_EnterBootMode0(UINT32 ch);
extern UINT32 MSDC_EMMC_ExitBootMode0(UINT32 ch);
extern UINT32 MSDC_EMMC_EnterBoot1(UINT32 ch);
extern UINT32 MSDC_EMMC_EnterBoot2(UINT32 ch);
extern UINT32 MSDC_EMMC_EnterUser(UINT32 ch);
extern UINT32 MSDC_EMMC_Read(UINT32 ch, UINT32 startAddr,UINT32 *pBuf,UINT32 dataSize);
//extern UINT32 MSDC_EMMC_ReadFromBoot2(UINT32 ch, UINT32 u4DramkPhyOffset, UINT32 u4DramkRunAddress, UINT32 u4DramkSize);



#define MSDC_DELAY(n)    \
    MSDC_DelayCounter = n;             \
    while(MSDC_DelayCounter--)


#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))
#define ALIGN(x,a)		__ALIGN_MASK(x,(typeof(x))(a)-1)



#endif
