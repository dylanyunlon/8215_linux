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

#ifndef ATCMOUNTSERVICEPARSER_H
#define ATCMOUNTSERVICEPARSER_H

#include "UeventList.hpp"
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <sys/un.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include "Utils.hpp"
#include "Vfat.hpp"
#include "Exfat.hpp"
#include "Process.hpp"
#include "ATCMountServiceImpl.hpp"

using namespace std;

#define NEEDFSCK true

class ATCMountServiceParser {
public:
	explicit ATCMountServiceParser();
	virtual ~ATCMountServiceParser();

	static ATCMountServiceParser *Instance();
	void setWatcher(std::shared_ptr<ATCMountServiceImpl> w) {watcher=w;} 

	bool start();
	bool stop();
	void broadcastUnmounted(const std::string &mountPoint);
	void broadcastMounted(const std::string &msg, const std::string &uuid);
	void broadcastPhyEject(const std::string &msg, const std::string &uuid);
	void broadcastEject(const std::string &msg, const std::string &uuid);
	void broadcastInsert(const std::string &msg, const std::string &uuid);
private:
	static ATCMountServiceParser *sInstance;
	static std::shared_ptr<ATCMountServiceImpl> watcher;
	static pthread_t thread;
	static bool threadTerminated;
	int init();

	static int mountPartition(std::string& devicePath, std::string& mountPoint, std::string& fsType, std::string& fsLabel, std::string& uuid, std::string& partUuid);
	static int unmountPartition(std::string devName, std::string mountedPartition);
	static void handleInsert(const ATCUevent& uevent);
	static void handleRemove(const ATCUevent& uevent);
	static void * parseUevent(void *);
};

#endif

