/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include "err_num.h"

#include <pthread.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include "atc_emptycore_upgrade.h"
#include "atc_update.h"
#include "recovery.h"
#include "blkwd.h"
#include "checksum.h"
#include "atc_safe_upgrade.h"
#include "atc_upgrade_common.h"
#include "nandupgrade.h"

static struct partition_upgrade_status_t g_upgrade_status_self;
static struct partition_upgrade_status_t g_upgrade_status_to_qt;

static char g_cmdline[CMDLINE_LENGTH];

#define KERN_BUILD_TIME_LEN 50
static char g_kernel_build_time[KERN_BUILD_TIME_LEN];

#define BD_DEV_NODE_LEN 24
static char g_bd_dev_node[BD_DEV_NODE_LEN];

static pthread_t g_upgrade_thread_tid;
static pthread_mutex_t g_upgrade_mutex;

static struct wd_name wd={-1, -1, "/dev/"};

static pthread_mutex_t g_hotplug_mutex;
static int g_hotplug_status = 0;
/* metazone partition start address offset from emmc start */
static u64 metazone_pt_addr = 0;
static u64 metazone_pt_size = 0;
static u64 metazone_real_size = 0;
static partitionread *pxml_tbl;
static u32 from_udisk;
static u32 from_udisk_monitor;
static int verify_image_flag = 0;

#ifdef CONFIG_BOOT_MMC
static int erase_emmc(void)
{
	int fdwp, ret = 0;
	unsigned long long emmc_size = 0;

	ret = get_emmc_total_size(&emmc_size);
	if (ret < 0) {
		rec_err("get emmc total size fail.\n");
		return -EEMMCSIZE;
	}

	fdwp = open_for_writeprotect();
	if (fdwp < 0) {
		rec_err("open_for_writeprotect fail\n");
		return -ESYSCALL;
	}
	
	rec_info("before upgrade auto dump info:\n");
	dump_writeprotect_region(fdwp);
	ret = clear_writeprotect(fdwp);
	if (ret < 0) {
		rec_err("clear_writeprotect fail.\n");
		close_for_writeprotect(fdwp);
		return -ESYSCALL;
	}

	struct msdc_ioctl arg1;
	arg1.host_num = 0;
	arg1.address = 0;
	arg1.total_size = emmc_size / 512;
	arg1.opcode = MSDC_ERASE_SELECTED_AREA;
	if (ioctl(fdwp, MSDC_ERASE_SELECTED_AREA, &arg1)){
		rec_err("erase emmc failed\n");
		close_for_writeprotect(fdwp);
		return -ESYSCALL;
	}
	rec_info("erase emmc succes\n");
	close_for_writeprotect(fdwp);

	return 0;
}

#else /* !CONFIG_BOOT_MMC */
#if 0
static int erase_nand(void)
{
	int ret = -1;
	struct nand_dev_info nand_info;

	rec_info("erase nand begin.\n");

	memset(&nand_info, 0, sizeof(struct nand_dev_info));
	ret = get_nand_info(&nand_info);
	if (ret < 0) {
		rec_err("get_nand_info fail.\n");
		return -ENANDINFO;
	}
	ret = nand_rw_start((nand_info.block_cnt) * (nand_info.block_size));
	if (ret < 0) {
		rec_err("nand_rw_start fail.\n");
		return -ENANDRW;
	}
	rec_info("erase nand start begin.\n");

	ret = nand_all_erase();
	if (ret < 0) {
		rec_err("nand_erase fail.\n");
		return -ENANDERASE;
	}

	rec_info("erase nand end begin.\n");
	ret = nand_rw_end();
	if (ret < 0) {
		rec_err("nand_rw_end fail.\n");
		return -ENANDRW;
	}
	rec_info("erase nand success.\n");

	return 0;
}
#endif

#endif /* CONFIG_BOOT_MMC */

/*
* return -1 if init fail
*return 0 if init success
*/
int  hotplug_monitor_init(bool from_udisk)
{
	int fd = 0;
	int ret = -1;
	int i = 0;
	char devnode[16] = {0};

	if (from_udisk) {
		fd = open("/dev/sda", O_RDONLY, 0);
		if (fd > 0) {
			close(fd);
			g_hotplug_status = 1;
			hotplug_device_tune(1, "/dev/sda");
		}

		for (i = 1; i < 10; i++) {
			snprintf(devnode, 16, "/dev/sda%d", i);
			fd = open(devnode, O_RDONLY, 0);
			if (fd > 0) {
				close(fd);
				g_hotplug_status = 1;
				hotplug_device_tune(1, devnode);
			}
		}
	} else {
	#ifdef CONFIG_BOOT_MMC
		fd = open("/dev/mmcblk1p1", O_RDONLY, 0);
		if (fd > 0) {
			close(fd);
			g_hotplug_status = 1;
		}
		fd = open("/dev/mmcblk1", O_RDONLY, 0);
		if (fd > 0) {
			close(fd);
			g_hotplug_status = 1;
		}
	#else
		fd = open("/dev/mmcblk0p1", O_RDONLY, 0);
		if (fd > 0) {
			close(fd);
			g_hotplug_status = 1;
		}
		fd = open("/dev/mmcblk0", O_RDONLY, 0);
		if (fd > 0) {
			close(fd);
			g_hotplug_status = 1;
		}
	#endif
	}

	if ((ret = init_blkwd(&wd)) < 0) {
		rec_err("init_blkwd fail.\n");
		return ret;
	} else {
		rec_info("init_blkwd success.\n");
		return 0;
	}
}

int  export_hotplug_monitor_init(void)
{
#if 0
	int fd = 0;
	int ret = -1;
	int i = 0;
	char devnode[16] = {0};
	rec_info("[qydebug]1\n");
	
	if (from_udisk) {
		fd = open("/dev/sda", O_RDONLY, 0);
		if (fd > 0) {
			close(fd);
			g_hotplug_status = 1;
			hotplug_device_tune(1, "/dev/sda");
		}

		for (i = 1; i < 10; i++) {
			snprintf(devnode, 16, "/dev/sda%d", i);
			fd = open(devnode, O_RDONLY, 0);
			if (fd > 0) {
				close(fd);
				g_hotplug_status = 1;
				hotplug_device_tune(1, devnode);
			}
		}
	} else {
		fd = open("/dev/mmcblk1p1", O_RDONLY, 0);
		if (fd > 0) {
			close(fd);
			g_hotplug_status = 1;
		}
		fd = open("/dev/mmcblk1", O_RDONLY, 0);
		if (fd > 0) {
			close(fd);
			g_hotplug_status = 1;
		}
	}

	if ((ret = init_blkwd(&wd)) < 0) {
		rec_err("init_blkwd fail.\n");
		return ret;
	} else {
		rec_info("init_blkwd success.\n");
		return 0;
	}
#endif
	return 0;
}

/**
* return 0 if plug-out
* return 1 if plug-in
*/
int export_get_hotplug_status(void)
{
	int ret;
	if (!from_udisk_monitor) {
		rec_info("Not init monitor,return .\n");
		return 1;
	}
	ret = blkwd_event(&wd);
	pthread_mutex_lock(&g_hotplug_mutex);
	if (from_udisk) {
		if (ret == UDISKPLUGOUT) {
			g_hotplug_status = 0;
		} else if (ret == UDISKPLUGIN) {
			g_hotplug_status = 1;
		}
	} else {
		if (ret == SDCARDPLUGOUT) {
			g_hotplug_status = 0;
		} else if (ret == SDCARDPLUGIN) {
			g_hotplug_status = 1;
		}
	}
	pthread_mutex_unlock(&g_hotplug_mutex);
	return g_hotplug_status;
}

int export_put_hotplug_status(void)
{
	pthread_mutex_lock(&g_hotplug_mutex);
	g_hotplug_status = 0;
	pthread_mutex_unlock(&g_hotplug_mutex);
	destory_blkwd(&wd);

	return 0;
}

struct partition_upgrade_status_t *export_get_upgrade_status(void)
{
	if (pthread_mutex_trylock(&g_upgrade_mutex) == 0) {
/*
* pthread_mutex_trylock will NOT block qt.
* if return value is 0, take this opportunity to copy upgrade status
* from g_upgrade_status_self to g_upgrade_status_to_qt.
*/
		memcpy(&g_upgrade_status_to_qt, &g_upgrade_status_self, sizeof(struct partition_upgrade_status_t));
		pthread_mutex_unlock(&g_upgrade_mutex);
	}

	if (g_upgrade_status_to_qt.total_size_need_upgrade != 0)
		g_upgrade_status_to_qt.progress_percent = g_upgrade_status_to_qt.size_upgrade_done * 100 / g_upgrade_status_to_qt.total_size_need_upgrade;

	return &g_upgrade_status_to_qt;
}

/*
* this function is called by qt to get which partition is ongoing in upgrade.
*/
char *export_get_part_upgrade_ongoing(void)
{
	if (pthread_mutex_trylock(&g_upgrade_mutex) == 0) {
/*
* pthread_mutex_trylock will NOT block qt.
* if return value is 0, take this opportunity to copy upgrade status
* from g_upgrade_status_self to g_upgrade_status_to_qt.
*/
		memcpy(&g_upgrade_status_to_qt, &g_upgrade_status_self, sizeof(struct partition_upgrade_status_t));
		pthread_mutex_unlock(&g_upgrade_mutex);
	}

	return g_upgrade_status_to_qt.upgrade_ongoing_part_name;
}

int export_get_upgrade_thread_status(void)
{
	return g_upgrade_status_self.upgrade_finish;
}

int export_get_upgrade_progress(void)
{
	if (pthread_mutex_trylock(&g_upgrade_mutex) == 0) {
/*
* pthread_mutex_trylock will NOT block qt.
* if return value is 0, take this opportunity to copy upgrade status
* from g_upgrade_status_self to g_upgrade_status_to_qt.
*/
		memcpy(&g_upgrade_status_to_qt, &g_upgrade_status_self, sizeof(struct partition_upgrade_status_t));
		pthread_mutex_unlock(&g_upgrade_mutex);
	}
	rec_dbg("pregress = 0x%llx.\n",g_upgrade_status_to_qt.total_size_need_upgrade);
	if (g_upgrade_status_to_qt.total_size_need_upgrade == 0)
		return 0;

	g_upgrade_status_to_qt.progress_percent = g_upgrade_status_to_qt.size_upgrade_done * 100 / g_upgrade_status_to_qt.total_size_need_upgrade;
	rec_info("g_upgrade_status_to_qt.progress_percent=%d.\n",g_upgrade_status_to_qt.progress_percent);
	return g_upgrade_status_to_qt.progress_percent;
}

static int parse_kernel_version(char *version)
{
	char *version_str = NULL;
	long long version_sn = 0;
	int idx = 0;

	if (version == NULL)
		return -1;

	version_str = strstr(version, "PREEMPT ");
	if (version_str == NULL) {
		rec_warn("version does NOT include PREEMPT.\n");
		return -1;
	}

	idx = 0;
	while ((version_str[idx + 8] != '\0' || version_str[idx + 8] != '\n')
			&& (idx < (KERN_BUILD_TIME_LEN - 1))) {
		g_kernel_build_time[idx] = version_str[idx + 8];
		idx++;
	}
	g_kernel_build_time[idx] = '\0';

	rec_info("kernel build time is (%s) idx (%d)\n", g_kernel_build_time, idx);

	return 0;

}

char *export_get_kernel_version(void)
{
	int fd = 0;
	int ret = 0;
	char version[512];

	memset(version, 0, 512);

	fd = open("/proc/version", O_RDONLY);
	if (fd <= 0) {
		rec_err("open /proc/version fail,%s.\n", strerror(errno));
		return NULL;
	}

	ret = read(fd, version, 512);
	if (ret <= 0) {
		rec_err("read /proc/version fail,%s.\n", strerror(errno));
		close(fd);
		return NULL;
	} else {
		rec_info("read /proc/version ret=%d\n", ret);
	}

	version[511] = 0;
	close(fd);

	rec_info("version: (%s)\n", version);
	if (parse_kernel_version(version) < 0)
		return NULL;

	return g_kernel_build_time;
}

/**
* get_cmdline from /proc/cmdline
*
* return 0 if get successfully, otherwise -1.
*/
static int get_cmdline(void)
{
	int fd;
	int ret;

	fd = open("/proc/cmdline", O_RDONLY);
	if (fd <= 0) {
		rec_err("open /proc/cmdline fail,%s.\n", strerror(errno));
		return -1;
	}

	ret = read(fd, g_cmdline, CMDLINE_LENGTH);
	if (ret <= 0) {
		rec_err("read /proc/cmdline fail,%s.\n", strerror(errno));
		close(fd);
		return -1;
	} else {
		rec_info("read /proc/cmdline ret=%d\n", ret);
	}

	g_cmdline[CMDLINE_LENGTH - 1] = 0;

	rec_info("cmdline: %s\n", g_cmdline);
	close(fd);

	return 0;
}

typedef struct upgrade_header {
	char szSignature[4];
	unsigned int bFormatFlash;
	unsigned int bEraseemmc;
	unsigned int bVerfyImage;
	unsigned int bModifyPartition;
	unsigned int bAdvanceMode;
	unsigned int nSegmentSize;
	unsigned long long u8UserdataPartitionAddress;
	unsigned int nWriteproSize;
	unsigned int u4Reserve[512 - 28];
} upgrade_header_t;

static int get_verify_image_flag(int fd, u64 img_start_addr)
{
	upgrade_header_t upg_hdr;
	off64_t cur_offset = 0;
	long long ret = 0;

	memset(&upg_hdr, 0, sizeof(upgrade_header_t));

	/* image in ext sdcard, lseek it first */
	cur_offset = lseek64(fd, img_start_addr, SEEK_SET);
	if (cur_offset != img_start_addr) {
		rec_err("lseek64 failed!\n");
		return -ESYSCALL;
	}

	ret = read(fd, &upg_hdr, sizeof(upgrade_header_t));
	if (ret != sizeof(upgrade_header_t)) {
		rec_err("read sdcard failed!\n");
		return -ESYSCALL;
	}

	/*verify the signature*/
	if (strcmp(upg_hdr.szSignature, "UPG")) {
		rec_err("verify the signature of upg header fail.\n");
		rec_err("Signature[0]=%d\n", upg_hdr.szSignature[0]);
		rec_err("Signature[1]=%d\n", upg_hdr.szSignature[1]);
		rec_err("Signature[2]=%d\n", upg_hdr.szSignature[2]);
		rec_err("Signature[3]=%d\n", upg_hdr.szSignature[3]);
		return -EUPGHDR;
	}

	if (upg_hdr.bVerfyImage)
		verify_image_flag = 1;

	rec_info("verify_image_flag=%d\n", verify_image_flag);
	return 0;
}

static int parse_mac_start_addr(char *cmdline,
	    unsigned long long *pmac_addr)
{
#define MAC_START_ADDR_LEN 20

	int idx;
	unsigned long long addr;
	char *mac_str;
	char mac_start_addr[MAC_START_ADDR_LEN];

	if (cmdline == NULL)
		return -1;

	mac_str = strstr(cmdline, "mac_start_addr=");
	if (mac_str == NULL) {
		rec_warn("cmdline does NOT include mac_start_addr.\n");
		return -1;
	}

/*
* mac_start_addr=0x12345678
* copy 0x12345678 into mac_start_addr[], example only
*
* 15 is length of mac_start_addr=
*/
	idx = 0;
	while ((mac_str[idx + 15] != ' ') && (mac_str[idx + 15] != '\0') && (idx < (MAC_START_ADDR_LEN - 1))) {
		mac_start_addr[idx] = mac_str[idx + 15];
		idx++;
	}
	mac_start_addr[idx] = '\0';

	rec_info("mac_start_addr is %s\n", mac_start_addr);
	addr = strtoll(mac_start_addr, NULL, 0);

	rec_info("mac_start_addr is 0x%llx\n", addr);

	*pmac_addr = addr;
	return 0;

}

static int parse_bootdevice(char *cmdline, int *pbd)
{
	char *bd_str;
	char bd_sn; //boot deivce serial number

	if (cmdline == NULL)
		return -1;

	bd_str = strstr(cmdline, "boot_device=");
	if (bd_str == NULL) {
		rec_warn("cmdline does NOT include bootdevice.\n");
		return -1;
	}

	bd_sn = bd_str[12]; //12 is strlen("boot_device=")
	if ((bd_sn == '0') || (bd_sn == '1') || (bd_sn == '2')) {
		*pbd = bd_sn - '0';
		return 0;
	} else {
		return -1;
	}
}

static int parse_image_start_addr(char *cmdline,
	    unsigned long long *pimg_addr)
{
	#define IMG_START_ADDR_LEN 20

	int idx;
	long long addr;
	char *img_str;
	char img_start_addr[IMG_START_ADDR_LEN];

	if (cmdline == NULL)
		return -1;

	img_str = strstr(cmdline, "image_start_addr=");
	if (img_str == NULL) {
		rec_warn("cmdline does NOT include image_start_addr.\n");
		return -1;
	}

/*
* image_start_addr=0x12345678
* copy 0x12345678 into img_start_addr[], example only
*
* 17 is length of image_start_addr=
*/
	idx = 0;
	while ((img_str[idx + 17] != ' ') && (img_str[idx + 17] != '\0') && (idx < (IMG_START_ADDR_LEN - 1))) {
		img_start_addr[idx] = img_str[idx + 17];
		idx++;
	}
	img_start_addr[idx] = '\0';

	rec_info("image_start_addr is %s\n", img_start_addr);
	addr = strtoll(img_start_addr, NULL, 0);

	rec_info("image_start_addr is 0x%llx\n", addr);

	*pimg_addr = addr;
	return 0;
}

static partitionread *mergepartitioninfo(partitioninfo *part)
{
	partitionread *ppartition = NULL;

	if (part == NULL)
	return NULL;

	ppartition = (partitionread *)malloc(sizeof(partitionread));
	if (ppartition == NULL) {
		rec_err("malloc failed\n");
		return NULL;
	}
	memset(ppartition, 0, sizeof(partitionread));

	//printf("mergepartitioninfo szPartName:%s\r\n",part->szPartName);
	strcpy(ppartition->szPartName, part->szPartName);
	strcpy(ppartition->szType, part->szType);
	strcpy(ppartition->szImageFileName, part->szImageFileName);

#if 1 //hack
	if (strcmp(ppartition->szPartName, "usrdata") == 0)
		strcpy(ppartition->szImageFileName, "usrdata.img.ext4");
#endif

	ppartition->u4Mount = part->u4Mount;
	ppartition->u8PartitionSize = part->u8PartitionSize;
	ppartition->u8RealDataSize = part->u8RealDataSize;
	ppartition->u8PartitionStartAddr = part->u8PartitionStartAddr;
#ifdef NEW_PARTITION_DESIGN
	ppartition->u4Flag= part->u4Flag;
#endif
	return ppartition;
}

static unsigned int u64_to_u32(unsigned long long u64,
	    unsigned int *uhigh, unsigned int *ulow)
{
	*ulow = (unsigned int)u64;
	*uhigh = (unsigned int)(u64 >> 32);

	return 0;
}

static void print_partitioninfo(partitioninfo *part)
{
	unsigned int vallow,valhigh;

	rec_info("----------------------------------------------------------------\r\n");
	rec_info("partition name:%s\r\n", part->szPartName);
	rec_info("partition type:%s\r\n", part->szType);
	u64_to_u32(part->u8PartitionStartAddr,&valhigh,&vallow);
	rec_info("partition Start address:0x%X%08X\r\n",valhigh,vallow);
	//printf("partition Start address:%lld\r\n",part->u8PartitionStartAddr);
	//printf("partition Start address:%llu\r\n",part->u8PartitionStartAddr);
	//printf("partition Start address:0x%llX\r\n",part->u8PartitionStartAddr);
	//printf("partition Start address:0x%X\r\n",part->u8PartitionStartAddr);
	//printf("partition Start address:0x%X%08X\r\n",part->u8PartitionStartAddr,(unsigned int)(part->u8PartitionStartAddr));
	//printf("partition Start address hh:0x%16X\r\n",(part->u8PartitionStartAddr));
	u64_to_u32(part->u8PartitionSize, &valhigh, &vallow);
	rec_info("partition Size:0x%X%08X\r\n", valhigh, vallow);
#ifdef NEW_PARTITION_DESIGN
	rec_info("Partition Image Name: %s\r\n", part->szImageFileName);
#endif
	//printf("offset data:0x%X\r\n",part->u4OffsetData);
	u64_to_u32(part->u8OffsetData, &valhigh, &vallow);
	rec_info("offset data:0x%X%08X\r\n", valhigh,vallow);
	//printf("Real Data size:0x%X\r\n",part->u4RealDataSize);
	//printf("next image offset:0x%X\r\n",part->u4OffsetNextImage);
	u64_to_u32(part->u8RealDataSize, &valhigh, &vallow);
	rec_info("Real Data size:0x%X%08X\r\n", valhigh,vallow);
#ifdef NEW_PARTITION_DESIGN
	rec_info("u4Flag:0x%X\r\n", part->u4Flag);
#endif
	u64_to_u32(part->u8OffsetNextImage, &valhigh, &vallow);
	rec_info("next image offset:0x%X%08X\r\n", valhigh, vallow);
	rec_info("----------------------------------------------------------------\r\n");

	return;
}

static int emptycore_get_file_length(const char *file,
	    unsigned long long *plen)
{
	char file_path[IMG_FULL_NAME_MAX] = {0};
	int ret = 0;
	unsigned long long len = 0;

	strcpy(file_path, ISO_ROOT);
	strcat(file_path, file);

	ret = get_file_len(file_path, &len);

	if (ret < 0) {
		rec_err("get file(%s) length fail.", file_path);
		ret = -EFILELEN;
	}

	*plen = len;
	return 0;
}


#ifdef CONFIG_BOOT_MMC
/**
* fd: ext sdcard fd
* devname: emmc device node name
* img_offset: image address in ext sdcard
* offset: image address in emmc
*/
static int emptycore_upg_raw_partition_from_sdcard(int fd, const char *devname,
	    long long img_offset, long long offset,
	    long long realdata_size, long long *psize_total)
{
	int fdev;
	long long sizer, sizew, sizetotal;
	char *buffer = NULL;
	off64_t cur_offset;
	int ret = -1;

	if (realdata_size <= 0) {
		rec_warn("realdata_size is 0.\n");
		*psize_total = 0;
		return 0;
	}

	//image in ext sdcard, lseek it first
	cur_offset = lseek64(fd, img_offset, SEEK_SET);
	if (cur_offset != img_offset) {
		rec_err("lseek64 failed-1!\n");
		return -ESYSCALL;
	}

	//open emmc device node to write
	fdev = open(devname, O_RDWR | O_LARGEFILE);
	if (fdev < 0) {
		rec_err("open %s fail.\n", devname);
		return -ESYSCALL;
	}

	buffer = (char *)malloc(FILE_RW_SIZE);
	if (!buffer) {
		rec_err("malloc fail.\n");
		close(fdev);
		return -ENOMEM;
	}
	//partiotion position in emmc
	cur_offset = lseek64(fdev, offset, SEEK_SET);
	if (cur_offset != offset) {
		rec_err("lseek64 failed-2!\n");
		close(fdev);
		free(buffer);
		return -ESYSCALL;
	}

	sizetotal = 0;
	while (realdata_size > 0) {
		if (realdata_size >= FILE_RW_SIZE) {
			sizer = read(fd, buffer, FILE_RW_SIZE);
		} else {
			sizer = read(fd, buffer, realdata_size);
		}
		sizew = write(fdev, buffer, sizer);
		sizetotal += sizew;
		realdata_size -= sizer;
		if (sizew < sizer)
			break;
	}

	close(fdev);
	free(buffer);

	if (sizetotal < realdata_size) {
		rec_err("sizetotal(0x%llx) < realdata_size(0x%llx)\n", sizetotal, realdata_size);
		return -ESYSCALL;
	}

	*psize_total = sizetotal;

	rec_info("succeeded, sizetotal(0x%llx)!\n", sizetotal);

	return 0;
}

static int emptycore_upg_ext4_partition_from_sdcard(int fd, const char *devname,
        long long img_offset, long long realdata_size,
        long long offset, long long size)
{
    int fdev;
    long long sizer, sizew, filelen;
    uchar *buffer = NULL;
    uint32_t chunk_cnt = 0;
    uint32_t block_size = 0;
    uchar *pfileBuffer = NULL;
    long long memlen = DEF_CHUNK_SIZE;
    long long datalen = 0;
    off64_t cur_offset;
    int ret=-1;
    sparse_header_t tSparseHeader;
    chunk_header_t  tChunkHeader;

    rec_info("dev(%s) offset(0x%llx) img_offset(0x%llx)\n", devname, offset, img_offset);

    if (realdata_size <= 0) {
        rec_warn("realdata_size is 0.\n");
        return 0;
    }

    //open emmc device node to write
    fdev = open(devname, O_RDWR | O_LARGEFILE);
    if (fdev < 0) {
        return -ESYSCALL;
    }

    // TODO, double cofirm here??
    // realdata_size ?=  lseek(fd, 0, SEEK_END)
    //filelen = lseek(fd, 0, SEEK_END);
    filelen = realdata_size ;

    buffer = (uchar *)malloc(memlen);
    if (!buffer){
        close(fdev);
        return -ENOMEM;
    }

    // Read file to buffer
    //image in ext sdcard, lseek it first
    cur_offset = lseek64(fd, img_offset, SEEK_SET);
    if (cur_offset != img_offset) {
        rec_err("lseek64 failed-1!\r\n");
        close(fdev);
        free(buffer);
        return -ESYSCALL;
    }
    sizer = read(fd, &tSparseHeader, sizeof(sparse_header_t));

    block_size = tSparseHeader.blk_sz;
    chunk_cnt = tSparseHeader.total_chunks;

    filelen -= sizeof(sparse_header_t);
    while ((chunk_cnt > 0) && (filelen > 0)) {
        sizer = read(fd, &tChunkHeader, CHUNK_HEADER_LEN);
        datalen = tChunkHeader.chunk_sz * block_size;

        if (CHUNK_TYPE_RAW == tChunkHeader.chunk_type) {
            if (datalen > memlen) {
                buffer = (uchar *)realloc((void *)buffer, datalen);
                if (!buffer) {
                    rec_err("realloc fail.\n");
                    close(fdev);
                    return -ENOMEM;
                }
                memlen = datalen;
            }
            pfileBuffer = buffer;

            sizer = read(fd, pfileBuffer, datalen);

            cur_offset = lseek64(fdev, offset, SEEK_SET);
            if (cur_offset != offset) {
                rec_err("lseek64 failed-2!\n");
                close(fdev);
                free(buffer);
                return -ESYSCALL;
            }
            sizew = write(fdev, pfileBuffer, datalen);

            filelen -= tChunkHeader.total_sz;
            chunk_cnt--;
            pfileBuffer+= datalen;
            offset += datalen;
        } else if (CHUNK_TYPE_DONT_CARE == tChunkHeader.chunk_type){
            lseek64(fd, tChunkHeader.total_sz - CHUNK_HEADER_LEN, SEEK_CUR);
            filelen -= tChunkHeader.total_sz;
            chunk_cnt--;
            offset += datalen;
        } else {
            rec_err("tChunkHeader.chunk_type error,type is %0x\n",tChunkHeader.chunk_type);
            close(fdev);
            free(buffer);
            return -ESPARSEFILE;
        }
    }

    close(fdev);
    if (buffer)
        free(buffer);

    if (0 != chunk_cnt) {
        rec_err("chunk_cnt error,chunk_cnt = %d\n", chunk_cnt);
        return -ESPARSEFILE;
    }

    rec_info("succeeded! memsize(0x%llx)\n", memlen);
    return 0;

}

#else /* !CONFIG_BOOT_MMC */

static int __upg_nand_preloader_by_raw_image(int fd, partitionread *ptbl,
	    long long img_offset, long long *psize_total)
{
	int ret = -1;
	int n = 0;
	char *buf = NULL;
	off64_t cur_off64 = 0;
	int64_t result = 0;
	uint64_t img_len;

	img_len = ptbl->u8RealDataSize;
	buf = (char *)malloc(PRELOADER_MAX_TOTAL_SIZE);
	if(buf == NULL) {
		rec_err("malloc fail .\n");
		ret = -ENOMEM;
		goto out;
	}
	memset(buf, 0, PRELOADER_MAX_TOTAL_SIZE);

	/*
	*positiion preloader image in sdcard by lseek64
	*/
	cur_off64 = lseek64(fd, img_offset, SEEK_SET);
	if (cur_off64 != img_offset) {
		rec_err("lseek fail.\n");
		ret = -ESYSCALL;
		goto out_free;
	}

	/*
	*skip 512 bytes,the 512 bytes is used to store BOOTLOADER_HEAD
	*/
	n = read(fd, buf + 512, img_len);
	if (n != img_len) {
		rec_err("read (%d) fail,expect (%d).\n", n, img_len);
		ret = -ESYSCALL;
		goto out_free;
	}

	ret = create_bootloader_header(buf, buf + 512, PRELOADER_SIZE, 0);
	if (ret < 0) {
		rec_err("create_bootloader_header fail.\n");
		ret = -EBLHEADER;
		goto out_free;
	}

	/*
	*upg preloader
	*/
	result = nand_raw_partition_write_by_emptycore(buf,
		    ptbl->u8PartitionStartAddr, PRELOADER_MAX_TOTAL_SIZE,
		    ptbl->u8PartitionSize);
	if (result < 0) {
		rec_err("nand_raw_partition_write_by_emptycore fail\n");
		ret = -ENANDWR;
		goto out_free;
	}

	*psize_total = img_len;
	ret = 0;

out_free:
	free(buf);
out:
	return ret;
}

static int upg_nand_preloader_by_raw_image(int fd, partitionread *ptbl,
	    long long img_offset, long long *psize_total)
{
	int ret = 0;

	if (fd < 0)
		return -EINVAL;

	if (ptbl == NULL)
		return -EINVAL;

	if (psize_total == NULL)
		return -EINVAL;

	if (ptbl->u8RealDataSize > (PRELOADER_MAX_TOTAL_SIZE - 512)) {
		rec_err("preloader image length(0x%llx) is larger than 63.5KB.\n",
			    ptbl->u8RealDataSize);
		return -EFILELEN;
	}
	
	ret = __upg_nand_preloader_by_raw_image(fd, ptbl, img_offset, psize_total);
	if (ret < 0) {
		rec_err("__upg_nand_preloader_by_raw_image fail.\n");
		return ret;
	}

	ret = nand_preloader_readback_check(ptbl, 0);
	if (ret < 0) {
		rec_err("nand preloader readback check fail.\n");
		return -ERDBACKCHK;
	}

	rec_info("upg nand preloader success .\n");
	return 0;
}

static int __upg_partition_from_sdcard(int fd,
	    long long img_offset, long long offset,
	    long long realdata_size, long long pt_size,
	    long long *psize_total, int mode)
{
	long long sizer = 0, sizew = 0, sizetotal = 0;
	long long org_realdata_size = 0;
	char *buffer = NULL;
	off64_t cur_offset = 0;
	uint32_t blk_size = 0;
	uint32_t buf_len = 0;
	uint32_t read_len = 0;
	uint32_t blk_cnt = 0;
	int ret = -1;

	rec_info("offset=0x%llx, realdata_size=0x%llx, pt_size=0x%llx, mode=%d\n",
		    offset, realdata_size, pt_size, mode);

	if (realdata_size <= 0) {
		rec_warn("realdata_size is 0.\n");
		*psize_total = 0;
		return 0;
	}

	org_realdata_size = realdata_size;

	ret = get_nand_block_size(&blk_size);
	if (ret < 0) {
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}
	rec_dbg("blk_size=0x%x\n", blk_size);

	/* check offset if align with nand block size */
	if (offset % blk_size) {
		rec_err("offset(0x%llx) not align nand block size(0x%x)\n",
			    offset, blk_size);
		return -ENANDALIGN;
	}

	/* calc how many nand blocks need to erase */
	//blk_cnt = (realdata_size + blk_size - 1) / blk_size;
	/* erase entire partition */
	blk_cnt = (pt_size + blk_size - 1) / blk_size;
	rec_info("blk_cnt = 0x%x\n", blk_cnt);

	/* image in ext sdcard, lseek it first */
	cur_offset = lseek64(fd, img_offset, SEEK_SET);
	if (cur_offset != img_offset) {
		rec_err("lseek64 failed!\n");
		return -ESYSCALL;
	}

	if (blk_size > FILE_RW_SIZE)
		buf_len = blk_size;
	else
		buf_len = FILE_RW_SIZE;
	buf_len = ALIGN(buf_len, blk_size);
	rec_dbg("buf_len=0x%x\n", buf_len);

	buffer = (char *)malloc(buf_len);
	if (buffer == NULL) {
		rec_err("malloc fail.\n");
		return -ENOMEM;
	}

	ret = nand_rw_start(lookup_idx_by_partname("allnand"), pt_size);
	if (ret < 0) {
		rec_err("nand_rw_start fail.\n");
		free(buffer);
		return -ENANDRW;
	}

	/* erase nand blocks */
	ret = nand_erase(offset, blk_cnt);
	if (ret < 0) {
		rec_err("nand_erase fail.\n");
		free(buffer);
		return -ENANDERASE;
	}

	sizetotal = 0;
	while (realdata_size > 0) {
		if (realdata_size >= buf_len) {
			sizer = read(fd, buffer, buf_len);
			read_len = buf_len;
		} else {
			sizer = read(fd, buffer, realdata_size);
			read_len = realdata_size;
		}

		if (read_len != sizer) {
			rec_err("read fail.\n");
			break;
		}

		rec_dbg("offset + sizetotal=0x%llx, sizer=0x%llx\n",
			    offset + sizetotal, sizer);
		if (mode)
			sizew = nand_ext4_common_write(buffer, offset + sizetotal, sizer);
		else
			sizew = nand_raw_common_write(buffer, offset + sizetotal, sizer);

		sizetotal += sizew;
		realdata_size -= sizer;
		if (sizew < sizer)
			break;
	}

	free(buffer);

	if (mode) {
		ret = nand_partition_reserve_blk_check(offset, pt_size);
		if (ret < 0) {
			rec_err("nand_partition_reserve_blk_check fail.\n");
			return -ENANDRSVBLK;
		}
	}

	ret = nand_rw_end();
	if (ret < 0) {
		rec_err("nand_rw_end fail.\n");
		return -ENANDRW;
	}

	if (sizetotal < org_realdata_size) {
		rec_err("sizetotal(0x%llx) < realdata_size(0x%llx)\n",
			    sizetotal, org_realdata_size);
		return -ENANDWR;
	}
	*psize_total = sizetotal;
	rec_info("succeeded, sizetotal(0x%llx)!\n", sizetotal);

	return 0;
}

#if 0
static int __upg_raw_partition_from_sdcard(int fd, 
	    long long img_offset, long long offset, long long part_size,
	    long long realdata_size, long long *psize_total)
{
	long long sizer = 0, sizew = 0, sizetotal = 0;
	long long org_realdata_size = 0;
	char *buffer = NULL;
	off64_t cur_offset = 0;
	uint32_t blk_size = 0;
	uint32_t buf_len = 0;
	uint32_t read_len = 0;
	int ret = -1;

	rec_info("offset=0x%llx, realdata_size=0x%llx,\n",
		    offset, realdata_size);

	if (realdata_size <= 0) {
		rec_warn("realdata_size is 0.\n");
		*psize_total = 0;
		return 0;
	}
	org_realdata_size = realdata_size;

	ret = get_nand_block_size(&blk_size);
	if (ret < 0) {
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}
	rec_dbg("blk_size=0x%x\n", blk_size);

	//image in ext sdcard, lseek it first
	cur_offset = lseek64(fd, img_offset, SEEK_SET);
	if (cur_offset != img_offset) {
		rec_err("lseek64 failed!\n");
		return -ESYSCALL;
	}

	if (blk_size > FILE_RW_SIZE)
		buf_len = blk_size;
	else
		buf_len = FILE_RW_SIZE;
	buf_len = ALIGN(buf_len, blk_size);
	rec_dbg("buf_len=0x%x\n", buf_len);

	buffer = (char *)malloc(buf_len);
	if (buffer == NULL) {
		rec_err("malloc fail.\n");
		return -ENOMEM;
	}

	ret = nand_rw_start(lookup_idx_by_partname("allnand"), realdata_size);
	if (ret < 0) {
		rec_err("nand_rw_start fail.\n");
		free(buffer);
		return -ENANDRW;
	}

	sizetotal = 0;
	while (realdata_size > 0) {
		if (realdata_size >= buf_len) {
			sizer = read(fd, buffer, buf_len);
			read_len = buf_len;
		} else {
			sizer = read(fd, buffer, realdata_size);
			read_len = realdata_size;
		}

		if (read_len != sizer) {
			rec_err("read fail.\n");
			break;
		}

		rec_dbg("offset + sizetotal=0x%llx, sizer=0x%llx\n",
			    offset + sizetotal, sizer);
		sizew = nand_raw_common_write(buffer, offset + sizetotal, sizer);

		sizetotal += sizew;
		realdata_size -= sizer;
		if (sizew < sizer)
			break;
	}

	free(buffer);
	ret = nand_rw_end();
	if (ret < 0) {
		rec_err("nand_rw_end fail.\n");
		return -ENANDRW;
	}
	if (sizetotal < org_realdata_size) {
		rec_err("sizetotal(0x%llx) < realdata_size(0x%llx)\n",
			    sizetotal, org_realdata_size);
		return -ENANDWR;
	}

	*psize_total = sizetotal;
	rec_info("succeeded, sizetotal(0x%llx)!\n", sizetotal);

	return 0;
}
#endif
static int __upg_raw_partition_from_sdcard(int fd, 
	    long long img_offset, long long offset,
	    long long realdata_size, long long part_size,
	    long long *psize_total)
{
	return __upg_partition_from_sdcard(fd, img_offset, offset,
			    realdata_size, part_size, psize_total, 0);
}

#if 0
static int __upg_ext4_partition_from_sdcard(int fd,
	    long long img_offset, long long offset,
	    long long realdata_size, long long pt_size, long long *psize_total)
{
	long long sizer = 0, sizew = 0, sizetotal = 0;
	long long org_realdata_size = 0;
	char *buffer = NULL;
	off64_t cur_offset = 0;
	uint32_t blk_size = 0;
	uint32_t buf_len = 0;
	uint32_t read_len = 0;
	uint32_t blk_cnt = 0;
	int ret = -1;

	rec_info("offset=0x%llx, realdata_size=0x%llx, pt_size=0x%llx\n",
		    offset, realdata_size, pt_size);

	if (realdata_size <= 0) {
		rec_warn("realdata_size is 0.\n");
		*psize_total = 0;
		return 0;
	}

	org_realdata_size = realdata_size;

	ret = get_nand_block_size(&blk_size);
	if (ret < 0) {
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}
	rec_dbg("blk_size=0x%x\n", blk_size);

	/* check offset if align with nand block size */
	if (offset % blk_size) {
		rec_err("offset(0x%llx) not align nand block size(0x%x)\n",
			    offset, blk_size);
		return -ENANDALIGN;
	}

	/* calc how many nand blocks need to erase */
	//blk_cnt = (realdata_size + blk_size - 1) / blk_size;
	/* erase entire partition */
	blk_cnt = (pt_size + blk_size - 1) / blk_size;
	rec_info("blk_cnt = 0x%x\n", blk_cnt);

	/* image in ext sdcard, lseek it first */
	cur_offset = lseek64(fd, img_offset, SEEK_SET);
	if (cur_offset != img_offset) {
		rec_err("lseek64 failed!\n");
		return -ESYSCALL;
	}

	if (blk_size > FILE_RW_SIZE)
		buf_len = blk_size;
	else
		buf_len = FILE_RW_SIZE;
	buf_len = ALIGN(buf_len, blk_size);
	rec_dbg("buf_len=0x%x\n", buf_len);

	buffer = (char *)malloc(buf_len);
	if (buffer == NULL) {
		rec_err("malloc fail.\n");
		return -ENOMEM;
	}

	ret = nand_rw_start(lookup_idx_by_partname("allnand"), realdata_size);
	if (ret < 0) {
		rec_err("nand_rw_start fail.\n");
		free(buffer);
		return -ENANDRW;
	}

	/* erase nand blocks */
	ret = nand_erase(offset, blk_cnt);
	if (ret < 0) {
		rec_err("nand_erase fail.\n");
		free(buffer);
		return -ENANDERASE;
	}

	sizetotal = 0;
	while (realdata_size > 0) {
		if (realdata_size >= buf_len) {
			sizer = read(fd, buffer, buf_len);
			read_len = buf_len;
		} else {
			sizer = read(fd, buffer, realdata_size);
			read_len = realdata_size;
		}

		if (read_len != sizer) {
			rec_err("read fail.\n");
			break;
		}

		rec_dbg("offset + sizetotal=0x%llx, sizer=0x%llx\n",
			    offset + sizetotal, sizer);
		sizew = nand_ext4_common_write(buffer, offset + sizetotal, sizer);

		sizetotal += sizew;
		realdata_size -= sizer;
		if (sizew < sizer)
			break;
	}

	free(buffer);
	ret = nand_rw_end();
	if (ret < 0) {
		rec_err("nand_rw_end fail.\n");
		return -ENANDRW;
	}
	
	if (sizetotal < org_realdata_size) {
		rec_err("sizetotal(0x%llx) < realdata_size(0x%llx)\n",
			    sizetotal, org_realdata_size);
		return -ENANDWR;
	}
	*psize_total = sizetotal;
	rec_info("succeeded, sizetotal(0x%llx)!\n", sizetotal);

	return 0;
}
#endif
static int __upg_ext4_partition_from_sdcard(int fd,
	    long long img_offset, long long offset,
	    long long realdata_size, long long pt_size,
	    long long *psize_total)
{
	return __upg_partition_from_sdcard(fd, img_offset, offset,
			    realdata_size, pt_size, psize_total, 1);
}

/**
* fd: ext sdcard fd
* devname: emmc device node name, useless for nand
* img_offset: image address in ext sdcard
* offset: image address in nand
*/
static int emptycore_upg_raw_partition_from_sdcard(int fd,
	    const char *devname, long long img_offset,
	    long long offset, long long part_size,
	    long long realdata_size, long long *psize_total)
{
	return __upg_raw_partition_from_sdcard(fd, img_offset,
				    offset,realdata_size, part_size,
				    psize_total);
}

#if 0
static int emptycore_upg_ext4_partition_from_sdcard(int fd, const char *devname,
        long long img_offset, long long realdata_size, long long offset, long long size)
{
	long long size_total;

	return __upg_partition_from_sdcard(fd, 1, img_offset, offset, realdata_size, &size_total);
}
#endif

static int emptycore_upg_ext4_partition_from_sdcard(int fd, const char *devname,
        long long img_offset, long long realdata_size,
        long long offset, long long pt_size)
{
	long long size_total;

	return __upg_ext4_partition_from_sdcard(fd,
		    img_offset, offset, realdata_size, 
		    pt_size, &size_total);
}

static uint32_t calc_checksum_from_sdcard(int fd,
	    long long img_offset, long long realdata_size)
{
	long long sizer = 0;
	char *buffer = NULL;
	off64_t cur_offset = 0;
	uint32_t chksum = 0;

	if (realdata_size <= 0) {
		rec_warn("realdata_size is 0.\n");
		return 0;
	}

	//image in ext sdcard, lseek it first
	cur_offset = lseek64(fd, img_offset, SEEK_SET);
	if (cur_offset != img_offset) {
		rec_err("lseek64 failed-1!\n");
		return 0;
	}

	buffer = (char *)malloc(FILE_RW_SIZE);
	if (!buffer) {
		rec_err("malloc fail.\n");
		return 0;
	}

	while (realdata_size > 0) {
		if (realdata_size >= FILE_RW_SIZE) {
			sizer = read(fd, buffer, FILE_RW_SIZE);
		} else {
			sizer = read(fd, buffer, realdata_size);
		}
		chksum = checksum32(chksum, buffer, sizer);
		realdata_size -= sizer;
	}

	free(buffer);
	
	rec_info("succeeded, chksum(0x%x)!\n", chksum);

	return chksum;
}

static uint32_t calc_checksum_from_sdcard_before_upg(int fd,
	    long long img_offset, long long realdata_size,
	    partitionread *pentry)
{
	if (pentry == NULL)
		return 0x12345678;

	if (realdata_size <= 0)
		return 0;

	if (strcmp(pentry->szPartName, "preloader") == 0 ||
		    strcmp(pentry->szPartName, "preloader_bk") == 0 ||
		    strcmp(pentry->szPartName, "datazone") == 0 ||
		    strcmp(pentry->szPartName, "datazone_bk") == 0) {
		return 0;
	}

	return calc_checksum_from_sdcard(fd, img_offset, realdata_size);
}

#endif /* CONFIG_BOOT_MMC */

static int emptycore_upg_raw_partition_from_file(const char *devname, 
	    const char *file, long long offset, long long size, long long *psize_total)
{
	return upg_raw_partition_from_file(devname, file, offset, size, psize_total);
}

static int emptycore_upg_ext4_partition_from_file(const char *devname, 
	    const char *file, long long offset, long long size)
{
	return upg_ext4_partition_from_file(devname, file, offset, size);
}

struct upg_one_part_arg_t {
	RecoveryUpdateModule *prum;
	int fd;
	partitionread *part;
	long long img_offset;
};

static int emptycore_upgrade_one_partition(struct upg_one_part_arg_t *arg, int from_udisk)
{
    int status = 0;
    char file[IMG_FULL_NAME_MAX];
    int ret = ESUCCESS;
    long long size_total = 0;
	int fd = 0;
	long long img_offset = 0;

	RecoveryUpdateModule *prum = NULL;
	partitionread *part = NULL;

	if (!arg)
		return 0;

	prum = arg->prum;
	part = arg->part;

	if (part == NULL)
		return 0;

#ifdef CONFIG_BOOT_MMC
	if (part->u8PartitionStartAddr < 0x10000)
		return 0;
#endif

	if (from_udisk == 0) {
		//upg from sdcard
		fd = arg->fd;
		img_offset = arg->img_offset;
	}

	if ((strcmp(part->szPartName, "mcu") == 0) && (prum->mMcu == 0))
		return ret;


	if ((strcmp(part->szPartName, "navi") == 0) && (prum->mNavi== 0))
		return ret;

	if (strcmp(part->szType, "fat32") == 0) {
		if (part->u4Mount & 0x2) {
			format_userdata_partition_fat32(part->u8PartitionStartAddr, part->u8PartitionSize);
		}

		if (part->u4Mount)
			part->u4Mount = 1;
			return ret;
	}

	/* which partition is ongoing upgrade. */
	pthread_mutex_lock(&g_upgrade_mutex);
	strncpy(g_upgrade_status_self.upgrade_ongoing_part_name, part->szPartName, PART_NAME_LEN_MAX - 1);
	pthread_mutex_unlock(&g_upgrade_mutex);
	rec_info("ongoing upgrade %s \n", part->szPartName);

	if (from_udisk) {
		if (strlen(part->szImageFileName)) {
			strcpy(file, ISO_ROOT);
			strcat(file, part->szImageFileName);
			if (!check_is_file_exist(file)) {
				return -ENOFILE;
			}
		} else {
			return 0;
		}
	}

	if (strcmp(part->szType, "raw") == 0) {
		if (from_udisk) {
			ret = emptycore_upg_raw_partition_from_file(devname, file, part->u8PartitionStartAddr ,
				part->u8PartitionSize, &size_total);
		} else {
			#ifndef CONFIG_BOOT_MMC
			/*for nand preloader upg*/
			if (strcmp(part->szPartName, "preloader") == 0 ||
				    strcmp(part->szPartName, "preloader_bk") == 0)
				ret = upg_nand_preloader_by_raw_image(fd, part, img_offset, &size_total);
			else
				ret = emptycore_upg_raw_partition_from_sdcard(fd,
					    devname, img_offset,
					    part->u8PartitionStartAddr, part->u8PartitionSize,
					    part->u8RealDataSize, &size_total);
			#else
				ret = emptycore_upg_raw_partition_from_sdcard(fd, devname, img_offset,
				    part->u8PartitionStartAddr, part->u8RealDataSize, &size_total);
			#endif
		}
		pthread_mutex_lock(&g_upgrade_mutex);
		if (ret >= 0) {
			g_upgrade_status_self.size_upgrade_done += size_total;
			g_upgrade_status_self.one_partition_status[g_upgrade_status_self.part_count_upgrade_done].flag = UPGRADE_FLAG_DONE | UPGRADE_FLAG_SUCCESS;
			strncpy(g_upgrade_status_self.one_partition_status[g_upgrade_status_self.part_count_upgrade_done].part_name, part->szPartName, PART_NAME_LEN_MAX - 1);
			size_total = ALIGN(size_total, 512ULL); //align 512
			part->u8RealDataSize = size_total;
			part->u4LastPartition |= UPDATE_FLAG_DONE;
			ret = ESUCCESS;
		} else {
			g_upgrade_status_self.one_partition_status[g_upgrade_status_self.part_count_upgrade_done].flag = UPGRADE_FLAG_DONE;
			strncpy(g_upgrade_status_self.one_partition_status[g_upgrade_status_self.part_count_upgrade_done].part_name, part->szPartName, PART_NAME_LEN_MAX - 1);
		}
		g_upgrade_status_self.part_count_upgrade_done++;
		if (g_upgrade_status_self.part_count_upgrade_done >= PART_NUM_MAX) {
			rec_err("partition num >= %d\n", PART_NUM_MAX);
			g_upgrade_status_self.part_count_upgrade_done--;
		}
		pthread_mutex_unlock(&g_upgrade_mutex);
		} else if (strcmp(part->szType, "ext4") == 0) {

		if (from_udisk) {
			ret = emptycore_upg_ext4_partition_from_file(devname,
				    file, part->u8PartitionStartAddr, part->u8PartitionSize);
		} else {
			ret = emptycore_upg_ext4_partition_from_sdcard(fd, devname,
				    img_offset, part->u8RealDataSize,
				    part->u8PartitionStartAddr,
				    part->u8PartitionSize);
		}
		pthread_mutex_lock(&g_upgrade_mutex);
		if (ret >= 0) {
			g_upgrade_status_self.size_upgrade_done += part->u8RealDataSize;;
			g_upgrade_status_self.one_partition_status[g_upgrade_status_self.part_count_upgrade_done].flag = UPGRADE_FLAG_DONE | UPGRADE_FLAG_SUCCESS;
			strncpy(g_upgrade_status_self.one_partition_status[g_upgrade_status_self.part_count_upgrade_done].part_name, part->szPartName, PART_NAME_LEN_MAX - 1);
			part->u8RealDataSize = ret;
			part->u4LastPartition |= UPDATE_FLAG_DONE;
			ret = ESUCCESS;
		} else {
			g_upgrade_status_self.one_partition_status[g_upgrade_status_self.part_count_upgrade_done].flag = UPGRADE_FLAG_DONE;
			strncpy(g_upgrade_status_self.one_partition_status[g_upgrade_status_self.part_count_upgrade_done].part_name, part->szPartName, PART_NAME_LEN_MAX - 1);
		}
		g_upgrade_status_self.part_count_upgrade_done++;
		if (g_upgrade_status_self.part_count_upgrade_done >= PART_NUM_MAX) {
			rec_err("partition num >= %d\n", PART_NUM_MAX);
			g_upgrade_status_self.part_count_upgrade_done--;
		}
		pthread_mutex_unlock(&g_upgrade_mutex);
	}

	return ret;
}

static int emptycore_calc_total_size_need_upgrade_sdcard(RecoveryUpdateModule *prum, int fd, u64 img_start_addr)
{
	int n;
	int ret;
	unsigned long long flen;
	off64_t cur_offset64;
	char buf[PTBL_BLOCK_SIZE];
	partitioninfo *partinfo;
	int board_type = 0;

	if (prum == NULL)
		return -EINVAL;

	do {
		cur_offset64 = lseek64(fd, img_start_addr, SEEK_SET);
		if (cur_offset64 != img_start_addr) {
			rec_err("lseek64 fails, img_start_addr=0x%llx\n", img_start_addr);
			return -ESYSCALL;
		}

		n = read(fd, buf, 512);
		if (n != 512) {
			rec_err("block_read fail result=%d\n", n);
			return -ESYSCALL;
		}

		partinfo = (partitioninfo *)buf;
		pthread_mutex_lock(&g_upgrade_mutex);
		g_upgrade_status_self.partition_total_num++;
		g_upgrade_status_self.need_upgrade_partition_num++;
		g_upgrade_status_self.total_size_need_upgrade += partinfo->u8RealDataSize;
	/*
	*preloader size is calculated into total size for nand
	*/
	#ifdef CONFIG_BOOT_MMC
		/* preloader is calculated into total size for nand upg, but emmc not */
		if (strcmp(partinfo->szPartName, "preloader") == 0) {
			g_upgrade_status_self.total_size_need_upgrade -= partinfo->u8RealDataSize;
			g_upgrade_status_self.need_upgrade_partition_num--;
		}
		else
	#endif
		if ((strcmp(partinfo->szPartName, "mcu") == 0) && (prum->mMcu == 0)) {
			g_upgrade_status_self.total_size_need_upgrade -= partinfo->u8RealDataSize;
			g_upgrade_status_self.need_upgrade_partition_num--;
		} else if ((strcmp(partinfo->szPartName, "navi") == 0) && (prum->mNavi == 0)) {
			g_upgrade_status_self.total_size_need_upgrade -= partinfo->u8RealDataSize;
			g_upgrade_status_self.need_upgrade_partition_num--;
		}
		pthread_mutex_unlock(&g_upgrade_mutex);
		img_start_addr = partinfo->u8OffsetNextImage;
	} while(partinfo->u8OffsetNextImage != 0);

	rec_info("total_size_need_upgrade(0x%llx)\n", g_upgrade_status_self.total_size_need_upgrade);

	return 0;
}

static int emptycore_calc_total_size_need_upgrade_udisk(RecoveryUpdateModule *prum,
	    partitionread *ptbl)
{
	int ret = 0;
	unsigned long long n = 0;

	if (prum == NULL || ptbl == NULL)
		return -EINVAL;

	//pthread_mutex_lock(&g_upgrade_mutex);
	for (; ptbl; ptbl = ptbl->nextpartition) {
		n = 0;
		if (!partition_need_imagefile(ptbl))
			continue;

		ret = emptycore_get_file_length(ptbl->szImageFileName, &n);
		if (ret < 0) {
			rec_err("emptycore_get_file_length fail.\n");
			//pthread_mutex_unlock(&g_upgrade_mutex);
			return -EFILELEN;
		}
		rec_info("[qydebug]szImageFileName=%s,n=%lld", ptbl->szImageFileName, n);
		ptbl->u8RealDataSize = n;
		g_upgrade_status_self.partition_total_num++;
		g_upgrade_status_self.need_upgrade_partition_num++;
		g_upgrade_status_self.total_size_need_upgrade += n;

		if (strcmp(ptbl->szPartName, "preloader") == 0) {
			g_upgrade_status_self.total_size_need_upgrade -= n;
			g_upgrade_status_self.need_upgrade_partition_num--;
		} else if ((strcmp(ptbl->szPartName, "mcu") == 0) && (!prum->mMcu)) {
			g_upgrade_status_self.total_size_need_upgrade -= n;
			g_upgrade_status_self.need_upgrade_partition_num--;
		} else if ((strcmp(ptbl->szPartName, "navi") == 0) && (!prum->mNavi)) {
			g_upgrade_status_self.total_size_need_upgrade -= n;
			g_upgrade_status_self.need_upgrade_partition_num--;
		}
	}

	//pthread_mutex_unlock(&g_upgrade_mutex);
	rec_info("total_size_need_upgrade(0x%llx)\n", g_upgrade_status_self.total_size_need_upgrade);

	return 0;
}

static int recalculate_mac_address(u32 *mac_addr_buf)
{
	mac_addr_buf[6] = 0;
	if (mac_addr_buf[5]== 255) {
		mac_addr_buf[5] = 0;
		if (mac_addr_buf[4] == 255) {
			mac_addr_buf[4] = 0;
			if (mac_addr_buf[3] == 255) {
				mac_addr_buf[3] = 0;
				if (mac_addr_buf[2]== 255) {
					mac_addr_buf[2] = 0;
					if (mac_addr_buf[1] == 255) {
						mac_addr_buf[1] = 0;
						if (mac_addr_buf[0]== 255)
							return -1;
						else {
							mac_addr_buf[0] += 1;
							return 0;
						}
					} else {
						mac_addr_buf[1] += 1;
						return 0;
					}
				} else {
					mac_addr_buf[2] += 1;
					return 0;
				}
			} else {
				mac_addr_buf[3] += 1;
				return 0;
			}
		} else {
			mac_addr_buf[4] += 1;
			return 0;
		}
	} else {
		mac_addr_buf[5] += 1;
		return 0;
	}
}

static int emptycore_update_connsys_info(int bootdevice,
	u64 mac_start_addr,
	u64 mz_pt_addr, u64 mz_pt_size, u64 mz_real_size)
{
	char bd_str[24] = {0};
	int ret;
	int fd;
#ifdef CONFIG_BOOT_MMC
	int fdev;
#endif
	int sizer, sizew;
	off64_t cur_offset, offset;
	u32 mac_cnt_buf[512 / sizeof(u32)];
	u32 mac_addr_buf[512 / sizeof(u32)];
	u32 mac_cnt;
	u16 chip_buf[512 / sizeof(u16)];
	u8 mac_addr[6];
	u8 buf[8];

	if ((bootdevice == 1) || (bootdevice == 2)) {
		snprintf(bd_str, 24, "slot%d", bootdevice);
		rec_info("bootdevice is %s \n", bd_str);
	} else {
		rec_err("boot device number(%d) is wrong.\n", bootdevice);
		ret = -EBADBOOTDEV;
		goto out;
	}
#ifdef CONFIG_BOOT_MMC
	//open boot sdcard to read.
	fd = open("/dev/mmcblk1", O_RDWR | O_LARGEFILE);
	if (fd < 0) {
		rec_err("open /dev/mmcblk1 fail.\n");
		ret = -ESYSCALL;
		goto out;
	}
#else
	//open boot sdcard to read.
	fd = open("/dev/mmcblk0", O_RDWR | O_LARGEFILE);
	if (fd < 0) {
		rec_err("open /dev/mmcblk0 fail.\n");
		ret = -ESYSCALL;
		goto out;
	}
#endif
#ifdef CONFIG_BOOT_MMC
	//open emmc device node to write
	fdev = open(devname, O_RDWR | O_LARGEFILE);
	if (fdev < 0) {
		rec_err("open %s fail.\n", devname);
		ret = -ESYSCALL;
		goto out_close_fd;
	}
#else
#if 0
	ret = nand_rw_start(metazone_pt_realsize);
	if (ret < 0) {
		rec_err("nand_rw_start fail.\n");
		ret = -ENANDRW;
		goto out_close_fd;
	}
#endif
#endif /* CONFIG_BOOT_MMC */

	offset = mac_start_addr + 2 * PTBL_BLOCK_SIZE;
	cur_offset = lseek64(fd, offset, SEEK_SET);
	if (cur_offset != offset) {
		rec_err("seek fail for wifi_mac_cnt read from sd.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
	sizer = read(fd, mac_cnt_buf, 512);
	if (sizer != 512) {
		rec_err("read fail for wifi_mac_cnt reads.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
	mac_cnt = mac_cnt_buf[0];
	rec_info("========read MMC wifi mac count %d========\n", mac_cnt);
	
	if (mac_cnt != 0) {
		ret = -1;
		offset = mac_start_addr;
		cur_offset = lseek64(fd, offset, SEEK_SET);
		if (cur_offset != offset) {
			rec_err("seek fail for wifi_mac_addr read from sd.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}

		sizer = read(fd, mac_addr_buf, 512);
		if (sizer != 512) {
			rec_err("read fail for wifi_mac_addr read.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
		mac_addr[0] = (u8)mac_addr_buf[0];
		mac_addr[1] = (u8)mac_addr_buf[1];
		mac_addr[2] = (u8)mac_addr_buf[2];
		mac_addr[3] = (u8)mac_addr_buf[3];
		mac_addr[4] = (u8)mac_addr_buf[4];
		mac_addr[5] = (u8)mac_addr_buf[5];
		rec_info("========read MMC wifi mac %x:%x:%x:%x:%x:%x========\n", 
			    mac_addr[0], mac_addr[1],
			    mac_addr[2], mac_addr[3],
			    mac_addr[4], mac_addr[5]);

		/*
		* metazone 
		* Index---------data size offset-----data offset
		* 0x10026------0x9F10-----------0x9F14
		*/
		offset = mz_pt_addr + WIFIMAC_DATASIZE_OFFSET;
		buf[0] = 6;
	#ifdef CONFIG_BOOT_MMC
		cur_offset = lseek64(fdev, offset, SEEK_SET);
		if (cur_offset != offset) {
			rec_err("seek fail for wifi_mac_addr datasize.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
		sizew = write(fdev, buf, 1);
		if (sizew != 1) {
			rec_err("write datasize for wifi_mac_addr in metazone.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
	#else /* !CONFIG_BOOT_MMC */
		offset = WIFIMAC_DATASIZE_OFFSET;
		ret = nand_raw_partition_write_offset_by_emptycore(buf,
				    mz_pt_addr, offset,
				    1, mz_real_size,
				    mz_pt_size);
		if (ret < 0) {
			rec_err("nand_raw_common_write_with_rwctrl fail -- 1\n");
			ret = -ENANDWR;
			goto out_close_fdev;
		}
	#endif /* CONFIG_BOOT_MMC */

		offset = mz_pt_addr + WIFIMAC_DATA_OFFSET;
	#ifdef CONFIG_BOOT_MMC
		cur_offset = lseek64(fdev, offset, SEEK_SET);
		if (cur_offset != offset) {
			rec_err("seek fail for wifi_mac_addr.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
		sizew = write(fdev, mac_addr, 6);
		if (sizew != 6) {
			rec_err("write fail for wifi_mac_addr in metazone, %s.\n", strerror(errno));
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
	#else /* !CONFIG_BOOT_MMC */
		offset = WIFIMAC_DATA_OFFSET;
		ret = nand_raw_partition_write_offset_by_emptycore(mac_addr,
				    mz_pt_addr, offset,
				    6, mz_real_size,
				    mz_pt_size);
		if (ret < 0) {
			rec_err("nand_raw_common_write_with_rwctrl fail -- 2\n");
			ret = -ENANDWR;
			goto out_close_fdev;
		}
	#endif /* CONFIG_BOOT_MMC */

		/*
		* write back sd to make wifi mac addr different value.
		*/
		recalculate_mac_address(mac_addr_buf);
		mac_cnt--;
		offset = mac_start_addr;
		cur_offset = lseek64(fd, offset, SEEK_SET);
		if (cur_offset != offset) {
			rec_err("seek fail for wifi_mac_addr backwrite into sd.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
		sizew = write(fd, (char *)mac_addr_buf, 512);
		if (sizew != 512) {
			rec_err("write fail for wifi_mac_addr backwrite.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
		offset = mac_start_addr + 2 * 512;
		cur_offset = lseek64(fd, offset, SEEK_SET);
		if (cur_offset != offset) {
			rec_err("seek fail for wifi_mac_cnt backwrite into sd.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
		sizew = write(fd, (char *)&mac_cnt, sizeof(u32));
		if (sizew != sizeof(u32)) {
			rec_err("write fail for wifi_mac_cnt backwrite.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
	} else {
		rec_warn("========Use random WiFi MAC Address========\n");
	}

	offset = mac_start_addr + 3 * 512;
	cur_offset = lseek64(fd, offset, SEEK_SET);
	if (cur_offset != offset) {
		rec_err("seek fail for bt_mac_cnt read from sd.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
	sizer = read(fd, mac_cnt_buf, 512);
	if (sizer != 512) {
		rec_err("read fail for bt_mac_cnt.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
	mac_cnt = mac_cnt_buf[0];
	rec_info("========read MMC bt mac count %d========\n", mac_cnt);

	if (mac_cnt != 0) {
		offset = mac_start_addr + 1 * 512;
		cur_offset = lseek64(fd, offset, SEEK_SET);
		if (cur_offset != offset) {
			rec_err("seek fail for bt_mac_addr read from sd.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
		sizer = read(fd, mac_addr_buf, 512);
		if (sizer != 512) {
			rec_err("read fail for bt_mac_addr.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
		
		mac_addr[0] = (u8)mac_addr_buf[0];
		mac_addr[1] = (u8)mac_addr_buf[1];
		mac_addr[2] = (u8)mac_addr_buf[2];
		mac_addr[3] = (u8)mac_addr_buf[3];
		mac_addr[4] = (u8)mac_addr_buf[4];
		mac_addr[5] = (u8)mac_addr_buf[5];
		rec_info("========read MMC bt mac %x:%x:%x:%x:%x:%x========\n",
			    mac_addr[0], mac_addr[1],
			    mac_addr[2], mac_addr[3],
			    mac_addr[4], mac_addr[5]);

		/*
		* metazone 
		* Index---------data size offset-----data offset
		* 0x10027------0x9F78-----------0x9F7B
		*/
		offset = mz_pt_addr + BTMAC_DATASIZE_OFFSET;
		buf[0] = 6;
	#ifdef CONFIG_BOOT_MMC
		cur_offset = lseek64(fdev, offset, SEEK_SET);
		if (cur_offset != offset) {
			rec_err("seek fail for bt_mac_addr datasize.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
		sizew = write(fdev, buf, 1);
		if (sizew != 1) {
			rec_err("write datasize for bt_mac_addr in metazone.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
	#else /* !CONFIG_BOOT_MMC */
		offset = BTMAC_DATASIZE_OFFSET;
		ret = nand_raw_partition_write_offset_by_emptycore(buf,
				    mz_pt_addr, offset,
				    1, mz_real_size,
				    mz_pt_size);
		if (ret < 0) {
			rec_err("nand_raw_common_write_with_rwctrl fail -- 3\n");
			ret = -ENANDWR;
			goto out_close_fdev;
		}
	#endif /* CONFIG_BOOT_MMC */

		offset = mz_pt_addr + BTMAC_DATA_OFFSET;
	#ifdef CONFIG_BOOT_MMC
		cur_offset = lseek64(fdev, offset, SEEK_SET);
		if (cur_offset != offset) {
			rec_err("seek fail for bt_mac_addr.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
		sizew = write(fdev, mac_addr, 6);
		if (sizew != 6) {
			rec_err("write fail for bt_mac_addr in metazone.%s\n", strerror(errno));
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
	#else /* !CONFIG_BOOT_MMC */
		offset = BTMAC_DATA_OFFSET;
		ret = nand_raw_partition_write_offset_by_emptycore(mac_addr,
				    mz_pt_addr, offset,
				    6, mz_real_size,
				    mz_pt_size);
		if (ret < 0) {
			rec_err("nand_raw_common_write_with_rwctrl fail -- 4\n");
			ret = -ENANDWR;
			goto out_close_fdev;
		}
	#endif /* CONFIG_BOOT_MMC */

		recalculate_mac_address(mac_addr_buf);
		mac_cnt--;
		offset = mac_start_addr + 1 * 512;
		cur_offset = lseek64(fd, offset, SEEK_SET);
		if (cur_offset != offset) {
			rec_err("seek fail for bt_mac_addr backwrite into sd.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
		sizew = write(fd, (char *)mac_addr_buf, 512);
		if (sizew != 512) {
			rec_err("write fail for bt_mac_addr backwrite.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}

		offset = mac_start_addr + 3 * 512;
		cur_offset = lseek64(fd, offset, SEEK_SET);
		if (cur_offset != offset) {
			rec_err("seek fail for bt_mac_cnt backwrite into sd.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
		sizew = write(fd, (char *)&mac_cnt, sizeof(u32));
		if (sizew != sizeof(u32)) {
			rec_err("write fail for bt_mac_cnt backwrite.\n");
			ret = -ESYSCALL;
			goto out_close_fdev;
		}
	} else {
		rec_warn("========Use random BT MAC Address========\n");
	}

	offset = mac_start_addr + 4 * 512;
	cur_offset = lseek64(fd, offset, SEEK_SET);
	if (cur_offset != offset) {
		rec_err("seek fail for chip-id read from sd.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
	sizer = read(fd, chip_buf, 512);
	if (sizer != 512) {
		rec_err("read fail for chip-id.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}

	rec_info("\nWiFi Chip ID: %d\n", chip_buf[0]);
	/*
	* metazone 
	* Index---------data size offset-----data offset
	* 0x10028------NA---------------0x80A0
	*/
	offset = mz_pt_addr + WIFICHIP_DATA_OFFSET;
#ifdef CONFIG_BOOT_MMC
	cur_offset = lseek64(fdev, offset, SEEK_SET);
	if (cur_offset != offset) {
		rec_err("seek fail for wifi-chip-id.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
	sizew = write(fdev, &chip_buf[0], sizeof(u16));
	if (sizew != sizeof(u16)) {
		rec_err("write fail for wifi-chip-id in metazone.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
#else /* !CONFIG_BOOT_MMC */
	offset = WIFICHIP_DATA_OFFSET;
	ret = nand_raw_partition_write_offset_by_emptycore(&chip_buf[0],
			    mz_pt_addr, offset,
			    sizeof(u16), mz_real_size,
			    mz_pt_size);
	if (ret < 0) {
		rec_err("nand_raw_common_write_with_rwctrl fail -- 5\n");
		ret = -ENANDWR;
		goto out_close_fdev;
	}
#endif /* CONFIG_BOOT_MMC */

	rec_info("\nBT Chip ID: %d\n", chip_buf[1]);
	/*
	* metazone 
	* Index---------data size offset-----data offset
	* 0x10029------NA---------------0x80A4
	*/
	offset = mz_pt_addr + GPSCHIP_DATA_OFFSET;
#ifdef CONFIG_BOOT_MMC
	cur_offset = lseek64(fdev, offset, SEEK_SET);
	if (cur_offset != offset) {
		rec_err("seek fail for gps-chip-id.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
	sizew = write(fdev, &chip_buf[2], sizeof(u16));
	if (sizew != sizeof(u16)) {
		rec_err("write fail for gps-chip-id in metazone.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
#else /* !CONFIG_BOOT_MMC */
	offset = GPSCHIP_DATA_OFFSET;
	ret = nand_raw_partition_write_offset_by_emptycore(&chip_buf[2],
			    mz_pt_addr, offset,
			    sizeof(u16), mz_real_size,
			    mz_pt_size);
	if (ret < 0) {
		rec_err("nand_raw_common_write_with_rwctrl fail -- 6\n");
		ret = -ENANDWR;
		goto out_close_fdev;
	}
#endif /* CONFIG_BOOT_MMC */

	rec_info("\nGPS Chip ID: %d\n", chip_buf[2]);
	/*
	* metazone 
	* Index---------data size offset-----data offset
	* 0x10030------NA---------------0x80C0
	*/
	offset = mz_pt_addr + BTCHIP_DATA_OFFSET;
#ifdef CONFIG_BOOT_MMC
	cur_offset = lseek64(fdev, offset, SEEK_SET);
	if (cur_offset != offset) {
		rec_err("seek fail for bt-chip-id.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
	sizew = write(fdev, &chip_buf[1], sizeof(u16));
	if (sizew != sizeof(u16)) {
		rec_err("write fail for bt-chip-id in metazone.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
#else /* !CONFIG_BOOT_MMC */
	offset = BTCHIP_DATA_OFFSET;
	ret = nand_raw_partition_write_offset_by_emptycore(&chip_buf[1],
			    mz_pt_addr, offset,
			    sizeof(u16), mz_real_size,
			    mz_pt_size);
	if (ret < 0) {
		rec_err("nand_raw_common_write_with_rwctrl fail -- 7\n");
		ret = -ENANDWR;
		goto out_close_fdev;
	}
#endif /* CONFIG_BOOT_MMC */

	//for front rear type
	offset = mac_start_addr + 5 * 512;
	cur_offset = lseek64(fd, offset, SEEK_SET);
	if (cur_offset != offset) {
		rec_err("seek fail for front rear type.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
	sizer = read(fd, chip_buf, 512);
	if (sizer != 512) {
		rec_err("read fail for fr_type.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}

	rec_info("\nFR Type: %d\n", chip_buf[0]);
	/*
	* metazone 
	* Index---------data size offset-----data offset
	* 0x10031------NA---------------0x80C4
	*/
	offset = mz_pt_addr + FRCHIP_DATA_OFFSET;
#ifdef CONFIG_BOOT_MMC
	cur_offset = lseek64(fdev, offset, SEEK_SET);
	if (cur_offset != offset) {
		rec_err("seek fail for fr-type.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
	sizew = write(fdev, &chip_buf[0], sizeof(u16));
	if (sizew != sizeof(u16)) {
		rec_err("write fail for fr-type in metazone.\n");
		ret = -ESYSCALL;
		goto out_close_fdev;
	}
#else /* !CONFIG_BOOT_MMC */
	offset = FRCHIP_DATA_OFFSET;
	ret = nand_raw_partition_write_offset_by_emptycore(&chip_buf[0],
			    mz_pt_addr, offset,
			    sizeof(u16), mz_real_size,
			    mz_pt_size);
	if (ret < 0) {
		rec_err("nand_raw_common_write_with_rwctrl fail -- 8\n");
		ret = -ENANDWR;
		goto out_close_fdev;
	}
#endif /* CONFIG_BOOT_MMC */

	ret = 0;
	rec_info("write wifi/bt/gps mac addres/chip type into metazone done.\n");

out_close_fdev:
#ifdef CONFIG_BOOT_MMC
	close(fdev);
#else
#if 0
	ret = nand_rw_end();
#endif
#endif

out_close_fd:
	close(fd);
out:
	return ret;
}

static int emptycore_upgrade_from_sdcard(RecoveryUpdateModule *prum,
	    int bootdevice, u64 img_start_addr)
{
	int fd = 0,fdsd = 0;
	int n = 0;
	int ret = 0;
	int idx = 0;
	char bd_str[24] = {0};
	char buf[PTBL_BLOCK_SIZE] = {0};
	unsigned long long emmc_size;
	off64_t cur_offset64 = 0;
	uint32_t chksum = 0;
	partitionread *partread_cur= NULL;
	partitionread *ptbl_dz = NULL;
	partitionread *ptbl_dz_bk = NULL;
	partitionread *ptbl_ub_bk = NULL;
	partitionread *ptbl_mz = NULL;
	partitioninfo *partinfo;
	struct upg_one_part_arg_t arg = {0};
	struct datazone_info dz;
	struct safeupg_partitionhead head;
	struct safeupg_bootloader_message bcb;
#ifdef CONFIG_BOOT_MMC
	/* emmc */
	const char *devorpart_name = devname;
#else
	/* nand */
	const char *devorpart_name = NULL;
	uint32_t chksum_sdcard = 0;
	uint32_t chksum_nand = 0;
	unsigned long long realsize = 0;
#endif

	if (prum == NULL)
		return -EINVAL;

	if ((bootdevice == 1) || (bootdevice == 2)) {
		snprintf(bd_str, 24, "slot%d", bootdevice);
		rec_info("bootdevice is %s \n", bd_str);
	} else {
		rec_err("boot device number(%d) is wrong.\n", bootdevice);
		ret = -EBADBOOTDEV;
		goto out;
	}
#ifdef CONFIG_BOOT_MMC
	fdsd = open("/dev/mmcblk1", O_RDONLY | O_LARGEFILE, 0);
	if (fdsd > 0) {
		close(fdsd);
		strcpy(bd_str, "/dev/mmcblk1");
		strncpy(g_bd_dev_node, "/dev/mmcblk1", 24);
	}
#else
	fdsd = open("/dev/mmcblk0", O_RDONLY | O_LARGEFILE, 0);
	if (fdsd > 0) {
		close(fdsd);
		strcpy(bd_str, "/dev/mmcblk0");
		strncpy(g_bd_dev_node, "/dev/mmcblk0", 24);
	}

#endif

	fd = open(bd_str, O_RDWR | O_LARGEFILE);
	if (fd < 0) {
		rec_err("Open block device(%s) failed.\n", bd_str);
		ret = -ESYSCALL;
		goto out;
	}
#ifndef CONFIG_BOOT_MMC
	ret = nand_clear_all_protect();
	if( ret < 0) {
		rec_err("nand_clear_all_protect fail.\n");
		goto out_close_fd;
	}

	ret = get_verify_image_flag(fd, img_start_addr - BLOCK_SIZE);
	if( ret < 0) {
		rec_err("get verify image flag failed.\n");
		goto out_close_fd;
	}
#endif
	pthread_mutex_lock(&g_hotplug_mutex);
	g_hotplug_status = 1;
	pthread_mutex_unlock(&g_hotplug_mutex);

	emptycore_calc_total_size_need_upgrade_sdcard(prum, fd, img_start_addr);

	arg.prum = prum;
	arg.fd =fd;

	do {
		cur_offset64 = lseek64(fd, img_start_addr, SEEK_SET);
		if (cur_offset64 != img_start_addr) {
			rec_err("lseek64 fails, img_start_addr=0x%llx\n", img_start_addr);
			ret = -ESYSCALL;
			goto out_free_tbl;
		}

		n = read(fd, buf, 512);
		if (n != 512) {
			rec_err("block_read fail, result=%d\n", n);
			ret = -ESYSCALL;
			goto out_free_tbl;
		}

		partinfo = (partitioninfo *)buf;
		print_partitioninfo(partinfo);

		partread_cur = mergepartitioninfo(partinfo);
		if (partread_cur == NULL) {
			rec_err("partread_cur is NULL.\n");
			ret = -1;
			goto out_free_tbl;
		}

		if (newtblhead == NULL)
			newtblhead = partread_cur;
		else
			newtblcur->nextpartition = partread_cur;

		newtblcur = partread_cur;

		#ifdef CONFIG_USRDATA_EXT4
		#ifdef CONFIG_BOOT_MMC
		char ptname[16] = {0};
		strcpy(ptname, "usrdata");
		if ((strcmp(partread_cur->szPartName, ptname) == 0) && (strcmp(partread_cur->szType, "ext4") == 0)) {
			ret = get_emmc_total_size(&emmc_size);
			if (ret < 0) {
				rec_err("get emmc total size failed.\n");
				ret = -EEMMCSIZE;
				goto out_free_tbl;
			}
			if (emmc_size < partread_cur->u8PartitionStartAddr) {
				rec_err("emmc_size less than userdate.startaddress\n");
				ret = -EPARTTBL;
				goto out_free_tbl;
			} else if(emmc_size < partread_cur->u8PartitionStartAddr + partread_cur->u8PartitionSize) {
				rec_err("emmc_size less than userdate.startaddress+usrdate.size\n");
				ret = -EPARTTBL;
				goto out_free_tbl;
			} else if(emmc_size < partread_cur->u8PartitionStartAddr+(50+2)*512) {
				rec_err("emmc_size less than userdate.startaddress+(50+2)*512)\n");
				ret = -EPARTTBL;
				goto out_free_tbl;
			} else {
				partread_cur->u8PartitionSize = emmc_size - partread_cur->u8PartitionStartAddr - (50+2)*512;
			}
		}
		#endif /* CONFIG_BOOT_MMC */
		#endif /* CONFIG_USRDATA_EXT4 */

		arg.part = partread_cur;
		arg.img_offset = img_start_addr + 512;

		#ifndef CONFIG_BOOT_MMC
		if (verify_image_flag) {
			/* 
			* nand, calc image's checksum in sdcard before taking logo upgrade
			*/
			rec_info("it's <%s> partition, calc checksum from sdcard.\n",
				    partread_cur->szPartName);
			realsize = partread_cur->u8RealDataSize;
			chksum_sdcard = calc_checksum_from_sdcard_before_upg(fd,
				    img_start_addr + 512, realsize, partread_cur);
			rec_info("chksum_sdcard=0x%x\n", chksum_sdcard);
		}
		#endif

		ret = emptycore_upgrade_one_partition(&arg, 0);
		if (ret < 0) {
			rec_err("emptycore_upgrade_from_ext_sdcard, ret=%d\n", ret);
			goto out_free_tbl;
		}

		#ifndef CONFIG_BOOT_MMC
		if (verify_image_flag) {
			/*
			* nand, calc image's checksum in nand, image is already upgrade into nand by above
			* emptycore_upgrade_one_partition.
			*/
			rec_info("it's <%s> partition, calc checksum from nand.\n",
				    partread_cur->szPartName);
			chksum_nand = calc_checksum_from_nand_after_upg(
				    partread_cur->u8PartitionStartAddr,
				    realsize,
				    partread_cur,
				    0);
			rec_info("chksum_nand=0x%x\n", chksum_nand);
			if (chksum_sdcard == 0 && chksum_nand == 0) {
				rec_warn("chksum_sdcard and chksum_nand are 0.\n");
			}
			if (chksum_sdcard != chksum_nand)
				rec_err("checksum compare error, chksum_sdcard(0x%x), chksum_nand(0x%x)\n",
					    chksum_sdcard, chksum_nand);
		}
		#endif

		img_start_addr = partinfo->u8OffsetNextImage;
	} while(partinfo->u8OffsetNextImage != 0);

	rec_info("*******************emptycore emmc partition table begin******************\n");
	dumpallpartitioninfo(newtblhead);
	rec_info("*******************emptycore emmc partition table end******************\n");

	//writepartitioninfotoflash(newtblhead);

	ptbl_mz = lookup_partition_by_name(newtblhead, "metazone");
	if (ptbl_mz == NULL) {
		rec_warn("can't lookup metazone partition.\n");
		metazone_pt_addr = 0;
		metazone_pt_size = 0;
		metazone_real_size = 0;
	} else {
		metazone_pt_addr = ptbl_mz->u8PartitionStartAddr;
		metazone_pt_size = ptbl_mz->u8PartitionSize;
		metazone_real_size = ptbl_mz->u8RealDataSize;
	}

	ptbl_dz = lookup_partition_by_name(newtblhead, "datazone");
	if (ptbl_dz == NULL) {
		rec_err("can't lookup datazone partition.\n");
		ret = -EPARTITION;
		goto out_free_tbl;
	}

	ptbl_dz_bk = lookup_partition_by_name(newtblhead, "datazone_bk");
	if (ptbl_dz_bk == NULL) {
		rec_err("can't lookup datzone_bk partition.\n");
		ret = -EPARTITION;
		goto out_free_tbl;
	}

	if (ptbl_dz->u8PartitionStartAddr != DATAZONE_MAIN_OFFSET_FROM_MMCBLK) {
		rec_err("datazone startaddress(0x%lx) != (0x%lx)\n", ptbl_dz->u8PartitionStartAddr, DATAZONE_MAIN_OFFSET_FROM_MMCBLK);
	}

	if (ptbl_dz_bk->u8PartitionStartAddr != DATAZONE_BK_OFFSET_FROM_MMCBLK) {
		rec_err("datazone_bk startaddress(0x%lx) != (0x%lx)\n", ptbl_dz_bk->u8PartitionStartAddr, DATAZONE_BK_OFFSET_FROM_MMCBLK);
	}

/*
* write partition info main
*/
	memset(&head, 0, sizeof(struct safeupg_partitionhead));
#ifdef CONFIG_BOOT_MMC
	ret = write_partition_info(&head, newtblhead, devorpart_name,
		    ptbl_dz->u8PartitionStartAddr + PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN);
#else
	ret = write_partition_info(&head, newtblhead, devorpart_name,
		    (unsigned long)ptbl_dz);
#endif
	if (ret < 0) {
		rec_err("write main partition info fail.\n");
		goto out_free_tbl;
	}

#ifdef CONFIG_BOOT_MMC
	ret = partition_info_readback_check(devorpart_name,
		    ptbl_dz->u8PartitionStartAddr + PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN);
#else
	ret = partition_info_readback_check(devorpart_name,
		    (unsigned long)ptbl_dz);
#endif
	if (ret == 0) {
		rec_err("main partition readback check fail.\n");
		goto out_free_tbl;
	}

/*
* write partition info bk
*/
#ifdef CONFIG_BOOT_MMC
	ret = write_partition_info(&head, newtblhead, devorpart_name,
		    ptbl_dz_bk->u8PartitionStartAddr + PARTITION_INFO_BK_OFFSET_FROM_DATAZONE_BK);
#else
	ret = write_partition_info(&head, newtblhead, devorpart_name,
		    (unsigned long)ptbl_dz_bk);
#endif
	if (ret < 0) {
		rec_err("write bk partition info fail.\n");
		goto out_free_tbl;
	}

#ifdef CONFIG_BOOT_MMC
	ret = partition_info_readback_check(devorpart_name,
		    ptbl_dz_bk->u8PartitionStartAddr + PARTITION_INFO_BK_OFFSET_FROM_DATAZONE_BK);
#else
	ret = partition_info_readback_check(devorpart_name,
		    (unsigned long)ptbl_dz_bk);
#endif
	if (ret == 0) {
		rec_err("bk partition info readback check fail.\n");
		goto out_free_tbl;
	}

	ptbl_ub_bk = lookup_partition_by_name(newtblhead, "uboot_bk");
	if (ptbl_ub_bk == NULL) {
		rec_err("can't lookup uboot_bk partition.\n");
		ret = -EPARTITION;
		goto out_free_tbl;
	}

/*
* write datazone
*
* datazone is written into emmc above, but it only write rd_data.bin
* the checksum is NOT calc.
* Now, read datazone from emmc, then calc checksum, write checksum into emmc.
*/
	memset(&dz, 0, sizeof(struct datazone_info));
#ifdef CONFIG_BOOT_MMC
	ret = read_datazone(&dz, devorpart_name, ptbl_dz->u8PartitionStartAddr);
#else
	ret = read_datazone(&dz, devorpart_name, (unsigned long)ptbl_dz);
#endif
	if (ret < 0) {
		rec_err("read main datazone fail.\n");
		goto out_free_tbl;
	}

	adjust_datazone_img_desc_bk(&dz, ptbl_ub_bk);
	chksum = calc_datazone_checksum(&dz);
	put_datazone_checksum(&dz, chksum);
#ifdef CONFIG_BOOT_MMC
	ret = write_datazone(&dz, devorpart_name, ptbl_dz->u8PartitionStartAddr);
#else
	ret = write_datazone(&dz, devorpart_name, (unsigned long)ptbl_dz);
#endif
	if(ret < 0){
		rec_err("write main datazone fail.\n");
		goto out_free_tbl;
	}

#ifdef CONFIG_BOOT_MMC
	ret = datazone_readback_check(devorpart_name, ptbl_dz->u8PartitionStartAddr);
#else
	ret = datazone_readback_check(devorpart_name, (unsigned long)ptbl_dz);
#endif
	if (ret == 0) {
		rec_err("datazone readback_check fail.\n");
		goto out_free_tbl;
	}

	/*check partition*/
#ifdef CONFIG_BOOT_MMC
	ret = partition_info_readback_check(devorpart_name,
		    ptbl_dz->u8PartitionStartAddr + PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN);
#else
	ret = partition_info_readback_check(devorpart_name,
		    (unsigned long)ptbl_dz);
#endif
	if (ret == 0) {
		rec_err("second, main partition readback check fail after datazone upg.\n");
		goto out_free_tbl;
	}

/*
* write datazone_bk
*/
#ifdef CONFIG_BOOT_MMC
	ret = write_datazone(&dz, devorpart_name, ptbl_dz_bk->u8PartitionStartAddr);
#else
	ret = write_datazone(&dz, devorpart_name, (unsigned long)ptbl_dz_bk);
#endif
	if(ret < 0){
		rec_err("write bk databkzone fail.\n");
		goto out_free_tbl;
	}
	
#ifdef CONFIG_BOOT_MMC
	ret = datazone_readback_check(devorpart_name, ptbl_dz_bk->u8PartitionStartAddr);
#else
	ret = datazone_readback_check(devorpart_name, (unsigned long)ptbl_dz_bk);
#endif
	if (ret == 0) {
		rec_err("datazone_bk readback_check fail.\n");
		goto out_free_tbl;
	}

	/*check partition*/
#ifdef CONFIG_BOOT_MMC
	ret = partition_info_readback_check(devorpart_name,
		    ptbl_dz->u8PartitionStartAddr + PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN);
#else
	ret = partition_info_readback_check(devorpart_name,
		    (unsigned long)ptbl_dz);
#endif
	if (ret == 0) {
		rec_err("third main datezone bk after partition readback check fail after bk datazone upg.\n");
		goto out_free_tbl;
	}

/*
* write bcb_main
*/
	memset(&bcb, 0, sizeof(struct safeupg_bootloader_message));
	set_bcb_tags(&bcb);
	put_bcb_bootflag(&bcb, BOOTFLAG_STARTUP_A);
	chksum = calc_bcb_checksum(&bcb);
	put_bcb_checksum(&bcb, chksum);
#ifdef CONFIG_BOOT_MMC
	ret = write_bcb(&bcb, devorpart_name, ptbl_dz->u8PartitionStartAddr + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN);
#else
	ret = write_bcb(&bcb, devorpart_name, (unsigned long)ptbl_dz);
#endif
	if (ret < 0) {
		rec_err("write main bcb fail.\n");
		goto out_free_tbl;
	}
#ifdef CONFIG_BOOT_MMC
	bcb_readback_check(devorpart_name, ptbl_dz->u8PartitionStartAddr + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN);
#else
	bcb_readback_check(devorpart_name, (unsigned long)ptbl_dz);
#endif

	/*check partition*/
#ifdef CONFIG_BOOT_MMC
	ret = partition_info_readback_check(devorpart_name,
		    ptbl_dz->u8PartitionStartAddr + PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN);
#else
	ret = partition_info_readback_check(devorpart_name,
		    (unsigned long)ptbl_dz);
#endif
	if (ret == 0) {
		rec_err("fourth main partition readback check fail after bcb upg.\n");
		goto out_free_tbl;
	}

/*
* write bcb_bk
*/
#ifdef CONFIG_BOOT_MMC
	ret = write_bcb(&bcb, devorpart_name, ptbl_dz_bk->u8PartitionStartAddr + BCB_BK_OFFSET_FROM_DATAZONE_BK);
#else
	ret = write_bcb(&bcb, devorpart_name, (unsigned long)ptbl_dz_bk);
#endif
	if (ret < 0) {
		rec_err("write bk bcb fail.\n");
		goto out_free_tbl;
	}

#ifdef CONFIG_BOOT_MMC
	bcb_readback_check(devorpart_name, ptbl_dz_bk->u8PartitionStartAddr + BCB_BK_OFFSET_FROM_DATAZONE_BK);
#else
	bcb_readback_check(devorpart_name, (unsigned long)ptbl_dz_bk);
#endif
	/*partition main check*/
#ifdef CONFIG_BOOT_MMC
	ret = partition_info_readback_check(devorpart_name,
		    ptbl_dz->u8PartitionStartAddr + PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN);
#else
	ret = partition_info_readback_check(devorpart_name,
		    (unsigned long)ptbl_dz);
#endif
	if (ret == 0) {
		rec_err("after write all datazone  partition readback check fail.\n");
		goto out_free_tbl;
	}

	/*datazone main check*/
#ifdef CONFIG_BOOT_MMC
	ret = datazone_readback_check(devorpart_name, ptbl_dz->u8PartitionStartAddr);
#else
	ret = datazone_readback_check(devorpart_name, (unsigned long)ptbl_dz);
#endif
	if (ret == 0) {
		rec_err("after write all datazone datazone readback_check fail.\n");
		goto out_free_tbl;
	}

	/*bcb main check*/
#ifdef CONFIG_BOOT_MMC
	ret = bcb_readback_check(devorpart_name, ptbl_dz->u8PartitionStartAddr + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN);
#else
	ret = bcb_readback_check(devorpart_name, (unsigned long)ptbl_dz);
#endif
	if (ret == 0) {
		rec_err("after wirte all datazone partition readback_check fail.\n");
		goto out_free_tbl;
	}

    ret = UPDATED_SUCCESS;
    goto out_close_fd;

out_free_tbl:
    freetblmemory(newtblhead);
    newtblhead = NULL;
out_close_fd:
    close(fd);
out:
    return ret;
}

static int emptycore_upgrade_from_udisk(RecoveryUpdateModule *prum, partitionread *ptbl)
{
	int n = 0;
	int ret = 0;
	int idx = 0;
	char buf[PTBL_BLOCK_SIZE] = {0};
	long long emmc_size = 0;
	uint32_t chksum = 0;
	partitionread *ptbl_temp = NULL;
	partitionread *ptbl_dz = NULL;
	partitionread *ptbl_dz_bk = NULL;
	partitionread *ptbl_ub_bk = NULL;
	partitionread *ptbl_mz = NULL;
	struct upg_one_part_arg_t arg = {0};
	struct datazone_info dz;
	struct safeupg_partitionhead head;
	struct safeupg_bootloader_message bcb;

	pthread_mutex_lock(&g_hotplug_mutex);
	g_hotplug_status = 1;
	pthread_mutex_unlock(&g_hotplug_mutex);

	arg.prum = prum;
	ptbl_temp = ptbl;
	while (ptbl_temp) {
		arg.part = ptbl_temp;
		ret = emptycore_upgrade_one_partition(&arg, 1);
		if (ret < 0) {
			rec_err("upgrade partition %s fail.\n", ptbl_temp->szPartName);
			return ret;
		}
		ptbl_temp = ptbl_temp->nextpartition;
	}

	//writepartitioninfotoflash(newtblhead);

	ptbl_mz = lookup_partition_by_name(ptbl, "metazone");
	if (ptbl_mz == NULL) {
		rec_warn("can't lookup metazone partition.\n");
		metazone_pt_addr = 0;
		metazone_pt_size = 0;
		metazone_real_size = 0;
	} else {
		metazone_pt_addr = ptbl_mz->u8PartitionStartAddr;
		metazone_pt_size = ptbl_mz->u8PartitionSize;
		metazone_real_size = ptbl_mz->u8RealDataSize;
	}

	ptbl_dz = lookup_partition_by_name(ptbl, "datazone");
	if (ptbl_dz == NULL) {
		rec_err("can't lookup datazone partition.\n");
		ret = -EPARTITION;
		goto out_free_tbl;
	}

	ptbl_dz_bk = lookup_partition_by_name(ptbl, "datazone_bk");
	if (ptbl_dz_bk == NULL) {
		rec_err("can't lookup datzone_bk partition.\n");
		ret = -EPARTITION;
		goto out_free_tbl;
	}

	if (ptbl_dz->u8PartitionStartAddr != DATAZONE_MAIN_OFFSET_FROM_MMCBLK) {
		rec_err("datazone startaddress(0x%lx) != (0x%lx)\n", ptbl_dz->u8PartitionStartAddr, DATAZONE_MAIN_OFFSET_FROM_MMCBLK);
	}

	if (ptbl_dz_bk->u8PartitionStartAddr != DATAZONE_BK_OFFSET_FROM_MMCBLK) {
		rec_err("datazone_bk startaddress(0x%lx) != (0x%lx)\n", ptbl_dz_bk->u8PartitionStartAddr, DATAZONE_BK_OFFSET_FROM_MMCBLK);
	}

/*
* write partition info main
*/
	memset(&head, 0, sizeof(struct safeupg_partitionhead));
	ret = write_partition_info(&head, ptbl, devname,
		ptbl_dz->u8PartitionStartAddr + PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN);
	if (ret < 0) {
		rec_err("write main partition info fail.\n");
		goto out_free_tbl;
	}

/*
* write partition info bk
*/
	ret = write_partition_info(&head, ptbl, devname,
		ptbl_dz_bk->u8PartitionStartAddr + PARTITION_INFO_BK_OFFSET_FROM_DATAZONE_BK);
	if (ret < 0) {
		rec_err("write bk partition info fail.\n");
		goto out_free_tbl;
	}

	ptbl_ub_bk = lookup_partition_by_name(ptbl, "uboot_bk");
	if (ptbl_ub_bk == NULL) {
		rec_err("can't lookup uboot_bk partition.\n");
		ret = -EPARTITION;
		goto out_free_tbl;
	}

/*
* write datazone
*
* datazone is written into emmc above, but it only write rd_data.bin
* the checksum is NOT calc.
* Now, read datazone from emmc, then calc checksum, write checksum into emmc.
*/
	memset(&dz, 0, sizeof(struct datazone_info));
	ret = read_datazone(&dz, devname, ptbl_dz->u8PartitionStartAddr);
	if (ret < 0) {
		rec_err("read main datazone fail.\n");
		goto out_free_tbl;
	}

	adjust_datazone_img_desc_bk(&dz, ptbl_ub_bk);
	chksum = calc_datazone_checksum(&dz);
	put_datazone_checksum(&dz, chksum);
	ret = write_datazone(&dz, devname, ptbl_dz->u8PartitionStartAddr);
	if(ret < 0){
		rec_err("write main datazone fail.\n");
		goto out_free_tbl;
	}

/*
* write datazone_bk
*/
	ret = write_datazone(&dz,devname, ptbl_dz_bk->u8PartitionStartAddr);
	if(ret < 0){
		rec_err("write bk databkzone fail.\n");
		goto out_free_tbl;
	}

/*
* write bcb_main
*/
	memset(&bcb, 0, sizeof(struct safeupg_bootloader_message));
	set_bcb_tags(&bcb);
	put_bcb_bootflag(&bcb, BOOTFLAG_STARTUP_A);
	chksum = calc_bcb_checksum(&bcb);
	put_bcb_checksum(&bcb, chksum);
	ret = write_bcb(&bcb, devname, ptbl_dz->u8PartitionStartAddr + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN);
	if (ret < 0) {
		rec_err("write main bcb fail.\n");
		goto out_free_tbl;
	}
	bcb_readback_check(devname, ptbl_dz->u8PartitionStartAddr + BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN);

/*
* write bcb_bk
*/
	ret = write_bcb(&bcb, devname, ptbl_dz_bk->u8PartitionStartAddr + BCB_BK_OFFSET_FROM_DATAZONE_BK);
	if (ret < 0) {
		rec_err("write bk bcb fail.\n");
		goto out_free_tbl;
	}
	bcb_readback_check(devname, ptbl_dz_bk->u8PartitionStartAddr + BCB_BK_OFFSET_FROM_DATAZONE_BK);

	ret = 0;

out_free_tbl:
	//freetblmemory(ptbl);
out:
	return ret;
}

static struct pthread_arg_info_t {
	RecoveryUpdateModule *prum;
	int bootdevice;
	u64 img_start_addr;
	u64 mac_start_addr;
} pthread_arg_info;

static void *emptycore_upgrade_thread_fn(void *arg)
{
	struct pthread_arg_info_t *parg;
	int ret, fdwp;

	if (arg == NULL)
		return (void *)-1;

	parg = (struct pthread_arg_info_t *)arg;

	if (from_udisk) {
		ret = emptycore_upgrade_from_udisk(parg->prum, pxml_tbl);
	} else {
		ret = emptycore_upgrade_from_sdcard(parg->prum, parg->bootdevice, parg->img_start_addr);
	}
	if (ret < 0) {
		rec_err("emptycore upg fail.\n");
	} else {
		if (metazone_pt_addr && metazone_pt_size) {
			ret = emptycore_update_connsys_info(parg->bootdevice,
				    parg->mac_start_addr,
				    metazone_pt_addr, metazone_pt_size, metazone_real_size);
			if (ret < 0) {
				rec_err("emptycore upg wifi/bt/gps info fail\n");
			}
		}
	}

	pthread_mutex_lock(&g_upgrade_mutex);
	if (ret >= 0) {
		g_upgrade_status_self.upgrade_finish = 1;
	} else {
		g_upgrade_status_self.upgrade_finish = ret;
	}
	pthread_mutex_unlock(&g_upgrade_mutex);

#ifdef NEW_PARTITION_DESIGN
	fdwp = open_for_writeprotect();
	if (fdwp < 0) {
		rec_err("open_for_writeprotect fail\n");
		goto skip_wp;
	}

	if (from_udisk)
		newtblhead = pxml_tbl;
	update_writeprotect_region(fdwp, newtblhead);
	rec_info("after upgrade auto dump info:\n");
	dump_writeprotect_region(fdwp);
	close_for_writeprotect(fdwp);
skip_wp:
#endif
	if (newtblhead) {
		freetblmemory(newtblhead);
		newtblhead = NULL;
		pxml_tbl = NULL;
	}
	pthread_exit((void *)ret);
}

static int emptycore_upgrade_sdcard(RecoveryUpdateModule *prum)
{
	int ret = -1;
	int bootdevice = 0;
	unsigned long long img_start_addr = 0;
	unsigned long long mac_start_addr = 0;

	BUILD_BUG_ON(sizeof(struct datazone_info) != 512);

	if (prum == NULL)
		return -EINVAL;

	ret = get_cmdline();
	if (ret < 0) {
		rec_err("get cmdline failed!\n");
		return -ECMDLINE;
	}

	ret = parse_bootdevice(g_cmdline, &bootdevice);
	if (ret < 0) {
		rec_err("get boot device fail!\n");
		return -EBADBOOTDEV;
	}

	ret = parse_image_start_addr(g_cmdline, &img_start_addr);
	if (ret < 0) {
		rec_err("get image start address fail!\n");
		return -ECMDLINE;
	}

	ret = parse_mac_start_addr(g_cmdline, &mac_start_addr);
	if (ret < 0) {
		rec_err("get mac start address fail!\n");
		return -ECMDLINE;
	}

/*
*open it and erase emmc at recovery process
*/
#ifdef CONFIG_BOOT_MMC
	if (erase_emmc() < 0) {
		rec_err("erase emmc fail\n");
		return -EEMMCERASE;
	}
#else
#if 0
    if (erase_nand() < 0) {
        rec_err("erase nand fail\n");
        return -ENANDERASE;
    }
	#endif
#endif

	memset(&g_upgrade_status_self, 0, sizeof(struct partition_upgrade_status_t));
	memset(&g_upgrade_status_to_qt, 0, sizeof(struct partition_upgrade_status_t));

	pthread_arg_info.prum = prum;
	pthread_arg_info.bootdevice = bootdevice;
	pthread_arg_info.img_start_addr = (u64)img_start_addr;
	pthread_arg_info.mac_start_addr = (u64)mac_start_addr;

	ret = pthread_mutex_init(&g_hotplug_mutex, NULL);
	if (ret != 0) {
		rec_err("fail init hotplug mutex:%s!\n", strerror(errno));
		return -EPTHREAD;
	}

	ret = pthread_mutex_init(&g_upgrade_mutex, NULL);
	if (ret != 0) {
		rec_err("fail init mutex:%s!\n", strerror(errno));
		return -EPTHREAD;
	}
	ret = pthread_create(&g_upgrade_thread_tid, NULL, emptycore_upgrade_thread_fn, (void *)&pthread_arg_info);
	if (ret != 0) {
		rec_err("can't create thread:%s!\n", strerror(errno));
		return -EPTHREAD;
	}

	return 0;
}

static int emptycore_upgrade_udisk(RecoveryUpdateModule *prum)
{
	int ret = -1;
	unsigned long long mac_start_addr = 0;
	char xml_path[128];
	const char *pxml_file = NULL;
	int result = 0;
	int board_type = 0;
	int bootdevice = 0;

	BUILD_BUG_ON(sizeof(struct datazone_info) != 512);

	if (prum == NULL)
		return -EINVAL;

	result = mkdir(UDISK_ROOT, S_IRUSR | S_IWUSR | S_IXUSR);
	if ((result < 0) && ((errno != EEXIST))) {
		rec_err("[qy]mkdir failed,return error.\n");
		return -ESYSCALL;
	}
	if (export_safeupg_iso_file_exist()) {
		rec_info("iso file exist.\n");
	} else {
		rec_info("iso file not exist.\n");
		return -ENOFILE;
	}
	if((ret = export_safeupg_iso_file_md5_verify()) < 0) {
		rec_err("md5 check fail.\n");
		return ret;
	}else {
		rec_info("md5 check ok.\n");
	}

	snprintf(xml_path, 128, "%s%s", ISO_ROOT, XML_FILE);
	rec_info("xml_path:%s\n", xml_path);
	ret = check_is_file_exist(xml_path);
	if (!ret ) {
		rec_err("xml_path %s is not exsit .\n", xml_path);
		ret = -ENOFILE;
		goto out;
	}
	
	/*
	* get xml partition table from xml file.
	*/
	pxml_tbl = read_partition_info_from_xml_file(xml_path);
	if (!pxml_tbl) {
		rec_err("can NOT get xml table from xml file.\n");
		ret = -EPARTTBL;
		goto out;
	}
	ret = adjust_xml_partition_info_size(pxml_tbl, board_type);
	if (ret < 0) {
		rec_err("adjust_xml_partition_info_userdata_size fail.\n");
		goto out_free_xml_tbl;
	}
	rec_info("adjust_xml_partition_info_userdata_size success.\n");
	
	rec_info("===============print xml table begin===============\n");
	dumpallpartitioninfo(pxml_tbl);
	rec_info("===============print xml table end===============\n");

	ret = check_partition_overlap(pxml_tbl);
	if (ret) {
		rec_err("partition overlap.\n");
		ret = -EPARTTBL;
		goto out_free_xml_tbl;
	}

	ret = check_files_exist_for_upgrade(pxml_tbl, ISO_ROOT);
	if (ret == 0) {
		rec_err("there are some files NOT exist for upg.\n");
		ret = -ENOFILE;
		goto out_free_xml_tbl;
	}
	
	memset(&g_upgrade_status_self, 0, sizeof(struct partition_upgrade_status_t));
	memset(&g_upgrade_status_to_qt, 0, sizeof(struct partition_upgrade_status_t));

	ret = emptycore_calc_total_size_need_upgrade_udisk(prum, pxml_tbl);
	if (ret < 0) {
		rec_err("calc total size fail.\n");
		goto out_free_xml_tbl;
	}

	ret = get_cmdline();
	if (ret < 0) {
		rec_err("get cmdline failed!\n");
		ret = -ECMDLINE;
		goto out_free_xml_tbl;
	}

	ret = parse_bootdevice(g_cmdline, &bootdevice);
	if (ret < 0) {
		rec_err("get boot device fail!\n");
		ret = -EBADBOOTDEV;
		goto out_free_xml_tbl;
	}

	ret = parse_mac_start_addr(g_cmdline, &mac_start_addr);
	if (ret < 0) {
		rec_err("get mac start address fail!\n");
		ret = -ECMDLINE;
		goto out_free_xml_tbl;
	}

	/*
	*open it and erase emmc at recovery process
	*/
#ifdef CONFIG_BOOT_MMC
	if (erase_emmc() < 0){
		rec_err("erase emmc fail\n");
		ret = -EEMMCERASE;
		goto out_free_xml_tbl;
	} else {
		rec_info("erase emmc success.\n");
	}
#else
	#if 0
	if (erase_nand() < 0){
		rec_err("erase nand fail\n");
		ret = -ENANDERASE;
		goto out_free_xml_tbl;
	} else {
		rec_info("erase nand success.\n");
	}
	#endif
#endif

	pthread_arg_info.prum = prum;
	pthread_arg_info.bootdevice = bootdevice;
	pthread_arg_info.mac_start_addr = (u64)mac_start_addr;

	ret = pthread_mutex_init(&g_hotplug_mutex, NULL);
	if (ret != 0) {
		rec_err("fail init hotplug mutex:%s!\n", strerror(errno));
		ret = -EPTHREAD;
		goto out_free_xml_tbl;
	}

	ret = pthread_mutex_init(&g_upgrade_mutex, NULL);
	if (ret != 0) {
		rec_err("fail init mutex:%s!\n", strerror(errno));
		ret = -EPTHREAD;
		goto out_free_xml_tbl;
	}

	ret = pthread_create(&g_upgrade_thread_tid, NULL, emptycore_upgrade_thread_fn, (void *)&pthread_arg_info);
	if (ret != 0) {
		rec_err("can't create thread:%s!\n", strerror(errno));
		ret = -EPTHREAD;
		goto out_free_xml_tbl;
	}

	return 0;

out_free_xml_tbl:
	freetblmemory(pxml_tbl);
	pxml_tbl = NULL;
out:
	return ret;

}

static RecoveryUpdateModule rum;

int export_emptycore_upgrade(RecoveryUpdateModule *prum)
{
	if (prum == NULL)
		return -EINVAL;

	memcpy(&rum, prum, sizeof(RecoveryUpdateModule));
	prum = &rum;

	rec_info("prum->mMcu=%d\n", prum->mMcu);
	rec_info("prum->mNavi=%d\n", prum->mNavi);
	//rec_info("prum->mUserdata=%d\n", prum->mUserdata);
	//rec_info("prum->mSystemdata=%d\n", prum->mSystemdata);
	rec_info("prum->mFromudisk=%d\n", prum->mFromudisk);

	if (hotplug_monitor_init(prum->mFromudisk) < 0) {
		rec_err("monitor init fail\n");
		return -1;
	}

	from_udisk_monitor = 1;
	if (prum->mFromudisk) {
		from_udisk = 1;
		rec_info("---->upgrade from udisk<----\n");
		return emptycore_upgrade_udisk(prum);
	} else {
		from_udisk = 0;
		rec_info("---->upgrade from sdcard<----\n");
		return emptycore_upgrade_sdcard(prum);
	}
}

