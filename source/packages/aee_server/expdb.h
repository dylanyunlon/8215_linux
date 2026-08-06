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
 
#ifndef _EXPDB_H_
#define _EXPDB_H_

typedef unsigned char __u8;
typedef unsigned int __u32;
typedef unsigned long long __u64;
typedef __u8 u8;
typedef __u32 u32;
typedef __u64 u64;

#define AEE_IPANIC_MAGIC            0xaee0dead
#define AEE_IPANIC_PHDR_VERSION     0x04
#define IPANIC_NR_SECTIONS          32

#define EXPDB_ALLOCATE              (3 * 1024 * 1024)
#define EXPDB_BIN                   ((3 * 1024 * 1024) - 2088)
#define MIN_STORAGE_SIZE            100 // 100M

#ifdef CONFIG_ATC_NAND
#define EXPDB_DEV           "/dev/mmcblk0"
#else
#define EXPDB_DEV           "/dev/expdb"
#endif
struct ipanic_data_header {
    u32 type;	/* data type(0-31) */
    u32 valid;	/* set to 1 when dump succeded */
    u32 offset;	/* offset in EXPDB partition */
    u32 used;	/* valid data size */
    u32 total;	/* allocated partition size */
    u32 encrypt;	/* data encrypted */
    u32 raw;	/* raw data or plain text */
    u32 compact;	/* data and header in same block, to save space */
    u8 name[32];
};

struct ipanic_header {
    u32 magic;
    u32 version;	/* ipanic version */
    u32 size;	/* ipanic_header size */
    u32 datas;	/* bitmap of data sections dumped */
    u32 dhblk;	/* data header blk size, 0 if no dup data headers */
    u32 blksize;	/* size per block */
    u32 partsize;	/* expdb partition totoal size */
    u32 bufsize;
    u64 buf;
    struct ipanic_data_header data_hdr[IPANIC_NR_SECTIONS];
};

#define IPANIC_CURRENT_TSK_NAME "PROC_CUR_TSK"
typedef enum {
    IPANIC_DT_HEADER = 0,
    IPANIC_DT_RESERVED31 = 31,
} IPANIC_DT;

#define AEE_NR_FRAME 32
#define AEE_SZ_SYMBOL_L 140
#define AEE_SZ_SYMBOL_S 80
/* aee_bt_frame size is 256 byte now, and the full backtrace should be 256x32 = 8KB */
struct aee_bt_frame{
    __u64 pc;
    __u64 lr;
    __u32 pad[5];
    char pc_symbol[AEE_SZ_SYMBOL_S];	/* Now we use different symbol length for PC &LR */
    char lr_symbol[AEE_SZ_SYMBOL_L];
};

#define AEE_PROCESS_NAME_LENGTH 256
#define AEE_BACKTRACE_LENGTH 3072
/* ipanic_oops_header struct should strictly small than ipanic_buffer, now 4KB */
struct aee_process_info
{
	char process_path[AEE_PROCESS_NAME_LENGTH];
	char backtrace[AEE_BACKTRACE_LENGTH];
	struct aee_bt_frame ke_frame;
};

typedef enum {
    AEE_FIQ_STEP_FIQ_ISR_BASE = 1,
    AEE_FIQ_STEP_WDT_FIQ_INFO = 4,
    AEE_FIQ_STEP_WDT_FIQ_STACK,
    AEE_FIQ_STEP_WDT_FIQ_LOOP,
    AEE_FIQ_STEP_WDT_FIQ_DONE,
    AEE_FIQ_STEP_WDT_IRQ_INFO = 8,
    AEE_FIQ_STEP_WDT_IRQ_KICK,
    AEE_FIQ_STEP_WDT_IRQ_SMP_STOP,
    AEE_FIQ_STEP_WDT_IRQ_STACK,
    AEE_FIQ_STEP_WDT_IRQ_TIME,
    AEE_FIQ_STEP_WDT_IRQ_GIC,
    AEE_FIQ_STEP_WDT_IRQ_LOCALTIMER,
    AEE_FIQ_STEP_WDT_IRQ_IDLE,
    AEE_FIQ_STEP_WDT_IRQ_SCHED,
    AEE_FIQ_STEP_WDT_IRQ_DONE,
    AEE_FIQ_STEP_KE_WDT_INFO = 20,
    AEE_FIQ_STEP_KE_WDT_PERCPU,
    AEE_FIQ_STEP_KE_WDT_LOG,
    AEE_FIQ_STEP_KE_SCHED_DEBUG,
    AEE_FIQ_STEP_KE_WDT_DONE,
    AEE_FIQ_STEP_KE_IPANIC_START = 32,
    AEE_FIQ_STEP_KE_IPANIC_OOP_HEADER,
    AEE_FIQ_STEP_KE_IPANIC_DETAIL,
    AEE_FIQ_STEP_KE_IPANIC_CONSOLE,
    AEE_FIQ_STEP_KE_IPANIC_USERSPACE,
    AEE_FIQ_STEP_KE_IPANIC_ANDROID,
    AEE_FIQ_STEP_KE_IPANIC_MMPROFILE,
    AEE_FIQ_STEP_KE_IPANIC_HEADER,
    AEE_FIQ_STEP_KE_IPANIC_DONE = 40,
    AEE_FIQ_STEP_KE_NESTED_PANIC = 64,
} AEE_FIQ_STEP_NUM;

struct ipanic_part_info {
    //int fd;
    int size;
    int blksize;
    int (*erase)(struct ipanic_part_info *partinfo);
};

struct mtd_info_user {
    __u8 type;
    __u32 flags;
    __u32 size;	 // Total size of the MTD
    __u32 erasesize;
    __u32 writesize;
    __u32 oobsize;   // Amount of OOB data per block (e.g. 16)
    /* The below two fields are obsolete and broken, do not use them
    * (TODO: remove at some point) */
    __u32 ecctype;
    __u32 eccsize;
};
struct erase_info_user {
    __u32 start;
    __u32 length;
};
#define MTD_MEMERASE		_IOW('M', 2, struct erase_info_user)
#define MTD_MEMGETINFO		_IOR('M', 1, struct mtd_info_user)


#endif /* _EXPDB_H_ */
