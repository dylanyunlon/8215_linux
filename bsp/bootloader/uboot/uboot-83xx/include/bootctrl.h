/*
* Copyright (c) 2019 AutoChips Inc.
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

#ifndef _BOOTCTRL_H_
#define _BOOTCTRL_H_

#include <stdint.h>

/* struct boot_ctrl occupies the slot_suffix field of
 * struct bootloader_message */
#define OFFSETOF_SLOT_SUFFIX 0

#define BOOTCTRL_MAGIC 0x19191100
#define BOOTCTRL_SUFFIX_A           "_a"
#define BOOTCTRL_SUFFIX_B           "_b"
#define BOOTCTRL_SLOT_A             0
#define BOOTCTRL_SLOT_B             1

#define BOOTCTRL_SUFFIX_MAXLEN      2

#define BOOT_CONTROL_VERSION    1
#define READ_PARTITION    0
#define WRITE_PARTITION    1
#define BOOTCTRL_RSV_SIZE 500

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

#ifdef ATC_AB_PARTITION_SUPPORT
/* bootctrl API */
void dump_slot_metadata(slot_metadata_t *slot_info);
const char *get_suffix(void);
const char *get_suffix_slot(slot_metadata_t *slot_info);
uint8_t get_retry_count(const char *suffix);
int get_bootup_status(const char *suffix);
int check_valid_slot(void);
int reduce_retry_count(const char *suffix);
int mark_slot_invalid(const char *suffix);
int set_slot_info(const char *suffix);
int get_slot_info(const char *suffix,slot_metadata_t *slot_info);
void ab_switch_slot(void);
void ab_boot_check(void);
int rollback_slot(const char *suffix);
#endif /* ATC_AB_PARTITION_SUPPORT */

#endif /* _BOOTCTRL_H_ */
