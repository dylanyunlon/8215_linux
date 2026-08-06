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
#include <dirent.h>
#include "atc_emptycore_upgrade.h"
#include "atc_update.h"
#include "recovery.h"
#include "blkwd.h"
#include "atc_upgrade_common.h"
#include "atc_safe_upgrade.h"
#include "md5.h"
#include "roots.h"
#include "checksum.h"
#include "nandupgrade.h"
#include "mz.h"
#include "mz_os.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"
#include "mz_strm_mem.h"

#define BOOT_CFG_PATH "/sys/block/mmcblk0/boot_part_cfg"

#define MAIN_STARTUP_A_FIRST_PART_NAME "uboot"
#define MAIN_STARTUP_A_LAST_PART_NAME "recovery"
#define MAIN_STARTUP_B_FIRST_PART_NAME "uboot_bk"
#define MAIN_STARTUP_B_LAST_PART_NAME "recovery_bk"
#define SYSTEM_AREA_FIRST_PART_NAME "system"
#define MMCBLK_BOOT_DEVNNODE_NAME 32

#define WHENCE_MAIN 0
#define WHENCE_BACKUP 1
#define DIRECTION_A2B 0
#define DIRECTION_B2A 1
#define MAIN_STARTUP_A 0
#define MAIN_STARTUP_B 1
#define WHENCE_PRELOADER_1 0
#define WHENCE_PRELOADER_2 1

#define DATAZONE_OFFSET_FROM_DATAZONE 0x0
#define BCB_OFFSET_FROM_DATAZONE 0x1000
#define PARTITION_OFFSET_DATAZONE 0x2000

#define ISO_FILE_NAME_LEN 128
static char iso_file_name[ISO_FILE_NAME_LEN];
static int safeupg_vfy_image_flag;

#define UDISK_DEVNODE_PATH_LEN 24
static char udisk_devnode_path[UDISK_DEVNODE_PATH_LEN];

static struct wd_name wd = {-1, -1, "/dev/"};
static pthread_mutex_t g_udisk_hotplug_mutex;
static int g_udisk_hotplug_status = 0;

static pthread_t g_safeupg_thread_tid;
static pthread_mutex_t g_safeupg_mutex;
static struct partition_upgrade_status_t g_safeupg_status_self;
static struct partition_upgrade_status_t g_safeupg_status_to_qt;

#define VERSION_LEN 48
static char etc_version[VERSION_LEN];
static char iso_file_version[VERSION_LEN];

static const char *startup_a_part_name[] = {
	"uboot",
	"trustzone",
	"arm2",
	"kernel",
	"dtb",
	"recovery"};

static const char *startup_b_part_name[] = {
	"uboot_bk",
	"trustzone_bk",
	"arm2_bk",
	"kernel_bk",
	"dtb_bk",
	"recovery_bk"};

#define STARTUP_PART_NUM ARRAY_SIZE(startup_a_part_name)

static const char *coreprog_tbl_name[] = {
	"preloader",
#ifndef CONFIG_BOOT_MMC
	"preloader_bk",
#endif
	"datazone",
	"datazone_bk",
	"uboot",
	"trustzone",
	"arm2",
	"dtb",
	"logo",
	"boot_misc",
	"vba",
	"metazone",
	"kernel",
	"system",
	"usrdata",
	"recovery",
	NULL};

static const char *systemprog_tbl_name[] = {
#ifdef CONFIG_BOOT_MMC
	"system",
	"systembk",
#else
	"system",
#endif
	NULL};

static const char *app_tbl_name[] = {
#ifdef CONFIG_BOOT_MMC
	"appbk",
	"app",
#else
	"app_ext4",
#endif
	NULL};

static const char *userdata_tbl_name[] = {
	"usrdata",
	NULL};

/*
 * export_udisk_hotplug_monitor_init
 * return 0 if init success, otherwise -1.
 */
int export_safeupg_udisk_hotplug_monitor_init(void)
{
	int fd = 0;
	int ret = -1;
	int i = 0;
	char devnode[16] = {0};

	fd = open("/dev/sda", O_RDONLY, 0);
	if (fd > 0)
	{
		close(fd);
		g_udisk_hotplug_status = 1;
		hotplug_device_tune(1, "/dev/sda");
	}
	for (i = 1; i < 10; i++)
	{
		snprintf(devnode, 16, "/dev/sda%d", i);
		fd = open(devnode, O_RDONLY, 0);
		if (fd > 0)
		{
			close(fd);
			g_udisk_hotplug_status = 1;
			hotplug_device_tune(1, devnode);
		}
	}
	if ((ret = init_blkwd(&wd)) < 0)
	{
		rec_err("init_blkwd fail.\n");
		return ret;
	}
	return 0;
}

/**
 * return 0 if plug-out
 * return 1 if plug-in
 */
int export_safeupg_get_udisk_hotplug(void)
{
	int ret;

	ret = blkwd_event(&wd);
	pthread_mutex_lock(&g_udisk_hotplug_mutex);
	if (ret == UDISKPLUGOUT)
	{
		g_udisk_hotplug_status = 0;
	}
	else if (ret == UDISKPLUGIN)
	{
		g_udisk_hotplug_status = 1;
	}
	pthread_mutex_unlock(&g_udisk_hotplug_mutex);

	return g_udisk_hotplug_status;
}

int export_safeupg_put_udisk_hotplug(void)
{
	pthread_mutex_lock(&g_udisk_hotplug_mutex);
	g_udisk_hotplug_status = 0;
	pthread_mutex_unlock(&g_udisk_hotplug_mutex);
	destory_blkwd(&wd);

	return 0;
}

struct partition_upgrade_status_t *export_safeupg_get_upg_status(void)
{
	if (pthread_mutex_trylock(&g_safeupg_mutex) == 0)
	{
		/*
		 * pthread_mutex_trylock will NOT block qt.
		 * if return value is 0, take this opportunity to copy upgrade status
		 * from safeupg_status_self to safeupg_status_to_qt.
		 */
		memcpy(&g_safeupg_status_to_qt, &g_safeupg_status_self, sizeof(struct partition_upgrade_status_t));
		pthread_mutex_unlock(&g_safeupg_mutex);
	}

	if (g_safeupg_status_to_qt.total_size_need_upgrade != 0)
		g_safeupg_status_to_qt.progress_percent = g_safeupg_status_to_qt.size_upgrade_done * 100 / g_safeupg_status_to_qt.total_size_need_upgrade;

	return &g_safeupg_status_to_qt;
}

/*
 * this function is called by qt to get which partition is ongoing in upgrade.
 */
char *export_safeupg_get_part_upgrade_ongoing(void)
{
	if (pthread_mutex_trylock(&g_safeupg_mutex) == 0)
	{
		/*
		 * pthread_mutex_trylock will NOT block qt.
		 * if return value is 0, take this opportunity to copy upgrade status
		 * from safeupg_status_self to safeupgupgrade_status_to_qt.
		 */
		memcpy(&g_safeupg_status_to_qt, &g_safeupg_status_self, sizeof(struct partition_upgrade_status_t));
		pthread_mutex_unlock(&g_safeupg_mutex);
	}

	return g_safeupg_status_to_qt.upgrade_ongoing_part_name;
}

int export_safeupg_get_upgrade_progress(void)
{
	if (pthread_mutex_trylock(&g_safeupg_mutex) == 0)
	{
		/*
		 * pthread_mutex_trylock will NOT block qt.
		 * if return value is 0, take this opportunity to copy upgrade status
		 * from safeupg_status_self to safeupg_status_to_qt.
		 */
		memcpy(&g_safeupg_status_to_qt, &g_safeupg_status_self, sizeof(struct partition_upgrade_status_t));
		pthread_mutex_unlock(&g_safeupg_mutex);
	}

	if (g_safeupg_status_to_qt.total_size_need_upgrade == 0 && g_safeupg_status_to_qt.upgrade_finish == 1)
	{
		rec_info("safe upgrade success 100%\n");
		return 100;
	}
	if (g_safeupg_status_to_qt.upgrade_finish == 1)
	{
		rec_info("need upgrade safe upgrade success 100%\n");
		return 100;
	}
	if (g_safeupg_status_to_qt.total_size_need_upgrade == 0)
	{
		return 0;
	}

	g_safeupg_status_to_qt.progress_percent = g_safeupg_status_to_qt.size_upgrade_done * 100 / g_safeupg_status_to_qt.total_size_need_upgrade;
	rec_info("safe upgrade success %d\n", g_safeupg_status_to_qt.progress_percent);

	return g_safeupg_status_to_qt.progress_percent;
}

static int iso_mount(char *iso_file, char *mount_point)
{
	int res = -1;

	if (mount_point == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	if (iso_file && mount_point)
	{
		rec_info("iso_file: %s, mount_point:%s\n", iso_file, mount_point);
	}
	else
	{
		rec_err("arguments invalid.\n");
		return -EINVAL;
	}

#if 0
	res = mount(iso_file, mount_point, "iso9660", MS_RDONLY, NULL);

	return res;
#else

	pid_t pid = fork();
	if (pid == 0)
	{
		char *argv[] = {"/bin/mount", "--loop", (char *)NULL, (char *)NULL, (char *)NULL};
		argv[2] = iso_file;
		argv[3] = mount_point;
		execv(argv[0], argv);
		// rec_err("Can't run %s (%s)\n", argv[0], strerror(errno));
		_exit(-1);
	}

	int status;
	waitpid(pid, &status, 0);
	return status;

#endif
}

static int findstr_in_tail(const char *str, const char *s)
{
	int dlen, slen;

	if (str == NULL || s == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}

	dlen = strlen(str);
	slen = strlen(s);

	if (dlen < slen)
		return 0;

	if (strcmp(str + dlen - slen, s) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static int search_iso_file(const char *dirname)
{
	int found = 0;
	DIR *dir;
	struct dirent *entry;

	if (dirname == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}

	dir = opendir(dirname);
	if (dir == NULL)
	{
		rec_err("opendir %s failed\n", dirname);
		return 0;
	}

	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_type == DT_REG)
		{
			// if (strstr(entry->d_name, ".iso")) {
			if (findstr_in_tail(entry->d_name, ".iso"))
			{
				rec_info("found iso:%s\n", entry->d_name);
				snprintf(iso_file_name, ISO_FILE_NAME_LEN, "%s/%s", dirname, entry->d_name);
				rec_info("iso path:%s\n", iso_file_name);
				found = 1;
				break;
			}
		}
	}
	closedir(dir);

	return found;
}

/*
 * export_safeupg_iso_file_exist
 * return 1 if iso file exist, otherwise 0.
 */
int export_safeupg_iso_file_exist(void)
{
	int res = 0;
	char iso_file_path[ISO_FILE_NAME_LEN] = {0};
	char buf[512] = {0};
	FILE *fp = NULL;
	char *str = NULL;

	res = udisk_mount(UDISK_MOUNT_POINT);
	if (res < 0)
	{
		rec_info("fat mount fail and begin to mount ntfs \n");
		res = ntfs_mount(ntfs_mount_device, UDISK_ROOT);
		if (res < 0)
		{
			rec_err("udisk ntfs not mount.\n");
			return 0;
		}
		rec_info("udisk ntfs mount success.\n");
	}

	fp = fopen(MD5_FILE, "r");
	if (fp == NULL)
	{
		rec_err("iso.md5 file NOT find.\n");
		return 0;
	}

	if (fgets(buf, sizeof(buf), fp) == NULL)
	{
		rec_err("fgets fail.\n");
		fclose(fp);
		return 0;
	}
	fclose(fp);

	str = strstr(buf, "linux_");
	if (str == NULL)
	{
		rec_err("iso.md5 NOT include linux keywords.\n");
		return 0;
	}

	strncpy(iso_file_path, UDISK_ROOT, ISO_FILE_NAME_LEN);
	strcat(iso_file_path, str);
	str = strstr(iso_file_path, ".iso");
	if (strlen(str) > 4)
		*(str + 4) = '\0';
	rec_info("iso_file_path:%s,iso_file_path length:%d\n", iso_file_path, strlen(iso_file_path));

	fp = fopen(iso_file_path, "r");
	if (fp == NULL)
	{
		rec_err("%s file NOT find.\n", iso_file_path);
		return 0;
	}
	fclose(fp);
	strncpy(iso_file_name, iso_file_path, ISO_FILE_NAME_LEN - 1);
	rec_info("iso_file_name =%s\n", iso_file_name);
	iso_mount(iso_file_name, ISO_MOUNT_POINT);
	rec_info("iso_mount\n");
	return 1;
}

/**
 * export_safeupg_iso_file_md5_verify
 * return 0 if md5 verify pass, otherwise -1.
 */

int export_safeupg_iso_file_md5_verify(void)
{
	int i = 0;
	int ret = -1;
	char iso_file_path[128] = {0};
	char buf[512] = {0};
	FILE *fp = NULL;
	char *str = NULL;
	unsigned char md5[MD5_LENGTH];
	unsigned char md5_str[MD5_LENGTH * 2];
	unsigned char md5_expect[MD5_LENGTH * 2];

	memset(md5, 0, MD5_LENGTH);
	memset(md5_expect, 0, MD5_LENGTH);

	fp = fopen(MD5_FILE, "r");
	if (fp == NULL)
	{
		rec_err("md5 file NOT find.\n");
		return -ENOFILE;
	}

	if (fgets(buf, sizeof(buf), fp) == NULL)
	{
		rec_err("fgets fail.\n");
		fclose(fp);
		return -ESYSCALL;
	}
	fclose(fp);

	rec_info("expect MD5:");
	for (i = 0; i < MD5_LENGTH * 2; i++)
	{
		md5_expect[i] = buf[i];
		printf("%c", md5_expect[i]);
	}
	printf("\n");

	str = strstr(buf, "linux_");
	if (str == NULL)
	{
		rec_err("iso.md5 NOT include linux keywords.\n");
		return -EINVAL;
	}

	strncpy(iso_file_path, UDISK_ROOT, 128);
	strcat(iso_file_path, str);
	str = strstr(iso_file_path, ".iso");
	if (strlen(str) > 4)
		*(str + 4) = '\0';
	rec_info("iso_file_path:%s,iso_file_path length:%d\n", iso_file_path, strlen(iso_file_path));

	if ((ret = GetMD5ByFile(iso_file_path, md5)) < 0)
	{
		rec_err("generate md5 from file fail!\n");
		return ret;
	}

	for (i = 0; i < MD5_LENGTH; i++)
	{
#if 0
		sprintf((char *)(md5_str + 2 * i), "%02x", md5[i]);
#endif
		md5_str[i * 2] = (((md5[i] >> 4) & 0xF) >= 10) ? (((md5[i] >> 4) & 0xF) - 10) + 'a' : ((md5[i] >> 4) & 0xF) + '0';
		md5_str[i * 2 + 1] = ((md5[i] & 0xF) >= 10) ? ((md5[i] & 0xF) - 10) + 'a' : (md5[i] & 0xF) + '0';
	}

	rec_info("generate MD5:");
	for (i = 0; i < MD5_LENGTH * 2; i++)
	{
		printf("%c", md5_str[i]);
	}
	printf("\n");

	for (i = 0; i < MD5_LENGTH * 2; i++)
	{
		if (md5_expect[i] != md5_str[i])
		{
			rec_info("i=%d,md5_expect[%d]=%c,md5_str[%d]=%c\n", i, i, md5_expect[i], i, md5_str[i]);
			rec_err("check fail.\n");
			return -EBADMD5;
		}
	}
	rec_info("check success.\n");

	return 0;
}

/*
 * get_iso_file_version -- get iso file version from MD5 file context.
 * return 0 if get successfully, else return -EBADVERSION
 */
static int get_iso_file_version(void)
{
	int ret = -1;
	int idx = 0;
	int pos = 0;
	FILE *fd = NULL;
	char *str_left = NULL;
	char *str_right = NULL;
	char buf[512] = {0};

	fd = fopen(MD5_FILE, "r");
	if (fd == NULL)
	{
		rec_err("iso.md5 file NOT find.\n");
		return 0;
	}
	if (fgets(buf, sizeof(buf), fd) == NULL)
	{
		rec_err("fgets fail.\n");
		fclose(fd);
		return -EBADVERN;
	}
	fclose(fd);
	str_left = strstr(buf, "linux_");
	if (str_left == NULL)
	{
		rec_err("iso.md5 NOT include <linux> keywords.\n");
		return -EBADVERN;
	}
	rec_info("iso version: (%s)\n", str_left);

	str_right = strstr(buf, ".iso");
	if (str_right == NULL)
	{
		rec_err("iso.md5 NOT include <.iso> keywords.\n");
		return -EBADVERN;
	}

	idx = 0;
	str_left += strlen("linux_");
	while ((str_left + idx < str_right) && (idx < VERSION_LEN - 1))
	{
		iso_file_version[idx] = *(str_left + idx);
		idx++;
	}
	iso_file_version[idx] = '\0';

	rec_info("iso file version is:%s\n", iso_file_version);

	return 0;
}

static int get_etc_version(void)
{
	int fd;
	int ret;
	int idx = 0;
	char buf[64];

	memset(buf, 0, 64);

	fd = open("/etc/version", O_RDONLY);
	if (fd <= 0)
	{
		rec_err("open /etc/version fail,%s.\n", strerror(errno));
		return -EBADVERN;
	}

	ret = read(fd, buf, 64);
	if (ret <= 0)
	{
		rec_err("read /etc/version fail,%s.\n", strerror(errno));
		close(fd);
		return -EBADVERN;
	}
	else
	{
		rec_info("read /etc/version ret=%d\n", ret);
	}

	close(fd);

	rec_info("version: (%s)\n", buf);
	idx = 0;
	while ((buf[idx] != '\0' || buf[idx] != '\n') && (idx < (VERSION_LEN - 1)))
	{
		etc_version[idx] = buf[idx];
		idx++;
	}
	etc_version[idx] = '\0';

	rec_info("make build time is (%s) idx (%d)\n", etc_version, idx);
	if (idx != 12) // 201707241756 len is 12.
		rec_err("etc_version length is NOT expect, check it.\n");

	return 0;
}

/*
 * version_match -- compare iso file version with /etc/version.
 * return 0 if match, otherwise -EXXXX.
 */
static int version_match(void)
{
	int ret = 0;

	ret = get_iso_file_version();
	if (ret < 0)
	{
		rec_err("get iso file version fail.\n");
		return ret;
	}

	ret = get_etc_version();
	if (ret < 0)
	{
		rec_err("get build version fail.\n");
		return ret;
	}

	if (strcmp(iso_file_version, etc_version))
	{
		// version NOT match.
		return -EVERNOMATCH;
	}

	return 0;
}

/*
 * get_file_length() - get file total size.
 * return file size, -1 means fail.
 */
static int get_file_length(const char *file, unsigned long long *plen)
{
	char file_path[IMG_FULL_NAME_MAX] = {0};
	int ret = 0;
	unsigned long long len = 0;

	if (file == NULL || plen == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	strcpy(file_path, file);
	ret = get_file_len(file_path, &len);
	if (ret < 0)
	{
		rec_err("get file(%s) length fail.", file_path);
		return -EFILELEN;
	}

	*plen = len;
	return 0;
}

#ifdef CONFIG_BOOT_MMC
/*
 * read datazone from emmc
 * if whence = 0, read bcb from emmc datazone zone,
 * if whence = 1, read bcb from emmc datazone backup zone.
 *
 * return 0 if read success, otherwise -1.
 */
static int safeupg_read_datazone(struct datazone_info *pdz, int whence)
{
	char devnode[DEVNODE_NAME_LEN] = {0};
	int ret = 0;

	if (pdz == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	if (whence == 0)
		strncpy(devnode, "/dev/datazone", DEVNODE_NAME_LEN);
	else if (whence == 1)
		strncpy(devnode, "/dev/datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("where(%d) is error.\n", whence);
		return -EINVAL;
	}

	return read_datazone(pdz, devnode, DATAZONE_OFFSET_FROM_DATAZONE);
}

static int safeupg_write_datazone(struct datazone_info *pdz, int whence)
{
	char devnode[DEVNODE_NAME_LEN] = {0};
	int ret = 0;

	if (pdz == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	if (whence == 0)
		strncpy(devnode, "/dev/datazone", DEVNODE_NAME_LEN);
	else if (whence == 1)
		strncpy(devnode, "/dev/datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("where(%d) is error.\n", whence);
		return -EINVAL;
	}

	return write_datazone(pdz, devnode, DATAZONE_OFFSET_FROM_DATAZONE);
}

/*
 * when datazone update done, read bcb back from emmc check
 * to make sure update is correct.
 *
 * return 1 if check pass, otherwise 0.
 */
static int safeupg_datazone_readback_check(int whence)
{
	struct datazone_info rb_dz;
	char devnode[DEVNODE_NAME_LEN] = {0};
	int ret = 0;

	if (whence == 0)
		strncpy(devnode, "/dev/datazone", DEVNODE_NAME_LEN);
	else if (whence == 1)
		strncpy(devnode, "/dev/datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("where(%d) is error.\n", whence);
		return -EINVAL;
	}

	memset(&rb_dz, 0, sizeof(struct datazone_info));

	return datazone_readback_check(devnode, DATAZONE_OFFSET_FROM_DATAZONE);
}

#else /* !CONFIG_BOOT_MMC */

/*
 * read datazone from emmc
 * if whence = 0, read bcb from emmc datazone zone,
 * if whence = 1, read bcb from emmc datazone backup zone.
 *
 * return 0 if read success, otherwise -1.
 */
static int safeupg_read_datazone(struct datazone_info *pdz, int whence)
{
	char partname[DEVNODE_NAME_LEN] = {0};
	int ret = 0;

	if (pdz == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	if (whence == 0)
	{
		strncpy(partname, "datazone", DEVNODE_NAME_LEN);
	}
	else if (whence == 1)
	{
		strncpy(partname, "datazone_bk", DEVNODE_NAME_LEN);
	}
	else
	{
		rec_err("where(%d) is error.\n", whence);
		return -EINVAL;
	}

	return read_datazone(pdz, partname, DATAZONE_OFFSET_FROM_DATAZONE);
}

static int safeupg_write_datazone(struct datazone_info *pdz, int whence)
{
	char partname[DEVNODE_NAME_LEN] = {0};
	int ret = 0;

	if (pdz == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	if (whence == 0)
	{
		strncpy(partname, "datazone", DEVNODE_NAME_LEN);
	}
	else if (whence == 1)
	{
		strncpy(partname, "datazone_bk", DEVNODE_NAME_LEN);
	}
	else
	{
		rec_err("where(%d) is error.\n", whence);
		return -EINVAL;
	}

	return write_datazone(pdz, partname, DATAZONE_OFFSET_FROM_DATAZONE);
}

/*
 * when datazone update done, read bcb back from emmc check
 * to make sure update is correct.
 *
 * return 1 if check pass, otherwise 0.
 */
static int safeupg_datazone_readback_check(int whence)
{
	struct datazone_info rb_dz;
	char partname[DEVNODE_NAME_LEN] = {0};
	int ret = 0;

	if (whence == 0)
	{
		strncpy(partname, "datazone", DEVNODE_NAME_LEN);
	}
	else if (whence == 1)
	{
		strncpy(partname, "datazone_bk", DEVNODE_NAME_LEN);
	}
	else
	{
		rec_err("where(%d) is error.\n", whence);
		return -EINVAL;
	}

	memset(&rb_dz, 0, sizeof(struct datazone_info));

	return datazone_readback_check(partname, DATAZONE_OFFSET_FROM_DATAZONE);
}

#endif /* CONFIG_BOOT_MMC */

int read_file_from_zip(char* file, void * buffer, uint64_t len) {
	void *zip_reader = NULL;
    mz_zip_file *file_info = NULL;
    const char *extract_file_name = NULL;

	// 使用Minizip处理ZIP文件
	rec_info("[qiyun debug] upgrade file:%s\n", file);
	zip_reader = mz_zip_reader_create();
	if (zip_reader == NULL) {
		rec_err("Failed to create zip reader.\n");
		return -ENOMEM;
	}

	if (mz_zip_reader_open_file(zip_reader, RECOVERY_UPDATE_ZIP_NAME) != MZ_OK) {
		rec_err("Failed to open zip file: %s\n", RECOVERY_UPDATE_ZIP_NAME);
		mz_zip_reader_delete(&zip_reader);
		return -ESYSCALL;
	}

	if (mz_zip_reader_is_open(zip_reader) != MZ_OK){
		rec_err("mz_zip_reader_is_open ,Failed to open zip file\n");
		mz_zip_reader_close(zip_reader);
		mz_zip_reader_delete(&zip_reader);
		return -ESYSCALL;
	}
	// 定位到要提取的文件
	if (mz_zip_reader_locate_entry(zip_reader, file, 1) == MZ_OK){
		if (mz_zip_reader_entry_get_info(zip_reader, &file_info) == MZ_OK) {
			extract_file_name = file_info->filename;
			rec_info("[qiyun debug] extract_file_name=%s, file_len:%llu\n",extract_file_name, file_info->uncompressed_size);
		}
	}

	// 打开当前文件用于读取
	int result = mz_zip_reader_entry_open(zip_reader);
	if (result != MZ_OK) {
		rec_err("1111Failed to open entry '%s' in zip file '%s', result:%d\n", extract_file_name, RECOVERY_UPDATE_ZIP_NAME, result);
		rec_err("ZIP file size: %llu bytes, uncompressed size: %llu bytes\n", 
				file_info->compressed_size, file_info->uncompressed_size);
		mz_zip_reader_entry_close(zip_reader);
		mz_zip_reader_close(zip_reader);
		mz_zip_reader_delete(&zip_reader);
		return -ESYSCALL;
	}

	if(mz_zip_reader_entry_read(zip_reader, buffer, len) == 0 ){
		rec_err("read file:%s fail in zip :%s\n", file, RECOVERY_UPDATE_ZIP_NAME);
		mz_zip_reader_entry_close(zip_reader);
		mz_zip_reader_close(zip_reader);
		mz_zip_reader_delete(&zip_reader);
		return -EACCES;
	}
	mz_zip_reader_entry_close(zip_reader);
	mz_zip_reader_close(zip_reader);
	mz_zip_reader_delete(&zip_reader);

	return 0;
}

static int upgrade_datazone(partitionread *ptbl, int whence)
{
	int ret = 0;
	int fd = 0;
	int n = 0;
	unsigned long long file_len = 0;
	uint32_t chksum = 0;
	partitionread *ptbl_dz = NULL;
	partitionread *ptbl_ub_bk = NULL;
	char file[80] = {0};
	struct datazone_info dz;

	if (ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	ptbl_dz = lookup_partition_by_name(ptbl, "datazone");
	if (!ptbl_dz)
	{
		rec_err("can NOT lookup datazone partition.\n");
		return -EPARTITION;
	}
#ifdef CONFIG_AB_PART
	ptbl_ub_bk = lookup_partition_by_name(ptbl, "uboot_bk");
	if (!ptbl_ub_bk)
	{
		rec_err("can NOT lookup uboot_bk partition.\n");
		return -EPARTITION;
	}
#endif
	memset(&dz, 0, sizeof(struct datazone_info));

	if (strlen(ptbl_dz->szImageFileName) == 0)
	{
		rec_err("datazone partition image file is NULL.\n");
		strcpy(ptbl_dz->szImageFileName, "rd_datazone.bin");
	}

	ret = get_file_length(ptbl_dz->szImageFileName, &file_len);
	if (ret < 0)
	{
		rec_err("can't get file (%s) length.\n", ptbl_dz->szImageFileName);
		return -EFILELEN;
	}
	rec_info("file(%s) length is %lld\n", ptbl_dz->szImageFileName, file_len);

	if (file_len != 512)
	{
		rec_warn("rd_datazone.bin file length is NOT 512 bytes.\n");
	}
	ptbl_dz->u8RealDataSize = ALIGN(file_len, 512);
#ifdef CONFIG_NO_ZIP_UPDATE
	strcpy(file, ISO_ROOT);
	strcat(file, ptbl_dz->szImageFileName);

	rec_info("datazone image file path:%s\n", file);

	fd = open(file, O_RDONLY);
	if (fd < 0)
	{
		rec_err("open %s fail, %s.\n", file, strerror(errno));
		return -ESYSCALL;
	}

	n = read(fd, &dz, sizeof(struct datazone_info));
	if (n != sizeof(struct datazone_info))
	{
		rec_err("read fail, read(%d), expect(%d)\n", n, sizeof(struct datazone_info));
		close(fd);
		return -ESYSCALL;
	}
	close(fd);
#else
    // Minizip相关变量
	strcpy(file, ptbl_dz->szImageFileName);
    if (read_file_from_zip(file, &dz, sizeof(struct datazone_info)) < 0) {
		rec_err("read file(%s) from zip \n", ptbl_dz->szImageFileName);
		return -1;
	}
#endif
	// adjust_datazone_img_desc_bk(&dz, ptbl_ub_bk);
	chksum = calc_datazone_checksum(&dz);
	put_datazone_checksum(&dz, chksum);

	ret = safeupg_write_datazone(&dz, whence);
	if (ret < 0)
	{
		rec_err("safeupg write datazone fail.\n");

		return ret;
	}

	ret = safeupg_datazone_readback_check(whence);
	if (ret < 0)
	{
		rec_err("safeupg datazone readback check fail.\n");

		return ret;
	}


	return 0;
}

#ifdef CONFIG_BOOT_MMC
/*
 * read bcb from emmc
 * if whence = 0, read bcb from emmc bcb zone,
 * if whence = 1, read bcb from emmc bcb backup zone.
 *
 * return 0 if read success, otherwise -1.
 */
static int safeupg_read_bcb(struct safeupg_bootloader_message *pbcb, int whence)
{
	char devnode[DEVNODE_NAME_LEN] = {0};
	int ret = 0;

	if (pbcb == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	if (whence == WHENCE_MAIN)
		strncpy(devnode, "/dev/datazone", DEVNODE_NAME_LEN);
	else if (whence == WHENCE_BACKUP)
		strncpy(devnode, "/dev/datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("where(%d) is error.\n", whence);
		return -EINVAL;
	}

	return read_bcb(pbcb, devnode, BCB_OFFSET_FROM_DATAZONE);
}

static int safeupg_write_bcb(struct safeupg_bootloader_message *pbcb, int whence)
{
	char devnode[DEVNODE_NAME_LEN] = {0};
	int ret = 0;

	if (pbcb == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	if (whence == WHENCE_MAIN)
		strncpy(devnode, "/dev/datazone", DEVNODE_NAME_LEN);
	else if (whence == WHENCE_BACKUP)
		strncpy(devnode, "/dev/datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("where(%d) is error.\n", whence);
		return -EINVAL;
	}

	return write_bcb(pbcb, devnode, BCB_OFFSET_FROM_DATAZONE);
}

/*
 * when bcb update done, read bcb back from emmc check
 * to make sure update is correct.
 *
 * return 1 if check pass, otherwise 0.
 */
static int safeupg_bcb_readback_check(int whence)
{
	char devnode[DEVNODE_NAME_LEN] = {0};
	int ret = 0;

	if (whence == WHENCE_MAIN)
		strncpy(devnode, "/dev/datazone", DEVNODE_NAME_LEN);
	else if (whence == WHENCE_BACKUP)
		strncpy(devnode, "/dev/datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("where(%d) is error.\n", whence);
		return -EINVAL;
	}

	return bcb_readback_check(devnode, BCB_OFFSET_FROM_DATAZONE);
}

#else /* !CONFIG_BOOT_MMC */

/*
 * read bcb from nand
 * if whence = 0, read bcb from emmc bcb zone,
 * if whence = 1, read bcb from emmc bcb backup zone.
 *
 * return 0 if read success, otherwise -1.
 */
static int safeupg_read_bcb(struct safeupg_bootloader_message *pbcb, int whence)
{
	char partname[DEVNODE_NAME_LEN] = {0};
	int ret = 0;

	if (pbcb == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	if (whence == WHENCE_MAIN)
		strncpy(partname, "datazone", DEVNODE_NAME_LEN);
	else if (whence == WHENCE_BACKUP)
		strncpy(partname, "datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("where(%d) is error.\n", whence);
		return -EINVAL;
	}

	return read_bcb(pbcb, partname, BCB_OFFSET_FROM_DATAZONE);
}

static int safeupg_write_bcb(struct safeupg_bootloader_message *pbcb, int whence)
{
	char partname[DEVNODE_NAME_LEN] = {0};
	int ret = 0;
	rec_dbg("write bcb to %s\n", partname);
	if (pbcb == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	if (whence == WHENCE_MAIN)
		strncpy(partname, "datazone", DEVNODE_NAME_LEN);
	else if (whence == WHENCE_BACKUP)
		strncpy(partname, "datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("where(%d) is error.\n", whence);
		return -EINVAL;
	}
	rec_dbg("write bcb11111\n");
	return write_bcb(pbcb, partname, BCB_OFFSET_FROM_DATAZONE);
}

/*
 * when bcb update done, read bcb back from emmc check
 * to make sure update is correct.
 *
 * return 1 if check pass, otherwise 0.
 */
static int safeupg_bcb_readback_check(int whence)
{
	char partname[DEVNODE_NAME_LEN] = {0};
	int ret = 0;

	if (whence == WHENCE_MAIN)
		strncpy(partname, "datazone", DEVNODE_NAME_LEN);
	else if (whence == WHENCE_BACKUP)
		strncpy(partname, "datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("where(%d) is error.\n", whence);
		return -EINVAL;
	}

	return bcb_readback_check(partname, BCB_OFFSET_FROM_DATAZONE);
}

#endif /* CONFIG_BOOT_MMC */

/*
 * translate_bcb_laststatus_to_upg_info() - translate <bcb.laststatus> to <upg information> format.
 */
static int translate_bcb_laststatus_to_upg_info(struct safeupg_bootloader_message *pbcb,
												struct safeupg_upg_info *pui)
{
	uint32_t val;

	if (pbcb == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}

	val = (pbcb->laststatus[3] << 24) | (pbcb->laststatus[2] << 16) |
		  (pbcb->laststatus[1] << 8) | pbcb->laststatus[0];

	if (val & UPG_INFO_COREPROG)
		pui->m_coreprog = 1;
	else
		pui->m_coreprog = 0;

	if (val & UPG_INFO_SYSTEMPROG)
		pui->m_systemprog = 1;
	else
		pui->m_systemprog = 0;

	if (val & UPG_INFO_APP)
		pui->m_app = 1;
	else
		pui->m_app = 0;

	if (val & UPG_INFO_USERDATA)
		pui->m_userdata = 1;
	else
		pui->m_userdata = 0;

	if (val & UPG_INFO_EXCEPTION)
		pui->exception = 1;
	else
		pui->exception = 0;

	if (val & UPG_VERIFY_IMG)
		pui->verifyimg = 1;
	else
		pui->verifyimg = 0;

	// TODO, if more status.

	return 0;
}

/*
 * translate_upg_info_to_bcb_laststatus() - translate <upg information> to <bcb.laststatus> format.
 */
static int translate_upg_info_to_bcb_laststatus(struct safeupg_upg_info *pui,
												struct safeupg_bootloader_message *pbcb)
{
	uint32_t val = 0;

	if (pui == NULL || pbcb == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}

	if (pui->m_coreprog)
		val |= UPG_INFO_COREPROG;

	if (pui->m_systemprog)
		val |= UPG_INFO_SYSTEMPROG;

	if (pui->m_app)
		val |= UPG_INFO_APP;

	if (pui->m_userdata)
		val |= UPG_INFO_USERDATA;

	if (pui->exception)
		val |= UPG_INFO_EXCEPTION;

	pbcb->laststatus[0] = val & 0xff;
	pbcb->laststatus[1] = (val >> 8) & 0xff;
	pbcb->laststatus[2] = (val >> 16) & 0xff;
	pbcb->laststatus[3] = (val >> 24) & 0xff;
	rec_info("pbcb->laststatus[0]=0x%x,pbcb->laststatus[1]=0x%x,pbcb->laststatus[2]=0x%x,pbcb->laststatus[3]=0x%x.\n",
			 pbcb->laststatus[0], pbcb->laststatus[1], pbcb->laststatus[2], pbcb->laststatus[3]);

	return 0;
}

static int read_write_bcb_to_change_laststatus(int whence, struct safeupg_upg_info *pui)
{
	int ret = 0;
	uint32_t chksum = 0;
	struct safeupg_bootloader_message bcb;

	if (pui == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	ret = safeupg_read_bcb(&bcb, whence);
	if (ret < 0)
	{
		rec_err("safeupg read_bcb fail.\n");
		return ret;
	}

	translate_upg_info_to_bcb_laststatus(pui, &bcb);
	chksum = calc_bcb_checksum(&bcb);
	put_bcb_checksum(&bcb, chksum);

	ret = safeupg_write_bcb(&bcb, whence);
	if (ret < 0)
	{
		rec_err("safeupg write_bcb fail.\n");
		return ret;
	}

	ret = safeupg_bcb_readback_check(whence);
	if (ret < 0)
	{
		rec_err("safeupg_bcb_readback_check fail.\n");
		return ret;
	}

	return 0;
}

static int write_bcb_to_change_laststatus(struct safeupg_bootloader_message *pbcb,
										  int whence, struct safeupg_upg_info *pui)
{
	int ret = 0;
	uint32_t chksum = 0;

	if (pbcb == NULL || pui == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	translate_upg_info_to_bcb_laststatus(pui, pbcb);
	rec_dbg("enter calc_bcb_checksum");
	chksum = calc_bcb_checksum(pbcb);
	rec_dbg("enter put_bcb_checksum");
	put_bcb_checksum(pbcb, chksum);
	rec_dbg("enter safeupg_write_bcb");

	ret = safeupg_write_bcb(pbcb, whence);
	if (ret < 0)
	{
		rec_err("safeupg_write_bcb fail.\n");
		return ret;
	}

	ret = safeupg_bcb_readback_check(whence);
	if (ret < 0)
	{
		rec_err("safeupg_bcb_readback_check fail.\n");
		return ret;
	}

	return 0;
}

static int write_bcb_to_clear_laststatus(struct safeupg_bootloader_message *pbcb, int whence)
{
	struct safeupg_upg_info ui;

	if (pbcb == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	memset(&ui, 0, sizeof(struct safeupg_upg_info));
	return write_bcb_to_change_laststatus(pbcb, whence, &ui);
}

static int write_bcb_to_set_bootflag(struct safeupg_bootloader_message *pbcb,
									 int whence, uint32_t bootflag)
{
	int ret = 0;
	uint32_t chksum = 0;

	if (pbcb == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	put_bcb_bootflag(pbcb, bootflag);
	chksum = calc_bcb_checksum(pbcb);
	put_bcb_checksum(pbcb, chksum);

	ret = safeupg_write_bcb(pbcb, whence);
	if (ret < 0)
	{
		rec_err("safeupg_write_bcb fail.\n");
		return ret;
	}

	ret = safeupg_bcb_readback_check(whence);
	if (ret < 0)
	{
		rec_err("safeupg_bcb_readback_check fail.\n");
		return ret;
	}

	return 0;
}

static int read_bcb_to_get_upg_info(int whence, struct safeupg_upg_info *pui)
{
	int ret = 0;
	struct safeupg_bootloader_message bcb;

	if (pui == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	ret = safeupg_read_bcb(&bcb, whence);
	if (ret < 0)
	{
		rec_err("safeupg_read_bcb fail.\n");
		return ret;
	}

	translate_bcb_laststatus_to_upg_info(&bcb, pui);
	return 0;
}

static int get_upg_info_from_bcb(struct safeupg_bootloader_message *pbcb, struct safeupg_upg_info *pui)
{
	return translate_bcb_laststatus_to_upg_info(pbcb, pui);
}

#ifdef CONFIG_BOOT_MMC

static int safeupg_read_partition_head(struct safeupg_partitionhead *phead, int whence)
{
	int ret = 0;
	char devnode[DEVNODE_NAME_LEN] = {0};

	if (phead == NULL)
	{
		rec_err("phead is NULL.\n");
		return -EINVAL;
	}

	if (whence == 0)
		strncpy(devnode, "/dev/datazone", DEVNODE_NAME_LEN);
	else if (whence == 1)
		strncpy(devnode, "/dev/datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("whence(%d) is error.\n", whence);
		return -EINVAL;
	}

	ret = read_partition_head(phead, devnode, PARTITION_OFFSET_DATAZONE); // 8KB offset from datazone/datazone_bk
	if (ret < 0)
	{
		rec_err("read partition head fail.\n");
		return ret;
	}
	return 0;
}

static partitionread *safeupg_read_partition_info(struct safeupg_partitionhead *phead, int whence)
{
	char devnode[DEVNODE_NAME_LEN] = {0};
	partitionread *partitioninfo = NULL;

	if (phead == NULL)
	{
		rec_err("phead is NULL.\n");
		return NULL;
	}

	if (whence == 0)
		strncpy(devnode, "/dev/datazone", DEVNODE_NAME_LEN);
	else if (whence == 1)
		strncpy(devnode, "/dev/datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("whence(%d) is error.\n", whence);
		return NULL;
	}

	partitioninfo = read_partition_info(phead, devnode, PARTITION_OFFSET_DATAZONE); // 8KB offset from datazone/datazone_bk

	if (partitioninfo == NULL)
	{
		rec_err("read partition info fail.\n");
	}
	return partitioninfo;
}

static int safeupg_write_partition_info(struct safeupg_partitionhead *phead, partitionread *ptbl, int whence)
{
	int ret = 0;
	char devnode[DEVNODE_NAME_LEN] = {0};

	if (phead == NULL || ptbl == NULL)
	{
		rec_err("No partition table to be written.\n");
		return -EINVAL;
	}

	if (whence == 0)
		strncpy(devnode, "/dev/datazone", DEVNODE_NAME_LEN);
	else if (whence == 1)
		strncpy(devnode, "/dev/datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("whence(%d) is error.\n", whence);
		return -EINVAL;
	}

	ret = write_partition_info(phead, ptbl, devnode, PARTITION_OFFSET_DATAZONE); // 8KB offset from datazone/datazone_bk
	if (ret < 0)
	{
		rec_err("write partition info fail.\n");
		return ret;
	}

	return 0;
}

/*
 * when partitioninfo update done, read bcb back from emmc to check
 * to make sure update is correct.
 *
 * return 1 if check pass, otherwise 0.
 */
static int safeupg_partition_info_readback_check(int whence)
{
	int ret = 0;
	char devnode[DEVNODE_NAME_LEN] = {0};

	if (whence == 0)
		strncpy(devnode, "/dev/datazone", DEVNODE_NAME_LEN);
	else if (whence == 1)
		strncpy(devnode, "/dev/datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("whence(%d) is error.\n", whence);
		return 0;
	}

	ret = partition_info_readback_check(devnode, PARTITION_OFFSET_DATAZONE); // 8KB offset from datazone/datazone_bk
	if (ret == 0)
	{
		rec_err("partition info readback check fail.\n");
		return 0;
	}
	return 1;
}

#else /* !CONFIG_BOOT_MMC */

static int safeupg_read_partition_head(struct safeupg_partitionhead *phead, int whence)
{
	int ret = 0;
	char partname[DEVNODE_NAME_LEN] = {0};

	if (phead == NULL)
	{
		rec_err("phead is NULL.\n");
		return -EINVAL;
	}

	if (whence == 0)
		strncpy(partname, "datazone", DEVNODE_NAME_LEN);
	else if (whence == 1)
		strncpy(partname, "datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("whence(%d) is error.\n", whence);
		return -EINVAL;
	}

	ret = read_partition_head(phead, partname, PARTITION_OFFSET_DATAZONE); // 8KB offset from datazone/datazone_bk
	if (ret < 0)
	{
		rec_err("read partition head fail.\n");
		return ret;
	}
	return 0;
}

static partitionread *safeupg_read_partition_info(struct safeupg_partitionhead *phead, int whence)
{
	char partname[DEVNODE_NAME_LEN] = {0};
	partitionread *partitioninfo = NULL;

	if (phead == NULL)
	{
		rec_err("phead is NULL.\n");
		return NULL;
	}

	if (whence == 0)
		strncpy(partname, "datazone", DEVNODE_NAME_LEN);
	else if (whence == 1)
		strncpy(partname, "datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("whence(%d) is error.\n", whence);
		return NULL;
	}

	partitioninfo = read_partition_info(phead, partname, PARTITION_OFFSET_DATAZONE); // 8KB offset from datazone/datazone_bk

	if (partitioninfo == NULL)
	{
		rec_err("read partition info fail.\n");
	}
	return partitioninfo;
}

static int safeupg_write_partition_info(struct safeupg_partitionhead *phead, partitionread *ptbl, int whence)
{
	int ret = 0;
	char partname[DEVNODE_NAME_LEN] = {0};

	if (phead == NULL || ptbl == NULL)
	{
		rec_err("No partition table to be written.\n");
		return -EINVAL;
	}

	if (whence == 0)
		strncpy(partname, "datazone", DEVNODE_NAME_LEN);
	else if (whence == 1)
		strncpy(partname, "datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("whence(%d) is error.\n", whence);
		return -EINVAL;
	}

	ret = write_partition_info(phead, ptbl, partname, PARTITION_OFFSET_DATAZONE); // 8KB offset from datazone/datazone_bk
	if (ret < 0)
	{
		rec_err("write partition info fail.\n");
		return ret;
	}

	return 0;
}

/*
 * when partitioninfo update done, read bcb back from emmc to check
 * to make sure update is correct.
 *
 * return 1 if check pass, otherwise 0.
 */
static int safeupg_partition_info_readback_check(int whence)
{
	int ret = 0;
	char partname[DEVNODE_NAME_LEN] = {0};

	if (whence == 0)
		strncpy(partname, "datazone", DEVNODE_NAME_LEN);
	else if (whence == 1)
		strncpy(partname, "datazone_bk", DEVNODE_NAME_LEN);
	else
	{
		rec_err("whence(%d) is error.\n", whence);
		return 0;
	}

	ret = partition_info_readback_check(partname, PARTITION_OFFSET_DATAZONE); // 8KB offset from datazone/datazone_bk
	if (ret == 0)
	{
		rec_err("partition info readback check fail.\n");
		return 0;
	}
	return 1;
}

static uint32_t calc_checksum_from_file(const char *file)
{
	long long sizer = 0;
	long long rd_want = 0;
	unsigned long long file_len = 0;
	char *buffer = NULL;
	uint32_t chksum = 0;
	int ret = 0;
	int fd = 0;

	if (file == NULL)
	{
		rec_err("file is NULL.\n");
		return 0;
	}

	ret = get_file_len(file, &file_len);
	if (ret < 0)
	{
		rec_err("file_len get fail.\n");
		return 0xdeaddead;
	}
	rec_info("%s length: 0x%llx\n", file, file_len);

	fd = open(file, O_RDONLY | O_LARGEFILE, 0);
	if (fd < 0)
	{
		rec_err("open %s fail.\n", file);
		return 0xdeaddead;
	}

	buffer = (char *)malloc(FILE_RW_SIZE);
	if (!buffer)
	{
		rec_err("malloc fail.\n");
		close(fd);
		return 0;
	}

	while (file_len > 0)
	{
		if (file_len >= FILE_RW_SIZE)
		{
			sizer = read(fd, buffer, FILE_RW_SIZE);
			rd_want = FILE_RW_SIZE;
		}
		else
		{
			sizer = read(fd, buffer, file_len);
			rd_want = file_len;
		}
		if (sizer != rd_want)
		{
			rec_err("read error.\n");
			break;
		}
		chksum = checksum32(chksum, buffer, sizer);
		file_len -= sizer;
	}

	close(fd);
	free(buffer);

	rec_info("succeeded, chksum(0x%x)!\n", chksum);

	return chksum;
}

static uint32_t calc_checksum_from_file_before_upg(
	partitionread *pentry)
{
	char file[IMG_FULL_NAME_MAX];

	if (pentry == NULL)
		return 0x12345678;

	if (strcmp(pentry->szPartName, "preloader") == 0 ||
		strcmp(pentry->szPartName, "preloader_bk") == 0 ||
		strcmp(pentry->szPartName, "datazone") == 0 ||
		strcmp(pentry->szPartName, "datazone_bk") == 0)
	{
		return 0;
	}

	strcpy(file, ISO_ROOT);
	strcat(file, pentry->szImageFileName);

	return calc_checksum_from_file(file);
}

#endif /* CONFIG_BOOT_MMC */

static int upgrade_partition_info(struct safeupg_partitionhead *phead, partitionread *ptbl, int whence)
{
	int ret = 0;

	if (phead == NULL || ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	ret = safeupg_write_partition_info(phead, ptbl, whence);
	if (ret < 0)
	{
		rec_err("safeupg write partition info fail.\n");
		return ret;
	}

	ret = safeupg_partition_info_readback_check(whence);
	if (ret == 0)
	{
		rec_err("safeupg partition info readback check fail.\n");
		return -ERDBACKCHK;
	}
	return 0;
}

/*
 * check_update_completeness()
 * return 0 if no error encountered, otherwise -1.
 */
static int merge_partition_info(partitionread **ppnew_tbl,
								partitionread *pemmc_tbl, partitionread *pxml_tbl)
{
	partitionread *ptblnewpre = *ppnew_tbl;
	partitionread *ptbloldtmp = pemmc_tbl;
	partitionread *ptblxmltmp = pxml_tbl;
	partitionread *ptbl_find = NULL;
	int ret = 0;
	int partition_changed = 0;

	if (pemmc_tbl == NULL || pxml_tbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	rec_info("begin ...\r\n");
	while (ptblxmltmp && ptbloldtmp)
	{
		// pick up one table entry from old table, then search it in xml table.
		ptbl_find = lookup_partition_by_name(pxml_tbl, ptbloldtmp->szPartName);
		if ((ptbl_find != NULL) && (ptbl_find->u8PartitionStartAddr == ptbloldtmp->u8PartitionStartAddr) &&
			(ptbl_find->u8PartitionSize == ptbloldtmp->u8PartitionSize))
		{
			// this partition [start, end] is NOT change, copy emmc partition as new one.
			partitionread *pinfo = (partitionread *)malloc(sizeof(partitionread));
			if (pinfo == NULL)
			{
				rec_err("No memory-1!\n");
				return -ENOMEM;
			}
			memcpy(pinfo, ptbloldtmp, sizeof(partitionread));
			// TODO,confirm, if emmc partition has image file name, this step skip
			strcpy(pinfo->szImageFileName, ptbl_find->szImageFileName);
			pinfo->nextpartition = NULL;
			pinfo->u4LastPartition = 0; // UPDATE_FLAG_REQUIRE;
			if (*ppnew_tbl == NULL)
				*ppnew_tbl = pinfo;
			else
				ptblnewpre->nextpartition = pinfo;
			ptblnewpre = pinfo;
			ptbloldtmp = ptbloldtmp->nextpartition;
			ptblxmltmp = ptbl_find->nextpartition;
		}
		else if (ptbl_find == NULL)
		{
			// this partition can't be found in xml table, copy old partition as new one.
			partitionread *pinfo = (partitionread *)malloc(sizeof(partitionread));
			if (pinfo == NULL)
			{
				rec_err("No memory-2!\n");
				return -ENOMEM;
			}
			memcpy(pinfo, ptbloldtmp, sizeof(partitionread));
			pinfo->nextpartition = NULL;
			pinfo->u4LastPartition = UPDATE_FLAG_REQUIRE;
			if (*ppnew_tbl == NULL)
				*ppnew_tbl = pinfo;
			else
				ptblnewpre->nextpartition = pinfo;
			ptblnewpre = pinfo;
			ptbloldtmp = ptbloldtmp->nextpartition;
		}
		else
		{
			/*
			 * if run here, it means partion [start end] changed.
			 *
			 * !!!important!!!
			 * merging xml and old partition is based the below assumption.
			 * if some xml partition changed, from this partion start, all the rest partitions should be provided in xml...
			 *
			 */
			partition_changed = 1;
			ptbloldtmp = NULL;
			ptblxmltmp = NULL;

			while (ptbl_find)
			{
				partitionread *pinfo = (partitionread *)malloc(sizeof(partitionread));
				if (pinfo == NULL)
				{
					rec_err("No memory-3!\n");
					return -ENOMEM;
				}
				memcpy(pinfo, ptbl_find, sizeof(partitionread));
				pinfo->nextpartition = NULL;
				pinfo->u4LastPartition = UPDATE_FLAG_REQUIRE;

				if (*ppnew_tbl == NULL)
					*ppnew_tbl = pinfo;
				else
					ptblnewpre->nextpartition = pinfo;

				ptblnewpre = pinfo;
				ptbl_find = ptbl_find->nextpartition;
			}
		}
	}
	if (partition_changed == 0)
	{
		while (ptblxmltmp)
		{
			partitionread *pinfo = (partitionread *)malloc(sizeof(partitionread));
			if (pinfo == NULL)
			{
				rec_err("No memory-4!\n");
				return -ENOMEM;
			}
			memcpy(pinfo, ptblxmltmp, sizeof(partitionread));
			pinfo->nextpartition = NULL;
			pinfo->u4LastPartition = UPDATE_FLAG_REQUIRE;
			if (*ppnew_tbl == NULL)
				*ppnew_tbl = pinfo;
			else
				ptblnewpre->nextpartition = pinfo;
			ptblnewpre = pinfo;
			ptblxmltmp = ptblxmltmp->nextpartition;
		}
		while (ptbloldtmp)
		{
			partitionread *pinfo = (partitionread *)malloc(sizeof(partitionread));
			if (pinfo == NULL)
			{
				rec_err("No memory-5!\n");
				return -ENOMEM;
			}
			memcpy(pinfo, ptbloldtmp, sizeof(partitionread));
			pinfo->nextpartition = NULL;
			pinfo->u4LastPartition = UPDATE_FLAG_REQUIRE;
			if (*ppnew_tbl == NULL)
				*ppnew_tbl = pinfo;
			else
				ptblnewpre->nextpartition = pinfo;
			ptblnewpre = pinfo;
			ptbloldtmp = ptbloldtmp->nextpartition;
		}
	}

	rec_info("end.\r\n");

	return 0;
}

static int content_changed(partitionread *ptbl)
{
	// TODO
	return 1; // Temp
}

static int decide_which_partitions_need_upgrade(partitionread *part_table)
{
	partitionread *ptbl = part_table;

	if (ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -1;
	}
	while (ptbl)
	{
		if (partition_need_imagefile(ptbl))
		{
			if (check_file_exist_for_one_table(ptbl, ISO_ROOT) == 0)
			{
				//  partition has image file,but the image file does NOT exsit.
				rec_info("partial upg: partition(%s) need image file to upgrade, but image file(%s) NOT exist\n", ptbl->szPartName, ptbl->szImageFileName);
				ptbl->u4LastPartition = 0;
			}
			else {
				// partition has image file,and the image file exsit.
				ptbl->u4LastPartition = UPDATE_FLAG_REQUIRE;
			}
		}
		ptbl = ptbl->nextpartition;
	}

	return 0;
}

static int calc_total_size_need_upgrade(partitionread *ptbl, struct safeupg_upg_info *pui)
{
	int ret = 0;
	unsigned long long n = 0;

	if (pui == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	for (; ptbl; ptbl = ptbl->nextpartition) {
		n = 0;
		if (!partition_need_imagefile(ptbl)) {
			ptbl->u4LastPartition = 0;
			continue;
		}

		ret = get_file_length(ptbl->szImageFileName, &n);
		if (ret < 0) {
			rec_err("file:%s, get_file_length fail.\n", ptbl->szImageFileName);
			ptbl->u4LastPartition = 0;
			continue;
		}

		ptbl->u8RealDataSize = n;
		ptbl->u4LastPartition = UPDATE_FLAG_REQUIRE;
		g_safeupg_status_self.need_upgrade_partition_num++;
		rec_info("safeupg_status_self.need_upgrade_partition_num=%d.\n", g_safeupg_status_self.need_upgrade_partition_num);
		g_safeupg_status_self.partition_total_num++;
		g_safeupg_status_self.total_size_need_upgrade += n;

		{
			if (n > 0)
				rec_info("partition(%s) is calculated in upgrade size.\n", ptbl->szPartName);
		}
	}

	pthread_mutex_unlock(&g_safeupg_mutex);

	rec_info("total_size_need_upgrade(0x%llx)\n", g_safeupg_status_self.total_size_need_upgrade);
	rec_info("need_upgrade_partition_num(%d)\n", g_safeupg_status_self.need_upgrade_partition_num);

	return 0;
}

static int check_partition_table_changed(partitionread *pemmc_tbl, partitionread *pxml_tbl)
{
	partitionread *ptbloldtmp = pemmc_tbl;
	partitionread *ptblxmltmp = pxml_tbl;
	partitionread *ptbl_find = NULL;

	if (pemmc_tbl == NULL || pxml_tbl == NULL)
	{
		rec_err("parameter is NULL.\n");
		return -EINVAL;
	}

	rec_info("begin check partition table change...\r\n");
	while (ptblxmltmp && ptbloldtmp)
	{
		// Look up the current emmc table partition in the XML table
		ptbl_find = lookup_partition_by_name(pxml_tbl, ptbloldtmp->szPartName);
		if (ptbl_find == NULL)
		{
			// If partition not found in XML table, there's a change
			rec_info("Partition:%s not found in new table.\r\n", ptbloldtmp->szPartName);
			return 1;
		}

		// Check if both start address and size are identical
		if (ptbl_find->u8PartitionStartAddr != ptbloldtmp->u8PartitionStartAddr ||
			ptbl_find->u8PartitionSize != ptbloldtmp->u8PartitionSize)
		{
			// Partition information has change
			rec_info("Partition:%s table change. start_addr: 0x%llx->0x%llx, size: 0x%llx->0x%llx\r\n",
					 ptbloldtmp->szPartName,
					 ptbloldtmp->u8PartitionStartAddr, ptbl_find->u8PartitionStartAddr,
					 ptbloldtmp->u8PartitionSize, ptbl_find->u8PartitionSize);
			return 1;
		}

		ptbloldtmp = ptbloldtmp->nextpartition;
		ptblxmltmp = ptbl_find->nextpartition;
	}

	// Check if one table has ended while the other still has remaining partitions
	if ((ptbloldtmp == NULL) != (ptblxmltmp == NULL))
	{
		rec_info("Partition table length not match.\r\n");
		return 1;
	}

	rec_info("Partition table unchanged.\r\n");
	return 0;
}

static long long get_main_startup_a_end_addr(partitionread *ptbl)
{

	partitionread *ptemp = NULL;

	if (ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	ptemp = lookup_partition_by_name(ptbl, MAIN_STARTUP_A_LAST_PART_NAME);
	if (!ptemp)
	{
		rec_err("can't lookup " MAIN_STARTUP_A_LAST_PART_NAME " partition.\n");
		return -EPARTITION;
	}

	return ptemp->u8PartitionStartAddr + ptemp->u8PartitionSize;
}

static int safeupg_update_raw_partition_from_file(const char *devname,
												  const char *file, long long offset, long long size, long long *psize_total)
{
	return upg_raw_partition_from_file(devname, file, offset, size, psize_total);
}

static int safeupg_update_ext4_partition_from_file(const char *devname,
												   const char *file, long long offset, long size)
{
	return upg_ext4_partition_from_file(devname, file, offset, size);
}



static int upgrade_one_partition(partitionread *part, const char *root)
{
	int status = 0;
	char file[IMG_FULL_NAME_MAX];
	int ret = 0;
	long long size_total = 0;
#ifndef CONFIG_BOOT_MMC
	uint32_t chksum_file = 0;
	uint32_t chksum_nand = 0;
	unsigned long long realsize = 0;
#endif

	if (!part)
		return 0;

#ifdef CONFIG_BOOT_MMC
	if (part->u8PartitionStartAddr < 0x10000)
		return 0;
#endif

	if (part->u4LastPartition != UPDATE_FLAG_REQUIRE)
	{
		rec_info("%s NOT need upgrade \n", part->szPartName);
		return ret;
	}

#ifdef CONFIG_BOOT_MMC
#ifdef NEW_PARTITION_DESIGN
	if (DISABLE == part->u4Flag)
		return ret;
#endif
#endif

	if (!strcmp(part->szType, "fat32"))
	{
		// it's fat32.
		if (part->u4Mount & 0x2)
		{
			format_userdata_partition_fat32(part->u8PartitionStartAddr, part->u8PartitionSize);
		}
		if (part->u4Mount)
			part->u4Mount = 1;
		return ret;
	}
	// rec_info("qiyundebug-----qiiiy.\n");

	// which partition is ongoing upgrade.
	pthread_mutex_lock(&g_safeupg_mutex);
	strncpy(g_safeupg_status_self.upgrade_ongoing_part_name, part->szPartName, PART_NAME_LEN_MAX - 1);
	pthread_mutex_unlock(&g_safeupg_mutex);
	// rec_info("qiyundebug-----qiiiyy.part->szPartName=%s,imagename=%s\n",part->szPartName,part->szImageFileName);

	if (strlen(part->szImageFileName))
	{
#ifdef CONFIG_NO_ZIP_UPDATE
		strcpy(file, root);
		strcat(file, part->szImageFileName);
#else
		strcpy(file, part->szImageFileName);
#endif

/*yzp
		if (!check_is_file_exist(file))
		{
			rec_err("%s not found\n");
			return -EINVAL;
		}
		rec_info("part->szPartName=%s,imagename=%s,sztype=%s\n", part->szPartName, part->szImageFileName, part->szType);
*/


#ifndef CONFIG_BOOT_MMC
		/* nand */
		if (safeupg_vfy_image_flag)
		{
			/* get file len */
			ret = get_file_len(file, &realsize);
			if (ret < 0)
				realsize = 0;

			chksum_file = calc_checksum_from_file_before_upg(part);
			rec_info("chksum_file=0x%x\n", chksum_file);
		}
#endif

		if (!strcmp(part->szType, "raw"))
		{
#ifdef CONFIG_BOOT_MMC
			/* emmc */
			ret = safeupg_update_raw_partition_from_file(devname,
														 file, part->u8PartitionStartAddr,
														 part->u8PartitionSize, &size_total);
#else
			/* nand */
			ret = safeupg_update_raw_partition_from_file(part->szPartName,
														 file, 0,
														 part->u8PartitionSize, &size_total);
#endif
			pthread_mutex_lock(&g_safeupg_mutex);
			if (ret >= 0)
			{
				g_safeupg_status_self.size_upgrade_done += size_total;
				g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].flag = UPGRADE_FLAG_DONE | UPGRADE_FLAG_SUCCESS;
				strncpy(g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].part_name, part->szPartName, PART_NAME_LEN_MAX - 1);
				size_total = ALIGN(size_total, 512); // align 512
				part->u8RealDataSize = size_total;
				part->u4LastPartition |= UPDATE_FLAG_DONE;
				ret = ESUCCESS;
			}
			else
			{
				g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].flag = UPGRADE_FLAG_DONE;
				strncpy(g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].part_name, part->szPartName, PART_NAME_LEN_MAX - 1);
			}
			g_safeupg_status_self.part_count_upgrade_done++;
			if (g_safeupg_status_self.part_count_upgrade_done >= PART_NUM_MAX)
			{
				rec_err("partition num >= %d\n", PART_NUM_MAX);
				g_safeupg_status_self.part_count_upgrade_done--;
			}
			pthread_mutex_unlock(&g_safeupg_mutex);
		}
		else if (!strcmp(part->szType, "ext4"))
		{
#ifdef CONFIG_BOOT_MMC
			/* emmc */
			ret = safeupg_update_ext4_partition_from_file(devname, file,
														  part->u8PartitionStartAddr, part->u8PartitionSize);
#else
			/* nand */
			ret = safeupg_update_ext4_partition_from_file(part->szPartName, file,
														  0, part->u8PartitionSize);
#endif
			pthread_mutex_lock(&g_safeupg_mutex);
			if (ret >= 0)
			{
				rec_info("qiyun upgrde image name=%s\n", part->szImageFileName);
				g_safeupg_status_self.size_upgrade_done += part->u8RealDataSize;
				;
				g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].flag = UPGRADE_FLAG_DONE | UPGRADE_FLAG_SUCCESS;
				strncpy(g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].part_name, part->szPartName, PART_NAME_LEN_MAX - 1);
				part->u8RealDataSize = ret;
				part->u4LastPartition |= UPDATE_FLAG_DONE;
				ret = ESUCCESS;
			}
			else
			{
				g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].flag = UPGRADE_FLAG_DONE;
				strncpy(g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].part_name, part->szPartName, PART_NAME_LEN_MAX - 1);
			}
			rec_info("qiyundebug upgrade name =%s", part->szPartName);
			g_safeupg_status_self.part_count_upgrade_done++;
			if (g_safeupg_status_self.part_count_upgrade_done >= PART_NUM_MAX)
			{
				rec_err("partition num >= %d\n", PART_NUM_MAX);
				g_safeupg_status_self.part_count_upgrade_done--;
			}
			pthread_mutex_unlock(&g_safeupg_mutex);
		}

#ifndef CONFIG_BOOT_MMC
		/* nand */
		if (safeupg_vfy_image_flag)
		{
			chksum_nand = calc_checksum_from_nand_after_upg(
				0, /* offset */
				realsize,
				part,
				1 /* safeupg */);
			rec_info("chksum_nand=0x%x\n", chksum_nand);

			if (chksum_file == 0 && chksum_nand == 0)
			{
				rec_warn("chksum_file and chksum_nand are 0.\n");
			}
			if (chksum_file != chksum_nand)
				rec_err("checksum compare error, chksum_file(0x%x), chksum_nand(0x%x)\n",
						chksum_file, chksum_nand);
		}
#endif
	}

	return ret;
}

static int check_main_startup_completeness(partitionread *ptbl, int whence_startup)
{
	partitionread *pfirst = NULL;
	partitionread *plast = NULL;
	char first_part_name[PART_NAME_LEN_MAX] = {0};
	char last_part_name[PART_NAME_LEN_MAX] = {0};

	if (ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	if (whence_startup == MAIN_STARTUP_A)
	{
		strncpy(first_part_name, MAIN_STARTUP_A_FIRST_PART_NAME, PART_NAME_LEN_MAX - 1);
		strncpy(last_part_name, MAIN_STARTUP_A_LAST_PART_NAME, PART_NAME_LEN_MAX - 1);
	}
	else
	{
		strncpy(first_part_name, MAIN_STARTUP_B_FIRST_PART_NAME, PART_NAME_LEN_MAX - 1);
		strncpy(last_part_name, MAIN_STARTUP_B_LAST_PART_NAME, PART_NAME_LEN_MAX - 1);
	}

	pfirst = lookup_partition_by_name(ptbl, first_part_name);
	if (!pfirst)
	{
		rec_err("can NOT lookup partition %s from partition table.\n", first_part_name);
		return -EPARTITION;
	}
	else
	{
		rec_info("lookup partition %s from partition table success.\n", first_part_name);
	}

	plast = lookup_partition_by_name(ptbl, last_part_name);
	if (!plast)
	{
		rec_err("can NOT lookup partition %s from partition table.\n", last_part_name);
		return -EPARTITION;
	}
	else
	{
		rec_info("lookup partition %s from partition table success.\n", last_part_name);
	}

	if ((pfirst->u8PartitionStartAddr + pfirst->u8PartitionSize) > (plast->u8PartitionStartAddr))
	{
		rec_err("%s range and %s range have overlap.\n", first_part_name, last_part_name);
		return -EPARTTBL;
	}

	return 0;
}

static int upgrade_partition(partitionread *ptbl)
{

	int ret = 0;
	partitionread *pfirst = ptbl;

	if (ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	while (pfirst != NULL)
	{
		if ((strcmp(pfirst->szPartName, "preloader") == 0 || (strcmp(pfirst->szPartName, "preloader_bk") == 0)))
		{
			rec_info("skip partition %s.has been upgraded.\n", pfirst->szPartName);
			pfirst = pfirst->nextpartition;
			continue;
		}
		if ((strcmp(pfirst->szPartName, "datazone") == 0 || (strcmp(pfirst->szPartName, "datazone_bk") == 0)))
		{
			rec_info("skip partition %s.has been upgraded.\n", pfirst->szPartName);
			pfirst = pfirst->nextpartition;
			continue;
		}
		ret = upgrade_one_partition(pfirst, ISO_ROOT);
		if (ret < 0)
		{
			rec_err("upgrade partition %s fail.\n", pfirst->szPartName);
			return ret;
		}
		rec_info("upgrade partition %s success.\n", pfirst->szPartName);

		pfirst = pfirst->nextpartition;
	}
	rec_info("%s upgrade done.\n");
	return 0;
}

/*
 * upgrade_main_startup() - upgrade MainStartup-A or MainStartup-B
 * @ptbl, partition table.
 * @whence_startup, indicates to upgrade MainStartup-A or MainStartup-B
 *	@whence_startup = 0, upgrade MainStartup-A, otherwise upgrade
 *	MainStartup-B.
 * return 0 if upgrade success, otherwise -1.
 */
#ifdef CONFIG_AB_PART
static int upgrade_main_startup(partitionread *ptbl, int whence_startup)
{

	int ret = 0;
	partitionread *pfirst = NULL;
	partitionread *plast = NULL;
	char first_part_name[PART_NAME_LEN_MAX] = {0};
	char last_part_name[PART_NAME_LEN_MAX] = {0};

	if (ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	if (whence_startup == MAIN_STARTUP_A)
	{
		rec_info("upgrade MainStartup-A.\n");
		strncpy(first_part_name, MAIN_STARTUP_A_FIRST_PART_NAME, PART_NAME_LEN_MAX - 1);
		strncpy(last_part_name, MAIN_STARTUP_A_LAST_PART_NAME, PART_NAME_LEN_MAX - 1);
	}
	else
	{
		rec_info("upgrade MainStartup-B.\n");
		strncpy(first_part_name, MAIN_STARTUP_B_FIRST_PART_NAME, PART_NAME_LEN_MAX - 1);
		strncpy(last_part_name, MAIN_STARTUP_B_LAST_PART_NAME, PART_NAME_LEN_MAX - 1);
	}
	ret = check_main_startup_completeness(ptbl, whence_startup);
	if (ret < 0)
	{
		rec_err("check_main_startup_completeness fail.\n");
		return ret;
	}
	pfirst = lookup_partition_by_name(ptbl, first_part_name);
	if (!pfirst)
	{
		rec_err("can NOT lookup partition %s from partition table.\n", first_part_name);
		return -EPARTITION;
	}
	else
	{
		rec_info("lookup partition %s from partition table success.\n", first_part_name);
	}

	plast = lookup_partition_by_name(ptbl, last_part_name);
	if (!plast)
	{
		rec_err("can NOT lookup partition %s from partition table.\n", last_part_name);
		return -EPARTITION;
	}
	else
	{
		rec_info("lookup partition %s from partition table success.\n", last_part_name);
	}

	while (pfirst != plast->nextpartition)
	{
		ret = upgrade_one_partition(pfirst, ISO_ROOT);
		if (ret < 0)
		{
			rec_err("upgrade partition %s fail.\n", pfirst->szPartName);
			return ret;
		}
		pfirst = pfirst->nextpartition;
	}

	rec_info("%s upgrade done.\n",
			 whence_startup == MAIN_STARTUP_A ? "Startup-A" : "Startup-B");

	return 0;
}

static int upgrade_system_area(partitionread *ptbl, struct safeupg_upg_info *pui)
{
	int ret = 0;
	partitionread *pfirst = NULL;

	if (pui == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	pfirst = lookup_partition_by_name(ptbl, SYSTEM_AREA_FIRST_PART_NAME);
	if (!pfirst)
	{
		rec_err("can NOT lookup partition %s from partition table.\n", SYSTEM_AREA_FIRST_PART_NAME);
		return -EPARTITION;
	}
	else
	{
		rec_info("lookup partition %s from partition table success.\n", SYSTEM_AREA_FIRST_PART_NAME);
	}

	while (pfirst)
	{
		if ((pui->m_systemprog == 0) && check_name_in_sets(systemprog_tbl_name, pfirst->szPartName))
		{
			rec_info("skip %s\n", pfirst->szPartName);
			pfirst = pfirst->nextpartition;
			continue;
		}

		if ((pui->m_app == 0) && check_name_in_sets(app_tbl_name, pfirst->szPartName))
		{
			rec_info("skip %s\n", pfirst->szPartName);
			pfirst = pfirst->nextpartition;
			continue;
		}

		if ((pui->m_userdata == 0) && check_name_in_sets(userdata_tbl_name, pfirst->szPartName))
		{
			rec_info("skip %s\n", pfirst->szPartName);
			pfirst = pfirst->nextpartition;
			continue;
		}

		ret = upgrade_one_partition(pfirst, ISO_ROOT);
		if (ret < 0)
		{
			rec_err("upgrade partition %s fail.\n", pfirst->szPartName);
			return ret;
		}
		pfirst = pfirst->nextpartition;
	}

	rec_info("upgrade done.\n");
	return 0;
}

static int copy_main_startup_partition_info(partitionread *ptbl, int direction)
{
	int ret = -1;
	long long skew = 0;
	partitionread *pentry_src_first = NULL;
	partitionread *pentry_src_last = NULL;
	partitionread *pentry_dest_first = NULL;

	if (ptbl == NULL)
	{
		rec_err("ptbl is NULL.\n");
		ret = -EINVAL;
		goto out;
	}

	if (direction == DIRECTION_A2B)
	{
		rec_info("direction is copy main-startup-a to main-startup-b.\n");
		pentry_src_first = lookup_partition_by_name(ptbl, startup_a_part_name[0]);
		if (pentry_src_first == NULL)
		{
			rec_err("can NOT lookup src %s partition.\n", startup_a_part_name[0]);
			ret = -EPARTITION;
			goto out;
		}

		pentry_src_last = lookup_partition_by_name(ptbl, startup_a_part_name[STARTUP_PART_NUM - 1]);
		if (pentry_src_first == NULL)
		{
			rec_err("can NOT lookup src %s partition.\n", startup_a_part_name[STARTUP_PART_NUM - 1]);
			ret = -EPARTITION;
			goto out;
		}

		pentry_dest_first = lookup_partition_by_name(ptbl, startup_b_part_name[0]);
		if (pentry_dest_first == NULL)
		{
			rec_err("can NOT lookup dest %s partition.\n", startup_b_part_name[0]);
			ret = -EPARTITION;
			goto out;
		}

		skew = pentry_src_last->u8PartitionStartAddr + pentry_src_last->u8PartitionSize - pentry_src_first->u8PartitionStartAddr;

		while (pentry_src_first != pentry_src_last->nextpartition)
		{
			pentry_dest_first->u8PartitionStartAddr = pentry_src_first->u8PartitionStartAddr + skew;
			pentry_dest_first->u8PartitionSize = pentry_src_first->u8PartitionSize;
			pentry_src_first = pentry_src_first->nextpartition;
			pentry_dest_first = pentry_dest_first->nextpartition;
		}
	}
	else if (direction == DIRECTION_B2A)
	{
		rec_info("direction is copy main-startup-b to main-startup-a.\n");
		pentry_src_first = lookup_partition_by_name(ptbl, startup_b_part_name[0]);
		if (pentry_src_first == NULL)
		{
			rec_err("can NOT lookup src %s partition.\n", startup_b_part_name[0]);
			ret = -EPARTITION;
			goto out;
		}

		pentry_src_last = lookup_partition_by_name(ptbl, startup_b_part_name[STARTUP_PART_NUM - 1]);
		if (pentry_src_first == NULL)
		{
			rec_err("can NOT lookup src %s partition.\n", startup_b_part_name[STARTUP_PART_NUM - 1]);
			ret = -EPARTITION;
			goto out;
		}

		pentry_dest_first = lookup_partition_by_name(ptbl, startup_a_part_name[0]);
		if (pentry_dest_first == NULL)
		{
			rec_err("can NOT lookup dest %s partition.\n", startup_a_part_name[0]);
			ret = -EPARTITION;
			goto out;
		}

		skew = pentry_src_first->u8PartitionStartAddr - pentry_dest_first->u8PartitionStartAddr;
		while (pentry_src_first != pentry_src_last->nextpartition)
		{
			pentry_dest_first->u8PartitionStartAddr = pentry_src_first->u8PartitionStartAddr - skew;
			pentry_dest_first->u8PartitionSize = pentry_src_first->u8PartitionSize;
			pentry_src_first = pentry_src_first->nextpartition;
			pentry_dest_first = pentry_dest_first->nextpartition;
		}
	}
	else
	{
		rec_err("direction is error.\n");
		ret = -EINVAL;
		goto out;
	}

	ret = 0;

out:
	return ret;
}
#endif
#ifdef CONFIG_BOOT_MMC
static int __copy_main_startup(int fd, unsigned long long dest_offset,
							   unsigned long long src_offset, unsigned long long len)
{
	int ret = 0;
	off64_t curpos = 0;
	unsigned long long sizer = 0;
	unsigned long long sizew = 0;
	char *buffer = NULL;

	buffer = (char *)malloc(PAGE_SIZE);
	if (buffer == NULL)
	{
		rec_err("malloc fail.\n");
		ret = -ENOMEM;
		goto out;
	}

	while (len > 0)
	{
		src_offset += sizer;
		curpos = lseek64(fd, src_offset, SEEK_SET);
		if (curpos != src_offset)
		{
			rec_err("src lseek64 fail.\n");
			ret = -ESYSCALL;
			goto out_free_buf;
		}
		if (len >= PAGE_SIZE)
		{
			sizer = read(fd, buffer, PAGE_SIZE);
		}
		else
		{
			sizer = read(fd, buffer, len);
		}
		if (sizer < 0)
		{
			rec_err("src read fail.\n");
			ret = -ESYSCALL;
			goto out_free_buf;
		}

		dest_offset += sizew;
		curpos = lseek64(fd, dest_offset, SEEK_SET);
		if (curpos != dest_offset)
		{
			rec_err("dest lseek64 fail.\n");
			ret = -ESYSCALL;
			goto out_free_buf;
		}
		sizew = write(fd, buffer, sizer);
		if (sizew < sizer)
		{
			rec_err("dest write fail.\n");
			ret = -ESYSCALL;
			goto out_free_buf;
		}
		len -= sizer;
	}

	ret = 0;

out_free_buf:
	free(buffer);
out:
	return ret;
}

static int copy_main_startup(partitionread *ptbl,
							 const char *devnode, int direction)
{
	int fd = 0;
	int ret = 0;
	int i = 0;

	const char **dest_part_name = NULL;
	const char **src_part_name = NULL;
	partitionread *pentry_dest = NULL;
	partitionread *pentry_src = NULL;

	if (ptbl == NULL)
	{
		rec_err("ptbl is NULL.\n");
		ret = -EINVAL;
		goto out;
	}

	if (devnode == NULL)
	{
		rec_err("devnode is NULL.\n");
		ret = -EINVAL;
		goto out;
	}

	if (direction == DIRECTION_A2B)
	{
		rec_info("direction is copy main-startup-a to main-startup-b.\n");
		src_part_name = startup_a_part_name;
		dest_part_name = startup_b_part_name;
	}
	else if (direction == DIRECTION_B2A)
	{
		rec_info("direction is copy main-startup-b to main-startup-a.\n");
		src_part_name = startup_b_part_name;
		dest_part_name = startup_a_part_name;
	}
	else
	{
		rec_err("direction is error.\n");
		ret = -EINVAL;
		goto out;
	}

	fd = open(devnode, O_RDWR | O_LARGEFILE);
	if (fd < 0)
	{
		rec_err("Open device(%s) failed, %s.\n", devnode, strerror(errno));
		ret = -ESYSCALL;
		goto out;
	}

	i = 0;
	while (i < STARTUP_PART_NUM)
	{
		pentry_src = lookup_partition_by_name(ptbl, src_part_name[i]);
		if (pentry_src == NULL)
		{
			rec_err("can NOT lookup src %s partition.\n", src_part_name[i]);
			ret = -EPARTITION;
			goto out_close_fd;
		}
		pentry_dest = lookup_partition_by_name(ptbl, dest_part_name[i]);
		if (pentry_dest == NULL)
		{
			rec_err("can NOT lookup dest %s partition.\n", dest_part_name[i]);
			ret = -EPARTITION;
			goto out_close_fd;
		}

		ret = __copy_main_startup(fd, pentry_dest->u8PartitionStartAddr,
								  pentry_src->u8PartitionStartAddr, pentry_src->u8RealDataSize);

		if (ret < 0)
		{
			rec_err("copy %s fail.\n", pentry_src->szPartName);
			goto out_close_fd;
		}
		pentry_dest->u8RealDataSize = pentry_src->u8RealDataSize;
		i++;
	}

	ret = 0;

out_close_fd:
	close(fd);
out:
	return ret;
}

#else /* !CONFIG_BOOT_MMC */

static int __copy_main_startup(partitionread *pentry_dest,
							   partitionread *pentry_src)
{
	int ret = 0;
	unsigned long long sizer = 0;
	unsigned long long sizew = 0;
	unsigned long long realsize = 0;
	unsigned long long src_offset = 0;
	unsigned long long dest_offset = 0;
	unsigned long long tmp = 0;
	char *buffer = NULL;

	if ((pentry_dest == NULL) || (pentry_src == NULL))
	{
		rec_err("ptbl is NULL.\n");
		ret = -EINVAL;
		goto out;
	}

	buffer = (char *)malloc(SIZE_48MB);
	if (buffer == NULL)
	{
		rec_err("malloc fail.\n");
		ret = -ENOMEM;
		goto out;
	}

	realsize = pentry_src->u8RealDataSize;

	while (realsize > 0)
	{
		src_offset += sizer;
		if (realsize >= SIZE_48MB)
		{
			sizer = nand_raw_partition_read_offset_by_safeupg(
				pentry_src->szPartName, buffer,
				0, src_offset,
				SIZE_48MB, pentry_src->u8PartitionSize);
			tmp = SIZE_48MB;
		}
		else
		{
			sizer = nand_raw_partition_read_offset_by_safeupg(
				pentry_src->szPartName, buffer,
				0, src_offset,
				realsize, pentry_src->u8PartitionSize);
			tmp = realsize;
		}
		if (sizer != tmp)
		{
			rec_err("src read from nand fail.\n");
			ret = -ENANDRD;
			goto out_free_buf;
		}

		dest_offset += sizew;
		sizew = nand_raw_partition_write_offset_by_safeupg(
			pentry_dest->szPartName, buffer,
			0, dest_offset,
			sizer, 0,
			pentry_dest->u8PartitionSize);
		if (sizew != sizer)
		{
			rec_err("dest write fail.\n");
			ret = -ENANDWR;
			goto out_free_buf;
		}
		realsize -= sizer;
	}

	ret = 0;

out_free_buf:
	free(buffer);
out:
	return ret;
}

static int copy_main_startup(partitionread *ptbl,
							 const char *devnode, int direction)
{
	/*
	 * devnode is useless for nand.
	 */
	int ret = 0;
	int i = 0;

	const char **dest_part_name = NULL;
	const char **src_part_name = NULL;
	partitionread *pentry_dest = NULL;
	partitionread *pentry_src = NULL;

	if (ptbl == NULL)
	{
		rec_err("ptbl is NULL.\n");
		ret = -EINVAL;
		goto out;
	}

	if (direction == DIRECTION_A2B)
	{
		rec_info("direction is copy main-startup-a to main-startup-b.\n");
		src_part_name = startup_a_part_name;
		dest_part_name = startup_b_part_name;
	}
	else if (direction == DIRECTION_B2A)
	{
		rec_info("direction is copy main-startup-b to main-startup-a.\n");
		src_part_name = startup_b_part_name;
		dest_part_name = startup_a_part_name;
	}
	else
	{
		rec_err("direction is error.\n");
		ret = -EINVAL;
		goto out;
	}

	i = 0;
	while (i < STARTUP_PART_NUM)
	{
		pentry_src = lookup_partition_by_name(ptbl, src_part_name[i]);
		if (pentry_src == NULL)
		{
			rec_err("can NOT lookup src %s partition.\n", src_part_name[i]);
			ret = -EPARTITION;
			goto out;
		}
		pentry_dest = lookup_partition_by_name(ptbl, dest_part_name[i]);
		if (pentry_dest == NULL)
		{
			rec_err("can NOT lookup dest %s partition.\n", dest_part_name[i]);
			ret = -EPARTITION;
			goto out;
		}

		ret = __copy_main_startup(pentry_dest, pentry_src);

		if (ret < 0)
		{
			rec_err("copy %s fail.\n", pentry_src->szPartName);
			goto out;
		}
		pentry_dest->u8RealDataSize = pentry_src->u8RealDataSize;
		i++;
	}

	ret = 0;

out:
	return ret;
}

#endif /* CONFIG_BOOT_MMC */

#ifdef CONFIG_BOOT_MMC
static int unlock_mmcblk_boot_readonly(int whence)
{
#define MMCBLK_BOOT_RD_NAME_LEN 48
	char file[MMCBLK_BOOT_RD_NAME_LEN] = {0};
	char buf[4] = {0};
	int fd = 0;
	int n = 0;
	int ret = 0;

	if ((whence != 0) && (whence != 1))
	{
		rec_err("whence(%d) is error.\n", whence);
		whence = 1;
	}

	snprintf(file, MMCBLK_BOOT_RD_NAME_LEN, "/sys/block/mmcblk0boot%d/force_ro", whence);
	rec_info("file name is %s\n", file);

	fd = open(file, O_RDWR);
	if (fd < 0)
	{
		rec_err("file(%s) open fail, %s\n", file, strerror(errno));
		return -ESYSCALL;
	}

	buf[0] = '0';
	buf[1] = 0;

	n = write(fd, buf, strlen(buf));
	if (n != strlen(buf))
	{
		rec_err("write error, write(%d) but expect (%d)\n", n, strlen(buf));
		ret = -ESYSCALL;
	}
	close(fd);

	return 0;
}

static int __write_preloader(char *buf, int whence)
{
	int fd = 0;
	int ret = 0;
	int n = 0;
	char devnode[MMCBLK_BOOT_DEVNNODE_NAME] = {0};

	if (buf == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	ret = unlock_mmcblk_boot_readonly(whence);
	if (ret < 0)
	{
		rec_err("unlock mmcblk boot readonly fail.\n");
		return ret;
	}

	snprintf(devnode, MMCBLK_BOOT_DEVNNODE_NAME, "/dev/mmcblk0boot%d", whence);
	rec_info("devnode is %s\n", devnode);

	fd = open(devnode, O_RDWR);
	if (fd < 0)
	{
		rec_err("devnode(%s) open fail, %s\n", devnode, strerror(errno));
		return -ESYSCALL;
	}

	if (lseek(fd, 0, SEEK_SET) != 0)
	{
		rec_err("lseek fail.\n");
		close(fd);
		return -ESYSCALL;
	}

	// preloader max possible size is 64KB,
	// 512 is preloader header size.
	n = write(fd, buf, PRELOADER_MAX_TOTAL_SIZE);
	if (n != PRELOADER_MAX_TOTAL_SIZE)
	{
		rec_err("write fail. %d is written but expect %d\n", n, PRELOADER_MAX_TOTAL_SIZE);
		close(fd);
		return -ESYSCALL;
	}
	close(fd);

	return 0;
}

/*
 * write_preloader() - upgrade preloader
 * @whence indicates to upgrade area.
 *@whence=0, uprade preloader0 which is located at emmc boot area.
 *@whence=1, upgrade preloader1 which is located at emmc user area.
 * when SoC is power up, the Romcode will verify preloader0 first, if the preloader0
 * verify pass by romcode, the preloader0 is used, otherwise, romcode will verify
 * preloader1 from emmc user area, if preloader1 is verified pass, the preloader1 is used,
 * otherwise, SoC startup fails.
 */
static int write_preloader(partitionread *ptbl, int whence)
{
	int ret = 0;
	int fd = 0;
	int n = 0;
	unsigned long long file_len = 0;
	char pl_file[80] = {0};
	char *buf = NULL;

	if (!ptbl)
	{
		rec_err("ptbl is NULL.\n");
		ret = -EINVAL;
		goto out;
	}

	if ((whence != WHENCE_PRELOADER_1) && (whence != WHENCE_PRELOADER_2))
	{
		rec_err("whence(%d) is error.\n", whence);
		whence = WHENCE_PRELOADER_2;
	}

	if (strcmp(ptbl->szPartName, "preloader"))
	{
		rec_err("the ptbl is NOT preloader partition table.\n");
		ret = -EPARTTBL;
		goto out;
	}

	if (!ptbl->szImageFileName)
	{
		rec_err("preloader ImageName is NULL.\n");
		ret = -EPARTTBL;
		goto out;
	}

	ret = get_file_length(ptbl->szImageFileName, &file_len);
	if (ret < 0)
	{
		rec_err("can't get file (%s) length.\n", ptbl->szImageFileName);
		ret = -EFILELEN;
		goto out;
	}
	rec_info("file(%s) length is %lld\n", ptbl->szImageFileName, file_len);

	if (file_len > (PRELOADER_MAX_TOTAL_SIZE - 512))
	{
		rec_err("preloader image file length(0x%x)is larger than 63.5KB.\n", file_len);
		ret = -EFILELEN;
		goto out;
	}

	strcpy(pl_file, ISO_ROOT);
	strcat(pl_file, ptbl->szImageFileName);
	rec_info("preloader image file path:%s\n", pl_file);

	fd = open(pl_file, O_RDONLY);
	if (fd < 0)
	{
		rec_err("open %s fail, %s.\n", pl_file, strerror(errno));
		ret = -ESYSCALL;
		goto out;
	}

	buf = (char *)malloc(PRELOADER_MAX_TOTAL_SIZE);
	if (!buf)
	{
		rec_err("malloc fail.\n");
		ret = -ENOMEM;
		goto out_close_fd;
	}
	memset(buf, 0, PRELOADER_MAX_TOTAL_SIZE);

	/*
	 * skip 512bytes, the 512bytes is used to store BOOTLOADER_HEADER.
	 */
	n = read(fd, (buf + 512), file_len);
	if (n != file_len)
	{
		rec_err("read fail, read(%d), expect(%d)\n", n, (int)file_len);
		ret = -ESYSCALL;
		goto out_free_buf;
	}

	create_bootloader_header((char *)buf, (char *)(buf + 512), PRELOADER_SIZE, 1);
	ret = __write_preloader(buf, whence);
	if (ret < 0)
	{
		rec_err("__write_preloader fail.\n");
		goto out_free_buf;
	}

	rec_info("write preloader success.\n");
	ptbl->u8RealDataSize = ALIGN(file_len, 512);
	ret = 0;

out_free_buf:
	free(buf);
out_close_fd:
	close(fd);
out:
	return ret;
}

static int preloader_readback_check(int whence)
{
	char devnode[MMCBLK_BOOT_DEVNNODE_NAME] = {0};
	BOOTL_HEADER *pbl_header_emmc = NULL;
	BOOTL_HEADER *pbl_header = NULL;
	char *buf = NULL;
	int fd = 0;
	int ret = 0;
	int n = 0;

	snprintf(devnode, MMCBLK_BOOT_DEVNNODE_NAME, "/dev/mmcblk0boot%d", whence);
	rec_info("devnode is %s\n", devnode);

	pbl_header_emmc = (BOOTL_HEADER *)malloc(sizeof(BOOTL_HEADER) * REPLICATION_NUMBER);
	if (pbl_header_emmc == NULL)
	{
		rec_err("malloc for pbl_header_emmc fail\n");
		ret = -ENOMEM;
		goto out;
	}
	memset(pbl_header_emmc, 0, sizeof(BOOTL_HEADER) * REPLICATION_NUMBER);

	fd = open(devnode, O_RDONLY);
	if (fd < 0)
	{
		rec_err("devnode(%s) open fail, %s\n", devnode, strerror(errno));
		ret = -ESYSCALL;
		goto out_free_blh;
	}

	if (lseek(fd, 0, SEEK_SET) != 0)
	{
		rec_err("lseek-0 fail.\n");
		ret = -ESYSCALL;
		goto out_close_fd;
	}

	buf = (char *)malloc(PRELOADER_SIZE + 512);
	if (!buf)
	{
		rec_err("malloc fail.\n");
		ret = -ENOMEM;
		goto out_close_fd;
	}

	n = read(fd, pbl_header_emmc, sizeof(BOOTL_HEADER) * REPLICATION_NUMBER);
	if (n != sizeof(BOOTL_HEADER) * REPLICATION_NUMBER)
	{
		rec_err("read fail, read(%d), expect(%d)\n", n, sizeof(BOOTL_HEADER) * REPLICATION_NUMBER);
		ret = -ESYSCALL;
		goto out_free_buf;
	}

	if (lseek(fd, 512, SEEK_SET) != 512)
	{ // skip bootloader header
		rec_err("lseek-512 fail.\n");
		ret = -ESYSCALL;
		goto out_free_buf;
	}

	n = read(fd, (buf + 512), PRELOADER_SIZE);
	if (n != PRELOADER_SIZE)
	{
		rec_err("read fail, read(%d), expect(%d)\n", n, PRELOADER_SIZE);
		ret = -ESYSCALL;
		goto out_free_buf;
	}

	ret = verify_bootloader_header(pbl_header_emmc, 1);
	if (ret == 0)
	{
		rec_err("verify_bootloader_header fail.\n");
		ret = -ERDBACKCHK;
		goto out_free_buf;
	}
	rec_info("verify_bootloader_header sucess.\n");

	create_bootloader_header(buf, (buf + 512), PRELOADER_SIZE, 1);
	pbl_header = (BOOTL_HEADER *)buf;

	if (pbl_header_emmc->checksum != pbl_header->checksum)
	{
		rec_err("bl_header checksum fail, emmc checksum(0x%x), calc checksum(0x%x)\n",
				pbl_header_emmc->checksum, pbl_header->checksum);
		ret = -ERDBACKCHK;
		goto out_free_buf;
	}

	ret = 0;

out_free_buf:
	free(buf);
out_close_fd:
	close(fd);
out_free_blh:
	free(pbl_header_emmc);
out:
	return ret;
}

static int __upgrade_preloader(partitionread *ptbl, int whence)
{
	int ret = 0;
	partitionread *ptemp = NULL;

	if (ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	ptemp = lookup_partition_by_name(ptbl, "preloader");
	if (!ptemp)
	{
		rec_err("can NOT lookup preloader partition.\n");
		return -EPARTITION;
	}

	ret = write_preloader(ptemp, whence);

	if (ret < 0)
	{
		rec_err("write preloader fail.\n");
		return ret;
	}

	ret = preloader_readback_check(whence);
	if (ret < 0)
	{
		rec_err("preloader readback check fail.\n");
		return ret;
	}

	return 0;
}

static int get_boot_cfg(void)
{
	int fd = 0;
	int ret = 0;
	int boot_cfg = -1;
	char buf[8] = {0};

	fd = open(BOOT_CFG_PATH, O_RDONLY);
	if (fd <= 0)
	{
		rec_err("open %s fail,%s.\n", BOOT_CFG_PATH, strerror(errno));
		return -ESYSCALL;
	}

	ret = read(fd, buf, 8);
	if (ret <= 0)
	{
		rec_err("read fail,%s.\n", strerror(errno));
		close(fd);
		return -ESYSCALL;
	}
	else
	{
		rec_info("read sucess, ret=%d\n", ret);
	}
	close(fd);

	// catch the boot_part_cfg value.
	if (buf[0] >= '0' && buf[0] <= '2')
	{
		boot_cfg = buf[0] - '0';
		rec_info("boot_cfg = %d\n", boot_cfg);
		return boot_cfg;
	}
	else
	{
		rec_err("boot_cfg = %d\n", boot_cfg);
		return -EINVAL;
	}
}

static int put_boot_cfg(int boot_cfg)
{
	int fd = 0;
	int ret = 0;
	char buf[8] = {0};

	if (boot_cfg < 0 || boot_cfg > 2)
	{
		rec_err("boot_cfg has wrong value(%d)\n", boot_cfg);
		return -EINVAL;
	}

	fd = open(BOOT_CFG_PATH, O_RDWR);
	if (fd <= 0)
	{
		rec_err("open %s fail,%s.\n", BOOT_CFG_PATH, strerror(errno));
		return -ESYSCALL;
	}

	buf[0] = '0' + boot_cfg;
	buf[1] = 0;

	ret = write(fd, buf, 1);
	if (ret != 1)
	{
		rec_err("write fail,%s.\n", strerror(errno));
		close(fd);
		return -ESYSCALL;
	}
	else
	{
		rec_info("write sucess, ret=%d\n", ret);
	}
	close(fd);

	return 0;
}

static int upgrade_preloader(partitionread *ptbl)
{
	int boot_cfg = -1;
	int ret = -1;
	unsigned long long file_len = 0;
	partitionread *ptbl_pl = NULL;

	rec_info("preloader upgrade begin...\n");

	if (ptbl == NULL)
	{
		rec_err("ptbl is NULL.\n");
		return -EINVAL;
	}

	boot_cfg = get_boot_cfg();
	if (boot_cfg < 0)
	{
		rec_err("get boot_cfg fail.\n");
		return boot_cfg;
	}

	if (boot_cfg == 0)
	{
		rec_warn("boot_cfg is 0, means boot mode is turn-off.\n");
		boot_cfg = 1;
	}

	rec_info("from boot%d bootup\n", boot_cfg);

	if (boot_cfg == 1)
	{
		/*
		 * from boot1 bootup
		 */
		ret = __upgrade_preloader(ptbl, WHENCE_PRELOADER_2);
		if (ret < 0)
		{
			rec_err("upgrade preloader-2 fail.\n");
			goto out;
		}

		ret = put_boot_cfg(2); // switch to boot2
		if (ret < 0)
		{
			rec_err("put_boot_cfg(2) fail.\n");
			goto out;
		}

		ret = __upgrade_preloader(ptbl, WHENCE_PRELOADER_1);
		if (ret < 0)
		{
			rec_err("upgrade preloader-1 fail.\n");
			goto out;
		}
		ret = put_boot_cfg(1); // switch to boot1
		if (ret < 0)
		{
			rec_err("put_boot_cfg(1) fail.\n");
			goto out;
		}
	}
	else
	{
		/*
		 * from boot2 bootup
		 */
		rec_info("qiyundebug-------1\n");
		ret = __upgrade_preloader(ptbl, WHENCE_PRELOADER_1);
		rec_info("qiyundebug-----2\n");
		if (ret < 0)
		{
			rec_err("upgrade preloader-1 fail.\n");
			goto out;
		}
		ret = put_boot_cfg(1); // switch to boot1
		if (ret < 0)
		{
			rec_err("put_boot_cfg(1) fail.\n");
			goto out;
		}

		ret = __upgrade_preloader(ptbl, WHENCE_PRELOADER_2);
		if (ret < 0)
		{
			rec_err("upgrade preloader-2 fail.\n");
			goto out;
		}
	}
	rec_info("qiyundebug22\n");

	ret = 0;

out:
	pthread_mutex_lock(&g_safeupg_mutex);
	if (ret == 0)
	{
		rec_info("preloader upgrade done\n");

		ptbl_pl = lookup_partition_by_name(ptbl, "preloader");
		if (ptbl_pl == NULL)
		{
			rec_err("can't lookup preloader partition.\n");
			pthread_mutex_unlock(&g_safeupg_mutex);
			return 0;
		}

		ret = get_file_length(ptbl_pl->szImageFileName, &file_len);
		if (ret < 0)
		{
			rec_err("can't get file (%s) length.\n", ptbl_pl->szImageFileName);
			ret = 0;
			file_len = 0;
		}

		g_safeupg_status_self.size_upgrade_done += file_len;
		g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].flag = UPGRADE_FLAG_DONE | UPGRADE_FLAG_SUCCESS;
		strncpy(g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].part_name, ptbl_pl->szPartName, PART_NAME_LEN_MAX - 1);
		ptbl_pl->u4LastPartition |= UPDATE_FLAG_DONE;
	}
	else
	{
		g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].flag = UPGRADE_FLAG_DONE;
		strncpy(g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].part_name, ptbl_pl->szPartName, PART_NAME_LEN_MAX - 1);
	}

	g_safeupg_status_self.part_count_upgrade_done++;
	if (g_safeupg_status_self.part_count_upgrade_done >= PART_NUM_MAX)
	{
		rec_err("partition num >= %d\n", PART_NUM_MAX);
		g_safeupg_status_self.part_count_upgrade_done--;
	}
	pthread_mutex_unlock(&g_safeupg_mutex);

	return ret;
}

#else /* !CONFIG_BOOT_MMC */

static int upgrade_nand_preloader_by_file(partitionread *ptbl, long long *psize_total)
{
	int ret = 0;
	int fd = 0;
	int n = 0;
	unsigned long long file_len = 0;
	uint64_t result = 0;
	char *buf = NULL;
	char pl_file[80] = {0};

	if (strlen(ptbl->szImageFileName) == 0)
	{
		rec_err("preloader ImageName is NULL.\n");
		ret = -EPARTTBL;
		goto out;
	}

	ret = get_file_length(ptbl->szImageFileName, &file_len);
	if (ret < 0)
	{
		rec_err("can't get file (%s) length.\n", ptbl->szImageFileName);
		ret = -EFILELEN;
		goto out;
	}
	rec_info("file(%s) length is %lld\n", ptbl->szImageFileName, file_len);

	if (file_len > (PRELOADER_MAX_TOTAL_SIZE - 512))
	{
		rec_err("preloader image file length(0x%x)is larger than 63.5KB.\n", file_len);
		ret = -EFILELEN;
		goto out;
	}
	buf = (char *)malloc(PRELOADER_MAX_TOTAL_SIZE);
	if (!buf)
	{
		rec_err("malloc fail.\n");
		ret = -ENOMEM;
		goto out_close_fd;
	}
	memset(buf, 0, PRELOADER_MAX_TOTAL_SIZE);
#ifdef CONFIG_NO_ZIP_UPDATE
	strcpy(pl_file, ISO_ROOT);
	strcat(pl_file, ptbl->szImageFileName);
	rec_info("preloader image file path:%s\n", pl_file);
	fd = open(pl_file, O_RDONLY);
	if (fd < 0)
	{
		rec_err("open %s fail, %s.\n", pl_file, strerror(errno));
		ret = -ESYSCALL;
		goto out;
	}
	/*
	 * skip 512bytes, the 512bytes is used to store BOOTLOADER_HEADER.
	 */
	n = read(fd, (buf + 512), file_len);
	if (n != file_len)
	{
		rec_err("read fail, read(%d), expect(0x%llx)\n", n, file_len);
		ret = -ESYSCALL;
		goto out_free_buf;
	}
#else
	strcpy(pl_file, ptbl->szImageFileName);
	if (read_file_from_zip(pl_file, (buf + 512), file_len)) {
		rec_err("read file(%s) from zip \n", ptbl->szImageFileName);
		return -1;
	}

#endif



	ret = create_bootloader_header((char *)buf, (char *)(buf + 512),
								   PRELOADER_SIZE, 0);
	if (ret < 0)
	{
		rec_err("create_bootloader_header fail.\n");
		goto out_free_buf;
	}

	/*
	 *upg preloader
	 */
	result = nand_raw_partition_write_offset_by_safeupg(ptbl->szPartName, buf,
														0, 0,
														PRELOADER_MAX_TOTAL_SIZE,
														PRELOADER_MAX_TOTAL_SIZE,
														ptbl->u8PartitionSize);
	if (result < 0)
	{
		rec_err("nand_raw_common_write_with_rwctrl fail\n");
		ret = -ENANDWR;
		goto out_free_buf;
	}

	ret = nand_preloader_readback_check(ptbl, 1);
	if (ret < 0)
	{
		rec_err("nand preloader readback check fail.\n");
		goto out_free_buf;
	}

	rec_info("write preloader success.\n");

	*psize_total = file_len;
	ret = 0;

out_free_buf:
	free(buf);
out_close_fd:
	close(fd);
out:
	return ret;
}

static int upgrade_preloader(partitionread *ptbl)
{
	partitionread *ptbl_pld = NULL;
	partitionread *ptbl_pld_bk = NULL;
	partitionread *ptbl_pld_tmp = NULL;
	int ret = 0;
	int i = 0;
	long long size_total = 0;

	ptbl_pld = lookup_partition_by_name(ptbl, "preloader");
	if (ptbl_pld == NULL)
	{
		rec_err("can't look up preloader partition.\n");
		return -1;
	}

	ptbl_pld_bk = lookup_partition_by_name(ptbl, "preloader_bk");
	if (ptbl_pld_bk == NULL)
	{
		rec_err("can't look up preloader_bk partition.\n");
		return -1;
	}

	for (i = 0; i < 2; i++)
	{
		if (i == 0)
			ptbl_pld_tmp = ptbl_pld;
		else
			ptbl_pld_tmp = ptbl_pld_bk;

		ret = upgrade_nand_preloader_by_file(ptbl_pld_tmp, &size_total);

		// pthread_mutex_lock(&g_safeupg_mutex);
		if (ret < 0)
		{
			rec_err("upg preloader%d fail.\n", i);
			ret = -1;

			g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].flag = UPGRADE_FLAG_DONE;
			strncpy(g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].part_name, ptbl_pld_tmp->szPartName, PART_NAME_LEN_MAX - 1);
		}
		else
		{
			rec_note("upg preloader%d success.\n", i);
			ret = 0;

			g_safeupg_status_self.size_upgrade_done += size_total;
			g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].flag = UPGRADE_FLAG_DONE | UPGRADE_FLAG_SUCCESS;
			strncpy(g_safeupg_status_self.one_partition_status[g_safeupg_status_self.part_count_upgrade_done].part_name, ptbl_pld_tmp->szPartName, PART_NAME_LEN_MAX - 1);
			ptbl_pld_tmp->u4LastPartition |= UPDATE_FLAG_DONE;
			ptbl_pld_tmp->u8RealDataSize = ALIGN(size_total, 512ULL);
		}

		g_safeupg_status_self.part_count_upgrade_done++;
		if (g_safeupg_status_self.part_count_upgrade_done >= PART_NUM_MAX)
		{
			rec_err("partition num >= %d\n", PART_NUM_MAX);
			g_safeupg_status_self.part_count_upgrade_done--;
		}
		// pthread_mutex_unlock(&g_safeupg_mutex);

		if (ret < 0)
		{
			rec_err("upg preloader%d fail.\n", i);
			return -1;
		}
	}

	rec_note("preloader0 and preloader1 upg success.\n");
	return 0;
}

#endif /* CONFIG_BOOT_MMC */

static struct safeupg_upg_info ui;
static struct datazone_info datazone;
static struct safeupg_bootloader_message bcb;
static struct safeupg_partitionhead partition_head;
static partitionread *pemmc_table;
static partitionread *pboard_table;
static partitionread *pxml_table;
static partitionread *pnew_table;
static uint32_t bootflag;

static inline int is_coreprog_upg(struct safeupg_upg_info *pui)
{
	if (!pui)
		return 0;
	return pui->m_coreprog;
}

static inline int is_systemprog_upg(struct safeupg_upg_info *pui)
{
	if (!pui)
		return 0;
	return pui->m_systemprog;
}

static inline int is_app_upg(struct safeupg_upg_info *pui)
{
	if (!pui)
		return 0;
	return pui->m_app;
}

static inline int is_userdata_upg(struct safeupg_upg_info *pui)
{
	if (!pui)
		return 0;
	return pui->m_userdata;
}

static inline int is_latest_exception(struct safeupg_upg_info *pui)
{
	if (!pui)
		return 0;
	return pui->exception;
}
static inline int is_need_verifyimg(struct safeupg_upg_info *pui)
{
	if (!pui)
		return 0;
	return pui->verifyimg;
}

static inline int is_all_upg(struct safeupg_upg_info *pui)
{
	return is_coreprog_upg(pui) &&
		   is_systemprog_upg(pui) &&
		   is_app_upg(pui) &&
		   is_userdata_upg(pui);
}

static inline int is_coreprog_upg_only(struct safeupg_upg_info *pui)
{
	return is_coreprog_upg(pui) &&
		   !is_systemprog_upg(pui) &&
		   !is_app_upg(pui) &&
		   !is_userdata_upg(pui);
}

static inline int is_app_upg_only(struct safeupg_upg_info *pui)
{
	return is_app_upg(pui) &&
		   !is_systemprog_upg(pui) &&
		   !is_coreprog_upg(pui) &&
		   !is_userdata_upg(pui);
}

#ifdef CONFIG_AB_PART
static void keep_main_startup_size(partitionread *pnew_tbl, partitionread *pemmc_tbl)
{
	long long new_startup_a_end_addr = 0;
	long long emmc_startup_a_end_addr = 0;
	long long skew = 0;
	partitionread *ptemp = NULL;

	if (!pnew_tbl)
	{
		rec_err("pnew_tbl is NULL.\n");
		return;
	}

	if (!pemmc_tbl)
	{
		rec_err("pemmc_tbl is NULL.\n");
		return;
	}

	new_startup_a_end_addr = get_main_startup_a_end_addr(pnew_tbl);
	if (new_startup_a_end_addr < 0)
	{
		rec_err("get new table startup a end address fail.\n");
		return;
	}

	emmc_startup_a_end_addr = get_main_startup_a_end_addr(pemmc_tbl);
	if (emmc_startup_a_end_addr < 0)
	{
		rec_err("get emmc table startup a end address fail.\n");
		return;
	}

	if (new_startup_a_end_addr > emmc_startup_a_end_addr)
	{
		rec_err("xmlStartupASize > emmcStartupASize.\n");
		return;
	}
	else if (new_startup_a_end_addr == emmc_startup_a_end_addr)
	{
		rec_info("xmlStartupASize == emmcStartupASize, Do NOT need to adjust.\n");
		return;
	}

	skew = emmc_startup_a_end_addr - new_startup_a_end_addr;

	rec_info("xmlStartupASize < emmcStartupASize, need to adjust to keep.\n");
	rec_info("skew is 0x%llx\n", skew);

	/*
	 * add skew to partition size of startup-a last partition(recovery partition),
	 * at the same time, the skew is also need to add the start address of the partitions which are
	 * following startup-a last partition.
	 */
	ptemp = lookup_partition_by_name(pnew_tbl, MAIN_STARTUP_A_LAST_PART_NAME);
	if (!ptemp)
	{
		rec_err("can NOT lookup " MAIN_STARTUP_A_LAST_PART_NAME "partition.\n");
		return;
	}

	// add skew to partition size of startup-a last partition(recovery partition).
	ptemp->u8PartitionSize += skew;

	ptemp = ptemp->nextpartition;
	if (!ptemp)
	{
		rec_warn("it looks like partions amount is too small.\n");
		return;
	}
	while (ptemp->nextpartition)
	{
		ptemp->u8PartitionStartAddr += skew;
	}

	rec_info("after adjust, print the partition info begin--------------->");
	dumpallpartitioninfo(pnew_tbl);
	rec_info("after adjust, print the partition info end<---------------");

	rec_info("done");
}

#define KEEP_STARTUP_A_PI 0
#define KEEP_STARTUP_B_PI 1

static int keep_startup_partition_info(partitionread *pdup_tbl,
									   partitionread *pemmc_tbl, int keep_who)
{
	partitionread *pentry_first = NULL;
	partitionread *pentry_last = NULL;
	partitionread *pentry_new = NULL;

	if (pdup_tbl == NULL || pemmc_tbl == NULL)
	{
		rec_err("ptbl is NULL.\n");
		return -EINVAL;
	}

	if (keep_who == KEEP_STARTUP_A_PI)
	{
		pentry_first = lookup_partition_by_name(pemmc_tbl, startup_a_part_name[0]);
		if (pentry_first == NULL)
		{
			rec_err("can NOT lookup %s partition.\n", startup_a_part_name[0]);
			return -EPARTITION;
		}
		pentry_last = lookup_partition_by_name(pemmc_tbl, startup_a_part_name[STARTUP_PART_NUM - 1]);
		if (pentry_last == NULL)
		{
			rec_err("can NOT lookup %s partition.\n", startup_a_part_name[STARTUP_PART_NUM - 1]);
			return -EPARTITION;
		}
		pentry_new = lookup_partition_by_name(pdup_tbl, startup_a_part_name[0]);
		if (pentry_new == NULL)
		{
			rec_err("can NOT lookup %s partition.\n", startup_a_part_name[0]);
			return -EPARTITION;
		}
	}
	else
	{
		pentry_first = lookup_partition_by_name(pemmc_tbl, startup_b_part_name[0]);
		if (pentry_first == NULL)
		{
			rec_err("can NOT lookup %s partition.\n", startup_b_part_name[0]);
			return -EPARTITION;
		}
		pentry_last = lookup_partition_by_name(pemmc_tbl, startup_b_part_name[STARTUP_PART_NUM - 1]);
		if (pentry_last == NULL)
		{
			rec_err("can NOT lookup %s partition.\n", startup_b_part_name[STARTUP_PART_NUM - 1]);
			return -EPARTITION;
		}
		pentry_new = lookup_partition_by_name(pdup_tbl, startup_b_part_name[0]);
		if (pentry_new == NULL)
		{
			rec_err("can NOT lookup %s partition.\n", startup_b_part_name[0]);
			return -EPARTITION;
		}
	}

	while (pentry_first != pentry_last->nextpartition)
	{
		pentry_new->u8PartitionStartAddr = pentry_first->u8PartitionStartAddr;
		pentry_new->u8PartitionSize = pentry_first->u8PartitionSize;
		pentry_new->u8RealDataSize = pentry_first->u8RealDataSize;

		pentry_first = pentry_first->nextpartition;
		pentry_new = pentry_new->nextpartition;
	}

	return 0;
}

static partitionread *duplicate_partition_info(partitionread *ptbl)
{
	partitionread *pinfo = NULL;
	partitionread *ptblnewpre = NULL;
	partitionread *pnew_tbl = NULL;

	while (ptbl)
	{
		pinfo = (partitionread *)malloc(sizeof(partitionread));
		if (pinfo == NULL)
		{
			rec_err("No memory!\n");
			goto err;
		}

		memcpy(pinfo, ptbl, sizeof(partitionread));
		pinfo->nextpartition = NULL;
		if (pnew_tbl == NULL)
			pnew_tbl = pinfo;
		else
			ptblnewpre->nextpartition = pinfo;
		ptblnewpre = pinfo;

		ptbl = ptbl->nextpartition;
	}

	return pnew_tbl;

err:
	if (pnew_tbl)
		freetblmemory(pnew_tbl);
	return NULL;
}

static int upgrade_by_way_new_startup_larger(partitionread *pnew_tbl, partitionread *pemmc_tbl)
{
	int ret = 0;
	partitionread *pdup_tbl = NULL;

	if (pnew_tbl == NULL || pemmc_tbl == NULL)
	{
		rec_err("ptbl is NULL.\n");
		return -EINVAL;
	}

	ret = write_bcb_to_set_bootflag(&bcb, WHENCE_MAIN, BOOTFLAG_STARTUP_A);
	if (ret < 0)
	{
		rec_err("write bcb to set bootflag=Startup-A fail-1 \n");
		goto out;
	}

	ret = upgrade_main_startup(pnew_tbl, MAIN_STARTUP_B);
	if (ret < 0)
	{
		rec_err("upgrade main startup-b fail\n");
		goto out;
	}

	ret = upgrade_system_area(pnew_tbl, &ui);
	if (ret < 0)
	{
		rec_err("upgrade system area fail\n");
		goto out;
	}

	ret = upgrade_datazone(pnew_tbl, WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("upgrade main datazone fail\n");
		goto out;
	}

	ret = upgrade_datazone(pnew_tbl, WHENCE_BACKUP);
	if (ret < 0)
	{
		rec_err("upgrade bk datazone fail\n");
		goto out;
	}

	pdup_tbl = duplicate_partition_info(pnew_tbl);
	if (pdup_tbl == NULL)
	{
		rec_err("duplicate partition info fail.\n");
		ret = -EPARTTBL;
		goto out;
	}
	rec_info("==== print duplicate partition info begin =====\n");
	dumpallpartitioninfo(pdup_tbl);
	rec_info("==== print duplicate partition info end =====\n");

	ret = keep_startup_partition_info(pdup_tbl, pemmc_tbl, KEEP_STARTUP_A_PI);
	if (ret < 0)
	{
		rec_err("keep startup-a partition info fail.\n");
		goto out_free;
	}

	// note, use pdup_tbl to keep Startup-A partition info consistent with emmc partition table.
	ret = upgrade_partition_info(&partition_head, pdup_tbl, WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("upgrade main partition info fail-1.\n");
		goto out_free;
	}

	ret = upgrade_partition_info(&partition_head, pdup_tbl, WHENCE_BACKUP);
	if (ret < 0)
	{
		rec_err("upgrade bk partition info fail-1.\n");
		goto out_free;
	}

	ret = write_bcb_to_set_bootflag(&bcb, WHENCE_MAIN, BOOTFLAG_STARTUP_B);
	if (ret < 0)
	{
		rec_err("write bcb to set bootflag=Startup-B fail\n");
		goto out_free;
	}

	ret = upgrade_main_startup(pnew_tbl, MAIN_STARTUP_A);
	if (ret < 0)
	{
		rec_err("upgrade main startup-a fail\n");
		goto out_free;
	}

	ret = upgrade_partition_info(&partition_head, pnew_tbl, WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("upgrade main partition info fail-2.\n");
		goto out_free;
	}

	ret = upgrade_partition_info(&partition_head, pnew_tbl, WHENCE_BACKUP);
	if (ret < 0)
	{
		rec_err("upgrade bk partition info fail-2.\n");
		goto out_free;
	}

	ret = write_bcb_to_set_bootflag(&bcb, WHENCE_MAIN, BOOTFLAG_STARTUP_A);
	if (ret < 0)
	{
		rec_err("write bcb to set bootflag=Startup-A fail-2\n");
		goto out_free;
	}

	ret = 0;

out_free:
	freetblmemory(pdup_tbl);
out:
	return ret;
}

static int upgrade_by_way_new_startup_smaller(partitionread *pnew_tbl, partitionread *pemmc_tbl)
{
	int ret = 0;
	partitionread *pdup_tbl = NULL;

	if (pnew_tbl == NULL || pemmc_tbl == NULL)
	{
		rec_err("ptbl is NULL.\n");
		return -EINVAL;
	}

	ret = write_bcb_to_set_bootflag(&bcb, WHENCE_MAIN, BOOTFLAG_STARTUP_B);
	if (ret < 0)
	{
		rec_err("write bcb to set bootflag=Startup-B fail\n");
		goto out;
	}

	// after Startup-A upgrade done, pemmc_tbl is updated.
	ret = upgrade_main_startup(pemmc_tbl, MAIN_STARTUP_A);
	if (ret < 0)
	{
		rec_err("upgrade main startup-b fail \n");
		goto out;
	}

	// only startup-a partition info is updated, others keep.
	ret = upgrade_partition_info(&partition_head, pemmc_tbl, WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("upgrade main partition info fail-1.\n");
		goto out;
	}

	ret = upgrade_partition_info(&partition_head, pemmc_tbl, WHENCE_BACKUP);
	if (ret < 0)
	{
		rec_err("upgrade bk partition info fail-1.\n");
		goto out;
	}

	ret = write_bcb_to_set_bootflag(&bcb, WHENCE_MAIN, BOOTFLAG_STARTUP_A);
	if (ret < 0)
	{
		rec_err("write bcb to set bootflag=Startup-A fail\n");
		goto out;
	}

	// copy startup-a partition info of pemmc_tbl to pnew_tbl.
	ret = keep_startup_partition_info(pnew_tbl, pemmc_tbl, KEEP_STARTUP_A_PI);
	if (ret < 0)
	{
		rec_err("keep startup-a partition info fail.\n");
		goto out;
	}

	ret = upgrade_main_startup(pnew_tbl, MAIN_STARTUP_B);
	if (ret < 0)
	{
		rec_err("upgrade main startup-b fail\n");
		goto out;
	}

	ret = upgrade_system_area(pnew_tbl, &ui);
	if (ret < 0)
	{
		rec_err("upgrade system area fail\n");
		goto out;
	}

	ret = upgrade_datazone(pnew_tbl, WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("upgrade main datazone fail\n");
		goto out;
	}

	ret = upgrade_datazone(pnew_tbl, WHENCE_BACKUP);
	if (ret < 0)
	{
		rec_err("upgrade bk datazone fail\n");
		goto out;
	}

	ret = upgrade_partition_info(&partition_head, pnew_tbl, WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("upgrade main partition info fail-2.\n");
		goto out;
	}

	ret = upgrade_partition_info(&partition_head, pnew_tbl, WHENCE_BACKUP);
	if (ret < 0)
	{
		rec_err("upgrade bk partition info fail-2.\n");
		goto out;
	}

	ret = 0;

out:
	return ret;
}
#endif

static int check_partition_tbl_change(const char **partname,
									  partitionread *pxml_tbl, partitionread *pemmc_tbl)
{
	partitionread *ptbl_xml_find = NULL;
	partitionread *ptbl_emmc_find = NULL;
	const char **name = partname;

	if (pxml_tbl == NULL || pemmc_tbl == NULL)
	{
		rec_err("ptbl is NULL.\n");
		return 0;
	}

	while (*name)
	{
		ptbl_xml_find = lookup_partition_by_name(pxml_tbl, *name);
		ptbl_emmc_find = lookup_partition_by_name(pemmc_tbl, *name);

		if (!ptbl_xml_find || !ptbl_emmc_find)
		{
			return 1; // changed
		}

		if ((ptbl_emmc_find->u8PartitionStartAddr == ptbl_xml_find->u8PartitionStartAddr) &&
			(ptbl_emmc_find->u8PartitionSize == ptbl_xml_find->u8PartitionSize))
		{
			rec_info("partition %s NOT change.\n", *name);
		}
		else
		{
			rec_note("partition %s changed.\n", *name);
			return 1;
		}
		name++;
	}

	return 0;
}

static int check_coreprog_partiton_tbl_change(partitionread *pxml_tbl, partitionread *pemmc_tbl)
{
	return check_partition_tbl_change(coreprog_tbl_name, pxml_tbl, pemmc_tbl);
}

static int check_systemprog_partiton_tbl_change(partitionread *pxml_tbl, partitionread *pemmc_tbl)
{
	return check_partition_tbl_change(systemprog_tbl_name, pxml_tbl, pemmc_tbl);
}

static int check_app_partiton_tbl_change(partitionread *pxml_tbl, partitionread *pemmc_tbl)
{
	return check_partition_tbl_change(app_tbl_name, pxml_tbl, pemmc_tbl);
}

static int check_userdata_partiton_tbl_change(partitionread *pxml_tbl, partitionread *pemmc_tbl)
{
	return check_partition_tbl_change(userdata_tbl_name, pxml_tbl, pemmc_tbl);
}

static int check_file_exist_by_part_name(const char **partname, partitionread *ptbl)
{
	partitionread *ptbl_find = NULL;
	const char **name = partname;
	int ret = 0;

	if (ptbl == NULL)
	{
		rec_err("ptbl is NULL.\n");
		return -EINVAL;
	}

	while (*name)
	{
		ptbl_find = lookup_partition_by_name(ptbl, *name);

		if (!ptbl_find)
			return 0;

		ret = check_file_exist_for_one_table(ptbl_find, ISO_ROOT);
		if (!ret)
			return 0;

		name++;
	}

	return 1;
}

static int check_coreprog_file_exsit(partitionread *ptbl)
{
	return check_file_exist_by_part_name(coreprog_tbl_name, ptbl);
}

static int check_systemprog_file_exsit(partitionread *ptbl)
{
	return check_file_exist_by_part_name(systemprog_tbl_name, ptbl);
}

static int check_app_file_exsit(partitionread *ptbl)
{
	return check_file_exist_by_part_name(app_tbl_name, ptbl);
}

static int check_userdata_file_exsit(partitionread *ptbl)
{
	return check_file_exist_by_part_name(userdata_tbl_name, ptbl);
}

/*
 * if only app upg is chosen, it needs to match version.
 * if only coreprogram is chosen, it needs to match version.
 */
static int version_handle(struct safeupg_upg_info *pui)
{
	int ret = 0;

	if (pui == NULL)
	{
		rec_err("ptbl is NULL.\n");
		return -EINVAL;
	}

	if (is_coreprog_upg_only(pui))
	{
		ret = version_match();
		if (ret < 0)
		{
			rec_err("version match fail for coreprog upg only\n");
			return ret;
		}
	}

	if (is_app_upg_only(pui))
	{
		ret = version_match();
		if (ret < 0)
		{
			rec_err("version match fail for app upg only\n");
			return ret;
		}
	}

	return 0;
}

static int bcb_is_from_main;
static int new_tbl_eq_emmc_tbl;

static int get_last_exception_info(void)
{
	int ret;
	struct safeupg_bootloader_message bcb;
	struct safeupg_upg_info ui;
	int need_check_bk = 0;

	ret = safeupg_read_bcb(&bcb, WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("can't safeupg_read_bcb main bcb.\n");
		need_check_bk = 1;
	}
	else
	{
		ret = check_bcb_tag_checksum(&bcb);
		if (ret == 0)
		{
			rec_warn("main bcb checksum check fail.\n");
			need_check_bk = 1;
		}
		else
		{
			rec_info("main bcb checksum check pass.\n");
			need_check_bk = 0;
			bcb_is_from_main = 1;
		}
	}

	if (need_check_bk)
	{
		/*
		 * get bk bcb and check its checksum.
		 */
		memset(&bcb, 0, sizeof(struct safeupg_bootloader_message));
		ret = safeupg_read_bcb(&bcb, WHENCE_BACKUP);
		if (ret < 0)
		{
			rec_err("can't safeupg_read_datazone backup bcb.\n");
			goto out;
		}
		else
		{
			ret = check_bcb_tag_checksum(&bcb);
			if (ret == 0)
			{
				rec_warn("backup bcb checksum check fail.\n");
				ret = -EBCB;
				goto out;
			}
			else
			{
				rec_info("backup bcb checksum check pass.\n");
				bcb_is_from_main = 0;
			}
		}
	}

	bootflag = get_bcb_bootflag(&bcb);
	rec_note("bootflag is %s.\n", bootflag == BOOTFLAG_STARTUP_A ? "Startup-A" : "Startup-B");

	// get_upg_info_from_bcb(&bcb, &ui);

	return ui.exception;
out:
	return ret;
}

static void copy_image_name(partitionread *ptbl_dst, partitionread *ptbl_src)
{
	partitionread *ptmp = NULL;
	partitionread *pcurrent_dst = ptbl_dst;
	int cnt = 0;

	if (ptbl_dst == NULL || ptbl_src == NULL)
		return;

	rec_info("begin ...\n");
	while (pcurrent_dst)
	{
		ptmp = lookup_partition_by_name(ptbl_src, pcurrent_dst->szPartName);
		if (ptmp == NULL)
		{
			rec_warn("can NOT look up %s in ptbl_src/xml_table\n", pcurrent_dst->szPartName);
			// continue;
		}
		else
		{
#ifdef NEW_PARTITION_DESIGN
			memcpy(pcurrent_dst->szImageFileName, ptmp->szImageFileName, 48);
#else
			memcpy(pcurrent_dst->szImageFileName, ptmp->szImageFileName, 40);
#endif
			cnt++;
		}
		pcurrent_dst = pcurrent_dst->nextpartition;
	}
	rec_info("copy count:%d \n", cnt);
	rec_info("end\n");
}

static int get_bootup_common(void)
{
	int ret = -1;
	int need_check_bk = 0;
	int dz_is_from_main = 0;
	int pi_is_from_main = 0;
	int board_type = 0;
	char xml_path[128];
	const char *pxml_file = NULL;
	int full_upg = 0;

	/*
	 * get main datazone and check its checksum.
	 */
	memset(&datazone, 0, sizeof(struct datazone_info));
	ret = safeupg_read_datazone(&datazone, WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("can't safeupg_read_datazone main datazone.\n");
		need_check_bk = 1;
	}
	else
	{
		ret = check_datazone_checksum(&datazone);
		if (ret == 0)
		{
			rec_warn("main datazone checksum check fail.\n");
			need_check_bk = 1;
		}
		else
		{
			rec_info("main datazone checksum check pass.\n");
			need_check_bk = 0;
			dz_is_from_main = 1;
		}
	}

	if (need_check_bk)
	{
		/*
		 * get bk datazone and check its checksum.
		 */
		memset(&datazone, 0, sizeof(struct datazone_info));
		ret = safeupg_read_datazone(&datazone, WHENCE_BACKUP);
		if (ret < 0)
		{
			rec_err("can't safeupg_read_datazone backup datazone.\n");
			goto out;
		}
		else
		{
			ret = check_datazone_checksum(&datazone);
			if (ret == 0)
			{
				rec_warn("backup datazone checksum check fail.\n");
				ret = -EDATAZONE;
				goto out;
			}
			else
			{
				rec_info("backup datazone checksum check pass.\n");
				dz_is_from_main = 0;
			}
		}
	}

	/*
	 * get main bcb and check its checksum.
	 */
	need_check_bk = 0;
	memset(&bcb, 0, sizeof(struct safeupg_bootloader_message));
	ret = safeupg_read_bcb(&bcb, WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("can't safeupg_read_bcb main bcb.\n");
		need_check_bk = 1;
	}
	else
	{
		ret = check_bcb_tag_checksum(&bcb);
		if (ret == 0)
		{
			rec_warn("main bcb checksum check fail.\n");
			need_check_bk = 1;
		}
		else
		{
			rec_info("main bcb checksum check pass.\n");
			need_check_bk = 0;
			bcb_is_from_main = 1;
		}
	}

	if (need_check_bk)
	{
		/*
		 * get bk bcb and check its checksum.
		 */
		memset(&bcb, 0, sizeof(struct safeupg_bootloader_message));
		ret = safeupg_read_bcb(&bcb, WHENCE_BACKUP);
		if (ret < 0)
		{
			rec_err("can't safeupg_read_datazone backup bcb.\n");
			goto out;
		}
		else
		{
			ret = check_bcb_tag_checksum(&bcb);
			if (ret == 0)
			{
				rec_warn("backup bcb checksum check fail.\n");
				ret = -EBCB;
				goto out;
			}
			else
			{
				rec_info("backup bcb checksum check pass.\n");
				bcb_is_from_main = 0;
			}
		}
	}

	bootflag = get_bcb_bootflag(&bcb);
	rec_note("bootflag is %s.\n", bootflag == BOOTFLAG_STARTUP_A ? "Startup-A" : "Startup-B");

	/*
	 * get main partition info and check its checksum.
	 */

	pemmc_table = safeupg_read_partition_info(&partition_head, WHENCE_MAIN);
	if (!pemmc_table)
	{
		rec_warn("can't read main partition info.\n");
		need_check_bk = 1;
	}
	else
	{
		ret = check_partition_info_checksum(&partition_head, pemmc_table);
		if (ret == 0)
		{
			rec_warn("main partitin info checksum check fail.\n");
			free(pemmc_table);
			need_check_bk = 1;
		}
		else
		{
			rec_info("main partitin info checksum check pass.\n");

			pi_is_from_main = 1;
		}
	}
	pboard_table = pemmc_table;

	/*
	 * pemmc_table is got from emmc, the nextpatition filed of each partitionread
	 * need to update. the adjust_nextpartition_field() is design for this.
	 */
	adjust_nextpartition_field(pemmc_table);

	rec_info("=============== print emmc/nand table begin===============\n");
	dumpallpartitioninfo(pemmc_table);
	rec_info("===============print emmc/nand table end===============\n");
#if 0
	/*
	 * get xml partition table from xml file.
	 */
	snprintf(xml_path, 128, "%s%s", ISO_ROOT, XML_FILE);
	rec_info("xml_path:%s\n", xml_path);
	ret = check_is_file_exist(xml_path);
	if (!ret)
	{
		rec_err("xml_path %s is not exsit .\n", xml_path);
		ret = -ENOFILE;
		goto out_free_emmc_tbl;
	}
	pxml_table = read_partition_info_from_xml_file(xml_path);
	if (!pxml_table)
	{
		rec_err("can NOT get xml table from xml file.\n");
		ret = -EPARTTBL;
		goto out_free_emmc_tbl;
	}

	ret = adjust_xml_partition_info_size(pxml_table, board_type);
	if (ret < 0)
	{
		rec_err("adjust_xml_partition_info_size fail.\n");
		goto out_free_xml_tbl;
	}
	rec_info("adjust_xml_partition_info_size success.\n");

	rec_info("===============print xml table begin===============\n");
	dumpallpartitioninfo(pxml_table);
	rec_info("===============print xml table end===============\n");

	//full_upg = check_partition_table_changed(pemmc_table, pxml_table);
	if (0)
	{
		rec_dbg("go to full upg\n");
		pnew_table = pxml_table;
		ret = decide_which_partitions_need_upgrade(pnew_table);
		if (ret < 0)
		{
			printf("decide_which_partitions_need_upgrade fail!\r\n");
			ret = -ENOFILE;
			goto out_free_xml_tbl;
		}
		ret = check_files_exist_for_upgrade(pnew_table, ISO_ROOT);
		if (ret == 0)
		{
			rec_err("there are some files NOT exist for upg.\n");
			ret = -ENOFILE;
			goto out_free_xml_tbl;
		}

		rec_info("===============print new table begin===============\n");
		dumpallpartitioninfo(pnew_table);
		rec_info("===============print new table end===============\n");

		ret = check_coreprog_file_exsit(pnew_table);
		if (!ret)
		{
			rec_err("it's all upg, but some coreprog files do NOT exsit, check them.\n");
			ret = -ENOFILE;
			goto out_free_new_tbl;
		}
	} else {
		rec_dbg("go to partial upg \n");	
	}
#endif
	rec_dbg("go to partial upg \n");
	pnew_table = pemmc_table;
	new_tbl_eq_emmc_tbl = 1;

	return 0;

out_free_new_tbl:
	freetblmemory(pnew_table);
	pnew_table = NULL;
out_free_xml_tbl:
	freetblmemory(pxml_table);
	pxml_table = NULL;
out_free_emmc_tbl:
	free(pemmc_table);
	pemmc_table = NULL;
out:
	return ret;
}

static int safeupg_finish_recovery(void);

static int safeupg_from_udisk(void)
{
	int ret = -1;
	int i = 0;
	int last_upgrade_exception = 0;
	uint32_t chksum = 0;
	long long new_startup_a_end_addr = 0;
	long long emmc_startup_a_end_addr = 0;
	partitionread *ptemp = NULL;
	const char **name = NULL;
#ifdef NEW_PARTITION_DESIGN
	int fdwp;
#endif
	rec_info("safeupg_from_udisk begin...\n");

#ifdef NEW_PARTITION_DESIGN
	fdwp = open_for_writeprotect();
	if (fdwp < 0)
	{
		rec_err("open_for_writeprotect fail\n");
		goto out_free_new_tbl;
	}

	rec_info("before upgrade auto dump info:\n");
	dump_writeprotect_region(fdwp);
	ret = clear_writeprotect(fdwp);
	if (ret < 0)
	{
		rec_err("clear_writeprotect fail.\n");
		goto out_free_new_tbl;
	}
#endif /* NEW_PARTITION_DESIGN */
	ret = nand_clear_all_protect();
	if (ret < 0){
		rec_err("nand_clear_all_protect fail.\n");
		goto out_free_new_tbl;
	}

	ret = calc_total_size_need_upgrade(pnew_table, &ui);
	if (ret < 0) {
		rec_err("calc_total_size_need_upgrade fail.\n");
		goto out_free_new_tbl;
	}


	 /* upgrade preloader */

	ret = upgrade_preloader(pnew_table);
	if (ret < 0)
	{
		rec_err("upgrade preloader fail.\n");
		goto out_free_new_tbl;
	}	

	ret = upgrade_partition(pnew_table);
	if (ret < 0)
	{
		rec_err("upgrade partition fail.\n");
		goto out_free_new_tbl;
	}
	// not update datazone part
	ret = upgrade_datazone(pnew_table, WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("upgrade main datazone fail\n");
		goto out_free_new_tbl;
	}
	ret = upgrade_datazone(pnew_table, WHENCE_BACKUP);
	if (ret < 0)
	{
		rec_err("upgrade bk datazone fail\n");
		goto out_free_new_tbl;
	}
	// note, update part table to datazone
	ret = upgrade_partition_info(&partition_head, pnew_table, WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("upgrade main partition info fail-1.\n");
		goto out_free_new_tbl;
	}
	// note, update part table to datazonebk
	ret = upgrade_partition_info(&partition_head, pnew_table, WHENCE_BACKUP);
	if (ret < 0)
	{
		rec_err("upgrade bk partition info fail-1.\n");
		goto out_free_new_tbl;
	}

	ret = 0;
out_done:
	rec_info("upgrade success, enjoy it ...\n");

	safeupg_finish_recovery();

#ifdef NEW_PARTITION_DESIGN
	update_writeprotect_region(fdwp, pnew_table);
	rec_info("after upgrade auto dump info:\n");
	dump_writeprotect_region(fdwp);
#endif
out_free_new_tbl:
	if (new_tbl_eq_emmc_tbl == 0)
		freetblmemory(pnew_table);
	pnew_table = NULL;
out_free_xml_tbl:
	freetblmemory(pxml_table);
	pxml_table = NULL;
out_free_emmc_tbl:
	free(pemmc_table);
	pemmc_table = NULL;
out:

#ifdef NEW_PARTITION_DESIGN
	close_for_writeprotect(fdwp);
#endif

	return ret;
}

static struct pthread_arg_info_t
{
	RecoveryUpdateModule *prum;
} pthread_arg_info;

static void *safeupg_thread_fn(void *arg)
{
	struct pthread_arg_info_t *parg;
	int ret;

	rec_info("do safeupg_thread_fn .\n");

	if (arg == NULL)
		return (void *)-EINVAL;

	rec_info("do safeupg_thread_fn .\n");

	parg = (struct pthread_arg_info_t *)arg;

	ret = safeupg_from_udisk();

	pthread_mutex_lock(&g_safeupg_mutex);
	if (ret >= 0)
	{
		rec_info("upgrade_finish = 1 \n");
		g_safeupg_status_self.upgrade_finish = 1;
	}
	else
	{
		g_safeupg_status_self.upgrade_finish = ret;
	}
	pthread_mutex_unlock(&g_safeupg_mutex);

	pthread_exit((void *)ret);
}

static void *safeupg_no_thread_fn(void)
{
	int ret = 0;
	ret = safeupg_from_udisk();
	if (ret >= 0)
	{
		rec_info("upgrade_finish = 1 \n");
	}
}

int export_safeupg_get_updatethread_state(void)
{
	return g_safeupg_status_to_qt.upgrade_finish;
}

int export_safeupg_upgrade(void)
{
	int ret = -1;

	BUILD_BUG_ON(sizeof(struct datazone_info) != 512);

	rec_info("begin to upgrade  continue.\n");

	if (export_safeupg_iso_file_exist())
	{
		rec_info("iso file exist.\n");
	}
	else
	{
		rec_info("iso file not exist.\n");
		return -ENOFILE;
	}
#if 0
	if((ret = export_safeupg_iso_file_md5_verify()) < 0){
		rec_err("md5 check fail.\n");
		return ret;
	}else {
		rec_info("md5 check ok.\n");
	}
#endif

	if (strlen(iso_file_name) == 0)
	{
		rec_warn("iso_file_name is NULL.\n");
		return -EINVAL;
	}

	ret = get_bootup_common(); // full upgrade ? partital upgrade? and xml table & emmc table
	if (ret < 0)
	{
		rec_err("get_bootup_common return fail.\n");
		return ret;
	}
	rec_info("bcb.laststatus[0]=%d, bcb.laststatus[1]=%d\n",
			 bcb.laststatus[0], bcb.laststatus[1]);
	memset(&g_safeupg_status_self, 0, sizeof(struct partition_upgrade_status_t));
	memset(&g_safeupg_status_to_qt, 0, sizeof(struct partition_upgrade_status_t));

	pthread_arg_info.prum = NULL;

	ret = pthread_mutex_init(&g_udisk_hotplug_mutex, NULL);
	if (ret != 0)
	{
		rec_err("failed init hotplug mutex:%s!\n", strerror(errno));
		return -EPTHREAD;
	}
	rec_info("debug 2.\n");

	ret = pthread_mutex_init(&g_safeupg_mutex, NULL);
	if (ret != 0)
	{
		rec_err("failed init safeupg mutex:%s!\n", strerror(errno));
		return -EPTHREAD;
	}

	g_udisk_hotplug_status = 1;

	safeupg_no_thread_fn();
	/* ret = pthread_create(&g_safeupg_thread_tid, NULL, safeupg_thread_fn, (void *)&pthread_arg_info);
	 if (ret != 0) {
		 rec_err("can't create thread:%s!\n", strerror(errno));
		 return -EPTHREAD;
	 }*/

	return 0;
}

static int safeupg_finish_recovery(void)
{
	int ret = 0;
	uint32_t chksum = 0;
	struct safeupg_bootloader_message bcb;

#ifndef CONFIG_BOOT_MMC
	rec_info("qydeub.\n");
	ret = nand_clear_all_protect();
	if (ret < 0)
	{
		rec_err("nand_clear_all_protect fail.\n");
		return ret;
	}
#endif

	memset(&bcb, 0, sizeof(safeupg_bootloader_message));
	set_bcb_tags(&bcb);
	bcb.bootflag = BOOTFLAG_STARTUP_A;

	chksum = calc_bcb_checksum(&bcb);
	put_bcb_checksum(&bcb, chksum);

	ret = safeupg_write_bcb(&bcb, WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("safeupg_write_bcb fail.\n");
		return ret;
	}

	ret = safeupg_bcb_readback_check(WHENCE_MAIN);
	if (ret < 0)
	{
		rec_err("safeup_bcb_readback_check fail.\n");
		return ret;
	}

	ret = safeupg_write_bcb(&bcb, WHENCE_BACKUP);
	if (ret < 0)
	{
		rec_err("safeupg_write_bcbbk fail.\n");
		return ret;
	}

	ret = safeupg_bcb_readback_check(WHENCE_BACKUP);
	if (ret < 0)
	{
		rec_err("safeup_bcb_readback_check fail.\n");
		return ret;
	}
	return 0;
}

void export_safeupg_finish_recovery()
{
	int ret;
#ifdef CONFIG_BOOT_MMC
#ifdef NEW_PARTITION_DESIGN
	int fdwp;
	struct wp_cmd_arg arg = {0};
#endif

#ifdef NEW_PARTITION_DESIGN
	fdwp = open("/dev/misc-sd", O_RDONLY);
	if (fdwp < 0)
	{
		rec_err("open /dev//misc-sd failed\n");
		goto reboot;
	}

	dump_writeprotect_region(fdwp);
	/*clear all write protect*/
	arg.wp_action = WP_CLEAR_AND_SAVE;
	ret = ioctl(fdwp, MSDC_EMMC_WRITE_PROTECT, &arg);
	if (ret)
	{
		rec_err("clear and save all wp fail,ret:%d\n", ret);
		close(fdwp);
		goto reboot;
	}
	else
	{
		rec_info("clear and save all wp success\n");
	}
	rec_info("after clear dump:\n");
	dump_writeprotect_region(fdwp);
#endif
#endif

	ret = get_last_exception_info();
	if (ret < 0)
	{
		rec_err("get last exception info failed\n");
	}
	else
	{
		rec_info("@@@@ last exception : %d @@@@\n", ret);
		if (!ret)
		{
			safeupg_finish_recovery();
		}
	}

#ifdef CONFIG_BOOT_MMC
#ifdef NEW_PARTITION_DESIGN
	arg.wp_action = WP_RESTORE;
	ret = ioctl(fdwp, MSDC_EMMC_WRITE_PROTECT, &arg);
	if (ret)
	{
		rec_err("restore all wp fail,ret:%d\n", ret);
		close(fdwp);
	}
	else
	{
		rec_info("restore all wp success\n");
	}
	rec_info("after update dump\n");
	dump_writeprotect_region(fdwp);
	close(fdwp);
#endif

reboot:
	rec_info("begin to reboot \n");
#endif
	system("reboot");
}
