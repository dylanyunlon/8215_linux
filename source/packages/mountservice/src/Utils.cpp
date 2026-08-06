/*
copyright (c) 2018 AutoChips Inc.
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

#include "Utils.hpp"

#include <syslog.h>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <atomic>
#include <thread>
#include <mutex>

static const char* kProcFilesystems = "/proc/filesystems";
static const char* kSgdiskPath = "/usr/sbin/sgdisk";
static const char* kBlkidPath = "/sbin/blkid";
static const char* kMountPath = "mount";
static pthread_mutex_t begin_mutex;
static bool begin_operat = false;

#define RESIZE_EXT4   "/sbin/resize2fs"
#define BUFFERSIZE 4096

#define MAX_MOUNT_POINT_LEN 128

#define ext4_read(buffer,off) \
(((uint8_t *)buffer)[(off)] + (((uint8_t *)buffer)[(off)+1] << 8) + \
(((uint8_t *)buffer)[(off)+2] << 16) + (((uint8_t *)buffer)[(off)+3] << 24))


int64_t getCurrentTimeMs(void) {
	struct timeval tv;
	memset(&tv, 0, sizeof(tv));
	gettimeofday(&tv, NULL);

	return ((int64_t)tv.tv_usec/1000 + (int64_t)tv.tv_sec * 1000);
}

/**
* Appends src to string dst of size siz (unlike strncat, siz is the
* full size of dst, not space left).  At most siz-1 characters
* will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
* Returns strlen(src) + MIN(siz, strlen(initial dst)).
* If retval >= siz, truncation occurred.
**/
size_t strlcat(char *dst, const char *src, size_t siz) {
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));

	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));	/* count does not include NUL */

}

/*
* Copy src to string dst of size siz.  At most siz-1 characters
* will be copied.  Always NUL terminates (unless siz == 0).
* Returns strlen(src); if retval >= siz, truncation occurred.
*/
size_t strlcpy(char *dst, const char *src, size_t siz) {
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

/*
* Initiliaze the switch controller, which contains whether process the hot-plug uevent of external storage device
*/
void initSwitchController() {
	pthread_mutex_init(&begin_mutex, NULL);
	return;
}

/*
* Destroy the switch controller, which contains whether process the hot-plug uevent of external storage device
*/
void destroySwitchController() {
	pthread_mutex_destroy(&begin_mutex);
	return;
}

/*
* Begin to process the hot-plug uevent of external storage device
*/

bool openAndWrite(const char *filename, const char *msg) {
	int fd = open(filename, O_RDWR);
	int rt = 0;

	if (fd < 0) {
		atc_sysloge(" failed to open %s, error : %s\n", filename, strerror(errno));
		return false;
	}
	rt = write(fd, msg, strlen(msg));
	if (rt < 0) {
		atc_sysloge(" failed to write %s, error : %s\n", filename, strerror(errno));
		close(fd);
		return false;
	}
	close(fd);
	return true;
}
void beginProcess() {
	static std::once_flag flag;
	try {
		std::call_once(flag, []() {
			bool rt = openAndWrite("/proc/sys/vm/dirty_expire_centisecs", "200");
			if (rt == false) {
			    throw std::logic_error("failed to write /proc/sys/vm/dirty_expire_centisecs");
			}
			rt = openAndWrite("/proc/sys/vm/dirty_writeback_centisecs", "300");
			if (rt == false) {
				throw std::logic_error("failed to write /proc/sys/vm/dirty_writeback_centisecs");
			}
			atc_syslogi("successfully set sync interval!\n");
		});
	} catch (std::exception &e) {
		atc_syslogw("write sync time error : %s\n", e.what());
	}
	pthread_mutex_lock(&begin_mutex);
	begin_operat = true;
	pthread_mutex_unlock(&begin_mutex);
	return;
}

/*
* End processing the hot-plug uevent of external storage device
*/
void pauseProcess() {
	pthread_mutex_lock(&begin_mutex);
	begin_operat = false;
	pthread_mutex_unlock(&begin_mutex);
	return;
}

/*
* Check whether process,  which processes the hot-plug uevent of external storage device, is began
*/
bool processBegan() {
	bool ret = false;
	pthread_mutex_lock(&begin_mutex);
	ret = begin_operat;
	pthread_mutex_unlock(&begin_mutex);
	return ret;
}

static int openFile(int *fd) {
	int ret = -1;
	int openFd = open(LAST_APP_SAVE_FILE, O_RDWR | O_CLOEXEC);
	if (-1 == openFd) {
		if (errno == ENOENT) {
			atc_sysloge("file not exist\r\n");
			ret = -1;
		} else {
			atc_sysloge("open fail\r\n");
			ret = -1;
		}
	}
	*fd = openFd;
	return ret;
}

static int readFile(int fd, int *appID) {
	int ret = 0;
	int i = 0;
	while (-1 != ret) {
		ret = read(fd, &appID[i], sizeof(int));
		if (ret == 0) {
			break;
		}
		i++;
	}

	return ret;
}

/**
* check whether exsist last app
*      true: last app exsistes
*      true: last app does not exsist
**/
bool hasLastAPP() {
	int fd = 0;
	bool ret = false;
	int appID[2] = {0};

	openFile(&fd);
	readFile(fd, appID);
	for (int i = 0; i < 2; i++) {
		if (appID[i]) {
			ret = true;
			atc_syslogd("has last app\r\n");
		}
	}
	close(fd);
	return ret;
}

/**
* check whether home UI has already shown
* return value:
*      true: home UI has already shown
*      false:home UI has not already shown
**/
bool homeReady() {
	if (access(HOME_READY_FILE, F_OK) == 0) {
		atc_syslogd("Home App has already done!\r\n");
		return true;
	} else {
		atc_syslogd("Home App has not already done!\r\n");
		return false;
	}
}

#define MAXDEVICEPORTNUMBER  10
#define MAXDEVICEPORTLEVEL   10

std::string mount_point_array[MAXDEVICEPORTNUMBER][MAXDEVICEPORTLEVEL]  =
	{{"usb1", "udisk1"},
	 {"usb2", "udisk2"},
	 {"mmc1", "ext_sdcard1"},
	 {"mmc2", "ext_sdcard2"}
	};

/*
* Rule:
*		 1. SD1, ext_sdcard1(signal partition), ext_sdcard1p[partition_sequence](multi partitions)
*		 2. SD2, ext_sdcard2(signal partition), ext_sdcard2p[partition_sequence](multi partitions)
*		 3. USB PORT1, udisk1(signal partition), udisk1p[partition_sequence](multi partitions)
*		 4. USB PORT2, udisk2(signal partition), udisk2p[partition_sequence](multi partitions)
*		 5. usb device serted to USB PORT1 from HUB, udisk1dev[device_sequence](signal partition), udisk1dev[device_sequence]p[partition_sequence](multi partitions)
*		 6. usb device serted to USB PORT2 from HUB, udisk2dev[device_sequence](signal partition), udisk2dev[device_sequence]p[partition_sequence](multi partitions)
*        7. partSeq is normally the value of partition minor
*/
int generateMountPoint(const std::string& eventPath, const int partSeq, std::string& mountPoint){
	int hub_level = 0;
	int ret = 0;
	char mount_point[128] = "";
	char mount_point_path[128] = "";
	bool find_device = false;
	char buff_num[8] = "";

	mountPoint.clear();

	for(int i = 0; i < MAXDEVICEPORTNUMBER; i++) {
		if(("" == mount_point_array[i][0]) || ("" == mount_point_array[i][1])) {
			continue;
		}

		for(int j = 0; j < MAXDEVICEPORTLEVEL/2; j++) {
			if(("" == mount_point_array[i][j*2]) || ("" == mount_point_array[i][j*2+1])) {
				hub_level = j - 1;
				break;
			}
			if(eventPath.find(mount_point_array[i][j*2]) != std::string::npos) {
				find_device = true;
				strlcat(mount_point, mount_point_array[i][j*2+1].c_str(), sizeof(mount_point));
				if(j > 0) {
					const std::string& matched = mount_point_array[i][j*2];
					sprintf(buff_num, "%d", matched[matched.size()-1] - '1' + 1);
					strlcat(mount_point, buff_num, sizeof(mount_point));
				}
			} else {
				if(0 == j) {
					break;
				} else {
					hub_level = j - 1;
					break;
				}
			}
		}

		if(find_device) {
			break;
		}
	}

	strlcat(mount_point_path, MOUNTPOINTPATHPREFIX, sizeof(mount_point_path));
	if(partSeq) {
		strlcat(mount_point_path, mount_point, sizeof(mount_point_path));
		if (access(mount_point_path, F_OK) != 0) {
			ret = mkdir(mount_point_path, 0777);
			if (ret != 0) {
				atc_sysloge ("mount point(%s) create fail(%s)!\r\n", mount_point_path, strerror(errno));
				return ret;
			}
		}
		strlcat(mount_point_path, "/", sizeof(mount_point_path));
		strlcat(mount_point_path, mount_point, sizeof(mount_point_path));
		strlcat(mount_point_path, "_partition", sizeof(mount_point_path));
		sprintf(buff_num, "%d", partSeq);
		strlcat(mount_point_path, buff_num, sizeof(mount_point_path));
	} else {
		strlcat(mount_point_path, mount_point, sizeof(mount_point_path));
	}
	if (find_device) {
		mountPoint = mount_point_path;
		atc_syslogd ("mount point of(%s) is:(%s)!\r\n", eventPath.c_str(), mountPoint.c_str());
	} else {
		atc_sysloge ("generate mount point for (%s) fail!\r\n", eventPath.c_str());
		return -1;
	}

	if (access(mount_point_path, F_OK) != 0) {
		ret = mkdir(mount_point_path, 0777);
		if (ret != 0) {
			atc_sysloge ("mount point(%s) create fail%s)!\r\n", mount_point_path, strerror(errno));
		}
	}

	return ret;
}
#if 0
int initMountPoint()
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	char *xmlValue = NULL;
	int tag = 0;

	doc = xmlParseFile(MOUNT_POINT_CONFIG_FILE);

	if (doc == NULL) {
		atc_sysloge("open file fail! (%s)\n", MOUNT_POINT_CONFIG_FILE);
		return -1;
	}

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		atc_sysloge("empty document!\n");
		xmlFreeDoc(doc);
		return -1;
	}

	cur = cur->xmlChildrenNode;

	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)USB_PORT)) {
			xmlValue = (char *)xmlGetProp(cur, (const xmlChar *) TAG_MOUNTPOINT);
			if (xmlValue) {
				if (strlen(xmlValue) < MAX_MOUNT_POINT_LEN) {
					mount_point_array[tag][1]= (std::string)xmlValue;
				}
			}
			xmlFree(xmlValue);

			xmlValue = (char *)xmlGetProp(cur, (const xmlChar *) TAG_HUBPREFIX);
			if (xmlValue) {
				if (strlen(xmlValue) < MAX_MOUNT_POINT_LEN) {
					mount_point_array[tag][3]= (std::string)xmlValue;
					mount_point_array[tag][5]= (std::string)xmlValue;
				}
			}
			tag++;
			xmlFree(xmlValue);

		} else if (!xmlStrcmp(cur->name, (const xmlChar *)SD_SLOT)) {
			xmlValue = (char *)xmlGetProp(cur, (const xmlChar *) TAG_MOUNTPOINT);
			if (xmlValue) {
				if (strlen(xmlValue) < MAX_MOUNT_POINT_LEN)  {
					mount_point_array[tag][1]= (std::string)xmlValue;
				}
			}
			xmlFree(xmlValue);
			tag++;
		}
		cur = cur->next;
	}

	return 0;
}
#endif
/** 
* remove the parent directory of multi-partition's mount point
**/
void removePath(const std::string& path) {
	int ret = 0;

	if(path.empty()) {
		atc_sysloge ("mount point remove fail (mount point NULL)!\r\n");
	} else if (access(path.c_str(), F_OK) == 0) {
		ret = rmdir(path.c_str());
		if (ret != 0) {
			atc_sysloge ("mount point(%s) remove fail(%s)!\r\n", path.c_str(), strerror(errno));
		} else {
			atc_syslogd ("mount point(%s) remove succeed!\r\n", path.c_str());
		}
	} else {
		atc_sysloge ("mount point(%s) create remove fail(%s does not exsit)!\r\n", path.c_str());
	}

	return;
}

/**
* remove the parent directory of multi-partition's mount point
**/
void removePathRecursion(const std::string& path) {
	int ret = 0;
	std::string parent_path;
	std::size_t loc = 0;

	if(path.empty()) {
		atc_sysloge ("path remove fail (path is NULL)!\r\n");
		return;
	} else if(path.find(EXTERNALDEVICEROOTDIRECTORY, 0) == std::string::npos) {
		atc_sysloge ("path remove fail (%s is not under /media)!\r\n", path.c_str());
		return;
	} else if(path == EXTERNALDEVICEROOTDIRECTORY) {
		atc_sysloge ("path remove fail (path is root directory of external device:%s)!\r\n", path.c_str());
		return;
	} else if (access(path.c_str(), F_OK) == 0) {
		ret = rmdir(path.c_str());
		if (ret != 0) {
			atc_sysloge ("path remove fail(%s, %s)!\r\n", path.c_str(), strerror(errno));
		}
	}

	loc = path.rfind("/");
	if ((loc != std::string::npos) && (loc > 0)) {
		parent_path = path.substr(0, loc);
		removePathRecursion(parent_path);
	}

	return;
}

/** get the all the partition info of the device specified by devName:
*     1. when get partition info fail, the retuen value is -errno;
*     2. when get partition info succeed, the retuen value is 0;
**/
int readPartitions(std::string devName, std::vector<std::string>& outPut) {
	char cmd[128];
	char line[1024];
	memset(line, 0x0, sizeof(line));

	sprintf(cmd, "%s", kBlkidPath);

	outPut.clear();
	FILE* fp = popen(cmd, "r");
	if (!fp) {
		atc_sysloge ("readPartitions:Failed to popen(%s)!\r\n", cmd);
		return -errno;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		if ((std::string(line).find(devName, 0) != std::string::npos) &&
			(std::string(line).find(" TYPE=", 0) != std::string::npos)){
			outPut.push_back(std::string(line));
		}

		memset(line, 0x0, sizeof(line));
	}

	if (pclose(fp) != 0) {
		atc_sysloge ("readPartitions:Failed to pclose(%s)!\r\n", cmd);
		return -errno;
	}

	return 0;
}


/** whether the partition specified by mountPoint is mounted:
*     1. when get partition is mounted, the retuen value is true;
*     2. when get partition is not mounted, the retuen value is false;
**/
bool pathMounted(std::string mountPoint) {
	char cmd[128];
	char line[1024];
	memset(line, 0x0, sizeof(line));

	sprintf(cmd, "%s", kMountPath);

	FILE* fp = popen(cmd, "r");
	if (!fp) {
		atc_sysloge ("pathMounted:Failed to popen(%s)!\r\n", cmd);
		return false;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		if ((std::string(line).find(mountPoint, 0) != std::string::npos) &&
			((std::string(line).find(" type fuseblk ", 0) != std::string::npos) ||
			(std::string(line).find(" type vfat ", 0) != std::string::npos))){
			atc_syslogd ("pathMounted:mountPoint(%s, %s) has been mounted!\r\n", mountPoint.c_str(), line);
			return true;
		}
		memset(line, 0x0, sizeof(line));
	}

	if (pclose(fp) != 0) {
		atc_sysloge ("pathMounted:Failed to pclose(%s)!\r\n", cmd);
	}
	return false;

}
/** get the all the partition info of the device specified by devName:
*     1. when get partition info fail, the retuen value is -errno;
*     2. when get partition info succeed, the retuen value is 0;
**/
int readPartitionWithoutPartitionTable(std::string devName, std::vector<std::string>& outPut) {
	char cmd[128];
	char line[1024];
	memset(line, 0x0, sizeof(line));

	sprintf(cmd, "%s", kBlkidPath);

	outPut.clear();
	FILE* fp = popen(cmd, "r");
	if (!fp) {
		atc_sysloge ("Failed to popen(%s)!\r\n", cmd);
		return -errno;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		if ((std::string(line).find(devName, 0) != std::string::npos)){
			outPut.push_back(std::string(line));
		}

		memset(line, 0x0, sizeof(line));
	}

	if (pclose(fp) != 0) {
		atc_sysloge ("Failed to pclose(%s)!\r\n", cmd);
		return -errno;
	}

	return 0;
}


/** get the all the mounted partition info of the device specified by devName:
*     1. when get mounted partition info fail, the retuen value is -errno;
*     2. when get mounted partition info succeed, the retuen value is 0;
**/
int readMountedPartitions(std::string devName, std::vector<std::string>& outPut) {
	char cmd[128];
	char line[1024];
	memset(line, 0x0, sizeof(line));
	sprintf(cmd, "%s", kMountPath);

	int ret = 0;
	int begin_pos = 0;
	int end_pos = 0;
	std::string mount_point;

	outPut.clear();
	FILE* fp = popen(cmd, "r");
	if (!fp) {
		atc_sysloge ("Failed to popen(%s)!\r\n", cmd);
		return -errno;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		const std::string tmp = std::move(std::string(line));
		size_t nEnd = tmp.find(MOUNTPOINTPATHPREFIX);

		 // shrink the match range which help make sure the line matched is exactly what we want
		nEnd = ((nEnd == std::string::npos) ? tmp.size() : nEnd);
		if ((tmp.substr(0, nEnd).find(devName, 0) != std::string::npos) &&
			((tmp.find(" type fuseblk", 0) != std::string::npos) ||
			(tmp.find(" type vfat", 0) != std::string::npos))){
			begin_pos = tmp.find(MOUNTPOINTPATHPREFIX);
			end_pos = tmp.find(TYPE);
			if (begin_pos != std::string::npos && end_pos != std::string::npos) {
				mount_point = tmp.substr(begin_pos, end_pos - begin_pos);
				outPut.push_back(mount_point);
			}
		}
		memset(line, 0x0, sizeof(line));
	}

	if (pclose(fp) != 0) {
		atc_sysloge ("Failed to pclose(%s)!\r\n", cmd);
		return -errno;
	}

	return 0;
}


/** get the all the filesystem info
*     parameter:
*      1. partInfo: output of readPartitions
*      2. fsType: file system type(vfat\exfat\ntfs)
*      3. fsLabel: file system label( not support for chinese)
*      4. uuid: uuid
*      5. partUuid: partition uuid
*
*    return value:
*     1. get the filesystem info fail, the retuen value is -1;
*     2. get the filesystem info succeed, the retuen value is 0;
**/
int readPartitionInfo(std::string partInfo, std::string& devicePath, std::string& fsType, std::string& fsLabel, std::string& uuid, std::string& partUuid) {
	char value[128];
	int ret = 0;
	devicePath.clear();
	fsType.clear();
	fsLabel.clear();
	uuid.clear();
	partUuid.clear();

	int index = partInfo.find(":");
	std::string temp_devicePath = partInfo.substr(0, index);
	devicePath = temp_devicePath;

	const char* cline = partInfo.c_str();
	const char* start = strstr(cline, " TYPE=");
	if (start != NULL && sscanf(start + 6, "\"%127[^\"]\"", value) == 1) {
		fsType = value;
	} else {
		atc_sysloge ("get TYPE fail(%s)!\r\n", partInfo.c_str());
		ret = -1;
	}

	start = strstr(cline, " UUID=");
	if (start != NULL && sscanf(start + 6, "\"%127[^\"]\"", value) == 1) {
		uuid = value;
	} else {
		atc_sysloge ("get UUID fail(%s)!\r\n", partInfo.c_str());
	}

	start = strstr(cline, " LABEL=");
	if(start != NULL && sscanf(start + 7, "\"%127[^\"]\"", value) == 1) {
		//not support the volume label, which has chinese
		if((!strstr(value,"M-")) && (!strstr(value,"?"))) {
			int i = 0;
			for(; (i < 128) && (value[i] != '\0'); i++) {
				fsLabel = value;
				break;
			}
		}
	} else {
		atc_sysloge ("get LABEL fail(%s)!\r\n", partInfo.c_str());
	}

	start = strstr(cline, " PARTUUID=");
	if(start != NULL && sscanf(start + 10, "\"%127[^\"]\"", value) == 1) {
		partUuid = value;
	} else {
		atc_sysloge ("get PARTUUID fail(%s)!\r\n", partInfo.c_str());
	}

	return ret;
}

/** 
*     1. Unmount the partition, specified by parameter path
*     2. when unmount the partition fail, because the partitition is used, kill the process, then try unmount the partition again;
**/
int forceUnmount(const std::string& path) {
	const char* cpath = path.c_str();

	for(int i = 0; i < 10; i++) {
		if (!umount2(cpath, UMOUNT_NOFOLLOW) || errno == EINVAL || errno == ENOENT) {
			return 0;
		}
		usleep(500000);
	}

	atc_sysloge ("First Failed to unmount (%s, %s)! and kill process\r\n", cpath, strerror(errno));
	Process::killProcessesWithOpenFiles(cpath, SIGINT);

	for(int i = 0; i < 10; i++) {
		if (!umount2(cpath, UMOUNT_NOFOLLOW) || errno == EINVAL || errno == ENOENT) {
			return 0;
		}
		usleep(500000);
	}
	atc_sysloge ("Secondary Failed to unmount (%s, %s)!and kill process\r\n", cpath, strerror(errno));
	Process::killProcessesWithOpenFiles(cpath, SIGTERM);

	for(int i = 0; i < 10; i++) {
		if (!umount2(cpath, UMOUNT_NOFOLLOW) || errno == EINVAL || errno == ENOENT) {
			return 0;
		}
		usleep(500000);
	}
	atc_sysloge ("Third Failed to unmount (%s, %s)!and kill process\r\n", cpath, strerror(errno));
	Process::killProcessesWithOpenFiles(cpath, SIGKILL);

	for(int i = 0; i < 10; i++) {
		if (!umount2(cpath, UMOUNT_NOFOLLOW) || errno == EINVAL || errno == ENOENT) {
			return 0;
		}
		usleep(500000);
	}

	atc_sysloge ("Failed to unmount (%s, %s)!\r\n", cpath, strerror(errno));
	return -1;
}

static void do_coldboot(DIR *d, int lvl) {
	struct dirent *de;
	int dfd, fd;

	dfd = dirfd(d);

	fd = openat(dfd, "uevent", O_WRONLY | O_CLOEXEC);
	if(fd >= 0) {
		write(fd, "add\n", 4);
		close(fd);
	}

	while((de = readdir(d))) {
		DIR *d2;

		if (de->d_name[0] == '.')
			continue;

		if (de->d_type != DT_DIR && lvl > 0)
			continue;

		fd = openat(dfd, de->d_name, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
		if(fd < 0)
			continue;

		d2 = fdopendir(fd);
		if(d2 == 0)
			close(fd);
		else {
			do_coldboot(d2, lvl + 1);
			closedir(d2);
		}
	}
}

/** trigger all the storage device to send uevent. 
*     1. this is used, when the atcmountservice begins, which start processing all the storage devices;
**/
void coldboot(const char *path) {
	DIR *d = opendir(path);
	if(d) {
		do_coldboot(d, 0);
		closedir(d);
	}
}

int fork_execv(const char *path, char * const argv[]) {
	pid_t pid;
	int ret=0;

	atc_syslogd("[fork_execv]start ###path:%s , PATH env : %s\r\n", path, getenv("PATH"));
	if((pid=fork())<0){
		atc_sysloge("[fork_exec]fork error\r\n");
	} else if (pid==0) {
		std::stringstream ss;
		for(int i = 0; argv[i] != NULL; ++i) {
			if (i >= 5 && i % 5 == 0) {
				atc_syslogw("more than %d args, take care!\r\n", i);
			}
			ss << argv[i] << " ";
		}
		atc_syslogd("%s exec %s\n",path, ss.str().c_str());
		if((ret = execvp(path,argv) ) < 0)
			atc_sysloge("[fork_exec]execv %s error, ret = %d, errno = %d,  error = %s\r\n", path, ret, errno, strerror(errno));
		exit(-1);
	} else {
		int status=0;
		if(waitpid(pid,&status,0)==pid){
			if(WIFEXITED(status)) {
				ret=WEXITSTATUS(status);
				atc_syslogd("[fork_exec]child exited with code %d\r\n", ret);
			} else if (WIFSIGNALED(status)) {
				ret=WTERMSIG(status);
				atc_syslogd("[fork_exec]%d signal case child aborted\r\n", ret);
			} else if (WIFSTOPPED(status)) {
				ret=WSTOPSIG(status);
				atc_sysloge("[fork_exec] %d signal child stopped\r\n", ret);
			} else if(WIFCONTINUED(status)) {
				ret=-2;
				atc_sysloge("[fork_exec]child stopped by SIGCONT\r\n");
			}
		} else {
			atc_sysloge("[fork_exec]waitpid error\r\n");
			ret=-2;
		}
	}
	return ret;

}

unsigned long long get_block_device_size(char * devPath)
{
	int fd;
	int ret;
	unsigned long long block_size = 0;
	if ((fd = open(devPath, O_RDONLY)) < 0) {
		atc_sysloge("Open block fail:%s.\r\n", (char*)strerror(errno));
		return 0;
	}
	ret = ioctl(fd, BLKGETSIZE64, &block_size);
	close(fd);

	if (ret)
		return 0;

	return block_size;
}

int resize_partition (char * devPath) {
	int fdin;
	int ret = 0;
	long long data_partition_size = 0;
	unsigned long long super_block_size = 0;
	int blk_size = 0;
	char buff[BUFFERSIZE];
	fdin = open(devPath, O_RDONLY);
	if (fdin < 0) {
		atc_sysloge("open(%s) failed\r\n", devPath);
		return -1;
	}
	ret = read(fdin,buff,BUFFERSIZE);
	if (ret != BUFFERSIZE) {
		atc_sysloge("read(%s) failed\r\n", devPath);
		return -1;
	}
	close(fdin);
	if (buff[0x418] == 0) {
		blk_size = 1024;
	} else if (buff[0x418] == 1) {
		blk_size = 2048;
	} else if (buff[0x418] == 2) {
		blk_size = 4096;
	}

	super_block_size = (unsigned long long)ext4_read(buff,0x404) * blk_size / 1024;
	atc_syslogd("super_block_size is %lluK\r\n", super_block_size);
	if ((data_partition_size = get_block_device_size(devPath)) < 0) {
		atc_sysloge("Get %s partition size fail.\r\n", devPath);
		return -1;
	}
	atc_syslogd("%s partition size is %lluK\\rn", devPath, data_partition_size / 1024);
	if (data_partition_size / 1024 == super_block_size)
		atc_sysloge("%s Partition has been resized,so nothing to do!\r\n", devPath);
	else {
		char *resize_ext4_argv[] = {
			RESIZE_EXT4,
			"-f",
			devPath,
			NULL
		};
		fork_execv(RESIZE_EXT4, resize_ext4_argv);
	}
	return 0;

}

