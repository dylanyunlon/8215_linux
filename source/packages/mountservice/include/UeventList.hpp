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

#ifndef UEVENTLIST_H_
#define UEVENTLIST_H_
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "Utils.hpp"
#include <list>
#include <memory>

#define NOTSUPPORTDEVICE "Device Not Support"
#define SUPPORTDEVICE "Support"

#define ACTION "ACTION="
#define DEVPATH "DEVPATH="
#define SUBSYSTEM "SUBSYSTEM="
#define MAJOR "MAJOR="
#define MONOR "MINOR="
#define DEVNAME "DEVNAME="
#define DEVTYPE "DEVTYPE="
#define PARTN "PARTN="
#define INSERT_INFO "INSERT_INFO="
#define HUB_UNSUPPORT "USB HUB NOT SUPPORT"
#define DEVICE_UNSUPPORT "USB DEVICE NOT SUPPORT"

using namespace std;

/** the type of uevent received from kernel
**/
enum class Action {
	kUnknown = 0,
	kAdd = 1,
	kRemove = 2,
	kChange = 3,
	kLinkUp = 4,
	kLinkDown = 5,
	kAddressUpdated = 6,
	kAddressRemoved = 7,
	kRdnss = 8,
	kRouteUpdated = 9,
	kRouteRemoved = 10,
};

/** Each ATCUevent presents each uevent received from kernel.
*   All the ATCUevent will be pushed into ATCUeventList, waiting to be processed by ATCMountServiceParser
**/
class ATCUevent {

public:
	explicit ATCUevent();
	explicit ATCUevent(std::string sub_system, std::string dev_path, int major_num, int minor_num, Action action_rec, std::string dev_name, std::string dev_type);
	explicit ATCUevent(bool device_supported, std::string unsupport_reason);
	explicit ATCUevent(const ATCUevent& orginalUevent);
	virtual ~ATCUevent();
	int id;
	std::string getSubSystem() const {
		return subSystem;
	}

	std::string getDevPath() const {
		return devPath;
	}

	int getMajorNum() const {
		return majorNum;
	}

	int getMinorNum() const {
		return minorNum;
	}

	Action getAction() const {
		return action;
	}

	std::string getDevName() const {
		return devName;
	}

	std::string getDevType() const {
		return devType;
	}

	bool getDeviceSupported() const {
		return deviceSupported;
	}
	std::string getUnSupportedReason() const {
		return unsupportedReason;
	}

	bool isNone() {
		return emptyUevent;
	}

	/** check whether the two uevent presents the same device.
	*   when two uevent presents the same device, the devPath, majorNum, minorNum, devName, devType will be the same
	*   all the uevent, which is not the supported, equals to none uevent
	**/
	bool compare(const ATCUevent& compared_uevent) const;
	void printUevent() const;

private:
	/** the sub system (block, scsi, scsi_generic and so on) of this uevent
	**/
	std::string subSystem;
	/** device path parsed from DEVPATH param
	**/
	std::string devPath;
	/** major parsed from MAJOR param
	**/
	int majorNum;
	/** minor parsed from MINOR param
	**/
	int minorNum;
	/** ACTION mode( common values is add, remove, change )
	**/
	Action action;
	/** device name parsed from DEVNAME( common values is sda, sdb, mmcblk1, mmcblk2 and so on )
	**/
	std::string devName;
	/** device tye parsed from DEVTYPE( common values is disk, partition )
	**/
	std::string devType;
	/** whether the device is supported by the platform
	**/
	bool deviceSupported;
	/** why the device is not supported, including:
	*     1. USB DEVICE NOT SUPPORTED;
	*     2. USB HUB NOT SUPPORTED;
	**/
	std::string unsupportedReason;

	/** whether the uevent is none\empty
	*     when the event is constructed by constructor(ATCUevent), the uevent is none\empty,
	*     until the uevent is assigned
	**/
	bool emptyUevent;
};

/**  ATCUeventList contains all the un-processed uevent received from kernel.
*    All the access to ATCUeventList must be mutually-exclusive.
*    All the pushed uevent must be added to the head of ATCUeventList
*    All the got uevent must be got & deleted from the bottom of ATCUeventList
**/
class ATCUeventList {

public:

	explicit ATCUeventList();

	virtual ~ATCUeventList();

	/** check whether the ATCUeventList is empty
	**/
	bool empty();

	/** add a uevent to the head of ueventList
	**/
	void addUevent(const ATCUevent& uevent_added);

	/** get & remove a uevent from the end of ueventList
	*   parameters:
	*      ueventGot, when get uevent succeed, the uevent got will be assigned to the input parameter ueventGo
	*   return value:
	*      a. true, get a uevent successful, the uevent got will be assigned to the input parameter ueventGot
	*      b. false, get a uevent fail, becaues the ATCUeventList is empty.
	**/
	bool getUevent(ATCUevent &ueventGot);

	/** get the instance of Uevent List
	*    the uevent list can only be got from this method
	**/
	static ATCUeventList *Instance();

private:
	static std::list<std::shared_ptr<ATCUevent>> ueventList;
	static pthread_mutex_t mutex;
	static ATCUeventList *sInstance;
	/** check whether the uevent received from kernel can be merged with an existed un-processed uevent
	*   In the following case, the uevents can will be merged:
	*      a. uevent_checked's action is remove, there exist a un-processed add uevent of the same device
	*      b. uevent_checked's action is remove, there exist a un-processed change uevent of the same device
	*
	*   Warning: this method does not lock the access to ueventList, so the caller should get the lock of mutex
	**/
	bool mergeIfNeed(ATCUevent& uevent_checked);

};
#endif
