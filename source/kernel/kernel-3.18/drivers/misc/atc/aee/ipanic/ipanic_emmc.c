/*
* Copyright (c) 2016 AutoChips Inc.
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

#include <linux/mmc/sd_misc.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "ipanic.h"

#define EMMC_BLOCK_SIZE		0x200


#define EMMC_EXPDB_PART_SIZE    0xA00000

#define IPANIC_MAX_OFFSET		0

extern int card_dump_func_read(unsigned char *buf, unsigned int len, unsigned long long offset,
			       int dev);
extern int card_dump_func_write(unsigned char *buf, unsigned int len, unsigned long long offset,
				int dev);
extern unsigned int reset_boot_up_device(int type); /* force to re-initialize the emmc host controller */

char *ipanic_read_size(int off, int len)
{
	int size;
	char *buff = NULL;

	if (len == 0)
		return NULL;
	size = ALIGN(len, EMMC_BLOCK_SIZE);
	buff = kzalloc(size, GFP_KERNEL);
	if (buff == NULL) {
		LOGE("%s: cannot allocate buffer(len:%d)\n", __func__, len);
		return NULL;
	}
#ifdef CONFIG_MTD_NAND_ATC
	if (card_dump_func_read(buff, size, off, DUMP_INTO_EXTERN_CARD) != 0) {
#else
	if (card_dump_func_read(buff, size, off, DUMP_INTO_BOOT_CARD_IPANIC) != 0) {
#endif
		LOGE("%s: read failed(offset:%d,size:%d)\n", __func__, off, size);
		kfree(buff);
		return NULL;
	}

	return buff;
}
EXPORT_SYMBOL(ipanic_read_size);

int ipanic_write_size(void *buf, int off, int len)
{
	if (len & (EMMC_BLOCK_SIZE - 1))
		return -2;
	if (len > 0) {
#ifdef CONFIG_MTD_NAND_ATC
		if (card_dump_func_write((unsigned char *)buf, len, off, DUMP_INTO_EXTERN_CARD)) {
#else
		if (card_dump_func_write((unsigned char *)buf, len, off, DUMP_INTO_BOOT_CARD_IPANIC)) {
#endif
			return -1;
		}
	}
	return len;
}
EXPORT_SYMBOL(ipanic_write_size);

static int bufsize;
static u64 buf;
void ipanic_msdc_init(void)
{
	bufsize = ALIGN(PAGE_SIZE, EMMC_BLOCK_SIZE);
	buf = (u64)(unsigned long)kmalloc(bufsize, GFP_KERNEL);
}
EXPORT_SYMBOL(ipanic_msdc_init);

int ipanic_msdc_info(struct ipanic_header *iheader)
{
	iheader->blksize = EMMC_BLOCK_SIZE;
	iheader->partsize = EMMC_EXPDB_PART_SIZE;
	iheader->buf = buf;
	iheader->bufsize = bufsize;
	if (iheader->buf == 0) {
		LOGE("kmalloc fail[%x]\n", iheader->bufsize);
		iheader = NULL;
		return -1;
	}
	if (oops_in_progress)
		//reset_boot_up_device(0);
	return 0;
}
EXPORT_SYMBOL(ipanic_msdc_info);

void ipanic_erase(void)
{
	char *zero = kzalloc(PAGE_SIZE, GFP_KERNEL);
	ipanic_write_size(zero, 0, PAGE_SIZE);
	kfree(zero);
}
EXPORT_SYMBOL(ipanic_erase);

