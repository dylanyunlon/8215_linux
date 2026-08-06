/*
copyright (c) 2020 AutoChips Inc.
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

#include <unistd.h>
#include <memory>

#include "utils/macro.hpp"
#include "utils/Util.hpp"
#include "updater/Updater.hpp"
#include "bootctrl/BootCtrl.hpp"
#include "ipc/socket/SocketServer.hpp"
#include <ctype.h>
#include <stdio.h>

#include <errno.h>
#include <syslog.h>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <atomic>
#include <thread>
#include <iostream>
#include <dirent.h>
#include <cstring>
#include <vector>
#include "dbus/dbus.h"

std::atomic<bool> running(true);
using namespace atcupdateservice;
using namespace atcupdateservice::ipc::socket;

#ifdef CONFIG_NAND_BOOT
bool isIsoFile(const std :: string & filename) {
    if (filename.size() < 10) {
        return false;
    }

    bool start = (filename.substr(0,6) == "linux_");
    bool end = (filename.substr(filename.size() - 4) == ".iso");

    return start && end;
}
#else
bool isIsoFile(const std::string& filename) {
    size_t pos = filename.rfind('.');
    return (pos != std::string::npos && filename.substr(pos) == ".iso");
}
#endif

std::vector<std::string> findIsoFiles(const std::string& directoryPath) {
    std::vector<std::string> isoFiles;
    DIR* dir = opendir(directoryPath.c_str());
    if (!dir) {
        std::cerr << "Unable to open directory: " << directoryPath << std::endl;
        return isoFiles;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        std::string fileName = entry->d_name;
        if (isIsoFile(fileName)) {
            isoFiles.push_back(fileName);
        }
    }

    closedir(dir);
    return isoFiles;
}

void signal_handler(DBusConnection* conn) {
	DBusMessage* msg;
    int ret = -1;
	std::string iso_path;

	while (running) {
		dbus_connection_read_write(conn, 0);
		msg = dbus_connection_pop_message(conn);

		if (msg) {
			if (dbus_message_is_signal(msg, "org.example.MountService.Interface", "Mounted")) {
				const char* device;
				const char* mount_point;
				if (dbus_message_get_args(msg, nullptr,
										  DBUS_TYPE_STRING, &device,
										  DBUS_TYPE_STRING, &mount_point,
										  DBUS_TYPE_INVALID)) {
					std::cout << "Signal Received - Device Mounted:" << std::endl;
					std::cout << "  Device: " << device << std::endl;
					std::cout << "  Mount Point: " << mount_point << std::endl;
					std::string mount_path(mount_point);
					std::vector<std::string> isoFiles = findIsoFiles(mount_path);
					if (isoFiles.empty()) {
						std::cout << "No ISO files found in the "<<  mount_point <<", current udisk not support upgrade" << std::endl;
					} else {
						if (isoFiles.size() > 1)
							std::cout << "current path has many iso file, select first iso file to upgrade" << std::endl;

						iso_path = mount_path + "/" + isoFiles[0];
						std::cout << "ISO files found:" << iso_path <<std::endl;
						auto updater = updater::Updater::Upd::getInstance();
						ret = updater-> beginUpdate(iso_path, "LocalWorker");
						if (ret != 0) {
							std::cout << "  system updater fail " <<std::endl;
							utils::removeUpdateConfigOnFailure();
						}
					}


				}
			}
			else if (dbus_message_is_signal(msg, "org.example.MountService.Interface", "Unmounted")) {
				const char* device;
				const char* mount_point;
				if (dbus_message_get_args(msg, nullptr,
										  DBUS_TYPE_STRING, &device,
										  DBUS_TYPE_STRING, &mount_point,
										  DBUS_TYPE_INVALID)) {
					std::cout << "Signal Received - Device Unmounted:" << std::endl;
					std::cout << "  Device: " << device << std::endl;
					std::cout << "  Mount Point: " << mount_point << std::endl;
				}
			}

			dbus_message_unref(msg);
		}

		usleep(100000); // 100 ms
	}
}

int main(void) {
    //set the boot time of atcupdateservice
    atcupdateservice::utils::getElapseTimeMs();
    SocketServer::ptr server = SocketServer::Instance::getInstance();
    server->start();
    updater::Updater::ptr updater =
        updater::Updater::Upd::getInstance();
    worker::LocalWorker::ptr worker(new worker::LocalWorker("LocalWorker_Proto"));
    std::string msg;
    updater -> run();
    updater -> registerProto("LocalWorker", worker);
    if (utils::FSUtil::disableWriteProtect() == false) {
        ATC_STREAM_LOGE() << "failed to disable wp" << std::endl;
    }
    if (bootctrl::markBootSuccessful() < 0) {
        ATC_STREAM_LOGE() << "failed to mark slot successful!" << std::endl;
    }
    if (utils::FSUtil::enableWriteProtect() == false) {
        ATC_STREAM_LOGE() << "failed to enable wp" << std::endl;
    }

    ATC_STREAM_LOGI() << "updater started, start to wait check !" << std::endl;
	std::string iso_path;
	int ret = -1;
	if (utils::getUpdateConfigValue("iso_data_path", iso_path)){
		ATC_STREAM_LOGI() << "use iso_data_path from /data/update_config:" << iso_path << std::endl;
		ret = updater->beginUpdate(iso_path, "LocalWorker");
		if (ret != 0) {
			std::cout << "  system updater fail " <<std::endl;
			utils::removeUpdateConfigOnFailure();
		}
	} else  {
		DBusError err;
		DBusConnection* conn;
	
		dbus_error_init(&err);
	
		// connect Session Bus
		conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
		if (dbus_error_is_set(&err)) {
			std::cerr << "Connection Error: " << err.message << std::endl;
			dbus_error_free(&err);
			return -1;
		}
		if (!conn) {
			std::cerr << "Failed to connect to the D-Bus daemon" << std::endl;
			return -1;
		}
	
		dbus_bus_add_match(conn,
							"type='signal',interface='org.example.MountService.Interface'",
							&err);
		dbus_connection_flush(conn);
		if (dbus_error_is_set(&err)) {
			std::cerr << "Match Error: " << err.message << std::endl;
			dbus_error_free(&err);
			return -1;
		}
	
		std::thread signal_thread(signal_handler, conn);
		signal_thread.detach();
	}
    while (1) {
        sleep(1);
    }
    return 0;
}
