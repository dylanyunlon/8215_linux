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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <asm/page.h>
#include <asm/io.h>
#include <linux/pm.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include "metazone_inter.h"
#include "metazone_ioctl.h"
#include "metazone.h"

#define CMD_BUFF_SIZE  (256)
#define CMD_MAX_SIZE   (20)
#define DW_TYPE        (5)
#define BIN_TYPE       (6)
#define CMD_SIZE       (12)

#define BINARY_DATA_SIZE 100

unsigned int dwbuff;
char binarybuff[BINARY_DATA_SIZE];
unsigned int binarysize = 0;
int mtztype;
int readwrite_dword(char *cmd, int index, unsigned int dwvalue)
{
	/* Dwrod write*/
	if (0 == strncmp(cmd, "dword_write", CMD_SIZE)) {
		if (MZ_FAILURE == MetaZone_Write(index, dwvalue)) {
			pr_err("[MTZ] Dwrod write fail by mtz_debug \n");
			return -1;
		}
		dwbuff = dwvalue;

		if (MetaZone_Flush(1) == -1) {
			pr_err("[MTZ] metazone flush fail by mtz_debug \n");
			return -1;
		}
		mtztype = 1;
		return 0;
	}

	/* Dwrod read*/
	if (0 == strncmp(cmd, "dword_read", CMD_SIZE)) {
		if (MZ_FAILURE == MetaZone_Read(index, &dwbuff)) {
			pr_err("[MTZ] metazone write fail by mtz_debug \n");
			return -1;
		}
		mtztype = 1;
		return 0;
	}

	/* unkwon info*/
	pr_err ("[MTZ] input info is error!\n");
	return -1;

}

int readwrite_binary(char *cmd, int index, char *pbdata, unsigned int binarysize)
{
	/*Binary write*/
	if (0 == strncmp(cmd, "binary_write", CMD_SIZE)) {
		if (MZ_FAILURE == MetaZone_WriteBinary(index, pbdata, binarysize)) {
			pr_err("[MTZ] Binary write binary fail by mtz_debug \n");
			return -1;
		}
		strcpy(binarybuff, pbdata);
		if (MetaZone_Flush(1) == -1) {
			pr_err("[MTZ] metazone flush fail by mtz_debug \n");
			return -1;
		}
		mtztype = 2;
		return 0;
	}

	/*Binary read*/
	if (0 == strncmp(cmd, "binary_read", CMD_SIZE)) {
		if (MZ_FAILURE == MetaZone_ReadBinary(index, binarybuff, binarysize)) {
			pr_err("[MTZ] metazone read fail by mtz_debug \n");
			return -1;
		}
		mtztype = 2;
		return 0;
	}
	/* unkwon info*/
	pr_err ("[MTZ] input info is error!");
	return -1;
}

static ssize_t proc_mtz_debug_write(struct file *file, const char *buf, size_t count, loff_t *data)
{
	unsigned int index = 0;
	unsigned int dwvalue = 0;
	unsigned long num;
	unsigned int debugsize = 8;
	char cmd_buf[CMD_BUFF_SIZE];
	char cmd[CMD_MAX_SIZE];

	char pbdata[BINARY_DATA_SIZE];
	char type[32];

	if (count == 0) {
		pr_err("[MTZ] count 0\n");
		return count;
	}

	if (count > CMD_BUFF_SIZE - 1) {
		count = CMD_BUFF_SIZE - 1;
	}

	num = copy_from_user(cmd_buf, buf, count);
	if (num > 0) {
		pr_err("[MTZ] copy_from_user fail \n");
	}

	cmd_buf[count] = '\0';
	pr_info("[MTZ] Metazone debug info: %s \n", cmd_buf);

	if (sscanf(cmd_buf, "%31s", type) != 1){
		pr_info("[MTZ] Metazone get type error!\n");
		return -EINVAL;;
	}
	pr_info("[MTZ] Metazone type info: %s \n", type);

	/* reset global buffer and type*/
	mtztype = 0;
	dwbuff = 0;
	memset(binarybuff, 0, BINARY_DATA_SIZE);

	if (0 == strncmp(type, "dword", DW_TYPE)) {
		if (strstr(type, "read")) {
			if (sscanf(cmd_buf, "%19s %x", cmd, &index) != 2) {
				pr_err("[MTZ] dword_read parse fail\n");
				return -EINVAL;
			}
			if (readwrite_dword(cmd, index, dwvalue) != 0) {
				return -EINVAL;
			}
		}
		if (strstr(type, "write")) {
			if (sscanf(cmd_buf, "%19s %x %d", cmd, &index, &dwvalue) != 3) {
				pr_err("[MTZ] dword_write parse fail\n");
				return -EINVAL;
			}
			if (readwrite_dword(cmd, index, dwvalue) != 0) {
				return -EINVAL;
			}
		}
	} else if (0 == strncmp(type, "binary", BIN_TYPE)) {
		if (strstr(type, "read")) {
			if (sscanf(cmd_buf, "%19s %x %d", cmd, &index, &binarysize) != 3) {
				pr_err("[MTZ] binary_read parse fail\n");
				return -EINVAL;
			}
			if (readwrite_binary(cmd, index, pbdata, binarysize) != 0){
				return -EINVAL;
			}
		}
		if (strstr(type, "write")) {
			if (sscanf(cmd_buf, "%19s %x %d %99s", cmd, &index, &binarysize, pbdata) != 4) {
				pr_err("[MTZ] binary_write parse fail\n");
				return -EINVAL;
			}
			if (readwrite_binary(cmd, index, pbdata, binarysize) != 0){
				return -EINVAL;
			}
		}
	} else {
		pr_err("[MTZ] input info is error,please check\n");
		return -EINVAL;
	}
	return count;
}

static int mtz_debug_proc_show(struct seq_file *m, void *v)
{
	int i;
	char pbuf[3];
	char buffhex[BINARY_DATA_SIZE*2];
	memset(buffhex, 0, sizeof(buffhex));
	if (mtztype == 1) {
		seq_printf(m, "[MTZ] Dword data :%d \n", dwbuff);
		return 0;
	}
	if (mtztype == 2) {
		seq_printf(m, "[MTZ] Binary data(string):%s \n", binarybuff);
		seq_printf(m, "[MTZ] Binary data(hex) :");
		for (i=0; i < binarysize; i++ ) {
			sprintf(pbuf, "%02X", binarybuff[i]);
			seq_printf(m, "%02X ", binarybuff[i]);
		}
		seq_printf(m, "\n");
		return 0;
	}


	return -EINVAL;

}

static int proc_mtz_debug_open (struct inode *inode, struct file *file)
{
	return single_open(file, mtz_debug_proc_show, NULL);
}


static const struct file_operations proc_mtz_debug_fops = {
	.open = proc_mtz_debug_open,
	.read = seq_read,
	.write = proc_mtz_debug_write,
	.llseek = seq_lseek,
	.release = single_release,
};

int mtz_proc_init (void)
{
	struct proc_dir_entry *proc_mtz_debug = NULL;

	proc_mtz_debug = proc_create("mtz_debug", 0660, NULL, &proc_mtz_debug_fops);
	if (!proc_mtz_debug) {
		pr_err("[MTZ]failed to create proc/mtz_debug \n");
		return -EINVAL;
	}

	return 0;
}
