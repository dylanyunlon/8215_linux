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
#include "UeventList.hpp"
#include "ATCSingleton.hpp"

ATCUeventList *ATCUeventList::sInstance = NULL;
//std::list<ATCUevent> ATCUeventList::ueventList = {};
std::list<std::shared_ptr<ATCUevent>> ATCUeventList::ueventList = {};
pthread_mutex_t ATCUeventList::mutex = PTHREAD_MUTEX_INITIALIZER;

ATCUevent::ATCUevent(){
		emptyUevent = true;
}

ATCUevent::ATCUevent(std::string sub_system, std::string dev_path, int major_num, int minor_num, Action action_rec, std::string dev_name, std::string dev_type):
	subSystem(sub_system), devPath(dev_path), majorNum(major_num), minorNum(minor_num), action(action_rec), devName(dev_name), devType(dev_type){
	deviceSupported = true;
	//atc_syslogd ("%s receive uevent of (devPath:%s, action:%d)!\r\n", devPath.c_str(), action);
}

ATCUevent::ATCUevent(const ATCUevent& orginalUevent){
	this->devPath = orginalUevent.getDevPath();
	this->subSystem = orginalUevent.getSubSystem();
	this->majorNum = orginalUevent.getMajorNum();
	this->minorNum = orginalUevent.getMinorNum();
	this->action = orginalUevent.getAction();
	this->devName = orginalUevent.getDevName();
	this->devType = orginalUevent.getDevType();
	this->deviceSupported = orginalUevent.getDeviceSupported();
	this->unsupportedReason = orginalUevent.getUnSupportedReason();
	emptyUevent = false;
}

void ATCUevent::printUevent() const {
	if (Action::kAdd == getAction()) {
		atc_syslogd ("ACTION=add\r\n");
	} else if (Action::kRemove == getAction()) {
		atc_syslogd ("ACTION=remove\r\n");
	} else {
		atc_syslogd ("ACTION=%d\r\n", getAction());
	}

	if (getDevPath().empty())
		atc_syslogd ("DEVPATH=NULL\r\n");
	else
		atc_syslogd ("DEVPATH=%s\r\n", getDevPath().c_str());

	if (getSubSystem().empty())
		atc_syslogd ("SUBSYSTEM=NULL\r\n");
	else
		atc_syslogd ("SUBSYSTEM=%s\r\n", getSubSystem().c_str());

	atc_syslogd ("MAJOR=%d\r\n", getMajorNum());
	atc_syslogd ("MINOR=%d\r\n", getMinorNum());

	if (getDevName().empty())
		atc_syslogd ("DEVNAME=NULL\r\n");
	else
		atc_syslogd ("DEVNAME=%s\r\n", getDevName().c_str());

	if (getDevType().empty())
		atc_syslogd ("DEVTYPE=NULL\r\n");
	else
		atc_syslogd ("DEVTYPE=%s\r\n", getDevType().c_str());

}

ATCUevent::ATCUevent(bool device_supported, std::string unsupport_reason):
	deviceSupported(device_supported), unsupportedReason(unsupport_reason){
	emptyUevent = false;
}

ATCUevent::~ATCUevent(){
	emptyUevent = true;
}

bool ATCUevent::compare(const ATCUevent& compared_uevent) const {
	if (deviceSupported == false) {
		return false;
	}
	if((compared_uevent.getDevPath().compare(devPath) == 0) &&
		(compared_uevent.getMajorNum() == majorNum) &&
		(compared_uevent.getMinorNum() == minorNum) &&
		(compared_uevent.getDevName().compare(devName) == 0) &&
		(compared_uevent.getDevType().compare(devType) == 0) &&
		(compared_uevent.getSubSystem().compare(subSystem) == 0)) {
		return true;
	} else {
		return false;
	}
}

ATCUeventList::ATCUeventList() {
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
	ueventList.clear();
	pthread_mutex_unlock(&mutex);
}

ATCUeventList::~ATCUeventList() {
	pthread_mutex_destroy(&mutex);
	ueventList.clear();
}

ATCUeventList *ATCUeventList::Instance() {
	return Singleton<ATCUeventList>::getInstance();
}

/** check whether the uevent received from kernel can be merged with an existed un-processed uevent
*   In the following case, the uevents can will be merged:
*      a. uevent_checked's action is remove, there exist a un-processed add uevent of the same device
*      b. uevent_checked's action is remove, there exist a un-processed change uevent of the same device
*
*   Warning: this method does not lock the access to ueventList, so the caller should get the lock of mutex
**/
bool ATCUeventList::mergeIfNeed(ATCUevent& uevent_checked) {
	bool isMerged = false;
	if((!uevent_checked.getDeviceSupported() && !uevent_checked.getUnSupportedReason().empty()) ||
		Action::kRemove != uevent_checked.getAction()) {
		atc_syslogd("uevent (devPath:%s, action:%d) does not need to be merged with any uevent in the uevent list!\r\n",
				uevent_checked.getDevPath().c_str(), uevent_checked.getAction());
		return isMerged;
	}

	std::list<std::shared_ptr<ATCUevent>>::iterator it;
	for (it = ueventList.begin(); it != ueventList.end();) {
		if ((*it)->compare(uevent_checked) && (*it)->getAction() == Action::kAdd) {
			ueventList.erase(it);
			isMerged = true;
			 atc_syslogd ("uevent (devPath:%s, action:%d) merged with(devPath:%s, action:%d)!\r\n",
					uevent_checked.getDevPath().c_str(), uevent_checked.getAction(),(*it)->getDevPath().c_str(), (*it)->getAction());
			break;
		} else {
			it++;
		}
	}

	return isMerged;
}

/** check whether the event is repeated with a previous received uevent:
**/
static bool repeatedUevent(const ATCUevent& uevent_added) {
	bool isReteated = false;
	return isReteated;
}

/** check whether the ATCUeventList is empty
**/
bool ATCUeventList::empty() {
	bool ret = true;
	pthread_mutex_lock(&mutex);
	ret = ueventList.empty();
	pthread_mutex_unlock(&mutex);
	return ret;
}

/** check whether the event is valid:
*        1. only when the uevent is valid,  the uevent can be inserted to the uevent list;
*        2. all the not-supported uevent is valid;
*        3. only the uevent of add and remove is valid;
*        4. only the uevent with "block" subsystem is valid;
*        5. only the uevent with "disk" subsystem is valid;
**/
static bool validUevent(const ATCUevent& uevent_added) {

	if (!(uevent_added.getDeviceSupported()) && !(uevent_added.getUnSupportedReason().empty())) {
		return true;
	} else if ((uevent_added.getAction() != Action::kAdd) &&
				(uevent_added.getAction() != Action::kRemove)) {
		atc_syslogd ("invalid action(%d), do not insert the uevent\r\n", (int)(uevent_added.getAction()));
		return false;
	} else if (uevent_added.getSubSystem().compare("block") != 0) {
		atc_syslogd (" invalid subsystem(%s), do not insert the uevent\r\n", uevent_added.getSubSystem().c_str());
		return false;
	} else if (uevent_added.getDevType().compare("disk")!= 0) {
		atc_syslogd (" invalid device type(%s), do not insert the uevent\r\n",  uevent_added.getDevType().c_str());
		return false;
	} else if ((uevent_added.getDevName().find("mmcblk0", 0) == 0)) { //consider the non nand case
#ifdef CONFIG_NAND_BOOT
		atc_syslogd ("vaild emmc(%s) uevnet for nand!\r\n", uevent_added.getDevName().c_str());
		return true;
#else
		atc_syslogd ("skip emmc(%s) uevnet!\r\n", uevent_added.getDevName().c_str());
		return false;
#endif
	} else if ((uevent_added.getDevName().find("mmcblk", 0) == 0) || (uevent_added.getDevName().find("sd", 0) == 0)) {
		atc_syslogd("valid device uevent(%s)\r\n", uevent_added.getDevName().c_str());
		return true;
	} else {
		atc_syslogd (" invalid device type(%s) or device name(%s), do not insert the uevent\r\n", uevent_added.getDevType().c_str(), uevent_added.getDevName().c_str());
		return false;
	}
}

/** add a uevent to the head of ueventList
**/
void ATCUeventList::addUevent(const ATCUevent& uevent_added) {
	bool isMerged = false;

	if (!validUevent(uevent_added)) {
		return;
	}

	//ATCUevent uevent_temp(uevent_added);
	std::shared_ptr<ATCUevent> uevent_temp = std::shared_ptr<ATCUevent>(new ATCUevent(uevent_added));

	pthread_mutex_lock(&mutex);
	isMerged = mergeIfNeed(*uevent_temp);
	//isMerged = mergeIfNeed(uevent_temp);
	if (!isMerged) {
		ueventList.push_front(uevent_temp);
	}
	pthread_mutex_unlock(&mutex);
	uevent_added.printUevent();
	if (!isMerged)
		atc_syslogd ("uevent (devPath:%s, action:%d) has been pushed into the uevent list!\r\n", uevent_added.getDevPath().c_str(), uevent_added.getAction());
	else
		atc_syslogd ("uevent (devPath:%s, action:%d) has been merged with an add uevent of the same device in the uevent list!\r\n", uevent_added.getDevPath().c_str(), uevent_added.getAction());

	return;
}

/** get & remove a uevent from the end of ueventList
*   parameters:
*      ueventGot, when get uevent succeed, the uevent got will be assigned to the input parameter ueventGo
*   return value:
*      a. true, get a uevent successful, the uevent got will be assigned to the input parameter ueventGot
*      b. false, get a uevent fail, becaues the ATCUeventList is empty.
**/
bool ATCUeventList::getUevent(ATCUevent &ueventGot) {
	bool ret = false;
	pthread_mutex_lock(&mutex);
	if(!(ueventList.empty())) {
		std::shared_ptr<ATCUevent> uevent_got = NULL;
		do {
			uevent_got = ueventList.back();
			ueventList.pop_back();
		} while (uevent_got == NULL);
		ueventGot = *uevent_got;
		ret = true;
	}
	pthread_mutex_unlock(&mutex);
	if (ret)
		atc_syslogd (" uevent (devPath:%s, action:%d) has been pulled from the uevent list!\r\n", ueventGot.getDevPath().c_str(), ueventGot.getAction());
	else
		atc_sysloge ("the uevent list is empty, so get uevent failed!\r\n");
	return ret;
}
