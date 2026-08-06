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
#include "ATCMountServiceParser.hpp"
#include "ATCSingleton.hpp"
#include <pthread.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include "dbus/dbus.h"

static std::string domain = "local";
static std::string instance = "atcmountservice";
static std::string connection = "atcmountservice_connection";

ATCMountServiceParser *ATCMountServiceParser::sInstance = NULL;
pthread_t ATCMountServiceParser::thread = 0;
bool ATCMountServiceParser::threadTerminated = true;

#define ATC_MOUNT_MAX_RETRY			20

ATCMountServiceParser::ATCMountServiceParser() {
}

ATCMountServiceParser::~ATCMountServiceParser() {
}

void ATCMountServiceParser::broadcastMounted(const std::string &mountPoint, const std::string &uuid) {
}

void ATCMountServiceParser::broadcastUnmounted(const std::string &mountPoint) {
}

void ATCMountServiceParser::broadcastPhyEject(const std::string &mountPoint, const std::string &uuid) {
}
void ATCMountServiceParser::broadcastEject(const std::string &mountPoint, const std::string &uuid) {
}

void ATCMountServiceParser::broadcastInsert(const std::string &mountPoint, const std::string &uuid) {
}

void send_mount_signal(DBusConnection* conn, const std::string& device, const std::string& mount_point, const std::string& signal) {
	DBusMessage* msg = NULL;

	if (signal == "Mounted") {
		msg = dbus_message_new_signal(
			"/org/example/MountService",
			"org.example.MountService.Interface",
			"Mounted"
		);
	} else if (signal == "Unmounted") {
		msg = dbus_message_new_signal(
			"/org/example/MountService",
			"org.example.MountService.Interface",
			"Unmounted"
		);
	}

	if (!msg) {
		atc_sysloge("Error creating signal message");
		return;
	}

	const char* device_cstr = device.c_str();
	const char* mount_point_cstr = mount_point.c_str();
	if (!dbus_message_append_args(msg,
					DBUS_TYPE_STRING, &device_cstr,
					DBUS_TYPE_STRING, &mount_point_cstr,
					DBUS_TYPE_INVALID)) {
		atc_sysloge("Error appending args to signal");
		dbus_message_unref(msg);
		return;
	}

	if (!dbus_connection_send(conn, msg, nullptr)) {
		atc_sysloge("Out of memory while sending signal");
	}

	atc_syslogd ("%s signal was successfully sent!\r\n", signal.c_str());
	dbus_connection_flush(conn);
	dbus_message_unref(msg);
}

ATCMountServiceParser *ATCMountServiceParser::Instance() {
	return Singleton<ATCMountServiceParser>::getInstance();
}

bool ATCMountServiceParser::start() {
	int err = -1;
	bool ret = true;

	if (thread) {
		atc_syslogd ("ATCMountServiceParser has been started!\r\n");
		return ret;
	}
	err = init();
	if (err == 0) {
		err = pthread_create(&thread, NULL, parseUevent, NULL);
		if (err != 0) {
			ret = false;
			atc_sysloge ("create thread(ATCMountServiceParser) fail\r\n");
		} else {
			threadTerminated = true;
			ret = true;
			atc_sysloge ("create thread(ATCMountServiceParser) succeed !\r\n");
		}
	} else {
		atc_sysloge ("thread(ATCMountServiceParser) init failed!\r\n");
		ret = false;
	}
	return ret;
}

bool ATCMountServiceParser::stop() {
	int err = -1;
	bool ret = false;
	if (thread) {
		err = pthread_cancel(thread);
		if (ret) {
			atc_sysloge ("cancel thread(ATCMountServiceParser) fail\r\n");
			ret = false;
		}
		err = pthread_join(thread, NULL);
		if (err == 0) {
			thread = 0;
			threadTerminated = false;
			ret = true;
		} else {
			atc_sysloge ("thread_join(ATCMountServiceParser) fail:%s!\r\n", strerror(err));
			ret = false;
		}
	} else {
		atc_sysloge ("thread(ATCMountServiceParser) has been terminated!\r\n");
		ret = true;
	}

	return ret;
}

int ATCMountServiceParser::init() {
	//initMountPoint(); /* init linux Udisk and SDcard mountpoint by /etc/mountpoint.xml*/
	return 0;
}

/** mount the mounted partition, specified by devicePath:
*     return:
*          0: umount succeed
*          -1: umount failed
**/
int ATCMountServiceParser::mountPartition(std::string& devicePath, std::string& mountPoint, std::string& fsType, std::string& fsLabel, std::string& uuid, std::string& partUuid) {
	bool need_fsck = false;
	uint32_t retry = 0;
	DBusError err;
	DBusConnection* conn;

	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) {
		atc_sysloge("Connection Error: %s\r\n", err.message);
		dbus_error_free(&err);
		return -1;
	}
	if (!conn) {
		atc_sysloge("Failed to connect to the D-Bus daemon\r\n");
		return -1;
	}

	while (access(devicePath.c_str(), F_OK) != 0) {
		if (++retry >= ATC_MOUNT_MAX_RETRY) {
			atc_sysloge ("unrecognized device:%s!, errno = %d, error = %s, still failure after %u retries\r\n", devicePath.c_str(), errno, strerror(errno), retry);
			goto mountfail;
		}
		/* sleep for 20 ms and retry again */
		usleep(20*1000);
	}
	if(retry > 0) {
		atc_syslogw ("find device(%s) after %u retries\r\n", devicePath.c_str(), retry);
	}
	if (pathMounted(mountPoint)) {
		atc_syslogd ("%s has been mounted, so skip mount operation!\r\n", mountPoint.c_str());
		goto mountfail;
	}

	if (fsType != "vfat") {
		atc_sysloge ("unsupported filesystem:%s!\r\n", fsType.c_str());
		goto mountfail;
	}

	if (!need_fsck) {
		atc_syslogd ("%s does not need to do fscking!\r\n", devicePath.c_str());
	} else {
		atc_syslogd ("%s start to fsck!\r\n", devicePath.c_str());
	}

	/*the following code logci can ben reconstructed*/
	if (fsType == "vfat") {
		if (!mountservice::vfat::IsSupported()) {
			atc_syslogd ("vfat is not supported on this system!\r\n");
			goto mountfail;
		}

		if (need_fsck && mountservice::vfat::Check(devicePath)) {
			atc_sysloge ("%s failed vfat filesystem check, stop mounting!\r\n", devicePath.c_str());
			//notifyEvent(ResponseCode::UNMOUNTABLE, res.c_str());

			goto mountfail;
		}

		if (mountservice::vfat::Mount(devicePath, mountPoint)) {
			atc_sysloge ("vfat: failed to mount:%s!\r\n", devicePath.c_str());
			//notifyEvent(ResponseCode::UNMOUNTABLE, res.c_str());

			goto mountfail;
		}

		send_mount_signal(conn, devicePath, mountPoint, "Mounted");
	}

	return 0;

mountfail:
	if (mountPoint.length()) {
		atc_syslogd ("%s mount succeed!\r\n", mountPoint.c_str());
		removePath(mountPoint);
	}
	return -1;
}

/** umount the mounted partition, specified by mountedPartition:
*     parameter:
*          device_node on mount_point tye filesystem_type (mount_parameters)
*     return:
*          0: umount succeed
*          -1: umount failed
**/
int ATCMountServiceParser::unmountPartition(std::string devName, std::string mountedPartition) {
	int ret = -1;
	std::string mount_point = mountedPartition;
	DBusError err;
	DBusConnection* conn;

	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) {
		atc_sysloge("Connection Error: %s\r\n", err.message);
		dbus_error_free(&err);
		return -1;
	}

	if (!conn) {
		atc_sysloge("Failed to connect to the D-Bus daemon\r\n");
		return -1;
	}

	if (mountedPartition.empty()) {
		atc_syslogd ("Mount Point is Empty, so it does not need to unmount!\r\n");
		return ret;
	}

	ret = forceUnmount(mountedPartition);

	if (ret == 0) {
		removePath(mountedPartition);
		send_mount_signal(conn, devName, mount_point, "Unmounted");

	} else {
		removePath(mountedPartition);
	}

	return ret;
}

/**  handle the insert event of external storage device (U-disk & SD card):
*      1. get the partition table of the external storage device;
*      2. get the file system & UUID information of all the partition on the external storage device from the partition table;
*      3. generate the mount point for each partition;
*      4. check & mount all the partition;
**/
void ATCMountServiceParser::handleInsert(const ATCUevent& uevent) {
	std::vector<std::string> partition_info;
	std::string device_path;
	std::string mount_point;
	std::string file_system;
	std::string fs_label;
	std::string uuid;
	std::string part_uuid;
	int part_num = 0;
	int ret = 0;
	int partition_succeed_mount = 0;

	atc_syslogi ("start to process external device insert event devname %s: , timestamp : %lld\r\n",
		    uevent.getDevName().c_str(), getCurrentTimeMs());

	partition_info.clear();
	ret = readPartitions(uevent.getDevName(), partition_info);
	if (ret != 0) {
		atc_syslogd ("get the partition info fail!\r\n");
		return;
	}

	int count = partition_info.size();
	if ( count == 0) {
		atc_syslogd ("the partition's number is 0!\r\n");
		generateMountPoint(uevent.getDevPath(), 0, mount_point);
		if (mount_point.empty()) {
			atc_sysloge ("generate the mount point for single partition(%s) fail!\r\n", uevent.getDevPath().c_str());
		} else {
			atc_syslogd ("mount of the single partition is (%s)!\r\n", mount_point.c_str());
			device_path = uevent.getDevPath();
			file_system = "vfat";
			fs_label = "";
			uuid = "";
			part_uuid = "";
			ret = mountPartition(device_path, mount_point, file_system, fs_label, uuid, part_uuid);
			if (ret) {
				atc_sysloge ("mount the single partition(%s) fail!\r\n", mount_point.c_str());
			} else {
				partition_succeed_mount++;
				atc_syslogd ("mount the single partition(%s) succeed!\r\n", mount_point.c_str());
			}
		}
		return;
	} else {
		atc_syslogd ("the partition's number is %d!\r\n", count);
	}

	for (int i = 0; i < count; i++) {
		if (readPartitionInfo(partition_info[i], device_path, file_system, fs_label, uuid, part_uuid) != 0) {
			atc_sysloge ("get the file system info fail!\r\n");
		} else {
			atc_syslogd ("device path(%s), file system(%s), file system label(%s), uuid(%s), partition uuid(%s)!\r\n",
				(device_path.empty())?"null":device_path.c_str(),
				(file_system.empty())?"null":file_system.c_str(),
				(fs_label.empty())?"null":fs_label.c_str(),
				(uuid.empty())?"null":uuid.c_str(),
				(part_uuid.empty())?"null":part_uuid.c_str());
		}
		generateMountPoint(uevent.getDevPath(), (count == 1) ? i : (i+1), mount_point);
		if (mount_point.empty()) {
			atc_sysloge ("generate the mount point for partition(%s, %d) fail!\r\n", uevent.getDevPath().c_str(), i);
			continue;
		} else {
			ret = mountPartition(device_path, mount_point, file_system, fs_label, uuid, part_uuid);
			if (ret) {
				atc_sysloge ("mount the partition(%s, %s) fail!\r\n", mount_point.c_str(), device_path.c_str());
			} else {
				partition_succeed_mount++;
				atc_syslogd ("mount the partition(%s, %s) succeed!\r\n", mount_point.c_str(), device_path.c_str());
			}
		}
	}

	if (partition_succeed_mount) {
		atc_syslogd ("%d partition mount succeed! devname %s: , timestamp : %lld\r\n", 
			partition_succeed_mount, uevent.getDevName().c_str(), getCurrentTimeMs());
	} else {
		removePathRecursion(mount_point);
		atc_syslogd (" 0 partition mount succeed, remove all the directory created!\r\n");
	}
	return;
}

void ATCMountServiceParser::handleRemove(const ATCUevent& uevent) {
	std::vector<std::string> mounted_partition_info;
	int ret = 0;
	int count = 0;
	std::string partition_unmount;

	mounted_partition_info.clear();
	ret = readMountedPartitions(uevent.getDevName(), mounted_partition_info);
	if (ret != 0) {
		atc_sysloge ("get the mounted partition info fail!\r\n");
		return;
	}

	count = mounted_partition_info.size();
	if ( count == 0) {
		atc_syslogd ("the mounted partition's number is 0!\r\n");
		return;
	} else {
		atc_syslogd ("the mounted partition's number is %d!\r\n", count);
	}

	for (int i = 0; i < count; i++) {
		if (mounted_partition_info[i].empty()) {
			atc_sysloge ("(%d) partition is Empty, so it does not need to unmount!\r\n", i);
			continue;
		}
		partition_unmount = mounted_partition_info[i];
		ret = unmountPartition(uevent.getDevName(), partition_unmount);
		if (ret) {
			atc_sysloge ("umount fail(%s)!\r\n", partition_unmount.c_str());
			continue;
		}
		atc_syslogd ("umount succeed(%s)!\r\n", partition_unmount.c_str());
	}

	if (partition_unmount.length()) {
		removePathRecursion(partition_unmount);
	}

	return;
}

void *ATCMountServiceParser::parseUevent(void *) {
	ATCUeventList *ueventList = ATCUeventList::Instance();
	while (1) {
		ATCUevent ueventGot;
		if (ueventList->empty() || !processBegan()) {
			goto nextround;
		}

		ueventList->getUevent(ueventGot);
		if(ueventGot.isNone()) {
			goto nextround;
		}

		if (ueventGot.getDeviceSupported() != false) {
			switch (ueventGot.getAction()) {
				case Action::kAdd:{
					handleInsert(ueventGot);
					break;
				}
				case Action::kRemove:{
					handleRemove(ueventGot);
					break;
				}
				default: {
					atc_syslogd ("unsupported uevent action(%d)!\r\n", (int)(ueventGot.getAction()));
					break;
				}
			}
		} else {
			atc_syslogd ("Boradcast Insert message:%s!\r\n", ueventGot.getUnSupportedReason().c_str());
		}

nextround:
		//usleep(50000);
		usleep(50000);
		pthread_testcancel();
	}
	pthread_exit(NULL);
	return NULL;
}

