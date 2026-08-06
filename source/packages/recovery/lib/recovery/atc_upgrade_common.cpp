#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include "recovery.h"
#include "atc_update.h"
#include "checksum.h"
#include "err_num.h"
#include "atc_upgrade_common.h"
#include "roots.h"
#include "atc_safe_upgrade.h"
#include "nandupgrade.h"
#include "mz.h"
#include "mz_os.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"
#include "mz_strm_mem.h"

char ntfs_mount_device[NTFS_MOUNT_DEVICE_LEN] = "/dev/sda1";

static void put_checksum32(char *dest, uint32_t chksum)
{
	if (dest == NULL)
	{
		rec_err("dest is NULL.\n");
		return;
	}

	// TODO, confirm how to checksum.
	memcpy(dest, &chksum, 4);
}

/**
 * get_checksum32() -get checksum from @src
 *
 * get 4 bytes data from@src starting, translate them
 * to 32bit checksum.
 */
static uint32_t get_checksum32(char *src)
{
	uint32_t chksum = 0;

	if (src == NULL)
	{
		rec_err("src is NULL.\n");
		return 0;
	}

	// TODO, confirm how to checksum.
	memcpy(&chksum, src, 4);

	return chksum;
}

int hotplug_device_tune(int id, char *str)
{
	Volume *v = NULL;
	int len;

	if (str == NULL)
	{
		rec_err("str is NULL .\n");
		return -1;
	}

	if (id == 1)
	{
		// usb
		v = volume_for_path(UDISK_ROOT);
	}
	else
	{
		// TODO, sdcard
		// v = volume_for_path(...);
	}

	if (v == NULL)
	{
		rec_warn("unknown volume for %s.\n", UDISK_ROOT);
		return -1;
	}
	rec_info("v->device=%s,udisk_root=%s,v->mount_point=%s.\n", v->device, UDISK_ROOT, v->mount_point);

	len = strlen(str);

	// strlen("/dev/sda1")
	if (len > strlen(v->device) + 1)
	{
		rec_warn("len(%d) is larger than v->device len(%d)", len, strlen(v->device));
		return -1;
	}

	strcpy((char *)v->device, str);
	strncpy(ntfs_mount_device, str, NTFS_MOUNT_DEVICE_LEN);
	rec_info("v->device=%s,str=%s.\n", v->device, str);
	return 0;
}

int get_emmc_total_size(unsigned long long *ptotal_sz)
{
	int fd;
	int i;
	int ret;
	char buf[36];
	unsigned long long emmc_size = 0;

	if (ptotal_sz == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -1;
	}

	fd = open("/sys/block/mmcblk0/size", O_RDONLY);
	if (fd <= 0)
	{
		rec_err("open /sys/block/mmcblk0/size fail,%s.\n", strerror(errno));
		return -1;
	}

	ret = read(fd, buf, 32);
	if (ret <= 0)
	{
		rec_err("read /sys/block/mmcblk0/size fail,%s.\n", strerror(errno));
		return -1;
	}
	else
	{
		rec_info("read mmcblk0/size, ret=%d\n", ret);
	}

	buf[ret] = 0;
	emmc_size = strtoll(buf, NULL, 10);
	emmc_size = emmc_size * 512;
	rec_info("emmc size is 0x%llx\n\n", emmc_size);

	*ptotal_sz = emmc_size;
	return 0;
}

int traverse_zip_file(const char *file, unsigned long long *file_len) {
	void *zip_reader = NULL;
    mz_zip_file *file_info = NULL;
    const char *extract_file_name = NULL;
    // 使用Minizip处理ZIP文件
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
			*file_len = (int64_t)file_info->uncompressed_size;
			rec_info("[qiyun debug] extract_file_name=%s, file_len:%llu\n",extract_file_name, file_info->uncompressed_size);
		}
	}

	if (extract_file_name ==NULL) {
		rec_err("Failed to find file: %s in zip:%s\n", file, RECOVERY_UPDATE_ZIP_NAME);
		mz_zip_reader_close(zip_reader);
		mz_zip_reader_delete(&zip_reader);
		return -ENOENT;
	}

	mz_zip_reader_close(zip_reader);
	mz_zip_reader_delete(&zip_reader);
	return 1;
}

int get_file_len(const char *path, unsigned long long *plen)
{
	struct stat64 buf;

	if ((path == NULL) || (plen == NULL))
	{
		rec_err("parameter is NULL .\n");
		return -1;
	}

	if (traverse_zip_file(path, plen) < 0)
	{
		rec_err("traverse_zip_file fail.\n");
		return -1;
	}
#ifdef COMFIG_NO_ZIP_UPDATE
	if (stat64(path, &buf) >= 0)
	{
		*plen = buf.st_size;
		return 0;
	}
	else
	{
		rec_err("stat64 fail, %s\n", strerror(errno));
		return -1;
	}
#endif
	return 0;
}

int is_file_exist_by_pathname(const char *file_path)
{
	int ret = 0;

	if (file_path != NULL)
	{
		if (access(file_path, F_OK) == 0)
		{
			ret = 1;
		}
	}

	return ret;
}

int check_name_in_sets(const char **namesets, const char *name)
{
	const char **tmp = NULL;

	if (!namesets || !name)
		return 0;

	tmp = namesets;

	while (*tmp)
	{
		if (strcmp(*tmp, name) == 0)
			return 1;
		tmp++;
	}

	return 0;
}

void freetblmemory(partitionread *ptbl)
{
	partitionread *ptemp = NULL;

	while (ptbl)
	{
		ptemp = ptbl;
		ptbl = ptbl->nextpartition;
		free(ptemp);
	}
	ptemp = NULL;

	return;
}

partitionread *lookup_partition_by_name(partitionread *ptbl, const char *name)
{
	if (ptbl == NULL || name == NULL)
	{
		rec_err("ptbl or name is NULL!\n");
		return NULL;
	}

	while (ptbl)
	{
		if (strcmp(ptbl->szPartName, name) == 0)
		{
			rec_info("found %s.\n", name);
			break;
		}
		else
		{
			ptbl = ptbl->nextpartition;
		}
	}

	return ptbl;
}

#ifdef CONFIG_BOOT_MMC
int read_partition_head(struct safeupg_partitionhead *phead, const char *devnode, unsigned long offset)
{
	int fd = 0;
	off_t curpos = 0;
	int n = 0;

	if (phead == NULL)
	{
		rec_err("phead is NULL.\n");
		return -EINVAL;
	}
	if (devnode == NULL)
	{
		rec_err("devnode is NULL.\n");
		return -EINVAL;
	}

	rec_info("--------------- start addr:0x%lx-----------------\r\n", offset);

	fd = open(devnode, O_RDWR);
	if (fd < 0)
	{
		rec_err("open block device(%s) failed, %s.\n", devnode, strerror(errno));
		return -ESYSCALL;
	}

	curpos = lseek(fd, offset, SEEK_SET);
	if (curpos != offset)
	{
		rec_err("lseek fail, %s.\n", strerror(errno));
		close(fd);
		return -ESYSCALL;
	}

	n = read(fd, phead, sizeof(struct safeupg_partitionhead));
	if (n != sizeof(struct safeupg_partitionhead))
	{
		rec_err("block_read-1 fail result=%d, expcet=%d\r\n", n, sizeof(struct safeupg_partitionhead));
		close(fd);
		return -ESYSCALL;
	}
	rec_info("block_read success  result=%d\r\n", n);
	rec_info("---------------end-------------------\r\n");

	close(fd);
	return 0;
}

partitionread *read_partition_info(struct safeupg_partitionhead *phead,
								   const char *devnode, unsigned long offset)
{
	int fd = 0;
	off_t curpos = 0;
	int n = 0;
	char *bufpartinfo = NULL;
	unsigned long addr = 0;

	if (phead == NULL)
	{
		rec_err("phead is NULL.\n");
		return NULL;
	}

	if (devnode == NULL)
	{
		rec_err("devnode is NULL.\n");
		return NULL;
	}

	rec_info("---------------start addr:0x%lx-----------------\r\n", offset);

	fd = open(devnode, O_RDWR);
	if (fd < 0)
	{
		rec_err("Open block device(%s) failed, %s.\n", devnode, strerror(errno));
		return NULL;
	}

	addr = offset;
	curpos = lseek(fd, addr, SEEK_SET);
	if (curpos != addr)
	{
		rec_err("lseek-1 fail, %s.\n", strerror(errno));
		close(fd);
		return NULL;
	}

	n = read(fd, phead, sizeof(safeupg_partitionhead));
	if (n != sizeof(safeupg_partitionhead))
	{
		rec_err("block_read-1 fail result=%d, expect=%d\r\n", n, sizeof(safeupg_partitionhead));
		close(fd);
		return NULL;
	}
	rec_info("block_read-1 success  result=%d\r\n", n);

	rec_info("block_read blockcnt=%d\r\n", phead->blockcnt);

	bufpartinfo = (char *)malloc(phead->blockcnt * PTBL_BLOCK_SIZE);
	if (bufpartinfo == NULL)
	{
		rec_err("malloc failed.\n");
		close(fd);
		return NULL;
	}

	addr = addr + PTBL_BLOCK_SIZE;
	curpos = lseek(fd, addr, SEEK_SET);
	if (curpos != addr)
	{
		rec_err("lseek-2 fail, %s.\n", strerror(errno));
		close(fd);
		free(bufpartinfo);
		return NULL;
	}

	n = read(fd, bufpartinfo, phead->blockcnt * PTBL_BLOCK_SIZE);
	if (n != phead->blockcnt * PTBL_BLOCK_SIZE)
	{
		rec_err("block_read-2 fail result=%d\r\n", n);
		close(fd);
		free(bufpartinfo);
		return NULL;
	}

	close(fd);

	rec_info("---------------end-------------------\r\n");

	return (partitionread *)bufpartinfo;
}

/*
 * write_partition_info() -write partition info to flash.
 * /----------------------/
 * /                                            /
 * / partitionhead(512Bytes)      /
 * /                                            /
 * /----------------------/
 * /                                            /
 * / partitionread                        /
 * /                                            /
 * /----------------------/
 */
int write_partition_info(struct safeupg_partitionhead *phead, partitionread *ptbl,
						 const char *devnode, unsigned long offset)
{
	char *buf = NULL, *p = NULL;
	int n = 0;
	int ret = 0;
	partitionread *ptmppart = NULL;
	int fd = 0;
	off_t curpos = 0;
	uint32_t chksum = 0;
	unsigned long addr = 0;

	if (phead == NULL || ptbl == NULL)
	{
		rec_err("No partition table to be written.\n");
		ret = -EINVAL;
		goto out;
	}

	if (devnode == NULL)
	{
		rec_err("devnode is NULL.\n");
		ret = -EINVAL;
		goto out;
	}

	memset(phead, 0, sizeof(struct safeupg_partitionhead));
	ptmppart = ptbl;
	while (ptmppart)
	{
		n++;
		ptmppart = ptmppart->nextpartition;
	}

	phead->blockcnt = (n * sizeof(partitionread) + (PTBL_BLOCK_SIZE - 1)) / PTBL_BLOCK_SIZE;
	phead->u4Signature = ATC_PTBL_SIGN;
	phead->u4Version = ATC_PARTITION_VER;
	phead->nextpartition = NULL;

	buf = (char *)malloc(phead->blockcnt * PTBL_BLOCK_SIZE);
	if (buf == NULL)
	{
		rec_err("malloc fail.\n");
		ret = -ENOMEM;
		goto out;
	}

	fd = open(devnode, O_RDWR);
	if (fd < 0)
	{
		rec_err("Open block device(%s) failed.\n", devnode);
		ret = -ESYSCALL;
		goto out_free;
	}

	memset(buf, 0, phead->blockcnt * PTBL_BLOCK_SIZE);
	ptmppart = ptbl;
	p = buf;

	rec_info("phead.blockcnt=%d\r\n", phead->blockcnt);

	while (ptmppart)
	{
		// printf("<writepartitioninfotoflash> szPartName:%s,nextpartition:0x%X \r\n",pcurpart->szPartName,pcurpart->nextpartition);
		if (ptmppart->nextpartition)
			ptmppart->u4LastPartition = 0;
		else
			ptmppart->u4LastPartition = 1;

		memcpy(p, ptmppart, sizeof(partitionread));
		// TODO, comfirm how to calc partition table checksum.
		chksum += checksum32(0, (char *)ptmppart, sizeof(partitionread));
		p += sizeof(partitionread);
		ptmppart = ptmppart->nextpartition;
	}

	addr = offset + PTBL_BLOCK_SIZE;
	curpos = lseek(fd, addr, SEEK_SET);
	if (curpos != addr)
	{
		rec_err("lseek-1 fail. %s\n", strerror(errno));
		ret = -ESYSCALL;
		goto out_close;
	}

	n = write(fd, buf, phead->blockcnt * PTBL_BLOCK_SIZE);
	if ((phead->blockcnt * PTBL_BLOCK_SIZE) != n)
	{
		rec_err("write-1 failed. write size(%d) real size(%d)\r\n", phead->blockcnt * PTBL_BLOCK_SIZE, n);
		ret = -ESYSCALL;
		goto out_close;
	}

	rec_info("checksum: 0x%x\n", chksum);
	put_checksum32(phead->checksum, chksum);
	memset(buf, 0, phead->blockcnt * PTBL_BLOCK_SIZE);
	memcpy(buf, phead, sizeof(struct safeupg_partitionhead));
	addr = offset;
	curpos = lseek(fd, addr, SEEK_SET);
	if (curpos != addr)
	{
		rec_err("lseek-2 fail, %s\n", strerror(errno));
		ret = -ESYSCALL;
		goto out_close;
	}
	n = write(fd, buf, PTBL_BLOCK_SIZE);
	if (n != PTBL_BLOCK_SIZE)
	{
		rec_err("write-2 failed fail result=%d\r\n", n);
		ret = -ESYSCALL;
		goto out_close;
	}

	ret = 0;
	rec_info("---------------succeed--------------\r\n");

out_close:
	close(fd);
out_free:
	free(buf);
out:
	return ret;
}

#else /* !CONFIG_BOOT_MMC */

int read_partition_head(struct safeupg_partitionhead *phead,
						const char *partname, unsigned long data)
{
	partitionread *ptbl_dz = NULL;
	int ret = 0;

	if (phead == NULL)
	{
		rec_err("phead is NULL.\n");
		return -EINVAL;
	}

	rec_info("-----for %s -----\n", partname == NULL ? "emptycore" : "safeupg");

	if (partname == NULL)
	{
		ptbl_dz = (partitionread *)data;
		if (ptbl_dz == NULL)
		{
			rec_err("ptbl_dz is NULL.\n");
			return -EINVAL;
		}
		ret = nand_raw_partition_read_offset_by_emptycore(phead,
														  ptbl_dz->u8PartitionStartAddr,
														  PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN,
														  sizeof(struct safeupg_partitionhead),
														  ptbl_dz->u8PartitionSize);
	}
	else
	{
		ret = nand_raw_partition_read_offset_by_safeupg(partname, phead,
														0,
														PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN,
														sizeof(struct safeupg_partitionhead),
														DATAZONE_PARTITION_SIZE);
	}

	if (ret != sizeof(struct safeupg_partitionhead))
	{
		rec_err("nand_raw_partition_read_offset_by_emptycore/_safeupg fail --1.\n");
		return -ENANDRD;
	}

	rec_info("---------------end-------------------\n");
	return 0;
}

partitionread *read_partition_info(struct safeupg_partitionhead *phead,
								   const char *partname, unsigned long data)
{
	char *bufpartinfo = NULL;
	partitionread *ptbl_dz = NULL;
	int ret = 0;

	if (phead == NULL)
	{
		rec_err("phead is NULL.\n");
		return NULL;
	}

	rec_info("-----for %s -----\n", partname == NULL ? "emptycore" : "safeupg");

	if (partname == NULL)
	{
		ptbl_dz = (partitionread *)data;
		if (ptbl_dz == NULL)
		{
			rec_err("ptbl_dz is NULL.\n");
			return NULL;
		}
		ret = nand_raw_partition_read_offset_by_emptycore(phead,
														  ptbl_dz->u8PartitionStartAddr,
														  PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN,
														  sizeof(struct safeupg_partitionhead),
														  ptbl_dz->u8PartitionSize);
	}
	else
	{
		ret = nand_raw_partition_read_offset_by_safeupg(partname, phead,
														0,
														PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN,
														sizeof(struct safeupg_partitionhead),
														DATAZONE_PARTITION_SIZE);
	}

	if (ret != sizeof(struct safeupg_partitionhead))
	{
		rec_err("nand_raw_partition_read_offset_by_emptycore/_safeupg fail --1.\n");
		return NULL;
	}
	rec_info("blockcnt=%d\n", phead->blockcnt);
	if (phead->blockcnt == 0)
	{
		rec_err("phead->blockcnt = 0.\n");
		return NULL;
	}

	bufpartinfo = (char *)malloc(phead->blockcnt * PTBL_BLOCK_SIZE);
	if (bufpartinfo == NULL)
	{
		rec_err("malloc failed.\n");
		return NULL;
	}

	if (partname == NULL)
	{
		ret = nand_raw_partition_read_offset_by_emptycore(bufpartinfo,
														  ptbl_dz->u8PartitionStartAddr,
														  PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN + PTBL_BLOCK_SIZE,
														  phead->blockcnt * PTBL_BLOCK_SIZE,
														  ptbl_dz->u8PartitionSize);
	}
	else
	{
		ret = nand_raw_partition_read_offset_by_safeupg(partname, bufpartinfo,
														0,
														PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN + PTBL_BLOCK_SIZE,
														phead->blockcnt * PTBL_BLOCK_SIZE,
														DATAZONE_PARTITION_SIZE);
	}

	if (ret != phead->blockcnt * PTBL_BLOCK_SIZE)
	{
		rec_err("nand_raw_partition_read_offset_by_emptycore/_safeupg fail --2.\n");
		free(bufpartinfo);
		return NULL;
	}

	rec_info("---------------end-------------------\n");
	return (partitionread *)bufpartinfo;
}

int write_partition_info(struct safeupg_partitionhead *phead,
						 partitionread *ptbl,
						 const char *partname,
						 unsigned long data)
{
	char *buf = NULL, *p = NULL;
	int n = 0;
	int ret = 0;
	partitionread *ptmppart = NULL;
	partitionread *ptbl_dz = NULL;
	uint32_t chksum = 0;
	uint32_t malloc_len = 0;

	if (phead == NULL || ptbl == NULL)
	{
		rec_err("No partition table to be written.\n");
		return -EINVAL;
	}

	rec_info("-----write_partition_info for %s -----\n", partname == NULL ? "emptycore" : "safeupg");
	memset(phead, 0, sizeof(struct safeupg_partitionhead));
	ptmppart = ptbl;
	while (ptmppart)
	{
		n++;
		ptmppart = ptmppart->nextpartition;
	}

	phead->blockcnt = (n * sizeof(partitionread) + (PTBL_BLOCK_SIZE - 1)) / PTBL_BLOCK_SIZE;
	phead->u4Signature = ATC_PTBL_SIGN;
	phead->u4Version = ATC_PARTITION_VER;
	phead->nextpartition = NULL;

	malloc_len = 512 + phead->blockcnt * PTBL_BLOCK_SIZE;
	buf = (char *)malloc(malloc_len);
	if (buf == NULL)
	{
		rec_err("malloc fail.\n");
		ret = -ENOMEM;
		goto out;
	}

	memset(buf, 0, malloc_len);
	ptmppart = ptbl;
	p = buf + 512; // skip header 512bytes
	while (ptmppart)
	{
		// printf("<writepartitioninfotoflash> szPartName:%s,nextpartition:0x%X \r\n",pcurpart->szPartName,pcurpart->nextpartition);
		if (ptmppart->nextpartition)
			ptmppart->u4LastPartition = 0;
		else
			ptmppart->u4LastPartition = 1;

		memcpy(p, ptmppart, sizeof(partitionread));
		// TODO, comfirm how to calc partition table checksum.
		chksum += checksum32(0, (char *)ptmppart, sizeof(partitionread));
		p += sizeof(partitionread);
		ptmppart = ptmppart->nextpartition;
	}

	put_checksum32(phead->checksum, chksum);
	memcpy(buf, phead, sizeof(struct safeupg_partitionhead));

	if (partname == NULL)
	{
		ptbl_dz = (partitionread *)data;
		if (ptbl_dz == NULL)
		{
			rec_err("ptbl_dz is NULL.\n");
			ret = -EINVAL;
			goto out_free;
		}
		ret = nand_raw_partition_write_offset_by_emptycore(
			buf,
			ptbl_dz->u8PartitionStartAddr,
			PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN,
			malloc_len,
			ptbl_dz->u8PartitionSize,
			ptbl_dz->u8PartitionSize);
	}
	else
	{
		ret = nand_raw_partition_write_offset_by_safeupg(
			partname,
			buf,
			0,
			PARTITION_INFO_MAIN_OFFSET_FROM_DATAZONE_MAIN,
			malloc_len,
			DATAZONE_PARTITION_SIZE,
			DATAZONE_PARTITION_SIZE);
	}

	if (ret != malloc_len)
	{
		rec_err("nand_raw_partition_write_offset_by_emptycore/_safeupg fail.\n");
		ret = -ENANDRW;
		goto out_free;
	}

	ret = 0;
	rec_info("---------------succeed--------------\n");

out_free:
	free(buf);
out:
	return ret;
}

#endif /* CONFIG_BOOT_MMC */

/*
 * when read partition info from emmc, invoke this function to adjust
 * nextpartition field of partitionread.
 *
 * Note, this function should only be invoked after partion info read from emmc.
 */
void adjust_nextpartition_field(partitionread *ptbl)
{
	partitionread *ppartread, *pprepartition, *pcurpartition;

	if (ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return;
	}

	ppartread = (partitionread *)ptbl;
	pcurpartition = ppartread;
	pprepartition = pcurpartition;

	rec_info("----------begin----------\n");
	while (pcurpartition != NULL)
	{
		if (pcurpartition->u4LastPartition == 1)
		{
			rec_info("this is last partition\r\n");
			pcurpartition->nextpartition = NULL;
			pcurpartition->u4LastPartition == 0;
			break;
		}
		else
		{
// TODO, ImageFileName should NOT be set here. just hack here...
#if 0
			if (strcmp(pcurpartition->szPartName, "preloader") == 0)
				strcpy(pcurpartition->szImageFileName, "83XX_Preloader_realchip_sd.bin");
#endif
			pcurpartition = pcurpartition + 1;
			pprepartition->nextpartition = pcurpartition;
			pprepartition = pcurpartition;
		}
	}
	rec_info("----------end----------\n");
}

uint32_t get_partition_info_checksum(struct safeupg_partitionhead *phead)
{
	return get_checksum32(phead->checksum);
}

uint32_t calc_partition_info_checksum(partitionread *ptbl)
{
	uint32_t chksum = 0;

	if (ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}

	while (ptbl->u4LastPartition == 0)
	{
		// TODO, comfirm how to calc partition table checksum.
		chksum += checksum32(0, (char *)ptbl, sizeof(partitionread));
		ptbl++;
	}

	chksum += checksum32(0, (char *)ptbl, sizeof(partitionread));

	return chksum;
}

/*
 * check partitioninfo crc
 * if tag and crc check pass, return 1, otherwise 0.
 */
int check_partition_info_checksum(struct safeupg_partitionhead *phead, partitionread *ptbl)
{
	uint32_t chksum_calc = 0;
	uint32_t chksum_pi = 0;

	if (phead == NULL || ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}

	chksum_calc = calc_partition_info_checksum(ptbl);
	chksum_pi = get_partition_info_checksum(phead);
	rec_note("chksum_calc=%d,chksum_pi=%d.\n", chksum_calc, chksum_pi);
	if (chksum_calc == 0)
		rec_err("chksum_calc is 0, maybe it's error.\n");
	if (chksum_pi == 0)
		rec_err("chksum_pi is 0, maybe it's error.\n");

	if (chksum_calc != chksum_pi)
	{
		rec_err("partition info checksum check fail.\n");
		return 0;
	}

	rec_info("partition info checksum check pass.\n");
	return 1;
}

int check_partition_head_signature(struct safeupg_partitionhead *phead)
{
	if (phead == NULL)
		return 0;

	if (phead->u4Signature == ATC_PTBL_SIGN)
		return 1;
	else
		return 0;
}

int partition_info_readback_check(const char *devnode,
								  unsigned long offset)
{
	struct safeupg_partitionhead rb_parthead;
	partitionread *rb_ptbl = NULL;

#ifdef CONFIG_BOOT_MMC
	if (devnode == NULL)
	{
		rec_err("devnode is NULL.\n");
		return 0;
	}
#endif

	rb_ptbl = read_partition_info(&rb_parthead, devnode, offset);

	if (rb_ptbl == NULL)
	{
		rec_err("read_partition_info fail.\n");
		return 0;
	}

	if (check_partition_head_signature(&rb_parthead) == 0)
	{
		rec_err("check_partition_head_signature fail.\n");
		free(rb_ptbl);
		return 0;
	}

	if (check_partition_info_checksum(&rb_parthead, rb_ptbl) == 0)
	{
		rec_err("check_partiton_info_checksum fail.\n");
		free(rb_ptbl);
		return 0;
	}

	free(rb_ptbl);
	return 1;
}

#ifdef CONFIG_BOOT_MMC
int read_common_area(const char *devnode, unsigned long offset,
					 void *pbuf, int len)
{
	int fd = 0;
	off_t curpos = 0;
	int rd_size = 0;

	if (devnode == NULL)
	{
		rec_err("devnode is NULL.\n");
		return -EINVAL;
	}

	if (pbuf == NULL)
	{
		rec_err("pbuf is NULL.\n");
		return -EINVAL;
	}

	fd = open(devnode, O_RDWR);
	if (fd < 0)
	{
		rec_err("open fail, %s.\n", strerror(errno));
		return -ESYSCALL;
	}

	curpos = lseek(fd, offset, SEEK_SET);
	if (curpos != offset)
	{
		rec_err("lseek fail, %s.\n", strerror(errno));
		close(fd);
		return -ESYSCALL;
	}

	rd_size = read(fd, pbuf, len);
	if (rd_size != len)
	{
		rec_err("read fail, rd_size(0x%x), len(0x%x), %s.\n", rd_size, len, strerror(errno));
		close(fd);
		return -ESYSCALL;
	}
	close(fd);

	return 0;
}

int write_common_area(const char *devnode, unsigned long offset,
					  void *pbuf, int len)
{
	int fd = 0;
	off_t curpos = 0;
	int wr_size = 0;

	if (devnode == NULL)
	{
		rec_err("devnode is NULL.\n");
		return -EINVAL;
	}

	if (pbuf == NULL)
	{
		rec_err("pbuf is NULL.\n");
		return -EINVAL;
	}

	fd = open(devnode, O_RDWR);
	if (fd < 0)
	{
		rec_err("open fail, %s.\n", strerror(errno));
		return -ESYSCALL;
	}

	curpos = lseek(fd, offset, SEEK_SET);
	if (curpos != offset)
	{
		rec_err("lseek fail, %s.\n", strerror(errno));
		close(fd);
		return -ESYSCALL;
	}
	wr_size = write(fd, pbuf, len);
	if (wr_size != len)
	{
		rec_err("write fail, wr_size(0x%x), len(0x%x), %s.\n", wr_size, len, strerror(errno));
		close(fd);
		return -ESYSCALL;
	}
	close(fd);

	return 0;
}

/*
 * read datazone from emmc
 *
 * return 0 if read success, otherwise -1.
 */
int read_datazone(struct datazone_info *pdz, const char *devnode, unsigned long offset)
{
	return read_common_area(devnode, offset, pdz, sizeof(struct datazone_info));
}

int write_datazone(struct datazone_info *pdz, const char *devnode, unsigned long offset)
{
	return write_common_area(devnode, offset, pdz, sizeof(struct datazone_info));
}

#else /* !CONFIG_BOOT_MMC */

#if 0
int read_common_area(const char *partname, unsigned long offset,
    void *pbuf, int len)
{
	int rd_size = 0;
	int ret = 0;

	if (pbuf == NULL) {
		rec_err("pbuf is NULL.\n");
		return -EINVAL;
	}
	
	rec_info("-----for %s -----\n", partname == NULL ? "emptycore" : "safeupg");
	if (partname == NULL)
		ret = nand_raw_partition_read_offset_by_emptycore(pbuf, offset, len);
	else
		ret = nand_raw_common_read_with_rwctrl_safeupg(partname, pbuf,
			    offset, len);
	if (ret < 0) {
		rec_err("nand_raw_common_read_with_rwctrl/_safeupg fail.\n");
		return -ENANDRD;
	}

	return 0;
}

int write_common_area(const char *partname, unsigned long offset,
	    void *pbuf, int len)
{
	int ret = 0;

	if (pbuf == NULL) {
		rec_err("pbuf is NULL.\n");
		return -EINVAL;
	}

	rec_info("-----for %s -----\n", partname == NULL ? "emptycore" : "safeupg");

	if (partname == NULL)
		ret = nand_raw_common_write_with_rwctrl(pbuf, offset, len);
	else
		ret = nand_raw_common_write_with_rwctrl_safeupg(partname, pbuf,
			    offset, len);
	if (ret < 0) {
		rec_err("nand_raw_common_write_with_rwctrl/_safeupg fail.\n");
		return -ENANDWR;
	}

	return 0;
}
#endif

int read_datazone(struct datazone_info *pdz, const char *partname,
				  unsigned long data)
{
	partitionread *ptbl_dz = NULL;

	if (partname)
	{
		// safeupg
		return nand_raw_partition_read_offset_by_safeupg(partname,
														 pdz, 0, 0, sizeof(struct datazone_info),
														 DATAZONE_PARTITION_SIZE);
	}
	else
	{
		// emptycore
		ptbl_dz = (partitionread *)data;
		if (ptbl_dz == NULL)
		{
			rec_err("ptbl_dz is NULL.\n");
			return -EINVAL;
		}
		return nand_raw_partition_read_offset_by_emptycore(pdz,
														   ptbl_dz->u8PartitionStartAddr, 0,
														   sizeof(struct datazone_info),
														   ptbl_dz->u8PartitionSize);
	}
}

int write_datazone(struct datazone_info *pdz, const char *partname,
				   unsigned long data)
{
	partitionread *ptbl_dz = NULL;

	if (partname)
	{
		// safeupg
		return nand_raw_partition_write_offset_by_safeupg(partname,
														  pdz, 0, 0, sizeof(struct datazone_info),
														  DATAZONE_PARTITION_SIZE,
														  DATAZONE_PARTITION_SIZE);
	}
	else
	{
		// emptycore
		ptbl_dz = (partitionread *)data;
		if (ptbl_dz == NULL)
		{
			rec_err("ptbl_dz is NULL.\n");
			return -EINVAL;
		}
		return nand_raw_partition_write_offset_by_emptycore(pdz,
															ptbl_dz->u8PartitionStartAddr, 0,
															sizeof(struct datazone_info),
															ptbl_dz->u8PartitionSize,
															ptbl_dz->u8PartitionSize);
	}
}

#endif /* CONFIG_BOOT_MMC */

uint32_t get_datazone_checksum(struct datazone_info *pdz)
{
	return get_checksum32(pdz->checksum);
}

uint32_t calc_datazone_checksum(struct datazone_info *pdz)
{
	/*
	 * calc checksum of datazone_info, but NOT include checksum[4].
	 */
	return checksum32(0, (char *)pdz, DATAZONE_INFO_DZ_LEN);
}

void put_datazone_checksum(struct datazone_info *pdz, uint32_t chksum)
{
	put_checksum32(pdz->checksum, chksum);
}

/*
 * check datazone checksum
 * if tag and crc check pass, return 1, otherwise 0.
 */
int check_datazone_checksum(struct datazone_info *pdz)
{
	uint32_t chksum_calc = 0;
	uint32_t chksum_dz = 0;

	if (pdz == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}

	chksum_calc = calc_datazone_checksum(pdz);
	chksum_dz = get_datazone_checksum(pdz);

	if (chksum_calc != chksum_dz)
	{
		rec_err("datazone checksum check fail.\n");
		return 0;
	}

	if (chksum_calc == 0)
	{
		rec_err("chksum_calc = 0\n");
	}

	if (chksum_dz == 0)
	{
		rec_err("chksum_dz = 0\n");
	}

	rec_info("checksum check pass.\n");
	return 1;
}

/*
 * when datazone update done, read bcb back from emmc check
 * to make sure update is correct.
 *
 * return 1 if check pass, otherwise 0.
 */
int datazone_readback_check(const char *devnode, unsigned long offset)
{
	struct datazone_info rb_dz;
	int res = 0;

#ifdef CONFIG_BOOT_MMC
	if (devnode == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}
#endif

	memset(&rb_dz, 0, sizeof(struct datazone_info));
	res = read_datazone(&rb_dz, devnode, offset);
	if (res < 0)
	{
		rec_err("read_datazone fail.\n");
		return 0;
	}

	return check_datazone_checksum(&rb_dz);
}

/**
 * adjust_datazone_img_desc_bk()
 * according to datazone image_desc field and uboot_bk table entry to adjust
 * datazone image_desc_bk field.
 */
void adjust_datazone_img_desc_bk(struct datazone_info *pdz, partitionread *ptbl_ub_bk)
{
	if (pdz == NULL || ptbl_ub_bk == NULL)
	{
		rec_err("parameter is NULL .\n");
		return;
	}

	// TODO, confirm how to adjust img_desc_bk ???
	memcpy(&pdz->img_desc_bk, &pdz->img_desc, sizeof(struct image_desc));
	pdz->img_desc_bk.dwStartAddr = ptbl_ub_bk->u8PartitionStartAddr;
	pdz->img_desc_bk.dwTtlLen = ptbl_ub_bk->u8PartitionSize;
}

#ifdef CONFIG_BOOT_MMC
int read_bcb(struct safeupg_bootloader_message *pbcb, const char *devnode, unsigned long offset)
{
	return read_common_area(devnode, offset, pbcb, sizeof(struct safeupg_bootloader_message));
}

int write_bcb(struct safeupg_bootloader_message *pbcb, const char *devnode, unsigned long offset)
{
	return write_common_area(devnode, offset, pbcb, sizeof(struct safeupg_bootloader_message));
}

#else /* !CONFIG_BOOT_MMC */

int read_bcb(struct safeupg_bootloader_message *pbcb,
			 const char *partname,
			 unsigned long data)
{
	partitionread *ptbl_dz = NULL;

	if (partname)
	{
		// safeupg
		return nand_raw_partition_read_offset_by_safeupg(partname,
														 pbcb, 0,
														 BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN,
														 sizeof(struct safeupg_bootloader_message),
														 DATAZONE_PARTITION_SIZE);
	}
	else
	{
		// emptycore
		ptbl_dz = (partitionread *)data;
		if (ptbl_dz == NULL)
		{
			rec_err("ptbl_dz is NULL.\n");
			return -EINVAL;
		}
		return nand_raw_partition_read_offset_by_emptycore(pbcb,
														   ptbl_dz->u8PartitionStartAddr,
														   BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN,
														   sizeof(struct safeupg_bootloader_message),
														   ptbl_dz->u8PartitionSize);
	}
}

int write_bcb(struct safeupg_bootloader_message *pbcb, const char *partname,
			  unsigned long data)
{
	partitionread *ptbl_dz = NULL;

	if (partname)
	{
		rec_info("write_bcb.\n");
		// safeupg
		return nand_raw_partition_write_offset_by_safeupg(partname,
														  pbcb, 0,
														  BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN,
														  sizeof(struct safeupg_bootloader_message),
														  DATAZONE_PARTITION_SIZE,
														  DATAZONE_PARTITION_SIZE);
	}
	else
	{
		rec_info("write_bcb2222.\n");
		// emptycore
		ptbl_dz = (partitionread *)data;
		if (ptbl_dz == NULL)
		{
			rec_err("ptbl_dz is NULL.\n");
			return -EINVAL;
		}
		return nand_raw_partition_write_offset_by_emptycore(pbcb,
															ptbl_dz->u8PartitionStartAddr,
															BCB_MAIN_OFFSET_FROM_DATAZONE_MAIN,
															sizeof(struct safeupg_bootloader_message),
															ptbl_dz->u8PartitionSize,
															ptbl_dz->u8PartitionSize);
	}
}
#endif

uint32_t get_bcb_checksum(struct safeupg_bootloader_message *pbcb)
{
	return get_checksum32(pbcb->checksum);
}

uint32_t calc_bcb_checksum(struct safeupg_bootloader_message *pbcb)
{
	/*
	 * calc checksum of bcb, but NOT include tages[16] and checksum[4].
	 */
	return checksum32(0, (char *)pbcb + 16 + 4, sizeof(struct safeupg_bootloader_message) - (16 + 4));
}

void put_bcb_checksum(struct safeupg_bootloader_message *pbcb, uint32_t chksum)
{
	put_checksum32(pbcb->checksum, chksum);
}

/*
 * check bcb checksum
 * if tag and checksum check pass, return 1, otherwise 0.
 */
int check_bcb_tag_checksum(struct safeupg_bootloader_message *pbcb)
{
	uint32_t chksum_calc = 0;
	uint32_t chksum_bcb = 0;

	if (pbcb == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}

	if (strncmp(pbcb->tags, BCB_TAG, strlen(BCB_TAG)))
	{
		rec_err("bcb.tags:%s\n", pbcb->tags);
		rec_err("bcb tag check fail.\n");
		return 0;
	}

	chksum_calc = calc_bcb_checksum(pbcb);
	chksum_bcb = get_bcb_checksum(pbcb);
	rec_info("chksum_calc=%d, chksum_bcb=%d\n", chksum_calc, chksum_bcb);

	if (chksum_calc == 0)
	{
		rec_warn("chksum_calc = 0.\n");
	}

	if (chksum_bcb == 0)
	{
		rec_warn("chksum_bcb = 0.\n");
	}

	if (chksum_calc != chksum_bcb)
	{
		rec_err("bcb checksum check fail. chksum_calc(0x%x), chksum_bcb(0x%x)\n", chksum_calc, chksum_bcb);
		return 0;
	}
	rec_info("bcb tag and checksum check pass.\n");
	return 1;
}

/*
 * when bcb update done, read bcb back from emmc check
 * to make sure update is correct.
 *
 */
int bcb_readback_check(const char *devnode, unsigned long offset)
{
	struct safeupg_bootloader_message rb_bcb;
	int ret = 0;

	if (devnode == NULL)
	{
		rec_err("devnode is NULL.\n");
		return 0;
	}

	memset(&rb_bcb, 0, sizeof(struct safeupg_bootloader_message));
	ret = read_bcb(&rb_bcb, devnode, offset);
	if (ret < 0)
	{
		rec_err("read_bcb fail.\n");
		return 0;
	}

	return check_bcb_tag_checksum(&rb_bcb);
}

uint32_t get_bcb_bootflag(struct safeupg_bootloader_message *pbcb)
{
	return pbcb->bootflag;
}

void put_bcb_bootflag(struct safeupg_bootloader_message *pbcb, uint32_t bootflag)
{
	pbcb->bootflag = bootflag;
}

void set_bcb_tags(struct safeupg_bootloader_message *pbcb)
{
	strncpy(pbcb->tags, BCB_TAG, 16);
}

void print_bcb_tags(struct safeupg_bootloader_message *pbcb)
{
	int i;
	if (pbcb == NULL)
	{
		rec_err("parameter is NULL .\n");
		return;
	}

	rec_info("bcb tags:");
	for (i = 0; i < 16; i++)
	{
		rec_info("%d", pbcb->tags[i]);
	}
	rec_info("\n");
}

#ifdef CONFIG_BOOT_MMC
void dump_writeprotect_region(int fdwp)
{
	struct wp_cmd_arg argdump = {0};
	int ret;

	argdump.wp_action = WP_REGIONINFO_GET;
	argdump.wp_dump_info = (char *)malloc(MAX_DUMP_BUFF_SIZE);
	ret = ioctl(fdwp, MSDC_EMMC_WRITE_PROTECT, &argdump);
	if (ret)
	{
		rec_err("dump wp fail, ret:%d\n", ret);
	}
	else
	{
		rec_info("dump success, result:%s\n", argdump.wp_dump_info);
	}

	free(argdump.wp_dump_info);
}

#ifdef NEW_PARTITION_DESIGN
int update_writeprotect_region(int fdwp, partitionread *ptbl)
{
	struct wp_cmd_arg arg = {0};
	unsigned long long wp_start_sect = 0;
	unsigned long long wp_end_sect = 0;
	int find_wp_part = 0;
	int ret;
	partitionread *ptmp = ptbl;

	if (ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -1;
	}

	rec_info("enter ...\n");
	while (ptmp)
	{
		if (ENABLE == isEnable(WRITE_PROTECT_ENABLE, ptmp->u4Flag))
		{
			if (find_wp_part == 0)
			{
				wp_start_sect = ptmp->u8PartitionStartAddr / PTBL_BLOCK_SIZE;
				wp_end_sect = (wp_start_sect * PTBL_BLOCK_SIZE + ptmp->u8PartitionSize) / PTBL_BLOCK_SIZE;
				find_wp_part = 1;
			}
			else
			{
				rec_info("continuous region has been wirte protect\r\n");
			}
		}
		else
		{
			if (find_wp_part)
			{
				arg.wp_action = WP_REGION_ENABLE;
				arg.wpg_size_of_xml = 16; // MB
				arg.sect_start = wp_start_sect;
				/*because of alligment ,the last partion of continuous region is not continuous address */
				arg.sect_end = ptmp->u8PartitionStartAddr / PTBL_BLOCK_SIZE;
				rec_info("<1>enable write protect on sect 0x%x~0x%x\n", arg.sect_start * PTBL_BLOCK_SIZE, arg.sect_end * PTBL_BLOCK_SIZE);
				ret = ioctl(fdwp, MSDC_EMMC_WRITE_PROTECT, &arg);
				if (ret)
				{
					rec_err("set wp fail\n");
					return -1;
				}
				else
				{
					rec_info("set wp success\n");
				}

				wp_start_sect = 0;
				wp_end_sect = 0;
				find_wp_part = 0;
			}
		}
		ptmp = ptmp->nextpartition;
	}

	if (find_wp_part)
	{
		// all parts are protect.
		arg.wp_action = WP_REGION_ENABLE;
		arg.wpg_size_of_xml = 16; // MB
		arg.sect_start = wp_start_sect;
		/*
		 * end sect aligns to 16M/512=0x8000
		 */
		arg.sect_end = ((wp_end_sect + 0x8000) & (~0x7FFFU));

		rec_info("<2>enable write protect on sect 0x%x~0x%x\n", arg.sect_start * PTBL_BLOCK_SIZE, arg.sect_end * PTBL_BLOCK_SIZE);
		ret = ioctl(fdwp, MSDC_EMMC_WRITE_PROTECT, &arg);
		if (ret)
		{
			rec_err("set wp fail-2\n");
			return -1;
		}
		else
		{
			rec_info("set wp success-2\n");
		}
	}

	return 0;
}
#endif /* NEW_PARTITION_DESIGN */

int open_for_writeprotect(void)
{
	int fdwp;

	fdwp = open("/dev/misc-sd", O_RDONLY);
	if (fdwp < 0)
	{
		rec_err("open /dev/misc-sd fail\n");
		return -1;
	}

	return fdwp;
}

void close_for_writeprotect(int fdwp)
{
	close(fdwp);
}

int clear_writeprotect(int fdwp)
{
	struct wp_cmd_arg arg = {0};
	int ret;

	/*clear all write protect*/
	arg.wp_action = WP_ALL_DISABLE;
	arg.partition_name = NULL;
	ret = ioctl(fdwp, MSDC_EMMC_WRITE_PROTECT, &arg);
	if (ret)
	{
		rec_err("clear all wp fail, ret:%d\n", ret);
		return -1;
	}
	else
	{
		rec_info("clear all wp success\n");
		return 0;
	}
}

#else /* CONFIG_BOOT_MMC */

void dump_writeprotect_region(int fdwp)
{
	// TODO, if nand has write-protect, need to implment
	rec_warn("writeprotect NOT support yet!\n");
}

int update_writeprotect_region(int fdwp, partitionread *ptbl)
{
	// TODO, if nand has write-protect, need to implment
	rec_warn("writeprotect NOT support yet!\n");
	return 0;
}

int open_for_writeprotect(void)
{
	// TODO, if nand has write-protect, need to implment
	rec_warn("writeprotect NOT support yet!\n");
	return 0;
}

void close_for_writeprotect(int fdwp)
{
	// TODO, if nand has write-protect, need to implment
	rec_warn("writeprotect NOT support yet!\n");
}

int clear_writeprotect(int fdwp)
{
	// TODO, if nand has write-protect, need to implment
	rec_warn("writeprotect NOT support yet!\n");
	return 0;
}

#endif /* CONFIG_BOOT_MMC */

partitionread *read_partition_info_from_xml_file(char *xml_file)
{
	if (xml_file == NULL)
	{
		rec_err("parameter is NULL .\n");
		return NULL;
	}

	rec_info("xml file name is %s\n", xml_file);
	read_partition_table_from_file(xml_file);
	// xmlptbl = newtblhead;
	return newtblhead;
}

/*
 * adjust_xml_partition_info_userdata_size() - adjust xml patition info userdata size according to emmc total size.
 * normally, the last partition is userdata partition with ext4 type.
 */
int adjust_xml_partition_info_size(partitionread *ptbl, int board_type)
{
	int ret = 0;
	unsigned long long emmc_size = 0;
	char ptname[16] = {0};

	if (ptbl == NULL)
	{
		rec_err("ptbl is NULL!");
		return -EINVAL;
	}
	strcpy(ptname, "usrdata");

	while (ptbl->nextpartition)
	{
		/* look for the last partition **/
		ptbl = ptbl->nextpartition;
	}

	if ((strcmp(ptbl->szPartName, ptname) == 0) && (strcmp(ptbl->szType, "ext4") == 0))
	{
		ret = get_emmc_total_size(&emmc_size);
		if (ret < 0)
		{
			rec_err("get emmc total size failed.\n");
			return -EEMMCSIZE; // emmc size error
		}
		if (emmc_size < ptbl->u8PartitionStartAddr)
		{
			rec_err("emmc_size less than map.startaddress\n");
			return -EPARTTBL;
		}
		else if (emmc_size < ptbl->u8PartitionStartAddr + ptbl->u8PartitionSize)
		{
			rec_err("emmc_size less than map.startaddress+usrdata.size\n");
			return -EPARTTBL;
		}
		else if (emmc_size < ptbl->u8PartitionStartAddr + (50 + 2) * 512)
		{
			// TODO, double confirm (50+2) is the same as before?????
			rec_err("emmc_size less than userdate.startaddress+(50+2)*512)\n");
			return -EPARTTBL;
		}
		else
		{
			ptbl->u8PartitionSize = emmc_size - ptbl->u8PartitionStartAddr - (50 + 2) * 512;
		}
	}
	return 0;
}

/*
 * check_partition_overlap() - check the partitions area overlapping.
 * return 1 if overlapping, otherwise 0.
 */
int check_partition_overlap(partitionread *ptbl)
{
	int ret = 0;
	unsigned long long addr = 0;
	partitionread *ptmp = ptbl;

	while (ptbl)
	{
		if (ptbl->u8PartitionStartAddr < addr)
			return 1; // overlap
		addr += ptbl->u8PartitionSize;
		ptbl = ptbl->nextpartition;
	}
	return 0;
}



bool check_is_file_exist(const char *file)
{
		int fd;
		fd = open(file, O_RDONLY | O_LARGEFILE, 0);
		if (fd < 0)
		{
			rec_err("open file fail.\n");
			return 0;
		}
		close(fd);
	return 1;
}

/*
 * check_files_exist_for_update() - check image files exist for upgrade.
 * return 1 if all check files exist, otherwise return 0.
 */
int check_files_exist_for_upgrade(partitionread *ptbl, const char *dir_path)
{
	int ret = 1;
	if (ptbl == NULL || dir_path == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}

	while (ptbl)
	{
		if (strlen(ptbl->szImageFileName))
		{
			char file[IMG_FULL_NAME_MAX];
			strcpy(file, ptbl->szImageFileName);
			if (!check_is_file_exist(file))
			{
				rec_warn("Required file %s \r\n", file);
				dumppartitioninfo(ptbl);
				ret = 0;
			}
		}
		ptbl = ptbl->nextpartition;
	}
	return ret;
}

int check_file_exist_for_one_table(partitionread *ptbl, const char *dir_path)
{
	int ret = 1;
	if (ptbl == NULL || dir_path == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}

	if (strlen(ptbl->szImageFileName))
	{
		char file[IMG_FULL_NAME_MAX];
		strcpy(file, ptbl->szImageFileName);
		if (!check_is_file_exist(file))
		{
			rec_err("Required file %s NOT exist\r\n", file);
			dumppartitioninfo(ptbl);
			ret = 0;
		}
	}

	return ret;
}

/*
 * partition_need_imagefile()
 * like cache and expdb do NOT need image file.
 *
 * return true if need image file, otherwise false.
 */
int partition_need_imagefile(partitionread *ptbl)
{
	if (ptbl == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}

	// rec_info("partition_need_imagefile ...\r\n");
	return !!strlen(ptbl->szImageFileName);
}

static uint32_t log2(uint32_t value)
{
	uint32_t rc = 0;

	while (0 != value)
	{
		value >>= 1;
		rc++;
	}
	rc--;

	return rc;
}

static char BLID1[12] = "BOOTLOADER!";
static char BLNFIID2[8] = "NFIINFO";
static char BLMSDCID2[8] = "MT3360A"; // don't change it, romcode uses this value to verify.

int create_bootloader_header(char *pbl_header, char *blbuf,
							 uint32_t image_size, int msdc_boot)
{
	BOOTL_HEADER bl_header;
	uint32_t chksum, temp32;
	unsigned char *pbuf = (unsigned char *)blbuf;
	uint32_t i;
	uint32_t oob_size = 0;
	uint32_t page_size = 0;
	uint32_t block_size = 0;
#ifndef CONFIG_BOOT_MMC
	int ret;
#endif
	if (pbl_header == NULL || blbuf == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	memset(&bl_header, 0x0, sizeof(BOOTL_HEADER));
	memcpy(bl_header.ID1, BLID1, 12);
	if (msdc_boot)
		memcpy(bl_header.ID2, BLMSDCID2, 8);
	else
		memcpy(bl_header.ID2, BLNFIID2, 8);

	bl_header.startAddr = 0x40000000;
	bl_header.length = image_size;
#ifndef CONFIG_BOOT_MMC
	ret = get_nand_oob_size(&oob_size);
	if (ret < 0)
	{
		rec_err("get_nand_oob_size fail.\n");
		return -ENANDINFO;
	}

	ret = get_nand_page_size(&page_size);
	if (ret < 0)
	{
		rec_err("get_nand_page_size fail.\n");
		return -ENANDINFO;
	}

	ret = get_nand_block_size(&block_size);
	if (ret < 0)
	{
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}
#endif

	if (!msdc_boot)
	{
		bl_header.NFIinfo.pageSize = (uint16_t)page_size; // TODO, double confirm???
		bl_header.pagesPerBlock = (uint16_t)256;
		bl_header.totalBlocks = (uint16_t)0x800;
		bl_header.NFIinfo.spareSize = (uint16_t)oob_size; // TODO, double confirm???

		rec_info("oob size[%d]", bl_header.NFIinfo.spareSize);
		bl_header.NFIinfo.addressCycle = (uint16_t)0x5;

		if (bl_header.NFIinfo.pageSize > 512)
			bl_header.NFIinfo.pageShift = (uint16_t)0x10;
		else
			bl_header.NFIinfo.pageShift = (uint16_t)0x8;

		bl_header.blockShift = (uint16_t)(log2(bl_header.pagesPerBlock) + bl_header.NFIinfo.pageShift);
	}

	chksum = 0;
	for (i = 0; i < image_size; i += 4)
	{
		memcpy(&temp32, ((unsigned char *)pbuf + i), 4);
		chksum ^= temp32;
	}

	bl_header.checksum = chksum;
	for (i = 0; i < REPLICATION_NUMBER; i++)
	{
		memcpy(pbl_header, &bl_header, sizeof(BOOTL_HEADER));
		pbl_header = (char *)pbl_header + sizeof(BOOTL_HEADER);
	}

	rec_info("done!\n");
	return 0;
}

int verify_bootloader_header(BOOTL_HEADER *pbl_header, int msdc_boot)
{
	int i = 0;
	int n = 0;

	if (pbl_header == NULL)
	{
		rec_err("parameter is NULL .\n");
		return 0;
	}

	for (i = 0; i < REPLICATION_NUMBER; i++)
	{
		for (n = 0; n < 12; n++)
		{
			if (pbl_header->ID1[n] != BLID1[n])
			{
				rec_err("ID1 verify fail.\n");
				return 0;
			}
		}

		for (n = 0; n < 8; n++)
		{
			if (msdc_boot)
			{
				if (pbl_header->ID2[n] != BLMSDCID2[n])
				{
					rec_err("ID2 verify fail.\n");
					return 0;
				}
			}
			else
			{
				if (pbl_header->ID2[n] != BLNFIID2[n])
				{
					rec_err("ID2 verify fail.\n");
					return 0;
				}
			}
		}
		pbl_header++;
	}

	return 1;
}

#ifdef CONFIG_BOOT_MMC
int upg_raw_partition_from_file(const char *devname,
								const char *file, long long offset, long long size, long long *psize_total)
{
	int ret;
	int fdev, fd;
	long long sizer, sizew, sizetotal;
	char *buffer = NULL;
	off64_t cur_offset;
	unsigned long long file_len = 0;
	if (devname == NULL || file == NULL || psize_total == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	ret = get_file_len(file, &file_len);
	if (ret < 0)
	{
		rec_err("file_len get fail.\n");
		return -EFILELEN;
	}

	rec_info("dev(%s) offset(0x%llx) file(%s)\n", devname, offset, file);
	fd = open(file, O_RDONLY | O_LARGEFILE, 0);
	if (fd < 0)
	{
		return -ESYSCALL;
	}

	fdev = open(devname, O_RDWR | O_LARGEFILE);
	if (fdev < 0)
	{
		close(fd);
		return -ESYSCALL;
	}
	buffer = (char *)malloc(FILE_RW_SIZE);
	if (!buffer)
	{
		close(fd);
		close(fdev);
		return -ENOMEM;
	}
	cur_offset = lseek64(fdev, offset, SEEK_SET);
	if (cur_offset != offset)
	{
		rec_err("lseek64 failed!\r\n");
		close(fd);
		close(fdev);
		return -ESYSCALL;
	}

	sizetotal = 0;
	while (sizer = read(fd, buffer, FILE_RW_SIZE))
	{
		sizew = write(fdev, buffer, sizer);
		sizetotal += sizew;
		if (sizew < sizer)
			break;
	}

	close(fd);
	close(fdev);
	free(buffer);

	if (sizetotal < file_len)
	{
		rec_err(" faild! the size of file(%s) is %d bytes, only %d bytes are write into emmc.\n",
				file, file_len, sizetotal);
		return -ESYSCALL;
	}

	*psize_total = sizetotal;

	rec_info("succeeded!\n");
	return 0;
}

int upg_ext4_partition_from_file(const char *devname,
								 const char *file, long long offset, long long size)
{
	int fdev = 0, fd = 0;
	long long sizer = 0, sizew = 0, filelen = 0;
	uchar *buffer = NULL;
	uint32_t chunk_cnt = 0;
	uint32_t block_size = 0;
	uchar *pfileBuffer = NULL;
	long long memlen = DEF_CHUNK_SIZE;
	long long datalen = 0;
	off64_t cur_offset;
	int ret = -1;

	sparse_header_t tSparseHeader;
	chunk_header_t tChunkHeader;
	if (devname == NULL || file == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	rec_info("dev(%s) offset(0x%llx) file(%s)\n", devname, offset, file);
	fd = open(file, O_RDONLY | O_LARGEFILE, 0);
	if (fd < 0)
	{
		rec_err("open %s fail.\n", file);
		return -ESYSCALL;
	}

	fdev = open(devname, O_RDWR | O_LARGEFILE);
	if (fdev < 0)
	{
		rec_err("open %s fail.\n", devname);
		close(fd);
		return -ESYSCALL;
	}

	filelen = lseek64(fd, 0, SEEK_END);

	buffer = (uchar *)malloc(memlen);
	if (!buffer)
	{
		close(fd);
		close(fdev);
		return -ENOMEM;
	}

	// Read file to buffer
	lseek64(fd, 0, SEEK_SET);
	sizer = read(fd, &tSparseHeader, sizeof(sparse_header_t));

	block_size = tSparseHeader.blk_sz;
	chunk_cnt = tSparseHeader.total_chunks;

	filelen -= sizeof(sparse_header_t);
	while ((chunk_cnt > 0) && (filelen > 0))
	{
		sizer = read(fd, &tChunkHeader, CHUNK_HEADER_LEN);
		datalen = tChunkHeader.chunk_sz * block_size;

		if (CHUNK_TYPE_RAW == tChunkHeader.chunk_type)
		{
			if (datalen > memlen)
			{
				buffer = (uchar *)realloc((void *)buffer, datalen);
				if (!buffer)
				{
					close(fd);
					close(fdev);
					return -ENOMEM;
				}
				memlen = datalen;
			}
			pfileBuffer = buffer;

			sizer = read(fd, pfileBuffer, datalen);

			cur_offset = lseek64(fdev, offset, SEEK_SET);
			if (cur_offset != offset)
			{
				rec_err("lseek64 failed!\r\n");
				close(fd);
				close(fdev);
				free(buffer);
				return -ESYSCALL;
			}
			sizew = write(fdev, pfileBuffer, datalen);

			filelen -= tChunkHeader.total_sz;
			chunk_cnt--;
			pfileBuffer += datalen;
			offset += datalen;
		}
		else if (CHUNK_TYPE_DONT_CARE == tChunkHeader.chunk_type)
		{
			lseek64(fd, tChunkHeader.total_sz - CHUNK_HEADER_LEN, SEEK_CUR);
			filelen -= tChunkHeader.total_sz;
			chunk_cnt--;
			offset += datalen;
		}
		else
		{
			rec_err("tChunkHeader.chunk_type error\r\n");
			close(fd);
			close(fdev);
			free(buffer);
			return -ESPARSEFILE;
		}
	}

	close(fd);
	close(fdev);
	free(buffer);

	if (0 != chunk_cnt)
	{
		rec_err("chunk_cnt error\r\n");
		return -ESPARSEFILE;
	}

	rec_info("succeeded! memsize(0x%x)\n", memlen);
	return 0;
}

#else /* !CONFIG_BOOT_MMC */

int nand_preloader_readback_check(partitionread *ptbl, int mode)
{
	BOOTL_HEADER *pbl_header_nand = NULL;
	BOOTL_HEADER *pbl_header = NULL;
	char *buf = NULL;
	int fd = 0;
	int ret = 0;
	int n = 0;

	if (ptbl == NULL)
		return -EINVAL;

	pbl_header_nand = (BOOTL_HEADER *)malloc(sizeof(BOOTL_HEADER) * REPLICATION_NUMBER);
	if (pbl_header_nand == NULL)
	{
		rec_err("malloc for pbl_header_nand fail\n");
		ret = -ENOMEM;
		goto out;
	}
	memset(pbl_header_nand, 0, sizeof(BOOTL_HEADER) * REPLICATION_NUMBER);

	buf = (char *)malloc(PRELOADER_SIZE + 512);
	if (!buf)
	{
		rec_err("malloc fail.\n");
		ret = -ENOMEM;
		goto out_free_blh;
	}

	if (mode == 0)
		n = nand_raw_partition_read_by_emptycore(pbl_header_nand,
												 ptbl->u8PartitionStartAddr,
												 sizeof(BOOTL_HEADER) * REPLICATION_NUMBER,
												 ptbl->u8PartitionSize);
	else
		n = nand_raw_partition_read_by_safeupg(ptbl->szPartName,
											   pbl_header_nand,
											   0,
											   sizeof(BOOTL_HEADER) * REPLICATION_NUMBER,
											   ptbl->u8PartitionSize);

	if (n != sizeof(BOOTL_HEADER) * REPLICATION_NUMBER)
	{
		rec_err("read fail, read(%d), expect(%d)\n",
				n, sizeof(BOOTL_HEADER) * REPLICATION_NUMBER);
		ret = -ENANDRD;
		goto out_free_buf;
	}

	rec_dbg("read preloader header done.\n");
	if (mode == 0)
		n = nand_raw_partition_read_offset_by_emptycore((buf + 512),
														ptbl->u8PartitionStartAddr, 512,
														PRELOADER_SIZE,
														ptbl->u8PartitionSize);
	else
		n = nand_raw_partition_read_offset_by_safeupg(ptbl->szPartName, (buf + 512),
													  0, 512,
													  PRELOADER_SIZE,
													  ptbl->u8PartitionSize);

	if (n != PRELOADER_SIZE)
	{
		rec_err("read fail, read(%d), expect(%d)\n", n, PRELOADER_SIZE);
		ret = -ENANDRD;
		goto out_free_buf;
	}

	rec_dbg("read preloader body.\n");
	ret = verify_bootloader_header(pbl_header_nand, 0);
	if (ret == 0)
	{
		rec_err("verify_bootloader_header fail.\n");
		ret = -ERDBACKCHK;
		goto out_free_buf;
	}
	rec_info("verify_bootloader_header sucess.\n");

	ret = create_bootloader_header(buf, (buf + 512), PRELOADER_SIZE, 0);
	if (ret < 0)
	{
		rec_err("create_bootloader_header fail.\n");
		ret = -ERDBACKCHK;
		goto out_free_buf;
	}
	pbl_header = (BOOTL_HEADER *)buf;

	if (pbl_header_nand->checksum != pbl_header->checksum)
	{
		rec_err("bl_header checksum fail, nand checksum(0x%x), calc checksum(0x%x)\n",
				pbl_header_nand->checksum, pbl_header->checksum);
		ret = -ERDBACKCHK;
		goto out_free_buf;
	}

	ret = 0;

out_free_buf:
	free(buf);
out_free_blh:
	free(pbl_header_nand);
out:
	return ret;
}

static struct
{
	const char *name;
	int idx;
} nand_pt_idx_map[] = {
	{"preloader", PRELOADER},
	{"preloader_bk", PRELOADER_BK},
	{"datazone", DATAZONE},
	{"datazone_bk", DATAZONE_BK},
	{"uboot", UBOOT},
	{"trustzone", TRUSTZONE},
	{"arm2", ARM2},
	{"dtb", DTB},
	{"logo", LOGO},
	{"boot_misc", BOOT_MISC},
	{"vba", VBA},
	{"metazone", METAZONE},
	{"kernel", KERNEL},
	{"system", SYSTEM},
	{"usrdata", USRDATA},
	{"recovery", RECOVERY},
	{"allnand", ALLNAND},

};

int lookup_idx_by_partname(const char *partname)
{
	int i;

	if (partname == NULL)
	{
		rec_err("partname is NULL.\n");
		return -1;
	}
	for (i = 0; i < ARRAY_SIZE(nand_pt_idx_map); i++)
	{
		if (strcmp(nand_pt_idx_map[i].name, partname) == 0)
		{
			rec_info("find part(%s) idx(%d)\n",
					 partname, nand_pt_idx_map[i].idx);
			return nand_pt_idx_map[i].idx;
		}
	}
	rec_err("can NOT lookup idx for part(%s)\n", partname);
	return -1;
}

static uint32_t nand_pagesize = 0;

int get_nand_page_size(uint32_t *psize)
{
	int ret = -1;
	struct nand_dev_info nand_info;

	if (psize == NULL)
		return -EINVAL;

	if (nand_pagesize)
	{
		*psize = nand_pagesize;
		return 0;
	}

	memset(&nand_info, 0, sizeof(struct nand_dev_info));
	ret = get_nand_info(&nand_info);
	if (ret < 0)
	{
		rec_err("get_nand_info fail.\n");
		return -ENANDINFO;
	}

	*psize = nand_info.page_size;
	rec_info("nand_page_size is %d\n", *psize);
	if (*psize == 0)
	{
		return -ENANDINFO;
	}
	else
	{
		nand_pagesize = *psize;
		return 0;
	}
}

static uint32_t nand_blockcnt = 0;

int get_nand_block_cnt(uint32_t *pcnt)
{
	int ret = -1;
	struct nand_dev_info nand_info;

	if (pcnt == NULL)
		return -EINVAL;

	if (nand_blockcnt)
	{
		*pcnt = nand_blockcnt;
		return 0;
	}

	memset(&nand_info, 0, sizeof(struct nand_dev_info));
	ret = get_nand_info(&nand_info);
	if (ret < 0)
	{
		rec_err("get_nand_info fail.\n");
		return -ENANDINFO;
	}

	*pcnt = nand_info.block_cnt;
	rec_info("nand_block_cnt is 0x%x\n", *pcnt);
	if (*pcnt == 0)
	{
		return -ENANDINFO;
	}

	nand_blockcnt = *pcnt;
	return 0;
}

static uint32_t nand_oobsize = 0;

int get_nand_oob_size(uint32_t *poob_size)
{
	int ret = -1;
	struct nand_dev_info nand_info;

	if (poob_size == NULL)
		return -EINVAL;

	if (nand_oobsize)
	{
		*poob_size = nand_oobsize;
		return 0;
	}

	memset(&nand_info, 0, sizeof(struct nand_dev_info));
	ret = get_nand_info(&nand_info);

	if (ret < 0)
	{
		rec_err("get_nand_info fail.\n");
		return -ENANDINFO;
	}

	*poob_size = nand_info.oob_size;
	if (*poob_size == 0)
	{
		return -ENANDINFO;
	}
	nand_oobsize = *poob_size;
	return 0;
}

static uint32_t nand_blocksize = 0;
int get_nand_block_size(uint32_t *pblk_size)
{
	int ret = -1;
	struct nand_dev_info nand_info;

	if (pblk_size == NULL)
		return -EINVAL;

	if (nand_blocksize)
	{
		*pblk_size = nand_blocksize;
		return 0;
	}

	memset(&nand_info, 0, sizeof(struct nand_dev_info));
	ret = get_nand_info(&nand_info);
	if (ret < 0)
	{
		rec_err("get_nand_info fail.\n");
		return -ENANDINFO;
	}

	*pblk_size = nand_info.block_size;
	rec_info("nand_block_cnt is 0x%x\n", *pblk_size);
	if (*pblk_size == 0)
	{
		return -ENANDINFO;
	}
	nand_blocksize = *pblk_size;
	return 0;
}

int get_nand_size(uint64_t *pnand_size)
{
	int ret = -1;
	uint32_t blk_cnt;
	uint32_t blk_size;

	if (pnand_size == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	ret = get_nand_block_cnt(&blk_cnt);
	if (ret < 0)
	{
		rec_err("get_nand_block_cnt fail.\n");
		return -ENANDINFO;
	}

	ret = get_nand_block_size(&blk_size);
	if (ret < 0)
	{
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}

	*pnand_size = (uint64_t)blk_size * (uint64_t)blk_cnt;
	rec_info("nand total size is 0x%llx.\n", *pnand_size);
	return 0;
}

/*
 * this function must be invoked at the position between nad_rw_start and nand_rw_end
 */
int nand_partition_reserve_blk_check(int64_t pt_start, int64_t pt_size)
{
	uint32_t blk_size;
	int64_t pt_blk_num;
	int64_t real_rsv_blk_num;
	int64_t rsv_blk_num;
	/* size includes write size and bad blks size */
	int64_t nand_endoffset;
	int ret;

	rec_dbg("pt_start=0x%llx, pt_size=0x%llx\n", pt_start, pt_size);

	ret = get_nand_block_size(&blk_size);
	if (ret < 0)
	{
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}
	rec_dbg("blk_size=0x%x\n", blk_size);

	/*
	 *
	 * nand_ext4_write_endoffset is from nand driver.
	 *
	 *	|-----------------------------------------------------|
	 *	|x1*blk_size bytes	|x2 bad blk	|x3 bytes	|				|
	 *	|					|			|			|				|
	 *	0											|				pt_size
	 *												nand_endoffset
	 */
	nand_endoffset = nand_ext4_write_endoffset();
	rec_info("nand_endoffset=0x%llx\n", nand_endoffset);
	nand_endoffset = ALIGN(nand_endoffset, blk_size);

	if ((nand_endoffset - pt_start) >= pt_size)
	{
		rec_err("out partition range.\n");
		return -ENANDRSVBLK;
	}

	rsv_blk_num = GET_RESERVED_BLOCK_NUM((pt_size / blk_size));
	rec_dbg("rsv_blk_num=0x%llx\n", rsv_blk_num);

	pt_blk_num = pt_size / blk_size;
	rec_dbg("pt_blk_num=0x%llx\n", pt_blk_num);

	if (pt_blk_num > (int64_t)MIN_PARTITION_BLOCK_NUM)
	{
		real_rsv_blk_num = (pt_size - (nand_endoffset - pt_start)) / blk_size;
		rec_info("real_rsv_blk_num(0x%llx), rsv_blk_num(0x%llx),pt_size=(0x%llx).\n",
				 real_rsv_blk_num, rsv_blk_num, pt_size);
		if (real_rsv_blk_num < rsv_blk_num)
		{
			rec_err("reserved blk num fail, real_rsv_blk_num(0x%llx), rsv_blk_num(0x%llx).\n",
					real_rsv_blk_num, rsv_blk_num);
			return -ENANDRSVBLK;
		}
	}

	return 0;
}

static int64_t __nand_write_image_align(void *buf, uint64_t offset_align,
										uint64_t size, int mode)
{
	int ret;
	int64_t result;

	if (buf == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	rec_dbg("offset_align=0x%llx, size=0x%llx, mode=%d\n", offset_align, size, mode);
#if 0
	ret = nand_rw_start(size);
	if (ret < 0) {
		rec_err("nand_rw_start fail.\n");
		return -ENANDRW;
	}
#endif
	if (mode == 0)
		result = nand_write_raw_image(buf, offset_align, size);
	else
		result = nand_write_ext4_image(buf, offset_align, size);

	if (result != size)
	{
		rec_err("nand_write_%s_image fail, result=%lld.\n",
				mode ? "ext4" : "raw", result);
		result = -ENANDWR;
	}

#if 0
	ret = nand_rw_end();
	if (ret < 0) {
		rec_err("nand_rw_end fail.\n");
		return -ENANDRW;
	}
#endif
	return result;
}

static int64_t nand_write_raw_image_align(void *buf, uint64_t offset_align,
										  uint64_t size)
{
	return __nand_write_image_align(buf, offset_align, size, 0);
}

static int64_t nand_write_ext4_image_align(void *buf, uint64_t offset_align,
										   uint64_t size)
{
	return __nand_write_image_align(buf, offset_align, size, 1);
}

static int64_t __nand_read_image_align(void *buf, uint64_t offset_align,
									   uint64_t size, int mode)
{
	int ret;
	int64_t result;

	if (buf == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	rec_dbg("offset_align=0x%llx, size=0x%llx, mode=%d\n", offset_align, size, mode);
#if 0
	ret = nand_rw_start(size);
	if (ret < 0) {
		rec_err("nand_rw_start fail.\n");
		return -ENANDRW;
	}
#endif
	if (mode == 0)
		result = nand_read_raw_image(buf, offset_align, size);
	else
		result = nand_read_ext4_image(buf, offset_align, size);

	if (result != size)
	{
		rec_err("nand_read_%s_image fail.result=%lld\n",
				mode ? "ext4" : "raw", result);
		result = -ENANDWR;
	}
#if 0
	ret = nand_rw_end();
	if (ret < 0) {
		rec_err("nand_rw_end fail.\n");
		return -ENANDRW;
	}
#endif
	return result;
}

static int64_t nand_read_raw_image_align(void *buf, uint64_t offset_align,
										 uint64_t size)
{
	return __nand_read_image_align(buf, offset_align, size, 0);
}

static int64_t nand_read_ext4_image_align(void *buf, uint64_t offset_align,
										  uint64_t size)
{
	return __nand_read_image_align(buf, offset_align, size, 1);
}

#if 0
/*
* __nand_common_write -- write data into nand by @offset and @size,
* the offset nand page size/block size align is taken internal.
*
* buf, data buf.
* offset, location in nand, NOT require nand page size/block size align
* size, data size to write
*/
static int64_t __nand_common_write(void *buf, uint64_t offset,
	    uint64_t size, int mode)
{
	int64_t ret;
	uint64_t size_tmp;
	uint64_t size_org;
	uint64_t addr_align;
	uint32_t remainder;
	uint32_t blk_size;
	uint32_t blk_cnt;
	uint8_t *buf_tmp;

	if (buf == NULL) {
		return -EINVAL;
	}

	rec_dbg("offset=0x%llx, size=0x%llx, mode=%d\n", offset, size, mode);

	if (size == 0) {
		return 0;
	}

	ret = get_nand_block_size(&blk_size);
	if (ret < 0) {
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}
	rec_dbg("nand blk_size is 0x%x\n", blk_size);

	size_org = size;
	remainder = offset % blk_size;
	addr_align = offset - remainder;
	rec_dbg("addr_align=0x%llx, remainder=0x%x\n", addr_align, remainder);

	buf_tmp = (uint8_t *)malloc(blk_size);
	if (buf_tmp == NULL) {
		rec_err("malloc fail.\n");
		return -ENOMEM;
	}

	if (remainder) {
		/*
		* if remainder is NOT 0, it means it is non nand bloc size align.
		* handle non-align firstly.
		*
		*      <n * blk_size>               <(n+1)*blk_size>
		* -------|-------------------------|
		*               |                   |<--size_tmp>-->|
		*               |                   |
		*          <addr_align>      |
		*                                    |
		*                              <remainder>
		*/
		if (mode == 0)
			ret = nand_read_raw_image_align(buf_tmp, addr_align, blk_size);
		else
			ret = nand_read_ext4_image_align(buf_tmp, addr_align, blk_size);
		if (ret != blk_size) {
			rec_err("nand_read_%s_image_align fail--1.\n", mode ? "ext4" : "raw");
			free(buf_tmp);
			return -ENANDRD;
		}

		size_tmp = MIN((blk_size - remainder), size);
		rec_dbg("size_tmp=0x%llx\n", size_tmp);
		memcpy(buf_tmp + remainder, buf, size_tmp);

		/*
		* erase this one block
		*/
		ret = nand_erase(addr_align, 1);
		if (ret < 0) {
			rec_err("nand_erase fail -- 1.\n");
			free(buf_tmp);
			return -ENANDERASE;
		}

		if (mode == 0)
			ret = nand_write_raw_image_align(buf_tmp, addr_align, blk_size);
		else
			ret = nand_write_ext4_image_align(buf_tmp, addr_align, blk_size);
		if (ret < 0) {
			rec_err("nand_write_raw_image_align fail --1.\n");
			free(buf_tmp);
			return -ENANDWR;
		}
		addr_align += blk_size;
		buf += size_tmp;
		size -= size_tmp;
	}

	if (size == 0) {
		rec_dbg("write success -- 1.\n");
		free(buf_tmp);
		return size_org;
	}

	/*
	* if touch here, the first non page size align is already handled.
	* addr_align is block_size align now.
	*/
	rec_dbg("addr_align=0x%llx, size=0x%llx\n", addr_align, size);
	blk_cnt = size / blk_size;
	if (blk_cnt) {
		size_tmp = blk_size * blk_cnt;
		rec_dbg("blk_cnt=%d, size_tmp=0x%llx\n", blk_cnt, size_tmp);
		/*
		* erase blk_cnt blocks
		*/
		ret = nand_erase(addr_align, blk_cnt);
		if (ret < 0) {
			rec_err("nand_erase fail -- 2.\n");
			free(buf_tmp);
			return -ENANDERASE;
		}

		if (mode == 0)
			ret = nand_write_raw_image_align(buf, addr_align, size_tmp);
		else
			ret = nand_write_ext4_image_align(buf, addr_align, size_tmp);

		if (ret < 0) {
			rec_err("nand_write_%s_image_align fail --2.\n", mode ? "ext4" : "raw");
			free(buf_tmp);
			return -ENANDWR;
		}
		addr_align += size_tmp;
		buf += size_tmp;
		size -= size_tmp;
	}

	if (size == 0) {
		rec_dbg("write success -- 2.\n");
		free(buf_tmp);
		return size_org;
	}

	/*
	* if touch here, size is in [1, blk_size - 1].
	* addr_align is block_size align
	*/
	if (mode == 0)
		ret = nand_read_raw_image_align(buf_tmp, addr_align, blk_size);
	else
		ret = nand_read_ext4_image_align(buf_tmp, addr_align, blk_size);

	if (ret < 0) {
		rec_err("nand_read_%s_image_align fail -- 2.\n", mode ? "ext4" : "raw");
		free(buf_tmp);
		return -ENANDRD;
	}

	/*
	* erase this one block
	*/
	ret = nand_erase(addr_align, 1);
	if (ret < 0) {
		rec_err("nand_erase fail -- 3.\n");
		free(buf_tmp);
		return -ENANDERASE;
	}

	memcpy(buf_tmp, buf, size);
	if (mode == 0)
		ret = nand_write_raw_image_align(buf_tmp, addr_align, blk_size);
	else
		ret = nand_write_ext4_image_align(buf_tmp, addr_align, blk_size);

	if (ret < 0) {
		rec_err("nand_write_%s_image_align fail -- 3.\n", mode ? "ext4" : "raw");
		free(buf_tmp);
		return -ENANDWR;
	}

	rec_dbg("write success -- 3.\n");
	free(buf_tmp);
	return size_org;
}
#endif

#if 0
int64_t nand_raw_common_write(void *buf, uint64_t offset,
	    uint64_t size)
{
	return __nand_common_write(buf, offset, size, 0);
}
#endif
int64_t nand_raw_common_write(void *buf, uint64_t offset,
							  uint64_t size)
{
	return nand_write_raw_image_align(buf, offset, size);
}

#if 0
int64_t __nand_raw_common_write_with_rwctrl(void *buf, uint64_t offset,
	    uint64_t size, int part_idx)
{
	int ret = 0;
	int64_t result = 0;

	if (buf == NULL) {
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	/*
	* nand raw write does NOT care about image realsize, only ask realsize > 0.
	* --- from Nand driver owner comments.
	*
	* TODO, if nand driver owner has new method or update,
	* remeber review these code here.
	*/
#define NAND_RAW_WR_SIZE 0x100000UL // 1MB

	ret = nand_rw_start(part_idx, NAND_RAW_WR_SIZE);
	if (ret < 0) {
		rec_err("nand_rw_start fail.\n");
		return -ENANDRW;
	}

	result = __nand_common_write(buf, offset, size, 0);

	ret = nand_rw_end();
	if (ret < 0) {
		rec_err("nand_rw_end fail.\n");
		return -ENANDRW;
	}

	return result;
}
#endif

#if 0
/*
* nand_raw_common_write_with_rwctrl for emptycore upg use only.
*/
int64_t nand_raw_common_write_with_rwctrl(void *buf, uint64_t offset,
	    uint64_t size)
{
	return __nand_raw_common_write_with_rwctrl(buf, offset, size, lookup_idx_by_partname("allnand"));
}
#endif

#if 0
/*
* nand_raw_common_write_with_rwctrl_safeupg for safe upg use only.
*/
int64_t nand_raw_common_write_with_rwctrl_safeupg(
	    const char *partname, void *buf,
	    uint64_t offset, uint64_t size)
{
	return __nand_raw_common_write_with_rwctrl(buf, offset, size, lookup_idx_by_partname(partname));
}
#endif

int64_t __nand_raw_partition_write(void *buf,
								   uint64_t offset, uint64_t size,
								   uint64_t part_size, int part_idx)
{
	long long sizew = 0;
	long long org_realdata_size = 0;
	uint32_t blk_size = 0;
	uint32_t blk_cnt = 0;
	int ret = -1;

	rec_info("offset=0x%llx, size=0x%llx, partt_size=0x%llx\n",
			 offset, size, part_size);

	if (size <= 0)
	{
		rec_warn("realdata_size is 0.\n");
		return 0;
	}

	org_realdata_size = size;

	ret = get_nand_block_size(&blk_size);
	if (ret < 0)
	{
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}
	rec_dbg("blk_size=0x%x\n", blk_size);

	/* check offset if align with nand block size */
	if (offset % blk_size)
	{
		rec_err("offset(0x%llx) not align nand block size(0x%x)\n",
				offset, blk_size);
		return -ENANDALIGN;
	}

	/* calc how many nand blocks need to erase */
	// blk_cnt = (realdata_size + blk_size - 1) / blk_size;
	/* erase entire partition */
	blk_cnt = (part_size + blk_size - 1) / blk_size;
	rec_info("blk_cnt = 0x%x\n", blk_cnt);

	ret = nand_rw_start(part_idx, part_size);
	if (ret < 0)
	{
		rec_err("nand_rw_start fail.\n");
		return -ENANDRW;
	}

	/* erase nand blocks */
	ret = nand_erase(offset, blk_cnt);
	if (ret < 0)
	{
		rec_err("nand_erase fail.\n");
		return -ENANDERASE;
	}

	sizew = nand_raw_common_write(buf, offset, size);

	ret = nand_rw_end();
	if (ret < 0)
	{
		rec_err("nand_rw_end fail.\n");
		return -ENANDRW;
	}

	if (sizew < org_realdata_size)
	{
		rec_err("sizew(0x%llx) < realdata_size(0x%llx)\n",
				sizew, org_realdata_size);
		return -ENANDWR;
	}
	rec_info("succeeded, sizew(0x%llx)!\n", sizew);

	return sizew;
}

int64_t nand_raw_partition_write_by_emptycore(void *buf,
											  uint64_t offset, uint64_t size,
											  uint64_t part_size)
{
	return __nand_raw_partition_write(buf, offset, size, part_size,
									  lookup_idx_by_partname("allnand"));
}

int64_t nand_raw_partition_write_offset_by_emptycore(void *buf,
													 uint64_t offset, uint64_t offset_in_part,
													 uint64_t size, uint64_t real_size,
													 uint64_t part_size)
{
	void *aux_buffer = NULL;
	uint64_t blk_size = 0;
	uint64_t size_align = 0;
	int64_t res = 0;
	int ret = 0;

	if (size == 0)
		return 0;

	rec_info("part_start_addr=0x%llx, offset_in_part=0x%llx\n",
			 offset, offset_in_part);
	rec_info("size=0x%llx, real_size=0x%llx, part_size=0x%llx\n",
			 size, real_size, part_size);

	ret = get_nand_block_size((uint32_t *)&blk_size);
	if (ret < 0)
	{
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}
	rec_dbg("blk_size=0x%x\n", blk_size);

	/* check offset if align with nand block size */
	if (offset % blk_size)
	{
		rec_err("offset(0x%llx) not align nand block size(0x%llx)\n",
				offset, blk_size);
		return -ENANDALIGN;
	}

	if (real_size == 0)
		real_size = offset_in_part + size;

	size_align = ALIGN(real_size, blk_size);
	aux_buffer = malloc(size_align);
	if (aux_buffer == NULL)
	{
		rec_err("malloc fail.\n");
		return -ENOMEM;
	}

	res = nand_raw_partition_read_by_emptycore(aux_buffer, offset,
											   size_align, part_size);
	if (res != size_align)
	{
		rec_err("read nand fail.\n");
		free(aux_buffer);
		return -ENANDRD;
	}
	memcpy(aux_buffer + offset_in_part, buf, size);

	res = __nand_raw_partition_write(aux_buffer, offset, size_align, part_size,
									 lookup_idx_by_partname("allnand"));
	if (res != size_align)
	{
		rec_err("write nand fail\n");
		free(aux_buffer);
		return -ENANDWR;
	}

	free(aux_buffer);
	return size;
}

int64_t nand_raw_partition_write_by_safeupg(
	const char *partname, void *buf,
	uint64_t offset, uint64_t size, uint64_t part_size)
{
	return __nand_raw_partition_write(buf, offset, size, part_size,
									  lookup_idx_by_partname(partname));
}

int64_t nand_raw_partition_write_offset_by_safeupg(
	const char *partname, void *buf,
	uint64_t offset, uint64_t offset_in_part,
	uint64_t size, uint64_t real_size,
	uint64_t part_size)
{
	void *aux_buffer = NULL;
	uint64_t blk_size = 0;
	uint64_t size_align = 0;
	int64_t res = 0;
	int ret = 0;

	if (size == 0)
		return 0;

	ret = get_nand_block_size((uint32_t *)&blk_size);
	if (ret < 0)
	{
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}
	rec_dbg("blk_size=0x%x\n", blk_size);

	/* check offset if align with nand block size */
	if (offset % blk_size)
	{
		rec_err("offset(0x%llx) not align nand block size(0x%llx)\n",
				offset, blk_size);
		return -ENANDALIGN;
	}

	if (real_size == 0)
		real_size = offset_in_part + size;

	size_align = ALIGN(real_size, blk_size);
	rec_info("enter nand_raw_partition_read_offset_by_safeupg malloc");
	aux_buffer = malloc(size_align);
	if (aux_buffer == NULL)
	{
		rec_err("malloc fail.\n");
		return -ENOMEM;
	}
	rec_info("enter nand_raw_partition_read_offset_by_safeupg");
	res = nand_raw_partition_read_by_safeupg(partname, aux_buffer, offset,
											 size_align, part_size);
	if (res != size_align)
	{
		rec_err("read nand fail.\n");
		free(aux_buffer);
		return -ENANDRD;
	}
	memcpy(aux_buffer + offset_in_part, buf, size);

	res = __nand_raw_partition_write(aux_buffer, offset, size_align, part_size,
									 lookup_idx_by_partname(partname));
	if (res != size_align)
	{
		rec_err("write nand fail\n");
		free(aux_buffer);
		return -ENANDWR;
	}
	rec_info("succeeded nanand_raw_partition_write_offset_by_safeupg\n");
	free(aux_buffer);
	return size;
}

#if 0
int64_t nand_ext4_common_write(void *buf, uint64_t offset,
	    uint64_t size)
{
	return __nand_common_write(buf, offset, size, 1);
}
#endif

int64_t nand_ext4_common_write(void *buf, uint64_t offset,
							   uint64_t size)
{
	return nand_write_ext4_image_align(buf, offset, size);
}

/*
 * nand_raw_common_read -- read raw data fromnand by @offset and @size,
 * the offset nand page size align is taken internal.
 *
 * buf, data buf.
 * offset, location in nand, NOT require nand page size align
 * size, data size to write
 */
static int64_t __nand_common_read(void *buf, uint64_t offset,
								  uint64_t size, int mode)
{
	int ret;
	uint64_t size_tmp;
	uint64_t size_org;
	uint64_t addr_align;
	uint32_t remainder;
	uint32_t page_size;
	uint8_t *buf_tmp;

	if (buf == NULL)
	{
		return -EINVAL;
	}

	rec_dbg("offset=0x%llx, size=0x%llx\n", offset, size);

	if (size == 0)
	{
		return 0;
	}

	ret = get_nand_page_size(&page_size);
	if (ret < 0)
	{
		rec_err("get_nand_page_size fail.\n");
		return -ENANDINFO;
	}

	size_org = size;
	remainder = offset % page_size;
	addr_align = offset - remainder;
	rec_dbg("addr_align=0x%llx, remainder=0x%x\n", addr_align, remainder);

	if (remainder)
	{
		/*
		 * if remainder is NOT 0, it means it is non nand page size align.
		 * handle non-align firstly.
		 *
		 *      <n * page_size>               <(n+1)*page_size>
		 * -------|-------------------------|
		 *               |                   |<--size_tmp>-->|
		 *               |                   |
		 *          <addr_align>      |
		 *                                    |
		 *                              <remainder>
		 */
		buf_tmp = (uint8_t *)malloc(page_size);
		if (buf_tmp == NULL)
		{
			rec_err("malloc fail.\n");
			return -ENOMEM;
		}
		if (mode == 0)
			ret = nand_read_raw_image_align(buf_tmp, addr_align, page_size);
		else
			ret = nand_read_ext4_image_align(buf_tmp, addr_align, page_size);
		if (ret != page_size)
		{
			rec_err("nand_read_%s_image_align fail.\n", mode ? "ext4" : "raw");
			free(buf_tmp);
			return -ENANDRD;
		}

		size_tmp = MIN((page_size - remainder), size);
		rec_dbg("size_tmp=0x%llx\n", size_tmp);
		memcpy(buf, buf_tmp + remainder, size_tmp);

		free(buf_tmp);
		addr_align += page_size;
		buf += size_tmp;
		size -= size_tmp;
	}

	if (size == 0)
	{
		rec_dbg("read success -- 1.\n");
		return size_org;
	}

	/*
	 * if touch here, non page size align is already handled.
	 */
	rec_dbg("addr_align=0x%llx, size=0x%llx\n", addr_align, size);
	if (mode == 0)
		ret = nand_read_raw_image_align(buf, addr_align, size);
	else
		ret = nand_read_ext4_image_align(buf, addr_align, size);
	if (ret < 0)
	{
		rec_err("nand_read_%s_image_align fail -- 2.\n", mode ? "ext4" : "raw");
		return -ENANDWR;
	}

	rec_dbg("read success -- 2.\n");
	return size_org;
}

int64_t nand_raw_common_read(void *buf, uint64_t offset,
							 uint64_t size)
{
	return __nand_common_read(buf, offset, size, 0);
}

int64_t __nand_raw_partition_read(void *buf,
								  uint64_t offset, uint64_t size,
								  uint64_t part_size, int part_idx)
{
	int ret = 0;
	uint32_t blk_size = 0;
	int64_t result = 0;

	if (buf == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	ret = get_nand_block_size(&blk_size);
	if (ret < 0)
	{
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}
	rec_dbg("blk_size=0x%x\n", blk_size);

	/* check offset if align with nand block size */
	if (offset % blk_size)
	{
		rec_err("offset(0x%llx) not align nand block size(0x%x)\n",
				offset, blk_size);
		return -ENANDALIGN;
	}

	// rec_info("[qiyun debug] part_idx=%d\n", part_idx);
	ret = nand_rw_start(part_idx, part_size);
	if (ret < 0)
	{
		rec_err("nand_rw_start fail.\n");
		return -ENANDRW;
	}
	// rec_info("[qiyun debug] 1\n");

	result = __nand_common_read(buf, offset, size, 0);

	ret = nand_rw_end();
	if (ret < 0)
	{
		rec_err("nand_rw_end fail.\n");
		return -ENANDRW;
	}

	return result;
}

/*
 * nand_raw_partition_read_by_emptycore for emptycore upg use only.
 */
int64_t nand_raw_partition_read_by_emptycore(void *buf,
											 uint64_t offset, uint64_t size, uint64_t part_size)
{
	return __nand_raw_partition_read(buf, offset, size, part_size,
									 lookup_idx_by_partname("allnand"));
}

int64_t nand_raw_partition_read_offset_by_emptycore(void *buf,
													uint64_t offset, uint64_t offset_in_part,
													uint64_t size, uint64_t part_size)
{
	void *aux_buffer = NULL;
	int64_t res = 0;

	if (offset_in_part == 0)
		return __nand_raw_partition_read(buf, offset, size, part_size,
										 lookup_idx_by_partname("allnand"));

	rec_dbg("begin...\n");
	aux_buffer = malloc(offset_in_part + size);
	if (aux_buffer == NULL)
	{
		rec_err("malloc fail.\n");
		return -ENOMEM;
	}

	res = __nand_raw_partition_read(aux_buffer, offset, offset_in_part + size,
									part_size, lookup_idx_by_partname("allnand"));
	if (res != offset_in_part + size)
	{
		rec_err("__nand_raw_partition_read fail.\n");
		free(aux_buffer);
		return -ENANDRD;
	}

	memcpy(buf, aux_buffer + offset_in_part, size);
	free(aux_buffer);
	rec_dbg("end\n");
	return size;
}

/*
 * nand_raw_partition_read_by_safeupg for safe upg use only.
 */
int64_t nand_raw_partition_read_by_safeupg(
	const char *pt_name, void *buf,
	uint64_t offset, uint64_t size,
	uint64_t part_size)
{
	return __nand_raw_partition_read(buf, offset, size, part_size,
									 lookup_idx_by_partname(pt_name));
}

int64_t nand_raw_partition_read_offset_by_safeupg(
	const char *pt_name, void *buf,
	uint64_t offset, uint64_t offset_in_part,
	uint64_t size, uint64_t part_size)
{
	void *aux_buffer = NULL;
	int64_t res = 0;

	if (offset_in_part == 0)
		return __nand_raw_partition_read(buf, offset, size, part_size,
										 lookup_idx_by_partname(pt_name));

	aux_buffer = malloc(offset_in_part + size);
	if (aux_buffer == NULL)
	{
		rec_err("malloc fail.\n");
		return -ENOMEM;
	}

	res = __nand_raw_partition_read(aux_buffer, offset, offset_in_part + size,
									part_size, lookup_idx_by_partname(pt_name));
	if (res != offset_in_part + size)
	{
		rec_err("__nand_raw_partition_read fail.\n");
		free(aux_buffer);
		return -ENANDRD;
	}

	memcpy(buf, aux_buffer + offset_in_part, size);
	free(aux_buffer);
	return size;
}

int64_t nand_ext4_common_read(void *buf, uint64_t offset,
							  uint64_t size)
{
	return __nand_common_read(buf, offset, size, 1);
}


static int __upg_partition_from_file(const char *file,
                                     int part_idx, long long offset,
                                     long long size, long long *psize_total, int mode)
{
    int ret = 0;
    int fd = 0;
    long long sizer = 0, sizew = 0, sizetotal = 0;
    unsigned long long file_len = 0;
    unsigned long long org_file_len = 0;
    char *buffer = NULL;
    off64_t cur_offset = 0;
    uint32_t blk_size = 0;
    uint32_t buf_len = 0;
    uint32_t read_len = 0;
    uint32_t blk_cnt = 0;

    // Minizip相关变量
    void *zip_reader = NULL;
    mz_zip_file *file_info = NULL;
    const char *extract_file_name = NULL;

    if (file == NULL) {
        rec_err("parameter is NULL .\n");
        return -EINVAL;
    }
    // 检查是否为ZIP文件
	int is_zip_file = 1;
    if (is_zip_file) {
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
				file_len = file_info->uncompressed_size;
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


    } else {
        // 原有逻辑：处理普通文件
        ret = get_file_len(file, &file_len);
        if (ret < 0) {
            rec_err("file_len get fail.\n");
            return -EFILELEN;
        }

        fd = open(file, O_RDONLY | O_LARGEFILE, 0);
        if (fd < 0) {
            rec_err("open %s fail.\n", file);
            return -ESYSCALL;
        }
    }

    org_file_len = file_len;

    ret = get_nand_block_size(&blk_size);
    if (ret < 0) {
        rec_err("get_nand_block_size fail.\n");
        if (is_zip_file) {
            mz_zip_reader_entry_close(zip_reader);
            mz_zip_reader_close(zip_reader);
            mz_zip_reader_delete(&zip_reader);
        } else {
            close(fd);
        }
        return -ENANDINFO;
    }

    /* check offset if align with nand block size */
    if (offset % blk_size) {
        rec_err("offset(0x%llx) not align nand block size(0x%x)\n",
                offset, blk_size);
        if (is_zip_file) {
            mz_zip_reader_entry_close(zip_reader);
            mz_zip_reader_close(zip_reader);
            mz_zip_reader_delete(&zip_reader);
        } else {
            close(fd);
        }
        return -ENANDALIGN;
    }

    /* calc how many nand blocks need to erase */
    blk_cnt = (size + blk_size - 1) / blk_size;

    if (blk_size > FILE_RW_SIZE)
        buf_len = blk_size;
    else
        buf_len = FILE_RW_SIZE;
    buf_len = ALIGN(buf_len, blk_size);

    buffer = (char *)malloc(buf_len);
    if (!buffer) {
        if (is_zip_file) {
            mz_zip_reader_entry_close(zip_reader);
            mz_zip_reader_close(zip_reader);
            mz_zip_reader_delete(&zip_reader);
        } else {
            close(fd);
        }
        return -ENOMEM;
    }

    ret = nand_rw_start(part_idx, size);
    if (ret < 0) {
        rec_err("nand_rw_start fail.\n");
        if (is_zip_file) {
            mz_zip_reader_entry_close(zip_reader);
            mz_zip_reader_close(zip_reader);
            mz_zip_reader_delete(&zip_reader);
        } else {
            close(fd);
        }
        free(buffer);
        return -ENANDRW;
    }

    /* erase nand blocks */
    ret = nand_erase(offset, blk_cnt);
    if (ret < 0) {
        rec_err("nand_erase fail.\n");
        if (is_zip_file) {
            mz_zip_reader_entry_close(zip_reader);
            mz_zip_reader_close(zip_reader);
            mz_zip_reader_delete(&zip_reader);
        } else {
            close(fd);
        }
        free(buffer);
        return -ENANDERASE;
    }

    sizetotal = 0;
    while (file_len > 0) {
        if (is_zip_file) {
            // 使用Minizip读取数据
            if (file_len >= buf_len) {
                sizer = mz_zip_reader_entry_read(zip_reader, buffer, buf_len);
                read_len = buf_len;
            } else {
                sizer = mz_zip_reader_entry_read(zip_reader, buffer, file_len);
                read_len = file_len;
            }
        } else {
            // 原有逻辑：从普通文件读取数据
            if (file_len >= buf_len) {
                sizer = read(fd, buffer, buf_len);
                read_len = buf_len;
            } else {
                sizer = read(fd, buffer, file_len);
                read_len = file_len;
            }
        }

        if (read_len != sizer) {
            rec_err("read fail.\n");
            break;
        }

        if (mode)
            sizew = nand_ext4_common_write(buffer, offset + sizetotal, sizer);
        else
            sizew = nand_raw_common_write(buffer, offset + sizetotal, sizer);

        sizetotal += sizew;
        file_len -= sizer;
        if (sizew < sizer)
            break;
    }

    if (is_zip_file) {
        mz_zip_reader_entry_close(zip_reader);
        mz_zip_reader_close(zip_reader);
        mz_zip_reader_delete(&zip_reader);
    } else {
        close(fd);
    }
    free(buffer);

    if (mode) {
        ret = nand_partition_reserve_blk_check(offset, size);
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

    if (sizetotal < org_file_len) {
        rec_err(" faild! the size of file(%s) is 0x%llx bytes, only 0x%llx bytes are write into emmc/nand.\n",
                file, org_file_len, sizetotal);
        return -ESYSCALL;
    }

    *psize_total = sizetotal;

    rec_info("succeeded!\n");
    return 0;
}

#if 0
static int __upg_raw_partition_from_file(const char *file,
	    int part_idx, long long offset,
	    long long size, long long *psize_total)
{
	int ret = 0;
	int fd = 0;
	long long sizer = 0, sizew = 0, sizetotal = 0;
	unsigned long long file_len = 0;
	unsigned long long org_file_len = 0;
	char *buffer = NULL;
	off64_t cur_offset = 0;
	uint32_t blk_size = 0;
	uint32_t buf_len = 0;
	uint32_t read_len = 0;

	if (file == NULL) {
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	ret = get_file_len(file, &file_len);
	if (ret < 0) {
		rec_err("file_len get fail.\n");
		return -EFILELEN;
	}
	rec_info("offset(0x%llx) file(%s), length(0x%llx)\n",
		    offset, file, file_len);
	org_file_len = file_len;

	ret = get_nand_block_size(&blk_size);
	if (ret < 0) {
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}
	rec_dbg("blk_size is 0x%x\n", blk_size);

	if (blk_size > FILE_RW_SIZE)
		buf_len = blk_size;
	else
		buf_len = FILE_RW_SIZE;
	buf_len = ALIGN(buf_len, blk_size);
	rec_dbg("buf_len=0x%x\n", buf_len);

	fd = open(file, O_RDONLY | O_LARGEFILE, 0);
	if (fd < 0) {
		rec_err("open %s fail.\n", file);
		return -ESYSCALL;
	}

	buffer = (char *)malloc(buf_len);
	if (!buffer) {
		close(fd);
		return -ENOMEM;
	}

#if 1
	ret = nand_rw_start(part_idx, file_len);
	if (ret < 0) {
		close(fd);
		free(buffer);
		rec_err("nand_rw_start fail.\n");
		return -ENANDRW;
	}
#endif
	
	sizetotal = 0;
	while (file_len > 0) {
		if (file_len >= buf_len) {
			sizer = read(fd, buffer, buf_len);
			read_len = buf_len;
		} else {
			sizer = read(fd, buffer, file_len);
			read_len = file_len;
		}

		if (read_len != sizer) {
			rec_err("read fail.\n");
			break;
		}

		rec_dbg("offset + sizetotal=0x%llx, sizer=0x%llx\n",
			    offset + sizetotal, sizer);

		sizew = nand_raw_common_write(buffer, offset + sizetotal, sizer);

		sizetotal += sizew;
		file_len -= sizer;
		if (sizew < sizer)
			break;
	}

	close(fd);
	free(buffer);

#if 1
	ret = nand_rw_end();
	if (ret < 0) {
		rec_err("nand_rw_end fail.\n");
		return -ENANDRW;
	}
#endif
	if (sizetotal < org_file_len) {
		rec_err(" faild! the size of file(%s) is 0x%llx bytes, only 0x%llx bytes are write into emmc/nand.\n",
			    file, org_file_len, sizetotal);
		return -ESYSCALL;
	}

	*psize_total = sizetotal;

	rec_info("succeeded!\n");
	return 0;
}
#endif
static int __upg_raw_partition_from_file(const char *file,
										 int part_idx, long long offset,
										 long long size, long long *psize_total)
{
	return __upg_partition_from_file(file,
									 part_idx, offset,
									 size, psize_total, 0);
}

#if 0
static int __upg_ext4_partition_from_file(const char *file,
	    int part_idx, long long offset,
	    long long size, long long *psize_total)
{
	int ret = 0;
	int fd = 0;
	long long sizer = 0, sizew = 0, sizetotal = 0;
	unsigned long long file_len = 0;
	unsigned long long org_file_len = 0;
	char *buffer = NULL;
	off64_t cur_offset = 0;
	uint32_t blk_size = 0;
	uint32_t buf_len = 0;
	uint32_t read_len = 0;
	uint32_t blk_cnt = 0;

	if (file == NULL) {
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	ret = get_file_len(file, &file_len);
	if (ret < 0) {
		rec_err("file_len get fail.\n");
		return -EFILELEN;
	}
	rec_info("offset(0x%llx) file(%s), length(0x%llx)\n",
		    offset, file, file_len);
	org_file_len = file_len;

	ret = get_nand_block_size(&blk_size);
	if (ret < 0) {
		rec_err("get_nand_block_size fail.\n");
		return -ENANDINFO;
	}
	rec_dbg("blk_size is 0x%x\n", blk_size);

	/* check offset if align with nand block size */
	if (offset % blk_size) {
		rec_err("offset(0x%llx) not align nand block size(0x%x)\n",
			    offset, blk_size);
		return -ENANDALIGN;
	}

	/* calc how many nand blocks need to erase */
	//blk_cnt = (file_len + blk_size - 1) / blk_size;
	/* erase entire partition */
	blk_cnt = (size + blk_size - 1) / blk_size;
	rec_info("blk_cnt = 0x%x\n", blk_cnt);

	if (blk_size > FILE_RW_SIZE)
		buf_len = blk_size;
	else
		buf_len = FILE_RW_SIZE;
	buf_len = ALIGN(buf_len, blk_size);
	rec_dbg("buf_len=0x%x\n", buf_len);

	fd = open(file, O_RDONLY | O_LARGEFILE, 0);
	if (fd < 0) {
		rec_err("open %s fail.\n", file);
		return -ESYSCALL;
	}

	buffer = (char *)malloc(buf_len);
	if (!buffer) {
		close(fd);
		return -ENOMEM;
	}

	ret = nand_rw_start(part_idx, file_len);
	if (ret < 0) {
		close(fd);
		free(buffer);
		rec_err("nand_rw_start fail.\n");
		return -ENANDRW;
	}

	/* erase nand blocks */
	ret = nand_erase(offset, blk_cnt);
	if (ret < 0) {
		close(fd);
		free(buffer);
		rec_err("nand_erase fail.\n");
		return -ENANDERASE;
	}
	
	sizetotal = 0;
	while (file_len > 0) {
		if (file_len >= buf_len) {
			sizer = read(fd, buffer, buf_len);
			read_len = buf_len;
		} else {
			sizer = read(fd, buffer, file_len);
			read_len = file_len;
		}

		if (read_len != sizer) {
			rec_err("read fail.\n");
			break;
		}

		rec_dbg("offset + sizetotal=0x%llx, sizer=0x%llx\n",
			    offset + sizetotal, sizer);

		sizew = nand_ext4_common_write(buffer, offset + sizetotal, sizer);

		sizetotal += sizew;
		file_len -= sizer;
		if (sizew < sizer)
			break;
	}

	close(fd);
	free(buffer);

	ret = nand_rw_end();
	if (ret < 0) {
		rec_err("nand_rw_end fail.\n");
		return -ENANDRW;
	}

	if (sizetotal < org_file_len) {
		rec_err(" faild! the size of file(%s) is 0x%llx bytes, only 0x%llx bytes are write into emmc/nand.\n",
			    file, org_file_len, sizetotal);
		return -ESYSCALL;
	}

	*psize_total = sizetotal;

	rec_info("succeeded!\n");
	return 0;
}
#endif
static int __upg_ext4_partition_from_file(const char *file,
										  int part_idx, long long offset,
										  long long size, long long *psize_total)
{
	return __upg_partition_from_file(file,
									 part_idx, offset,
									 size, psize_total, 1);
}

int upg_raw_partition_from_file(const char *devorpt_name, const char *file, long long offset, long long size, long long *psize_total)
{
	return __upg_raw_partition_from_file(file,
										 lookup_idx_by_partname(devorpt_name), offset,
										 size, psize_total);
}

#if 0
int upg_ext4_partition_from_file(const char *devname,
	    const char *file, long long offset, long long size)
{
	long long size_total;

	return __upg_partition_from_file(file, 1, offset, size, &size_total);
}
#endif

int upg_ext4_partition_from_file(const char *devorpt_name,
								 const char *file, long long offset, long long size)
{
	long long size_total;

	return __upg_ext4_partition_from_file(file,
										  lookup_idx_by_partname(devorpt_name), offset,
										  size, &size_total);
}

static uint32_t calc_checksum_from_nand(long long offset,
										long long realdata_size, long long part_size,
										int mode, int pt_idx)
{
	long long sizer = 0;
	char *buffer = NULL;
	uint32_t read_len = 0;
	uint32_t chksum = 0;
	int64_t (*nand_read_fn)(void *buf, uint64_t offset, uint64_t size) = NULL;
	int ret = 0;
	int result = 0;

	rec_info("offset=0x%llx, realdata_size=0x%llx, mode=%d\n",
			 offset, realdata_size, mode);

	if (realdata_size <= 0)
	{
		rec_warn("realdata_size is 0.\n");
		return 0;
	}

	if (mode)
		nand_read_fn = nand_ext4_common_read;
	else
		nand_read_fn = nand_raw_common_read;

	buffer = (char *)malloc(FILE_RW_SIZE);
	if (buffer == NULL)
	{
		rec_err("malloc fail.\n");
		return 0;
	}

	ret = nand_rw_start(pt_idx, part_size);
	if (ret < 0)
	{
		rec_err("nand_rw_start fail.\n");
		free(buffer);
		return 0;
	}

	while (realdata_size > 0)
	{
		if (realdata_size >= FILE_RW_SIZE)
		{
			sizer = nand_read_fn(buffer, offset, FILE_RW_SIZE);
			read_len = FILE_RW_SIZE;
		}
		else
		{
			sizer = nand_read_fn(buffer, offset, realdata_size);
			read_len = realdata_size;
		}

		if (sizer != read_len)
		{
			rec_err("nand read fail.\n");
			result = -ENANDRD;
			break;
		}

		chksum = checksum32(chksum, buffer, read_len);
		realdata_size -= read_len;
		offset += read_len;
	}

	free(buffer);
	ret = nand_rw_end();
	if (ret < 0)
	{
		rec_err("nand_rw_end fail.\n");
		return 0;
	}

	if (result < 0)
		return result;

	rec_info("succeeded, chksum(0x%x)!\n", chksum);

	return chksum;
}

uint32_t calc_checksum_from_nand_after_upg(
	long long offset, long long realdata_size,
	partitionread *pentry, int upg_stage)
{
	int mode = 0;
	int pt_idx = 0;

	if (pentry == NULL)
		return 0x87654321;

	if (realdata_size <= 0)
		return 0;

	if (strcmp(pentry->szPartName, "preloader") == 0 ||
		strcmp(pentry->szPartName, "preloader_bk") == 0 ||
		strcmp(pentry->szPartName, "datazone") == 0 ||
		strcmp(pentry->szPartName, "datazone_bk") == 0)
	{
		return 0;
	}

	mode = strcmp(pentry->szType, "ext4") == 0 ? 1 : 0;
	if (upg_stage)
	{
		/* safeupg */
		pt_idx = lookup_idx_by_partname(pentry->szPartName);
		rec_info("safeupg pt_idx=%d\n", pt_idx);
	}
	else
	{
		/* emptycore upg */
		pt_idx = lookup_idx_by_partname("allnand");
		rec_info("emptycore pt_idx=%d\n", pt_idx);
	}

	return calc_checksum_from_nand(offset, realdata_size,
								   pentry->u8PartitionSize, mode, pt_idx);
}

int nand_clear_all_protect(void)
{
	FILE *fdwp;
	int n = 0, ret = 0;

	rec_info("enter nand_clear_all_protect fopen\n");
	fdwp = fopen("/proc/nand_wp", "w");
	if (fdwp == NULL)
	{
		rec_err("fopen /proc/nand_wp fail\n");
		return -ESYSCALL;
	}

	rec_info("enter nand_clear_all_protect fwrite\n");
	n = fwrite("clear_all_wp", 1, 12, fdwp);
	if (n != 12)
	{
		rec_err("fwrite clear_all_wp to /proc/nand_wp failed, result=%d\r\n", n);
		ret = -ESYSCALL;
		fclose(fdwp);
		return ret;
	}

	fclose(fdwp);
	return ret;
}

#endif /* CONFIG_BOOT_MMC */

int udisk_mount(const char *mount_point)
{
	int res = 0;

	if (mount_point == NULL)
	{
		rec_err("parameter is NULL .\n");
		return -EINVAL;
	}

	res = ensure_path_mounted(mount_point);
	if (res < 0)
	{
		res = -EMOUNT;
	}
	return res;
}

int ntfs_mount(char *device, char *mount_point)
{
	int res = -1;

	if (device && mount_point)
	{
		rec_info("iso_file: %s, mount_point:%s\n", device, mount_point);
	}
	else
	{
		rec_err("arguments invalid.\n");
		return -EINVAL;
	}

#if 0
	//res = mount(iso_file, mount_point, "iso9660", MS_RDONLY, NULL);
	res = mount(iso_file, mount_point, "iso9660", MS_RDONLY, NULL);

	return res;
#else
	pid_t pid = fork();
	if (pid == 0)
	{
		char *argv[] = {"/usr/bin/ntfs-3g", (char *)NULL, (char *)NULL, (char *)NULL};
		argv[1] = device;
		argv[2] = mount_point;
		execv(argv[0], argv);
		// rec_err("Can't run %s (%s)\n", argv[0], strerror(errno));
		// execv run full new program, and will not touch here, except execv fail.
		_exit(-1);
	}

	int status;
	waitpid(pid, &status, 0);
	return status;
#endif
}

#if 0
void print_buf(char *buf, int n)
{
	int i;
	rec_dbg("--------------------------------------------.\n");
	for (i=0;i<n;i++) {
		if(i%16 == 0)
			printf("\n");
		printf("%4x ",buf[i]);
	}
	printf("\n");
	rec_dbg("--------------------------------------------.\n\n");
	
}

void fill_buf_by_seg(char *buf,int start_num,int n)
{
	int i;

	for (i = 0;i<n;i++) {
		buf[i] = start_num +i;
	}
}

#define DATAZONE_OFFSET 0x100000UL
#define BCB_OFFSET 0x101000UL
#define PTBL_OFFSET 0x102000UL

#define BUFFER_LEN 256

void nand_read_write_test(void)
{
	char wr_buf[BUFFER_LEN];
	char rd_buf[BUFFER_LEN];
	int ret;

	/*
	*write 0x0,0x1,0x2.....0xFF into datazone
	*
	*
	*test-1
	*
	*
	*/
	fill_buf_by_seg(wr_buf,0x0,BUFFER_LEN);
	ret = nand_raw_common_write_with_rwctrl(wr_buf,DATAZONE_OFFSET,BUFFER_LEN);
	if(ret < 0) {
		rec_err("write dz fail-1.\n");
		while(1);
		
	}
	rec_info("write dz success .\n");

	/*
	*read data from datazone
	*
	*
	*/
	ret = nand_raw_common_read_with_rwctrl(rd_buf,DATAZONE_OFFSET,BUFFER_LEN);
	if(ret < 0) {
		rec_err("read dz fail-1.\n");
		while(1);
		
	}
	rec_info("read dz success-1.\n");
	rec_info("--------datazone data-1------------.\n");
	print_buf(rd_buf,BUFFER_LEN);

	/*
	* wirte 0x80,0x81,x082.....0x1ff ...0x0,0x1.....0x79 into bcb
	*
	* test-2
	*
	*/
	fill_buf_by_seg(wr_buf,0x80,BUFFER_LEN);
	ret = nand_raw_common_write_with_rwctrl(wr_buf,BCB_OFFSET,BUFFER_LEN);
	if(ret < 0) {
		rec_err("write bcb fail-2.\n");
		while(1);
		
	}
	rec_info("write bcb success-2.\n");

	/*
	*read data from datazone
	*
	*
	*/
	ret = nand_raw_common_read_with_rwctrl(rd_buf,DATAZONE_OFFSET,BUFFER_LEN);
	if(ret < 0) {
		rec_err("read dz fail-2.\n");
		while(1);
		
	}
	rec_info("read dz success-2.\n");
	rec_info("--------datazone data-2------------.\n");
	print_buf(rd_buf,BUFFER_LEN);

	/*
	*read data from bcb
	*
	*
	*/
	ret = nand_raw_common_read_with_rwctrl(rd_buf,BCB_OFFSET,BUFFER_LEN);
	if(ret < 0) {
		rec_err("read bcb fail-2.\n");
		while(1);
		
	}
	rec_info("read bcb success-2.\n");
	rec_info("--------bcb data-2------------.\n");
	print_buf(rd_buf,BUFFER_LEN);

		/*
	*read data from datazone
	*
	*
	*/
	ret = nand_raw_common_read_with_rwctrl(rd_buf,DATAZONE_OFFSET,BUFFER_LEN);
	if(ret < 0) {
		rec_err("read dz fail-2.\n");
		while(1);
		
	}
	rec_info("read dz success-2.\n");
	rec_info("--------datazone data-2------------.\n");
	print_buf(rd_buf,BUFFER_LEN);


	/*
	* wirte 0xC0,,0xC1,0xC2.....0xFF,0x0,0x1......0xBF into ptb1
	*
	* test-3
	*
	*/
	fill_buf_by_seg(wr_buf,0xc0,BUFFER_LEN);
	ret = nand_raw_common_write_with_rwctrl(wr_buf,PTBL_OFFSET,BUFFER_LEN);
	if(ret < 0) {
		rec_err("write bcb fail-3.\n");
		while(1);
		
	}
	rec_info("write bcb success-3.\n");

	/*
	*read data from datazone
	*
	*
	*/
	ret = nand_raw_common_read_with_rwctrl(rd_buf,DATAZONE_OFFSET,BUFFER_LEN);
	if(ret < 0) {
		rec_err("read bcb fail-3.\n");
		while(1);
		
	}
	rec_info("read datazone success-3.\n");
	rec_info("--------datazone data-3------------.\n");
	print_buf(rd_buf,BUFFER_LEN);

		/*
	*read data from bcb
	*
	*
	*/
	ret = nand_raw_common_read_with_rwctrl(rd_buf,BCB_OFFSET,BUFFER_LEN);
	if(ret < 0) {
		rec_err("read bcb fail-3.\n");
		while(1);
		
	}
	rec_info("read bcb success-3.\n");
	rec_info("--------bcb data-3------------.\n");
	print_buf(rd_buf,BUFFER_LEN);

/*
*read data from ptbl
*
*
*/
ret = nand_raw_common_read_with_rwctrl(rd_buf,PTBL_OFFSET,BUFFER_LEN);
if(ret < 0) {
	rec_err("read ptbl fail-3.\n");
	while(1);
	
}
rec_info("read ptbl success-3.\n");
rec_info("--------ptbl data-3------------.\n");
print_buf(rd_buf,BUFFER_LEN);

	
}
#endif
