/* #include <generated/autoconf.h> */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/blkdev.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/core.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/dma-mapping.h>

#include <mach/dma.h>
#include <mach/board.h>		/* FIXME */

#include "atc_msdc.h"
#include "atc_msdc_dbg.h"

#include <linux/mmc/atc_msdc_misc.h>

#include "../card/queue.h"
#include "../core/bus.h"
#include "../core/core.h"
#include "atc_msdc.h"

#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/genhd.h>


#define DRV_NAME_MISC            "misc-sd"

#define DEBUG_MMC_IOCTL   0
#define DEBUG_MSDC_SSC    1
/*
 * For simple_sd_ioctl
 */
#define FORCE_IN_DMA (0x11)
#define FORCE_IN_PIO (0x10)
#define FORCE_NOTHING (0x0)

#if (3 == HOST_MAX_NUM)
static int dma_force[HOST_MAX_NUM] =	/* used for sd ioctrol */
{
	FORCE_NOTHING,
	FORCE_NOTHING,
	FORCE_NOTHING,
};
#endif

#define dma_is_forced(host_id)				(dma_force[(host_id)] & 0x10)
#define get_forced_transfer_mode(host_id)	(dma_force[(host_id)] & 0x01)

#define WP_LOG_FFL(pr_level, fmt, args...)                         \
        pr_level("[MSDC][WP]%s():"fmt", @%s:%d\n", __FUNCTION__, ##args, FILE_ONLY, __LINE__)

#define WP_LOG(pr_level, fmt, args...)                             \
        pr_level("[MSDC][WP]"fmt"\n", ##args)

struct wp_information {
	bool valid;
	u8 usr_wp;
	u8 boot_wp;
	u8 part_cfg_raw;
	uint32_t wpg_size_in_sect;
	uint32_t wpg_cnt_max;
};
#define MAX_REGION_CNT (5)
struct wp_region_info_t {
	struct {
		uint32_t start_sector;
		uint32_t end_sector;
	} region[MAX_REGION_CNT];
	unsigned int region_cnt;
};
static struct wp_information wp_info = {0};
static struct wp_region_info_t wp_region_save_info = {0};

static u32 *sg_msdc_multi_buffer;
static int simple_sd_open(struct inode *inode, struct file *file)
{
	return 0;
}

int msdc_reinit(struct msdc_host *host)
{
	struct mmc_host *mmc;
	struct mmc_card *card;
	int ret = -1;
	u32 err = 0;
	u32 status = 0;
	unsigned long tmo = 12U;

	if (!host) {
		MSDC_LOG(ERR, "msdc_host is NULL");
		return -1;
	}

	mmc = host->mmc;

	if (!mmc) {
		MSDC_LOG(ERR, "mmc is NULL");
		return -1;
	}

	card = mmc->card;

	if (card == NULL) {
		MSDC_LOG(ERR, "mmc->card is NULL");
	}

	if (host->block_bad_card) {
		MSDC_LOG(ERR, "Need block this bad SD card from re-initialization");
	}

	/* eMMC first */
#ifdef MTK_EMMC_SUPPORT

	if (host->hw->host_function == MSDC_EMMC) {
		/* Fixme: */
		return -1;
	}

#endif

	if (host->hw->host_function == MSDC_SD) {
		if (((host->hw->flags & MSDC_CD_PIN_EN) == 0) && (host->block_bad_card == 0)) {
			/* power cycle */
			MSDC_LOG_NORMAL(pr_info, "SD card Re-Init!");
			mmc_claim_host(host->mmc);
			MSDC_LOG_NORMAL(pr_info, "SD card Re-Init get host!");
			spin_lock(&host->lock);
			MSDC_LOG_NORMAL(pr_info, "SD card Re-Init get lock!");

			if (host->app_cmd_arg) {
				while ((err = msdc_get_card_status(mmc, host, &status))) {
					MSDC_LOG(ERR, "SD card Re-Init in get card status!err(%d)",
						 err);

					if (err == (unsigned int)-EIO) {
						if (msdc_tune_cmdrsp(host)) {
							MSDC_LOG(ERR, "update cmd para failed");
							break;
						}
					} else {
						break;
					}
				}

				if (err == 0) {
					spin_unlock(&host->lock);
					mmc_release_host(host->mmc);
					MSDC_LOG_NORMAL(pr_info, "SD Card is ready.");
					return 0;
				}
			}

			MSDC_LOG_NORMAL(pr_info, "Reinit start..");
			mmc->ios.clock = HOST_MIN_MCLK;
			mmc->ios.bus_width = MMC_BUS_WIDTH_1;
			mmc->ios.timing = MMC_TIMING_LEGACY;
			host->card_inserted = 1;
			msdc_set_mclk(host, 0, HOST_MIN_MCLK);
			spin_unlock(&host->lock);
			mmc_release_host(host->mmc);

			if (host->mmc->card) {
				mmc_remove_card(host->mmc->card);
				host->mmc->card = NULL;
				mmc_claim_host(host->mmc);
				mmc_detach_bus(host->mmc);
				mmc_release_host(host->mmc);
			}

			mmc_power_off(host->mmc);
			mmc_detect_change(host->mmc, 0);

			while (tmo) {
				if ((host->mmc->card) && mmc_card_present(host->mmc->card)) {
					ret = 0;
					break;
				}

				msleep(50);
				tmo--;
			}

			MSDC_LOG(ERR, "Reinit %s", ret == 0 ? "success" : "fail");

		}

		if ((host->hw->flags & MSDC_CD_PIN_EN) && (host->mmc->card)
		    && mmc_card_present(host->mmc->card) && (!mmc_card_removed(host->mmc->card))
		    && (host->block_bad_card == 0)) {
			ret = 0;
		}
	}

	return ret;
}

int check_sdcard_present(void)
{
	int ret = 0;
	struct mmc_host *mmc;
	struct mmc_card *card;
	struct msdc_host *host = msdc_get_host(MSDC_SD, 0, 0);

	if (!host) {
		MSDC_LOG(ERR, "msdc_host is NULL");
		return 0;
	}

	mmc = host->mmc;

	if (!mmc) {
		MSDC_LOG(ERR, "mmc is NULL");
		return 0;
	}

	card = mmc->card;
	if (card == NULL) {
		MSDC_LOG(ERR, "mmc->card is NULL");
		return 0;
	}

	if (host->block_bad_card) {
		MSDC_LOG(ERR, "Need block this bad SD card check");
		return 0;
	}

	MSDC_LOG_NORMAL(pr_info, "sdcard state is 0x%x\n", host->mmc->card->state);

	if ((host->mmc->card) && mmc_card_present(host->mmc->card)
		&& (!mmc_card_removed(host->mmc->card)) && (host->block_bad_card == 0)) {
			ret = 1;
	}

	MSDC_LOG_NORMAL(pr_info, "sdcard present status is %d\n", ret);
	return ret;
}
EXPORT_SYMBOL(check_sdcard_present);

static int sd_ioctl_reinit(struct msdc_ioctl *msdc_ctl)
{
	struct msdc_host *host = msdc_get_host(MSDC_SD, 0, 0);

	return msdc_reinit(host);
}

#ifdef MTK_EMMC_SUPPORT
void msdc_check_init_done(void)
{
	struct msdc_host *host = NULL;

	host = msdc_get_host(MSDC_EMMC, 1, 0);
	BUG_ON(!host);
	BUG_ON(!host->mmc);
	host->mmc->card_init_wait(host->mmc);
	BUG_ON(!host->mmc->card);
}
#endif

static int simple_sd_ioctl_single_rw(struct msdc_ioctl *msdc_ctl)
{
	char l_buf[512];
	struct scatterlist msdc_sg;
	struct mmc_data msdc_data;
	struct mmc_command msdc_cmd;
	struct mmc_request msdc_mrq;
	struct msdc_host *host_ctl;

	host_ctl = atc_msdc_host[msdc_ctl->host_num];
	BUG_ON(!host_ctl);
	BUG_ON(!host_ctl->mmc);
	BUG_ON(!host_ctl->mmc->card);

	mmc_claim_host(host_ctl->mmc);

#if DEBUG_MMC_IOCTL
	MSDC_LOG_NORMAL(pr_info, "user want access %d partition\n", msdc_ctl->partition);
#endif

	mmc_send_ext_csd(host_ctl->mmc->card, l_buf);

	switch (msdc_ctl->partition) {
	case EMMC_PART_BOOT1:
		if (0x1 != (l_buf[179] & 0x7)) {
			/* change to access boot partition 1 */
			l_buf[179] &= ~0x7;
			l_buf[179] |= 0x1;
			mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179], 1000);
		}

		break;

	case EMMC_PART_BOOT2:
		if (0x2 != (l_buf[179] & 0x7)) {
			/* change to access boot partition 2 */
			l_buf[179] &= ~0x7;
			l_buf[179] |= 0x2;
			mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179], 1000);
		}

		break;

	default:

		/* make sure access partition is user data area */
		if (0 != (l_buf[179] & 0x7)) {
			/* set back to access user area */
			l_buf[179] &= ~0x7;
			l_buf[179] |= 0x0;
			mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179], 1000);
		}

		break;
	}

	if (msdc_ctl->total_size > 512) {
		msdc_ctl->result = -1;
		return msdc_ctl->result;
	}

#if DEBUG_MMC_IOCTL
	MSDC_LOG_NORMAL(pr_info, "start MSDC_SINGLE_READ_WRITE !!\n");
#endif
	memset(&msdc_data, 0, sizeof(struct mmc_data));
	memset(&msdc_mrq, 0, sizeof(struct mmc_request));
	memset(&msdc_cmd, 0, sizeof(struct mmc_command));

	msdc_mrq.cmd = &msdc_cmd;
	msdc_mrq.data = &msdc_data;

	if (msdc_ctl->trans_type) {
		dma_force[host_ctl->id] = FORCE_IN_DMA;
	} else {
		dma_force[host_ctl->id] = FORCE_IN_PIO;
	}

	if (msdc_ctl->iswrite) {
		msdc_data.flags = MMC_DATA_WRITE;
		msdc_cmd.opcode = MMC_WRITE_BLOCK;
		msdc_data.blocks = msdc_ctl->total_size / 512;

		if (MSDC_CARD_DUNM_FUNC != msdc_ctl->opcode) {
			if (copy_from_user(sg_msdc_multi_buffer, msdc_ctl->buffer, 512)) {
				dma_force[host_ctl->id] = FORCE_NOTHING;
				return -EFAULT;
			}
		} else {
			/* called from other kernel module */
			memcpy(sg_msdc_multi_buffer, msdc_ctl->buffer, 512);
		}
	} else {
		msdc_data.flags = MMC_DATA_READ;
		msdc_cmd.opcode = MMC_READ_SINGLE_BLOCK;
		msdc_data.blocks = msdc_ctl->total_size / 512;

		memset(sg_msdc_multi_buffer, 0, 512);
	}

	msdc_cmd.arg = msdc_ctl->address;

	if (!mmc_card_blockaddr(host_ctl->mmc->card)) {
		MSDC_LOG_NORMAL(pr_info, "the device is used byte address!\n");
		msdc_cmd.arg <<= 9;
	}

	msdc_cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;

	msdc_data.stop = NULL;
	msdc_data.blksz = 512;
	msdc_data.sg = &msdc_sg;
	msdc_data.sg_len = 1;

#if DEBUG_MMC_IOCTL
	MSDC_LOG_NORMAL(pr_info, "single block: ueser buf address is 0x%p!\n", msdc_ctl->buffer);
#endif
	sg_init_one(&msdc_sg, sg_msdc_multi_buffer, msdc_ctl->total_size);
	mmc_set_data_timeout(&msdc_data, host_ctl->mmc->card);

	mmc_wait_for_req(host_ctl->mmc, &msdc_mrq);

	if (!msdc_ctl->iswrite) {
		if (MSDC_CARD_DUNM_FUNC != msdc_ctl->opcode) {
			if (copy_to_user(msdc_ctl->buffer, sg_msdc_multi_buffer, 512)) {
				dma_force[host_ctl->id] = FORCE_NOTHING;
				return -EFAULT;
			}
		} else {
			/* called from other kernel module */
			memcpy(msdc_ctl->buffer, sg_msdc_multi_buffer, 512);
		}
	}

	if (msdc_ctl->partition) {
		mmc_send_ext_csd(host_ctl->mmc->card, l_buf);

		if (l_buf[179] & 0x7) {
			/* set back to access user area */
			l_buf[179] &= ~0x7;
			l_buf[179] |= 0x0;
			mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179], 1000);
		}
	}

	mmc_release_host(host_ctl->mmc);

	if (msdc_cmd.error) {
		msdc_ctl->result = msdc_cmd.error;
	}

	if (msdc_data.error) {
		msdc_ctl->result = msdc_data.error;
	} else {
		msdc_ctl->result = 0;
	}

	dma_force[host_ctl->id] = FORCE_NOTHING;
	return msdc_ctl->result;
}

static int simple_sd_ioctl_multi_rw(struct msdc_ioctl *msdc_ctl)
{
	char l_buf[512];
	struct scatterlist msdc_sg;
	struct mmc_data msdc_data;
	struct mmc_command msdc_cmd;
	struct mmc_command msdc_stop;
	struct mmc_request msdc_mrq;

	struct msdc_host *host_ctl;

	host_ctl = atc_msdc_host[msdc_ctl->host_num];
	BUG_ON(!host_ctl);
	BUG_ON(!host_ctl->mmc);
	BUG_ON(!host_ctl->mmc->card);

	mmc_claim_host(host_ctl->mmc);

#if DEBUG_MMC_IOCTL
	MSDC_LOG_NORMAL(pr_info, "user want access %d partition\n", msdc_ctl->partition);
#endif

	mmc_send_ext_csd(host_ctl->mmc->card, l_buf);

	switch (msdc_ctl->partition) {
	case EMMC_PART_BOOT1:
		if (0x1 != (l_buf[179] & 0x7)) {
			/* change to access boot partition 1 */
			l_buf[179] &= ~0x7;
			l_buf[179] |= 0x1;
			mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179], 1000);
		}

		break;

	case EMMC_PART_BOOT2:
		if (0x2 != (l_buf[179] & 0x7)) {
			/* change to access boot partition 2 */
			l_buf[179] &= ~0x7;
			l_buf[179] |= 0x2;
			mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179], 1000);
		}

		break;

	default:

		/* make sure access partition is user data area */
		if (0 != (l_buf[179] & 0x7)) {
			/* set back to access user area */
			l_buf[179] &= ~0x7;
			l_buf[179] |= 0x0;
			mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179], 1000);
		}

		break;
	}

	if (msdc_ctl->total_size > 64U * 1024U) {
		msdc_ctl->result = -1;
		return msdc_ctl->result;
	}

	memset(&msdc_data, 0, sizeof(struct mmc_data));
	memset(&msdc_mrq, 0, sizeof(struct mmc_request));
	memset(&msdc_cmd, 0, sizeof(struct mmc_command));
	memset(&msdc_stop, 0, sizeof(struct mmc_command));

	msdc_mrq.cmd = &msdc_cmd;
	msdc_mrq.data = &msdc_data;

	if (msdc_ctl->trans_type) {
		dma_force[host_ctl->id] = FORCE_IN_DMA;
	} else {
		dma_force[host_ctl->id] = FORCE_IN_PIO;
	}

	if (msdc_ctl->iswrite) {
		msdc_data.flags = MMC_DATA_WRITE;
		msdc_cmd.opcode = MMC_WRITE_MULTIPLE_BLOCK;
		msdc_data.blocks = msdc_ctl->total_size / 512;

		if (MSDC_CARD_DUNM_FUNC != msdc_ctl->opcode) {
			if (copy_from_user
			    (sg_msdc_multi_buffer, msdc_ctl->buffer, msdc_ctl->total_size)) {
				dma_force[host_ctl->id] = FORCE_NOTHING;
				return -EFAULT;
			}
		} else {
			/* called from other kernel module */
			memcpy(sg_msdc_multi_buffer, msdc_ctl->buffer, msdc_ctl->total_size);
		}
	} else {
		msdc_data.flags = MMC_DATA_READ;
		msdc_cmd.opcode = MMC_READ_MULTIPLE_BLOCK;
		msdc_data.blocks = msdc_ctl->total_size / 512;
		memset(sg_msdc_multi_buffer, 0, msdc_ctl->total_size);
	}

	msdc_cmd.arg = msdc_ctl->address;

	if (!mmc_card_blockaddr(host_ctl->mmc->card)) {
		MSDC_LOG_NORMAL(pr_info, "this device use byte address!!\n");
		msdc_cmd.arg <<= 9;
	}

	msdc_cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;

	msdc_stop.opcode = MMC_STOP_TRANSMISSION;
	msdc_stop.arg = 0;
	msdc_stop.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;

	msdc_data.stop = &msdc_stop;
	msdc_data.blksz = 512;
	msdc_data.sg = &msdc_sg;
	msdc_data.sg_len = 1;

#if DEBUG_MMC_IOCTL
	MSDC_LOG_NORMAL(pr_info, "total size is %d\n", msdc_ctl->total_size);
#endif
	sg_init_one(&msdc_sg, sg_msdc_multi_buffer, msdc_ctl->total_size);
	mmc_set_data_timeout(&msdc_data, host_ctl->mmc->card);
	mmc_wait_for_req(host_ctl->mmc, &msdc_mrq);

	if (!msdc_ctl->iswrite) {
		if (MSDC_CARD_DUNM_FUNC != msdc_ctl->opcode) {
			if (copy_to_user
			    (msdc_ctl->buffer, sg_msdc_multi_buffer, msdc_ctl->total_size)) {
				dma_force[host_ctl->id] = FORCE_NOTHING;
				return -EFAULT;
			}
		} else {
			/* called from other kernel module */
			memcpy(msdc_ctl->buffer, sg_msdc_multi_buffer, msdc_ctl->total_size);
		}
	}

	if (msdc_ctl->partition) {
		mmc_send_ext_csd(host_ctl->mmc->card, l_buf);

		if (l_buf[179] & 0x7) {
			/* set back to access user area */
			l_buf[179] &= ~0x7;
			l_buf[179] |= 0x0;
			mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179], 1000);
		}
	}

	mmc_release_host(host_ctl->mmc);

	if (msdc_cmd.error) {
		msdc_ctl->result = msdc_cmd.error;
	}

	if (msdc_data.error) {
		msdc_ctl->result = msdc_data.error;
	} else {
		msdc_ctl->result = 0;
	}

	dma_force[host_ctl->id] = FORCE_NOTHING;
	return msdc_ctl->result;

}

int simple_sd_ioctl_rw(struct msdc_ioctl *msdc_ctl)
{
	if (msdc_ctl->total_size > 512U) {
		return simple_sd_ioctl_multi_rw(msdc_ctl);
	} else {
		return simple_sd_ioctl_single_rw(msdc_ctl);
	}
}

static int simple_sd_ioctl_get_cid(struct msdc_ioctl *msdc_ctl)
{
	struct msdc_host *host_ctl;

	host_ctl = atc_msdc_host[msdc_ctl->host_num];

	BUG_ON(!host_ctl);
	BUG_ON(!host_ctl->mmc);
	BUG_ON(!host_ctl->mmc->card);

#if DEBUG_MMC_IOCTL
	MSDC_LOG_NORMAL(pr_info, "user want the cid in msdc slot%d\n", msdc_ctl->host_num);
#endif

	if (copy_to_user(msdc_ctl->buffer, &host_ctl->mmc->card->raw_cid, 16)) {
		return -EFAULT;
	}

#if DEBUG_MMC_IOCTL
	MSDC_LOG_NORMAL(pr_info, "cid:0x%x,0x%x,0x%x,0x%x\n", host_ctl->mmc->card->raw_cid[0],
		host_ctl->mmc->card->raw_cid[1],
		host_ctl->mmc->card->raw_cid[2], host_ctl->mmc->card->raw_cid[3]);
#endif
	return 0;

}

static int simple_sd_ioctl_get_csd(struct msdc_ioctl *msdc_ctl)
{
	struct msdc_host *host_ctl;

	host_ctl = atc_msdc_host[msdc_ctl->host_num];

	BUG_ON(!host_ctl);
	BUG_ON(!host_ctl->mmc);
	BUG_ON(!host_ctl->mmc->card);

#if DEBUG_MMC_IOCTL
	MSDC_LOG_NORMAL(pr_info, "user want the csd in msdc slot%d\n", msdc_ctl->host_num);
#endif

	if (copy_to_user(msdc_ctl->buffer, &host_ctl->mmc->card->raw_csd, 16)) {
		return -EFAULT;
	}

#if DEBUG_MMC_IOCTL
	MSDC_LOG_NORMAL(pr_info, "csd:0x%x,0x%x,0x%x,0x%x\n", host_ctl->mmc->card->raw_csd[0],
		host_ctl->mmc->card->raw_csd[1],
		host_ctl->mmc->card->raw_csd[2], host_ctl->mmc->card->raw_csd[3]);
#endif
	return 0;

}

static int simple_sd_ioctl_get_excsd(struct msdc_ioctl *msdc_ctl)
{
	char l_buf[512];
	struct msdc_host *host_ctl;

#if DEBUG_MMC_IOCTL
	int i;
#endif

	host_ctl = atc_msdc_host[msdc_ctl->host_num];

	BUG_ON(!host_ctl);
	BUG_ON(!host_ctl->mmc);
	BUG_ON(!host_ctl->mmc->card);

	mmc_claim_host(host_ctl->mmc);

#if DEBUG_MMC_IOCTL
	MSDC_LOG_NORMAL(pr_info, "user want the extend csd in msdc slot%d\n", msdc_ctl->host_num);
#endif
	mmc_send_ext_csd(host_ctl->mmc->card, l_buf);

	if (copy_to_user(msdc_ctl->buffer, l_buf, 512)) {
		return -EFAULT;
	}

#if DEBUG_MMC_IOCTL

	for (i = 0; i < 512; i++) {
		pr_info("%x", l_buf[i]);

		if (0 == ((i + 1) % 16)) {
			pr_info("\n");
		}
	}

#endif

	if (copy_to_user(msdc_ctl->buffer, l_buf, 512UL)) {
		return -EFAULT;
	}

	mmc_release_host(host_ctl->mmc);

	return 0;

}

#define INAND_CMD38_ARG_EXT_CSD  113
#define INAND_CMD38_ARG_ERASE    0x00
#define INAND_CMD38_ARG_TRIM     0x01
#define INAND_CMD38_ARG_SECERASE 0x80
#define INAND_CMD38_ARG_SECTRIM1 0x81
#define INAND_CMD38_ARG_SECTRIM2 0x88
static int simple_sd_ioctl_erase_selected_area(struct msdc_ioctl* msdc_ctl)
{
    //This function is coded by reference to  mmc_blk_issue_discard_rq()@block.c
    char l_buf[512];
    struct msdc_host *host_ctl;
    struct mmc_card *card;
    unsigned int from, nr, arg;
    int err = 0;

    host_ctl = atc_msdc_host[msdc_ctl->host_num];
    BUG_ON(!host_ctl);
    BUG_ON(!host_ctl->mmc);
    BUG_ON(!host_ctl->mmc->card);

    card = host_ctl->mmc->card;

    mmc_claim_host(host_ctl->mmc);

    if ( mmc_card_mmc(card) ) {
        mmc_send_ext_csd(host_ctl->mmc->card,l_buf);
        //Only support erase to user area now. Therefore, check for msdc_ctl->partition is omitted
        if (l_buf[179] & 0x7) {
            /* set to access user area */
            l_buf[179] &= ~0x7;
            l_buf[179] |= 0x0;
            mmc_switch(host_ctl->mmc->card, 0, 179, l_buf[179], 1000);
        }
    }

	if (!mmc_can_erase(card)) {
		err = -EOPNOTSUPP;
		goto out;
	}

	from = msdc_ctl->address;
	nr = msdc_ctl->total_size;

	if (mmc_can_discard(card))
		arg = MMC_DISCARD_ARG;
	else if (mmc_can_trim(card))
		arg = MMC_TRIM_ARG;
	else
		arg = MMC_ERASE_ARG;

#if DEBUG_MMC_IOCTL
	MSDC_LOG_NORMAL(pr_info, "Erase range %x~%x\n",from, from+nr-1);
#endif

	if ( mmc_card_mmc(card) ) {
		if (card->quirks & MMC_QUIRK_INAND_CMD38) {
			err = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
					INAND_CMD38_ARG_EXT_CSD,
					arg == MMC_TRIM_ARG ?
					INAND_CMD38_ARG_TRIM :
					INAND_CMD38_ARG_ERASE,
					0);
			if (err)
				goto out;
		}
	}

	err = mmc_erase(card, from, nr, arg);
out:

    mmc_release_host(host_ctl->mmc);

    msdc_ctl->result=err;

    return msdc_ctl->result;

}


#ifdef MTK_EMMC_SUPPORT
static int simple_sd_ioctl_set_driving(struct msdc_ioctl *msdc_ctl)
{
	u32 base;
	struct msdc_host *host;
#if DEBUG_MMC_IOCTL
	unsigned int l_value;
#endif

	if (msdc_ctl->host_num == 0) {
#ifndef CFG_DEV_MSDC0
		MSDC_LOG_NORMAL(pr_info, "host%d is not config\n", msdc_ctl->host_num);
		return -1;
#endif
	} else if (msdc_ctl->host_num == 1) {
#ifndef CFG_DEV_MSDC1
		MSDC_LOG_NORMAL(pr_info, "host%d is not config\n", msdc_ctl->host_num);
		return -1;
#endif
	} else if (msdc_ctl->host_num == 2) {
#ifndef CFG_DEV_MSDC2
		MSDC_LOG_NORMAL(pr_info, "host%d is not config\n", msdc_ctl->host_num);
		return -1;
#endif
	}

	base = atc_msdc_host[msdc_ctl->host_num]->base;
#if DEBUG_MMC_IOCTL
	/* pr_info("set: clk pull down driving is 0x%x\n", msdc_ctl->clk_pd_driving); */
	MSDC_LOG_NORMAL(pr_info, "set: clk driving is 0x%x\n", msdc_ctl->clk_pu_driving);
	/* pr_info("set: cmd pull down driving is 0x%x\n", msdc_ctl->cmd_pd_driving); */
	MSDC_LOG_NORMAL(pr_info, "set: cmd driving is 0x%x\n", msdc_ctl->cmd_pu_driving);
	/* pr_info("set: dat pull down driving is 0x%x\n", msdc_ctl->dat_pd_driving); */
	MSDC_LOG_NORMAL(pr_info, "set: dat driving is 0x%x\n", msdc_ctl->dat_pu_driving);
#endif

	host = atc_msdc_host[msdc_ctl->host_num];
	host->hw->clk_drv = msdc_ctl->clk_pu_driving;
	host->hw->cmd_drv = msdc_ctl->cmd_pu_driving;
	host->hw->dat_drv = msdc_ctl->dat_pu_driving;
	host->hw->clk_drv_sd_18 = msdc_ctl->clk_pu_driving;
	host->hw->cmd_drv_sd_18 = msdc_ctl->cmd_pu_driving;
	host->hw->dat_drv_sd_18 = msdc_ctl->dat_pu_driving;
	msdc_set_driving(host, host->hw, 0);

	return 0;
}

static int simple_sd_ioctl_get_driving(struct msdc_ioctl *msdc_ctl)
{
	u32 base;
	/* unsigned int l_value; */


	if (msdc_ctl->host_num == 0) {
#ifndef CFG_DEV_MSDC0
		MSDC_LOG_NORMAL(pr_info, "host%d is not config\n", msdc_ctl->host_num);
		return -1;
#endif
	} else if (msdc_ctl->host_num == 1) {
#ifndef CFG_DEV_MSDC1
		MSDC_LOG_NORMAL(pr_info, "host%d is not config\n", msdc_ctl->host_num);
		return -1;
#endif
	} else if (msdc_ctl->host_num == 2) {
#ifndef CFG_DEV_MSDC2
		MSDC_LOG_NORMAL(pr_info, "host%d is not config\n", msdc_ctl->host_num);
		return -1;
#endif
	}

	base = atc_msdc_host[msdc_ctl->host_num]->base;

#if DEBUG_MMC_IOCTL
	/* pr_info("read: clk pull down driving is 0x%x\n", msdc_ctl->clk_pd_driving); */
	MSDC_LOG_NORMAL(pr_info, "read: clk driving is 0x%x\n", msdc_ctl->clk_pu_driving);
	/* pr_info("read: cmd pull down driving is 0x%x\n", msdc_ctl->cmd_pd_driving); */
	MSDC_LOG_NORMAL(pr_info, "read: cmd driving is 0x%x\n", msdc_ctl->cmd_pu_driving);
	/* pr_info("read: dat pull down driving is 0x%x\n", msdc_ctl->dat_pd_driving); */
	MSDC_LOG_NORMAL(pr_info, "read: dat driving is 0x%x\n", msdc_ctl->dat_pu_driving);
#endif

	return 0;
}
#endif
static int simple_sd_ioctl_sd30_mswitch(struct msdc_ioctl *msdc_ctl)
{
	/* u32 base; */
	/* struct msdc_hw hw; */
	int id = msdc_ctl->host_num;
#if DEBUG_MMC_IOCTL
	unsigned int l_value;
#endif

	if (msdc_ctl->host_num == 0) {
#ifndef CFG_DEV_MSDC0
		MSDC_LOG_NORMAL(pr_info, "host%d is not config\n", msdc_ctl->host_num);
		return -1;
#endif
	} else if (msdc_ctl->host_num == 1) {
#ifndef CFG_DEV_MSDC1
		MSDC_LOG_NORMAL(pr_info, "host%d is not config\n", msdc_ctl->host_num);
		return -1;
#endif
	} else if (msdc_ctl->host_num == 2) {
#ifndef CFG_DEV_MSDC2
		MSDC_LOG_NORMAL(pr_info, "host%d is not config\n", msdc_ctl->host_num);
		return -1;
#endif
	}

	switch (msdc_ctl->sd30_mode) {
	case SDHC_HIGHSPEED:
		msdc_host_mode[id] |= MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED;
		msdc_host_mode[id] &=
			(~MMC_CAP_UHS_SDR12) & (~MMC_CAP_UHS_SDR25) & (~MMC_CAP_UHS_SDR50) &
			(~MMC_CAP_UHS_DDR50) & (~MMC_CAP_1_8V_DDR) & (~MMC_CAP_UHS_SDR104);
		msdc_host_mode2[id] &= ~MMC_CAP2_HS200_1_8V_SDR;
		MSDC_LOG_NORMAL(pr_info, "[****SD_Debug****]host will support Highspeed\n");
		break;

	case UHS_SDR12:
		msdc_host_mode[id] |= MMC_CAP_UHS_SDR12;
		msdc_host_mode[id] &=
			(~MMC_CAP_UHS_SDR25) & (~MMC_CAP_UHS_SDR50) & (~MMC_CAP_UHS_DDR50) &
			(~MMC_CAP_1_8V_DDR) & (~MMC_CAP_UHS_SDR104);
		msdc_host_mode2[id] &= ~MMC_CAP2_HS200_1_8V_SDR;
		MSDC_LOG_NORMAL(pr_info, "[****SD_Debug****]host will support UHS-SDR12\n");
		break;

	case UHS_SDR25:
		msdc_host_mode[id] |= MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25;
		msdc_host_mode[id] &=
			(~MMC_CAP_UHS_SDR50) & (~MMC_CAP_UHS_DDR50) & (~MMC_CAP_1_8V_DDR) &
			(~MMC_CAP_UHS_SDR104);
		msdc_host_mode2[id] &= ~MMC_CAP2_HS200_1_8V_SDR;
		MSDC_LOG_NORMAL(pr_info, "[****SD_Debug****]host will support UHS-SDR25\n");
		break;

	case UHS_SDR50:
		msdc_host_mode[id] |= MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 | MMC_CAP_UHS_SDR50;
		msdc_host_mode[id] &=
			(~MMC_CAP_UHS_DDR50) & (~MMC_CAP_1_8V_DDR) & (~MMC_CAP_UHS_SDR104);
		msdc_host_mode2[id] &= ~MMC_CAP2_HS200_1_8V_SDR;
		MSDC_LOG_NORMAL(pr_info, "[****SD_Debug****]host will support UHS-SDR50\n");
		break;

	case UHS_SDR104:
		msdc_host_mode[id] |=
			MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 | MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_DDR50 |
			MMC_CAP_1_8V_DDR | MMC_CAP_UHS_SDR104;
		msdc_host_mode2[id] |= MMC_CAP2_HS200_1_8V_SDR;
		MSDC_LOG_NORMAL(pr_info, "[****SD_Debug****]host will support UHS-SDR104\n");
		break;

	case UHS_DDR50:
		msdc_host_mode[id] |=
			MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 | MMC_CAP_UHS_DDR50 | MMC_CAP_1_8V_DDR;
		msdc_host_mode[id] &= (~MMC_CAP_UHS_SDR50) & (~MMC_CAP_UHS_SDR104);
		msdc_host_mode2[id] &= ~MMC_CAP2_HS200_1_8V_SDR;
		MSDC_LOG_NORMAL(pr_info, "[****SD_Debug****]host will support UHS-DDR50\n");
		break;

	default:
		break;

	}

#ifdef MTK_EMMC_SUPPORT

	/* just for emmc slot */
	if (msdc_ctl->host_num == 0) {
		g_emmc_mode_switch = 1;
	} else {
		g_emmc_mode_switch = 0;
	}

#endif


	switch (msdc_ctl->sd30_drive) {
	case DRIVER_TYPE_A:
		msdc_host_mode[id] |= MMC_CAP_DRIVER_TYPE_A;
		msdc_host_mode[id] &= (~MMC_CAP_DRIVER_TYPE_C) & (~MMC_CAP_DRIVER_TYPE_D);
		MSDC_LOG_NORMAL(pr_info, "[****SD_Debug****]host will support DRIVING TYPE A\n");
		break;

	case DRIVER_TYPE_B:
		msdc_host_mode[id] &=
			(~MMC_CAP_DRIVER_TYPE_A) & (~MMC_CAP_DRIVER_TYPE_C) & (~MMC_CAP_DRIVER_TYPE_D);
		MSDC_LOG_NORMAL(pr_info, "[****SD_Debug****]host will support DRIVING TYPE B\n");
		break;

	case DRIVER_TYPE_C:
		msdc_host_mode[id] |= MMC_CAP_DRIVER_TYPE_C;
		msdc_host_mode[id] &= (~MMC_CAP_DRIVER_TYPE_A) & (~MMC_CAP_DRIVER_TYPE_D);
		MSDC_LOG_NORMAL(pr_info, "[****SD_Debug****]host will support DRIVING TYPE C\n");
		break;

	case DRIVER_TYPE_D:
		msdc_host_mode[id] |= MMC_CAP_DRIVER_TYPE_D;
		msdc_host_mode[id] &= (~MMC_CAP_DRIVER_TYPE_A) & (~MMC_CAP_DRIVER_TYPE_C);
		MSDC_LOG_NORMAL(pr_info, "[****SD_Debug****]host will support DRIVING TYPE D\n");
		break;

	default:
		break;
	}

#if 0

	switch (msdc_ctl->sd30_max_current) {
	case MAX_CURRENT_200:
		msdc_host_mode[id] |= MMC_CAP_MAX_CURRENT_200;
		msdc_host_mode[id] &=
			(~MMC_CAP_MAX_CURRENT_400) & (~MMC_CAP_MAX_CURRENT_600) &
			(~MMC_CAP_MAX_CURRENT_800);
		pr_info("[****SD_Debug****]host will support MAX_CURRENT_200\n");
		break;

	case MAX_CURRENT_400:
		msdc_host_mode[id] |= MMC_CAP_MAX_CURRENT_200 | MMC_CAP_MAX_CURRENT_400;
		msdc_host_mode[id] &= (~MMC_CAP_MAX_CURRENT_600) & (~MMC_CAP_MAX_CURRENT_800);
		pr_info("[****SD_Debug****]host will support MAX_CURRENT_400\n");
		break;

	case MAX_CURRENT_600:
		msdc_host_mode[id] |=
			MMC_CAP_MAX_CURRENT_200 | MMC_CAP_MAX_CURRENT_400 | MMC_CAP_MAX_CURRENT_600;
		msdc_host_mode[id] &= (~MMC_CAP_MAX_CURRENT_800);
		pr_info("[****SD_Debug****]host will support MAX_CURRENT_600\n");
		break;

	case MAX_CURRENT_800:
		msdc_host_mode[id] |=
			MMC_CAP_MAX_CURRENT_200 | MMC_CAP_MAX_CURRENT_400 | MMC_CAP_MAX_CURRENT_600 |
			MMC_CAP_MAX_CURRENT_800;
		pr_info("[****SD_Debug****]host will support MAX_CURRENT_800\n");
		break;

	default:
		break;
	}

	switch (msdc_ctl->sd30_power_control) {
	case SDXC_NO_POWER_CONTROL:
		msdc_host_mode[id] &=
			(~MMC_CAP_SET_XPC_330) & (~MMC_CAP_SET_XPC_300) & (~MMC_CAP_SET_XPC_180);
		pr_info("[****SD_Debug****]host will not support SDXC power control\n");
		break;

	case SDXC_POWER_CONTROL:
		msdc_host_mode[id] |=
			MMC_CAP_SET_XPC_330 | MMC_CAP_SET_XPC_300 | MMC_CAP_SET_XPC_180;
		pr_info("[****SD_Debug****]host will support SDXC power control\n");
		break;

	default:
		break;
	}

#endif
	return 0;
}

/*  to ensure format operate is clean the emmc device fully(partition erase) */
typedef struct mbr_part_info {
	unsigned int start_sector;
	unsigned int nr_sects;
	unsigned int part_no;
	unsigned char *part_name;
} MBR_PART_INFO_T;

#define MBR_PART_NUM  6
#define __MMC_ERASE_ARG        0x00000000
#define __MMC_TRIM_ARG         0x00000001

struct __mmc_blk_data {
	spinlock_t lock;
	struct gendisk *disk;
	struct mmc_queue queue;

	unsigned int usage;
	unsigned int read_only;
};


static u64 msdc_get_user_capacity(struct msdc_host *host)
{
	u64 device_capacity = 0;
	u32 device_legacy_capacity = 0;
	struct mmc_host *mmc = NULL;

	/* BUG_ON(!host); */
	BUG_ON(host == NULL);
	BUG_ON(!host->mmc);
	BUG_ON(!host->mmc->card);
	mmc = host->mmc;

	if (mmc_card_mmc(mmc->card)) {
		if (mmc->card->csd.read_blkbits)
			device_legacy_capacity =
				mmc->card->csd.capacity * (2 << (mmc->card->csd.read_blkbits - 1));
		else {
			device_legacy_capacity = mmc->card->csd.capacity;
			MSDC_LOG_NORMAL(pr_info, "XXX read_blkbits = 0 XXX");
		}

		device_capacity =
			(u64)(mmc->card->ext_csd.sectors) * 512 >
			device_legacy_capacity ? (u64)(mmc->card->ext_csd.sectors) *
			512 : device_legacy_capacity;
	} else if (mmc_card_sd(mmc->card)) {
		device_capacity = (u64)(mmc->card->csd.capacity) << (mmc->card->csd.read_blkbits);
	}

	return device_capacity;
}

u64 msdc_get_capacity(int get_emmc_total)
{
	u64 user_size = 0;
	u32 other_size = 0;
	u64 total_size = 0;
	int index = 0;

	for (index = 0; index < HOST_MAX_NUM; ++index) {
		if ((atc_msdc_host[index] != NULL) && (atc_msdc_host[index]->hw->boot)) {
			user_size = msdc_get_user_capacity(atc_msdc_host[index]);
#ifdef MTK_EMMC_SUPPORT

			if (get_emmc_total) {
				if (mmc_card_mmc(atc_msdc_host[index]->mmc->card)) {
					other_size = msdc_get_other_capacity();
				}
			}

#endif
			break;
		}
	}

	total_size = user_size + (u64) other_size;
	return total_size / 512ULL;
}
EXPORT_SYMBOL(msdc_get_capacity);


int msdc_get_info(STORAGE_TPYE storage_type, GET_STORAGE_INFO info_type, struct storage_info *info)
{
	struct msdc_host *host = NULL;
	int host_function = 0;
	bool boot = 0;

	switch (storage_type) {
	case EMMC_CARD_BOOT:
		host_function = MSDC_EMMC;
		boot = MSDC_BOOT_EN;
		break;

	case EMMC_CARD:
		host_function = MSDC_EMMC;
		break;

	case SD_CARD_BOOT:
		host_function = MSDC_SD;
		boot = MSDC_BOOT_EN;
		break;

	case SD_CARD:
		host_function = MSDC_SD;
		break;

	default:
		MSDC_LOG(ERR, "No supported storage type!");
		return 0;
		/* break; */
	}

	host = msdc_get_host(host_function, boot, 0);

	switch (info_type) {
	case CARD_INFO:
		if ((host->mmc) && (host->mmc->card)) {
			info->card = host->mmc->card;
		} else {
			MSDC_LOG(ERR, "CARD was not ready<get card>!");
			return 0;
		}

		break;

	case DISK_INFO:
		if ((host->mmc) && (host->mmc->card)) {
			info->disk = mmc_get_disk(host->mmc->card);
		} else {
			MSDC_LOG(ERR, "CARD was not ready<get disk>!");
			return 0;
		}

		break;

	case EMMC_USER_CAPACITY:
		info->emmc_user_capacity = msdc_get_capacity(0);
		break;

	case EMMC_CAPACITY:
		info->emmc_capacity = msdc_get_capacity(1);
		break;

	default:
		MSDC_LOG(ERR, "Please check INFO_TYPE");
		return 0;
	}

	return 1;
}

#ifdef MTK_EMMC_SUPPORT
static int simple_mmc_get_disk_info(struct mbr_part_info *mpi, unsigned char *name)
{
	int i = 0;
	char *no_partition_name = "n/a";
	struct disk_part_iter piter;
	struct hd_struct *part;
	struct msdc_host *host;
	struct gendisk *disk;
	struct __mmc_blk_data *md;

	/* emmc always in slot0 */
	host = msdc_get_host(MSDC_EMMC, MSDC_BOOT_EN, 0);
	BUG_ON(!host);
	BUG_ON(!host->mmc);
	BUG_ON(!host->mmc->card);

	md = mmc_get_drvdata(host->mmc->card);
	BUG_ON(!md);
	BUG_ON(!md->disk);

	disk = md->disk;

	/* use this way to find partition info is to avoid handle addr transfer in scatter file
	 * and 64bit address calculate */
	disk_part_iter_init(&piter, disk, 0);

	while ((part = disk_part_iter_next(&piter))) {
		for (i = 0; i < PART_NUM; i++) {
			if (PartInfo[i].partition_idx != 0
			    && PartInfo[i].partition_idx == part->partno) {
#if DEBUG_MMC_IOCTL
				MSDC_LOG_NORMAL(pr_info, "part_name = %s    name = %s\n", PartInfo[i].name, name);
#endif

				if (!strncmp(PartInfo[i].name, name, 25)) {
					mpi->start_sector = part->start_sect;
					mpi->nr_sects = part->nr_sects;
					mpi->part_no = part->partno;

					if (i < PART_NUM) {
						mpi->part_name = PartInfo[i].name;
					} else {
						mpi->part_name = no_partition_name;
					}

					disk_part_iter_exit(&piter);
					return 0;
				}

				break;
			}
		}
	}

	disk_part_iter_exit(&piter);

	return 1;
}

/* call mmc block layer interface for userspace to do erase operate */
static int simple_mmc_erase_func(unsigned int start, unsigned int size)
{
	struct msdc_host *host;

	/* emmc always in slot0 */
	host = msdc_get_host(MSDC_EMMC, MSDC_BOOT_EN, 0);
	BUG_ON(!host);
	BUG_ON(!host->mmc);
	BUG_ON(!host->mmc->card);

	mmc_claim_host(host->mmc);

	if (!mmc_can_trim(host->mmc->card)) {
		MSDC_LOG_NORMAL(pr_info, "emmc card can't support trim\n");
		return 0;
	}

	mmc_erase(host->mmc->card, start, size, __MMC_TRIM_ARG);

#if DEBUG_MMC_IOCTL
	MSDC_LOG_NORMAL(pr_info, "erase done....\n");
#endif

	mmc_release_host(host->mmc);

	return 0;
}
#endif

#ifdef MTK_EMMC_SUPPORT
static int simple_mmc_erase_partition(unsigned char *name)
{
	struct mbr_part_info mbr_part;
	int l_ret = -1;

	BUG_ON(!name);

	/* just support erase cache & data partition now */
	if ((('u' == *name) && ('s' == *(name + 1)) && ('r' == *(name + 2)) && ('d' == *(name + 3)) &&
	     ('a' == *(name + 4)) && ('t' == *(name + 5)) && ('a' == *(name + 6))) ||
	    (('c' == *name) &&( 'a' == *(name + 1)) && ('c' == *(name + 2)) && ('h' == *(name + 3))
	     && ('e' == *(name + 4)))) {
		/* find erase start address and erase size, just support high capacity emmc card now */
		l_ret = simple_mmc_get_disk_info(&mbr_part, name);


		if (l_ret == 0) {
			/* do erase */
			MSDC_LOG_NORMAL(pr_info, "erase %s start sector: 0x%x size: 0x%x\n", mbr_part.part_name,
				mbr_part.start_sector, mbr_part.nr_sects);
			simple_mmc_erase_func(mbr_part.start_sector, mbr_part.nr_sects);
		}
	}

	return 0;
}

#endif

#ifdef MTK_EMMC_SUPPORT
static int simple_mmc_erase_partition_wrap(struct msdc_ioctl *msdc_ctl)
{
	unsigned char name[25];

	if (copy_from_user(name, (unsigned char *)msdc_ctl->buffer, msdc_ctl->total_size)) {
		return -EFAULT;
	}

	return simple_mmc_erase_partition(name);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// emmc write protect
static int
mmc_send_cxd_data_with_arg(struct mmc_card *card, u32 opcode, u32 arg, void *buf, unsigned len)
{
	struct mmc_request mrq;
	struct mmc_command cmd;
	struct mmc_data data;

	struct scatterlist sg;
	int err;

	memset(&mrq, 0, sizeof(struct mmc_request));
	memset(&cmd, 0, sizeof(struct mmc_command));
	memset(&data, 0, sizeof(struct mmc_data));

	mrq.cmd = &cmd;
	mrq.data = &data;

	cmd.opcode = opcode;
	cmd.arg = arg;

	cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;

	data.blksz = len;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;
	data.sg = &sg;
	data.sg_len = 1;

	memset(sg_msdc_multi_buffer, 0, 512);

	mmc_claim_host(card->host);
	sg_init_one(&sg, sg_msdc_multi_buffer, len);
	mmc_set_data_timeout(&data, card);
	mmc_wait_for_req(card->host, &mrq);
	mmc_release_host(card->host);

	if (cmd.error) {
		dev_err(mmc_dev(card->host), "%s: cmd error %d\n", __func__, cmd.error);
		err = cmd.error;
		goto cmd_release_host;
	}
	if (data.error) {
		dev_err(mmc_dev(card->host), "%s: data error %d\n", __func__, data.error);
		err = data.error;
		goto cmd_release_host;
	}
	memcpy(buf, sg_msdc_multi_buffer, len);
	err = 0;

cmd_release_host:
	return err;
}

static int mmc_get_wp_region(struct mmc_card *card, char *dump_buf)
{
	uint32_t wpg_cnt, wpg_cnt_grp_32, cnt_tmp;
	uint8_t result[8] = {0xff};
	uint8_t wp_type;
	uint32_t i;
	int k, t;
	int err = 0;
	int wp_start_found = 0;
	uint32_t wpg_start = 0, wpg_end = 0, id = 0;
	int offset = 0;
	u64 addr_start, addr_end;

	cnt_tmp = 0;
	wpg_cnt = wp_info.wpg_cnt_max;
	wpg_cnt_grp_32 = (wpg_cnt % 32) ? (wpg_cnt/32 + 1) : (wpg_cnt/32);
	WP_LOG(pr_info, "total blks: %u, total wp group cnt: %u, wpg_size_in_sect: 0x%x", card->ext_csd.sectors, wpg_cnt, wp_info.wpg_size_in_sect);

	for(i = 0; i < wpg_cnt_grp_32; i++) {
		err = mmc_send_cxd_data_with_arg(card, MMC_SEND_WRITE_PROT_TYPE, i * 32 * wp_info.wpg_size_in_sect, result, 8);
		if(err) {
			WP_LOG_FFL(pr_err, "mmc_send_cxd_data_with_arg fail %d", err);
			return err;
		}
		WP_LOG(pr_debug, "[grp%d]:result[7~0]:%x %x %x %x %x %x %x %x", i, result[7], result[6], result[5], result[4], result[3], result[2], result[1], result[0]);
		for(k=7; k>=0; k--) {
			for(t=0; t<4; t++, cnt_tmp++) {
				if(cnt_tmp == wpg_cnt) {
					WP_LOG(pr_info, "got end");
					goto out;
				}
				wp_type = (result[k] >> (t*2)) & 0x03;
#if 0
				switch(wp_type) {
					case 0x01://temp
						printk("wpg %u: temporary\n", cnt_tmp);
						break;
					case 0x10://power-on
						printk("wpg %u: power-on\n", cnt_tmp);
						break;
					case 0x11://permanent
						printk("wpg %u: permanent\n", cnt_tmp);
						break;
					//default://not protect
				}
#endif
				if((wp_type != 0) && (wp_start_found == 0)) {
					wp_start_found = 1;
					wpg_start = cnt_tmp;
				}
				if((wp_type == 0) && (wp_start_found == 1)){//find the first unprotect wpg
					wp_start_found = 0;
					wpg_end = cnt_tmp;
					id++;
					addr_start = 512;
					addr_end = 512;
					addr_start *= wpg_start * wp_info.wpg_size_in_sect;
					addr_end *= wpg_end * wp_info.wpg_size_in_sect;
					if(addr_end)
						addr_end -= 1;
					WP_LOG(pr_info, "region%d: wpg%d ~ wpg%d is write protect on", id, wpg_start, wpg_end?(wpg_end-1):wpg_end);
					if(dump_buf == NULL) {
						if(id <= MAX_REGION_CNT) {
							wp_region_save_info.region[id-1].start_sector = wpg_start * wp_info.wpg_size_in_sect;
							wp_region_save_info.region[id-1].end_sector = wpg_end * wp_info.wpg_size_in_sect;
							wp_region_save_info.region_cnt = id;
						} else {
							WP_LOG_FFL(pr_err, "wp region count too much!!!");
							err = -2;
						}
					} else {
						offset += sprintf(dump_buf + offset, "region[%d]:wpg%d ~ wpg%d (0x%llx ~ 0x%llx) write protect on.\n", id,
								wpg_start, wpg_end?(wpg_end-1):wpg_end, addr_start, addr_end);
						if(offset >= (MAX_DUMP_BUFF_SIZE - 200)) {
							WP_LOG_FFL(pr_err, "insufficient dump buffer");
							err =  -3;
							goto out;
						}
					}
				}
			}
		}
	}
out:
	return err;
}

static struct mmc_card *emmc_get_wp_info(void)
{
	struct msdc_host *host = atc_msdc_host[0];
	struct mmc_card *card = NULL;
	u32 *resp;
	u32 wp_grpsz = 0;
	u8 *ext_csd = (u8 *)sg_msdc_multi_buffer;

	BUG_ON(!host);
	BUG_ON(!host->mmc);
	BUG_ON(!host->mmc->card);

	card = host->mmc->card;
	if(wp_info.valid == true) {
		return card;
	}

	mmc_claim_host(host->mmc);
	if(mmc_send_ext_csd(card, ext_csd)) {
		WP_LOG_FFL(pr_err, "get ext_csd fail");
		mmc_release_host(host->mmc);
		return NULL;
	}
	mmc_release_host(host->mmc);

	wp_info.usr_wp = ext_csd[EXT_CSD_USR_WP];
	wp_info.boot_wp = ext_csd[EXT_CSD_BOOT_WP];
	wp_info.part_cfg_raw = ext_csd[EXT_CSD_PART_CONFIG];

	resp = card->raw_csd;

	if(ext_csd[EXT_CSD_ERASE_GROUP_DEF] & 0x01) {
		WP_LOG(pr_debug, "use high-capacity write protect group size definition");
		wp_info.wpg_size_in_sect = (ext_csd[EXT_CSD_HC_WP_GRP_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]) * 1024;
	} else {
		WP_LOG(pr_debug, "use old write protect group size definition");
		wp_grpsz = UNSTUFF_BITS(resp, 32, 5);
		wp_info.wpg_size_in_sect = (wp_grpsz + 1) * card->csd.erase_size;
	}
	WP_LOG(pr_info, "mmc wp group size %u blks(%u MB)", wp_info.wpg_size_in_sect , wp_info.wpg_size_in_sect / 2048);
	if(wp_info.wpg_size_in_sect == 0) {
		return NULL;
	}
	wp_info.wpg_cnt_max = card->ext_csd.sectors / wp_info.wpg_size_in_sect;

	wp_info.valid = true;
	return card;
}

static int mmc_do_write_protect(struct mmc_card *card, uint32_t addr, bool enable)
{
	int err;
	struct mmc_command cmd = {0};

	if(enable)
		cmd.opcode = MMC_SET_WRITE_PROT;
	else
		cmd.opcode = MMC_CLR_WRITE_PROT;
	cmd.arg = addr;
	cmd.flags = MMC_RSP_R1B;

	err = mmc_wait_for_cmd(card->host, &cmd, 3);
	mmc_delay(1);

	return err;
}

static int mmc_set_user_wp_by_group(struct mmc_card *card, uint32_t wpg_start, uint32_t wpg_end, bool enable)
{
	int ret = 0;
	uint32_t index;
	uint8_t value = wp_info.part_cfg_raw & 0x7;

	//switch to user area
	if(value != 0) {
		ret = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONFIG, value , card->ext_csd.part_time);
		if(ret) {
			WP_LOG_FFL(pr_err, "switch to user part fail");
			goto out;
		}
	}
	/* check args */
	if(wpg_start > wp_info.wpg_cnt_max)
		wpg_start = wp_info.wpg_cnt_max;
	if(wpg_end > wp_info.wpg_cnt_max)
		wpg_end = wp_info.wpg_cnt_max;

	for(index = wpg_start; index < wpg_end; index++) {
		ret = mmc_do_write_protect(card, wp_info.wpg_size_in_sect * index, enable);
		if(ret) {
			WP_LOG_FFL(pr_err, "mmc_do_write_protect fail on wpg %u", index);
			goto out;
		}
	}
	//verify??
out:
	return ret;
}

int mmc_set_user_temp_wp_by_region(struct mmc_card *card, uint32_t sect_start, uint32_t sect_end, bool enable, bool force_align)
{
	uint8_t value;
	int ret;
	uint32_t wpg_start, wpg_end;

	if(card->csd.mmca_vsn < 4) {
		WP_LOG_FFL(pr_err, "mmc version not support");
		ret = -2;
		goto out;
	}

	value = wp_info.usr_wp;
	value &= ~(0x5);//clear PERN_EN, PWR_EN
	mmc_claim_host(card->host);
	if(wp_info.usr_wp != value) {
		ret = mmc_switch(card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_USR_WP, value, 500);
		if(ret)
			goto out;
		else
			wp_info.usr_wp = value;
	}

	WP_LOG(pr_info, "region before align: 0x%x ~ 0x%x", sect_start, sect_end);
	if(sect_start % wp_info.wpg_size_in_sect) {
		if(force_align) {
			sect_start -= sect_start % wp_info.wpg_size_in_sect;
		} else {
			WP_LOG_FFL(pr_err, "sector addr 0x%x is not aligned to wpg size", sect_start);
			ret = -EINVAL;
			goto out;
		}
	}
	if(sect_end % wp_info.wpg_size_in_sect) {
		if(force_align) {
			sect_end += (wp_info.wpg_size_in_sect - sect_end % wp_info.wpg_size_in_sect);
		} else {
			WP_LOG_FFL(pr_err, "sector addr 0x%x is not aligned to wpg size", sect_end);
			ret = -EINVAL;
			goto out;
		}
	}
	if(sect_start > card->ext_csd.sectors)
		sect_start = card->ext_csd.sectors;
	if(sect_end > card->ext_csd.sectors)
		sect_end = card->ext_csd.sectors;
	WP_LOG(pr_info, "region after align: sect 0x%x ~ 0x%x, 0x%x", sect_start, sect_end, wp_info.wpg_size_in_sect);

	wpg_start = sect_start/wp_info.wpg_size_in_sect;
	wpg_end = sect_end/wp_info.wpg_size_in_sect;
	ret = mmc_set_user_wp_by_group(card, wpg_start, wpg_end, enable);
out:
	mmc_release_host(card->host);
	return ret;
}

int mmc_set_user_temp_wp_by_name(struct mmc_card *card, const char *partname, bool enable)
{
	int ret;
	uint32_t sect_start, sect_end;
	int whole_emmc = 0;
#ifdef ATC_LINUX_PLATFORM
	struct mmc_part_info *part_info;
#else
	struct hd_struct *hd_info;
#endif

	if(partname == NULL) {
		whole_emmc = 1;
		goto do_wp_action;
	}
#ifdef ATC_LINUX_PLATFORM
	part_info = mmc_find_part_by_name(partname);
	if(!part_info) {
		WP_LOG_FFL(pr_err, "get partition %s info fail", partname);
		ret = -1;
		goto out;
	}
	sect_start = part_info->sector_offset;
	sect_end = part_info->sector_offset + part_info->sector_size;
#else
	hd_info = mmc_find_part_by_name(partname);
	if(!hd_info) {
		WP_LOG_FFL(pr_err, "get partition %s info fail", partname);
		ret = -1;
		goto out;
	}
	sect_start = hd_info->start_sect;
	sect_end = hd_info->start_sect + hd_info->nr_sects;
#endif

do_wp_action:
	if(whole_emmc) {
		sect_start = 0;
		sect_end = card->ext_csd.sectors;
	}
	ret = mmc_set_user_temp_wp_by_region(card, sect_start, sect_end, enable, true);
out:
	return ret;
}

int mmc_clear_wp_and_save(void)
{
	int ret = -1;
	unsigned int i;
	struct mmc_card *card;
	card = emmc_get_wp_info();
	if(!card) {
		WP_LOG_FFL(pr_err, "get wp info fail");
		goto out;
	}

	ret = mmc_get_wp_region(card, NULL);
	if(ret) {
		WP_LOG_FFL(pr_err, "get wp region fail: %d", ret);
		goto out;
	}

	for(i=0; i<wp_region_save_info.region_cnt; i++) {
		ret = mmc_set_user_temp_wp_by_region(card,  wp_region_save_info.region[i].start_sector,
				wp_region_save_info.region[i].end_sector, false, false);
		if(ret) {
			WP_LOG_FFL(pr_err, "clear region wp fail:%d", ret);
			break;
		}
	}

out:
	return ret;
}

int mmc_restore_wp(void)
{
	int ret = -1;
	unsigned int i;
	struct mmc_card *card;
	card = emmc_get_wp_info();
	if(!card) {
		WP_LOG_FFL(pr_err, "get wp info fail");
		goto out;
	}

	if(wp_region_save_info.region_cnt == 0) {
		ret = 0;
		goto out;
	}

	for(i=0; i<wp_region_save_info.region_cnt; i++) {
		ret = mmc_set_user_temp_wp_by_region(card,  wp_region_save_info.region[i].start_sector,
				wp_region_save_info.region[i].end_sector, true, false);
		if(ret) {
			WP_LOG_FFL(pr_err, "restore wp fail");
			break;
		}
	}
out:
	return ret;
}
////////////////////////////////////////////////////////////////////////////////

static long simple_sd_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct msdc_ioctl msdc_ctl;
	struct wp_cmd_arg wp_arg = {0};
	char partname[64] = {0};
	int ret = -1;

	if (arg == 0) {
		switch (cmd) {
		case MSDC_REINIT_SDCARD:
			ret = sd_ioctl_reinit(&msdc_ctl);
			break;

		default:
			MSDC_LOG_NORMAL(pr_info, "mt_sd_ioctl:this opcode value is illegal!!\n");
			return -EINVAL;
		}

		return ret;

	/* ###write protect interface for recovery### */
	} else 	if(cmd == MSDC_EMMC_WRITE_PROTECT) {
		struct mmc_card *card;
		card = emmc_get_wp_info();
		if(!card) {
			WP_LOG_FFL(pr_err, "get wp info fail");
			return ret;
		}

		if(copy_from_user(&wp_arg, (struct wp_cmd_arg *)arg, sizeof(struct wp_cmd_arg))) {
			WP_LOG_FFL(pr_err, "copy_from_user fail");
			return -EFAULT;
		}
		if(wp_arg.partition_name) {
			if(copy_from_user(partname, wp_arg.partition_name, 60)) {
				WP_LOG_FFL(pr_err, "copy_from_user fail");
				return -EFAULT;
			}
		}

		if(wp_arg.wp_action == WP_PARTICIAL_ENABLE) {
			ret = mmc_set_user_temp_wp_by_name(card, partname, true);
		} else if(wp_arg.wp_action == WP_PARTICIAL_DISABLE) {
			ret = mmc_set_user_temp_wp_by_name(card, partname, false);
		} else if(wp_arg.wp_action == WP_ALL_DISABLE) {
			ret = mmc_set_user_temp_wp_by_name(card, NULL, false);
		} else if(wp_arg.wp_action == WP_CLEAR_AND_SAVE) {
			ret = mmc_clear_wp_and_save();
		} else if(wp_arg.wp_action == WP_RESTORE) {
			ret = mmc_restore_wp();
		} else if(wp_arg.wp_action == WP_REGION_ENABLE) {
			if((wp_arg.wpg_size_of_xml < (wp_info.wpg_size_in_sect/2048)) || (wp_arg.wpg_size_of_xml % (wp_info.wpg_size_in_sect/2048))) {
				WP_LOG_FFL(pr_err,"###wpg size in xml(%dMB) is not power of wpg size of emmc(%dMB), write proctec may fail",
							 wp_arg.wpg_size_of_xml, wp_info.wpg_size_in_sect/2048);
			}
			ret = mmc_set_user_temp_wp_by_region(card, wp_arg.sect_start, wp_arg.sect_end, true, false);
		} else if(wp_arg.wp_action == WP_REGIONINFO_GET) {
			wp_arg.wp_dump_info = kzalloc(MAX_DUMP_BUFF_SIZE, GFP_KERNEL);
			if(!wp_arg.wp_dump_info)
				return -ENOMEM;
			ret = mmc_get_wp_region(card, wp_arg.wp_dump_info);
			if(copy_to_user(((struct wp_cmd_arg *)arg)->wp_dump_info, wp_arg.wp_dump_info, MAX_DUMP_BUFF_SIZE)) {
				WP_LOG_FFL(pr_err, "copy_to_user fail");
				kfree(wp_arg.wp_dump_info);
				return -EFAULT;
			}
			kfree(wp_arg.wp_dump_info);
		} else {
			WP_LOG_FFL(pr_err, "unsupported action: 0x%x", wp_arg.wp_action);
			ret = -EINVAL;
		}
		return ret;
	}

	if(copy_from_user(&msdc_ctl, (struct msdc_ioctl *)arg, sizeof(struct msdc_ioctl))) {
		return -EFAULT;
	}

	switch (msdc_ctl.opcode) {

#ifdef MTK_EMMC_SUPPORT
	case MSDC_SINGLE_READ_WRITE:
		msdc_ctl.result = simple_sd_ioctl_single_rw(&msdc_ctl);
		break;

	case MSDC_MULTIPLE_READ_WRITE:
		msdc_ctl.result = simple_sd_ioctl_multi_rw(&msdc_ctl);
		break;

	case MSDC_GET_CID:
		msdc_ctl.result = simple_sd_ioctl_get_cid(&msdc_ctl);
		break;
#endif

	case MSDC_GET_CSD:
		msdc_ctl.result = simple_sd_ioctl_get_csd(&msdc_ctl);
		break;

	case MSDC_GET_EXCSD:
		msdc_ctl.result = simple_sd_ioctl_get_excsd(&msdc_ctl);
		break;

        case MSDC_ERASE_SELECTED_AREA:
		msdc_ctl.result = simple_sd_ioctl_erase_selected_area(&msdc_ctl);
		break;
#ifdef MTK_EMMC_SUPPORT
	case MSDC_DRIVING_SETTING:
		MSDC_LOG_NORMAL(pr_info, "in ioctl to change driving\n");

		if (1 == msdc_ctl.iswrite) {
			msdc_ctl.result = simple_sd_ioctl_set_driving(&msdc_ctl);
		} else {
			msdc_ctl.result = simple_sd_ioctl_get_driving(&msdc_ctl);
		}

		break;

	case MSDC_ERASE_PARTITION:
		msdc_ctl.result = simple_mmc_erase_partition_wrap(&msdc_ctl);
		break;
#endif

	case MSDC_SD30_MODE_SWITCH:
		msdc_ctl.result = simple_sd_ioctl_sd30_mswitch(&msdc_ctl);
		break;

	default:
		MSDC_LOG_NORMAL(pr_info, "simple_sd_ioctl:opcode value %d is illegal!!\n", msdc_ctl.opcode);
		return -EINVAL;
	}
	if(copy_to_user((struct msdc_ioctl *)arg, &msdc_ctl, sizeof(struct msdc_ioctl))) {
		return -EFAULT;
	}

	return msdc_ctl.result;
}

static const struct file_operations simple_msdc_em_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = simple_sd_ioctl,
	.open = simple_sd_open,
};

static struct miscdevice simple_msdc_em_dev[] = {
	{
		.minor = MISC_DYNAMIC_MINOR,
		.name = "misc-sd",
		.fops = &simple_msdc_em_fops,
	}
};

static int simple_sd_probe(struct platform_device *pdev)
{
	int ret = 0;

	MSDC_LOG_NORMAL(pr_info, "in misc_sd_probe function\n");

	return ret;
}

static int simple_sd_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver simple_sd_driver = {
	.probe = simple_sd_probe,
	.remove = simple_sd_remove,

	.driver = {
		.name = DRV_NAME_MISC,
		.owner = THIS_MODULE,
	},
};

#ifdef ATC_LINUX_PLATFORM
static struct proc_dir_entry *proc_emmc_wp_info;
#define NOT_PROTECTED	(0)
#define PART_PROTECTED	(1)
#define ALL_PROTECTED  	(2)
int check_partition_wp_status(struct boot_partition_info *info)
{

	int i;
	uint32_t part_start;
	uint32_t part_end;

	if(!info)
		return -1;

	part_start = info->part_offset;
	part_end = info->part_offset + info->part_size;
	for(i = 0; i < wp_region_save_info.region_cnt; i++) {
		if((part_start >= wp_region_save_info.region[i].start_sector) && (part_end <= wp_region_save_info.region[i].end_sector))
			return ALL_PROTECTED;
		else if(!((part_start >= wp_region_save_info.region[i].end_sector) || (part_end <= wp_region_save_info.region[i].start_sector)))
			return PART_PROTECTED;
	}
	return NOT_PROTECTED;
}

static int proc_emmc_wp_show(struct seq_file *m, void *v)
{
	/* boot_pinfo is defined in mmc/card/block.c */
	extern struct boot_partition_info boot_pinfo[32];
	const char *wp_pattern[3] = {"NOT protected", "PART protected", "ALL protected"};

	int ret = -1;
	int i = 0;
	uint64_t addr_start, addr_end;
	struct boot_partition_info *info = boot_pinfo;
	struct mmc_card *card;
	card = emmc_get_wp_info();
	if(!card) {
		WP_LOG_FFL(pr_err, "get wp info fail");
		goto out;
	}

	ret = mmc_get_wp_region(card, NULL);
	if(ret) {
		WP_LOG_FFL(pr_err, "get wp region fail: %d", ret);
		goto out;
	}
	for(i = 0; i < wp_region_save_info.region_cnt; i++) {
		addr_start = wp_region_save_info.region[i].start_sector * 512;
		addr_end = wp_region_save_info.region[i].end_sector * 512;
		seq_printf(m, "write protect region[%d]: 0x%llx ~ 0x%llx\n", i, addr_start, addr_end);
	}
	seq_printf(m, "==============================\n");

	i = 0;
	while(i < 32) {
		if(strlen(info->part_name)) {
			ret = check_partition_wp_status(info);

			/* preloader in in boot area, not protected */
			if(0 == strncmp(info->part_name, "preloader", 9))
				ret = 0;

			if(ret >= 0)
				seq_printf(m, "mmcblk0p%d: %s\t[%s]\n", i, info->part_name, wp_pattern[ret]);
			else
				seq_printf(m, "mmcblk0p%d: %s\t[unknown]\n", i, info->part_name);
		}
		else
			break;

		i++;
		info++;
	}
	ret = 0;
out:
	return ret;
}

static int proc_emmc_wp_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_emmc_wp_show, NULL);
}

static const struct file_operations proc_emmc_wp_fops = {
    .open = proc_emmc_wp_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};
#endif

static int __init simple_sd_init(void)
{
	int ret;
	char *buf;
	struct msdc_host *host = NULL;

	buf = (char *)kmalloc(64 * 1024, GFP_KERNEL);
	sg_msdc_multi_buffer = (u32 *)buf;

	if (sg_msdc_multi_buffer == NULL) {
		MSDC_LOG_NORMAL(pr_info, "allock 64KB memory failed\n");
	}

#ifdef ATC_LINUX_PLATFORM
	proc_emmc_wp_info = proc_create("emmc_wp", 0, NULL, &proc_emmc_wp_fops);
#endif

	ret = platform_driver_register(&simple_sd_driver);

	if (ret) {
		MSDC_LOG(ERR, DRV_NAME_MISC ": Can't register driver\n");
		return ret;
	}

	MSDC_LOG_NORMAL(pr_debug, DRV_NAME_MISC ": ATC simple SD/MMC Card Driver\n");

	/*msdc0 is for emmc only, just for emmc */
	/* ret = misc_register(&simple_msdc_em_dev[host->id]); */
	ret = misc_register(&(simple_msdc_em_dev[0]));

	if (ret) {
		MSDC_LOG(ERR, "register MSDC Slot[0] misc driver failed (%d)", ret);
		return ret;
	}

	return 0;
}

static void __exit simple_sd_exit(void)
{
	if (sg_msdc_multi_buffer != NULL) {
		kfree(sg_msdc_multi_buffer);
		sg_msdc_multi_buffer = NULL;
	}

	misc_deregister(&(simple_msdc_em_dev[0]));
	platform_driver_unregister(&simple_sd_driver);

#ifdef ATC_LINUX_PLATFORM
	if(proc_emmc_wp_info)
		proc_remove(proc_emmc_wp_info);
#endif
}
module_init(simple_sd_init);
module_exit(simple_sd_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple ATC SD/MMC Card Driver");
