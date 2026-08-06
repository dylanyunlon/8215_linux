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

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#include <syslog.h>
#include <thread>
#include <string>
#include <vector>
#include "Process.hpp"
#include <pthread.h>

#define TAG "[ATCMountService 1.0.0]"

#define LOG_FILE	"/media/sdcard/atcmountservice.log"

#include <stdarg.h>

static inline void wrap_syslog(int level, const char *format, ...) {
	va_list args;
	FILE *fp = fopen(LOG_FILE, "a+");
	if (fp != nullptr) {
		va_start(args, format);
		vfprintf(fp, format, args);
		va_end(args);
		fflush(fp);
		fclose(fp);
	}
	va_start(args, format);
	vsyslog(level, format, args);
	va_end(args);
}

#define atc_syslogd(format, ...)    wrap_syslog(LOG_DEBUG,   TAG "[I]" format, ##__VA_ARGS__)
#define atc_syslogi(format, ...)    wrap_syslog(LOG_INFO,    TAG "[I]" format, ##__VA_ARGS__)
#define atc_syslogn(format, ...)    wrap_syslog(LOG_NOTICE,  TAG "[N]" format, ##__VA_ARGS__)
#define atc_syslogw(format, ...)    wrap_syslog(LOG_WARNING, TAG "[W]" format, ##__VA_ARGS__)
#define atc_sysloge(format, ...)    wrap_syslog(LOG_ERR,     TAG "[E]" format, ##__VA_ARGS__)
#define atc_syslogc(format, ...)    wrap_syslog(LOG_CRIT,    TAG "[C]" format, ##__VA_ARGS__)
#define atc_sysloga(format, ...)    wrap_syslog(LOG_ALERT,   TAG "[A]" format, ##__VA_ARGS__)
#define atc_syslogm(format, ...)    wrap_syslog(LOG_EMERG,   TAG "[M]" format, ##__VA_ARGS__)

#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4
#define MOUNTPOINTPATHPREFIX "/media/"
#define EXTERNALDEVICEROOTDIRECTORY "/media"
#define TYPE " type "
#define LAST_APP_SAVE_FILE "/data/startapp.txt"
#define MOUNT_POINT_CONFIG_FILE "/etc/mountpoint.xml"
#define HOME_READY_FILE "/tmp/homeready"
#define USB_PORT "Udisk"
#define SD_SLOT "SDcard"
#define VFST_CHECK_TMO (15000)
#define EMMC_MAJOR 179
#define EMMC_MINOR 0
#define DEFAULT_SECTOR_NUM (8u)

static const char* TAG_MOUNTPOINT = "MountPoint";
static const char* TAG_HUBPREFIX = "HubPrefix";
static const char* TAG_PORT = "Port";


int64_t getCurrentTimeMs(void);
size_t strlcat(char *dst, const char *src, size_t siz);
size_t strlcpy(char *dst, const char *src, size_t siz);

void initSwitchController();
void destroySwitchController();
void beginProcess();
void pauseProcess();
bool processBegan();

bool hasLastAPP();
bool homeReady();

int generateMountPoint(const std::string& eventPath, const int partSeq, std::string& mountPoint);
int readPartitions(std::string devName, std::vector<std::string>& outPut);
int readPartitionWithoutPartitionTable(std::string devName, std::vector<std::string>& outPut);
int readMountedPartitions(std::string devName, std::vector<std::string>& outPut) ;
int readPartitionInfo(std::string partInfo, std::string& devicePath, std::string& fsType, std::string& fsLabel, std::string& uuid, std::string& partUuid);
bool pathMounted(std::string mountPoint);
int fork_execv(const char *path, char * const argv[]);

int forceUnmount(const std::string& path);
void removePath(const std::string& path);
void removePathRecursion(const std::string& path);

void coldboot(const char *path);

unsigned long long get_block_device_size(char * devPath);
int resize_partition (char * devPath);

class ExecThread : public std::enable_shared_from_this<ExecThread> {

public:
	ExecThread(std::string cmd, int fd):
		mCmd(cmd),mFd(fd) {
	}
	virtual ~ExecThread() {
	}

	virtual bool threadLoop() {
		int rc = system(mCmd.c_str());
		int ret = rc >> 8; //actual return value is 8~15bit of system call return values
		write(mFd, &ret, sizeof(int));
		return false;
	}

	bool start() {
		mThread = std::thread(&ExecThread ::threadLoop, shared_from_this());
		mThread.detach();
		return true;
	}

private:
	std::string mCmd;
	int mFd;
	//static pthread_t mThread;
	std::thread mThread;
};

int initMountPoint();

//#difine 
#endif
