/*
* Copyright (c) 2020 AutoChips Inc.
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

/* THE HAL BOOTCTRL HEADER MUST BE IN SYNC WITH THE UBOOT BOOTCTRL HEADER */

#ifndef ATC_BOOTCTRL_HPP
#define ATC_BOOTCTRL_HPP

#include <stdint.h>

/* struct boot_ctrl occupies the slot_suffix field of
 * struct bootloader_message */
#define OFFSETOF_SLOT_SUFFIX 4096
#define NAND_LOAD_BCB_SIZE 2 * OFFSETOF_SLOT_SUFFIX
#define NAND_LOAD_PTBL_SIZE 3 * OFFSETOF_SLOT_SUFFIX
#define TAG_AND_CHECKSUM_SIZE 20

#define BOOTCTRL_MAGIC              0x19191100
#define BOOTCTRL_SUFFIX_A           "_a"
#define BOOTCTRL_SUFFIX_B           "_b"
#define BOOTCTRL_SLOT_A             0
#define BOOTCTRL_SLOT_B             1
#define BOOTCTRL_RSV_SIZE 500

#define BOOTCTRL_SUFFIX_MAXLEN      2

#define BOOT_CONTROL_VERSION    1
#define READ_PARTITION    0
#define WRITE_PARTITION    1

#define CMDLINE_TARGET_PREFIX       "slot_suffix="

#define CMDLINE_DEV                 "/proc/cmdline"
#define BOOTCTRL_DEV                "/dev/datazone"
#define BOOTCTRLBK_DEV              "/dev/datazone_bk"

namespace atcupdateservice {
namespace bootctrl {

typedef struct slot_metadata {
    uint8_t priority : 3;
    uint8_t retry_count : 3;
    uint8_t successful_boot : 1;
    uint8_t normal_boot : 1;
} slot_metadata_t;

typedef struct boot_ctrl {
    /* Magic for identification */
    uint32_t magic;
    /* Version of struct. */
    uint8_t version;
    /* Information about each slot. */
    uint8_t doublepart;
    slot_metadata_t slot_info[2];
    uint32_t checksum;
    uint8_t reserved[BOOTCTRL_RSV_SIZE];
} boot_ctrl_t,*boot_ctrl_t_p;

typedef struct bootloader_message {
	char tags[16];
	uint32_t checksum;
	char command[32];
	char status[32];
	char recovery[32];
	unsigned int bootflag;
	char laststatus[32];
	boot_ctrl_t metadata;
	char reserved[512];
}bootloader_message_t;

std::string getSuffix(int slot);
int getSlotFromSuffix(const char *suffix);
int getCurrentSlot();
std::string getCurrentSuffix(void);
int setActiveSlot(int slot);

int setSlotInfo(const char *suffix,slot_metadata_t *slot_info);
int getSlotInfo(const char *suffix, slot_metadata_t *slot_info);
int getSlotBootable(unsigned slot);
int checkCurrentSlotValid(void);
int setSlotUnbootable(int slot);
int markBootSuccessful();
int getInactiveSlot(slot_metadata_t *slotp);

}
}
#endif /* _BOOTCTRL_H_ */