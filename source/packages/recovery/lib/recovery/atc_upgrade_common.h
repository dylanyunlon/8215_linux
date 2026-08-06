#ifndef __ATC_UPGRADE_COMMON_H__
#define __ATC_UPGRADE_COMMON_H__

#include <stdint.h>
#include "recovery.h"
#include "atc_update.h"

#define SIZE_1MB  (1 * 1024 * 1024)
#define SIZE_16MB (16 * SIZE_1MB)
#define SIZE_48MB (48 * SIZE_1MB)
#define PAGE_SIZE 4096
#define DEVNODE_NAME_LEN 24

#define WIFIMAC_DATASIZE_OFFSET 0x9F10
#define WIFIMAC_DATA_OFFSET     0x9F14
#define BTMAC_DATASIZE_OFFSET   0x9F78
#define BTMAC_DATA_OFFSET       0x9F7C
#define WIFICHIP_DATA_OFFSET    0x80A0
#define GPSCHIP_DATA_OFFSET     0x80A4
#define BTCHIP_DATA_OFFSET      0x80C0
#define FRCHIP_DATA_OFFSET      0x80C4

/*
* preloader header
*/
typedef struct _NFIType
{
    u16 pageSize;
    u16 spareSize;
    u16 addressCycle;
    u16 pageShift;
} NFI_MENU;

typedef struct _BOOTLHeader_
{
    char ID1[12];
    char version[4];
    u32 length;
    u32 startAddr;
    u32 checksum;
    char ID2[8];
    NFI_MENU NFIinfo;
    u16 pagesPerBlock;
    u16  totalBlocks;
    u16  blockShift;
    u16  linkAddr[6];
    u16  lastBlock;
} BOOTL_HEADER;

#define REPLICATION_NUMBER (512/sizeof(BOOTL_HEADER))

#define PRELOADER_SIZE (0x7000)
#define PRELOADER_MAX_TOTAL_SIZE (64 * 1024) //(64 * 1024 + 512)

struct safeupg_partitionhead {
    unsigned int blockcnt;
    unsigned int u4Version;
    unsigned int u4Signature;
    struct partitionread *nextpartition;
    char checksum[4];
};

#define BCB_TAG "BCBHead"
#define BOOTFLAG_STARTUP_A 0
#define BOOTFLAG_STARTUP_B 1

struct safeupg_bootloader_message {
    char tags[16]; //"BCBHead"
    char checksum[4];
    char command[32];
    char status[32];
    char recovery[32];
    uint32_t bootflag; //TODO, double confirm
    char laststatus[32];
    char reserved[1024];
};

#if 0
struct safeupg_image_descriptor {
    char tags[16]; //"DATAZONEHead"
    uint32_t dwLoadAddress;
    uint32_t dwLoadPhyaddr;
    uint32_t dwJumpAddress;
    uint32_t dwJumpPhyAddr;
    uint32_t dwStartAddr;
    uint32_t dwTtlLen;
    uint32_t dwReserved[16];
    uint32_t dwLoadAddress_bk;
    uint32_t dwLoadPhyAddr_bk;
    uint32_t dwJumpAddress_bk;
    uint32_t dwStartAddr_bk;
    uint32_t dwTtlLen_bk;
    uint32_t dwResrvd[16];
    uint32_t checksum[4];
};
#endif

#define DATAZONE_INFO_DZ_LEN 0x104
#define DATAZONE_INFO_RSV0 0x3C
#define DATAZONE_INFO_RSV1 (0xBC - 0x4C - 24)
#define DATAZONE_INFO_RSV2 (DATAZONE_INFO_DZ_LEN - 0xBC - 24)
#define DATAZONE_INFO_RSV3 (512 - DATAZONE_INFO_DZ_LEN - 4)

struct image_desc {
    uint32_t dwLoadAddress;
    uint32_t dwLoadPhyAddr;
    uint32_t dwJumpAddress;
    uint32_t dwJumpPhyAddr;
    uint32_t dwStartAddr;
    uint32_t dwTtlLen;
};

struct datazone_info {
    char rsv0[DATAZONE_INFO_RSV0];
    char magic[16];
    struct image_desc img_desc;
    char rsv1[DATAZONE_INFO_RSV1];
    struct image_desc img_desc_bk;
    char rsv2[DATAZONE_INFO_RSV2];
    char checksum[4];
    char rsv3[DATAZONE_INFO_RSV3];
};

#ifndef CONFIG_BOOT_MMC
#define MIN_PARTITION_BLOCK_NUM 1024
#define RESERVED_DATA_SIZE 6
#define RESERVED_PAGE_SIZE 2048
#define RESERVED_NUM 3
#define GET_RESERVED_BLOCK_NUM(x) ((((x) * RESERVED_DATA_SIZE) / RESERVED_PAGE_SIZE) + RESERVED_NUM) //X is partition block size
#endif
#define RECOVERY_UPDATE_ZIP_NAME "/mnt/recovery_update.zip"
#define NTFS_MOUNT_DEVICE_LEN 20
extern char ntfs_mount_device[NTFS_MOUNT_DEVICE_LEN];
int hotplug_device_tune(int id, char *str);
int get_emmc_total_size(unsigned long long *ptotal_sz);
int get_file_len(const char *path, unsigned long long *plen);
int is_file_exist_by_pathname(const char *file_path);
int check_name_in_sets(const char **namesets, const char *name);
void freetblmemory(partitionread *ptbl);
partitionread *lookup_partition_by_name(partitionread *ptbl, const char*name);

int read_partition_head(struct safeupg_partitionhead *phead, const char *devnode, unsigned long offset);
partitionread *read_partition_info(struct safeupg_partitionhead *phead,
        const char *devnode, unsigned long offset);
int write_partition_info(struct safeupg_partitionhead *phead, partitionread *ptbl,
        const char *devnode, unsigned long offset);

void adjust_nextpartition_field(partitionread *ptbl);
uint32_t get_partition_info_checksum(struct safeupg_partitionhead *phead);
uint32_t calc_partition_info_checksum(partitionread *ptbl);
int check_partition_info_checksum(struct safeupg_partitionhead *phead, partitionread *ptbl);
int partition_info_readback_check(const char *devnode, unsigned long offset);
int read_common_area(const char *devnode, unsigned long offset, void *pbuf, int len);
int write_common_area(const char *devnode, unsigned long offset, void *pbuf, int len);
int read_datazone(struct datazone_info*pdz, const char *devnode, unsigned long offset);
int write_datazone(struct datazone_info *pdz, const char *devnode, unsigned long offset);
uint32_t get_datazone_checksum(struct datazone_info *pdz);
uint32_t calc_datazone_checksum(struct datazone_info *pdz);
void put_datazone_checksum(struct datazone_info *pdz, uint32_t chksum);
int check_datazone_checksum(struct datazone_info *pdz);
int datazone_readback_check(const char *devnode, unsigned long offset);
void adjust_datazone_img_desc_bk(struct datazone_info *pdz, partitionread *ptbl_ub_bk);
int read_bcb(struct safeupg_bootloader_message *pbcb, const char *devnode, unsigned long offset);
int write_bcb(struct safeupg_bootloader_message *pbcb, const char *devnode, unsigned long offset);
uint32_t get_bcb_checksum(struct safeupg_bootloader_message *pbcb);
uint32_t calc_bcb_checksum(struct safeupg_bootloader_message *pbcb);
void put_bcb_checksum(struct safeupg_bootloader_message *pbcb, uint32_t chksum);
int check_bcb_tag_checksum(struct safeupg_bootloader_message *pbcb);
int bcb_readback_check(const char *devnode, unsigned long offset);
uint32_t get_bcb_bootflag(struct safeupg_bootloader_message *pbcb);
void put_bcb_bootflag(struct safeupg_bootloader_message *pbcb, uint32_t bootflag);
void set_bcb_tags(struct safeupg_bootloader_message *pbcb);
void print_bcb_tags(struct safeupg_bootloader_message *pbcb);
void dump_writeprotect_region(int fdwp);
int clear_writeprotect(int fdwp);
int open_for_writeprotect(void);
void close_for_writeprotect(int fdwp);
#ifdef NEW_PARTITION_DESIGN
int update_writeprotect_region(int fdwp, partitionread *ptbl);
#endif /* NEW_PARTITION_DESIGN */

partitionread *read_partition_info_from_xml_file(char *xml_file);
int adjust_xml_partition_info_size(partitionread *ptbl, int board_type);
int check_partition_overlap(partitionread *ptbl);
bool check_is_file_exist(const char *file);
int check_files_exist_for_upgrade(partitionread *ptbl, const char *dir_path);
int check_file_exist_for_one_table(partitionread *ptbl, const char *dir_path);
int partition_need_imagefile(partitionread *ptbl);
int upg_raw_partition_from_file(const char *devname,
	    const char *file, long long offset, long long size, long long *psize_total);
int upg_ext4_partition_from_file(const char *devname,
	    const char *file, long long offset, long long size);
int create_bootloader_header(char *pbl_header, char* blbuf,
	    uint32_t image_size, int msdc_boot);
int verify_bootloader_header(BOOTL_HEADER *pbl_header, int msdc_boot);

#ifndef CONFIG_BOOT_MMC
int64_t nand_raw_common_write(void *buf, uint64_t offset, uint64_t size);
int64_t nand_raw_partition_write_by_emptycore(void *buf,
	    uint64_t offset, uint64_t size,
	    uint64_t part_size);
int64_t nand_raw_partition_write_offset_by_emptycore(void *buf,
	    uint64_t offset, uint64_t offset_in_part,
	    uint64_t size, uint64_t real_size,
	    uint64_t part_size);

int64_t nand_raw_partition_write_by_safeupg(
	    const char *partname, void *buf,
	    uint64_t offset, uint64_t size, uint64_t part_size);
int64_t nand_raw_partition_write_offset_by_safeupg(
	    const char *partname, void *buf,
	    uint64_t offset, uint64_t offset_in_part,
	    uint64_t size, uint64_t real_size,
	    uint64_t part_size);

int64_t nand_raw_common_read(void *buf, uint64_t offset, uint64_t size);
int64_t nand_raw_partition_read_by_emptycore(void *buf,
	    uint64_t offset, uint64_t size, uint64_t part_size);
int64_t nand_raw_partition_read_offset_by_emptycore(void *buf,
	    uint64_t offset, uint64_t offset_in_part,
	    uint64_t size, uint64_t part_size);
int64_t nand_raw_partition_read_by_safeupg(
	    const char *pt_name, void *buf,
	    uint64_t offset, uint64_t size, uint64_t part_size);
int64_t nand_raw_partition_read_offset_by_safeupg(
	    const char *pt_name, void *buf,
	    uint64_t offset, uint64_t offset_in_part,
	    uint64_t size, uint64_t part_size);
int64_t nand_ext4_common_write(void *buf, uint64_t offset, uint64_t size);
int64_t nand_ext4_common_read(void *buf, uint64_t offset, uint64_t size);
int get_nand_page_size(uint32_t *psize);
int get_nand_block_cnt(uint32_t *pcnt);
int get_nand_block_size(uint32_t *pblk_size);
int get_nand_size(uint64_t *pnand_size);
int get_nand_oob_size(uint32_t *poob_size);
int nand_preloader_readback_check(partitionread *ptbl, int mode);
int lookup_idx_by_partname(const char *partname);
uint32_t calc_checksum_from_nand_after_upg(
	    long long offset, long long realdata_size,
	    partitionread *pentry, int upg_stage);
int nand_clear_all_protect(void);
int nand_partition_reserve_blk_check(int64_t pt_start, int64_t pt_size);

#define DATAZONE_PARTITION_SIZE			(0x80000)

#endif /* CONFIG_BOOT_MMC */

#ifdef CONFIG_BOOT_MMC

#define DATAZONE_MAIN_OFFSET_FROM_MMCBLK	(64 * 1024)
#define BCB_MAIN_OFFSET_FROM_MMCBLK (DATAZONE_MAIN_OFFSET_FROM_MMCBLK + 4 * 1024)
#define PARTITION_INFO_MAIN_OFFSET_FROM_MMCBLK (BCB_MAIN_OFFSET_FROM_MMCBLK + 4 * 1024)
#define DATAZONE_BK_OFFSET_FROM_MMCBLK	(PARTITION_INFO_MAIN_OFFSET_FROM_MMCBLK + 472 * 1024)
#define BCB_BK_OFFSET_FROM_MMCBLK (DATAZONE_BK_OFFSET_FROM_MMCBLK + 4 * 1024)
#define PARTITION_INFO_BK_OFFSET_FROM_MMCBLK (BCB_BK_OFFSET_FROM_MMCBLK + 4 * 1024)

#define BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN (4 * 1024)
#define PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN (BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN + 4 * 1024)
//#define DATAZONE_BK_OFFSET_FROM_DATAZONE_MAIN (PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN + 472 * 1024)

#define BCB_BK_OFFSET_FROM_DATAZONE_BK (4 * 1024)
#define PARTITION_INFO_BK_OFFSET_FROM_DATAZONE_BK (BCB_BK_OFFSET_FROM_DATAZONE_BK + 4 * 1024)

#else /* !CONFIG_BOOT_MMC */

#define DATAZONE_MAIN_OFFSET_FROM_MMCBLK	(0x100000)
#define BCB_MAIN_OFFSET_FROM_MMCBLK (DATAZONE_MAIN_OFFSET_FROM_MMCBLK + 4 * 1024)
#define PARTITION_INFO_MAIN_OFFSET_FROM_MMCBLK (BCB_MAIN_OFFSET_FROM_MMCBLK + 4 * 1024)
#define DATAZONE_BK_OFFSET_FROM_MMCBLK	(PARTITION_INFO_MAIN_OFFSET_FROM_MMCBLK + 504 * 1024)
#define BCB_BK_OFFSET_FROM_MMCBLK (DATAZONE_BK_OFFSET_FROM_MMCBLK + 4 * 1024)
#define PARTITION_INFO_BK_OFFSET_FROM_MMCBLK (BCB_BK_OFFSET_FROM_MMCBLK + 4 * 1024)

#define BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN (4 * 1024)
#define PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN (BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN + 4 * 1024)
//#define DATAZONE_BK_OFFSET_FROM_DATAZONE_MAIN (PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN + 472 * 1024)

#define BCB_BK_OFFSET_FROM_DATAZONE_BK (4 * 1024)
#define PARTITION_INFO_BK_OFFSET_FROM_DATAZONE_BK (BCB_BK_OFFSET_FROM_DATAZONE_BK + 4 * 1024)

#endif /* CONFIG_BOOT_MMC */

int udisk_mount(const char *mount_point);
int ntfs_mount(char *device, char *mount_point);

#endif /* __ATC_UPGRADE_COMMON_H__ */
