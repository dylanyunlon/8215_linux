/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#include <stdint.h>
#include <common.h>
#include <command.h>
#include <mmc.h>
#include <nand.h>
#include <config.h>
#include <partition.h>
#include <types.h>
#include "bootctrl.h"

#ifdef ATC_AB_PARTITION_SUPPORT

const char *suffix[2] = {BOOTCTRL_SUFFIX_A, BOOTCTRL_SUFFIX_B};

void dump_slot_metadata(slot_metadata_t *slot_info)
{
    int i;

    printf("++++ slot_metadata ++++\n");
    for (i = 0; i < 2; i++) {
        printf("priority[%d]=%d\n", i, slot_info[i].priority);
        printf("retry_count[%d]=%d\n", i, slot_info[i].retry_count);
        printf("successful_boot[%d]=%d\n", i, slot_info[i].successful_boot);
        printf("normal_boot[%d]=%d\n", i, slot_info[i].normal_boot);
    }
}

void dumpBuffer(char* buff, int size) {
    int i;

    printf("Dump buffer\n");
    for (i=0;i<size;i++) {
        printf("++++ buff[%d] = 0x%x ++++\n", i, buff[i]);
    }
}

const char *get_suffix_slot(slot_metadata_t *slot_info)
{
    int slot = 0, ret = -1;
    boot_ctrl_t metadata;

    if(slot_info[0].priority >= slot_info[1].priority)
        slot = 0;
    else if (slot_info[0].priority < slot_info[1].priority)
        slot = 1;

    return suffix[slot];
}

const char *get_suffix(void)
{
    int slot = 0;
    struct bootloader_message bcb;

    memset(&bcb, 0x0, sizeof(struct bootloader_message));
    if (read_datazone_bcb(&bcb)) {
        printf("read_datazone_bcb fail.\n");
        return NULL;
    }

    if (bcb.metadata.slot_info[0].priority >= bcb.metadata.slot_info[1].priority)
        slot = 0;
    else if (bcb.metadata.slot_info[0].priority < bcb.metadata.slot_info[1].priority)
        slot = 1;

    return suffix[slot];
}

int check_suffix_with_slot(const char *suffix)
{
    int slot = -1;

    if(suffix == NULL) {
        printf("input suffix is NULL\n");
        return -1;
    }

    if(!strcmp(suffix, BOOTCTRL_SUFFIX_A)) {
        slot = 0;
    } else if(!strcmp(suffix, BOOTCTRL_SUFFIX_B)) {
        slot = 1;
    } else {
        printf("unknow slot suffix\n");
    }

    return slot;
}

uint8_t get_retry_count(const char *suffix)
{
    int slot = 0;
    //slot_metadata_t slot_info[2];
    struct bootloader_message bcb;

    //get_slotinfo_from_bcb(slot_info);
    memset(&bcb, 0x0, sizeof(struct bootloader_message));
    slot = check_suffix_with_slot(suffix);
    if(slot == -1) {
        printf("get_retry_count failed, slot: 0x%x\n", slot);
        return 0;
    }

    if (read_datazone_bcb(&bcb)) {
        printf("read_datazone_bcb fail.\n");
        return 0;
    }

    return bcb.metadata.slot_info[slot].retry_count;
}

int check_valid_slot(void)
{
    //slot_metadata_t slot_info[2];
    struct bootloader_message bcb;

    memset(&bcb, 0x0, sizeof(struct bootloader_message));
    if (read_datazone_bcb(&bcb)) {
        printf("read_datazone_bcb fail.\n");
        return -1;
    }

    //get_slotinfo_from_bcb(slot_info);
    if(bcb.metadata.slot_info[0].priority > 0)
        return 0;
    else if(bcb.metadata.slot_info[1].priority > 0)
        return 0;

    printf("#### check slot invalid ! ####\n");
    return -1;
}

int get_bootup_status(const char *suffix)
{
    int slot = 0;
    //slot_metadata_t slot_info[2];
    struct bootloader_message bcb;

    memset(&bcb, 0x0, sizeof(struct bootloader_message));
    slot = check_suffix_with_slot(suffix);
    if(slot == -1) {
        printf("set_not_normal_boot failed, slot: 0x%x\n", slot);
        return -1;
    }

    if (read_datazone_bcb(&bcb)) {
        printf("read_datazone_bcb fail.\n");
        return -1;
    }

    //get_slotinfo_from_bcb(slot_info);
    return (int)bcb.metadata.slot_info[slot].successful_boot;
}

int reduce_retry_count(const char *suffix)
{
    int slot = 0, ret = -1;
    slot_metadata_t slot_info[2];
    struct bootloader_message bcb;

    memset(&bcb, 0x0, sizeof(struct bootloader_message));
    if (read_datazone_bcb(&bcb)) {
        printf("read_datazone_bcb fail.\n");
        return -1;
    }

    slot = check_suffix_with_slot(suffix);
    if(slot == -1) {
        printf("check fail, slot: 0x%x\n", slot);
        return -1;
    }

    if(bcb.metadata.slot_info[slot].retry_count > 0)
        bcb.metadata.slot_info[slot].retry_count--;

    ret = set_datazone_bcb(&bcb);
    if(ret < 0) {
        printf("set_datazone_bcb fail, ret: 0x%x\n", ret);
        return -1;
    }

    return 0;
}


int mark_slot_invalid(const char *suffix)
{
    int slot = 0, ret = -1;
    slot_metadata_t *slotp;
    struct bootloader_message bcb;

    memset(&bcb, 0x0, sizeof(struct bootloader_message));
    if (read_datazone_bcb(&bcb)) {
        printf("read_datazone_bcb fail.\n");
        return -1;
    }

    slot = check_suffix_with_slot(suffix);
    if(slot == -1) {
        printf("invliad slot , slot: 0x%x\n", slot);
        return -1;
    }

    slotp = &(bcb.metadata.slot_info[slot]);
    slotp->successful_boot = 0;
    slotp->priority = 0;
    slotp->retry_count = 0;

    ret = set_datazone_bcb(&bcb);
    if(ret < 0) {
        printf("set_datazone_bcb fail, ret: 0x%x\n", ret);
        return -1;
    }

    return 0;
}

int rollback_slot(const char *suffix)
{
    int slot = 0, slot1 = 0;
    int ret = -1;
    slot_metadata_t *slotp;
    struct bootloader_message bcb;

    memset(&bcb, 0x0, sizeof(struct bootloader_message));
    if (read_datazone_bcb(&bcb)) {
        printf("read_datazone_bcb fail.\n");
        return -1;
    }

    slot = check_suffix_with_slot(suffix);
    if(slot == -1) {
        printf("rollback_slot fail, slot: 0x%x\n", slot);
        return -1;
    }

    if(suffix == NULL) {
        printf("suffix NULL\n");
        return -1;
    }

    printf("rollback_slot to %d\n", slot);

    /* Set highest priority and reset retry count */
    slotp = &(bcb.metadata.slot_info[slot]);
    slotp->priority = 3;

    /* Ensure other slot doesn't have as high a priority. */
    slot1 = (slot == 0) ? 1 : 0;
    slotp = &(bcb.metadata.slot_info[slot1]);
    if(slotp->priority == 3)
        slotp->priority = 3 - 1;

    ret = set_datazone_bcb(&bcb);
    if(ret) {
        printf("partition_write fail, ret: 0x%x\n", ret);
        return -1;
    }

    printf("rollback_slot success\n");
    return 0;
}

void ab_boot_check(void)
{
    int ab_slot = 0;
    const char *ab_suffix = get_suffix();
    int ab_retry = get_retry_count(ab_suffix);
    printf("#### ab_retry:%d ####\n", ab_retry);

    if(0 == strcmp(ab_suffix, BOOTCTRL_SUFFIX_A)) {
        ab_slot = 0;
    } else {
        ab_slot = 1;
    }
    printf("#### ab_slot:%d ####\n", ab_slot);

    //one or more valid slot
    while(!check_valid_slot()) {
        printf("ab_suffix: %s, ab_retry: %d\n", ab_suffix, ab_retry);
        if(get_bootup_status(ab_suffix)) {
            printf("boot_success is 1!\n");
            return;
        } else {
            if(ab_retry > 0) {
                reduce_retry_count(ab_suffix);
                return;
            } else {
                mark_slot_invalid(ab_suffix);

                if(check_valid_slot() == -1) {
                    break;
                }

                if(!memcmp(ab_suffix, BOOTCTRL_SUFFIX_A, 2)) {
                    ab_suffix = BOOTCTRL_SUFFIX_B;
                    rollback_slot(BOOTCTRL_SUFFIX_B);
                } else {
                    ab_suffix = BOOTCTRL_SUFFIX_A;
                    rollback_slot(BOOTCTRL_SUFFIX_A);
                }
                printf("Reboot for switch slot !!!\n");
                _reset(0, NULL);
                //ab_retry = get_retry_count(ab_suffix);
            }
        }
    }

    printf("no valid slot!\n");
    return;
}

#endif /* ATC_AB_PARTITION_SUPPORT */