/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef _RECOVERY_H_
#define _RECOVERY_H_
#include <syslog.h> 
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define RECOVERYTAG "[Recovery] "
#define rec_dbg(fmt, arg...)    printf(RECOVERYTAG "Debug:%s, " fmt, __FUNCTION__, ##arg)
#define rec_info(fmt, arg...)    printf(RECOVERYTAG "Info:%s, " fmt, __FUNCTION__, ##arg)
#define rec_note(fmt, arg...)    printf(RECOVERYTAG "Note:%s, " fmt, __FUNCTION__, ##arg)
#define rec_warn(fmt, arg...)    printf(RECOVERYTAG "Warn:%s, " fmt, __FUNCTION__, ##arg)
#define rec_err(fmt, arg...)    printf(RECOVERYTAG "Error:%s, " fmt, __FUNCTION__, ##arg)

#define ALIGN(x, a) (((x) + ((a) -1)) & (~((a) - 1)))
#define ALIGN_DOWN(x, a) (((x)) & (~((a) - 1)))
#define IS_ALIGN(x, a) (((x) & (((a) - 1))) == 0UL)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
/* yzp debug
#ifndef NAND_PROJ
#define CONFIG_BOOT_MMC 1
#endif
*/


#define SPARSE_HEADER_MAGIC    0xed26ff3a
#define CHUNK_TYPE_RAW         0xCAC1
#define CHUNK_TYPE_FILL        0xCAC2
#define CHUNK_TYPE_DONT_CARE   0xCAC3
#define CHUNK_TYPE_CRC32       0xCAC4
#define CHUNK_HEADER_LEN       (sizeof(chunk_header_t))
#define DEF_CHUNK_SIZE         0x1000000 //16 MB
#define FILE_RW_SIZE           0x100000UL // 1MB


typedef unsigned char uchar;
//typedef short  uint16_t;
//typedef int    uint32_t ;

typedef struct chunk_header {
  uint16_t       chunk_type;     /* 0xCAC1 -> raw; 0xCAC2 -> fill; 0xCAC3 -> don't care */
  uint16_t       reserved1;
  uint32_t        chunk_sz;       /* in blocks in output image */
  uint32_t        total_sz;       /* in bytes of chunk input file including chunk header and data */
} chunk_header_t;


typedef struct sparse_header {
  uint32_t    magic;            /* 0xed26ff3a */
  uint16_t   major_version;    /* (0x1) - reject images with higher major versions */
  uint16_t   minor_version;    /* (0x0) - allow images with higer minor versions */
  uint16_t   file_hdr_sz;      /* 28 bytes for first revision of the file format */
  uint16_t   chunk_hdr_sz;     /* 12 bytes for first revision of the file format */
  uint32_t    blk_sz;           /* block size in bytes, must be a multiple of 4 (4096) */
  uint32_t    total_blks;       /* total blocks in the non-sparse output image */
  uint32_t    total_chunks;     /* total chunks in the sparse input image */
  uint32_t    image_checksum;   /* CRC32 checksum of the original data, counting "don't care" */
                              /* as 0. Standard 802.3 polynomial, use a Public Domain */
                              /* table implementation */
} sparse_header_t;
//write protect info 
#define MSDC_EMMC_WRITE_PROTECT _IOW('r',12,int)
#define WP_PARTICIAL_ENABLE  (0X11)
#define WP_PARTICIAL_DISABLE (0X22)
#define WP_ALL_DISABLE       (0X33)
#define WP_REGIONINFO_GET    (0X44)
#define WP_REGION_ENABLE     (0X55)
#define WP_CLEAR_AND_SAVE    (0X66)
#define WP_RESTORE           (0X77)
#define MAX_DUMP_BUFF_SIZE   (1024)
#define DEVICE_NODE "dev/misc-sd"
#define MAX_PARTITION_NAME_LEN (64)

struct wp_cmd_arg {
    int wp_action;
    unsigned int wpg_size_of_xml;//the write protect group size configed in xml
    unsigned int sect_start;
    unsigned int sect_end;
    char *partition_name;
    char *wp_dump_info;
};
/*for emmc*/
#define MSDC_ERASE_SELECTED_AREA            (11)
struct msdc_ioctl{
    int  opcode;
    int  host_num;
    int  iswrite;
    int  trans_type;
    unsigned int  total_size;
    unsigned int  address;
    unsigned int* buffer;
    int  cmd_pu_driving;
    int  cmd_pd_driving;
    int  dat_pu_driving;
    int  dat_pd_driving;
    int  clk_pu_driving;
    int  clk_pd_driving;
    int  ds_pu_driving;
    int  ds_pd_driving;
    int  rst_pu_driving;
    int  rst_pd_driving;
    int  clock_freq;
    int  partition;
    int  hopping_bit;
    int  hopping_time;
    int  result;
    int  sd30_mode;
    int  sd30_max_current;
    int  sd30_drive;
    int  sd30_power_control;
};

extern int update_raw_partition_from_file(const char *devname,  const char *file, long long offset, int size);
extern int update_ext4_partition_from_file(const char *devname,  const char *file, long long offset, int size);
extern bool is_file_exist(const char *file);
extern int factory_reset();
extern void recovery_init();
extern void reboot_system();;
void msdc_emmc_clear_write_protect(void);
void msdc_emmc_restore_write_protect(void);

#ifdef __cplusplus
}
#endif


#endif
