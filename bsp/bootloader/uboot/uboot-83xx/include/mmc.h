/*
 * Copyright 2008, Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based (loosely) on the Linux code
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

#ifndef _MMC_H_
#define _MMC_H_

#include <linux/list.h>

#include "../drivers/mmc/atc_msdc_customed.h"

#define UBOOT_MMC_LOG_ENABLE	(1)
#if UBOOT_MMC_LOG_ENABLE

// Log level define
#define MMC_LOG_ERR				((u32)1 << 0)
#define MMC_LOG_CARD			((u32)1 << 1)
#define MMC_LOG_REQ				((u32)1 << 2)
#define MMC_LOG_CLK				((u32)1 << 3)
#define MMC_LOG_RETRY			((u32)1 << 4)
#define MMC_LOG_INIT			((u32)1 << 5)
#define MMC_LOG_FUNC			((u32)1 << 6)
#define MMC_LOG_RW				((u32)1 << 11)
#define MMC_LOG_BUFFER			((u32)1 << 12)
#define MMC_LOG_DBG				((u32)1 << 13)
#define MMC_LOG_INFO			((u32)1 << 15)

#define LOG_TAG		"mmc"
static unsigned int mmc_log_mask = MMC_LOG_ERR | MMC_LOG_DBG;
extern unsigned int mmc_log_disable;

#define MMC_LOG(mask, format, x...) \
if ((mmc_log_mask & (mask)) && !mmc_log_disable){	\
	if (mmc){	\
		printf("[MMC%d]: <%s> <Line %u> --> "format" <--\r\n", mmc->host_id, __FUNCTION__, __LINE__, ## x);	\
	}	\
	else {	\
		printf("[MMC]: <%s> <Line %u> --> "format" <--\r\n", __FUNCTION__, __LINE__, ## x);	\
	}	\
}

#else
#define MMC_LOG(mask, format, x...)
#endif


#define SD_VERSION_SD	0x20000
#define SD_VERSION_2	(SD_VERSION_SD | 0x20)
#define SD_VERSION_1_0	(SD_VERSION_SD | 0x10)
#define SD_VERSION_1_10	(SD_VERSION_SD | 0x1a)

#define MMC_VERSION_MMC		0x10000
#define MMC_VERSION_UNKNOWN	(MMC_VERSION_MMC)
#define MMC_VERSION_1_2		(MMC_VERSION_MMC | 0x12)
#define MMC_VERSION_1_4		(MMC_VERSION_MMC | 0x14)
#define MMC_VERSION_2_2		(MMC_VERSION_MMC | 0x22)
#define MMC_VERSION_3		(MMC_VERSION_MMC | 0x30)
#define MMC_VERSION_4		(MMC_VERSION_MMC | 0x40)

#define MMC_MODE_HS			0x001
#define MMC_MODE_HS_52MHz	0x010
#define MMC_MODE_4BIT		0x100
#define MMC_MODE_8BIT		0x200

#define SD_DATA_4BIT	0x00040000

#define MMC_DATA_READ		1
#define MMC_DATA_WRITE		2

#define NO_CARD_ERR			(-16) /* No SD/MMC card inserted */
#define UNUSABLE_ERR		(-17) /* Unusable Card */
#define COMM_ERR			(-18) /* Communications Error */
#define TIMEOUT				(-19)

#define SD_CMD_BIT                      (1 << 7)
#define SD_CMD_APP_BIT                  (1 << 8)
#define SD_CMD_AUTO_BIT                 (1 << 9)

// Command definitions
#define MMC_CMD_GO_IDLE_STATE			(0)
#define MMC_CMD_SEND_OP_COND			(1)
#define MMC_CMD_ALL_SEND_CID			(2)
#define MMC_CMD_SET_RELATIVE_ADDR		(3)
#define MMC_CMD_SET_DSR					(4)
#define MMC_CMD_SWITCH					(6)
#define MMC_CMD_SELECT_CARD				(7)
#define MMC_CMD_SEND_EXT_CSD			(8)
#define MMC_CMD_SEND_CSD				(9)
#define MMC_CMD_SEND_CID				(10)
#define MMC_CMD_STOP_TRANSMISSION		(12)
#define MMC_CMD_SEND_STATUS				(13)
#define MMC_CMD_SET_BLOCKLEN			(16)
#define MMC_CMD_READ_SINGLE_BLOCK		(17)
#define MMC_CMD_READ_MULTIPLE_BLOCK		(18)
#define MMC_CMD_BUSTEST_W               (19)            /* adtc. R1         */
#define MMC_SEND_TUNING_BLOCK_HS200		(21)			/* adtc R1  		*/
#define MMC_CMD_WRITE_SINGLE_BLOCK		(24)
#define MMC_CMD_WRITE_MULTIPLE_BLOCK	(25)
#define MMC_CMD_SET_WRITE_PROT		(28)
#define MMC_CMD_CLR_WRITE_PROT		(29)
#define MMC_CMD_SEND_WRITE_PROT		(30)
#define MMC_CMD_SEND_WRITE_PROT_TYPE	(31)
#define MMC_CMD_SET_TIME                (49)            /* adtc. R1         */
#define MMC_CMD_APP_CMD					(55)

 /* class 5 */
#define MMC_ERASE_GROUP_START    		(35)   /* ac   [31:0] data addr   R1  */
#define MMC_ERASE_GROUP_END      		(36)   /* ac   [31:0] data addr   R1  */
#define MMC_ERASE                		(38)   /* ac                      R1b */

#define SD_CMD_SWITCH_FUNC				(6)
#define SD_CMD_APP_SET_BUS_WIDTH		(6)
#define SD_CMD_APP_SEND_OP_COND			(41)
#define SD_CMD_APP_SEND_SCR				(51)

/* SD Card command numbers */
#define SD_CMD_SEND_RELATIVE_ADDR       (3 | SD_CMD_BIT)
#define SD_CMD_SWITCH                   (6 | SD_CMD_BIT)
#define SD_CMD_SEND_IF_COND             (8 | SD_CMD_BIT)
#define SD_CMD_VOL_SWITCH               (11 | SD_CMD_BIT)
#define SD_CMD_SEND_TUNING_BLOCK        (19 | SD_CMD_BIT)
#define SD_CMD_SPEED_CLASS_CTRL         (20 | SD_CMD_BIT)

#define SD_ACMD_SET_BUSWIDTH	        (6  | SD_CMD_APP_BIT)
#define SD_ACMD_SD_STATUS               (13 | SD_CMD_APP_BIT)
#define SD_ACMD_SEND_NR_WR_BLOCKS       (22 | SD_CMD_APP_BIT)
#define SD_ACMD_SET_WR_ERASE_CNT        (23 | SD_CMD_APP_BIT)
#define SD_ACMD_SEND_OP_COND            (41 | SD_CMD_APP_BIT)
#define SD_ACMD_SET_CLR_CD              (42 | SD_CMD_APP_BIT)
#define SD_ACMD_SEND_SCR                (51 | SD_CMD_APP_BIT)

/* SCR definitions in different words */
#define SD_HIGHSPEED_BUSY				0x00020000
#define SD_HIGHSPEED_SUPPORTED			0x00020000

#define MMC_HS_TIMING					0x00000100
#define MMC_HS_52MHZ					0x2

#define OCR_BUSY	0x80000000
#define OCR_HCS		0x40000000

#define MMC_VDD_165_195		0x00000080	/* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21		0x00000100	/* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22		0x00000200	/* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23		0x00000400	/* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24		0x00000800	/* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25		0x00001000	/* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26		0x00002000	/* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27		0x00004000	/* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28		0x00008000	/* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29		0x00010000	/* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30		0x00020000	/* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31		0x00040000	/* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32		0x00080000	/* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33		0x00100000	/* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34		0x00200000	/* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35		0x00400000	/* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36		0x00800000	/* VDD voltage 3.5 ~ 3.6 */

#define MMC_SWITCH_MODE_CMD_SET		0x00 /* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS	0x01 /* Set bits in EXT_CSD byte
						addressed by index which are
						1 in value field */
#define MMC_SWITCH_MODE_CLEAR_BITS	0x02 /* Clear bits in EXT_CSD byte
						addressed by index, which are
						1 in value field */
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03 /* Set target byte to value */

#define SD_SWITCH_CHECK		0
#define SD_SWITCH_SWITCH	1

#define R1_OUT_OF_RANGE         (1UL << 31) /* er, c */
#define R1_ADDRESS_ERROR        (1 << 30)   /* erx, c */
#define R1_BLOCK_LEN_ERROR      (1 << 29)   /* er, c */
#define R1_ERASE_SEQ_ERROR      (1 << 28)   /* er, c */
#define R1_ERASE_PARAM          (1 << 27)   /* ex, c */
#define R1_WP_VIOLATION         (1 << 26)   /* erx, c */
#define R1_CARD_IS_LOCKED       (1 << 25)   /* sx, a */
#define R1_LOCK_UNLOCK_FAILED   (1 << 24)   /* erx, c */
#define R1_COM_CRC_ERROR        (1 << 23)   /* er, b */
#define R1_ILLEGAL_COMMAND      (1 << 22)   /* er, b */
#define R1_CARD_ECC_FAILED      (1 << 21)   /* ex, c */
#define R1_CC_ERROR             (1 << 20)   /* erx, c */
#define R1_ERROR                (1 << 19)   /* erx, c */
#define R1_UNDERRUN             (1 << 18)   /* ex, c */
#define R1_OVERRUN              (1 << 17)   /* ex, c */
#define R1_CID_CSD_OVERWRITE    (1 << 16)   /* erx, c, CID/CSD overwrite */
#define R1_WP_ERASE_SKIP        (1 << 15)   /* sx, c */
#define R1_CARD_ECC_DISABLED    (1 << 14)   /* sx, a */
#define R1_ERASE_RESET          (1 << 13)   /* sr, c */
#define R1_STATUS(x)            (x & 0xFFFFE000)
#define R1_CURRENT_STATE(x)     ((x & 0x00001E00) >> 9) /* sx, b (4 bits) */
#define R1_READY_FOR_DATA       (1 << 8)    /* sx, a */
#define R1_SWITCH_ERROR         (1 << 7)    /* ex, b */
#define R1_URGENT_BKOPS         (1 << 6)    /* sr, a */
#define R1_EXCEPTION_EVENT      (1 << 6)    /* sr, a */
#define R1_APP_CMD              (1 << 5)    /* sr, c */


/*
 * EXT_CSD fields
 */

#define EXT_CSD_BADBLK_MGMT             134 /* R/W */
#define EXT_CSD_ENH_START_ADDR          136 /* R/W 4 bytes */
#define EXT_CSD_ENH_SIZE_MULT           140 /* R/W 3 bytes */
#define EXT_CSD_GP1_SIZE_MULT           143 /* R/W 3 bytes */
#define EXT_CSD_GP2_SIZE_MULT           146 /* R/W 3 bytes */
#define EXT_CSD_GP3_SIZE_MULT           149 /* R/W 3 bytes */
#define EXT_CSD_GP4_SIZE_MULT           152 /* R/W 3 bytes */
#define EXT_CSD_PART_SET_COMPL          155 /* R/W */
#define EXT_CSD_PART_ATTR               156 /* R/W 3 bytes */
#define EXT_CSD_MAX_ENH_SIZE_MULT       157 /* R/W 3 bytes */
#define EXT_CSD_PART_SUPPORT            160 /* R */
#define EXT_CSD_HPI_MGMT                161 /* R/W/E_P (4.41) */
#define EXT_CSD_RST_N_FUNC              162 /* R/W */
#define EXT_CSD_BKOPS_EN                163 /* R/W (4.41) */
#define EXT_CSD_BKOPS_START             164 /* W/E_P (4.41) */
#define EXT_CSD_SANITIZE_START			165 /* W */
#define EXT_CSD_WR_REL_PARAM            166 /* R (4.41) */
#define EXT_CSD_WR_REL_SET              167 /* R/W (4.41) */
#define EXT_CSD_RPMB_SIZE_MULT          168 /* R */
#define EXT_CSD_FW_CONFIG               169 /* R/W */
#define EXT_CSD_USR_WP                  171 /* R/W, R/W/C_P & R/W/E_P */
#define EXT_CSD_BOOT_WP                 173 /* R/W, R/W/C_P */
#define EXT_CSD_BOOT_WP_STATUS		174 /* R */
#define EXT_CSD_ERASE_GRP_DEF           175 /* R/W/E */
#define EXT_CSD_BOOT_BUS_WIDTH          177 /* R/W/E */
#define EXT_CSD_BOOT_CONFIG_PROT        178 /* R/W & R/W/C_P */
#define EXT_CSD_PART_CFG                179 /* R/W/E & R/W/E_P */
#define EXT_CSD_ERASED_MEM_CONT         181 /* R */
#define EXT_CSD_BUS_WIDTH               183 /* R/W */
#define EXT_CSD_HS_TIMING               185 /* R/W */
#define EXT_CSD_PWR_CLASS               187 /* R/W/E_P */
#define EXT_CSD_CMD_SET_REV             189 /* R */
#define EXT_CSD_CMD_SET                 191 /* R/W/E_P */
#define EXT_CSD_REV                     192 /* R */
#define EXT_CSD_STRUCT                  194 /* R */
#define EXT_CSD_CARD_TYPE               196 /* RO */
#define EXT_CSD_OUT_OF_INTR_TIME        198 /* R (4.41) */
#define EXT_CSD_PART_SWITCH_TIME        199 /* R (4.41) */
#define EXT_CSD_PWR_CL_52_195           200 /* R */
#define EXT_CSD_PWR_CL_26_195           201 /* R */
#define EXT_CSD_PWR_CL_52_360           202 /* R */
#define EXT_CSD_PWR_CL_26_360           203 /* R */
#define EXT_CSD_MIN_PERF_R_4_26         205 /* R */
#define EXT_CSD_MIN_PERF_W_4_26         206 /* R */
#define EXT_CSD_MIN_PERF_R_8_26_4_25    207 /* R */
#define EXT_CSD_MIN_PERF_W_8_26_4_25    208 /* R */
#define EXT_CSD_MIN_PERF_R_8_52         209 /* R */
#define EXT_CSD_MIN_PERF_W_8_52         210 /* R */
#define EXT_CSD_SEC_CNT                 212 /* RO, 4 bytes */
#define EXT_CSD_S_A_TIMEOUT             217 /* R */
#define EXT_CSD_S_C_VCCQ                219 /* R */
#define EXT_CSD_S_C_VCC                 220 /* R */
#define EXT_CSD_HC_WP_GPR_SIZE          221 /* R */
#define EXT_CSD_REL_WR_SEC_C            222 /* R */
#define EXT_CSD_ERASE_TIMEOUT_MULT      223 /* R */
#define EXT_CSD_HC_ERASE_GRP_SIZE       224 /* R */
#define EXT_CSD_ACC_SIZE                225 /* R */
#define EXT_CSD_BOOT_SIZE_MULT          226 /* R */
#define EXT_CSD_BOOT_INFO               228 /* R */
#define EXT_CSD_SEC_TRIM_MULT           229 /* R */
#define EXT_CSD_SEC_ERASE_MULT          230 /* R */
#define EXT_CSD_SEC_FEATURE_SUPPORT     231 /* R */
#define EXT_CSD_TRIM_MULT               232 /* R */
#define EXT_CSD_MIN_PERF_DDR_R_8_52     234 /* R */
#define EXT_CSD_MIN_PERF_DDR_W_8_52     235 /* R */
#define EXT_CSD_PWR_CL_DDR_52_195       238 /* R */
#define EXT_CSD_PWR_CL_DDR_52_360       239 /* R */
#define EXT_CSD_INI_TIMEOUT_AP          241 /* R */
#define EXT_CSD_CORRECT_PRG_SECTS_NUM   242 /* R, 4 bytes (4.41) */
#define EXT_CSD_BKOPS_STATUS            246 /* R (4.41) */
#define EXT_CSD_MAX_PACKED_WRITES       500 /* R (4.5) */
#define EXT_CSD_MAX_PACKED_READS        501 /* R (4.5) */
#define EXT_CSD_BKOPS_SUPP              502 /* R (4.41) */
#define EXT_CSD_HPI_FEATURE             503 /* R (4.41) */
#define EXT_CSD_S_CMD_SET               504 /* R */


/*
 * EXT_CSD field definitions
 */

/* PARTITIONING_SUPPORT[160] */
#define EXT_CSD_PART_SUPPORT_PART_EN     (1)
#define EXT_CSD_PART_SUPPORT_ENH_ATTR_EN (1<<1)

#define EXT_CSD_CMD_SET_NORMAL			(1<<0)
#define EXT_CSD_CMD_SET_SECURE			(1<<1)
#define EXT_CSD_CMD_SET_CPSECURE		(1<<2)

/* CARD_TYPE[196] */
#define EXT_CSD_CARD_TYPE_26            (1<<0)  /* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52            (1<<1)  /* Card can run at 52MHz */
#define EXT_CSD_CARD_TYPE_DDR_52        (1<<2)  /* Card can run at DDR 52MHz@1.8V or 3V */
#define EXT_CSD_CARD_TYPE_DDR_52_1_2V   (1<<3)  /* Card can run at DDR 52MHz@1.2V */
#define EXT_CSD_CARD_TYPE_HS200_1_8V    (1<<4)  /* Card can run at DDR 52MHz@1.2V */
#define EXT_CSD_CARD_TYPE_HS200_1_2V    (1<<5)  /* Card can run at DDR 52MHz@1.2V */
#if SUPPORT_EMMC_50
#define EXT_CSD_CARD_TYPE_HS400_1_8V    (1<<6)  /* Card can run at DDR 200MHz@1.8V */
#define EXT_CSD_CARD_TYPE_HS400_1_2V    (1<<7)  /* Card can run at DDR 200MHz@1.2V */
#endif 


/* BUS_WIDTH[183] */
#define EXT_CSD_BUS_WIDTH_1             (0) /* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4             (1) /* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8             (2) /* Card is in 8 bit mode */
#define EXT_CSD_BUS_WIDTH_4_DDR         (5) /* Card is in 4 bit mode + DDR */
#define EXT_CSD_BUS_WIDTH_8_DDR         (6) /* Card is in 8 bit mode + DDR */
	#define BUS_MODE_SDR	(0)
	#define BUS_MODE_DDR	(1)

/* HS_TIMING[185] */
#define EXT_CSD_TIMING_DF               (0) /* Card is in default speed mode */
#define EXT_CSD_TIMING_HS               (1) /* Card is in hign speed mode */
#define EXT_CSD_TIMING_HS200            (2) /* Card is in HS200 mode */


#define R1_ILLEGAL_COMMAND		(1 << 22)
#define R1_APP_CMD			(1 << 5)

#define MMC_RSP_PRESENT (1 << 0)
#define MMC_RSP_136     (1 << 1)                /* 136 bit response */
#define MMC_RSP_CRC     (1 << 2)                /* expect valid crc */
#define MMC_RSP_BUSY    (1 << 3)                /* card may send busy */
#define MMC_RSP_OPCODE  (1 << 4)                /* response contains opcode */

#define MMC_RSP_NONE    (0)
#define MMC_RSP_R1      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE| \
			MMC_RSP_BUSY)
#define MMC_RSP_R2      (MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3      (MMC_RSP_PRESENT)
#define MMC_RSP_R4      (MMC_RSP_PRESENT)
#define MMC_RSP_R5      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)


#define BOOT_OP_NONE             		(0)          // no operation, for init
#define BOOT_OP_ENTER_BOOT_MODE       	(1)          // enter boot mode
#define BOOT_OP_EXIT_BOOT_MODE          (2)          // exit boot mode
#define BOOT_OP_WRITE_BOOT_PART1        (5)          // write boot partition 1
#define BOOT_OP_READ_BOOT_PART1         (6)          // read boot partition 1
#define BOOT_OP_WRITE_BOOT_PART2        (7)          // write boot partition 2
#define BOOT_OP_READ_BOOT_PART2         (8)          // read boot partition 2

#define BOOT_PART_NONE                 	(0)
#define BOOT_PART_PART1                 (1)
#define BOOT_PART_PART2                 (2)

#define CMD6_ARGU_EXTCSD(index, value)   	((0x03<<24)|(index<<16)|(value<<8))
#define CMD6_ARGS(access, index, value)		(((access&0x3)<<24)|((index&0xff)<<16)|((value&0xff)<<8))
#define EXT_CSD_179_BOOTPARTITION_MASK		(7)
#define EXT_CSD_179_BOOTPARTITION_BOOT1		(1)
#define EXT_CSD_179_BOOTPARTITION_BOOT2		(2)

#define EXT_CSD_ACCESS_MODE_COMMANDSET		(0)
#define EXT_CSD_ACCESS_MODE_SETBITS			(1)
#define EXT_CSD_ACCESS_MODE_CLEARBITS		(2)
#define EXT_CSD_ACCESS_MODE_WRITEBYTE		(3)

#define BOOT_CONFIG_RESET_SOFTWARE 		(1)
#define BOOT_CONFIG_RESET_HARDWARE		(2)

#define EXT_CSD_179_BOOT_ACK   							(0x01 << 6)
#define EXT_CSD_179_NO_BOOT_ACK							(0x00 << 6)
#define EXT_CSD_179_HAS_BOOT_ACK						(0x01 << 6)

#define EXT_CSD_179_BOOT_PARTITION_ENABLE   			(0x07 << 3)
#define EXT_CSD_179_BOOT_PARTITION_ENABLE_NO_BOOT		(0x00 << 3)
#define EXT_CSD_179_BOOT_PARTITION_ENABLE_BOOT1			(0x01 << 3)
#define EXT_CSD_179_BOOT_PARTITION_ENABLE_BOOT2			(0x02 << 3)
#define EXT_CSD_179_BOOT_PARTITION_ENABLE_USER_PART		(0x07 << 3)

#define EXT_CSD_179_PARTITION_ACCESS   					(0x07 << 0)
#define EXT_CSD_179_PARTITION_ACCESS_NO_BOOT			(0x00 << 0)
#define EXT_CSD_179_PARTITION_ACCESS_BOOT1				(0x01 << 0)
#define EXT_CSD_179_PARTITION_ACCESS_BOOT2 				(0x02 << 0)

#define EXT_CSD_177_BOOT_SPEED_MODE						(0x03 << 3)
#define EXT_CSD_177_BOOT_SPEED_MODE_BC					(0x00 << 3)
#define EXT_CSD_177_BOOT_SPEED_MODE_HS					(0x01 << 3)
#define EXT_CSD_177_BOOT_SPEED_MODE_DDR					(0x02 << 3)

#define EXT_CSD_177_BOOT_WITDH_REBOOT					(0x01 << 2)

#define EXT_CSD_177_BOOT_BUS_WITDH						(0x03 << 0)
#define EXT_CSD_177_BOOT_BUS_WITDH_X1					(0x00 << 0)
#define EXT_CSD_177_BOOT_BUS_WITDH_X4					(0x01 << 0)
#define EXT_CSD_177_BOOT_BUS_WITDH_X8					(0x02 << 0)

#define EXT_CSD_171_PWR_WP_EN                                           (0x01 << 0)
#define EXT_CSD_171_PERM_WP_EN                                          (0x01 << 2)
#define EXT_CSD_171_PWR_WP_DIS						(0x01 << 3)
//#define EXT_CSD_171_US_PERM_WP_DIS					(0x01 << 4)//one time programed
//#define EXT_CSD_171_CD_PERM_WP_DIS					(0x01 << 6)//one time programed
#define EXT_CSD_173_B_PWR_WP_EN                                         (0x01 << 0)
#define EXT_CSD_173_B_PWR_WP_SEC_SEL					(0x01 << 1)
//#define EXT_CSD_173_B_PERM_WP_EN                                      (0x01 << 2)//one time programed
//#define EXT_CSD_173_B_PERM_WP_DIS					(0x01 << 4)//one time programed
#define EXT_CSD_173_B_PWR_WP_DIS					(0x01 << 6)
#define EXT_CSD_173_B_SEC_WP_SEL					(0x01 << 7)

#define MMC_OCR_1V7_1V95                (1 << 7)
#define MMC_OCR_2V7_3V6                 (0x1FF << 15)
#define MMC_OCR_SECTOR_MODE             (2 << 29)

/*
 * Given a 128-bit response, decode to our card CSD structure.
 */
#define CSD_STRUCT_VER_1_0  0   /* Valid for system specification 1.0 - 1.2 */
#define CSD_STRUCT_VER_1_1  1   /* Valid for system specification 1.4 - 2.2 */
#define CSD_STRUCT_VER_1_2  2   /* Valid for system specification 3.1 - 3.2 - 3.31 - 4.0 - 4.1 */
#define CSD_STRUCT_EXT_CSD  3   /* Version is coded in CSD_STRUCTURE in EXT_CSD */


struct mmc_cmd {
    u32 opcode;
    u32 arg;
    u32 rsptype;
    u32 resp[4];
    u32 timeout;
    u32 retries;    /* max number of retries */
    u32 error;      /* command error */ 
};


struct mmc_data {
	union {
		u8 *dest;
		const u8 *src; /* src buffers don't get written to */
	};
	uint flags;
	uint blocks;
	uint blocksize;
};

#define MMC_TYPE_UNKNOWN  (0x00)
#define MMC_TYPE_SD 	(0x01)
#define MMC_TYPE_MMC 	(0x02)

struct mmc_csd {
    unsigned char  csd_struct;          /* csd structure version */
    unsigned char  mmca_vsn;
    unsigned short cmdclass;            /* card command classes */
    unsigned short tacc_clks;           /* data read access-time-1 in clks */
    unsigned int   tacc_ns;             /* data read access-time-2 */
    unsigned int   r2w_factor;          /* write speed factor */
    unsigned int   max_dtr;             /* max. data transfer rate */
    unsigned int   read_blkbits;        /* max. read data block length */
    unsigned int   write_blkbits;       /* max. write data block length */
    unsigned long long   capacity;            /* card capacity */
    unsigned int   erase_sctsz;         /* erase sector size */
    unsigned int   write_prot_grpsz;
    unsigned int   read_partial:1,
                   read_misalign:1,
                   write_partial:1,
                   write_misalign:1,
                   write_prot_grp:1,
                   perm_wr_prot:1,
                   tmp_wr_prot:1,
                   erase_blk_en:1,
                   copy:1,
                   dsr:1;
};


struct mmc_cid {
    unsigned int   manfid;    
    char           prod_name[8];
    unsigned int   serial;
    unsigned short oemid;
    unsigned short year;
    u8  hwrev;		//unsigned char
    unsigned char  fwrev;
    unsigned char  month;
    unsigned char  cbx;                 /* device type: card(0) BGA(1) POP(2) */
};

struct mmc_ext_csd {
    unsigned int    trim_tmo_ms;
    unsigned int    hc_wp_grp_sz;
    unsigned int    hc_erase_grp_sz;
    unsigned int    sectors;
    unsigned int    hs_max_dtr;
    unsigned int    boot_part_sz;
    unsigned int    rpmb_sz;
    unsigned int    access_sz;
    unsigned int    enh_sz;
    unsigned int    enh_start_addr;
    unsigned char   rev;
    unsigned char   boot_info;    
    unsigned char   part_en:1,
                    enh_attr_en:1,
                    ddr_support:1;
    unsigned char   erased_mem_cont;
    unsigned char   bkops_supp:1;
    unsigned char   hpi_supp:1;
    unsigned int    hpi_cmd;
    unsigned int    out_of_int_time;
    unsigned char   max_packed_reads;
    unsigned char   max_packed_writes;
    unsigned char   usr_wp;
    unsigned char   boot_wp;
    unsigned char   boot_wp_status;
};

struct mmc {
	struct list_head link;
	char name[32];
	struct msdc_host* host_hw;
    uint base_address;
	uint voltages;
	uint version;
	uint f_min;
	uint f_max;
	uint high_capacity;
	uint bus_width;
	uint clock;					/* request setting clock */
	uint cur_clock;				/* current msdc module set clock */
	uint real_clock;			/* real clock according to selected clock source */
	uint work_clock_freq;		/* after init, msdc use this clock to work */
	uint work_clock_mode;		/* 0 - SDR, 1 - DDR */
	uint switch_hs200;
	uint ddr_mode;				/* request setting ddr mode */
	uint cur_ddr_mode;			/* current msdc module ddr mode state */
	uint card_caps;
	uint in_ett;
	uint ocr;
	uint raw_scr[2];
	uint raw_csd[4];
	uint raw_cid[4];
	u8 raw_ext_csd[512];
	ushort rca;
	struct mmc_csd csd;
	struct mmc_cid cid;
	struct mmc_ext_csd ext_csd;
	uint tran_speed;
	uint read_bl_len;
	uint write_bl_len;
	u64  capacity;
	u32  card_type;
	uint host_inited;		// Host has been init
	uint in_boot_mode;		// Card in boot mode, only for slot0
	uint host_id;
	uint tune_clock_times;
	uint in_tuning_process;
	block_dev_desc_t block_dev;
	int (*send_cmd)(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data);
	int (*set_ios)(struct mmc *mmc);
	int (*init)(struct mmc *mmc);
	uint wp_size; //write protect group size
};

#define mmc_card_mmc(c)		((c)->card_type == MMC_TYPE_MMC)
#define mmc_card_sd(c)		((c)->card_type == MMC_TYPE_SD)

typedef enum
{
WP_ALL_CARD=0,
WP_USER_AREA,
WP_BOOT_AREA,
WP_MAX_AREA,
} wp_area_t;

typedef enum
{
WP_TEMP_TYPE=0,
WP_PWON_TYPE,
WP_PERM_TYPE,
WP_MAX_TYPE,
} wp_type_t;

typedef enum
{
WP_DISABLE=0,
WP_ENABLE,
} wp_action_t;

/* PARTITION CONFIG[179] */
#define EXT_CSD_PART_CFG_DEFT_PART      (0)
#define EXT_CSD_PART_CFG_BOOT_PART_1    (1)
#define EXT_CSD_PART_CFG_BOOT_PART_2    (2)
#define EXT_CSD_PART_CFG_RPMB_PART      (3)
#define EXT_CSD_PART_CFG_GP_PART_1      (4)
#define EXT_CSD_PART_CFG_GP_PART_2      (5)
#define EXT_CSD_PART_CFG_GP_PART_3      (6)
#define EXT_CSD_PART_CFG_GP_PART_4      (7)
#define EXT_CSD_PART_CFG_EN_NO_BOOT     (0 << 3)
#define EXT_CSD_PART_CFG_EN_BOOT_PART_1 (1 << 3)
#define EXT_CSD_PART_CFG_EN_BOOT_PART_2 (2 << 3)
#define EXT_CSD_PART_CFG_EN_USER_AREA   (7 << 3)
#define EXT_CSD_PART_CFG_EN_NO_ACK      (0 << 6)
#define EXT_CSD_PART_CFG_EN_ACK         (1 << 6)

#define EMMC_PART_USER		(0)
#define EMMC_PART_BOOT1		(1)
#define EMMC_PART_BOOT2		(2)
#define EMMC_PART_RPMB		(3)
#define EMMC_PART_GP1		(4)
#define EMMC_PART_GP2		(5)
#define EMMC_PART_GP3		(6)
#define EMMC_PART_GP4		(7)


int mmc_register(struct mmc *mmc);
int mmc_initialize(bd_t *bis);
int mmc_init(struct mmc *mmc);
ulong mmc_bwrite(int dev_num, ulong start, lbaint_t blkcnt, const void*src);
ulong mmc_bread(int dev_num, ulong start, lbaint_t blkcnt, void *dst);
// add for emmc boot partition opereation
int mmc_boot_enter_bootmode(int dev_num);
int mmc_boot_exit_bootmode(int dev_num);
ulong mmc_boot_bwrite(int dev_num, int boot_part_num, ulong start, lbaint_t blkcnt, const void*src);
ulong mmc_boot_bread(int dev_num, int boot_part_num, ulong start, lbaint_t blkcnt, void *dst);


struct mmc *find_mmc_device(int dev_num);
void print_mmc_devices(char separator);
extern int mmc_dump_wp_status(struct mmc *mmc, uint32_t start_blk, uint32_t blk_nr);
extern int emmc_set_user_wp(wp_action_t act, uint32_t blknr, uint32_t blkcnt, uint8_t do_align);
extern int emmc_clear_all_wp();
extern int emmc_wpg_type(uint32_t wpg_id);
extern int mmc_ext_csd_get(struct mmc *mmc, uint16_t ext_csd_index, uint8_t *val);
extern int mmc_ext_csd_set(struct mmc *mmc, uint16_t ext_csd_index, uint8_t val);

#ifndef CONFIG_GENERIC_MMC
int mmc_legacy_init(int verbose);
#endif
#endif /* _MMC_H_ */
