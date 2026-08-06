
#ifndef __EMMC_INIT__
#define __EMMC_INIT__

/*--------------------------------------------------------------------------*/
/* head file define                                                         */
/*--------------------------------------------------------------------------*/
#define MTK_MMC_DUMP_DBG        (0)
#if MTK_MMC_DUMP_DBG
#define MTK_DUMP_PR_ERR(fmt, args...)	pr_err(fmt, ##args)
#define MTK_DUMP_PR_DBG(fmt, args...)	pr_err(fmt, ##args)
#else
#define MTK_MMC_DUMP(fmt, args...)
#define MTK_DUMP_PR_ERR(fmt, args...)
#define MTK_DUMP_PR_DBG(fmt, args...)
#endif

#define MAX_POLLING_STATUS      (50000)

struct simp_mmc_card;
struct simp_msdc_host;
#define MSDC_PS_DAT0            (0x1  << 16)    /* R  */
struct simp_mmc_host {
	int index;
	unsigned int            f_min;
	unsigned int            f_max;
	unsigned int            f_init;
	u32                     ocr_avail;

	unsigned int            caps;           /* Host capabilities */
	unsigned int            caps2;          /* More host capabilities */

	/* host specific block data */
	unsigned int            max_seg_size;   /* blk_queue_max_segment_size */
	unsigned short          max_segs;       /* blk_queue_max_segments */
	unsigned short          unused;
	unsigned int            max_req_size;   /* maximum bytes in one req */
	unsigned int            max_blk_size;   /* maximum size of one block */
	unsigned int            max_blk_count;  /* maximum blocks in one req */
	unsigned int            max_discard_to; /* max. discard timeout in ms */

	u32                     ocr;            /* the current OCR setting */

	struct simp_mmc_card    *card;          /* card attached to this host */

	unsigned int            actual_clock;   /* Actual HC clock */

	/* add msdc struct */
	struct simp_msdc_host   *mtk_host;
};

struct simp_mmc_card {
	struct simp_mmc_host    *host;          /* card's belonging host */
	unsigned int            rca;            /* relative card address */
	unsigned int            type;           /* card type */
	unsigned int            state;          /* (our) card state */
	unsigned int            quirks;         /* card quirks */

	unsigned int            erase_size;     /* erase size in sectors */
	unsigned int            erase_shift;    /* if erase unit is power 2 */
	unsigned int            pref_erase;     /* in sectors */
	u8                      erased_byte;    /* value of erased bytes */

	u32                     raw_cid[4];     /* raw card CID */
	u32                     raw_csd[4];     /* raw card CSD */
	u32                     raw_scr[2];     /* raw card SCR */
	struct mmc_cid          cid;            /* card identification */
	struct mmc_csd          csd;            /* card specific */
	struct mmc_ext_csd      ext_csd;        /* MMC extended card specific */
	struct sd_scr           scr;            /* extra SD information */
	struct sd_ssr           ssr;            /* yet more SD information */
	struct sd_switch_caps   sw_caps;        /* switch (CMD6) caps */

	unsigned int            sd_bus_speed;   /* Current Bus Speed Mode */
};

struct simp_msdc_card {
	unsigned int            rca;            /* relative card address */
	unsigned int            type;           /* card type */
	unsigned short          state;          /* card state */
	unsigned short          file_system;    /* FAT16/FAT32 */
	unsigned short          card_cap;       /* High Capcity/standard */
};

struct simp_msdc_host {
	struct simp_msdc_card   *card;
	void __iomem            *base;          /* host base address */
	unsigned char           id;             /* host id number */
	unsigned int            clk;            /* host clock value from
						   clock source */
	unsigned int            sclk;           /* working SD clock speed */
	unsigned char           clksrc;         /* clock source */
	void                    *priv;          /* private data */
};

enum {
	MSDC_CLKSRC_200M = 0
};

enum {
	FAT16 = 0,
	FAT32 = 1,
	exFAT = 2,
	_RAW_ = 3,
};

enum {
	standard_capacity = 0,
	high_capacity = 1,
	extended_capacity = 2,
};

/* command argument */
#define CMD0_ARG                (0)
#define CMD2_ARG                (0)
#define CMD3_ARG                (0)
#define CMD7_ARG                (phost->card->rca)
#define CMD8_ARG_VOL_27_36      (0x100)
#define CMD8_ARG_PATTERN        (0x5a)          /* or 0xAA */
#define CMD8_ARG                (CMD8_ARG_VOL_27_36 | CMD8_ARG_PATTERN)
#define CMD9_ARG                (phost->card->rca)
#define CMD10_ARG               (phost->card->rca)
#define CMD12_ARG               (0)
#define CMD13_ARG               (phost->card->rca)
#define CMD55_ARG               (phost->card->rca)

#define ACMD6_ARG_BUS_WIDTH_4   (0x2)
#define ACMD6_ARG               (ACMD6_ARG_BUS_WIDTH_4)

#define ACMD41_ARG_HCS          (1 << 30)
#define ACMD41_ARG_VOL_27_36    (0xff8000)
#define ACMD41_ARG_20           (ACMD41_ARG_VOL_27_36 | ACMD41_ARG_HCS)
#define ACMD41_ARG_10           (ACMD41_ARG_VOL_27_36)

#define ACMD6_ARG_EMMC          ((MMC_SWITCH_MODE_WRITE_BYTE << 24) \
					| (EXT_CSD_BUS_WIDTH << 16) \
					| (EXT_CSD_BUS_WIDTH_4 << 8) \
					| EXT_CSD_CMD_SET_NORMAL)

#define ACMD6_ARG_DISABLE_CACHE ((MMC_SWITCH_MODE_WRITE_BYTE << 24) \
					| (EXT_CSD_CACHE_CTRL << 16) \
					| (0 << 8) | EXT_CSD_CMD_SET_NORMAL)

#define CMD_RAW(cmd, rspt, dtyp, rw, len, stop) \
				((cmd) | (rspt << 7) | (dtyp << 11) | \
				 (rw << 13) | (len << 16) | (stop << 14))

#define CMD0_RAW		CMD_RAW(0 , msdc_rsp[RESP_NONE], 0, 0, 0, 0)
#define CMD1_RAW		CMD_RAW(1 , msdc_rsp[RESP_R3]  , 0, 0, 0, 0)
#define CMD2_RAW		CMD_RAW(2 , msdc_rsp[RESP_R2]  , 0, 0, 0, 0)
#define CMD3_RAW		CMD_RAW(3 , msdc_rsp[RESP_R1]  , 0, 0, 0, 0)
#define CMD7_RAW		CMD_RAW(7 , msdc_rsp[RESP_R1]  , 0, 0, 0, 0)
#define CMD8_RAW		CMD_RAW(8 , msdc_rsp[RESP_R7]  , 0, 0, 0, 0)
#define CMD8_RAW_EMMC		CMD_RAW(8 , msdc_rsp[RESP_R1]  , 1, 0, 512, 0)
#define CMD9_RAW		CMD_RAW(9 , msdc_rsp[RESP_R2]  , 0, 0, 0, 0)
#define CMD10_RAW		CMD_RAW(10, msdc_rsp[RESP_R2]  , 0, 0, 0, 0)
#define CMD12_RAW		CMD_RAW(12, msdc_rsp[RESP_R1B] , 0, 0,   0, 1)
#define CMD13_RAW		CMD_RAW(13, msdc_rsp[RESP_R1]  , 0, 0, 0, 0)
#define CMD17_RAW		CMD_RAW(17, msdc_rsp[RESP_R1]  , 1, 0, 512, 0)
#define CMD18_RAW		CMD_RAW(18, msdc_rsp[RESP_R1]  , 2, 0, 512, 0)
#define CMD24_RAW		CMD_RAW(24, msdc_rsp[RESP_R1]  , 1, 1, 512, 0)
#define CMD25_RAW		CMD_RAW(25, msdc_rsp[RESP_R1]  , 2, 1, 512, 0)
#define CMD55_RAW		CMD_RAW(55, msdc_rsp[RESP_R1]  , 0, 0, 0, 0)

#define ACMD6_RAW		CMD_RAW(6 , msdc_rsp[RESP_R1]  , 0, 0, 0, 0)
#define ACMD6_RAW_EMMC		CMD_RAW(6 , msdc_rsp[RESP_R1B] , 0, 0, 0, 0)
#define ACMD41_RAW		CMD_RAW(41, msdc_rsp[RESP_R3]  , 0, 0, 0, 0)

/* command response */
#define R3_OCR_POWER_UP_BIT        (1 << 31)
#define R3_OCR_CARD_CAPACITY_BIT   (1 << 30)

/* some marco will be reuse with mmc subsystem */
static int simp_mmc_init(int card_type, bool boot);
static int module_init_emmc;
static int module_init_sd;
static int emmc_init;
static int sd_init;

#define SIMP_SUCCESS            (0)
#define SIMP_FAILED             (-1)

/* the base address of sd card slot */
#define BOOT_STORAGE_ID         (0)
#define EXTEND_STORAGE_ID       (1)
#define MSDC_CLKSRC             (MSDC_CLKSRC_200M)
static unsigned int clks[] = { 200000000 };

#define BLK_LEN                 (512)
#define MAX_SCLK                (52000000)
#define NORMAL_SCLK             (25000000)
#define MIN_SCLK                (260000)

#define MAX_DMA_CNT             (64 * 1024 - 512)
#define CMD_WAIT_RETRY          (0x8FFFFFFF)

extern struct hd_struct *mmc_find_part_by_name(char *name);

#endif /* end of __EMMC_INIT__ */



