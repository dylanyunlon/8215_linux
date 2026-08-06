#ifndef __ATC_MSDC_ETT_H__
#define __ATC_MSDC_ETT_H__

#include "atc_msdc.h"

typedef enum EMMC_CHIP_TAG {
	SAMSUNG_EMMC_CHIP = 0x15,
	SANDISK_EMMC_CHIP = 0x45,
	HYNIX_EMMC_CHIP = 0x90,
} EMMC_VENDOR_T;

struct msdc_ett_settings {
#define MSDC_DEFAULT_MODE (0)
#define MSDC_SDR50_MODE   (1)
#define MSDC_DDR50_MODE   (2)
#define MSDC_HS200_MODE   (3)
#define MSDC_HS400_MODE   (4)
	unsigned int reg_addr;
	unsigned int reg_offset;
	unsigned int value;
};


#ifdef MSDC_SUPPORT_SANDISK_COMBO_ETT
struct msdc_ett_settings msdc0_ett_hs200_settings_for_sandisk[] = {
	{ 0xb0,  (0x7 << 7), 0 }, /* PATCH_BIT0[MSDC_PB0_INT_DAT_LATCH_CK_SEL] */
	{ 0xb0,  (0x1f << 10), 0 }, /* PATCH_BIT0[MSDC_PB0_CKGEN_MSDC_DLY_SEL] */

	/* command & resp ett settings */
	{ 0xb4,  (0x7 << 3), 1 }, /* PATCH_BIT1[MSDC_PB1_CMD_RSP_TA_CNTR] */
	{ 0x4,   (0x1 << 1), 1 }, /* MSDC_IOCON[MSDC_IOCON_RSPL] */
	{ 0xf0,  (0x1f << 16), 0 }, /* PAD_TUNE[MSDC_PAD_TUNE_CMDRDLY] */
	{ 0xf0,  (0x1f << 22), 6 }, /* PAD_TUNE[MSDC_PAD_TUNE_CMDRRDLY] */

	/* write ett settings */
	{ 0xb4,  (0x7 << 0), 1 }, /* PATCH_BIT1[MSDC_PB1_WRDAT_CRCS_TA_CNTR] */
	{ 0xf0,  (0x1f << 0), 15 }, /* PAD_TUNE[MSDC_PAD_TUNE_DATWRDLY] */
	{ 0x4,   (0x1 << 10), 1 }, /* MSDC_IOCON[MSDC_IOCON_W_D0SPL] */
	{ 0xf8,  (0x1f << 24), 5 }, /* DAT_RD_DLY0[MSDC_DAT_RDDLY0_D0] */

	/* read ett settings */
	{ 0xf0,  (0x1f << 8), 18}, /* PAD_TUNE[MSDC_PAD_TUNE_DATRRDLY] */
	{ 0x4,   (0x1 << 2), 3 }, /* MSDC_IOCON[MSDC_IOCON_R_D_SMPL] */
};
struct msdc_ett_settings msdc0_ett_hs400_settings_for_sandisk[] = {
	{ 0xb0,  (0x7 << 7), 0 }, /* PATCH_BIT0[MSDC_PB0_INT_DAT_LATCH_CK_SEL] */
	{ 0xb0,  (0x1f << 10), 0 }, /* PATCH_BIT0[MSDC_PB0_CKGEN_MSDC_DLY_SEL] */
	{ 0x188, (0x1f << 2), 2 /*0x0*/ }, /* EMMC50_PAD_DS_TUNE[MSDC_EMMC50_PAD_DS_TUNE_DLY1] */
	{ 0x188, (0x1f << 12), 18 /*0x13*/}, /* EMMC50_PAD_DS_TUNE[MSDC_EMMC50_PAD_DS_TUNE_DLY3] */

	{ 0xb4,  (0x7 << 3), 1 }, /* PATCH_BIT1[MSDC_PB1_CMD_RSP_TA_CNTR] */
	{ 0x4,   (0x1 << 1), 1 }, /* MSDC_IOCON[MSDC_IOCON_RSPL] */
	{ 0xf0,  (0x1f << 16), 0 }, /* PAD_TUNE[MSDC_PAD_TUNE_CMDRDLY] */
	{ 0xf0,  (0x1f << 22), 11 /*0x0*/ }, /* PAD_TUNE[MSDC_PAD_TUNE_CMDRRDLY] */
};
#endif /* mt3561m MSDC_SUPPORT_SANDISK_COMBO_ETT */



#endif /* end of __ATC_MSDC_ETT_H__ */

