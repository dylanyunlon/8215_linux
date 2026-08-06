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
#ifndef ATCMOUNTSERVICELISTENER_H_
#define ATCMOUNTSERVICELISTENER_H_

#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include "UeventList.hpp"

using namespace std;

#define HOTPLUG_BUFFER_SIZE 1024
#define HOTPLUG_NUM_ENVP 32
#define OBJECT_SIZE 512
struct uevent {
	void *next;
	char buffer[HOTPLUG_BUFFER_SIZE + OBJECT_SIZE];
	char *devpath;
	char *action;
	char *envp[HOTPLUG_NUM_ENVP];
};

class ATCMountServiceListener {
public:
	explicit ATCMountServiceListener();
	virtual ~ATCMountServiceListener();

	static ATCMountServiceListener *Instance();

	bool start();
	bool stop();

private:
	static ATCMountServiceListener *sInstance;
	static pthread_t thread;
	static bool threadTerminated;

	static int mSock;
	int init();
	static void * listenUevent(void *);
};
#endif
