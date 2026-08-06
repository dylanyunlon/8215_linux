#ifndef __MTKFTLCORE_H
#define __MTKFTLCORE_H

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/err.h>
#include <linux/stringify.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/blktrans.h>
#include <linux/mutex.h>
#include <linux/mtd/partitions.h>

#define ADD_TIMER 1
#define MTK_BLOCK_SIZE 512
#define MTK_BLOCK_SHIFT 9
#define VERSION "[MTKLib]Version 2012112101\r\n"
enum state{ STATE_EMPTY, STATE_CLEAN, STATE_DIRTY, STATE_NULL} ;
struct mtk_dev {
	struct mtd_blktrans_dev mbd;
	int count;
	unsigned int index;
	uint64_t size;
	struct mutex cache_mutex;
	unsigned char *cache_data;
	unsigned long cache_offset;
	unsigned int cache_size;
	unsigned int cachesize_shift;
	enum state cache_state;
	unsigned long dev_size;
#if ADD_TIMER
	struct timer_list mtklib_timer;
	struct task_struct *flush_thread;
	struct completion flush_sync;
	int flag;  //exit flag
	unsigned int timer_cnt;
#endif
};

#define MAX_PARAM_NAME_LEN 50
struct mtk_dev_param{
	char name[MAX_PARAM_NAME_LEN];
	int vid_hdr_offs;
};

#endif /*__MTKFTLCORE_H*/
