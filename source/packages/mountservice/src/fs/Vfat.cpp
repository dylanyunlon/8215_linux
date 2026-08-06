/*
*copyright (c) 2018 AutoChips Inc.
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
#include "Vfat.hpp"
#include "Utils.hpp"
#include "ATCMountServiceParser.hpp"

//static const char* kMkfsPath = "/system/bin/newfs_msdos";
static const char* kFsckPath = "/usr/bin/fsck.msdos";

#define MKFS_VFAT      \
	"mkfs.vfat"
#include <string>

namespace mountservice {
namespace vfat {
	bool IsSupported() {
		bool ret = access(kFsckPath, X_OK) == 0;
		atc_syslogd (" %s vfat::IsSupported() = %s!\r\n", kFsckPath, ret?"true":"false");
		return ret;
	}

	int Check(const std::string& source) {
		if (!IsSupported()) {
			atc_syslogd ("Skipping VFAT filesystem checks!\r\n");
			return -1;
		}
		int pass = 1;
		do {
			int rec = 3;
			std::string exec;
			std::shared_ptr<ExecThread> execThread;
			exec = kFsckPath;
			exec = exec + " -p -f " + source;
			//rec = system(exec.c_str());

			int pipefd[2];
			if (pipe(pipefd) < 0) {
				atc_sysloge("vfat::Check pipe failed\r\n");
				return -1;
			}
			execThread = std::make_shared<ExecThread>(exec,pipefd[1]);
			execThread->start();

			int mEpollFd = epoll_create(2);
			struct epoll_event eventItem;
			memset(&eventItem, 0, sizeof(epoll_event));
			eventItem.events = EPOLLIN;
			eventItem.data.fd = pipefd[0];
			epoll_ctl(mEpollFd, EPOLL_CTL_ADD, pipefd[0], &eventItem);
			struct epoll_event eventItems[2];
			int eventCount = epoll_wait(mEpollFd, eventItems, 2, VFST_CHECK_TMO);
			for (int i = 0; i < eventCount; i++) {
				int fd = eventItems[i].data.fd;
				unsigned int epollEvents = eventItems[i].events;
				if (fd == pipefd[0]) {
					if (epollEvents & EPOLLIN) {
						read(fd, &rec, sizeof(int));
					}
				}
			}
			close(mEpollFd);
			close(pipefd[0]);
			close(pipefd[1]);

			switch(rec) {
				case 0:
					atc_syslogd ("vfat Filesystem check completed OK!\r\n");
					return 0;
				case 2:
					atc_syslogd ("vfat::Check Filesystem check failed (not a FAT filesystem)");
					errno = ENODATA;
					return -1;
				case 3:
					atc_sysloge ("vfat::Check Filesystem check timeout!\r\n");
					errno = ENODATA;
					return -1;
				case 4:
					if (pass++ <= 3) {
						atc_sysloge ("vfat::Check Filesystem modified - rechecking (pass: %d)", pass);
						continue;
					}
					atc_sysloge ( "vfat::Check Failing check after too many rechecks");
					errno = EIO;
					return -1;
				case 8:
					atc_sysloge ("vfat::Check Filesystem check failed (no filesystem)");
					errno = ENODATA;
					return -1;
				default:
					atc_sysloge ("vfat::Check Filesystem check failed (unknown exit code %d)!\r\n", rec);
					errno = EIO;
					return -1;
			}
		}while(1);

		return 0;
	}

	int Mount(const std::string& source, const std::string& target) {
		if (!IsSupported()) {
			atc_syslogd ("Skipping fs checks!\r\n");
			return -1;
		}
		const char* c_source = source.c_str();
		const char* c_target = target.c_str();
		const char* c_opt = "utf8,usefree,errors=continue";
		unsigned long flags = MS_NODEV | MS_NOSUID | MS_DIRSYNC | MS_NOATIME | MS_NODIRATIME;
		int rec = mount(c_source, c_target, "vfat", flags, c_opt);
		while (rec && errno == EROFS) {
			atc_sysloge ("%s appears to be a read only filesystem - retrying mount RO!\r\n", c_source);
			flags |= MS_RDONLY;
			rec = mount(c_source, c_target, "vfat", flags, c_opt);
		}
		if(rec)
			atc_sysloge ("vfat: failed to mount(%s, %s):%s!\r\n", c_source, c_target, strerror(errno));

		return rec;
	}
// a global latch may necessary
	int Format(const std::string& source, unsigned numSectors) {
		std::vector<std::string> outPut;
		std::vector<std::string> partInfo;
		std::string uuid;
		std::string partUUid;
		std::string devPath;
		std::string fsType;
		std::string fsLabel;
		auto parser = ATCMountServiceParser::Instance();
		int rt = 0;

		char *formatArgs[] = {
			MKFS_VFAT,
			const_cast<char *>(source.c_str()),
			NULL,
		};

		if (source.empty()) {
			atc_syslogi("device node is empty!\r\n");
			return -1;
		}
		atc_syslogi("numSectors is not supported currently(vfat), ignored!\r\n");
		rt = readMountedPartitions(source, outPut);
		if (rt < 0) {
			return rt;
		}
		if (outPut.size() != 1) {
			atc_sysloge("unexpected outPath.size() = %lu\r\n", outPut.size());
			return -1;
		}
		atc_sysloge("source : %s, outPut : %s", source.c_str(), outPut[0].c_str());
		readPartitions(source, partInfo);
		if (partInfo.size() != 1) {
			atc_sysloge("unexpected partInfo.size() = %lu\r\n", partInfo.size());
			return -1;
		}

		atc_syslogi("path : %s, partInfo : %s\r\n", outPut[0].c_str(), partInfo[0].c_str());
		rt = readPartitionInfo(partInfo[0], devPath, fsType, fsLabel, uuid, partUUid);
		if (rt < 0) {
			atc_sysloge("readPartitionInfo failure\r\n");
			return rt;
		}
		atc_syslogi("fsType : %s, uuid : %s\n", fsType.c_str(), uuid.c_str());
		parser->broadcastPhyEject(outPut[0], uuid);
		parser->broadcastEject(outPut[0], uuid);
		rt = forceUnmount(outPut[0]);
		if (rt < 0) {
			atc_sysloge("forceUmount failure\r\n");
			return rt;
		}
		removePath(outPut[0]);
		parser->broadcastUnmounted(outPut[0]);
		atc_syslogi("remove path : %s\n", outPut[0]);
		rt = fork_execv(formatArgs[0], formatArgs);
		if (rt) {
			atc_sysloge("command execute failure! rt = %d, errno = %d, error = %s, fs = vfat\r\n", rt, errno, strerror(errno));
		}
		if (access(outPut[0].c_str(), F_OK) != 0) {
			rt = mkdir(outPut[0].c_str(), 0777);
			if (rt != 0) {
				atc_sysloge ("mount point(%s) create fail %s)!\r\n", outPut[0].c_str(), strerror(errno));
				return rt;
			}
		}
		atc_syslogi("old uuid is %s\n", uuid.c_str());
		readPartitions(source, partInfo);
		rt = readPartitionInfo(partInfo[0], devPath, fsType, fsLabel, uuid, partUUid);
		if (rt < 0) {
			atc_sysloge("readPartitionInfo failed!\n");
			return rt;
		}
		atc_syslogi("new uuid is %s\n", uuid.c_str());
		parser->broadcastInsert(outPut[0], uuid);
		rt = Mount(source, outPut[0]);
		if (rt < 0) {
			return rt;
		}
		parser->broadcastMounted(outPut[0], uuid);
		return rt;
	}
}
}


