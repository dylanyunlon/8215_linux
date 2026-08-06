#ifndef __ATC_MSDC_H__
#define __ATC_MSDC_H__

#include <linux/bitops.h>
#include <linux/mmc/host.h>
#include <mach/sync_write.h>
#include <linux/semaphore.h>

#define MTK_MSDC_USE_CMD23
#ifdef MTK_MSDC_USE_CMD23
#define MSDC_USE_AUTO_CMD23   			(0) 
#endif

//#define MSDC_POWER_FAIL_WP

#define MSDC_CLK_200MHZ					(200U * 1000U * 1000U)
#define MSDC_CLK_100MHZ					(100U * 1000U * 1000U)
#define MSDC_CLK_50MHZ					(50U * 1000U * 1000U)
#define MSDC_CLK_25MHZ					(25U * 1000U * 1000U)
#define MSDC_CLK_400KHZ					(400U * 1000U)
#define MSDC_CLK_1MHZ					(1000U * 1000U)

//#define MTK_EMMC_SUPPORT // Max Make it support MMC
#define ATC_EMMC_SUPPORT

//------------------------------
// Temp define
typedef enum {
	EMMC_PART_UNKNOWN=0
	,EMMC_PART_BOOT1
	,EMMC_PART_BOOT2
	,EMMC_PART_RPMB
	,EMMC_PART_GP1
	,EMMC_PART_GP2
	,EMMC_PART_GP3
	,EMMC_PART_GP4
	,EMMC_PART_USER
	,EMMC_PART_END
} Region;

#ifdef MTK_EMMC_SUPPORT
#define PART_NUM			23
typedef enum  {
	EMMC = 1,
	NAND = 2,
} dev_type;

struct excel_info{
	char * name;
	unsigned long long size;
	unsigned long long start_address;
	dev_type type ;
	unsigned int partition_idx;
	Region region;
};

static const struct excel_info PartInfo_Private[PART_NUM]={			{"preloader",262144,0, EMMC, 0,EMMC_PART_BOOT1},
			{"mbr",524288,0x0, EMMC, 0,EMMC_PART_USER},
			{"ebr1",524288,0x80000, EMMC, 1,EMMC_PART_USER},
			{"pro_info",3145728,0x100000, EMMC, 0,EMMC_PART_USER},
			{"nvram",5242880,0x400000, EMMC, 0,EMMC_PART_USER},
			{"protect_f",10485760,0x900000, EMMC, 2,EMMC_PART_USER},
			{"protect_s",10485760,0x1300000, EMMC, 3,EMMC_PART_USER},
			{"seccfg",131072,0x1d00000, EMMC, 0,EMMC_PART_USER},
			{"uboot",393216,0x1d20000, EMMC, 0,EMMC_PART_USER},
			{"bootimg",16777216,0x1d80000, EMMC, 0,EMMC_PART_USER},
			{"recovery",16777216,0x2d80000, EMMC, 0,EMMC_PART_USER},
			{"sec_ro",6291456,0x3d80000, EMMC, 4,EMMC_PART_USER},
			{"misc",524288,0x4380000, EMMC, 0,EMMC_PART_USER},
			{"logo",3145728,0x4400000, EMMC, 0,EMMC_PART_USER},
			{"expdb",10485760,0x4700000, EMMC, 0,EMMC_PART_USER},
			{"tee1",5242880,0x5100000, EMMC, 0,EMMC_PART_USER},
			{"tee2",5242880,0x5600000, EMMC, 0,EMMC_PART_USER},
			{"kb",1048576,0x5b00000, EMMC, 0,EMMC_PART_USER},
			{"dkb",1048576,0x5c00000, EMMC, 0,EMMC_PART_USER},
			{"android",1073741824,0x5d00000, EMMC, 5,EMMC_PART_USER},
			{"cache",132120576,0x45d00000, EMMC, 6,EMMC_PART_USER},
			{"usrdata",2147483648,0x4db00000, EMMC, 7,EMMC_PART_USER},
			{"bmtpool",22020096,0xFFFF00a8, EMMC, 0,EMMC_PART_USER},
};


extern struct excel_info *PartInfo;

#endif
//------------------------------

/* the macro need removed after SQC */
//#define MSDC_DMA_BOUNDARY_LIMITAION


#define MAX_GPD_NUM         (1 + 1)  /* one null gpd */
#define MAX_BD_NUM          (1024)
#define MAX_BD_PER_GPD      (MAX_BD_NUM)
#define HOST_MAX_NUM        (3)		// MSDC host controller number
#define CLK_SRC_MAX_NUM		(1)

#define CUST_EINT_POLARITY_LOW              0
#define CUST_EINT_POLARITY_HIGH             1
#define CUST_EINT_DEBOUNCE_DISABLE          0
#define CUST_EINT_DEBOUNCE_ENABLE           1
#define CUST_EINT_EDGE_SENSITIVE            0
#define CUST_EINT_LEVEL_SENSITIVE           1
#define SDIO_ERROR_BYPASS

// Host Request Error Code definition
#define REQ_CMD_EIO  	(0x1 << 0)
#define REQ_CMD_TMO  	(0x1 << 1)
#define REQ_DAT_ERR  	(0x1 << 2)
#define REQ_STOP_EIO 	(0x1 << 3)
#define REQ_STOP_TMO 	(0x1 << 4)

//////////////////////////////////////////////////////////////////////////////


//#define CONFIG_MACH_AC8317FPGA
#ifdef CONFIG_MACH_AC8317FPGA
#define FPGA_PLATFORM   			// Now, this marco definition is disable......
#endif

#define MSDC_AUTOCMD12          ((u8)0x0001)
#define MSDC_AUTOCMD23          ((u8)0x0002)
#define MSDC_AUTOCMD19          ((u8)0x0008)
/*--------------------------------------------------------------------------*/
/* Common Macro                                                             */
/*--------------------------------------------------------------------------*/
//#define REG_ADDR(x)                 ((volatile u32*)(base + OFFSET_##x))
#define REG_ADDR(x)                 ((volatile u32*)(base + (x)))


/*--------------------------------------------------------------------------*/
/* Common Definition                                                        */
/*--------------------------------------------------------------------------*/
#define MSDC_FIFO_SZ            (128)
#define MSDC_FIFO_THD           (64)  // (128)
#define MSDC_NUM                (4)

#define MSDC_MS                 (0) //No memory stick mode, 0 use to gate clock
#define MSDC_SDMMC              (1)

#define MSDC_MODE_UNKNOWN       (0)
#define MSDC_MODE_PIO           (1)
#define MSDC_MODE_DMA_BASIC     (2)
#define MSDC_MODE_DMA_DESC      (3)
#define MSDC_MODE_DMA_ENHANCED  (4)
#define MSDC_MODE_MMC_STREAM    (5)

#define MSDC_BUS_1BITS          (0)
#define MSDC_BUS_4BITS          (1)
#define MSDC_BUS_8BITS          (2)

#define MSDC_BRUST_8B           (3)
#define MSDC_BRUST_16B          (4)
#define MSDC_BRUST_32B          (5)
#define MSDC_BRUST_64B          (6)

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

/*--------------------------------------------------------------------------*/
/* Register Offset                                                          */
/*--------------------------------------------------------------------------*/
#define OFFSET_MSDC_CFG         (0x00U)
#define OFFSET_MSDC_IOCON       (0x04U)
#define OFFSET_MSDC_PS          (0x08U)
#define OFFSET_MSDC_INT         (0x0CU)
#define OFFSET_MSDC_INTEN       (0x10U)
#define OFFSET_MSDC_FIFOCS      (0x14U)
#define OFFSET_MSDC_TXDATA      (0x18U)
#define OFFSET_MSDC_RXDATA      (0x1CU)
#define OFFSET_SDC_CFG          (0x30U)
#define OFFSET_SDC_CMD          (0x34U)
#define OFFSET_SDC_ARG          (0x38U)
#define OFFSET_SDC_STS          (0x3CU)
#define OFFSET_SDC_RESP0        (0x40U)
#define OFFSET_SDC_RESP1        (0x44U)
#define OFFSET_SDC_RESP2        (0x48U)
#define OFFSET_SDC_RESP3        (0x4CU)
#define OFFSET_SDC_BLK_NUM      (0x50U)
#define OFFSET_SDC_CSTS         (0x58U)
#define OFFSET_SDC_CSTS_EN      (0x5CU)
#define OFFSET_SDC_DCRC_STS     (0x60U)
#define OFFSET_EMMC_CFG0        (0x70U)
#define OFFSET_EMMC_CFG1        (0x74U)
#define OFFSET_EMMC_STS         (0x78U)
#define OFFSET_EMMC_IOCON       (0x7CU)
#define OFFSET_SDC_ACMD_RESP    (0x80U)
#define OFFSET_SDC_ACMD19_TRG   (0x84U)
#define OFFSET_SDC_ACMD19_STS   (0x88U)
#define OFFSET_MSDC_DMA_SA      (0x90U)
#define OFFSET_MSDC_DMA_CA      (0x94U)
#define OFFSET_MSDC_DMA_CTRL    (0x98U)
#define OFFSET_MSDC_DMA_CFG     (0x9CU)
#define OFFSET_MSDC_DBG_SEL     (0xA0U)
#define OFFSET_MSDC_DBG_OUT     (0xA4U)
#define OFFSET_MSDC_DMA_LEN     (0xA8U)
#define OFFSET_MSDC_PATCH_BIT0  (0xB0U)
#define OFFSET_MSDC_PATCH_BIT1  (0xB4U)
#ifdef MTK_SDIO30_ONLINE_TUNING_SUPPORT
#define OFFSET_DAT0_TUNE_CRC    (0xC0U)
#define OFFSET_DAT1_TUNE_CRC    (0xC4U)
#define OFFSET_DAT2_TUNE_CRC    (0xC8U)
#define OFFSET_DAT3_TUNE_CRC    (0xCCU)
#define OFFSET_CMD_TUNE_CRC     (0xD0U)
#define OFFSET_SDIO_TUNE_WIND   (0xD4U)
#endif	// MTK_SDIO30_ONLINE_TUNING_SUPPORT
#define OFFSET_MSDC_PAD_TUNE    (0xECU)
#define OFFSET_MSDC_DAT_RDDLY0  (0xF0U)
#define OFFSET_MSDC_DAT_RDDLY1  (0xF4U)
#define OFFSET_MSDC_HW_DBG      (0xF8U)
#define OFFSET_MSDC_VERSION     (0x100U)
#define OFFSET_MSDC_ECO_VER     (0x104U)

/*--------------------------------------------------------------------------*/
/* Register Address                                                         */
/*--------------------------------------------------------------------------*/

/* common register */
/*#define MSDC_CFG                REG_ADDR(OFFSET_MSDC_CFG)*/
#define MSDC_CFG                ((volatile u32*)(base + (OFFSET_MSDC_CFG)))
#define MSDC_IOCON              REG_ADDR(OFFSET_MSDC_IOCON)
#define MSDC_PS                 REG_ADDR(OFFSET_MSDC_PS)
#define MSDC_INT                REG_ADDR(OFFSET_MSDC_INT)
#define MSDC_INTEN              REG_ADDR(OFFSET_MSDC_INTEN)
#define MSDC_FIFOCS             REG_ADDR(OFFSET_MSDC_FIFOCS)
#define MSDC_TXDATA             REG_ADDR(OFFSET_MSDC_TXDATA)
#define MSDC_RXDATA             REG_ADDR(OFFSET_MSDC_RXDATA)

/* sdmmc register */
#define SDC_CFG                 REG_ADDR(OFFSET_SDC_CFG)
#define SDC_CMD                 REG_ADDR(OFFSET_SDC_CMD)
#define SDC_ARG                 REG_ADDR(OFFSET_SDC_ARG)
#define SDC_STS                 REG_ADDR(OFFSET_SDC_STS)
#define SDC_RESP0               REG_ADDR(OFFSET_SDC_RESP0)
#define SDC_RESP1               REG_ADDR(OFFSET_SDC_RESP1)
#define SDC_RESP2               REG_ADDR(OFFSET_SDC_RESP2)
#define SDC_RESP3               REG_ADDR(OFFSET_SDC_RESP3)
#define SDC_BLK_NUM             REG_ADDR(OFFSET_SDC_BLK_NUM)
#define SDC_CSTS                REG_ADDR(OFFSET_SDC_CSTS)
#define SDC_CSTS_EN             REG_ADDR(OFFSET_SDC_CSTS_EN)
#define SDC_DCRC_STS            REG_ADDR(OFFSET_SDC_DCRC_STS)

/* emmc register*/
#define EMMC_CFG0               REG_ADDR(OFFSET_EMMC_CFG0)
#define EMMC_CFG1               REG_ADDR(OFFSET_EMMC_CFG1)
#define EMMC_STS                REG_ADDR(OFFSET_EMMC_STS)
#define EMMC_IOCON              REG_ADDR(OFFSET_EMMC_IOCON)

/* auto command register */
#define SDC_ACMD_RESP           REG_ADDR(OFFSET_SDC_ACMD_RESP)
#define SDC_ACMD19_TRG          REG_ADDR(OFFSET_SDC_ACMD19_TRG)
#define SDC_ACMD19_STS          REG_ADDR(OFFSET_SDC_ACMD19_STS)

/* dma register */
#define MSDC_DMA_SA             REG_ADDR(OFFSET_MSDC_DMA_SA)
#define MSDC_DMA_CA             REG_ADDR(OFFSET_MSDC_DMA_CA)
#define MSDC_DMA_CTRL           REG_ADDR(OFFSET_MSDC_DMA_CTRL)
#define MSDC_DMA_CFG            REG_ADDR(OFFSET_MSDC_DMA_CFG)

/* data read delay */
#define MSDC_DAT_RDDLY0         REG_ADDR(OFFSET_MSDC_DAT_RDDLY0)
#define MSDC_DAT_RDDLY1         REG_ADDR(OFFSET_MSDC_DAT_RDDLY1)

/* debug register */
#define MSDC_DBG_SEL            REG_ADDR(OFFSET_MSDC_DBG_SEL)
#define MSDC_DBG_OUT            REG_ADDR(OFFSET_MSDC_DBG_OUT)

#ifdef MTK_SDIO30_ONLINE_TUNING_SUPPORT
/* sdio register */
#define DAT0_TUNE_CRC           REG_ADDR(OFFSET_DAT0_TUNE_CRC)
#define DAT1_TUNE_CRC           REG_ADDR(OFFSET_DAT1_TUNE_CRC)
#define DAT2_TUNE_CRC           REG_ADDR(OFFSET_DAT2_TUNE_CRC)
#define DAT3_TUNE_CRC           REG_ADDR(OFFSET_DAT3_TUNE_CRC)
#define CMD_TUNE_CRC            REG_ADDR(OFFSET_CMD_TUNE_CRC)
#define SDIO_TUNE_WIND          REG_ADDR(OFFSET_SDIO_TUNE_WIND)
#endif	// MTK_SDIO30_ONLINE_TUNING_SUPPORT

#define MSDC_DMA_LEN            REG_ADDR(OFFSET_MSDC_DMA_LEN)

/* misc register */
#define MSDC_PATCH_BIT0         REG_ADDR(OFFSET_MSDC_PATCH_BIT0)
#define MSDC_PATCH_BIT1         REG_ADDR(OFFSET_MSDC_PATCH_BIT1)
#define MSDC_PAD_TUNE           REG_ADDR(OFFSET_MSDC_PAD_TUNE)
#define MSDC_HW_DBG             REG_ADDR(OFFSET_MSDC_HW_DBG)
#define MSDC_VERSION            REG_ADDR(OFFSET_MSDC_VERSION)
#define MSDC_ECO_VER            REG_ADDR(OFFSET_MSDC_ECO_VER) /* ECO Version */

/*--------------------------------------------------------------------------*/
/* Register Mask                                                            */
/*--------------------------------------------------------------------------*/

/* MSDC_CFG mask */
#define MSDC_CFG_MODE           (0x1U  << 0)     /* RW */
#define MSDC_CFG_CKPDN          (0x1U  << 1)     /* RW */
#define MSDC_CFG_RST            (0x1U  << 2)     /* RW */
#define MSDC_CFG_PIO            (0x1U  << 3)     /* RW */
#define MSDC_CFG_CKDRVEN        (0x1U  << 4)     /* RW */
#define MSDC_CFG_BV18SDT        (0x1U  << 5)     /* RW */
#define MSDC_CFG_BV18PSS        (0x1U  << 6)     /* R  */
#define MSDC_CFG_CKSTB          (0x1U  << 7)     /* R  */
#define MSDC_CFG_CKDIV          (0xffU << 8)     /* RW */
#define MSDC_CFG_CKMOD          (0x3U  << 16)    /* RW */

/* MSDC_IOCON mask */
#define MSDC_IOCON_SDR104CKS    (0x1U  << 0)     /* RW */
#define MSDC_IOCON_RSPL         (0x1U  << 1)     /* RW */
#define MSDC_IOCON_DSPL         (0x1U  << 2)     /* RW */
#define MSDC_IOCON_DDLSEL       (0x1U  << 3)     /* RW */
#define MSDC_IOCON_DDR50CKD     (0x1U  << 4)     /* RW */
#define MSDC_IOCON_DSPLSEL      (0x1U  << 5)     /* RW */
#define MSDC_IOCON_RDSPLSEL     (0x1U  << 5)     /* RW */	/* New definition of MSDC_CODA_SD30_v2.13 */
#define MSDC_IOCON_WDSPLSEL     (0x1U  << 9)     /* RW */	/* New definition of MSDC_CODA_SD30_v2.13 */
#define MSDC_IOCON_W_DSPL       (0x1U  << 8)     /* RW */
#define MSDC_IOCON_WD0_SMPL     (0x1U  << 10)    /* RW */
#define MSDC_IOCON_WD1_SMPL     (0x1U  << 11)    /* RW */
#define MSDC_IOCON_WD2_SMPL     (0x1U  << 12)    /* RW */
#define MSDC_IOCON_WD3_SMPL     (0x1U  << 13)    /* RW */
#define MSDC_IOCON_D0SPL        (0x1U  << 16)    /* RW */
#define MSDC_IOCON_D1SPL        (0x1U  << 17)    /* RW */
#define MSDC_IOCON_D2SPL        (0x1U  << 18)    /* RW */
#define MSDC_IOCON_D3SPL        (0x1U  << 19)    /* RW */
#define MSDC_IOCON_D4SPL        (0x1U  << 20)    /* RW */
#define MSDC_IOCON_D5SPL        (0x1U  << 21)    /* RW */
#define MSDC_IOCON_D6SPL        (0x1U  << 22)    /* RW */
#define MSDC_IOCON_D7SPL        (0x1U  << 23)    /* RW */
#define MSDC_IOCON_RISCSZ       (0x3U  << 24)    /* RW */

/* MSDC_PS mask */
#define MSDC_PS_CDEN            (0x1U  << 0)     /* RW */
#define MSDC_PS_CDSTS           (0x1U  << 1)     /* R  */
#define MSDC_PS_CDDEBOUNCE      (0xfU  << 12)    /* RW */
#define MSDC_PS_DAT             (0xffU << 16)    /* R  */
#define MSDC_PS_CMD             (0x1U  << 24)    /* R  */
#define MSDC_PS_WP              (0x1U<< 31)    /* R  */

/* MSDC_INT mask */
#define MSDC_INT_MMCIRQ         (0x1U  << 0)     /* W1C */
#define MSDC_INT_CDSC           (0x1U  << 1)     /* W1C */
#define MSDC_INT_ACMDRDY        (0x1U  << 3)     /* W1C */
#define MSDC_INT_ACMDTMO        (0x1U  << 4)     /* W1C */
#define MSDC_INT_ACMDCRCERR     (0x1U  << 5)     /* W1C */
#define MSDC_INT_DMAQ_EMPTY     (0x1U  << 6)     /* W1C */
#define MSDC_INT_SDIOIRQ        (0x1U  << 7)     /* W1C */
#define MSDC_INT_CMDRDY         (0x1U  << 8)     /* W1C */
#define MSDC_INT_CMDTMO         (0x1U  << 9)     /* W1C */
#define MSDC_INT_RSPCRCERR      (0x1U  << 10)    /* W1C */
#define MSDC_INT_CSTA           (0x1U  << 11)    /* R */
#define MSDC_INT_XFER_COMPL     (0x1U  << 12)    /* W1C */
#define MSDC_INT_DXFER_DONE     (0x1U  << 13)    /* W1C */
#define MSDC_INT_DATTMO         (0x1U  << 14)    /* W1C */
#define MSDC_INT_DATCRCERR      (0x1U  << 15)    /* W1C */
#define MSDC_INT_ACMD19_DONE    (0x1U  << 16)    /* W1C */
#ifdef MTK_SDIO30_ONLINE_TUNING_SUPPORT
#define MSDC_INT_GEAR_OUT_BOUND (0x1U  << 20)    /* W1C */
#define MSDC_INT_ACMD53_DONE    (0x1U  << 21)    /* W1C */
#define MSDC_INT_ACMD53_FAIL    (0x1U  << 22)    /* W1C */
#endif	// MTK_SDIO30_ONLINE_TUNING_SUPPORT

/* MSDC_INTEN mask */
#define MSDC_INTEN_MMCIRQ       (0x1U  << 0)     /* RW */
#define MSDC_INTEN_CDSC         (0x1U  << 1)     /* RW */
#define MSDC_INTEN_ACMDRDY      (0x1U  << 3)     /* RW */
#define MSDC_INTEN_ACMDTMO      (0x1U  << 4)     /* RW */
#define MSDC_INTEN_ACMDCRCERR   (0x1U  << 5)     /* RW */
#define MSDC_INTEN_DMAQ_EMPTY   (0x1U  << 6)     /* RW */
#define MSDC_INTEN_SDIOIRQ      (0x1U  << 7)     /* RW */
#define MSDC_INTEN_CMDRDY       (0x1U  << 8)     /* RW */
#define MSDC_INTEN_CMDTMO       (0x1U  << 9)     /* RW */
#define MSDC_INTEN_RSPCRCERR    (0x1U  << 10)    /* RW */
#define MSDC_INTEN_CSTA         (0x1U  << 11)    /* RW */
#define MSDC_INTEN_XFER_COMPL   (0x1U  << 12)    /* RW */
#define MSDC_INTEN_DXFER_DONE   (0x1U  << 13)    /* RW */
#define MSDC_INTEN_DATTMO       (0x1U  << 14)    /* RW */
#define MSDC_INTEN_DATCRCERR    (0x1U  << 15)    /* RW */
#define MSDC_INTEN_ACMD19_DONE  (0x1U  << 16)    /* RW */

/* MSDC_FIFOCS mask */
#define MSDC_FIFOCS_RXCNT       (0xffU << 0)     /* R */
#define MSDC_FIFOCS_TXCNT       (0xffU << 16)    /* R */
#define MSDC_FIFOCS_CLR         (0x1U<< 31)    /* RW */

/* SDC_CFG mask */
#define SDC_CFG_SDIOINTWKUP     (0x1U  << 0)     /* RW */
#define SDC_CFG_INSWKUP         (0x1U  << 1)     /* RW */
#define SDC_CFG_BUSWIDTH        (0x3U  << 16)    /* RW */
#define SDC_CFG_SDIO            (0x1U  << 19)    /* RW */
#define SDC_CFG_SDIOIDE         (0x1U  << 20)    /* RW */      
#define SDC_CFG_INTATGAP        (0x1U  << 21)    /* RW */
#define SDC_CFG_DTOC            (0xffU << 24)  /* RW */

/* SDC_CMD mask */
#define SDC_CMD_OPC             (0x3fU << 0)     /* RW */
#define SDC_CMD_BRK             (0x1U  << 6)     /* RW */
#define SDC_CMD_RSPTYP          (0x7U  << 7)     /* RW */
#define SDC_CMD_DTYP            (0x3U  << 11)    /* RW */
#define SDC_CMD_DTYP            (0x3U  << 11)    /* RW */
#define SDC_CMD_RW              (0x1U  << 13)    /* RW */
#define SDC_CMD_STOP            (0x1U  << 14)    /* RW */
#define SDC_CMD_GOIRQ           (0x1U  << 15)    /* RW */
#define SDC_CMD_BLKLEN          (0xfffU<< 16)    /* RW */
#define SDC_CMD_AUTOCMD         (0x3U  << 28)    /* RW */
#define SDC_CMD_VOLSWTH         (0x1U  << 30)    /* RW */

/* SDC_STS mask */
#define SDC_STS_SDCBUSY         (0x1U  << 0)     /* RW */
#define SDC_STS_CMDBUSY         (0x1U  << 1)     /* RW */
#define SDC_STS_SWR_COMPL       (0x1U  << 31)    /* RW */

/* SDC_DCRC_STS mask */
#define SDC_DCRC_STS_NEG        (0xffU << 8)     /* RO */
#define SDC_DCRC_STS_POS        (0xffU << 0)     /* RO */

/* EMMC_CFG0 mask */
#define EMMC_CFG0_BOOTSTART     (0x1U  << 0)     /* W */
#define EMMC_CFG0_BOOTSTOP      (0x1U  << 1)     /* W */
#define EMMC_CFG0_BOOTMODE      (0x1U  << 2)     /* RW */
#define EMMC_CFG0_BOOTACKDIS    (0x1U  << 3)     /* RW */
#define EMMC_CFG0_BOOTWDLY      (0x7U  << 12)    /* RW */
#define EMMC_CFG0_BOOTSUPP      (0x1U  << 15)    /* RW */

/* EMMC_CFG1 mask */
#define EMMC_CFG1_BOOTDATTMC    (0xfffffU << 0)  /* RW */
#define EMMC_CFG1_BOOTACKTMC    (0xfffU << 20) /* RW */

/* EMMC_STS mask */
#define EMMC_STS_BOOTCRCERR     (0x1U  << 0)     /* W1C */
#define EMMC_STS_BOOTACKERR     (0x1U  << 1)     /* W1C */
#define EMMC_STS_BOOTDATTMO     (0x1U  << 2)     /* W1C */
#define EMMC_STS_BOOTACKTMO     (0x1U  << 3)     /* W1C */
#define EMMC_STS_BOOTUPSTATE    (0x1U  << 4)     /* R */
#define EMMC_STS_BOOTACKRCV     (0x1U  << 5)     /* W1C */
#define EMMC_STS_BOOTDATRCV     (0x1U  << 6)     /* R */

/* EMMC_IOCON mask */
#define EMMC_IOCON_BOOTRST      (0x1U  << 0)     /* RW */

/* SDC_ACMD19_TRG mask */
#define SDC_ACMD19_TRG_TUNESEL  (0xfU  << 0)     /* RW */

/* MSDC_DMA_CTRL mask */
#define MSDC_DMA_CTRL_START     (0x1U  << 0)     /* W */
#define MSDC_DMA_CTRL_STOP      (0x1U  << 1)     /* W */
#define MSDC_DMA_CTRL_RESUME    (0x1U  << 2)     /* W */
#define MSDC_DMA_CTRL_MODE      (0x1U  << 8)     /* RW */
#define MSDC_DMA_CTRL_LASTBUF   (0x1U  << 10)    /* RW */
#define MSDC_DMA_CTRL_BRUSTSZ   (0x7U  << 12)    /* RW */
#define MSDC_DMA_CTRL_XFERSZ    (0xffffU << 16)/* RW */

/* MSDC_DMA_CFG mask */
#define MSDC_DMA_CFG_STS        (0x1U  << 0)     /* R */
#define MSDC_DMA_CFG_DECSEN     (0x1U  << 1)     /* RW */
#define MSDC_DMA_CFG_BDCSERR    (0x1U  << 4)     /* R */
#define MSDC_DMA_CFG_GPDCSERR   (0x1U  << 5)     /* R */

/* MSDC_PATCH_BIT0 mask */
#define MSDC_PATCH_BIT_ODDSUPP    (0x1U  <<  1)     /* RW */

#ifdef MTK_SDIO30_ONLINE_TUNING_SUPPORT
#define MSDC_MASK_ACMD53_CRC_ERR_INTR   (0x1U << 4)
#define MSDC_ACMD53_FAIL_ONE_SHOT       (0X1U << 5)
#endif	// MTK_SDIO30_ONLINE_TUNING_SUPPORT

#define MSDC_INT_DAT_LATCH_CK_SEL (0x7U  <<  7)
#define MSDC_CKGEN_MSDC_DLY_SEL   (0x1FU << 10)
#define MSDC_PATCH_BIT_IODSSEL    (0x1U  << 16)    /* RW */
#define MSDC_PATCH_BIT_IOINTSEL   (0x1U  << 17)    /* RW */
#define MSDC_PATCH_BIT_BUSYDLY    (0xfU  << 18)    /* RW */
#define MSDC_PATCH_BIT_WDOD       (0xfU  << 22)    /* RW */
#define MSDC_PATCH_BIT_IDRTSEL    (0x1U  << 26)    /* RW */
#define MSDC_PATCH_BIT_CMDFSEL    (0x1U  << 27)    /* RW */
#define MSDC_PATCH_BIT_INTDLSEL   (0x1U  << 28)    /* RW */
#define MSDC_PATCH_BIT_SPCPUSH    (0x1U  << 29)    /* RW */
#define MSDC_PATCH_BIT_DECRCTMO   (0x1U  << 30)    /* RW */

/* MSDC_PATCH_BIT1 mask */
#define MSDC_PATCH_BIT1_WRDAT_CRCS			(0x7U << 0)
#define MSDC_PATCH_BIT1_CMD_RSP     		(0x7U << 3)
#define MSDC_PATCH_BIT1_GET_BUSY_MARGIN    	(0x1U << 6)    /* for write: 3T need wait before host check busy after crc status */
#define MSDC_PATCH_BIT1_GET_CRC_MARGIN     	(0x1U << 7)    /* for write: host check timeout change to 16T */


/* MSDC_PAD_TUNE mask */
#define MSDC_PAD_TUNE_DATWRDLY  (0x1FU << 0)     /* RW */
#define MSDC_PAD_TUNE_DATRRDLY  (0x1FU << 8)     /* RW */
#define MSDC_PAD_TUNE_CMDRDLY   (0x1FU << 16)    /* RW */
#define MSDC_PAD_TUNE_CMDRRDLY  (0x1FU << 22)  /* RW */
#define MSDC_PAD_TUNE_CLKTXDLY  (0x1FU << 27)  /* RW */

/* MSDC_DAT_RDDLY0/1 mask */
#define MSDC_DAT_RDDLY0_D3      (0x1FU << 0)     /* RW */
#define MSDC_DAT_RDDLY0_D2      (0x1FU << 8)     /* RW */
#define MSDC_DAT_RDDLY0_D1      (0x1FU << 16)    /* RW */
#define MSDC_DAT_RDDLY0_D0      (0x1FU << 24)    /* RW */

#define MSDC_DAT_RDDLY1_D7      (0x1FU << 0)     /* RW */
#define MSDC_DAT_RDDLY1_D6      (0x1FU << 8)     /* RW */
#define MSDC_DAT_RDDLY1_D5      (0x1FU << 16)    /* RW */
#define MSDC_DAT_RDDLY1_D4      (0x1FU << 24)    /* RW */

#define CARD_READY_FOR_DATA             (1U << 8)
#define CARD_CURRENT_STATE(x)           (((x) & 0x00001E00U)>>9)

//======================================================================================================
#define IO_BASE						(0xFD000000UL)

// AC8317 Clock Source 
#define MSDC0_CLK_SEL				(IO_BASE + 0x14UL) // SD01_AP_SEL [15:12]
#define MSDC1_CLK_SEL				(IO_BASE + 0x14UL) // SD11_AP_SEL [19:16]
#define MSDC2_CLK_SEL				(IO_BASE + 0x08UL) // SD21_AP_SEL [27:24]

#define MSDC0_CLK_SEL_OFFSET		(12) // SD01_AP_SEL [15:12]
#define MSDC1_CLK_SEL_OFFSET		(16) // SD11_AP_SEL [19:16]
#define MSDC2_CLK_SEL_OFFSET		(24) // SD21_AP_SEL [27:24]

#define MSDC_CLK_SEL_MASK			(0x0FU)

#define MSDC_CLK_SEL_27MHZ			(0x00)
#define MSDC_CLK_SEL_MSDCPLL_D2		(0x01)
#define MSDC_CLK_SEL_ARMPLL2_D2		(0x02)
#define MSDC_CLK_SEL_SYSPLL_D4		(0x03)
#define MSDC_CLK_SEL_USBPLL_D4		(0x04)
#define MSDC_CLK_SEL_SYSPLL_D6		(0x05)
#define MSDC_CLK_SEL_SYSPLL_D12		(0x06)
#define MSDC_CLK_SEL_USBPLL_D10		(0x07)
#define MSDC_CLK_SEL_DMPLL_D2		(0x08)
#define MSDC_CLK_SEL_APLL2_D2		(0x09)
#define MSDC_CLK_SEL_APLL2_D3		(0x0A)
#define MSDC_CLK_SEL_APLL1_D2		(0x0B)
#define MSDC_CLK_SEL_MSDCPLL_D3		(0x0C)
#define MSDC_CLK_SEL_MSDCPLL_D4		(0x0D)

#define MSDC_SELECT_CLK_SRC(clk_src) 				\
	do {    										\
		u32 clk_reg = 0;							\
		u32 bit_offset = 0;							\
		if (host->id == 0){							\
			bit_offset = MSDC0_CLK_SEL_OFFSET;		\
			clk_reg = MSDC0_CLK_SEL; }				\
		else if (host->id == 1){					\
			bit_offset = MSDC1_CLK_SEL_OFFSET;		\
			clk_reg = MSDC1_CLK_SEL; }				\
		else if (host->id == 2){					\
			bit_offset = MSDC2_CLK_SEL_OFFSET;		\
			clk_reg = MSDC2_CLK_SEL; }				\
		if (clk_reg)								\
			MSDC_SET_FIELD(clk_reg, (MSDC_CLK_SEL_MASK << bit_offset), (clk_src)); \
	} while(0)


// AC8317 HClock Source, for module internal use
#define MSDC_HCLK_SEL				(IO_BASE + 0x14UL) // SD20_AP_SEL [11:9], SD10_AP_SEL [8:6], SD00_AP_SEL [5:3]
	
#define MSDC0_HCLK_SEL_OFFSET		(3) // SD00_AP_SEL [5:3]
#define MSDC1_HCLK_SEL_OFFSET		(6) // SD10_AP_SEL [8:6]
#define MSDC2_HCLK_SEL_OFFSET		(9) // SD20_AP_SEL [11:9]
	
#define MSDC_HCLK_SEL_MASK			(0x07U)
	
#define MSDC_HCLK_SEL_27MHZ			(0x00)
#define MSDC_HCLK_SEL_APLL2_D3		(0x01)
#define MSDC_HCLK_SEL_USBPLL_D6		(0x02)
#define MSDC_HCLK_SEL_SYSPLL_D9		(0x03)
#define MSDC_HCLK_SEL_USBPLL_D8		(0x04)
#define MSDC_HCLK_SEL_SYSPLL_D12	(0x05)
#define MSDC_HCLK_SEL_USBPLL_D10	(0x06)
#define MSDC_HCLK_SEL_SYSPLL_D18	(0x07)
	
#define MSDC_SELECT_HCLK_SRC(clk_src) 				\
	do {											\
		u32 bit_offset = 0;							\
		if (host->id == 0){ 						\
			bit_offset = MSDC0_HCLK_SEL_OFFSET; }	\
		else if (host->id == 1){					\
			bit_offset = MSDC1_HCLK_SEL_OFFSET; }	\
		else if (host->id == 2){					\
			bit_offset = MSDC2_HCLK_SEL_OFFSET;	}	\
		MSDC_SET_FIELD(MSDC_HCLK_SEL, (MSDC_HCLK_SEL_MASK << bit_offset), (clk_src));	\
	} while(0)


// AC8317 EINT Setting, for module internal use
#define MSDC0_EINT4_CFG				(IO_BASE + 0x8754UL)
#define MSDC1_EINT5_CFG				(IO_BASE + 0x8758UL)
#define MSDC2_EINT6_CFG				(IO_BASE + 0x875CUL)

#define MSDC_EINT_EN_MASK			(0x01U << 13)
#define MSDC_EINT_EN_DISABLE		(0x00)
#define MSDC_EINT_EN_ENABLE			(0x01)

#define MSDC_EINT_TYPE_MASK			(0x07U << 10)
#define MSDC_EINT_TYPE_POS_EDGE		(0x00)
#define MSDC_EINT_TYPE_NEG_EDGE		(0x01)
#define MSDC_EINT_TYPE_HIGH_LEVEL	(0x02)
#define MSDC_EINT_TYPE_LOW_LEVEL	(0x03)
#define MSDC_EINT_TYPE_DUAL_EDGE	(0x04)



#define MSDC_CONFIG_CD_EINT(host, en, type)		\
	do {										\
		u32 eint_reg = 0;						\
		if ((host)->id == 0){ 					\
			eint_reg = MSDC0_EINT4_CFG; }		\
		else if ((host)->id == 1){				\
			eint_reg = MSDC1_EINT5_CFG; }		\
		else if ((host)->id == 2){				\
			eint_reg = MSDC2_EINT6_CFG;	}		\
		MSDC_SET_FIELD(eint_reg, MSDC_EINT_EN_MASK, (en)); 	\
		MSDC_SET_FIELD(eint_reg, MSDC_EINT_TYPE_MASK, (type)); \
	} while(0)


// Clock gate register
#define MSDC_CKEN_GATE_REG			(IO_BASE + 0x000000A8UL)
	#define MSDC0_CKEN_GATE				(0x01U << 16)
	#define MSDC1_CKEN_GATE				(0x01U << 17)
	#define MSDC2_CKEN_GATE				(0x01U << 18)
	#define GATE_ENABLE_CLOCK				(1)
	#define GATE_DISABLE_CLOCK				(0)

#define MSDC_CLOCK_GATE(host, enable)			\
	do {										\
		u32 bit_offset = 0;						\
		if ((host)->id == 0){ 					\
			bit_offset = MSDC0_CKEN_GATE; }		\
		else if ((host)->id == 1){				\
			bit_offset = MSDC1_CKEN_GATE; }		\
		else if ((host)->id == 2){				\
			bit_offset = MSDC2_CKEN_GATE; }		\
		MSDC_SET_FIELD(MSDC_CKEN_GATE_REG, bit_offset, (enable)); 	\
	} while(0)


//************************************************************//
//
// MSDC Reset Setting
//
//************************************************************//
// Register
#define MSDC_RESET_REG				(IO_BASE + 0x00C4UL)
		
// SW Reset bit offset
#define MSDC0_MODULE_SW_RESET			(1U << 19) 
#define MSDC1_MODULE_SW_RESET			(1U << 20)
#define MSDC2_MODULE_SW_RESET			(1U << 21)
		
// HW Reset bit offset
#define MSDC0_MODULE_HW_RESET			(1U << 16) 
#define MSDC1_MODULE_HW_RESET			(1U << 17)
#define MSDC2_MODULE_HW_RESET			(1U << 18)
		
// Value
#define MSDC_MODULE_RESET_ENABLE		(0)
#define MSDC_MODULE_RESET_DISABLE		(1)
		
#define MSDC_MOD_SW_RESET(host, val)	 				\
		do {											\
			u32 bit_offset = 0; 						\
			BUG_ON((host)->id > 2UL);						\
			if (host->id == 0)							\
				bit_offset = MSDC0_MODULE_SW_RESET; 	\
			else if ((host)->id == 1) 					\
				bit_offset = MSDC1_MODULE_SW_RESET; 	\
			else if ((host)->id == 2)		 				\
				bit_offset = MSDC2_MODULE_SW_RESET; 	\
			MSDC_SET_FIELD(MSDC_RESET_REG, bit_offset, (val));\
		} while(0)
		
#define MSDC_MOD_HW_RESET(host, val)	 				\
		do {											\
			u32 bit_offset = 0; 						\
			BUG_ON((host)->id > 2UL);						\
			if ((host)->id == 0)							\
				bit_offset = MSDC0_MODULE_HW_RESET; 	\
			else if ((host)->id == 1)		 				\
				bit_offset = MSDC1_MODULE_HW_RESET; 	\
			else if ((host)->id == 2)		 				\
				bit_offset = MSDC2_MODULE_HW_RESET; 	\
			MSDC_SET_FIELD(MSDC_RESET_REG, bit_offset, (val));\
		} while(0)
		
#define MSDC_MODULE_SW_RESET(host)		 							\
		do {														\
			MSDC_MOD_SW_RESET(host, MSDC_MODULE_RESET_ENABLE);		\
			mdelay(1);												\
			MSDC_MOD_SW_RESET(host, MSDC_MODULE_RESET_DISABLE);		\
		} while(0)

#define MSDC_MODULE_HW_RESET(host)		 							\
		do {														\
			MSDC_MOD_HW_RESET(host, MSDC_MODULE_RESET_ENABLE);		\
			mdelay(1); 												\
			MSDC_MOD_HW_RESET(host, MSDC_MODULE_RESET_DISABLE);		\
		} while(0)

//************************************************************//
//
//  PAD Setting Config
//
//************************************************************//

// For 4bit mode, only use for nand bootup
#define PAD_CFG_SD0_4BIT_CLK		(IO_BASE + 0x000002C0UL)
#define PAD_CFG_SD0_4BIT_CMD		(IO_BASE + 0x000002C4UL)
#define PAD_CFG_SD0_4BIT_DAT0		(IO_BASE + 0x000002C8UL)
#define PAD_CFG_SD0_4BIT_DAT1		(IO_BASE + 0x000002CCUL)
#define PAD_CFG_SD0_4BIT_DAT2		(IO_BASE + 0x000002D0UL)
#define PAD_CFG_SD0_4BIT_DAT3		(IO_BASE + 0x000002D4UL)
#define PAD_CFG_SD0_4BIT_RST		(IO_BASE + 0x00000334UL)


#define PAD_CFG_SD1_CLK				(IO_BASE + 0x000002D8UL)
#define PAD_CFG_SD1_CMD				(IO_BASE + 0x000002DCUL)
#define PAD_CFG_SD1_DAT0			(IO_BASE + 0x000002E0UL)
#define PAD_CFG_SD1_DAT1			(IO_BASE + 0x000002E4UL)
#define PAD_CFG_SD1_DAT2			(IO_BASE + 0x000002E8UL)
#define PAD_CFG_SD1_DAT3			(IO_BASE + 0x000002ECUL)
#define PAD_CFG_SD1_RST				(IO_BASE + 0x0000033CUL)


#define PAD_CFG_SD2_CLK				(IO_BASE + 0x000002F0UL)
#define PAD_CFG_SD2_CMD				(IO_BASE + 0x000002F4UL)
#define PAD_CFG_SD2_DAT0			(IO_BASE + 0x000002F8UL)
#define PAD_CFG_SD2_DAT1			(IO_BASE + 0x000002FCUL)
#define PAD_CFG_SD2_DAT2			(IO_BASE + 0x00000300UL)
#define PAD_CFG_SD2_DAT3			(IO_BASE + 0x00000304UL)
#define PAD_CFG_SD2_RST				(IO_BASE + 0x00000340UL)


#define PAD_CFG_SD0_CLK				(IO_BASE + 0x0000030CUL)
#define PAD_CFG_SD0_CMD				(IO_BASE + 0x00000310UL)
#define PAD_CFG_SD0_DAT0			(IO_BASE + 0x00000314UL)
#define PAD_CFG_SD0_DAT1			(IO_BASE + 0x00000318UL)
#define PAD_CFG_SD0_DAT2			(IO_BASE + 0x0000031CUL)
#define PAD_CFG_SD0_DAT3			(IO_BASE + 0x00000320UL)
#define PAD_CFG_SD0_DAT4			(IO_BASE + 0x00000324UL)
#define PAD_CFG_SD0_DAT5			(IO_BASE + 0x00000328UL)
#define PAD_CFG_SD0_DAT6			(IO_BASE + 0x0000032CUL)
#define PAD_CFG_SD0_DAT7			(IO_BASE + 0x00000330UL)
#define PAD_CFG_SD0_RST				(IO_BASE + 0x00000338UL)

// Bit mask and bit offset
#define PAD_CFG_TDSEL_MASK			(0x0FU << 23)
#define PAD_CFG_RDSEL_MASK			(0xFFU << 15)
#define PAD_CFG_SMT_MASK			(0x01U << 14)
#define PAD_CFG_RESISTOR_MASK		(0x03U << 12)
#define PAD_CFG_PUPD_MASK			(0x01U << 11)
#define PAD_CFG_IES_MASK			(0x01U << 10)
#define PAD_CFG_DRV_MASK			(0x3FU << 4)
#define PAD_CFG_SR_MASK				(0x0FU << 0)


/* pad_msdc_cfg36 0x00000350*/
#define MSDC_PAD_RST_RXDLY				(IO_BASE + 0x00000350UL)
	#define SD1_DATA_PINS_AS_SD0_HIGH_4DATA		(0x01U << 0)  /* SD1's data used for SD0 high 4bit data*/
	#define MSDC_PAD_RXDLY_P0_4BIT_RST_MASK		(0x1FU << 4)
	#define MSDC_PAD_RXDLY_P0_RST_MASK			(0x1FU << 9)
	#define MSDC_PAD_RXDLY_P1_RST_MASK			(0x1FU << 14)
	#define MSDC_PAD_RXDLY_P2_RST_MASK			(0x1FU << 19)


// Pad MSDC Function Select
#define MSDC_PAD_FUNC_SELECT				(IO_BASE + 0x00000308UL)
	#define MSDC_PAD_FUNC_SD2_RST_GPIO_CTL			(0x01U << 31)
	#define MSDC_PAD_FUNC_SD1_RST_GPIO_CTL			(0x01U << 30)
	#define MSDC_PAD_FUNC_SD0_8BIT_RST_GPIO_CTL		(0x01U << 29)
	#define MSDC_PAD_FUNC_SD0_RST_GPIO_CTL			(0x01U << 28)
	#define MSDC_PAD_FUNC_SD0_8BIT_GPIO_CTL			(0x3FFU << 18)
	#define MSDC_PAD_FUNC_SD2_GPIO_CTL				(0x3FU << 12)
	#define MSDC_PAD_FUNC_SD1_GPIO_CTL				(0x3FU << 6)
	#define MSDC_PAD_FUNC_SD0_GPIO_CTL				(0x3FU << 0)

#define MSDC_PAD_MISC_CTL					(IO_BASE + 0x0000094UL)
	#define MSDC_PAD_MISC_SD2_DRAM_AGENT		(0x01U << 14)
	#define MSDC_PAD_MISC_SD1_DRAM_AGENT		(0x01U << 13)
	#define MSDC_PAD_MISC_SD0_DRAM_AGENT		(0x01U << 12)

// For switch IO Voltage between 3.3V and 1.8V
#define MSDC_SW_GPIO_ENABLE_OUTPUT  		(IO_BASE + 0x00000080UL)  //gpio0
	#define SD_V33_18_SW0_ENABLE  				(0x01U << 18)
	#define SD_V33_18_SW1_ENABLE  				(0x01U << 19)
	#define SD_V33_18_SW2_ENABLE  				(0x01U << 20)
	
#define MSDC_SW_GPIO_OUTPUT_VALUE 			(IO_BASE + 0x000000ECUL)
	#define SD_V33_18_SW0_VALUE  				(0x01U << 18)
	#define SD_V33_18_SW1_VALUE  				(0x01U << 19)
	#define SD_V33_18_SW2_VALUE  				(0x01U << 20)

//======================================================================================================


// Max Xia, Add RISCSZ for PIO mode
#define MSDC_IOCON_RISCSZ_1BYTE		(0)
#define MSDC_IOCON_RISCSZ_2BYTE		(1)
#define MSDC_IOCON_RISCSZ_4BYTE		(2)

/*--------------------------------------------------------------------------*/
/* Descriptor DMA Transfer Structure                                                     */
/*--------------------------------------------------------------------------*/
typedef struct {
    u32  hwo:1; /* could be changed by hw */
    u32  bdp:1;
    u32  rsv0:6;
    u32  chksum:8;
    u32  intr:1;
    u32  rsv1:15;
    void *next;
    void *ptr;
    u32  buflen:16;
    u32  extlen:8;
    u32  rsv2:8;
    u32  arg;
    u32  blknum;
    u32  cmd;
} gpd_t;

typedef struct {
    u32  eol:1;
    u32  rsv0:7;
    u32  chksum:8;
    u32  rsv1:1;
    u32  blkpad:1;
    u32  dwpad:1;
    u32  rsv2:13;
    void *next;
    void *ptr;
    u32  buflen:16;
    u32  rsv3:16;
} bd_t;

/*--------------------------------------------------------------------------*/
/* Tuning Releated Structure                                              */
/*--------------------------------------------------------------------------*/
typedef enum{
	cmd_counter = 0,
	read_counter,
	write_counter,
	all_counter,
}TUNE_COUNTER;

/*--------------------------------------------------------------------------*/
/* Register Debugging Structure                                             */
/*--------------------------------------------------------------------------*/

#define DMA_FLAG_NONE       (0x00000000)
#define DMA_FLAG_EN_CHKSUM  (0x00000001)
#define DMA_FLAG_PAD_BLOCK  (0x00000002)
#define DMA_FLAG_PAD_DWORD  (0x00000004)

struct msdc_dma {
    u32 flags;                   /* flags */
    u32 xfersz;                  /* xfer size in bytes */
    u32 sglen;                   /* size of scatter list */
    u32 blklen;                  /* block size */
    struct scatterlist *sg;      /* I/O scatter list */
    u8  mode;                    /* dma mode        */
    u8  burstsz;                 /* burst size      */
    u8  intr;                    /* dma done interrupt */
    u8  padding;                 /* padding */
    u32 cmd;                     /* enhanced mode command */
    u32 arg;                     /* enhanced mode arg */
    u32 rsp;                     /* enhanced mode command response */
    u32 autorsp;                 /* auto command response */

    gpd_t *gpd;                  /* pointer to gpd array */
    bd_t  *bd;                   /* pointer to bd array */
    dma_addr_t gpd_addr;         /* the physical address of gpd array */
    dma_addr_t bd_addr;          /* the physical address of bd array */
    u32 used_gpd;                /* the number of used gpd elements */
    u32 used_bd;                 /* the number of used bd elements */
};

struct tune_counter
{
	u32 time_cmd;
	u32 time_read;
	u32 time_write;
};

// Struct for suspend/resume.
struct msdc_saved_para
{
	u32							pad_tune;
	u32							ddly0;
	u32							ddly1;
	u8							cmd_resp_ta_cntr;
	u8							wrdat_crc_ta_cntr;
	u8							suspend_flag;
	u32 						msdc_cfg;
	u32 						mode;
	u32 						div;
	u32 						sdc_cfg;
	u32 						iocon;
	u32 						patch_bit0;
	u32 						patch_bit1;
	int             			ddr;
	u32             			hz;
	u8							int_dat_latch_ck_sel;
	u8							ckgen_msdc_dly_sel;
	u8							inten_sdio_irq;
};

#ifdef MTK_SDIO30_ONLINE_TUNING_SUPPORT
struct ot_data
{
    u32 eco_ver;
    u32 orig_blknum;
    u32 orig_patch_bit0;
    u32 orig_iocon;

#define DMA_ON 0
#define DMA_OFF 1
    u32 orig_dma;
    u32 orig_cmdrdly;
    u32 orig_dat0rddly;
    u32 orig_dat1rddly;
    u32 orig_dat2rddly;
    u32 orig_dat3rddly;
    u32 orig_dtoc;

    u32 cmdrdly;
    u32 dat0rddly;
    u32 dat1rddly;
    u32 dat2rddly;
    u32 dat3rddly;

    u32 cmddlypass;
    u32 dat0rddlypass;
    u32 dat1rddlypass;
    u32 dat2rddlypass;
    u32 dat3rddlypass;

    u32 fCmdTestedGear;
    u32 fDat0TestedGear;
    u32 fDat1TestedGear;
    u32 fDat2TestedGear;
    u32 fDat3TestedGear;

    u32 rawcmd;
    u32 rawarg;
    u32 tune_wind_size;
    u32 fn;
    u32 addr;
	u32 retry;
};

struct ot_work_t
{
	struct delayed_work ot_delayed_work;
	struct msdc_host *host;
};
#endif // MTK_SDIO30_ONLINE_TUNING_SUPPORT

struct msdc_host
{
    struct msdc_hw              *hw;

    struct mmc_host             *mmc;           /* mmc structure */
    struct mmc_command          *cmd;
    struct mmc_data             *data;
    struct mmc_request          *mrq; 
    int                         cmd_rsp;
    int                         cmd_rsp_done;
    int                         cmd_r1b_done;

    int                         error; 
    spinlock_t                  lock;           /* mutex */
	spinlock_t                  remove_bad_card;	/*to solve removing bad card race condition with hot-plug enable*/
    struct semaphore            sem; 

    u32                         blksz;          /* host block size */
    u32                         base;           /* host base address */    
    int                         id;             /* host id */
    int                         pwr_ref;        /* core power reference count */

    u32                         xfer_size;      /* total transferred size */

    struct msdc_dma             dma;            /* dma channel */
    u32                         dma_addr;       /* dma transfer address */
    u32                         dma_left_size;  /* dma transfer left size */
    u32                         dma_xfer_size;  /* dma transfer size in bytes */
    int                         dma_xfer;       /* dma transfer mode */

    u32                         timeout_ns;     /* data timeout ns */
    u32                         timeout_clks;   /* data timeout clks */

    atomic_t                    abort;          /* abort transfer */

    int                         irq;            /* host interrupt */
	int							cd_gpio;		/* card detect gpio*/
	int							cd_irq;			/* card detect interrupt */

	int 						vol_sw_gpio;	/* 3.3v/1.8V switch gpio*/

    struct tasklet_struct       card_tasklet;
    struct tasklet_struct       emmc_protected;
	u32                         protected;
	u32                         protect_init;
	u32                         protect_gpio;
	//struct delayed_work       	remove_card;
#ifdef MTK_SDIO30_ONLINE_TUNING_SUPPORT
	int	                        pre_temper;	    /* previous set temperature */
	struct workqueue_struct     *ot_wq;         /* online tuning work queue */
	struct ot_work_t            ot_work;
	bool                        ot_work_in_wq;
#endif // MTK_SDIO30_ONLINE_TUNING_SUPPORT

	struct completion           cmd_done;
	struct completion           xfer_done;
	struct pm_message           pm_state;

	u8 							timing;			/* timing specification used */
    u32                         mclk;           /* mclk: the request clock of mmc sub-system */
    u32                         hclk;           /* hclk: clock of clock source to msdc controller */		
    u32                         sclk;           /* sclk: the really clock after divition */
    u8                          power_mode;     /* host power mode */
    u8                          card_inserted;  /* card inserted ? */
    u8                          suspend;        /* host suspended ? */    
    u8                          reserved;
    u8                          app_cmd;        /* for app command */     
    u32                         app_cmd_arg;    
    u64                         starttime;
    //struct timer_list           timer;     
    struct tune_counter         t_counter;
	u32							rwcmd_time_tune;
	int							read_time_tune;
    int                         write_time_tune;
	u32							write_timeout_uhs104;
	u32							read_timeout_uhs104;
	u32							write_timeout_emmc;
	u32							read_timeout_emmc;
	u32							rwcmd_timeout_emmc;
	u8						    autocmd;
	u32							sw_timeout;
	u32							power_cycle; /* power cycle done in tuning flow*/
	bool						power_cycle_enable;/*Enable power cycle*/
	u32							sd_30_busy;
	bool						tune;		/* tuning status, 1 means in tuning process*/
	bool						ddr;		/* ddr mode status, 1 means in ddr mode */
	struct msdc_saved_para		saved_para;	
	int 						sd_cd_polarity;
	int							sd_cd_insert_work; //to make sure insert mmc_rescan this work in start_host when boot up
												   //driver will get a EINT(Level sensitive) when boot up phone with card insert
	bool						block_bad_card;											   
#ifdef SDIO_ERROR_BYPASS      
    int                         sdio_error;     /* sdio error can't recovery */
#endif									  

	int							card_status;
	int							old_status;
	int							init_card_status;
	#define MSDC_INIT_RETRY_TIMES		(8)
	int							init_retry_times;
	#define SD_VERSION_RETRY_TIMES		(2)
	u32							sd_version_retry;
	
	spinlock_t 					detect_queue_lock;
	// current option idle
	int 						queue_len;

	int							sdio_wifi_type; /* -1-not set, 0-on board wifi, 1-removable wifi */
	u32 		hs200_cmd_int_delay;
	u32 		hs200_write_int_delay;

	/* save last data transfer error information */
	u32		last_dat_err_cmd_opcode;
	u32		last_dat_err_cmd_arg;
	u32		last_dat_err_intr;

	/* save last cmd transfer error information */
	u32		last_cmd_err_cmd_opcode;
	u32		last_cmd_err_cmd_arg;
	u32		last_cmd_err_intr;
	u32		last_cmd_err_retry_cnt;
	struct clk *clk_source;
	struct clk *h_clk;
	struct clk *clk_gate;
	void	(*power_control)(struct msdc_host *host,u32 on);
	void	(*power_switch)(struct msdc_host *host,u32 on);
};


typedef enum {
   TRAN_MOD_PIO,
   TRAN_MOD_DMA,
   TRAN_MOD_NUM
}transfer_mode;

typedef enum {
   OPER_TYPE_READ,
   OPER_TYPE_WRITE,
   OPER_TYPE_NUM
}operation_type;

struct dma_addr{
   u32 start_address;
   u32 size;
   u8 end; 
   struct dma_addr *next;
};

static inline unsigned int uffs(unsigned int x)
{
    unsigned int r = 1U;

    if (!x)
        return 0;
    if (!(x & 0xffffU)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xffU)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xfU)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3U)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1U)) {
        x >>= 1;
        r += 1;
    }
    return r;
}

#if 1
#define MSDC_READ8(reg)				__raw_readb((const volatile void *)reg)
#define MSDC_READ16(reg)			__raw_readw((const volatile void *)reg)
#define MSDC_READ32(reg)			__raw_readl((const volatile void *)reg)
#else
#define MSDC_READ8(reg)				__raw_readb((unsigned int)reg)
#define MSDC_READ16(reg)			__raw_readw((unsigned int)reg)
#define MSDC_READ32(reg)			__raw_readl((unsigned int)reg)
#endif

#define MSDC_WRITE8(reg, val)		ac83xx_reg_sync_writeb(val, reg)
#define MSDC_WRITE16(reg, val)		ac83xx_reg_sync_writew(val, reg)
#define MSDC_WRITE32(reg, val)		ac83xx_reg_sync_writel(val, reg)

#define MSDC_SET_BITS(reg, bs) \
	do{\
		volatile unsigned int tv = MSDC_READ32(reg);\
		tv |= (u32)(bs); \
		MSDC_WRITE32(reg, tv); \
	}while(0)

#define MSDC_CLR_BITS(reg, bs) \
do{\
		volatile unsigned int tv = MSDC_READ32(reg);\
		tv &= ~((u32)(bs)); \
		MSDC_WRITE32(reg, tv); \
	}while(0)

#define MSDC_SET_FIELD(reg, field, val) \
		do {	\
			volatile unsigned int tv = MSDC_READ32(reg); \
			tv &= ~(field); \
			tv |= ((val) << (uffs((unsigned int)(field)) - 1)); \
			MSDC_WRITE32(reg, tv); \
		} while(0)
		
#define MSDC_GET_FIELD(reg, field, val) \
		do {	\
			volatile unsigned int tv = MSDC_READ32(reg); \
			val = ((tv & (field)) >> (uffs((unsigned int)(field)) - 1)); \
		} while(0)


/******************************************************************/
// MSDC General Function Macro Definitions
/******************************************************************/
#define DRV_NAME            		"atc-msdc"
#define IO_PHYS_TO_VIRT(x)			((x) + 0x0D000000UL)

#define MSDC_TXFIFO_CNT()   		((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_TXCNT) >> 16)
#define MSDC_RXFIFO_CNT()   		((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_RXCNT) >> 0)
#define MSDC_FIFO_WRITE32(v)   		MSDC_WRITE32(MSDC_TXDATA, (v))
#define MSDC_FIFO_WRITE8(v)    		MSDC_WRITE8(MSDC_TXDATA, (v))
#define MSDC_FIFO_READ32()   		MSDC_READ32(MSDC_RXDATA)
#define MSDC_FIFO_READ8()    		MSDC_READ8(MSDC_RXDATA)	

#define MSDC_DMA_ON()        		MSDC_CLR_BITS(MSDC_CFG, MSDC_CFG_PIO)
#define MSDC_DMA_OFF()       		MSDC_SET_BITS(MSDC_CFG, MSDC_CFG_PIO)
#define MSDC_DMA_STATUS()    		((MSDC_READ32(MSDC_CFG) & MSDC_CFG_PIO) >> 3)


/* For Inhanced DMA */
#define MSDC_INIT_GPD_EX(gpd, extlen, cmd, arg, blknum) \
    do { \
        ((gpd_t*)(gpd))->extlen = extlen; \
        ((gpd_t*)(gpd))->cmd    = cmd; \
        ((gpd_t*)(gpd))->arg    = arg; \
        ((gpd_t*)(gpd))->blknum = blknum; \
    }while(0)
    
#define MSDC_INIT_BD(bd, blkpad, dwpad, dptr, dlen) \
    do { \
        BUG_ON((dlen) > 0xFFFFUL); \
        ((bd_t*)(bd))->blkpad = blkpad; \
        ((bd_t*)(bd))->dwpad  = dwpad; \
        ((bd_t*)(bd))->ptr    = (void*)dptr; \
        ((bd_t*)(bd))->buflen = dlen; \
    }while(0)

#define MSDC_RETRY(expr, retry, cnt, id)				\
		do {											\
			int backup = (cnt);							\
			while (retry) {								\
				if ((expr) == 0) {							\
					break;								\
				}										\
				(cnt)--;									\
				if ((cnt) == 0) {							\
					(retry)--;							\
					mdelay(1);							\
					(cnt) = backup;						\
				}										\
			}											\
			if ((retry) == 0) {							\
				pr_err("[MSDC]-----> MSDC_RETRY failed <-----\n"); \
				msdc_dump_info(id);						\
			}											\
			WARN_ON((retry) == 0); \
		} while(0)

#define MSDC_RESET(id) \
		do { \
			int retry = 3;									\
			int cnt = 1000;									\
			MSDC_SET_BITS(MSDC_CFG, MSDC_CFG_RST);			\
			mb();											\
			MSDC_RETRY((MSDC_READ32(MSDC_CFG) & MSDC_CFG_RST), retry, cnt, (id)); \
		} while(0)
	
#define MSDC_CLR_INT() \
		do { \
			volatile u32 val = MSDC_READ32(MSDC_INT); \
			MSDC_WRITE32(MSDC_INT, val); \
		} while(0)
	
#define MSDC_CLR_FIFO(id) \
		do { \
			int retry = 3;											\
			int cnt = 1000;											\
			MSDC_SET_BITS(MSDC_FIFOCS, MSDC_FIFOCS_CLR);			\
			MSDC_RETRY((MSDC_READ32(MSDC_FIFOCS) & MSDC_FIFOCS_CLR), retry, cnt, (id)); \
		} while(0)
	
#define MSDC_RESET_HW(id) \
			MSDC_RESET(id); \
			MSDC_CLR_FIFO(id); \
			MSDC_CLR_INT(); 

#define msdc_irq_save(val) \
    do { \
        val = MSDC_READ32(MSDC_INTEN); \
        MSDC_CLR_BITS(MSDC_INTEN, (val)); \
    } while(0)
	
#define msdc_irq_restore(val) \
    do { \
        MSDC_SET_BITS(MSDC_INTEN, (val)); \
    } while(0)

#define SDC_IS_BUSY()          (MSDC_READ32(SDC_STS) & SDC_STS_SDCBUSY)
#define SDC_IS_CMD_BUSY()      (MSDC_READ32(SDC_STS) & SDC_STS_CMDBUSY)
	
#define SDC_SEND_CMD(cmd, arg) \
	do { \
		MSDC_WRITE32(SDC_ARG, (arg)); \
		mb(); \
		MSDC_WRITE32(SDC_CMD, (cmd)); \
	} while(0)

#define UNSTUFF_BITS(resp, start, size)          \
	({				  \
		const int __size = size;		\
		const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1;  \
		const int __off = 3 - ((start) / 32);	   \
		const int __shft = (start) & 31;	  \
		u32 __res;			  \
			  \
		__res = resp[__off] >> __shft;		  \
		if (__size + __shft > 32)		 \
			__res |= resp[__off-1] << ((32 - __shft) % 32);  \
		__res & __mask; \
	})

#define IS_CARD_PRESENT(host)     (((struct msdc_host*)(host))->card_inserted)
//#define IS_CARD_SDIO(host)        (((struct msdc_host*)(host))->hw->register_pm) // Double check!!!
#define IS_CARD_SDIO(host)        0


// Tuning status
#define IN_TUNE_PROCESS(host)				(((struct msdc_host*)(host))->tune == 1)
#define NOT_IN_TUNE_PROCESS(host)			(((struct msdc_host*)(host))->tune == 0)
#define SET_TUNE_STS(host, status)			(((struct msdc_host*)(host))->tune = status)
	#define TUNE_STS_IN_TUNE			(1)
	#define TUNE_STS_NOT_IN_TUNE		(0)

// Clock mode 
#define IS_SDR_DIV_MODE(clkmode)			((clkmode) == 0)	//(((MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKMOD) >> 16) == 0)
#define IS_SDR_DIRECT_MODE(clkmode)			((clkmode) == 1)	//(((MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKMOD) >> 16) == 1) 
#define IS_DDR_MODE(clkmode)				((clkmode) == 2)	//(((MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKMOD) >> 16) == 2)


/*--------------------------------------------------------------------------*/
/* MSDC Functions Select Macro Definition                                            */
/*--------------------------------------------------------------------------*/

// Tuning Method Select
//#define TUNE_NOT_TUNE					(2)		// Don't tune 
#define TUNE_EACH_DAT_LINE				(0)		// Each data line has its own delay
#define TUNE_ALL_DAT_LINE				(1)		// All data lines use same delay

#define TUNE_PARAMS_DETAILS				(0)



// Force reduce eMMC Work Clock
#define MSDC_FORCE_REDUCE_EMMC_CLOCK	(0)
#define MSDC_EMMC_WORK_CLOCK			(100UL * 1000UL * 1000UL)

// Host OPS Attributes
#define HOST_MAX_MCLK       (200000000) // 200MHz 
#define HOST_MIN_MCLK       (260000) 	// 260KHz

#define MSDC_OCR_AVAIL      (MMC_VDD_28_29 | MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33)
#define HOST_MAX_BLKSZ      (2048)

// Max Tuning times
#define CMD_TUNE_UHS_MAX_TIME 			(2*32*8*8)	// (MSDC_IOCON_RSPL * MSDC_PAD_TUNE_CMDRDLY * MSDC_PATCH_BIT1_CMD_RSP * MSDC_INT_DAT_LATCH_CK_SEL) 
#define CMD_TUNE_HS_MAX_TIME 			(2*32)		// (MSDC_IOCON_RSPL * MSDC_PAD_TUNE_CMDRDLY  ) 
#define CMD_TUNE_SD_MAX_TIME			(100)

#define READ_TUNE_UHS_CLKMOD1_MAX_TIME 		(2*32*32*8)	// (MSDC_IOCON_DSPL * MSDC_DAT_RDDLY0/1 * MSDC_CKGEN_MSDC_DLY_SEL * MSDC_INT_DAT_LATCH_CK_SEL) 
#define READ_TUNE_UHS_MAX_TIME 			(2*32*32)	// (MSDC_IOCON_DSPL * MSDC_DAT_RDDLY0/1 * MSDC_CKGEN_MSDC_DLY_SEL) 
#define READ_TUNE_HS_MAX_TIME 			(2*32)		// (MSDC_IOCON_DSPL * MSDC_DAT_RDDLY0/1) 

#define WRITE_TUNE_HS_MAX_TIME 			(2*32)		// (MSDC_IOCON_W_DSPL * MSDC_DAT_RDDLY0 ) 
#define WRITE_TUNE_UHS_MAX_TIME 		(2*32*8)	// (MSDC_IOCON_W_DSPL * MSDC_DAT_RDDLY0 * MSDC_PATCH_BIT1_WRDAT_CRCS) 

// For tuning all data lines with same delay cycle
#define READ_TUNE_MAX_TIME			(2*32*32*8)
#define READ_TUNE_DDR_MAX_TIME			(32*8)
#define WRITE_TUNE_MAX_TIME			(2*32*32*32*8)
#define WRITE_TUNE_DDR_MAX_TIME			(32*8)
#define CMD_TUNE_MAX_TIME			(2*32*32*32*8)

#define MAX_DTOC        	(0xFF)   	//Max timeout setting
#define DEFAULT_DTOC        (0x3)      		// data timeout counter. 65536x40(75/77) /1048576 * 3(83/85) sclk. 

#define CMD_TIMEOUT         (HZ/10 * 5)   	/* 100ms x5 */
#define DAT_TIMEOUT         (HZ    * 5)   	/* 1000ms x5 */
#define CLK_TIMEOUT         (HZ    * 5)  	/* 5s    */ 
#define POLLING_BUSY		(HZ	   * 3)
#define MAX_DMA_CNT         (64 * 1024 - 512)   /* a single transaction for WIFI may be 50K*/

#define MAX_HW_SGMTS        (MAX_BD_NUM)
#define MAX_PHY_SGMTS       (MAX_BD_NUM)
#define MAX_SGMT_SZ         (MAX_DMA_CNT)
#define MAX_REQ_SZ          (512*1024)  

/*================================= */
/* Reduce MSDC work clock after tuning parameters failed.*/
#define MSDC_LOWER_FREQ					(1)
#define MSDC_MAX_FREQ_DIV   			(2)	/* 200 / (4 * 2) */
#define MSDC_MAX_TIMEOUT_RETRY			(1)
#define MSDC_MAX_TIMEOUT_RETRY_EMMC		(2)
#define MSDC_MAX_W_TIMEOUT_TUNE			(5)
#define MSDC_MAX_W_TIMEOUT_TUNE_EMMC	(64)
#define MSDC_MAX_R_TIMEOUT_TUNE_EMMC	(64)
#define MSDC_MAX_R_TIMEOUT_TUNE			(3)
#define	MSDC_MAX_POWER_CYCLE			(3)

/*SDIO init status */
#define SDIO_INIT_SUCCESS (1)
#define SDIO_INIT_FAIL (2)

/****************************************************************************/
/* Extern Function in header file for KS Coding Style */
/****************************************************************************/
extern struct msdc_host *atc_msdc_host[HOST_MAX_NUM];
extern transfer_mode msdc_latest_transfer_mode[HOST_MAX_NUM];
extern operation_type msdc_latest_operation_type[HOST_MAX_NUM];

#ifdef MTK_EMMC_SUPPORT
extern u32 g_emmc_mode_switch;
#endif

int msdc_tune_cmdrsp(struct msdc_host *host);
int msdc_get_card_status(struct mmc_host *mmc, struct msdc_host *host, u32 *status);
void msdc_set_driving(struct msdc_host *host, struct msdc_hw *hw, bool sd_18);
void msdc_set_sr(struct msdc_host *host, int clk, int cmd, int dat);
void msdc_set_rdtdsel_dbg(struct msdc_host *host, bool rdsel, u32 value);
struct msdc_host *msdc_get_host(int host_function, bool boot, bool secondary);
void msdc_set_mclk(struct msdc_host *host, int ddr, u32 hz);
struct gendisk *mmc_get_disk(struct mmc_card *card);
unsigned int msdc_do_command(struct msdc_host *host,
				    struct mmc_command *cmd, int tune, unsigned long timeout);
//int msdc_pio_read(struct msdc_host *host, struct mmc_data *data);
//int msdc_pio_write(struct msdc_host *host, struct mmc_data *data);




/* Exprot from mmc Core*/
extern void mmc_remove_card(struct mmc_card *card);
extern void mmc_detach_bus(struct mmc_host *host);
extern void mmc_power_off(struct mmc_host *host);
extern int mmc_send_ext_csd(struct mmc_card *card, u8 *ext_csd);
extern u32 __mmc_sd_num_wr_blocks(struct mmc_card *card);
extern struct mmc_part_info *linux_mmc_find_partinfo_by_name(char *name);
#ifdef ATC_LINUX_PLATFORM
struct mmc_part_info *mmc_find_part_by_name(char *name);
#else
struct hd_struct *mmc_find_part_by_name(char *name);
#endif
void simp_msdc_init_hw(struct msdc_host *host);

#endif /* end of __ATC_MSDC_H__ */

