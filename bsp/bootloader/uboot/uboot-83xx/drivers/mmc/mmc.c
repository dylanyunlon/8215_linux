/*
 * Copyright 2008, Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the Linux code
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <linux/list.h>
#include <div64.h>
#include <malloc.h>

#include "uboot_atc_msdc.h"

extern unsigned int _sdagentflag;
extern unsigned int mmc_bootup_device;

extern void flush_invalid_cache(unsigned int start, unsigned int size);
extern void flush_cache(unsigned int start, unsigned int size);

unsigned int mmc_log_disable = 0;

#define UInt32x32x32To64(a, b, c) ((unsigned long long)((unsigned long)(a)) * (unsigned long long)((unsigned long)(b)) * (unsigned long long)((unsigned long)(c)) )

static const unsigned int tran_exp[] = {
	10000,		100000,		1000000,	10000000,
	0,		0,		0,		0
};

static const unsigned char tran_mant[] = {
	0,	10,	12,	13,	15,	20,	25,	30,
	35,	40,	45,	50,	55,	60,	70,	80,
};

static const unsigned int tacc_exp[] = {
	1,	10,	100, 1000, 10000, 100000, 1000000, 10000000,
};

static const unsigned int tacc_mant[] = {
	0,	10,	12,	13,	15,	20,	25,	30,
	35,	40,	45,	50,	55,	60,	70,	80,
};

static const unsigned char mmc_tran_mant[] = {
    0,  10, 12, 13, 15, 20, 26, 30,
    35, 40, 45, 52, 55, 60, 70, 80,
};

static const u8 tuning_blk_pattern_4bit[] = {
	0xff, 0x0f, 0xff, 0x00, 0xff, 0xcc, 0xc3, 0xcc,
	0xc3, 0x3c, 0xcc, 0xff, 0xfe, 0xff, 0xfe, 0xef,
	0xff, 0xdf, 0xff, 0xdd, 0xff, 0xfb, 0xff, 0xfb,
	0xbf, 0xff, 0x7f, 0xff, 0x77, 0xf7, 0xbd, 0xef,
	0xff, 0xf0, 0xff, 0xf0, 0x0f, 0xfc, 0xcc, 0x3c,
	0xcc, 0x33, 0xcc, 0xcf, 0xff, 0xef, 0xff, 0xee,
	0xff, 0xfd, 0xff, 0xfd, 0xdf, 0xff, 0xbf, 0xff,
	0xbb, 0xff, 0xf7, 0xff, 0xf7, 0x7f, 0x7b, 0xde,
};

static const u8 tuning_blk_pattern_8bit[] = {
	0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
	0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc,
	0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
	0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
	0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
	0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
	0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
	0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
	0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
	0x00, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc,
	0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff,
	0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
	0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
	0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
	0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
	0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,
};


#define UNSTUFF_BITS(resp,start,size)					\
	({								\
		const int __size = size;				\
		const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1; \
		const int __off = 3 - ((start) / 32);			\
		const int __shft = (start) & 31;			\
		u32 __res;						\
									\
		__res = resp[__off] >> __shft;				\
		if (__size + __shft > 32)				\
			__res |= resp[__off-1] << ((32 - __shft) % 32); \
		__res & __mask; 					\
	})

//===========================================================
// Local function defines for fixing build warning
int mmc_force_reinit(struct mmc *mmc);
int mmc_go_idle(struct mmc* mmc);
int mmc_send_op_cond(struct mmc *mmc);
int mmc_get_card_registers(struct mmc *mmc);
int mmc_send_ext_csd(struct mmc *mmc, u8 *ext_csd);
int atc_config_emmc_booting(struct mmc *mmc);
//===========================================================


static struct list_head mmc_devices;
static int cur_dev_num = -1;


extern int msdc_mmc_init(bd_t *bis);

int mmc_init(struct mmc *mmc);
void mmc_set_clock(struct mmc *mmc, uint clock);
void mmc_set_bus_width(struct mmc *mmc, uint width);
int mmc_mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value);


static void mmc_dump_csd(struct mmc *mmc)
{
    struct mmc_csd *csd = &mmc->csd;
    u32 *resp = mmc->raw_csd;
    int i;
    unsigned int csd_struct;
    static char *sd_csd_ver[] = {"v1.0", "v2.0"};
    static char *mmc_csd_ver[] = {"v1.0", "v1.1", "v1.2", "Ver. in EXT_CSD"};
    static char *mmc_cmd_cls[] = {"basic", "stream read", "block read",
        "stream write", "block write", "erase", "write prot", "lock card",
        "app-spec", "I/O", "rsv.", "rsv."};
    static char *sd_cmd_cls[] = {"basic", "rsv.", "block read",
        "rsv.", "block write", "erase", "write prot", "lock card",
        "app-spec", "I/O", "switch", "rsv."};

    if (mmc->card_type == MMC_TYPE_SD){
        csd_struct = UNSTUFF_BITS(resp, 126, 2);
        printf("[CSD] CSD %s\n", sd_csd_ver[csd_struct]);
        printf("[CSD] TACC_NS: %d ns, TACC_CLKS: %d clks\n", csd->tacc_ns, csd->tacc_clks);
        if (csd_struct == 1) {
            printf("[CSD] Read/Write Blk Len = 512bytes\n");
        } else {
            printf("[CSD] Read Blk Len = %d, Write Blk Len = %d\n",
                1 << csd->read_blkbits, 1 << csd->write_blkbits);
        }
		printf("[CSD] max_dtr = %d\n", csd->max_dtr);
		printf("[CSD] Card capacity = %llu\n", csd->capacity);
        printf("[CSD] CMD Class:");
        for (i = 0; i < 12; i++) {
            if ((csd->cmdclass >> i) & 0x1)
                printf("'%s' ", sd_cmd_cls[i]);
        }
        printf("\n");
    }
	else {
        csd_struct = UNSTUFF_BITS(resp, 126, 2);
        printf("[CSD] CSD Version:		%s\n", mmc_csd_ver[csd_struct]);
        printf("[CSD] MMCA Spec :		v%d\n", csd->mmca_vsn);
        printf("[CSD] TACC_NS: %d ns, TACC_CLKS: %d clks\n", csd->tacc_ns, csd->tacc_clks);
        printf("[CSD] Read Blk Len = %d, Write Blk Len = %d\n", 1 << csd->read_blkbits, 1 << csd->write_blkbits);
		printf("[CSD] max_dtr = %d\n", csd->max_dtr);
		printf("[CSD] Card capacity = %llu\n", csd->capacity);
        printf("[CSD] CMD Class:");
        for (i = 0; i < 12; i++) {
            if ((csd->cmdclass >> i) & 0x1)
                printf("'%s' ", mmc_cmd_cls[i]);
        }
        printf("\n");
    }
}

void mmc_dump_ext_csd(struct mmc* mmc)
{
    u8 *ext_csd = &mmc->raw_ext_csd[0];
    u32 tmp;
    char *rev[] = { "4.0", "4.1", "4.2", "4.3", "Obsolete", "4.41", "4.5/4.51", "5.0" };

	if (mmc->in_ett)
		return;

    printf("===========================================================\n");
    printf("[EXT_CSD] EXT_CSD rev.              : v1.%d (MMCv%s)\n",
        ext_csd[EXT_CSD_REV], rev[ext_csd[EXT_CSD_REV]]);
    printf("[EXT_CSD] CSD struct rev.           : v1.%d\n", ext_csd[EXT_CSD_STRUCT]);
    printf("[EXT_CSD] Supported command sets    : %xh\n", ext_csd[EXT_CSD_S_CMD_SET]);
    printf("[EXT_CSD] HPI features              : %xh\n", ext_csd[EXT_CSD_HPI_FEATURE]);
    printf("[EXT_CSD] BG operations support     : %xh\n", ext_csd[EXT_CSD_BKOPS_SUPP]);
    printf("[EXT_CSD] BG operations status      : %xh\n", ext_csd[EXT_CSD_BKOPS_STATUS]);
    memcpy(&tmp, &ext_csd[EXT_CSD_CORRECT_PRG_SECTS_NUM], 4);
    printf("[EXT_CSD] Correct prg. sectors      : %xh\n", tmp);
    printf("[EXT_CSD] 1st init time after part. : %d ms\n", ext_csd[EXT_CSD_INI_TIMEOUT_AP] * 100);
    printf("[EXT_CSD] Min. write perf.(DDR,52MH,8b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_DDR_W_8_52]);
    printf("[EXT_CSD] Min. read perf. (DDR,52MH,8b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_DDR_R_8_52]);
    printf("[EXT_CSD] TRIM timeout: %d ms\n", ext_csd[EXT_CSD_TRIM_MULT] & 0xFF * 300);
    printf("[EXT_CSD] Secure feature support: %xh\n", ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT]);
    printf("[EXT_CSD] Secure erase timeout  : %d ms\n", 300 *
        ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT] * ext_csd[EXT_CSD_SEC_ERASE_MULT]);
    printf("[EXT_CSD] Secure trim timeout   : %d ms\n", 300 *
        ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT] * ext_csd[EXT_CSD_SEC_TRIM_MULT]);
    printf("[EXT_CSD] Access size           : %d bytes\n", ext_csd[EXT_CSD_ACC_SIZE] * 512);
    printf("[EXT_CSD] HC erase unit size    : %d kbytes\n", ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512);
    printf("[EXT_CSD] HC erase timeout      : %d ms\n", ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT] * 300);
    printf("[EXT_CSD] HC write prot grp size: %d kbytes\n", 512 *
        ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * ext_csd[EXT_CSD_HC_WP_GPR_SIZE]);
    printf("[EXT_CSD] HC erase grp def.     : %xh\n", ext_csd[EXT_CSD_ERASE_GRP_DEF]);
    printf("[EXT_CSD] Reliable write sect count: %xh\n", ext_csd[EXT_CSD_REL_WR_SEC_C]);
    printf("[EXT_CSD] Sleep current (VCC) : %xh\n", ext_csd[EXT_CSD_S_C_VCC]);
    printf("[EXT_CSD] Sleep current (VCCQ): %xh\n", ext_csd[EXT_CSD_S_C_VCCQ]);
    printf("[EXT_CSD] Sleep/awake timeout : %d ns\n",
        100 * (2 << ext_csd[EXT_CSD_S_A_TIMEOUT]));
    memcpy(&tmp, &ext_csd[EXT_CSD_SEC_CNT], 4);
    printf("[EXT_CSD] Sector count : %xh\n", tmp);
    printf("[EXT_CSD] Min. WR Perf.  (52MH,8b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_W_8_52]);
    printf("[EXT_CSD] Min. Read Perf.(52MH,8b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_R_8_52]);
    printf("[EXT_CSD] Min. WR Perf.  (26MH,8b,52MH,4b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_W_8_26_4_25]);
    printf("[EXT_CSD] Min. Read Perf.(26MH,8b,52MH,4b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_R_8_26_4_25]);
    printf("[EXT_CSD] Min. WR Perf.  (26MH,4b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_W_4_26]);
    printf("[EXT_CSD] Min. Read Perf.(26MH,4b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_R_4_26]);
    printf("[EXT_CSD] Power class: %x\n", ext_csd[EXT_CSD_PWR_CLASS]);
    printf("[EXT_CSD] Power class(DDR,52MH,3.6V): %xh\n", ext_csd[EXT_CSD_PWR_CL_DDR_52_360]);
    printf("[EXT_CSD] Power class(DDR,52MH,1.9V): %xh\n", ext_csd[EXT_CSD_PWR_CL_DDR_52_195]);
    printf("[EXT_CSD] Power class(26MH,3.6V)    : %xh\n", ext_csd[EXT_CSD_PWR_CL_26_360]);
    printf("[EXT_CSD] Power class(52MH,3.6V)    : %xh\n", ext_csd[EXT_CSD_PWR_CL_52_360]);
    printf("[EXT_CSD] Power class(26MH,1.9V)    : %xh\n", ext_csd[EXT_CSD_PWR_CL_26_195]);
    printf("[EXT_CSD] Power class(52MH,1.9V)    : %xh\n", ext_csd[EXT_CSD_PWR_CL_52_195]);
    printf("[EXT_CSD] Part. switch timing    : %xh\n", ext_csd[EXT_CSD_PART_SWITCH_TIME]);
    printf("[EXT_CSD] Out-of-INTR busy timing: %xh\n", ext_csd[EXT_CSD_OUT_OF_INTR_TIME]);
    printf("[EXT_CSD] Card type       : %xh\n", ext_csd[EXT_CSD_CARD_TYPE]);
    printf("[EXT_CSD] Command set     : %xh\n", ext_csd[EXT_CSD_CMD_SET]);
    printf("[EXT_CSD] Command set rev.: %xh\n", ext_csd[EXT_CSD_CMD_SET_REV]);
    printf("[EXT_CSD] HS timing       : %xh\n", ext_csd[EXT_CSD_HS_TIMING]);
    printf("[EXT_CSD] Bus width       : %xh\n", ext_csd[EXT_CSD_BUS_WIDTH]);
    printf("[EXT_CSD] Erase memory content : %xh\n", ext_csd[EXT_CSD_ERASED_MEM_CONT]);
    printf("[EXT_CSD] Partition config      : %xh\n", ext_csd[EXT_CSD_PART_CFG]);
    printf("[EXT_CSD] Boot partition size   : %d kbytes\n", ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128);
    printf("[EXT_CSD] Boot information      : %xh\n", ext_csd[EXT_CSD_BOOT_INFO]);
    printf("[EXT_CSD] Boot config protection: %xh\n", ext_csd[EXT_CSD_BOOT_CONFIG_PROT]);
    printf("[EXT_CSD] Boot bus width        : %xh\n", ext_csd[EXT_CSD_BOOT_BUS_WIDTH]);
    printf("[EXT_CSD] Boot area write prot  : %xh\n", ext_csd[EXT_CSD_BOOT_WP]);
    printf("[EXT_CSD] User area write prot  : %xh\n", ext_csd[EXT_CSD_USR_WP]);
    printf("[EXT_CSD] FW configuration      : %xh\n", ext_csd[EXT_CSD_FW_CONFIG]);
    printf("[EXT_CSD] RPMB size : %d kbytes\n", ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128);
    printf("[EXT_CSD] Write rel. setting  : %xh\n", ext_csd[EXT_CSD_WR_REL_SET]);
    printf("[EXT_CSD] Write rel. parameter: %xh\n", ext_csd[EXT_CSD_WR_REL_PARAM]);
    printf("[EXT_CSD] Start background ops : %xh\n", ext_csd[EXT_CSD_BKOPS_START]);
    printf("[EXT_CSD] Enable background ops: %xh\n", ext_csd[EXT_CSD_BKOPS_EN]);
    printf("[EXT_CSD] H/W reset function   : %xh\n", ext_csd[EXT_CSD_RST_N_FUNC]);
    printf("[EXT_CSD] HPI management       : %xh\n", ext_csd[EXT_CSD_HPI_MGMT]);
    memcpy(&tmp, &ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT], 4);
    printf("[EXT_CSD] Max. enhanced area size : %xh (%d kbytes)\n",
        tmp & 0x00FFFFFF, (tmp & 0x00FFFFFF) * 512 *
        ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    printf("[EXT_CSD] Part. support  : %xh\n", ext_csd[EXT_CSD_PART_SUPPORT]);
    printf("[EXT_CSD] Part. attribute: %xh\n", ext_csd[EXT_CSD_PART_ATTR]);
    printf("[EXT_CSD] Part. setting  : %xh\n", ext_csd[EXT_CSD_PART_SET_COMPL]);
    printf("[EXT_CSD] General purpose 1 size : %xh (%d kbytes)\n",
        (ext_csd[EXT_CSD_GP1_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_GP1_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_GP1_SIZE_MULT + 2] << 16),
        (ext_csd[EXT_CSD_GP1_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_GP1_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_GP1_SIZE_MULT + 2] << 16) * 512 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
         ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    printf("[EXT_CSD] General purpose 2 size : %xh (%d kbytes)\n",
        (ext_csd[EXT_CSD_GP2_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_GP2_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_GP2_SIZE_MULT + 2] << 16),
        (ext_csd[EXT_CSD_GP2_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_GP2_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_GP2_SIZE_MULT + 2] << 16) * 512 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
         ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    printf("[EXT_CSD] General purpose 3 size : %xh (%d kbytes)\n",
        (ext_csd[EXT_CSD_GP3_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_GP3_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_GP3_SIZE_MULT + 2] << 16),
        (ext_csd[EXT_CSD_GP3_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_GP3_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_GP3_SIZE_MULT + 2] << 16) * 512 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
         ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    printf("[EXT_CSD] General purpose 4 size : %xh (%d kbytes)\n",
        (ext_csd[EXT_CSD_GP4_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_GP4_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_GP4_SIZE_MULT + 2] << 16),
        (ext_csd[EXT_CSD_GP4_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_GP4_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_GP4_SIZE_MULT + 2] << 16) * 512 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
         ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    printf("[EXT_CSD] Enh. user area size : %xh (%d kbytes)\n",
        (ext_csd[EXT_CSD_ENH_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_ENH_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_ENH_SIZE_MULT + 2] << 16),
        (ext_csd[EXT_CSD_ENH_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_ENH_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_ENH_SIZE_MULT + 2] << 16) * 512 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
         ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    printf("[EXT_CSD] Enh. user area start: %xh\n",
        (ext_csd[EXT_CSD_ENH_START_ADDR + 0] |
         ext_csd[EXT_CSD_ENH_START_ADDR + 1] << 8 |
         ext_csd[EXT_CSD_ENH_START_ADDR + 2] << 16 |
         ext_csd[EXT_CSD_ENH_START_ADDR + 3]) << 24);
    printf("[EXT_CSD] Bad block mgmt mode: %xh\n", ext_csd[EXT_CSD_BADBLK_MGMT]);
    printf("===========================================================\n");
}

static u32 u4Erase_Group_Size = 0;
static u32 u4Erase_Group_Mult = 0;
static u32 u4HC_Erase_Group_Size = 0;

static void mmc_dump_erase_releated_params(struct mmc *mmc)
{
	if (mmc->in_ett)
		return;

	printf("===========================================================\n");
	printf("============   eMMC Erase Releated Parameters  ============\n");
	printf("===========================================================\n");
	printf("eMMC Support Erase: 0x%08X\n", 		mmc->raw_ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT]);
	printf("After Erase Content: 0x%08X\n", 	mmc->raw_ext_csd[EXT_CSD_ERASED_MEM_CONT]);
	printf("Erase Group Def: 0x%08X\n", 		mmc->raw_ext_csd[EXT_CSD_ERASE_GRP_DEF]);
	printf("ERASE_TIMEOUT_MULT: 0x%08X\n", 		mmc->raw_ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT]);
	printf("HC_ERASE_GRP_SIZE: 0x%08X\n", 		mmc->raw_ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
	printf("SEC_TRIM_MULT: 0x%08X\n", 			mmc->raw_ext_csd[EXT_CSD_SEC_TRIM_MULT]);
	printf("SEC_ERASE_MULT: 0x%08X\n", 			mmc->raw_ext_csd[EXT_CSD_SEC_ERASE_MULT]);

	printf("ERASE_GRP_SIZE: 0x%08X\n", 			u4Erase_Group_Size);
	printf("ERASE_GRP_MULT: 0x%08X\n", 			u4Erase_Group_Mult);
	printf("ERASE Unit Size: 0x%08X\n", 		mmc->csd.erase_sctsz);
	printf("WRITE_BL_LEN: 0x%08X\n", 			mmc->csd.write_blkbits);

	printf("===========================================================\n");
}


int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	int err;
    int retry = cmd->retries;

    do {
        err = mmc->send_cmd(mmc, cmd, data);
        if (err == MMC_ERR_NONE)
            break;
		if (err == (-MMC_ERR_ENOMEDIUM))
            break;
	if((err == MMC_ERR_TIMEOUT)
		&& (mmc->host_inited == 0)
		&& (mmc->host_id == 0)
		&& (cmd->opcode == MMC_CMD_SEND_EXT_CSD)
		&& (mmc->clock <= MSDC_INIT_CLOCK))
		break;
    } while(retry--);

    return err;
}

int mmc_set_blocklen(struct mmc *mmc, int len)
{
	struct mmc_cmd cmd;

	cmd.opcode 	= MMC_CMD_SET_BLOCKLEN;
	cmd.rsptype = MMC_RSP_R1;
	cmd.arg 	= len;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

	return mmc_send_cmd(mmc, &cmd, NULL);
}

struct mmc *find_mmc_device(int dev_num)
{
	struct mmc *mmc = NULL;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		mmc = list_entry(entry, struct mmc, link);

		if (mmc->block_dev.dev == dev_num)
			return mmc;
	}

	MMC_LOG(MMC_LOG_ERR, "MMC Device %d not found", dev_num);

	#ifdef STALL_SLOT0_ERROR
	if (dev_num == 0)
	{while(1){};}
	#endif

	return NULL;
}

int mmc_go_idle_state(struct mmc* mmc, u32 cmd_arg)
{
	int err = 0;
	struct mmc_cmd cmd;
	memset(&cmd, 0, sizeof(struct mmc_cmd));

	cmd.opcode 	= MMC_CMD_GO_IDLE_STATE;
	cmd.rsptype = MMC_RSP_NONE;
	cmd.arg 	= cmd_arg;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err  = mmc_send_cmd(mmc, &cmd, NULL);
    return err;
}

void mmc_dump_card_status(u32 card_status)
{
    msdc_dump_card_status(card_status);
}

static void mmc_dump_ocr_reg(u32 resp)
{
    msdc_dump_ocr_reg(resp);
}

static void mmc_dump_rca_resp(u32 resp)
{
    msdc_dump_rca_resp(resp);
}

int mmc_send_status(struct mmc *mmc, u32 *status)
{
    int err;
    struct mmc_cmd cmd;

    cmd.opcode  = MMC_CMD_SEND_STATUS;
    cmd.arg     = mmc->rca << 16;
    cmd.rsptype = MMC_RSP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

	*status = 0;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err == MMC_ERR_NONE) {
        *status = cmd.resp[0];
        #if 0
        mmc_dump_card_status(*status);
        #endif
    }
    return err;
}


int mmc_send_stopcmd(struct mmc *mmc)
{
    int err;
    struct mmc_cmd stop;

	stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
	stop.rsptype = MMC_RSP_R1b;
	stop.arg	 = 0;
	stop.retries = 0;
	stop.timeout = CMD_TIMEOUT;
	err = mmc_send_cmd(mmc, &stop, NULL);

    return err;
}



int mmc_mmc_set_ext_csd(struct mmc* mmc, u32 access, u32 index, u32 value)
{
	int err = 0;
	u32 status = 0;
	struct mmc_cmd cmd;
	memset(&cmd, 0, sizeof(struct mmc_cmd));

    cmd.opcode 	= MMC_CMD_SWITCH;
	cmd.rsptype = MMC_RSP_R1b;
    cmd.arg 	= CMD6_ARGS(access, index, value);
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
	{
		return err;
	}

	do {
        err = mmc_send_status(mmc, &status);
        if (err)
		{
            MMC_LOG(MMC_LOG_ERR, "fail to send status %d", err);
            break;
        }
        if (status & R1_SWITCH_ERROR)
		{
            MMC_LOG(MMC_LOG_ERR, "switch error. arg(0x%x)\n", cmd.arg);
            return 1;
        }
    } while (!(status & R1_READY_FOR_DATA) || (R1_CURRENT_STATE(status) == 7));

    return err;
}

int _mmc_set_ext_csd(struct mmc* mmc, u32 index, u32 value, u32 mask)
{
	int err = 0;

	err = mmc_mmc_set_ext_csd(mmc, EXT_CSD_ACCESS_MODE_CLEARBITS, index, mask);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "send CMD6 to clear partition select failed");
	}

	err = mmc_mmc_set_ext_csd(mmc, EXT_CSD_ACCESS_MODE_SETBITS, index, value);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "send CMD6 to set partition select failed");
	}

	return err;
}

int _mmc_boot_setting(struct mmc* mmc, u32 boot_part_num, u8 bus_width)
{
    u8 partition_config = 0, boot_bus_width = 0;
    int err = 0;

    /* step 1 - boot area setting */
    //1.1 - partition configuration setting
    if (boot_part_num == BOOT_PART_PART1)
    {
    	partition_config |= EXT_CSD_179_PARTITION_ACCESS_BOOT1;
    }
	else if (boot_part_num == BOOT_PART_PART2)
	{
		partition_config |= EXT_CSD_179_PARTITION_ACCESS_BOOT2;
	}
	else
	{
		partition_config |= EXT_CSD_179_PARTITION_ACCESS_NO_BOOT;
	}

    //1.2 - boot bus width setting
    if (bus_width == 1)
    {
		boot_bus_width = EXT_CSD_177_BOOT_BUS_WITDH_X1;
    }
	else if (bus_width == 4)
    {
		boot_bus_width = EXT_CSD_177_BOOT_BUS_WITDH_X4;
    }
	else if (bus_width == 8)
    {
		boot_bus_width = EXT_CSD_177_BOOT_BUS_WITDH_X8;
	}

    //1.3 - programming PARTITION_CONFIG in emmc device
    if(_mmc_set_ext_csd(mmc, 179, partition_config, 0))
    {
        MMC_LOG(MMC_LOG_ERR, "programming ext_csd[179] by %02x failed", partition_config);
        err = 1;
        goto end;
    }

    //1.4 - programming BOOT_BUS_WIDTH in emmc device
    if(_mmc_set_ext_csd(mmc, 177, boot_bus_width, 0))
    {
        MMC_LOG(MMC_LOG_ERR, "programming ext_csd[177] by %02x failed", boot_bus_width);
        err = 1;
        goto end;
    }


end:
    return err;
}

int _mmc_boot_go_preidle_state(struct mmc* mmc, u32 reset_mode)
{
	int err = 0;
	if(reset_mode == BOOT_CONFIG_RESET_SOFTWARE)
    {
        err = mmc_go_idle_state(mmc, 0xF0F0F0F0);
        if(err)
        {
            MMC_LOG(MMC_LOG_ERR, "send CMD0 + 0xF0F0F0F0 to let card go into preidle state failed");
            err = 1;
            goto end;
        }
    }
    else if(reset_mode == BOOT_CONFIG_RESET_HARDWARE)
    {
        // TO-DO
        MMC_LOG(MMC_LOG_ERR, "not support hardware reset to let card go into preidle state failed");
		err = 1;
    }
    else
    {
        MMC_LOG(MMC_LOG_ERR, "unknown reset mode");
        err = 1;
        goto end;
    }

end:

	return err;
}

int _mmc_boot_go_boot_state(struct mmc* mmc)
{
	int err = 0;
	//err = msdc_boot_go_boot_state();
	return err;
}

int _mmc_boot_start(struct mmc* mmc)
{
	int err = 0;

	// step 1 - force card enters "Pre-idle" state
	err = _mmc_boot_go_preidle_state(mmc, BOOT_CONFIG_RESET_SOFTWARE);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "make card enters pre-idle state failed.");
		return err;
	}

	// step 2 - force card enters "boot-mode" state
	err = _mmc_boot_go_boot_state(mmc);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "make card enters boot-mode state failed");
		return err;
	}


	return err;
}

int emmc_init(struct mmc *mmc)
{
	int err;
	int i;

	// if already init, return directly
	if (mmc->host_inited && !mmc->cur_ddr_mode)
	{
		return 0;
	}

	err = mmc->init(mmc);
	//in cmd, the clk should set to 100khz - 400khz
	mmc_set_clock(mmc, MSDC_INIT_CLOCK);
	mmc_set_bus_width(mmc, 1);

	//Reset the Card
	for (i=0; i<5; i++)
	{
		err = mmc_go_idle(mmc);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "reset card error: err = %d", err);
			return err;
		}
	}

	err = mmc_send_op_cond(mmc);
	if (err == 0)
	{
		mmc->card_type = MMC_TYPE_MMC;
	}
	else
	{
		MMC_LOG(MMC_LOG_ERR, "it is not emmc");
		return 1;
	}

	err = mmc_get_card_registers(mmc);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "get registers from mmc failed");
	}

	err = mmc_send_ext_csd(mmc, &mmc->raw_ext_csd[0]);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "get ext_csd from mmc failed");
		return 2;
	}

	if (mmc->raw_ext_csd[212] || mmc->raw_ext_csd[213] || mmc->raw_ext_csd[214] || mmc->raw_ext_csd[215])
	{
		MMC_LOG(MMC_LOG_ERR, "card is high_capacity");
		mmc->high_capacity = 1;
	}
	// dump ext_csd register
	//mmc_dump_ext_csd(mmc);

	return 0;
}

int mmc_boot_enter_bootmode(int dev_num)
{
	int err = 0;
	struct mmc *mmc = find_mmc_device(dev_num);
	if (!mmc)
	{
		MMC_LOG(MMC_LOG_ERR, "find mmc%d device failed", dev_num);
		return -1;
	}

	err = emmc_init(mmc);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "init emmc failed");
		return 5;
	}

	// Change Clock
	err = mmc_mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, EXT_CSD_TIMING_DF);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "switch high speed for emmc failed");
		return err;
	}
	mmc_set_clock(mmc, EMMC_BOOT_OP_CLOCK);

	if (mmc->raw_ext_csd[EXT_CSD_BOOT_SIZE_MULT] == 0)
	{
		MMC_LOG(MMC_LOG_ERR, "boot partition size is zero");
		return 1;
	}
	else
	{
		MMC_LOG(MMC_LOG_ERR, "boot partition size is %dKB", mmc->raw_ext_csd[EXT_CSD_BOOT_SIZE_MULT]*128);
	}

	//mmc_dump_ext_csd(mmc);

	err = _mmc_set_ext_csd(mmc, 179, EXT_CSD_179_BOOTPARTITION_BOOT1, EXT_CSD_179_BOOTPARTITION_MASK);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "set ext_csd for boot partition op failed");
		return 2;
	}

	// set bus width, 4bit
	#if 1

	err = _mmc_set_ext_csd(mmc, 177, EXT_CSD_177_BOOT_BUS_WITDH_X4, EXT_CSD_177_BOOT_BUS_WITDH);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "select 4bit bus width failed");
		return 3;
	}
	err = _mmc_set_ext_csd(mmc, 183, EXT_CSD_177_BOOT_BUS_WITDH_X4, 0xFF);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "select 4bit bus width failed");
		return 4;
	}
	mmc_set_bus_width(mmc, 4);

	MMC_LOG(MMC_LOG_ERR, "boot mode with 4 bit bus width");

	#else

	err = _mmc_set_ext_csd(mmc, 177, EXT_CSD_177_BOOT_BUS_WITDH_X1, EXT_CSD_177_BOOT_BUS_WITDH);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "select 1bit bus width failed");
		return 3;
	}
	err = _mmc_set_ext_csd(mmc, 183, EXT_CSD_177_BOOT_BUS_WITDH_X1, 0xFF);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "select 1bit bus width failed");
		return 4;
	}
	mmc_set_bus_width(mmc, 1);
	MMC_LOG(MMC_LOG_ERR, "boot mode with 1 bit bus width");

	#endif

	// set to high speed timings
	err = _mmc_set_ext_csd(mmc, 177, EXT_CSD_177_BOOT_SPEED_MODE_HS, EXT_CSD_177_BOOT_SPEED_MODE);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "select high speed timings failed");
		return 5;
	}


	mmc->in_boot_mode = 1;
	return err;
}

int mmc_boot_exit_bootmode(int dev_num)
{
	int err = 0;

	struct mmc *mmc = find_mmc_device(dev_num);
	if (!mmc)
	{
		MMC_LOG(MMC_LOG_ERR, "find mmc%d device failed", dev_num);
		return -1;
	}

	// re-init card
	err = mmc_force_reinit(mmc);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "re-init card failed");
		return 1;
	}

	mmc->in_boot_mode = 0;
	return err;
}

ulong mmc_boot_bwrite(int dev_num, int boot_part_num, ulong start, lbaint_t blkcnt, const void*src)
{
	int err = 0;
	ulong real_block = 0;

	struct mmc *mmc = find_mmc_device(dev_num);
	if (!mmc)
	{
		MMC_LOG(MMC_LOG_ERR, "find mmc%d device failed", dev_num);
		return -1;
	}

	if (mmc->in_boot_mode == 0)
	{
		MMC_LOG(MMC_LOG_ERR, "emmc is not in boot mode");
		return -1;
	}

	// maybe need double check setting result
	err = _mmc_set_ext_csd(mmc, 179, boot_part_num, EXT_CSD_179_BOOTPARTITION_MASK);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "select boot partition(%d) failed", boot_part_num);
		return 0;
	}
	//printf("--------------> write boot partition %d <--------------\n", boot_part_num);

	real_block = mmc_bwrite(dev_num, start, blkcnt, src);
	if (real_block != blkcnt)
	{
		MMC_LOG(MMC_LOG_ERR, "write error, want to write %lu block, but real write %lu block", blkcnt, real_block);
	}

	return real_block;
}

ulong mmc_boot_bread(int dev_num, int boot_part_num, ulong start, lbaint_t blkcnt, void *dst)
{
	int err = 0;
	ulong real_block = 0;

	struct mmc *mmc = find_mmc_device(dev_num);
	if (!mmc)
	{
		MMC_LOG(MMC_LOG_ERR, "find mmc%d device failed", dev_num);
		return -1;
	}

	if (mmc->in_boot_mode == 0)
	{
		MMC_LOG(MMC_LOG_ERR, "emmc is not in boot mode");
		return -1;
	}

	// maybe need double check setting result
	err = _mmc_set_ext_csd(mmc, 179, boot_part_num, EXT_CSD_179_BOOTPARTITION_MASK);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "select boot partition(%d) failed", boot_part_num);
		return 0;
	}
	// set bus width, TODO

	real_block = mmc_bread(dev_num, start, blkcnt, dst);
	if (real_block != blkcnt)
	{
		MMC_LOG(MMC_LOG_ERR, "read error, want to read %lu block, but real read %lu block", blkcnt, real_block);
	}

	return real_block;
}


ulong mmc_bwrite(int dev_num, ulong start, lbaint_t blkcnt, const void* src)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	lbaint_t trans_blk_cnt;
	int err;
	int stoperr = 0;
	int blklen;
	u32 write_opcode;

	struct mmc *mmc = find_mmc_device(dev_num);
	if (!mmc)
	{
		MMC_LOG(MMC_LOG_ERR, "find mmc%d device failed", dev_num);
		return -1;
	}

	if (0 == blkcnt)
	{
		MMC_LOG(MMC_LOG_ERR, "write block count is 0.");
		return 0;
	}

	blklen = mmc->write_bl_len = 512;

	err = mmc_set_blocklen(mmc, mmc->write_bl_len);
	if (err) {
		MMC_LOG(MMC_LOG_ERR, "set write block len failed: err = %d, block_len = %d", err, mmc->write_bl_len);
		return err;
	}

	trans_blk_cnt = blkcnt;
	while(trans_blk_cnt)
	{
		if(trans_blk_cnt >= MAX_TRANSFER_BLOCK)
	 		data.blocks = MAX_TRANSFER_BLOCK;
		else
			data.blocks = trans_blk_cnt;

		data.blocksize = mmc->write_bl_len = 512;

		if (data.blocks > 1)
			cmd.opcode = MMC_CMD_WRITE_MULTIPLE_BLOCK;
		else
			cmd.opcode = MMC_CMD_WRITE_SINGLE_BLOCK;

		if (mmc->high_capacity)
			cmd.arg = start;
		else
			cmd.arg = start * blklen;

		cmd.rsptype = MMC_RSP_R1;

		data.src = src;
		data.blocksize = blklen;
		data.flags = MMC_DATA_WRITE;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		write_opcode = cmd.opcode;

		err = mmc_send_cmd(mmc, &cmd, &data);
		if (err) {
			MMC_LOG(MMC_LOG_ERR, "write failed: err = %d", err);
			return err;
		}

		#if ((MSDC_AUTOCMD12_EN == 0)&&(MSDC_AUTOCMD23_EN == 0))
		// Send Stop Command (CMD12)
		if (data.blocks > 1) {
			cmd.opcode 	= MMC_CMD_STOP_TRANSMISSION;
			cmd.arg 	= 0;
			cmd.rsptype = MMC_RSP_R1b;
			cmd.retries = CMD_RETRIES;
			cmd.timeout = CMD_TIMEOUT;
			stoperr = mmc_send_cmd(mmc, &cmd, NULL);
			if((stoperr == MMC_ERR_NONE) && (cmd.resp[0] & R1_WP_VIOLATION)) {
				err = MMC_ERR_WP_VIOLATION;
				goto out;
			}
		} else {
			err = msdc_get_err_from_card_status(mmc);
			if(err == MMC_ERR_WP_VIOLATION)
				goto out;
		}
		#else
		/*if(data.blocks > 1) {
			if(MSDC_READ32(SDC_ACMD_RESP) & R1_WP_VIOLATION) {
				err = MMC_ERR_WP_VIOLATION;
				goto out;
			}
		} else { */
			err = msdc_get_err_from_card_status(mmc);
			if(err == MMC_ERR_WP_VIOLATION)
				goto out;
		//}
		#endif

		start += data.blocks;
		trans_blk_cnt -= data.blocks;
		src = (void *)((unsigned char *)src + (512 * data.blocks));

	}

	return blkcnt;
out:
	SD_LOG(SD_LOG_ERROR, "%s: error %d, <CMD%d, BLK %d>\n", (err == MMC_ERR_WP_VIOLATION) ? "###Write protect violation###":"Unknown error",
                           err, write_opcode, data.blocks);

	return err;
}

ulong mmc_bread(int dev_num, ulong start, lbaint_t blkcnt, void *dst)
{
	int err;
	lbaint_t trans_blk_cnt;
	int ret;
	struct mmc_cmd cmd;
	struct mmc_data data;

	struct mmc *mmc = find_mmc_device(dev_num);
	if (!mmc) {
		MMC_LOG(MMC_LOG_ERR, "find mmc%d device failed", dev_num);
		return 0;
	}

	if (0 == blkcnt) {
		MMC_LOG(MMC_LOG_ERR, "read block count is 0");
		return 0;
	}

	mmc->read_bl_len = 512;

	/* We always do full block reads from the card */
	err = mmc_set_blocklen(mmc, mmc->read_bl_len);
	if (err) {
		MMC_LOG(MMC_LOG_ERR, "set read block len failed: err = %d, block_len = %d", err, mmc->read_bl_len);
		return 0;
	}

    //Make Max Block count = 1024, need consider timeout setting
	trans_blk_cnt = blkcnt;
	while(trans_blk_cnt)
	{
		data.dest = dst;
		if(trans_blk_cnt >= MAX_TRANSFER_BLOCK)
	 		data.blocks = MAX_TRANSFER_BLOCK;
		else
			data.blocks = trans_blk_cnt;

		data.blocksize = mmc->read_bl_len = 512;
		data.flags = MMC_DATA_READ;
		if (data.blocks > 1)
			cmd.opcode = MMC_CMD_READ_MULTIPLE_BLOCK;
		else
			cmd.opcode = MMC_CMD_READ_SINGLE_BLOCK;

		if (mmc->high_capacity)
			cmd.arg = start;
		else
			cmd.arg = start * mmc->read_bl_len;

		cmd.rsptype = MMC_RSP_R1;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, &data);
		if (err) {
			MMC_LOG(MMC_LOG_ERR, "read failed: err = %d", err);
			return err;
		}

		#if ((MSDC_AUTOCMD12_EN == 0)&&(MSDC_AUTOCMD23_EN == 0))
		// Send Stop Command (CMD12)
		if (data.blocks > 1) {
			cmd.opcode 	= MMC_CMD_STOP_TRANSMISSION;
			cmd.arg 	= 0;
			cmd.rsptype = MMC_RSP_R1b;
			cmd.retries = CMD_RETRIES;
    		cmd.timeout = CMD_TIMEOUT;
			ret = mmc_send_cmd(mmc, &cmd, NULL);
		}
		#endif

		start += data.blocks;
		trans_blk_cnt -= data.blocks;
		dst = (void *)((unsigned char *)dst + (512 * data.blocks));

	}

	return blkcnt;
}

// ========================================================================
// ETT Releated Functions  (Begin)
// ========================================================================
#ifdef CONFIG_MSDC_ETT

ulong mmc_ett_bread(int dev_num, ulong start, lbaint_t blkcnt, void *dst)
{
	int err;
	lbaint_t trans_blk_cnt;
	int ret;
	struct mmc_cmd cmd;
	struct mmc_data data;

	struct mmc *mmc = find_mmc_device(dev_num);
	mmc->read_bl_len = 512;

	if (0 == blkcnt) {
		MMC_LOG(MMC_LOG_ERR, "read block count is 0");
		return 0;
	}

	if (!mmc) {
		MMC_LOG(MMC_LOG_ERR, "find mmc%d device failed", dev_num);
		return 0;
	}

	/* We always do full block reads from the card */
	err = mmc_set_blocklen(mmc, mmc->read_bl_len);
	if (err) {
		MMC_LOG(MMC_LOG_ERR, "set read block len failed: err = %d, block_len = %d", err, mmc->read_bl_len);
		return 0;
	}

    //Make Max Block count = 1024, need consider timeout setting
	trans_blk_cnt = blkcnt;
	while(trans_blk_cnt)
	{
		data.dest = dst;
		if(trans_blk_cnt >= MAX_TRANSFER_BLOCK)
	 		data.blocks = MAX_TRANSFER_BLOCK;
		else
			data.blocks = trans_blk_cnt;

		data.blocksize = mmc->read_bl_len = 512;
		data.flags = MMC_DATA_READ;
		if (data.blocks > 1)
			cmd.opcode = MMC_CMD_READ_MULTIPLE_BLOCK;
		else
			cmd.opcode = MMC_CMD_READ_SINGLE_BLOCK;

		if (mmc->high_capacity)
			cmd.arg = start;
		else
			cmd.arg = start * mmc->read_bl_len;

		cmd.rsptype = MMC_RSP_R1;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = msdc_ett_read(mmc, &cmd, &data);
		if (err) {
			MMC_LOG(MMC_LOG_ERR, "read failed: err = %d", err);
			return err;
		}

		#if ((MSDC_AUTOCMD12_EN == 0)&&(MSDC_AUTOCMD23_EN == 0))
		// Send Stop Command (CMD12)
		if (data.blocks > 1) {
			cmd.opcode 	= MMC_CMD_STOP_TRANSMISSION;
			cmd.arg 	= 0;
			cmd.rsptype = MMC_RSP_R1b;
			cmd.retries = CMD_RETRIES;
    		cmd.timeout = CMD_TIMEOUT;
			ret = mmc_send_cmd(mmc, &cmd, NULL);
		}
		#endif

		start += data.blocks;
		trans_blk_cnt -= data.blocks;
		dst = (void *)((unsigned char *)dst + (512 * data.blocks));

	}

	return blkcnt;
}


ulong mmc_ett_bwrite(int dev_num, ulong start, lbaint_t blkcnt, const void* src)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	lbaint_t trans_blk_cnt;
	int err;
	int stoperr = 0;
	int blklen;

	struct mmc *mmc = find_mmc_device(dev_num);
	if (!mmc)
	{
		MMC_LOG(MMC_LOG_ERR, "find mmc%d device failed", dev_num);
		return -1;
	}

	if (0 == blkcnt)
	{
		MMC_LOG(MMC_LOG_ERR, "write block count is 0.");
		return 0;
	}

	blklen = mmc->write_bl_len = 512;

	err = mmc_set_blocklen(mmc, mmc->write_bl_len);
	if (err) {
		MMC_LOG(MMC_LOG_ERR, "set write block len failed: err = %d, block_len = %d", err, mmc->write_bl_len);
		return err;
	}

	trans_blk_cnt = blkcnt;
	while(trans_blk_cnt)
	{
		if(trans_blk_cnt >= MAX_TRANSFER_BLOCK)
	 		data.blocks = MAX_TRANSFER_BLOCK;
		else
			data.blocks = trans_blk_cnt;

		data.blocksize = mmc->write_bl_len = 512;

		if (data.blocks > 1)
			cmd.opcode = MMC_CMD_WRITE_MULTIPLE_BLOCK;
		else
			cmd.opcode = MMC_CMD_WRITE_SINGLE_BLOCK;

		if (mmc->high_capacity)
			cmd.arg = start;
		else
			cmd.arg = start * blklen;

		cmd.rsptype = MMC_RSP_R1;

		data.src = src;
		data.blocksize = blklen;
		data.flags = MMC_DATA_WRITE;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = msdc_ett_write(mmc, &cmd, &data);
		if (err) {
			MMC_LOG(MMC_LOG_ERR, "write failed: err = %d", err);
			return err;
		}

		#if ((MSDC_AUTOCMD12_EN == 0)&&(MSDC_AUTOCMD23_EN == 0))
		// Send Stop Command (CMD12)
		if (data.blocks > 1) {
			cmd.opcode 	= MMC_CMD_STOP_TRANSMISSION;
			cmd.arg 	= 0;
			cmd.rsptype = MMC_RSP_R1b;
			cmd.retries = CMD_RETRIES;
    		cmd.timeout = CMD_TIMEOUT;
			stoperr = mmc_send_cmd(mmc, &cmd, NULL);
		}
		#endif

		start += data.blocks;
		trans_blk_cnt -= data.blocks;
		src = (void *)((unsigned char *)src + (512 * data.blocks));

	}

	return blkcnt;
}

#define MSDC_ETT_READ			(0)
#define MSDC_ETT_WRITE			(1)
#define MSDC_ETT_CMD			(2)

int mmc_ett(struct mmc* mmc, int devnum, int ett_type)
{
	char* buf = malloc(ETT_MAX_TRANSFER_BLOCK * 512);
	if (!buf)
	{
		printf("=====> Allocate Buffer for MSDC ETT Test Failed <=====\n");
	}

	if (ett_type == MSDC_ETT_READ)
	{
		printf("=====> msdc%d read ett: clock<%dKHz> real_clock<%dHz> pre_setting %s <=====\n",
					devnum, mmc->work_clock_freq/1000, mmc->real_clock/1000,  mmc->host_hw->read_pre_setting_en ? "enable" : "disable");

		// Read from 1GB offset, size is ETT_MAX_TRANSFER_BLOCK
		mmc_ett_bread(devnum, 0x200000, ETT_MAX_TRANSFER_BLOCK, buf); // 0x200000 *512
	}
	else if (ett_type == MSDC_ETT_WRITE)
	{
		printf("=====> msdc%d write ett: clock<%dKHz> real_clock<%dHz> pre_setting %s <=====\n",
					devnum, mmc->work_clock_freq/1000, mmc->real_clock/1000, mmc->host_hw->write_pre_setting_en ? "enable" : "disable");

		// Write to 3GB offset, size is ETT_MAX_TRANSFER_BLOCK
		mmc_ett_bwrite(devnum, 0x600000, ETT_MAX_TRANSFER_BLOCK, buf); // 0x600000 * 512
	}
	else if (ett_type == MSDC_ETT_CMD)
	{
	}
	else
	{
		// Wrong Type!!!
	}

	free(buf);

	return MMC_ERR_NONE;
}

int mmc_set_rw_pre_setting_en(struct mmc* mmc, int rw, int enable)
{
	if (rw == 0) // Read
	{
		msdc_host_hw_read_setting_en(mmc, enable);
	}
	else if (rw == 1) // Write
	{
		msdc_host_hw_write_setting_en(mmc, enable);
	}
	else if (rw == 2) // Read & Write
	{
		msdc_host_hw_read_setting_en(mmc, enable);
		msdc_host_hw_write_setting_en(mmc, enable);
	}

	return MMC_ERR_NONE;
}

int mmc_get_rw_pre_setting_en(struct mmc* mmc)
{
	printf("=====> msdc%d read pre_setting enable = %d <=====\n", mmc->host_id, mmc->host_hw->read_pre_setting_en);
	printf("=====> msdc%d write pre_setting enable = %d <=====\n", mmc->host_id, mmc->host_hw->write_pre_setting_en);

	return MMC_ERR_NONE;
}

#define MSDC_HQA_BLOCK_CNT		(ETT_MAX_TRANSFER_BLOCK * 2 * 4) //4MB
int mmc_hqa_test(struct mmc* mmc, int rw, int times)
{
	char* buf = malloc(MSDC_HQA_BLOCK_CNT * 512);
	char* org_buf = buf;
	if (!buf){
		printf("=====> msdc%d start hqa test, allocate buffer failed. <=====\n", mmc->host_id);
		return -1;
	}

	printf("=====> msdc%d start hqa test <=====\n", mmc->host_id);

	mmc_set_rw_pre_setting_en(mmc, 2, 1);	// Enable Pre-Setting for RW OPs

	if (rw == 0)
	{
		printf("=====> msdc%d read : clock<%dKHz> real_clock<%dHz> pre_setting %s <=====\n",
					mmc->host_id, mmc->work_clock_freq/1000, mmc->real_clock/1000,  mmc->host_hw->read_pre_setting_en ? "enable" : "disable");
		while(times > 0)
		{
			buf = org_buf;
			// Read from 1GB offset, size is MSDC_HQA_BLOCK_CNT
			mmc_bread(mmc->host_id, 0x200000, MSDC_HQA_BLOCK_CNT, buf); // 0x200000 *512
			times--;
		}
	}
	else if (rw == 1)
	{
		printf("=====> msdc%d write : clock<%dKHz> real_clock<%dHz> pre_setting %s <=====\n",
					mmc->host_id, mmc->work_clock_freq/1000, mmc->real_clock/1000, mmc->host_hw->write_pre_setting_en ? "enable" : "disable");
		while(times > 0)
		{
			buf = org_buf;
			// Write to 3GB offset, size is MSDC_HQA_BLOCK_CNT
			mmc_bwrite(mmc->host_id, 0x600000, MSDC_HQA_BLOCK_CNT, buf); // 0x600000 * 512
			times--;
		}
	}

	printf("=====> msdc%d end hqa test <=====\n", mmc->host_id);

	free(buf);

	return MMC_ERR_NONE;
}

#define MSDC_PATTERN_BLOCK_CNT		(ETT_MAX_TRANSFER_BLOCK * 2) // 1MB
int mmc_pattern_test(struct mmc* mmc)
{
	ulong pattern_start = 0;
	ulong pattern_size = 0;

	char* buf = malloc(MSDC_PATTERN_BLOCK_CNT * 512);
	char* org_buf = buf;
	if (!buf)
	{
		printf("=====> msdc%d start pattern test, allocate buffer failed. <=====\n", mmc->host_id);
		return -1;
	}

	// Set Pattern
	memset(buf, 0x55, MSDC_PATTERN_BLOCK_CNT * 512); // Pattern is 0x55 OR 0xAA

	printf("=====> msdc%d start pattern test <=====\n", mmc->host_id);

	mmc_set_rw_pre_setting_en(mmc, 2, 1);	// Enable Pre-Setting for RW OPs

	printf("=====> msdc%d write : clock<%dKHz> real_clock<%dHz> pre_setting %s <=====\n",
			mmc->host_id, mmc->work_clock_freq/1000, mmc->real_clock/1000, mmc->host_hw->write_pre_setting_en ? "enable" : "disable");

	// Write Pattern to 3GB offset, size is 1GB
	pattern_start = 0x600000;
	pattern_size = 0x200000;
	while(pattern_size)
	{
		buf = org_buf;
		mmc_bwrite(mmc->host_id, pattern_start, MSDC_PATTERN_BLOCK_CNT, buf);
		pattern_start += MSDC_PATTERN_BLOCK_CNT;
		pattern_size -= MSDC_PATTERN_BLOCK_CNT;
	}

	printf("=====> msdc%d end pattern test <=====\n", mmc->host_id);

	free(buf);

	return MMC_ERR_NONE;
}


int mmc_set_hs200_mode(struct mmc* mmc, u32 enable)
{
	printf("=====> msdc%d enter hs200 mode = %d change to %d <=====\n", mmc->host_id, mmc->switch_hs200, enable);
	mmc->switch_hs200 = enable;
	return MMC_ERR_NONE;
}

int mmc_get_hs200_mode(struct mmc* mmc)
{
	printf("=====> msdc%d enter hs200 mode = %d <=====\n", mmc->host_id, mmc->switch_hs200);
	return MMC_ERR_NONE;
}

int mmc_set_clock_source(struct mmc* mmc, u32 clock, u32 reinit)
{
	msdc_change_clock_source(mmc, clock);
	if (reinit)
	{
		mmc_force_reinit(mmc);
	}
	return MMC_ERR_NONE;
}

int mmc_get_clock_source(struct mmc* mmc)
{
	msdc_show_clock_source(mmc);
	return MMC_ERR_NONE;
}


// Apply new pre_setting params to driver
int mmc_set_pre_setting_params(struct mmc* mmc, u32 rw, u32 ddr,
									u32 ck_sel, u32 ckgen_delay, u32 pad_delay, u32 internal_delay, u32 sample_edge)
{
	if (rw == 0) // Read
	{
		msdc_set_read_pre_setting_params(mmc, ddr, ck_sel, ckgen_delay, pad_delay, sample_edge);
	}
	else if (rw == 1)
	{
		msdc_set_write_pre_setting_params(mmc, ddr, ck_sel, ckgen_delay, pad_delay, internal_delay, sample_edge);
	}
	else
	{
		// Wrong!!!
	}
	return MMC_ERR_NONE;
}

int mmc_get_pre_setting_params(struct mmc* mmc, u32 rw, u32 ddr)
{
	if (rw == 0) // Read
	{
		msdc_get_read_pre_setting_params(mmc, ddr);
	}
	else if (rw == 1)
	{
		msdc_get_write_pre_setting_params(mmc, ddr);
	}
	else
	{
		// Wrong!!!
	}
	return MMC_ERR_NONE;
}


int mmc_hw_reset_whole_module(struct mmc* mmc, int reset_type)
{
	if (reset_type)
		return msdc_hw_reset_whole_module(mmc);
	else
		return msdc_sw_reset_whole_module(mmc);
}

int mmc_set_pad_params(struct mmc* mmc, u32 clk_drv, u32 cmd_drv, u32 dat_drv, u32 resistor, u32 slew_rate)
{
	return msdc_set_pad_params(mmc, clk_drv, cmd_drv, dat_drv, resistor, slew_rate);
}

int mmc_get_pad_params(struct mmc* mmc)
{
	return msdc_get_pad_params(mmc);
}


int mmc_set_work_clock(struct mmc* mmc, u32 ddr, u32 clk)
{
	printf("msdc%d =====> change clock %dKHz to %dKHz <=====\n", mmc->host_id, mmc->work_clock_freq/1000, clk * 1000);
	printf("msdc%d =====> change clock mode %s to %s <=====\n", mmc->host_id, mmc->work_clock_mode ? "DDR" : "SDR", ddr ? "DDR" : "SDR");
	mmc->work_clock_freq = clk * 1000 * 1000;
	mmc->work_clock_mode = ddr;

	return mmc_force_reinit(mmc);
}

// Dump data to external SD Card..
#define MSDC_DUMP_BUF_BLOCKS 		(1024)
#define MSDC_DUMP_BUF_SIZE 			(MSDC_DUMP_BUF_BLOCKS * 512)
int mmc_dump_data(struct mmc* mmc, struct mmc* dst_mmc, u64 dst_offset, u64 start_addr, u64 size)
{
	u32 ret = 0;
	u32 dst_offset_in_block = 0;
	u32 start_addr_in_block = 0;
	u32 size_in_block = 0;
	u32 trans_size_in_block = 0;
	u32 i = 0;

	u8* buf = (u8*)malloc(MSDC_DUMP_BUF_SIZE);
	if (buf == NULL)
	{
		printf("msdc%d =====> allocate buffer for RW failed <=====\n", mmc->host_id);
		return MMC_ERR_FAILED;
	}

	if ((mmc == NULL) || (dst_mmc == NULL))
	{
		free(buf);
		printf("msdc%d =====> bad mmc pointer for dump <=====\n", mmc->host_id);
		return MMC_ERR_FAILED;
	}

	// Convert address and size to block units
	dst_offset_in_block = ALIGN(dst_offset, 512) / 512;
	start_addr_in_block = ALIGN(start_addr, 512) / 512;
	size_in_block = ALIGN(size, 512) / 512;
	printf("msdc%d =====> source(mmc%d)     : start address: 0x%08X %08X (%d) <=====\n", mmc->host_id, mmc->host_id, (u32)(start_addr >> 32),(u32)(start_addr & 0xFFFFFFFF), start_addr_in_block);
	printf("msdc%d =====> destination(mmc%d): dest address : 0x%08X %08X (%d) <=====\n", mmc->host_id, dst_mmc->host_id, (u32)(dst_offset >> 32),(u32)(dst_offset & 0xFFFFFFFF), dst_offset_in_block);
	printf("msdc%d =====> dump data size : 0x%08X %08X (%d) <=====\n", mmc->host_id, (u32)(size >> 32),(u32)(size & 0xFFFFFFFF), size_in_block);

	if (0 == size_in_block) {
		MMC_LOG(MMC_LOG_ERR, "dump block count is 0");
		return MMC_ERR_FAILED;
	}

	while (size_in_block)
	{
		if (size_in_block > MSDC_DUMP_BUF_BLOCKS){
			trans_size_in_block = MSDC_DUMP_BUF_BLOCKS;
		}else{
			trans_size_in_block = size_in_block;
		}

		// Read data from  source address
		mmc_bread(mmc->host_id, start_addr_in_block, trans_size_in_block, buf);

		// Write data to destination device with special address (dst_offset)
		mmc_bwrite(dst_mmc->host_id, dst_offset_in_block, trans_size_in_block, buf);

		size_in_block -= trans_size_in_block;
		start_addr_in_block += trans_size_in_block;
		dst_offset_in_block += trans_size_in_block;

		i++;
		printf(".");
		if ((i%64 == 0) && (i != 0))
		{
			printf("\r\n");
		}
	}

	if (buf)
	{
		free(buf);
		buf = NULL;
	}

	printf("============> dump data completed <============\n");

	return MMC_ERR_NONE;
}


#define MSDC_REG_TYPE_ALL		(0)
#define MSDC_REG_TYPE_CSD		(1)
#define MSDC_REG_TYPE_CID		(2)
#define MSDC_REG_TYPE_OCR		(3)
#define MSDC_REG_TYPE_RCA		(4)
#define MSDC_REG_TYPE_DSR		(5)
#define MSDC_REG_TYPE_ECSD		(6)		// eMMC Only
#define MSDC_REG_TYPE_SCR		(7)		// SD only
#define MSDC_REG_TYPE_SSR		(8)		// SD only

int mmc_dump_register(struct mmc* mmc, u32 type)
{
	switch(type)
	{
	case MSDC_REG_TYPE_ALL:
		break;

	case MSDC_REG_TYPE_CSD:
		break;

	case MSDC_REG_TYPE_CID:
		break;

	case MSDC_REG_TYPE_OCR:
		break;

	case MSDC_REG_TYPE_RCA:
		break;

	case MSDC_REG_TYPE_DSR:
		break;

	case MSDC_REG_TYPE_ECSD:	// eMMC Only
		break;

	case MSDC_REG_TYPE_SCR:		// SD Only
		break;

	case MSDC_REG_TYPE_SSR:		// SD Only
		break;

	default:
		break;

	}

	return MMC_ERR_NONE;
}

#endif //CONFIG_MSDC_ETT
// ========================================================================
// ETT Releated Functions  (End)
// ========================================================================

#define MSDC_ERASE_TYPE_NONE			(0)
#define MSDC_ERASE_TYPE_ERASE			(1)
#define MSDC_ERASE_TYPE_TRIM			(2)
#define MSDC_ERASE_TYPE_DISCARD			(3)
#define MSDC_ERASE_TYPE_SANITIZE		(4)
#define MSDC_ERASE_TYPE_HC_ERASE		(5)
#define MSDC_ERASE_TYPE_REAL_WRITE		(6)
#define MSDC_ERASE_TYPE_ENTIRE_DEVICE	(8)


#define MMC_ERASE_ARG				0x00000000
#define MMC_SECURE_ERASE_ARG		0x80000000
#define MMC_TRIM_ARG				0x00000001
#define MMC_DISCARD_ARG				0x00000003
#define MMC_SECURE_TRIM1_ARG		0x80000001
#define MMC_SECURE_TRIM2_ARG		0x80008000

#define MMC_SECURE_ARGS				0x80000000
#define MMC_TRIM_ARGS				0x00008001

#define MSDC_ERASE_PATTERN_BLOCK	(2048)
#define MSDC_ERASE_PATTERN_SIZE		(2048 * 512)

int mmc_erase(struct mmc* mmc, u32 type, u64 start_addr, u64 size)
{
	struct mmc_cmd cmd;
	int err = 0;
	u8 *pattern = NULL;
	u32 wtimes = 0;
	u32 waddr = 0;

	switch(type)
	{
	// =========================  ERASE  =========================//
	case MSDC_ERASE_TYPE_ERASE:
		cmd.opcode 	= MMC_ERASE_GROUP_START;
		cmd.arg 	= (u32)((start_addr / 512) & 0xFFFFFFFF); // start_addr;
		cmd.rsptype = MMC_RSP_R1;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send MMC_ERASE_GROUP_START failed: err = %d", err);
			return err;
		}
		printf("============> MMC_ERASE_GROUP_START : 0x%08X <============\n", cmd.resp[0]);

		cmd.opcode 	= MMC_ERASE_GROUP_END;
		cmd.arg 	= (u32)(((start_addr + size - 512 * 1024) / 512) & 0xFFFFFFFF); // start_addr + size;
		printf("======>  Erase End Arg: 0x%08X  <=====\n", cmd.arg);
		cmd.rsptype = MMC_RSP_R1;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send MMC_ERASE_GROUP_END failed: err = %d", err);
			return err;
		}
		printf("============>  MMC_ERASE_GROUP_END: 0x%08X  <============\n", cmd.resp[0]);

		printf("============>   MMC_ERASE START     <============\n");
		cmd.opcode 	= MMC_ERASE;
		cmd.arg 	= MMC_ERASE_ARG;
		cmd.rsptype = MMC_RSP_R1b;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send MMC_ERASE failed: err = %d", err);
			return err;
		}
		printf("============>     MMC_ERASE END :  0x%08X  <============\n", cmd.resp[0]);

		printf("Wait for operation is completed.\n");
		do {
			memset(&cmd, 0, sizeof(struct mmc_cmd));

			cmd.opcode  = MMC_CMD_SEND_STATUS;
		    cmd.arg     = mmc->rca << 16;
		    cmd.rsptype = MMC_RSP_R1;
		    cmd.retries = CMD_RETRIES;
		    cmd.timeout = CMD_TIMEOUT;

		    err = mmc_send_cmd(mmc, &cmd, NULL);

			if (err || (cmd.resp[0] & 0xFDF92000)) {
				printf("error %d requesting status %#x\n", err, cmd.resp[0]);
				break;
			}
			printf("*\n");
		} while (!(cmd.resp[0] & R1_READY_FOR_DATA) || R1_CURRENT_STATE(cmd.resp[0]) == 7);

		printf("============> Erase Real Completed  <============\n");

		err = mmc_mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_SANITIZE_START, 1);
		if (err)
		{
			MMC_LOG(MMC_LOG_ERR, "Start eMMC Sanitize Failed");
			return err;
		}
		printf("============>    Sanitize    <============\n");

		printf("Wait for operation is completed.\n");
		do {
			memset(&cmd, 0, sizeof(struct mmc_cmd));

			cmd.opcode  = MMC_CMD_SEND_STATUS;
		    cmd.arg     = mmc->rca << 16;
		    cmd.rsptype = MMC_RSP_R1;
		    cmd.retries = CMD_RETRIES;
		    cmd.timeout = CMD_TIMEOUT;

		    err = mmc_send_cmd(mmc, &cmd, NULL);

			if (err || (cmd.resp[0] & 0xFDF92000)) {
				printf("error %d requesting status %#x\n", err, cmd.resp[0]);
				break;
			}
			//printf("*\n");
		} while (!(cmd.resp[0] & R1_READY_FOR_DATA) || R1_CURRENT_STATE(cmd.resp[0]) == 7);

		printf("============> Sanitize Real Completed  <============\n");

		break;

	// =========================  TRIM  =========================//
	case MSDC_ERASE_TYPE_TRIM:
		cmd.opcode 	= MMC_ERASE_GROUP_START;
		cmd.arg 	= (u32)((start_addr / (2 << mmc->csd.write_blkbits)) & 0xFFFFFFFF); // start_addr;
		cmd.rsptype = MMC_RSP_R1;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send MMC_ERASE_GROUP_START failed: err = %d", err);
			return err;
		}
		printf("============> MMC_ERASE_GROUP_START <============\n");

		cmd.opcode 	= MMC_ERASE_GROUP_END;
		cmd.arg 	= (u32)(((start_addr + size) / (2 << mmc->csd.write_blkbits)) & 0xFFFFFFFF); // start_addr + size;
		cmd.rsptype = MMC_RSP_R1;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send MMC_ERASE_GROUP_END failed: err = %d", err);
			return err;
		}
		printf("============>  MMC_ERASE_GROUP_END  <============\n");

		printf("============>   MMC_ERASE (TRIM) START     <============\n");
		cmd.opcode 	= MMC_ERASE;
		cmd.arg 	= MMC_TRIM_ARG;
		cmd.rsptype = MMC_RSP_R1b;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send MMC_ERASE (TRIM) failed: err = %d", err);
			return err;
		}
		printf("============>     MMC_ERASE (TRIM) END     <============\n");


		printf("Wait for operation is completed.\n");
		do {
			memset(&cmd, 0, sizeof(struct mmc_cmd));

			cmd.opcode	= MMC_CMD_SEND_STATUS;
			cmd.arg 	= mmc->rca << 16;
			cmd.rsptype = MMC_RSP_R1;
			cmd.retries = CMD_RETRIES;
			cmd.timeout = CMD_TIMEOUT;

			err = mmc_send_cmd(mmc, &cmd, NULL);

			if (err || (cmd.resp[0] & 0xFDF92000)) {
				printf("error %d requesting status %#x\n", err, cmd.resp[0]);
				break;
			}
			printf("*\n");
		} while (!(cmd.resp[0] & R1_READY_FOR_DATA) || R1_CURRENT_STATE(cmd.resp[0]) == 7);

		printf("============> TRIM Real Completed	<============\n");

		break;

	// =========================  Discard  =========================//
	case MSDC_ERASE_TYPE_DISCARD:
		break;

	// =========================  Sanitize  =========================//
	case MSDC_ERASE_TYPE_SANITIZE:
		err = mmc_mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_SANITIZE_START, 1);
		if (err)
		{
			MMC_LOG(MMC_LOG_ERR, "Start eMMC Sanitize Failed");
			return err;
		}
		printf("============>    Sanitize    <============\n");
		break;

	// ========================= HC Erase  =========================//
	case MSDC_ERASE_TYPE_HC_ERASE:
		err = _mmc_set_ext_csd(mmc, EXT_CSD_ERASE_GRP_DEF, 1, 0x01);
		//err = mmc_mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_ERASE_GRP_DEF, 1);
		if (err)
		{
			MMC_LOG(MMC_LOG_ERR, "Switch to HC Erase Failed");
			return err;
		}

		printf("============> After Switch Command <============\n");

		err = mmc_send_ext_csd(mmc, &mmc->raw_ext_csd[0]);
		if (err)
		{
			MMC_LOG(MMC_LOG_ERR, "get ext_csd from mmc failed");
			return 2;
		}

		if (mmc->raw_ext_csd[EXT_CSD_ERASE_GRP_DEF] & 0x01)
		{
			MMC_LOG(MMC_LOG_ERR, "Switch to HC Erase Successfully");
		}

		cmd.opcode 	= MMC_ERASE_GROUP_START;
		cmd.arg 	= (u32)((start_addr/u4HC_Erase_Group_Size) & 0xFFFFFFFF); // start_addr;
		cmd.rsptype = MMC_RSP_R1;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send MMC_ERASE_GROUP_START failed: err = %d", err);
			return err;
		}
		printf("============> MMC_ERASE_GROUP_START <============\n");

		cmd.opcode 	= MMC_ERASE_GROUP_END;
		cmd.arg 	= (u32)(((start_addr + size) / u4HC_Erase_Group_Size) & 0xFFFFFFFF); // start_addr + size;
		cmd.rsptype = MMC_RSP_R1;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send MMC_ERASE_GROUP_END failed: err = %d", err);
			return err;
		}
		printf("============>  MMC_ERASE_GROUP_END  <============\n");

		printf("============>   MMC_ERASE (HC Erase) START     <============\n");
		cmd.opcode 	= MMC_ERASE;
		cmd.arg 	= MMC_ERASE_ARG;
		cmd.rsptype = MMC_RSP_R1b;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send MMC_ERASE failed: err = %d", err);
			return err;
		}
		printf("============>     MMC_ERASE (HC Erase) END     <============\n");

		break;

	// =========================  Real Write Pattern  =========================//
	case MSDC_ERASE_TYPE_REAL_WRITE:
		pattern = (u8*)malloc(MSDC_ERASE_PATTERN_SIZE);
		if (pattern == NULL)
		{
			printf("============> Allocate Pattern Pattern Buffer Failed <============\n");
			return 1;
		}

		//memset(pattern, (u8)(start_addr & 0xFF), MSDC_ERASE_PATTERN_SIZE);	// 'start_addr' is used as pattern
		memset(pattern, 1, MSDC_ERASE_PATTERN_SIZE);
		wtimes = (u32)((size / MSDC_ERASE_PATTERN_SIZE) & 0xFFFFFFFF);
		waddr = (u32)((start_addr / 512) & 0xFFFFFFFF);
		while (wtimes--)
		{
			mmc_bwrite(mmc->host_id, waddr, MSDC_ERASE_PATTERN_BLOCK, &pattern);
			waddr += MSDC_ERASE_PATTERN_BLOCK;
			printf("*\n");
		}
		free(pattern);
		printf("============>    Write Pattern Completed    <============\n");
		break;

	// =========================  ERASE Entire Device  =========================//
	case MSDC_ERASE_TYPE_ENTIRE_DEVICE:
		cmd.opcode 	= MMC_ERASE_GROUP_START;
		cmd.arg 	= 0; // start_addr;
		cmd.rsptype = MMC_RSP_R1;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send MMC_ERASE_GROUP_START failed: err = %d", err);
			return err;
		}
		printf("============> MMC_ERASE_GROUP_START : 0x%08X <============\n", cmd.resp[0]);

		cmd.opcode 	= MMC_ERASE_GROUP_END;
		cmd.arg 	= (u32)(((mmc->capacity - 512 * 1024) / 512) & 0xFFFFFFFF); // use block address, reduce 512K;
		printf("======>  Erase End Arg: 0x%08X  <=====\n", cmd.arg);
		cmd.rsptype = MMC_RSP_R1;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send MMC_ERASE_GROUP_END failed: err = %d", err);
			return err;
		}
		printf("============>  MMC_ERASE_GROUP_END: 0x%08X  <============\n", cmd.resp[0]);

		printf("============>   MMC_ERASE START     <============\n");
		cmd.opcode 	= MMC_ERASE;
		cmd.arg 	= MMC_ERASE_ARG;
		cmd.rsptype = MMC_RSP_R1b;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send MMC_ERASE failed: err = %d", err);
			return err;
		}
		printf("============>     MMC_ERASE END :  0x%08X  <============\n", cmd.resp[0]);

		printf("Wait for Erase operation is completed.\n");
		do {
			memset(&cmd, 0, sizeof(struct mmc_cmd));

			cmd.opcode  = MMC_CMD_SEND_STATUS;
		    cmd.arg     = mmc->rca << 16;
		    cmd.rsptype = MMC_RSP_R1;
		    cmd.retries = CMD_RETRIES;
		    cmd.timeout = CMD_TIMEOUT;

		    err = mmc_send_cmd(mmc, &cmd, NULL);

			if (err || (cmd.resp[0] & 0xFDF92000)) {
				printf("error %d requesting status %#x\n", err, cmd.resp[0]);
				break;
			}
			printf("*\n");
		} while (!(cmd.resp[0] & R1_READY_FOR_DATA) || R1_CURRENT_STATE(cmd.resp[0]) == 7);

		printf("============> Erase Real Completed  <============\n");

		if (size == 1)  // Do sanitize
		{
			err = mmc_mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_SANITIZE_START, 1);
			if (err)
			{
				MMC_LOG(MMC_LOG_ERR, "Start eMMC Sanitize Failed");
				return err;
			}
			printf("============>    Sanitize    <============\n");

			printf("Wait for Sanitize operation is completed.\n");
			do {
				memset(&cmd, 0, sizeof(struct mmc_cmd));

				cmd.opcode  = MMC_CMD_SEND_STATUS;
			    cmd.arg     = mmc->rca << 16;
			    cmd.rsptype = MMC_RSP_R1;
			    cmd.retries = CMD_RETRIES;
			    cmd.timeout = CMD_TIMEOUT;

			    err = mmc_send_cmd(mmc, &cmd, NULL);

				if (err || (cmd.resp[0] & 0xFDF92000)) {
					printf("error %d requesting status %#x\n", err, cmd.resp[0]);
					break;
				}
				//printf("*\n");
			} while (!(cmd.resp[0] & R1_READY_FOR_DATA) || R1_CURRENT_STATE(cmd.resp[0]) == 7);

			printf("============> Sanitize Real Completed  <============\n");
		}

		break;
	default:
		break;
	}


	return MMC_ERR_NONE;
}


// success -> return 0
int mmc_go_idle(struct mmc* mmc)
{
	struct mmc_cmd cmd;
	int err;

	udelay(1000);

	cmd.opcode 	= MMC_CMD_GO_IDLE_STATE;
	cmd.arg 	= 0;
	cmd.rsptype = MMC_RSP_NONE;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err){
		MMC_LOG(MMC_LOG_ERR, "send cmd0 failed: err = %d", err);
		return err;
	}
	udelay(2000);

	return 0;
}

//
// Send ACMD41 for SD card
//
int mmc_sd_send_op_cond(struct mmc *mmc)
{
	int timeout = 1000;
	int err;
	struct mmc_cmd cmd;

	do {
		cmd.opcode 	= MMC_CMD_APP_CMD;
		cmd.rsptype = MMC_RSP_R1;
		cmd.arg 	= 0;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
            MMC_LOG(MMC_LOG_ERR, "send cmd55 failed, return err = %d", err);
			return err;
        }

		cmd.opcode 	= SD_CMD_APP_SEND_OP_COND;
		cmd.rsptype = MMC_RSP_R3;
//		cmd.arg 		= mmc->voltages;
		cmd.arg 	= ((0x1FF << 15) | 0x40000000);
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		if (mmc->version == SD_VERSION_2)
			cmd.arg |= OCR_HCS;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
            MMC_LOG(MMC_LOG_ERR, "send cmd41 failed, return err = %d", err);
			return err;
        }

		udelay(5000);
	} while ((!(cmd.resp[0] & OCR_BUSY)) && timeout--);

	if (timeout <= 0){
        MMC_LOG(MMC_LOG_ERR, "ACMD41 time out");
		return UNUSABLE_ERR;
    }

	if (mmc->version != SD_VERSION_2)
		mmc->version = SD_VERSION_1_0;

	mmc->ocr = cmd.resp[0];

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	mmc->rca = 0;

	return 0;
}

int mmc_send_op_cond(struct mmc *mmc)
{
	int timeout = 1000;
	struct mmc_cmd cmd;
	int err;

	/* Some cards seem to need this */
	//mmc_go_idle(mmc);  fix sumsung eMMC ocr_busy error

	do {
		cmd.opcode 	= MMC_CMD_SEND_OP_COND;
		cmd.rsptype = MMC_RSP_R3;
		//cmd.darg = OCR_HCS | mmc->voltages;
		cmd.arg 	= MMC_OCR_2V7_3V6 | MMC_OCR_1V7_1V95 | MMC_OCR_SECTOR_MODE | OCR_HCS;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err){
			MMC_LOG(MMC_LOG_ERR, "send cmd1 failed: err = %d", err);
			return err;
		}

		udelay(1000);
	} while (!(cmd.resp[0] & OCR_BUSY) && timeout--);

	if (timeout <= 0){
		MMC_LOG(MMC_LOG_ERR, "mmc_send_op_cond timeout");
		return UNUSABLE_ERR;
	}

	mmc->version = MMC_VERSION_UNKNOWN;
	mmc->ocr = cmd.resp[0];

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	mmc->rca = 0;

	return 0;
}


int mmc_send_ext_csd(struct mmc *mmc, u8 *ext_csd)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;

	/* Get the Card Status Register */
	cmd.opcode 	= MMC_CMD_SEND_EXT_CSD;
	cmd.rsptype = MMC_RSP_R1;
	cmd.arg 	= 0;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

	data.dest = ext_csd;
	data.blocks = 1;
	data.blocksize = 512;
	data.flags = MMC_DATA_READ;

	flush_invalid_cache(ext_csd, 512);
	err = mmc_send_cmd(mmc, &cmd, &data);

	return err;
}


int mmc_mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value)
{
	struct mmc_cmd cmd;

	cmd.opcode 	= MMC_CMD_SWITCH;
	cmd.rsptype = MMC_RSP_R1b;
	cmd.arg 	= (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
					(index << 16) |
					(value << 8);
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

	return mmc_send_cmd(mmc, &cmd, NULL);
}

int mmc_change_freq(struct mmc *mmc)
{
	u8 ext_csd[512];
	char cardtype;
	int err;

	mmc->card_caps = 0;

	/* Only version 4 supports high-speed */
	//if (mmc->version < MMC_VERSION_4)
	//{
	//	printf("[mmc][%s]mmc version err. ver = %d\r\n", __FUNCTION__, mmc->version);
	//	return 0;
	//}

	mmc->card_caps |= MMC_MODE_4BIT;

	err = mmc_send_ext_csd(mmc, &ext_csd[0]);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "get ext_csd from mmc failed");
		return err;
	}

	if (ext_csd[212] || ext_csd[213] || ext_csd[214] || ext_csd[215])
	{
		MMC_LOG(MMC_LOG_ERR, "card is high_capacity");
		mmc->high_capacity = 1;
	}

	cardtype = ext_csd[196] & 0xf;

	err = mmc_mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "switch high speed for mmc failed");
		return err;
	}

	/* Now check to see that it worked */
	err = mmc_send_ext_csd(mmc, &ext_csd[0]);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "after switch high speed, get ext_csd from mmc failed");
		return err;
	}

	/* No high-speed support */
	if (!ext_csd[185])
	{
		MMC_LOG(MMC_LOG_ERR, "high speed set failed.");
		return 0;
	}

	/* High Speed is set, there are two types: 52MHz and 26MHz */
	if (cardtype & MMC_HS_52MHZ)
		mmc->card_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	else
		mmc->card_caps |= MMC_MODE_HS;

	return 0;
}

// For SD Card  Switch Function
int mmc_sd_switch(struct mmc *mmc, int mode, int group, u8 value, u8 *resp)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;

	err = mmc_set_blocklen(mmc, 64);
    if (err)
	{
        MMC_LOG(MMC_LOG_ERR, "set blocklen err = %d\r\n", err);
	}

	/* Switch the frequency */
	cmd.opcode 	= SD_CMD_SWITCH_FUNC;
	cmd.rsptype = MMC_RSP_R1;
	cmd.arg 	= (mode << 31) | 0x00ffffff;
	cmd.arg    &= ~(0xf << (group * 4));
	cmd.arg    |= value << (group * 4);
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

	data.dest = resp;
	data.blocksize = 64;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	return mmc_send_cmd(mmc, &cmd, &data);
}

// For SD Card Change Freq, TODO
int mmc_sd_change_freq(struct mmc *mmc)
{
	int err = 0;
	struct mmc_cmd cmd;
	u32 scr[2];
	u32 switch_status[16];
	struct mmc_data data;
	int timeout;

#if 0
	err = mmc_set_blocklen(mmc, 8);
    if (err)
	{
        MMC_LOG(MMC_LOG_ERR, "set blocklen err = %d", err);
	}
#endif

#if 1
	mmc->card_caps = 0;

	/* Read the SCR to find out if this card supports higher speeds */
	cmd.opcode	= MMC_CMD_APP_CMD;
	cmd.rsptype = MMC_RSP_R1;
	cmd.arg 	= mmc->rca << 16;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err){
		MMC_LOG(MMC_LOG_ERR, "send MMC_CMD_APP_CMD failed: err = %d", err);
		return err;
	}

	timeout = 3;

retry_scr:

	cmd.opcode 	= SD_CMD_APP_SEND_SCR;
	cmd.rsptype = MMC_RSP_R1;
	cmd.arg 	= 0;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

	data.dest = (u8 *)&scr;
	data.blocksize = 8;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);
	if (err) {
		if (timeout--)
			goto retry_scr;
		MMC_LOG(MMC_LOG_ERR, "Send SD_CMD_APP_SEND_SCR failed: err = %d", err);
		return err;
	}

	// Send Stop Command
	//mmc_send_stopcmd(mmc);

	MMC_LOG(MMC_LOG_DBG, "SCR: 0x%08X 0x%08X", scr[0], scr[1]);
	mmc->raw_scr[0] = __be32_to_cpu(scr[0]);
	mmc->raw_scr[1] = __be32_to_cpu(scr[1]);

	MMC_LOG(MMC_LOG_DBG, "SCR Register: 0x%08X 0x%08X", mmc->raw_scr[0], mmc->raw_scr[1]);

	switch ((mmc->raw_scr[0] >> 24) & 0xf) {
		case 0:
			mmc->version = SD_VERSION_1_0;
			break;
		case 1:
			mmc->version = SD_VERSION_1_10;
			break;
		case 2:
			mmc->version = SD_VERSION_2;
			break;
		default:
			mmc->version = SD_VERSION_1_0;
			break;
	}

	if (mmc->raw_scr[0] & SD_DATA_4BIT)
	{
		MMC_LOG(MMC_LOG_DBG, "SD Card Support 4Bit Bus-Width Mode From SCR Register");
		mmc->card_caps |= MMC_MODE_4BIT;
	}

	/* Version 1.0 doesn't support switching */
	if (mmc->version == SD_VERSION_1_0)
		return 0;
#endif

#if 0
	timeout = 4;
	while (timeout--) {
		err = mmc_sd_switch(mmc, SD_SWITCH_CHECK, 0, 1, (u8 *)&switch_status);
		if (err)
			return err;

		/* The high-speed function is busy.  Try again */
		if (!(__be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY))
			break;
	}

	/* If high-speed isn't supported, we return */
	if (!(__be32_to_cpu(switch_status[3]) & SD_HIGHSPEED_SUPPORTED))
		return 0;
#endif

#if 0
	err = mmc_sd_switch(mmc, SD_SWITCH_SWITCH, 0, 1, (u8 *)&switch_status);
	if (err){
		MMC_LOG(MMC_LOG_ERR, "send cmd6 failed: err = %d", err);
		return err;
	}

	MMC_LOG(MMC_LOG_DBG, "Send CMD6 to switch card to High-Speed mode successfully");

	if ((__be32_to_cpu(switch_status[4]) & 0x0f000000) == 0x01000000)
	{
		mmc->card_caps |= MMC_MODE_HS;
		MMC_LOG(MMC_LOG_DBG, "SD card supports High-Speed mode");
	}
#endif

	return 0;
}


void mmc_set_ios(struct mmc *mmc)
{
	mmc->set_ios(mmc);
}

void mmc_set_clock(struct mmc *mmc, uint clock)
{
	if (clock > mmc->f_max)
		clock = mmc->f_max;

	if (clock < mmc->f_min)
		clock = mmc->f_min;

	mmc->clock = clock;
	mmc_set_ios(mmc);
}

void mmc_set_bus_width(struct mmc *mmc, uint width)
{
	mmc->bus_width = width;
	mmc_set_ios(mmc);
}

int mmc_send_if_cond(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;

	cmd.opcode 	= SD_CMD_SEND_IF_COND;
	cmd.arg 	= (1 << 8) | 0xaa;	/* We set the bit if the host supports voltages between 2.7 and 3.6 V */
	cmd.rsptype = MMC_RSP_R7;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err){
		MMC_LOG(MMC_LOG_ERR, "send CMD8 failed: err = %d", err);
		return err;
    }

	// check pattern
	if ((cmd.resp[0] & 0xff) != 0xaa){
		//MMC_LOG(MMC_LOG_ERR, "response error: resp = 0x%08X", cmd.resp[0]);
		return UNUSABLE_ERR;
	}
	else{
		mmc->version = SD_VERSION_2;
        //printf("SD VERSION 2\r\n");
    }
	return 0;
}

extern int msdc_mmc_inited;
extern int msdc_mmc_reinit_id;
int mmc_register(struct mmc *mmc)
{
	/* Setup the universal parts of the block interface just once */
	mmc->block_dev.if_type 		= IF_TYPE_MMC;
	if(msdc_mmc_inited)
		mmc->block_dev.dev = msdc_mmc_reinit_id;
	else
		mmc->block_dev.dev = cur_dev_num++;		// Make first host id is 0
	mmc->block_dev.removable 	= 1;
	mmc->block_dev.part_type 	= PART_TYPE_DOS;

	mmc->block_dev.block_read 	= mmc_bread;
	mmc->block_dev.block_write 	= mmc_bwrite;

	INIT_LIST_HEAD (&mmc->link);

	list_add_tail (&mmc->link, &mmc_devices);

	return 0;
}

int mmc_unregister(struct mmc *mmc)
{
	list_del(&mmc->link);
	return 0;
}

block_dev_desc_t *mmc_get_dev(int dev)
{
	struct mmc *mmc = find_mmc_device(dev);

	return mmc ? &mmc->block_dev : NULL;
}

int mmc_update_rw_param(struct mmc *mmc, unsigned int man_id, unsigned int oem_id)
{
	/* Toshiba eMMC on EVB Board */
	if ((man_id == 17) && (oem_id == 0))
	{
		mmc->host_hw->read_dat_latch_ck_sel = atc_msdc0_param[1]->read_dat_latch_ck_sel;
		mmc->host_hw->read_ckgen_delay_sel = atc_msdc0_param[1]->read_ckgen_delay_sel;
		mmc->host_hw->read_sample_edge = atc_msdc0_param[1]->read_sample_edge;
		mmc->host_hw->read_pad_delay = atc_msdc0_param[1]->read_pad_delay;

		mmc->host_hw->write_dat_latch_ck_sel = atc_msdc0_param[1]->write_dat_latch_ck_sel;
		mmc->host_hw->write_ckgen_delay_sel = atc_msdc0_param[1]->write_ckgen_delay_sel;
		mmc->host_hw->write_sample_edge = atc_msdc0_param[1]->write_sample_edge;
		mmc->host_hw->write_pad_delay = atc_msdc0_param[1]->write_pad_delay;
		mmc->host_hw->write_internal_delay = atc_msdc0_param[1]->write_internal_delay;
	}

	return 0;
}

static void mmc_decode_cid(struct mmc *mmc)
{
    u32 *resp = mmc->raw_cid;

    memset(&mmc->cid, 0, sizeof(struct mmc_cid));

    if (mmc_card_sd(mmc))
	{
    	/*
    	 * SD doesn't currently have a version field so we will
    	 * have to assume we can parse this.
    	 */
    	mmc->cid.manfid			= UNSTUFF_BITS(resp, 120, 8);
    	mmc->cid.oemid			= UNSTUFF_BITS(resp, 104, 16);
    	mmc->cid.prod_name[0]	= UNSTUFF_BITS(resp, 96, 8);
    	mmc->cid.prod_name[1]	= UNSTUFF_BITS(resp, 88, 8);
    	mmc->cid.prod_name[2]	= UNSTUFF_BITS(resp, 80, 8);
    	mmc->cid.prod_name[3]	= UNSTUFF_BITS(resp, 72, 8);
    	mmc->cid.prod_name[4]	= UNSTUFF_BITS(resp, 64, 8);
    	mmc->cid.hwrev			= UNSTUFF_BITS(resp, 60, 4);
    	mmc->cid.fwrev			= UNSTUFF_BITS(resp, 56, 4);
    	mmc->cid.serial			= UNSTUFF_BITS(resp, 24, 32);
    	mmc->cid.year			= UNSTUFF_BITS(resp, 12, 8);
    	mmc->cid.month			= UNSTUFF_BITS(resp, 8, 4);

    	mmc->cid.year += 2000; /* SD cards year offset */
    }
	else
    {

		/*
         * The selection of the format here is based upon published
         * specs from sandisk and from what people have reported.
         */
        switch (mmc->csd.mmca_vsn)
		{
		    case 0: /* MMC v1.0 - v1.2 */
		    case 1: /* MMC v1.4 */
		        mmc->cid.manfid        = UNSTUFF_BITS(resp, 104, 24);
		        mmc->cid.prod_name[0]  = UNSTUFF_BITS(resp, 96, 8);
		        mmc->cid.prod_name[1]  = UNSTUFF_BITS(resp, 88, 8);
		        mmc->cid.prod_name[2]  = UNSTUFF_BITS(resp, 80, 8);
		        mmc->cid.prod_name[3]  = UNSTUFF_BITS(resp, 72, 8);
		        mmc->cid.prod_name[4]  = UNSTUFF_BITS(resp, 64, 8);
		        mmc->cid.prod_name[5]  = UNSTUFF_BITS(resp, 56, 8);
		        mmc->cid.prod_name[6]  = UNSTUFF_BITS(resp, 48, 8);
		        mmc->cid.hwrev         = UNSTUFF_BITS(resp, 44, 4);
		        mmc->cid.fwrev         = UNSTUFF_BITS(resp, 40, 4);
		        mmc->cid.serial        = UNSTUFF_BITS(resp, 16, 24);
		        mmc->cid.month         = UNSTUFF_BITS(resp, 12, 4);
		        mmc->cid.year          = UNSTUFF_BITS(resp, 8, 4) + 1997;
		        break;

		    case 2: /* MMC v2.0 - v2.2 */
		    case 3: /* MMC v3.1 - v3.3 */
		    case 4: /* MMC v4 */
		        mmc->cid.manfid        = UNSTUFF_BITS(resp, 120, 8);
		        mmc->cid.cbx           = UNSTUFF_BITS(resp, 112, 2);
		        mmc->cid.oemid         = UNSTUFF_BITS(resp, 104, 8);
		        mmc->cid.prod_name[0]  = UNSTUFF_BITS(resp, 96, 8);
		        mmc->cid.prod_name[1]  = UNSTUFF_BITS(resp, 88, 8);
		        mmc->cid.prod_name[2]  = UNSTUFF_BITS(resp, 80, 8);
		        mmc->cid.prod_name[3]  = UNSTUFF_BITS(resp, 72, 8);
		        mmc->cid.prod_name[4]  = UNSTUFF_BITS(resp, 64, 8);
		        mmc->cid.prod_name[5]  = UNSTUFF_BITS(resp, 56, 8);
		        mmc->cid.hwrev         = UNSTUFF_BITS(resp, 48, 8);
		        mmc->cid.serial        = UNSTUFF_BITS(resp, 16, 32);
		        mmc->cid.month         = UNSTUFF_BITS(resp, 12, 4);
		        mmc->cid.year          = UNSTUFF_BITS(resp, 8, 4) + 1997;
		        break;

		    default:
		        MMC_LOG(MMC_LOG_ERR, "Unknown MMCA version %d", mmc->csd.mmca_vsn);
		        break;
		}

		// For Special eMMC, use Special parameters
		mmc_update_rw_param(mmc, mmc->cid.manfid, mmc->cid.oemid);
    }

	MMC_LOG(MMC_LOG_ERR, "MANID=%d, OEMID=%d, Product_Name = %s", mmc->cid.manfid, mmc->cid.oemid, mmc->cid.prod_name);
}


static int mmc_decode_csd(struct mmc *mmc)
{
    struct mmc_csd *csd = &mmc->csd;
    unsigned int e, m, csd_struct;
    u32 *resp = mmc->raw_csd;

    if (mmc_card_sd(mmc))
	{
        csd_struct = UNSTUFF_BITS(resp, 126, 2);
        csd->csd_struct = csd_struct;

        switch (csd_struct) {
        case 0:
            m = UNSTUFF_BITS(resp, 115, 4);
            e = UNSTUFF_BITS(resp, 112, 3);
            csd->tacc_ns	 = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
            csd->tacc_clks	 = UNSTUFF_BITS(resp, 104, 8) * 100;

            m = UNSTUFF_BITS(resp, 99, 4);
            e = UNSTUFF_BITS(resp, 96, 3);
            csd->max_dtr	  = tran_exp[e] * tran_mant[m];
            csd->cmdclass	  = UNSTUFF_BITS(resp, 84, 12);

            e = UNSTUFF_BITS(resp, 47, 3);
            m = UNSTUFF_BITS(resp, 62, 12);
            csd->capacity	  = (1 + m) << (e + 2);

            csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);

			csd->capacity	  = csd->capacity << (csd->read_blkbits - 9);
			MMC_LOG(MMC_LOG_ERR, "SDSC capacity: 0x%08X %08X",(u32)(csd->capacity >> 32), (u32)(csd->capacity & 0xFFFFFFFF));

            csd->read_partial = UNSTUFF_BITS(resp, 79, 1);
            csd->write_misalign = UNSTUFF_BITS(resp, 78, 1);
            csd->read_misalign = UNSTUFF_BITS(resp, 77, 1);
            csd->r2w_factor = UNSTUFF_BITS(resp, 26, 3);
            csd->write_blkbits = UNSTUFF_BITS(resp, 22, 4);
            csd->write_partial = UNSTUFF_BITS(resp, 21, 1);

            csd->erase_blk_en = UNSTUFF_BITS(resp, 46, 1);
            csd->erase_sctsz = UNSTUFF_BITS(resp, 39, 7) + 1;
            csd->write_prot_grpsz = UNSTUFF_BITS(resp, 32, 7);
            csd->write_prot_grp = UNSTUFF_BITS(resp, 31, 1);
            csd->perm_wr_prot = UNSTUFF_BITS(resp, 13, 1);
            csd->tmp_wr_prot = UNSTUFF_BITS(resp, 12, 1);
            csd->copy = UNSTUFF_BITS(resp, 14, 1);
            csd->dsr = UNSTUFF_BITS(resp, 76, 1);
            break;
        case 1:
            /*
             * This is a block-addressed SDHC card. Most
             * interesting fields are unused and have fixed
             * values. To avoid getting tripped by buggy cards,
             * we assume those fixed values ourselves.
             */
            //mmc_card_set_blockaddr(mmc);

            csd->tacc_ns	 = 0; /* Unused */
            csd->tacc_clks	 = 0; /* Unused */

            m = UNSTUFF_BITS(resp, 99, 4);
            e = UNSTUFF_BITS(resp, 96, 3);
            csd->max_dtr	  = tran_exp[e] * tran_mant[m];
            csd->cmdclass	  = UNSTUFF_BITS(resp, 84, 12);

            m = UNSTUFF_BITS(resp, 48, 22);

			csd->capacity     = UInt32x32x32To64((1 + m), 1024, 512); // Bytes
			MMC_LOG(MMC_LOG_ERR, "SDHC capacity: 0x%08X %08X", (u32)(csd->capacity >> 32),(u32)(csd->capacity & 0xFFFFFFFF));

            csd->read_blkbits = 9;
            csd->read_partial = 0;
            csd->write_misalign = 0;
            csd->read_misalign = 0;
            csd->r2w_factor = 4; /* Unused */
            csd->write_blkbits = 9;
            csd->write_partial = 0;

            csd->erase_blk_en = UNSTUFF_BITS(resp, 46, 1);
            csd->erase_sctsz = UNSTUFF_BITS(resp, 39, 7) + 1;
            csd->write_prot_grpsz = UNSTUFF_BITS(resp, 32, 7);
            csd->write_prot_grp = UNSTUFF_BITS(resp, 31, 1);
            csd->perm_wr_prot = UNSTUFF_BITS(resp, 13, 1);
            csd->tmp_wr_prot = UNSTUFF_BITS(resp, 12, 1);
            csd->copy = UNSTUFF_BITS(resp, 14, 1);
            csd->dsr = UNSTUFF_BITS(resp, 76, 1);
            break;
        default:
            MMC_LOG(MMC_LOG_ERR, "Unknown CSD ver %d", csd_struct);
            return MMC_ERR_INVALID;
        }
    }
	else // ============  eMMC  ============
	{
        /*
         	  * We only understand CSD structure v1.1 and v1.2.
         	  * v1.2 has extra information in bits 15, 11 and 10.
         	*/
        csd_struct = UNSTUFF_BITS(resp, 126, 2);

        if (csd_struct != CSD_STRUCT_VER_1_0 && csd_struct != CSD_STRUCT_VER_1_1
            && csd_struct != CSD_STRUCT_VER_1_2 && csd_struct != CSD_STRUCT_EXT_CSD) {
            MMC_LOG(MMC_LOG_ERR, "Unknown CSD ver %d", csd_struct);
            return MMC_ERR_INVALID;
        }

        csd->csd_struct = csd_struct;
        csd->mmca_vsn    = UNSTUFF_BITS(resp, 122, 4);
        m = UNSTUFF_BITS(resp, 115, 4);
        e = UNSTUFF_BITS(resp, 112, 3);
        csd->tacc_ns     = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
        csd->tacc_clks   = UNSTUFF_BITS(resp, 104, 8) * 100;

        m = UNSTUFF_BITS(resp, 99, 4);
        e = UNSTUFF_BITS(resp, 96, 3);
        csd->max_dtr      = tran_exp[e] * mmc_tran_mant[m];
        csd->cmdclass     = UNSTUFF_BITS(resp, 84, 12);

        e = UNSTUFF_BITS(resp, 47, 3);
        m = UNSTUFF_BITS(resp, 62, 12);
        csd->capacity     = (1 + m) << (e + 2);  // 512Bytes unit
        //csd->capacity	  = csd->capacity * 512;
		//MMC_LOG(MMC_LOG_ERR, "EMMC capacity: 0x%08X, 0x%08X", (u32)(csd->capacity & 0xFFFFFFFF), (u32)(csd->capacity >> 32));

        csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);
        csd->read_partial = UNSTUFF_BITS(resp, 79, 1);
        csd->write_misalign = UNSTUFF_BITS(resp, 78, 1);
        csd->read_misalign = UNSTUFF_BITS(resp, 77, 1);
        csd->r2w_factor = UNSTUFF_BITS(resp, 26, 3);
        csd->write_blkbits = UNSTUFF_BITS(resp, 22, 4);
        csd->write_partial = UNSTUFF_BITS(resp, 21, 1);

		u4Erase_Group_Size = UNSTUFF_BITS(resp, 42, 5);
		u4Erase_Group_Mult = UNSTUFF_BITS(resp, 37, 5);

        csd->erase_sctsz = (UNSTUFF_BITS(resp, 42, 5) + 1) * (UNSTUFF_BITS(resp, 37, 5) + 1);
        csd->write_prot_grpsz = UNSTUFF_BITS(resp, 32, 5);
        csd->write_prot_grp = UNSTUFF_BITS(resp, 31, 1);
        csd->perm_wr_prot = UNSTUFF_BITS(resp, 13, 1);
        csd->tmp_wr_prot = UNSTUFF_BITS(resp, 12, 1);
        csd->copy = UNSTUFF_BITS(resp, 14, 1);
        csd->dsr = UNSTUFF_BITS(resp, 76, 1);
    }

	mmc->read_bl_len 	= 512;
	mmc->write_bl_len 	= 512;
	mmc->tran_speed 	= csd->max_dtr;
	mmc->capacity 		= csd->capacity;
	//MMC_LOG(MMC_LOG_ERR, "capacity finally: 0x%08X, 0x%08X", (u32)(csd->capacity & 0xFFFFFFFF), (u32)(csd->capacity >> 32));

#if ATC_MSDC_DUMP
    mmc_dump_csd(mmc);
#endif

    return 0;
}


static int mmc_decode_ext_csd(struct mmc *mmc)
{
	char *rev[] = { "4.0", "4.1", "4.2", "4.3", "Obsolete", "4.41", "4.5/4.51", "5.0", "5.1" };

    u8 *ext_csd = &mmc->raw_ext_csd[0];

	if (mmc->raw_ext_csd[EXT_CSD_SEC_CNT] || mmc->raw_ext_csd[EXT_CSD_SEC_CNT+1] || mmc->raw_ext_csd[EXT_CSD_SEC_CNT+2] || mmc->raw_ext_csd[EXT_CSD_SEC_CNT+3])
	{
		mmc->high_capacity = 1;
	}

    mmc->ext_csd.sectors =
       	ext_csd[EXT_CSD_SEC_CNT + 0] << 0 |
    	ext_csd[EXT_CSD_SEC_CNT + 1] << 8 |
    	ext_csd[EXT_CSD_SEC_CNT + 2] << 16 |
    	ext_csd[EXT_CSD_SEC_CNT + 3] << 24;
	mmc->capacity = (u64)mmc->ext_csd.sectors * 512;

	#ifdef CONFIG_MSDC_ETT
	MMC_LOG(MMC_LOG_ERR, "eMMC v%s", rev[ext_csd[EXT_CSD_REV]]);
	MMC_LOG(MMC_LOG_ERR, "eMMC Capacity: %luMB", (mmc->capacity / 1024) / 1024);
	MMC_LOG(MMC_LOG_ERR, "eMMC FW Ver: 0x%02X", mmc->cid.hwrev);
	#endif

    mmc->ext_csd.rev = ext_csd[EXT_CSD_REV];
    mmc->ext_csd.hc_erase_grp_sz = ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512 * 1024;
    mmc->ext_csd.hc_wp_grp_sz = ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512 * 1024;
    mmc->ext_csd.trim_tmo_ms = ext_csd[EXT_CSD_TRIM_MULT] * 300;
    mmc->ext_csd.boot_info   = ext_csd[EXT_CSD_BOOT_INFO];
    mmc->ext_csd.boot_part_sz = ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024;
    mmc->ext_csd.access_sz = (ext_csd[EXT_CSD_ACC_SIZE] & 0xf) * 512;
    mmc->ext_csd.rpmb_sz = ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128 * 1024;
    mmc->ext_csd.erased_mem_cont = ext_csd[EXT_CSD_ERASED_MEM_CONT];
    mmc->ext_csd.part_en = ext_csd[EXT_CSD_PART_SUPPORT] & EXT_CSD_PART_SUPPORT_PART_EN ? 1 : 0;
    mmc->ext_csd.enh_attr_en = ext_csd[EXT_CSD_PART_SUPPORT] & EXT_CSD_PART_SUPPORT_ENH_ATTR_EN ? 1 : 0;
    mmc->ext_csd.enh_start_addr =
        (ext_csd[EXT_CSD_ENH_START_ADDR + 0] |
         ext_csd[EXT_CSD_ENH_START_ADDR + 1] << 8 |
         ext_csd[EXT_CSD_ENH_START_ADDR + 2] << 16 |
         ext_csd[EXT_CSD_ENH_START_ADDR + 3] << 24);
    mmc->ext_csd.enh_sz =
        (ext_csd[EXT_CSD_ENH_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_ENH_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_ENH_SIZE_MULT + 2] << 16) * 512 * 1024 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];

	u4HC_Erase_Group_Size = ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512 * 1024;

    if (mmc->ext_csd.rev >= 5) {
        if (ext_csd[EXT_CSD_BKOPS_SUPP] & 0x1)
            mmc->ext_csd.bkops_supp = 1;

        if (ext_csd[EXT_CSD_HPI_FEATURE] & 0x1) {
            mmc->ext_csd.hpi_supp = 1;
            if (ext_csd[EXT_CSD_HPI_FEATURE] & 0x2)
                mmc->ext_csd.hpi_cmd = MMC_CMD_STOP_TRANSMISSION;
            else
                mmc->ext_csd.hpi_cmd = MMC_CMD_SEND_STATUS;

            mmc->ext_csd.out_of_int_time =
                ext_csd[EXT_CSD_OUT_OF_INTR_TIME] * 10;
        }

    }

    mmc->ext_csd.max_packed_reads = ext_csd[EXT_CSD_MAX_PACKED_READS];
    mmc->ext_csd.max_packed_writes = ext_csd[EXT_CSD_MAX_PACKED_WRITES];

    if (mmc->ext_csd.rev >= 2) {
        /* 'rev > = 2' means the version of MMC is 4.2 at least, to addressing
           the block number */
        //if (mmc->ext_csd.sectors)
        //   	mmc_card_set_blockaddr(mmc);
    }

    if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_26)) {
        mmc->ext_csd.hs_max_dtr = 26000000;
    } else {
        /* MMC v4 spec says this cannot happen */
        SD_LOG(SD_LOG_ERROR, "MMCv4 but HS unsupported\r");
    }

    if (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_52) {
        mmc->ext_csd.hs_max_dtr = 52000000;
    }

    if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_DDR_52_1_2V) ||
        (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_DDR_52)) {
        mmc->ext_csd.ddr_support = 1;
        mmc->ext_csd.hs_max_dtr = 52000000;
    }

    if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_HS200_1_2V) ||
        (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_HS200_1_8V)) {
        mmc->ext_csd.hs_max_dtr = 200000000;
    }

#if SUPPORT_EMMC_50
	if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_HS400_1_2V) ||
		(ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_HS400_1_8V))
	{
		mmc->ext_csd.ddr_support = 1;
		mmc->ext_csd.hs_max_dtr = 200000000;
		mmc->ext_csd.hs400_support = 1;
	}
#endif

#if 1//emmc write protect
	mmc->ext_csd.usr_wp = ext_csd[EXT_CSD_USR_WP];
	mmc->ext_csd.boot_wp = ext_csd[EXT_CSD_BOOT_WP];
	mmc->ext_csd.boot_wp_status = ext_csd[EXT_CSD_BOOT_WP_STATUS];
	if(ext_csd[EXT_CSD_ERASE_GRP_DEF] & 0x01) {
		MMC_LOG(MMC_LOG_ERR, "use high-capacity write protect group size definition");
		mmc->wp_size = (ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]) * 1024;//num of blks

	} else {
		MMC_LOG(MMC_LOG_ERR, "use old write protect group size definition");
		mmc->wp_size = (mmc->csd.write_prot_grpsz + 1) * mmc->csd.erase_sctsz;//num of blks
	}

	MMC_LOG(MMC_LOG_ERR, "mmc wp size = %u MB\n", mmc->wp_size/2048);
#endif

#if ATC_MSDC_DUMP
    mmc_dump_ext_csd(mmc);
#endif

	#ifdef CONFIG_MSDC_ETT
	MMC_LOG(MMC_LOG_ERR, "card is ddr_support = %d, hs_max_dtr = %d", mmc->ext_csd.ddr_support, mmc->ext_csd.hs_max_dtr);
	#endif

    return MMC_ERR_NONE;
}


int mmc_get_card_registers(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    unsigned int err;

	// for MMC, SD Memory and SD Combo cards, retreive the CID
    cmd.opcode 	= MMC_CMD_ALL_SEND_CID;
    cmd.rsptype = MMC_RSP_R2;
    cmd.arg 	= 0;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err){
		MMC_LOG(MMC_LOG_ERR, "send CMD2 to get CID failed");
        return err;
    }

	memcpy(mmc->raw_cid, cmd.resp, 16);

     /*
      * For MMC cards, set the Relative Address.
      * For SD cards, get the Relatvie Address.
      * This also puts the cards into Standby State
      */
	if (mmc->card_type == MMC_TYPE_MMC)
	{
		mmc->rca = 0x5678;
		cmd.opcode 	= MMC_CMD_SET_RELATIVE_ADDR;
     	cmd.arg 	= mmc->rca << 16;
     	cmd.rsptype = MMC_RSP_R1;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;
	}
	else if (mmc->card_type == MMC_TYPE_SD)
	{
		mmc->rca = 0;
     	cmd.opcode 	= SD_CMD_SEND_RELATIVE_ADDR;
     	cmd.arg 	= mmc->rca << 16;
     	cmd.rsptype = MMC_RSP_R6;
		cmd.retries = CMD_RETRIES;
    	cmd.timeout = CMD_TIMEOUT;
	}

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err){
        MMC_LOG(MMC_LOG_ERR, "send cmd3 SEND_RCA error: err = %d", err);
        return err;
     }

	 if (mmc->card_type == MMC_TYPE_SD)
	 {
     	mmc->rca = (cmd.resp[0] >> 16) & 0xffff;
	 }
	 else if (mmc->card_type == MMC_TYPE_MMC)
	 {
	 	mmc->ocr = cmd.resp[0];
		//MMC_LOG(MMC_LOG_ERR, "mmc card ocr = 0x%.8X", mmc->ocr);
	 }

    /* Get the Card-Specific Data */
    cmd.opcode 	= MMC_CMD_SEND_CSD;
    cmd.rsptype = MMC_RSP_R2;
    cmd.arg 	= mmc->rca << 16;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err){
        MMC_LOG(MMC_LOG_ERR, "send cmd9 SEND_CSD error: err= %d", err);
        return err;
    }

    mmc->raw_csd[0] = cmd.resp[0];
    mmc->raw_csd[1] = cmd.resp[1];
    mmc->raw_csd[2] = cmd.resp[2];
    mmc->raw_csd[3] = cmd.resp[3];

	mmc_decode_csd(mmc);

	mmc_decode_cid(mmc);

	// now in order to get the SCR register we must be in the trans state
	// also in order to do a few other things, so lets select the card now and
	//leave it
	// selected
	// send CMD 7 to select the card and keep it selected, this is required  for SDIO cards
	// too as mentioned in I/O working group newsgroup
	/* Select the card, and put it into Transfer Mode */
    cmd.opcode 	= MMC_CMD_SELECT_CARD;
    cmd.rsptype = MMC_RSP_R1b;
    cmd.arg 	= mmc->rca << 16;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err){
        MMC_LOG(MMC_LOG_ERR, "send CMD7 (Select Card) error: err = %d", err);
        return err;
    }

    return 0;
}

int mmc_sd_set_buswidth_4bit(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    unsigned int err;

    cmd.opcode 	= MMC_CMD_APP_CMD;
    cmd.rsptype = MMC_RSP_R1;
    cmd.arg 	= mmc->rca << 16;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err){
		MMC_LOG(MMC_LOG_ERR, "send CMD55(App Cmd) error: err = %d\n", err);
      	return err;
    }

    cmd.opcode	= SD_CMD_APP_SET_BUS_WIDTH;
    cmd.rsptype = MMC_RSP_R1;
    cmd.arg 	= 2;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err){
		MMC_LOG(MMC_LOG_ERR, "send CMD6 (Set bus width) error: err = %d", err);
      	return err;
    }

    mmc_set_bus_width(mmc, 4);

    return 0;
}

// Only for setting 4 bit or 8 bit data bus width
int mmc_mmc_select_buswidth(struct mmc *mmc, int ddr)
{
	unsigned int err = MMC_ERR_NONE;
	unsigned int cardbuswidth = EXT_CSD_BUS_WIDTH_4;
	unsigned int hostbuswidth = 4;

	static char *data_buswidth_desc[] = {"1bit", "4bit", "8bit", "reserved", "reserved", "4bit ddr", "8bit ddr"};

	// Select 8bit data bus width
	if ((mmc->host_hw->data_pins == 8) && (mmc->card_caps & MMC_MODE_8BIT))
	{
		hostbuswidth = 8;

		if (ddr)
			cardbuswidth = EXT_CSD_BUS_WIDTH_8_DDR;
		else
			cardbuswidth = EXT_CSD_BUS_WIDTH_8;
	}
	else
	{
		hostbuswidth = 4;

		if (ddr)
			cardbuswidth = EXT_CSD_BUS_WIDTH_4_DDR;
		else
			cardbuswidth = EXT_CSD_BUS_WIDTH_4;
	}

	err = mmc_mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
			EXT_CSD_BUS_WIDTH,
			cardbuswidth);
	if (err)
	{
		MMC_LOG(MMC_LOG_ERR, "switch to %s data bus width mode for emmc failed", data_buswidth_desc[cardbuswidth]);
		return err;
	}

	#ifdef CONFIG_MSDC_ETT
	MMC_LOG(MMC_LOG_ERR, "switch to %s data bus width mode for emmc", data_buswidth_desc[cardbuswidth]);
	#endif

	if (ddr)
		mmc->ddr_mode = 1;
	else
		mmc->ddr_mode = 0;

	mmc_set_bus_width(mmc, hostbuswidth);

	return err;
}

//#ifdef CONFIG_CMD_SDAGENT
u32 mmc_get_sd_freq_from_boothdr(struct mmc *mmc)
{
	u8 boothdr_buf[512];
	BOOTL_HEADER boothdr;
	flush_invalid_cache(&boothdr_buf, 512);
	mmc_bread(mmc_bootup_device, 0, 1, &boothdr_buf);  // Slot2/Slot1, block0, 1block size
	memcpy(&boothdr, &boothdr_buf, sizeof(boothdr));

	MMC_LOG(MMC_LOG_ERR, "----->>> Get MSDC FREQ: %dMHz <<<-----", boothdr.totalBlocks);

	return boothdr.totalBlocks;
}
//#endif

int mmc_send_tuning_block_hs200(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;

	u8 tuning_blk_8bit[128];

	cmd.opcode 	= MMC_SEND_TUNING_BLOCK_HS200;
	cmd.rsptype = MMC_RSP_R1;
	cmd.arg 	= 0;
	cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

	data.src = &tuning_blk_8bit[0];
	data.blocks = 1;
	data.blocksize = 128;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);

	return err;
}

int mmc_disable_dump(struct mmc *mmc, unsigned int flag)
{
	mmc->in_ett = flag;
	mmc_log_disable = flag;
}


int mmc_force_reinit(struct mmc *mmc)
{
	mmc->host_inited 	= 0;
	mmc->card_type		= MMC_TYPE_UNKNOWN;
	return mmc_init(mmc);
}

#if MMC_CMD_TUNE
int msdc_execute_tuning(struct mmc *mmc, u32 opcode);
#endif

// Clock factor for reduce clock after error occured
static u32 clock_div_factor = 1;
#define FIRST_BOOTUP_SD_CARD		(0)
int mmc_init(struct mmc *mmc)
{
	int err;
    int i;

	if (mmc->host_inited)
	{
		return 0;
	}

start_init:

	#if FIRST_BOOTUP_SD_CARD
	{
		err = mmc->init(mmc);

	    mmc_set_clock(mmc, MSDC_INIT_CLOCK);
		mmc_set_bus_width(mmc, 1);

		//Reset the Card
	    for (i=0; i<5; i++)
		{
	    	err = mmc_go_idle(mmc);
	    	if (err){
	            MMC_LOG(MMC_LOG_ERR, "reset card error: err = %d", err);
	    		return err;
	        }
			else
			{
				break;
			}
	    }

		// Test for SD version 2
		err = mmc_send_if_cond(mmc);
	    // Reset the Card if not SD
	    if (err)
		{
	        for (i=0; i<5; i++)
	        {
	            err = mmc_go_idle(mmc);
	            if (err){
					MMC_LOG(MMC_LOG_ERR, "reset card error: err = %d", err);
	                return err;
	            }
				else
				{
					break;
				}
	        }
	    }
		else{
	        mmc->card_type = MMC_TYPE_SD;
			mmc->card_caps |= MMC_MODE_4BIT;
	    }

		if (mmc->card_type == MMC_TYPE_SD)
		{
			// Now try to get the SD card's operating condition, and test if it's a SDHC
			err = mmc_sd_send_op_cond(mmc);
	    	if (err){
	        	MMC_LOG(MMC_LOG_ERR, "Try to get OCR failed: err = %d, it's not a SD card", err);
	        	//return err;
	    	}

	    	err = mmc_get_card_registers(mmc);
	    	if (err){
	        	MMC_LOG(MMC_LOG_ERR, "cannot get card registers: err = %d \r\n", err);
			mmc->card_type = MMC_TYPE_UNKNOWN;
			return err;
		}
		}
	}

	if (mmc->card_type != MMC_TYPE_SD)
	{
	#endif

	// ================== Init it as eMMC ===================
	// reset host, clock, bus width
	err = mmc->init(mmc);
	mmc_set_clock(mmc, MSDC_INIT_CLOCK);
	mmc_set_bus_width(mmc, 1);

    err = mmc_go_idle(mmc);

	err = mmc_send_op_cond(mmc);
	if (err == 0)
	{
		//MMC_LOG(MMC_LOG_ERR,">>> Find a eMMC card <<<");
		mmc->card_type = MMC_TYPE_MMC;
		mmc->card_caps |= MMC_MODE_8BIT;
	}

	if (mmc->card_type == MMC_TYPE_MMC)
	{
		err = mmc_get_card_registers(mmc);
		if (err)
		{
			MMC_LOG(MMC_LOG_ERR, "get registers from mmc failed");
		}

		err = mmc_send_ext_csd(mmc, &mmc->raw_ext_csd[0]);
		if (err)
		{
			MMC_LOG(MMC_LOG_ERR, "get ext_csd from mmc failed");
			msdc_force_reinit(mmc);
			MMC_LOG(MMC_LOG_ERR, "force retry init");
			mmc->host_inited = 0;
			mmc->card_type = MMC_TYPE_UNKNOWN;
			goto start_init;
		}

		err = mmc_decode_ext_csd(mmc);
		if (err)
		{
			MMC_LOG(MMC_LOG_ERR, "decode ext_csd failed");
		}

		err = atc_config_emmc_booting(mmc);
		if(err) {
			MMC_LOG(MMC_LOG_ERR, "config emmc booting mode failed");
		}

		#ifdef CONFIG_MSDC_ETT
		mmc_dump_ext_csd(mmc);
		mmc_dump_erase_releated_params(mmc);
		#endif
		//mmc_change_freq(mmc);
	}

	#if FIRST_BOOTUP_SD_CARD
	}
	#endif

	#if (FIRST_BOOTUP_SD_CARD == 0)
	// ================== Init it as SD Card ====================
	if (mmc->card_type != MMC_TYPE_MMC)
	{
		err = mmc->init(mmc);

	    mmc_set_clock(mmc, MSDC_INIT_CLOCK);
		mmc_set_bus_width(mmc, 1);

		//Reset the Card
	    for (i=0; i<5; i++)
		{
	    	err = mmc_go_idle(mmc);
	    	if (err){
	            MMC_LOG(MMC_LOG_ERR, "reset card error: err = %d", err);
	    		return err;
	        }
			else
			{
				break;
			}
	    }

		// Test for SD version 2
		err = mmc_send_if_cond(mmc);
	    // Reset the Card if not SD
	    if (err)
		{
	        for (i=0; i<5; i++)
	        {
	            err = mmc_go_idle(mmc);
	            if (err){
					MMC_LOG(MMC_LOG_ERR, "reset card error: err = %d", err);
	                return err;
	            }
				else
				{
					break;
				}
	        }
	    }
		else{
	        mmc->card_type = MMC_TYPE_SD;
			mmc->card_caps |= MMC_MODE_4BIT;
	    }

		if (mmc->card_type == MMC_TYPE_SD)
		{
			// Now try to get the SD card's operating condition, and test if it's a SDHC
			err = mmc_sd_send_op_cond(mmc);
	    	if (err){
	        	MMC_LOG(MMC_LOG_ERR, "Try to get OCR failed: err = %d, it's not a SD card", err);
				if (err == UNUSABLE_ERR){
					mmc->card_type = MMC_TYPE_UNKNOWN;
	        		return err;
				}
	    	}

	    	err = mmc_get_card_registers(mmc);
	    	if (err){
	        	MMC_LOG(MMC_LOG_ERR, "cannot get card registers: err = %d \r\n", err);
			mmc->card_type = MMC_TYPE_UNKNOWN;
			return err;
		}
		}
	}
	#endif

	if (mmc->card_type == MMC_TYPE_MMC)
	{
		// Change to HS200 mode
		if ((mmc->ext_csd.hs_max_dtr == MSDC_CLK_200MHZ) && (mmc->work_clock_mode == 0) && (mmc->work_clock_freq > MSDC_CLK_100MHZ))
		{
			err = mmc_mmc_select_buswidth(mmc, BUS_MODE_SDR);

			#if EMMC_ENTER_HS200_TIMING
			if (mmc->switch_hs200){
				// Switch to HS200 Mode
				err = mmc_mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, EXT_CSD_TIMING_HS200);
				if (err)
				{
					MMC_LOG(MMC_LOG_ERR, "switch HS200 mode speed for emmc failed");
					return err;
				}
				else
				{
					MMC_LOG(MMC_LOG_ERR, "switch to HS200 mode");
				}
			}
			else
			{
				// Switch to High Speed Mode
				err = mmc_mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, EXT_CSD_TIMING_HS);
				if (err)
				{
					MMC_LOG(MMC_LOG_ERR, "switch high speed for emmc failed");
					return err;
				}
				else
				{
					MMC_LOG(MMC_LOG_ERR, "switch to high mode");
				}
			}
			#endif

			mmc_set_clock(mmc, mmc->work_clock_freq);
			//msdc_dump_register(mmc);

			// Send Tuning Block HS200 (CMD21) for eMMC
			//mmc_send_tuning_block_hs200(mmc);

		}
		// End change to HS200 mode
		// Enter EMMC DDR52 Mode
		else if ((mmc->ext_csd.ddr_support == 1) && (mmc->work_clock_mode == 1) && (mmc->work_clock_freq < MSDC_CLK_100MHZ))
		{
			if(mmc->work_clock_freq>mmc->ext_csd.hs_max_dtr){
				mmc->work_clock_freq=mmc->ext_csd.hs_max_dtr;
			}
			err = mmc_mmc_select_buswidth(mmc, BUS_MODE_DDR);
			mmc_set_clock(mmc, mmc->work_clock_freq);
		}
		else
		{
			if(mmc->work_clock_freq>mmc->ext_csd.hs_max_dtr){
				mmc->work_clock_freq=mmc->ext_csd.hs_max_dtr;
			}
			mmc_set_clock(mmc, mmc->work_clock_freq);

			// Switch to High Speed Mode
			err = mmc_mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, EXT_CSD_TIMING_HS);
			if (err)
			{
				MMC_LOG(MMC_LOG_ERR, "switch high speed for emmc failed");
				return err;
			}

			err = mmc_mmc_select_buswidth(mmc, BUS_MODE_SDR);
		}

	}

	if (mmc->card_type == MMC_TYPE_SD)
	{
	    u32 sd_freq;
		if (_sdagentflag == 1)
		{
            sd_freq = mmc_get_sd_freq_from_boothdr(mmc);
			if ((sd_freq == 0) || (sd_freq > SD_WORK_CLOCK/(1000*1000)))
			{
				sd_freq = SD_WORK_CLOCK;
			}
			else
			{
				sd_freq = sd_freq * 1000 * 1000;
			}
		}
		else
		{
		    sd_freq = mmc->work_clock_freq;
		}

		// set SD Card work clock and 4bit bus width
		//mmc_sd_change_freq(mmc); // Get SCR register and change SD Card to High-Speed mode

		mmc_set_clock(mmc, (sd_freq / clock_div_factor));

		// set SD Card work clock and 4bit bus width
		//mmc_sd_change_freq(mmc); // Get SCR register and change SD Card to High-Speed mode


		// Check Card Status. Sometimes long connected line cause status is not 'tran'.
		u32 card_status = 0;
		mmc_send_status(mmc, &card_status);
		if (R1_CURRENT_STATE(card_status) != 4) // "Tran" Status
		{
			clock_div_factor++;
			mmc_init(mmc);
		}

		mmc_sd_set_buswidth_4bit(mmc);

	}
#if MMC_CMD_TUNE
	if(mmc->host_id==0)
		msdc_execute_tuning(mmc, MMC_SEND_TUNING_BLOCK_HS200);
#endif

	mmc->host_inited = 1;
	return 0;
}

void print_mmc_devices(char separator)
{
	struct mmc *m;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

		printf("%s: %d", m->name, m->block_dev.dev);

		if (entry->next != &mmc_devices)
			printf("%c ", separator);
	}

	printf("\n");
}


// Interface  function for start_armboot invoked
int mmc_initialize(bd_t *bis)
{
	INIT_LIST_HEAD (&mmc_devices);
	cur_dev_num = 0;

    msdc_mmc_init(bis);

    //print_mmc_devices(',');

	return 0;
}

#if 1// emmc write protect
int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value)
{
    int err;
    u32 status = 0;
    uint count = 0;
    struct mmc_cmd cmd;

    cmd.opcode = MMC_CMD_SWITCH;
    cmd.arg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
        (index << 16) | (value << 8) | set;
    cmd.rsptype = MMC_RSP_R1b;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err != MMC_ERR_NONE)
        return err;

    do {
	err = mmc_send_status(mmc, &status);
        if (err) {
            printf("[mmc0] Fail to send status %d\n", err);
            break;
        }
        if (status & R1_SWITCH_ERROR) {
            printf("[mmc0] switch error. arg(0x%x)\n", cmd.arg);
            return MMC_ERR_FAILED;
        }
        if (count++ >= 600000)
        {
            printf("[%s]: timeout happend, count=%d, status=0x%x\n", __func__, count, status);
            break;
        }

    } while (!(status & R1_READY_FOR_DATA) || (R1_CURRENT_STATE(status) == 7));

    return err;
}

int mmc_switch_part(struct mmc *mmc, uint8_t part_id)
{
	int err = MMC_ERR_NONE;
	uint8_t cfg;
	uint8_t *ext_csd;

	if((!mmc) || (part_id > EMMC_PART_GP4))
		return MMC_ERR_INVALID;
	ext_csd = &mmc->raw_ext_csd[0];
	if(mmc_card_mmc(mmc) && ext_csd[EXT_CSD_REV] >= 3) {
		cfg = ext_csd[EXT_CSD_PART_CFG];

		if(part_id == (cfg & 0x7))//already set to specific partition
			return MMC_ERR_NONE;
		cfg = (cfg & ~0x7) | part_id;

		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CFG, cfg);
		if(err == MMC_ERR_NONE) {
			err = mmc_send_ext_csd(mmc, &mmc->raw_ext_csd[0]);
			if(err == MMC_ERR_NONE) {
				if(cfg != mmc->raw_ext_csd[EXT_CSD_PART_CFG])
					err = MMC_ERR_FAILED;
			}
		}
	}

	return err;
}

int mmc_ext_csd_set(struct mmc *mmc, uint16_t ext_csd_index, uint8_t val)
{
	int err = MMC_ERR_NONE;

	if(!mmc) {
		err = MMC_ERR_INVALID;
		goto out;
	}

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, ext_csd_index, val);
	if(err == MMC_ERR_NONE) {
		err = mmc_send_ext_csd(mmc, &mmc->raw_ext_csd[0]);
		if(err == MMC_ERR_NONE) {
			if(val != mmc->raw_ext_csd[ext_csd_index])
				err = MMC_ERR_FAILED;
		}
	}
out:
	return err;
}

int mmc_ext_csd_get(struct mmc *mmc, uint16_t ext_csd_index, uint8_t *val)
{
	int err = MMC_ERR_NONE;
	uint8_t cfg;
	uint8_t *ext_csd;

	if(!mmc) {
		err = MMC_ERR_INVALID;
		goto out;
	}

	ext_csd = &mmc->raw_ext_csd[0];
	*val = ext_csd[ext_csd_index];
	err= MMC_ERR_NONE;
out:
	return err;
}

int atc_config_emmc_booting(struct mmc *mmc)
{
#define ATC_VAL_177  (0x0)
#define ATC_VAL_179  (0x48)
	int ret = -1;

	if(!mmc)
		goto out;

	if(ATC_VAL_177 != mmc->raw_ext_csd[EXT_CSD_BOOT_BUS_WIDTH]) {
		if(mmc_ext_csd_set(mmc, EXT_CSD_BOOT_BUS_WIDTH, ATC_VAL_177)) {
			ret = -2;
			goto out;
		}
	}
	if(ATC_VAL_179 != (mmc->raw_ext_csd[EXT_CSD_PART_CFG] & 0xf8)) {
		if(mmc_ext_csd_set(mmc, EXT_CSD_PART_CFG, ATC_VAL_179 | (mmc->raw_ext_csd[EXT_CSD_PART_CFG] & 0x7))) {
			ret = -3;
			goto out;
		}
	}
	ret = 0;
out:
	return ret;
}

int mmc_do_write_protect(struct mmc* mmc, uint32_t addr, wp_action_t act)
{
	struct mmc_cmd cmd;
	int err = 0;

	if(act == WP_ENABLE)
	{
		cmd.opcode  = MMC_CMD_SET_WRITE_PROT;
		cmd.rsptype = MMC_RSP_R1b;
		//cmd.arg     = (mmc->high_capacity == 1) ? (wpAddr >> 9) : wpAddr;
		cmd.arg     = addr;
		cmd.retries = CMD_RETRIES;
		cmd.timeout = CMD_TIMEOUT;

		return mmc_send_cmd(mmc, &cmd, NULL);
	}
	else if(act == WP_DISABLE)
	{
		cmd.opcode  = MMC_CMD_CLR_WRITE_PROT;
		cmd.rsptype = MMC_RSP_R1b;
		//cmd.arg     = (mmc->high_capacity == 1) ? (wpAddr >> 9) : wpAddr;
		cmd.arg     = addr;
		cmd.retries = CMD_RETRIES;
		cmd.timeout = CMD_TIMEOUT;

		return mmc_send_cmd(mmc, &cmd, NULL);
	}

	return MMC_ERR_NONE;
}

int mmc_get_wp_status(struct mmc* mmc, uint32_t wpAddr, uint32_t *pWPState, uint32_t *pWPType)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int fail = 0;

	if(pWPState)
	{
		cmd.opcode 	= MMC_CMD_SEND_WRITE_PROT;
		cmd.rsptype = MMC_RSP_R1;
		//cmd.arg 	= (mmc->high_capacity == 1) ? (wpAddr >> 9) : wpAddr;
		cmd.arg 	= wpAddr;
		cmd.retries = CMD_RETRIES;
		cmd.timeout = CMD_TIMEOUT;

		data.dest = (uint8_t *)pWPState;
		data.blocks = 1;
		data.blocksize = 4;
		data.flags = MMC_DATA_READ;

		if(mmc_send_cmd(mmc, &cmd, &data) != MMC_ERR_NONE)
			fail++;
		//ReverseBuffer((u8 *)pWP, 4);
	}

	if(pWPType)
	{
		cmd.opcode 	= MMC_CMD_SEND_WRITE_PROT_TYPE;
		cmd.rsptype = MMC_RSP_R1;
		//cmd.arg 	= (mmc->high_capacity == 1) ? (wpAddr >> 9) : wpAddr;
		cmd.arg 	= wpAddr;
		cmd.retries = CMD_RETRIES;
		cmd.timeout = CMD_TIMEOUT;

		data.dest = (uint8_t *)pWPType;
		data.blocks = 1;
		data.blocksize = 8;
		data.flags = MMC_DATA_READ;

		if(mmc_send_cmd(mmc, &cmd, &data) != MMC_ERR_NONE)
			fail++;
	}

	return fail;
}

int mmc_verify_wp(struct mmc *mmc, uint32_t start_blk, uint32_t blk_nr, wp_type_t type)
{
	int err = MMC_ERR_FAILED;
	uint32_t count_wp;
	uint32_t count_wp_rest;
	uint32_t count_wp_group;
	uint8_t wp_type;
	uint32_t i, j;
	uint8_t ret_type[8] = {0xff};

	if((start_blk % mmc->wp_size) || (blk_nr % mmc->wp_size)) {
		//not aligned
		goto out;
	}

	count_wp = blk_nr / mmc->wp_size;
	count_wp_group = count_wp/32;
	count_wp_rest = count_wp%32;
	if(type == WP_TEMP_TYPE)
		wp_type = 0x55;
	else if(type == WP_PWON_TYPE)
		wp_type = 0xaa;
	else if(type == WP_PERM_TYPE)
		wp_type = 0xff;
	else
		wp_type = 0;

	for(i=0; i<count_wp_group; i++) {
		err = mmc_get_wp_status(mmc, start_blk + i * 32 * mmc->wp_size, NULL, (uint32_t *)ret_type);
		if(err) {
			MMC_LOG(MMC_LOG_ERR, "[WP] mmc_get_wp_status err %d", err);
			goto out;
		}

		for(j=0; j<8; j++) {
			if(ret_type[j] != wp_type) {//wp type not match
				goto out;
			}
		}
	}

	err = mmc_get_wp_status(mmc, start_blk + i * 32 * mmc->wp_size, NULL, (uint32_t *)ret_type);
	if(err) {
		MMC_LOG(MMC_LOG_ERR, "[WP] mmc_get_wp_status err %d", err);
		goto out;
	}
	for(j=0; j<8; j++) {
		if(count_wp_rest >= 4) {
			if(ret_type[7-j] != wp_type) {//wp type not match
				goto out;
			}
			count_wp_rest -= 4;
		}
		else if(count_wp_rest >= 1) {
			uint8_t status_real = ret_type[7-j] & (0xff >> (8 - count_wp_rest*2));
			uint8_t	status_should = wp_type >> (8 - count_wp_rest*2);
			if(status_real != status_should) {//wp type not match
				goto out;
			}
			break;
		}
		else
			break;
	}
	err = MMC_ERR_NONE;

out:
	return err;

}

static int __mmc_set_wp_internal(struct mmc *mmc, wp_type_t type, wp_action_t act, uint32_t blknr, uint32_t blkcnt)
{
	int ret = MMC_ERR_FAILED;
	uint32_t count_wp;
	uint32_t index;
	uint32_t start_blk;
	uint32_t end_blk;

	ret = mmc_switch_part(mmc, EMMC_PART_USER);
	if(ret) {
		MMC_LOG(MMC_LOG_ERR, "[WP] mmc_switch_part err %d", ret);
		goto out;
	}
	start_blk = blknr;
	end_blk = blknr + blkcnt;

	count_wp = (end_blk - start_blk)/mmc->wp_size;
	//printf("[WP] count_wp = %u\n", count_wp);

	for(index = 0; index < count_wp; index ++) {
		ret = mmc_do_write_protect(mmc, start_blk + (index * mmc->wp_size), act);
		if(ret) {
			MMC_LOG(MMC_LOG_ERR, "[WP] mmc_do_write_protect fail");
			goto out;
		}
	}

	ret = mmc_verify_wp(mmc, start_blk, end_blk-start_blk, type);
	if(ret) {
		MMC_LOG(MMC_LOG_ERR, "[WP] verify fail");
	}
out:
	return ret;
}

static int mmc_set_usr_wp_by_group(struct mmc *mmc, wp_type_t type, wp_action_t act, uint32_t blknr, uint32_t blkcnt)
{
	int ret = MMC_ERR_FAILED;
	uint32_t start_blk = blknr;
	uint32_t end_blk = blknr + blkcnt;

	if(start_blk > mmc->ext_csd.sectors)
		start_blk = mmc->ext_csd.sectors;
	if(end_blk > mmc->ext_csd.sectors)
		end_blk = mmc->ext_csd.sectors;


	if(start_blk % mmc->wp_size) {
		MMC_LOG(MMC_LOG_ERR, "[WP]error: start blk 0x%x not align", start_blk);
		goto out;
	}
	if(end_blk % mmc->wp_size) {
		MMC_LOG(MMC_LOG_ERR, "[WP]error: end blk 0x%x not align", end_blk);
		goto out;
	}

	ret = __mmc_set_wp_internal(mmc, type, act, blknr, blkcnt);
out:
	return ret;
}

static int mmc_set_usr_wp_by_group_align(struct mmc *mmc, wp_type_t type, wp_action_t act, uint32_t blknr, uint32_t blkcnt)
{
	int ret = MMC_ERR_FAILED;
	uint32_t start_blk;
	uint32_t end_blk;

	start_blk = blknr;
	end_blk = blknr + blkcnt;

	if(start_blk > mmc->ext_csd.sectors)
		start_blk = mmc->ext_csd.sectors;
	if(end_blk > mmc->ext_csd.sectors)
		end_blk = mmc->ext_csd.sectors;

	if(start_blk % mmc->wp_size) {
		start_blk -= (start_blk % mmc->wp_size);
		printf("[WP] start blk align: %u => %u\n", blknr, start_blk);
	}
	if(end_blk % mmc->wp_size) {
		end_blk += mmc->wp_size - (end_blk % mmc->wp_size);
		printf("[WP] end blk align: %u => %u\n", blknr + blkcnt , end_blk);
	}
	ret = __mmc_set_wp_internal(mmc, type, act, start_blk, end_blk-start_blk);

	return ret;

}

int mmc_set_boot_wp(struct mmc *mmc, wp_type_t type, wp_action_t act)
{
	int ret = MMC_ERR_NONE;
	uint8_t value;

	if((type >= WP_PERM_TYPE) || ((act == WP_DISABLE) && (type != WP_TEMP_TYPE))) {
		MMC_LOG(MMC_LOG_ERR, "[WP] not support");
		ret = MMC_ERR_INVALID;
		goto out;
	}
	if(!mmc_card_mmc(mmc)) {
		MMC_LOG(MMC_LOG_ERR, "[WP] Not an emmc device!");
		ret = MMC_ERR_INVALID;
		goto out;
	}

        if (mmc->csd.mmca_vsn < 4) {
		MMC_LOG(MMC_LOG_ERR, "[WP] mmc version not support");
		ret = MMC_ERR_INVALID;
                goto out;
	}

	value = mmc->ext_csd.boot_wp;

	if(type == WP_TEMP_TYPE) {
		//not support ??
	}
	else if (type == WP_PWON_TYPE) {
		if(mmc->ext_csd.boot_wp & (0x1<<6)) {//power-on wp disabled
			ret = MMC_ERR_FAILED;
			goto out;
		}

		value &= ~(0x1<<7);//all boot area
		value |= 0x1;//enable power-on wp
	}

	if(mmc->ext_csd.boot_wp == value) {
		//already set
		goto out;
	}

	ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BOOT_WP, value);
	if(ret)
		goto out;

	//check by read
	ret = mmc_send_ext_csd(mmc, &mmc->raw_ext_csd[0]);
	if(ret) {
		MMC_LOG(MMC_LOG_ERR, "[WP] read ext_csd err %d\n", ret);
		goto out;
	}
	if(value != mmc->raw_ext_csd[EXT_CSD_BOOT_WP]) {
		MMC_LOG(MMC_LOG_ERR, "[WP] check ext_csd fail");
	}

out:
	return ret;
}

#define EMMC_DEV_ID  (0)
/* we use temportary wp on user area */
int emmc_set_user_wp(wp_action_t act, uint32_t blknr, uint32_t blkcnt, uint8_t do_align)
{
	int ret = MMC_ERR_NONE;
	uint8_t value;
	struct mmc *mmc = find_mmc_device(EMMC_DEV_ID);

	if(!mmc) {
		printf("[WP] get emmc device fail\n");
		goto out;
	}
	if(mmc_init(mmc)) {
		printf("[WP] init emmc device fail\n");
		goto out;
	}

	if(!mmc_card_mmc(mmc)) {
		MMC_LOG(MMC_LOG_ERR, "[WP] Not an emmc device!");
		ret = MMC_ERR_INVALID;
		goto out;
	}

        if (mmc->csd.mmca_vsn < 4) {
		MMC_LOG(MMC_LOG_ERR, "[WP] mmc version not support");
		ret = MMC_ERR_INVALID;
                goto out;
	}

	if(mmc->wp_size == 0) {
		MMC_LOG(MMC_LOG_ERR, "[WP] wp group size is 0");
		ret = MMC_ERR_INVALID;
                goto out;

	}

	value = mmc->ext_csd.usr_wp;
	value &= ~(0x5);//clear PERN_EN, PWR_EN

	if(mmc->ext_csd.usr_wp == value) {
		//already set
		goto write_wp;
	}

	ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_USR_WP, value);
	if(ret)
		goto out;

	//check by read
	ret = mmc_send_ext_csd(mmc, &mmc->raw_ext_csd[0]);
	if(ret) {
		MMC_LOG(MMC_LOG_ERR, "[WP] read ext_csd err %d", ret);
		goto out;
	}
	if(value != mmc->raw_ext_csd[EXT_CSD_USR_WP]) {
		MMC_LOG(MMC_LOG_ERR, "[WP] check ext_csd fail");
		ret = MMC_ERR_INVALID;
		goto out;
	}

write_wp:
	if(do_align)
		ret = mmc_set_usr_wp_by_group_align(mmc, WP_TEMP_TYPE, act, blknr, blkcnt);
	else
		ret = mmc_set_usr_wp_by_group(mmc, WP_TEMP_TYPE, act, blknr, blkcnt);

	if(ret) {
		MMC_LOG(MMC_LOG_ERR, "[WP] set wp fail");
		goto out;
	}
#if 0
	//need to restore US_PWR_WP_EN ???
	ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_USR_WP,  mmc->raw_ext_csd[EXT_CSD_USR_WP] & ~(0x1));
	if(ret) {
		MMC_LOG(MMC_LOG_ERR, "[WP] restore ext_csd fail");
		goto out;
	}
#endif
out:

	return ret;
}

int emmc_clear_all_wp()
{
	struct mmc *mmc = find_mmc_device(EMMC_DEV_ID);
	uint32_t blkcnt;

	if(!mmc) {
		printf("[WP] get emmc device fail\n");
		goto fail;
	}
	if(mmc_init(mmc)) {
		printf("[WP] init emmc device fail\n");
		goto fail;
	}
	blkcnt = mmc->ext_csd.sectors;

	return emmc_set_user_wp(WP_DISABLE, 0, blkcnt, 1);
fail:
	return -1;
}

int mmc_dump_wp_status(struct mmc *mmc, uint32_t start_blk, uint32_t blk_nr)
{
	int err = MMC_ERR_FAILED;
	uint32_t count_wp, count_tmp;
	uint32_t count_wp_rest;
	uint32_t count_wp_group;
	uint8_t wp_type;
	uint32_t i;
	int k, t;
	uint8_t ret_type[8] = {0xff};
	uint8_t  region_flag = 0;
	uint32_t wp_region_start = 0, wp_region_end = 0;

	printf("wp group size: %uMB, start_blk %u, blk_cnt %u\n", mmc->wp_size/2048, start_blk, blk_nr);
	if(start_blk % mmc->wp_size) {
		printf("warning: %u is not aligned to wp group size\n", start_blk);
		start_blk -= start_blk % mmc->wp_size;
		printf("align to %u\n", start_blk);
	}
	if(blk_nr % mmc->wp_size) {
		printf("warning: %u is not aligned to wp group size\n", blk_nr);
		blk_nr -= blk_nr % mmc->wp_size;
		printf("align to %u\n", blk_nr);
	}

	count_wp = blk_nr / mmc->wp_size;
	count_wp_group = count_wp/32 + 1;
	count_tmp = 0;
	printf("Total wp group: %u\n", count_wp);
	//printf("total wp group group %d\n", count_wp_group);

	printf("=============================\n");
	for(i = 0; i < count_wp_group; i++) {
		err = mmc_get_wp_status(mmc, start_blk + i * 32 * mmc->wp_size, NULL, (uint32_t *)ret_type);
		if(err) {
			MMC_LOG(MMC_LOG_ERR, "[WP] mmc_get_wp_status err %d", err);
			goto out;
		}
#if 0//debug
		printf("[grp%d]: ret_type[7~0]:%x %x %x %x %x %x %x %x\n", i, ret_type[7], ret_type[6], ret_type[5], ret_type[4],
			ret_type[3], ret_type[2], ret_type[1], ret_type[0]);
#endif
		for(k=7; k>=0; k--) {
			for(t=0; t<4; t++, count_tmp++) {
				if(count_tmp == count_wp) {
					err = MMC_ERR_NONE;
					printf("===got end===\n");
					goto out;
				}
				wp_type = (ret_type[k] >> (t*2)) & 0x03;
				switch(wp_type) {
					case 0x01://temp
						//printf("wpg %u: is temporary pattern\n", count_tmp);
						if(region_flag == 0) {
							region_flag = 1;
							wp_region_start = (start_blk +  count_tmp * mmc->wp_size)/2048;
						}
						break;
					case 0x10://power-on
						printf("wpg %u:###is power-on parttern, please check!!!\n", count_tmp + start_blk/mmc->wp_size);
						//break;
					case 0x11://permanent
						printf("wpg %u:###is permanent pattern, please check!!!\n", count_tmp + start_blk/mmc->wp_size);
						//break;
					default://wp off
						if(region_flag) {
							region_flag = 0;
							wp_region_end =  (start_blk + count_tmp * mmc->wp_size)/2048;
							printf("region: %dMB ~ %dMB use temporary write protect\n",
									wp_region_start, wp_region_end);
						}
				}
			}
		}

	}
	err = MMC_ERR_NONE;

out:
	return err;
}

int emmc_wpg_type(uint32_t wpg_id)
{
	int ret;
	uint32_t wpg_group_rest;
	uint32_t wpg_group_id;
	uint8_t wp_type;
	uint8_t ret_type[8] = {0xff};

	struct mmc *mmc = find_mmc_device(EMMC_DEV_ID);

	if(!mmc) {
		printf("[WP] get emmc device fail\n");
		return -1;
	}
	if(mmc_init(mmc)) {
		printf("[WP] init emmc device fail\n");
		return -1;
	}

	if(wpg_id >= (mmc->ext_csd.sectors/mmc->wp_size)) {
		printf("[WP] invalid wp group id:%d\n", wpg_id);
		return -1;
	}

	wpg_group_id = wpg_id/32;
	wpg_group_rest = wpg_id%32;

	ret = mmc_get_wp_status(mmc, wpg_group_id * 32 * mmc->wp_size, NULL, (uint32_t *)ret_type);
	if(ret) {
		MMC_LOG(MMC_LOG_ERR, "[WP] mmc_get_wp_status err %d", ret);
		return -1;
	}

	//printf("ret[7~0]:%x %x %x %x %x %x %x %x\n", ret_type[7], ret_type[6], ret_type[5], ret_type[4],
	//		ret_type[3], ret_type[2], ret_type[1], ret_type[0]);

	wp_type = (ret_type[7-(wpg_group_rest/4)] >> ((wpg_group_rest%4)*2)) & 0x03;

	if(wp_type == 0x01)//temp wp
		return 1;
	else if(wp_type == 0x10) {//poweron wp
		printf("wpg %u:###is power-on parttern, please check!!!\n", wpg_id);
		return 2;
	}
	else if(wp_type == 0x11) {//permanent wp
		printf("wpg %u:###is permanent pattern, please check!!!\n", wpg_id);
		return 3;
	}

	return 0;
}
#endif
