#include <stdint.h>
#include <string.h>
#include "x_printf.h"
#include "mtd_nand.h"
#include "partition.h"
#include "bootctrl.h"
#include "x_typedef.h"

#ifdef ATC_AB_PARTITION_SUPPORT

const char *suffix[2] = { BOOTCTRL_SUFFIX_A, BOOTCTRL_SUFFIX_B };
extern struct bootloader_message bcb;

#define BOOTCTRL_TAG_AND_CHECKSUM_SZIE 20

extern int set_datazone_bcb(struct bootloader_message *bcb);

UINT32 calculate_checksum(void *data, UINT32 size)
{
    UINT32 s = 0, i;
    uint8_t *p = (uint8_t *)data;

    for (i = 0; i < size; i++)
        s += p[i];

    return s;
}

#if 0
static int is_valid_bcb(struct bootloader_message *bcb)
{
    UINT32 exp;
    UINT32 got;

    if (bcb->metadata.magic != BOOTCTRL_MAGIC) {
        Printf("Invalid magic: 0x%x\n", bcb->metadata.magic);
        return 0;
    }

    if (bcb->metadata.version != BOOT_CONTROL_VERSION) {
        Printf("Invalid version: %d\n", bcb->metadata.version);
        return 0;
    }

    exp = calculate_checksum((char *)bcb + BOOTCTRL_TAG_AND_CHECKSUM_SZIE,
                             sizeof(struct bootloader_message) - BOOTCTRL_TAG_AND_CHECKSUM_SZIE);

    got = ((UINT32)bcb->checksum[3] << 24) |
          ((UINT32)bcb->checksum[2] << 16) |
          ((UINT32)bcb->checksum[1] << 8) |
          (UINT32)bcb->checksum[0];

    if (exp != got) {
        Printf("Checksum mismatch: calculated 0x%x, stored 0x%x\n", exp, got);
        return 0;
    }

    return 1;
}

static void init_default_bcb(struct bootloader_message *bcb)
{
    UINT32 chk;

    memset(bcb, 0, sizeof(*bcb));
    memcpy(bcb->tags, "BCBHead", sizeof("BCBHead"));

    bcb->metadata.magic = BOOTCTRL_MAGIC;
    bcb->metadata.version = BOOT_CONTROL_VERSION;
    bcb->metadata.doublepart = 0;

    bcb->metadata.slot_info[0].priority = 3;
    bcb->metadata.slot_info[0].retry_count = 3;
    bcb->metadata.slot_info[0].successful_boot = 0;
    bcb->metadata.slot_info[0].normal_boot = 1;

    bcb->metadata.slot_info[1].priority = 2;
    bcb->metadata.slot_info[1].retry_count = 3;
    bcb->metadata.slot_info[1].successful_boot = 0;
    bcb->metadata.slot_info[1].normal_boot = 1;

    chk = calculate_checksum((char *)bcb + BOOTCTRL_TAG_AND_CHECKSUM_SZIE,
                             sizeof(struct bootloader_message) - BOOTCTRL_TAG_AND_CHECKSUM_SZIE);
    memcpy(bcb->checksum, &chk, sizeof(chk));
}

static int read_bcb(struct bootloader_message *dtz_bcb)
{
    int ret;

    memcpy(dtz_bcb, &bcb, sizeof(struct bootloader_message));
    init_default_bcb(bcb);
    return 0;
}
#endif

static int slot_from_suffix(const char *s)
{
    if (!s) {
        Printf("input suffix is NULL\n");
        return -1;
    }

    if (!strcmp(s, BOOTCTRL_SUFFIX_A))
        return 0;

    if (!strcmp(s, BOOTCTRL_SUFFIX_B))
        return 1;

    Printf("unknown slot suffix\n");
    return -1;
}

const char *get_suffix_slot(slot_metadata_t *slot_info)
{
    if (!slot_info)
        return NULL;

    return (slot_info[0].priority >= slot_info[1].priority) ?
           suffix[0] : suffix[1];
}

const char *get_suffix(void)
{
    return get_suffix_slot(bcb.metadata.slot_info);
}

int check_suffix_with_slot(const char *suffix_in)
{
    return slot_from_suffix(suffix_in);
}

uint8_t get_retry_count(const char *suffix_in)
{
    int slot = slot_from_suffix(suffix_in);

    if (slot == -1) {
        Printf("get_retry_count failed, slot: 0x%x\n", slot);
        return 0;
    }

    return bcb.metadata.slot_info[slot].retry_count;
}

int check_valid_slot(void)
{
    if (bcb.metadata.slot_info[0].priority > 0 ||
        bcb.metadata.slot_info[1].priority > 0)
        return 0;

    Printf("#### check slot invalid ! ####\n");
    return -1;
}

int get_bootup_status(const char *suffix_in)
{
    int slot = slot_from_suffix(suffix_in);

    if (slot == -1) {
        Printf("get_bootup_status failed, slot: 0x%x\n", slot);
        return -1;
    }

    return (int)bcb.metadata.slot_info[slot].successful_boot;
}

int reduce_retry_count(const char *suffix_in)
{
    int slot = slot_from_suffix(suffix_in);
    int ret;

    if (slot == -1) {
        Printf("check fail, slot: 0x%x\n", slot);
        return -1;
    }

    if (bcb.metadata.slot_info[slot].retry_count > 0)
    bcb.metadata.slot_info[slot].retry_count--;

    ret = set_datazone_bcb(&bcb);
    if (ret < 0) {
        Printf("set_datazone_bcb fail, ret: 0x%x\n", ret);
        return -1;
    }

    return 0;
}

int mark_slot_invalid(const char *suffix_in)
{
    int slot = slot_from_suffix(suffix_in);
    int ret;

    if (slot == -1) {
        Printf("invalid slot , slot: 0x%x\n", slot);
        return -1;
    }

    bcb.metadata.slot_info[slot].successful_boot = 0;
    bcb.metadata.slot_info[slot].priority = 0;
    bcb.metadata.slot_info[slot].retry_count = 0;

    ret = set_datazone_bcb(&bcb);
    if (ret < 0) {
        Printf("set_datazone_bcb fail, ret: 0x%x\n", ret);
        return -1;
    }

    return 0;
}

int rollback_slot(const char *suffix_in)
{
    int slot = slot_from_suffix(suffix_in);
    int other_slot;
    int ret;

    if (slot == -1) {
        Printf("rollback_slot fail, slot: 0x%x\n", slot);
        return -1;
    }

    Printf("rollback_slot to %d\n", slot);

    bcb.metadata.slot_info[slot].priority = 3;

    other_slot = (slot == 0) ? 1 : 0;
    if (bcb.metadata.slot_info[other_slot].priority >= 3)
    bcb.metadata.slot_info[other_slot].priority = 2;

    ret = set_datazone_bcb(&bcb);
    if (ret) {
        Printf("set_datazone_bcb fail, ret: 0x%x\n", ret);
        return -1;
    }

    Printf("rollback_slot success\n");
    return 0;
}

void ab_boot_check(void)
{
    const char *ab_suffix;
    int ab_retry;
    int ab_slot;

    ab_suffix = get_suffix();
    if (!ab_suffix) {
        Printf("no valid slot!\n");
        return;
    }

    ab_retry = get_retry_count(ab_suffix);
    Printf("#### ab_retry:%d ####\n", ab_retry);

    ab_slot = !strcmp(ab_suffix, BOOTCTRL_SUFFIX_A) ? 0 : 1;
    Printf("#### ab_slot:%d ####\n", ab_slot);

    while (!check_valid_slot()) {
        //Printf("ab_suffix: %s\n", ab_suffix);
        Printf("ab_retry: %d\n", ab_retry);

        if (get_bootup_status(ab_suffix)) {
            Printf("boot_success is 1!\n");
            return;
        }

        if (ab_retry > 0) {
            reduce_retry_count(ab_suffix);
            return;
        }

        mark_slot_invalid(ab_suffix);

        if (check_valid_slot() == -1)
            break;

        if (!strcmp(ab_suffix, BOOTCTRL_SUFFIX_A)) {
            ab_suffix = BOOTCTRL_SUFFIX_B;
            rollback_slot(BOOTCTRL_SUFFIX_B);
        } else {
            ab_suffix = BOOTCTRL_SUFFIX_A;
            rollback_slot(BOOTCTRL_SUFFIX_A);
        }
        ab_retry = get_retry_count(ab_suffix);
    }

    Printf("no valid slot!\n");
}

#endif /* ATC_AB_PARTITION_SUPPORT */