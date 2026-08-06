#include <common.h>
#include <command.h>
#include <mmc.h>


#include "fat.h"

#ifndef CONFIG_BOOT_MMC
#define NAND_UPG_RDBACK_CHECK 1
#endif


extern int check_udisk_available(void);
extern int check_ext_sdcard_available(void);
extern int upg_rsd_read_raw_image(char* filename, char* dev_type, int dev_part, unsigned long long u8Addr, unsigned long long u8Size);
extern int upg_rsd_write_raw_image(char* filename, unsigned long long u8ReadAddr, unsigned long long u8WriteAddr, unsigned long long u8Size);
extern int upg_rsd_read_ext4_image(char* filename, char* dev_type, int dev_part, unsigned long long u8Addr, unsigned long long u8Size);
extern int upg_rsd_write_ext4_image(unsigned long long u8ReadAddr, unsigned long long u8WriteAddr, unsigned long long u8Size);
extern int upg_rsd_read_partial_ext4_image(char* filename, char* dev_type, int dev_part, unsigned long long u8Addr, unsigned long long u8Pos, unsigned long long u8Size);
extern int upg_rsd_write_partial_ext4_image(unsigned long long u8ReadAddr, unsigned long long u8WriteAddr, unsigned long long u8Size, uint32_t block_size, int chunk_cnt);

extern int do_rsd_upgrade(void);
