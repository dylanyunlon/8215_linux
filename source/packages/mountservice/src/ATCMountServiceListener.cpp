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
#include "ATCMountServiceListener.hpp"
#include "ATCSingleton.hpp"

ATCMountServiceListener *ATCMountServiceListener::sInstance = NULL;
pthread_t ATCMountServiceListener::thread = 0;
bool ATCMountServiceListener::threadTerminated = true;
int ATCMountServiceListener::mSock = -1;

static struct uevent *alloc_uevent(void) {
	return ( struct uevent *)malloc(sizeof(struct uevent));
}

static void free_uevent(uevent *uevent_to_free) {
	free(uevent_to_free);
	return;
}

ATCMountServiceListener::ATCMountServiceListener() {
	//test code
	mSock = 0;
}

ATCMountServiceListener::~ATCMountServiceListener() {
	//test code
	mSock = 0;
}

ATCMountServiceListener *ATCMountServiceListener::Instance() {
	return Singleton<ATCMountServiceListener>::getInstance();
}

bool ATCMountServiceListener::start() {
	int err = -1;
	bool ret = true;

	if (thread) {
		atc_syslogd("ATCMountServiceListener has been started!\r\n");
		return ret;
	}
	err = init();
	if (err == 0) {
		err = pthread_create(&thread, NULL, listenUevent, NULL);
		if (err != 0) {
			ret = false;
			atc_sysloge ("create thread(ATCMountServiceListener) fail\r\n");
		} else {
			threadTerminated = true;
			ret = true;
			atc_sysloge ("create thread(ATCMountServiceListener) succeed !\r\n");
		}
	} else {
		atc_sysloge ("thread(ATCMountServiceListener) init failed!\r\n");
		ret = false;
	}
	return ret;
}

bool ATCMountServiceListener::stop() {
	int err = -1;
	bool ret = false;
	if (thread) {
		err = pthread_cancel(thread);
		if (ret) {
			atc_sysloge ("cancel thread(ATCMountServiceListener) fail\r\n");
			ret = false;
		}
		err = pthread_join(thread, NULL);
		if (err == 0) {
			thread = 0;
			threadTerminated = false;
			ret = true;
		} else {
			atc_sysloge ("thread_join(ATCMountServiceListener) fail:%s!\r\n", strerror(err));
			ret = false;
		}
	} else {
		atc_sysloge ("thread(ATCMountServiceListener) has been terminated!\r\n");
		ret = true;
	}

	// close socket, needs to be added!
	if (mSock > 0) {
		close(mSock);
	}
	return ret;
}

int ATCMountServiceListener::init() {
	struct sockaddr_nl nladdr;
	int sz = 64 * 1024;
	int on = 1;

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = getpid();
	nladdr.nl_groups = 0xffffffff;

	if ((mSock = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_KOBJECT_UEVENT)) < 0) {
		atc_sysloge ("Unable to create uevent socket: %s!\r\n", strerror(errno));
		return -1;
	}

	if ((setsockopt(mSock, SOL_SOCKET, SO_RCVBUF, &sz, sizeof(sz)) < 0) &&
	   (setsockopt(mSock, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz)) < 0)) {
		atc_sysloge ("Unable to set uevent socket SO_RCVBUF/SO_RCVBUFFORCE option: %s!\r\n", strerror(errno));
		goto out;
	}

	if (setsockopt(mSock, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) < 0) {
		atc_sysloge ("Unable to set uevent socket SO_PASSCRED option: %s!\r\n", strerror(errno));
		goto out;
	}

	if (bind(mSock, (struct sockaddr *) &nladdr, sizeof(nladdr))) {
		atc_sysloge ("Unable to bind uevent socket: %s!\r\n", strerror(errno));
		goto out;
	}
	return 0;

out:
	close(mSock);
	return -1;
}

static int generateUevent(uevent * uev) {
	std::string dev_path;
	int major_num = 0;
	int minor_num = 0;
	Action action_rec = Action::kUnknown;
	std::string dev_name;
	std::string dev_type;
	std::string sub_system;
	std::string unsupport_reason;
	int part_num = 0;
	bool device_supported = true;
	char *pos_temp = NULL;

	/** print payload environment  **/
	for (int i = 0; uev->envp[i] != NULL; i++) {
		//atc_syslogd ("%s %s\r\n", uev->envp[i]);
		if (strstr(uev->envp[i], ACTION)) {
			if (strstr(uev->envp[i], "add")) {
				action_rec = Action::kAdd;
			} else if (strstr(uev->envp[i], "remove")) {
				action_rec = Action::kRemove;
			}
		} else if (strstr(uev->envp[i], SUBSYSTEM)) {
			pos_temp = uev->envp[i] + strlen(SUBSYSTEM);
			sub_system.assign(pos_temp);
		} else if (strstr(uev->envp[i], INSERT_INFO) && (strstr(uev->envp[i], HUB_UNSUPPORT) || strstr(uev->envp[i], DEVICE_UNSUPPORT))) {
			device_supported = false;
			if (strstr(uev->envp[i], HUB_UNSUPPORT)) {
				unsupport_reason = HUB_UNSUPPORT;
			} else {
				unsupport_reason = DEVICE_UNSUPPORT;
			}
		} else if (strstr(uev->envp[i], DEVPATH)) {
			pos_temp = uev->envp[i] + strlen(DEVPATH);
			dev_path.assign(pos_temp);
		} else if (strstr(uev->envp[i], MAJOR)) {
			pos_temp = uev->envp[i] + strlen(MAJOR);
			major_num = atoi(pos_temp);
		} else if (strstr(uev->envp[i], MONOR)) {
			pos_temp = uev->envp[i] + strlen(MONOR);
			minor_num = atoi(pos_temp);
		} else if (strstr(uev->envp[i], DEVNAME)) {
			pos_temp = uev->envp[i] + strlen(DEVNAME);
			dev_name.assign(pos_temp);
		} else if (strstr(uev->envp[i], DEVTYPE)) {
			pos_temp = uev->envp[i] + strlen(DEVTYPE);
			dev_type.assign(pos_temp);
		} else if (strstr(uev->envp[i], PARTN)) {
			pos_temp = uev->envp[i] + strlen(PARTN);
			part_num = atoi(pos_temp);
		}
	}

	ATCUeventList *ueventList = ATCUeventList::Instance();
	if (device_supported) {
		ATCUevent uevent_temp(sub_system, dev_path, major_num, minor_num, action_rec, dev_name, dev_type);
		ueventList->addUevent(uevent_temp);
	} else {
		ATCUevent uevent_temp(false, unsupport_reason);
		ueventList->addUevent(uevent_temp);
	}

	return 0;
}

void *ATCMountServiceListener::listenUevent(void *) {
	int ret = -1;

	while (1) {
		int i;
		char *pos;
		size_t bufpos;
		ssize_t buflen;
		struct uevent *uev;
		char * buffer;
		struct msghdr smsg;
		struct iovec iov;
		struct cmsghdr *cmsg;
		struct ucred *cred;
		char cred_msg[CMSG_SPACE(sizeof(struct ucred))];
		static char buf[HOTPLUG_BUFFER_SIZE + OBJECT_SIZE];

		memset(buf, 0x00, sizeof(buf));
		iov.iov_base = &buf;
		iov.iov_len = sizeof(buf);
		memset(&smsg, 0x00, sizeof(struct msghdr));
		smsg.msg_iov = &iov;
		smsg.msg_iovlen = 1;
		smsg.msg_control = cred_msg;
		smsg.msg_controllen = sizeof(cred_msg);

		buflen = recvmsg(mSock, &smsg, 0);
		if (buflen < 0) {
			if (errno != EINTR)
				atc_sysloge ("Error receiving message: %s!\r\n", strerror(errno));
			continue;
		}
		cmsg = CMSG_FIRSTHDR(&smsg);
		cred = (struct ucred *)CMSG_DATA(cmsg);
		if (cred->uid != 0) {
			atc_sysloge ("sender uid=%d, message ignored\r\n", cred->uid);
			continue;
		}

		/** skip header **/
		bufpos = strlen(buf) + 1;
		if (bufpos < sizeof("a@/d") || bufpos >= sizeof(buf)) {
			atc_sysloge ("invalid message length(%d)\r\n", bufpos);
			continue;
		}

		/** check message header **/
		if(strstr(buf, "@/") == NULL) {
			//atc_sysloge ("unrecognized message header!\r\n");
			continue;
		}

		uev = alloc_uevent();//there is no free_uevent , and is a memory leak risk
		if (!uev) {
			atc_sysloge ("alloc uevent fail!\r\n");
			continue;
		}

		if ((size_t)buflen > sizeof(buf)-1) {
			buflen = sizeof(buf) - 1;
		}

		/** Copy the shared receive buffer contents to buffer private
		*   to this uevent so we can immediately reuse the shared buffer.
		**/
		memcpy(uev->buffer, buf, HOTPLUG_BUFFER_SIZE+OBJECT_SIZE);
		buffer = uev->buffer;
		buffer[buflen] = '\0';

		/** save start of payload **/
		bufpos = strlen(buffer) + 1;

		/** action string  **/
		uev->action = buffer;
		pos = strchr(buffer, '@');
		if (!pos) {
			atc_sysloge ("bad action string '%s'!\r\n", buffer);
			continue;
		}
		pos[0] = '\0';

		/** hotplug events have the environment attached - reconstruct evnp[]  **/
		for (i = 0; (bufpos < (size_t)buflen) && (i< HOTPLUG_NUM_ENVP); i++) {
			int keylen;
			char *key;

			key = &buffer[bufpos];
			keylen = strlen(key);
			uev->envp[i] = key;
			bufpos += keylen +1;
		}
		uev->envp[i] = NULL;

		ret = generateUevent(uev);
		free_uevent(uev);
		pthread_testcancel();
	}

	pthread_exit(NULL);
	return NULL;
}
