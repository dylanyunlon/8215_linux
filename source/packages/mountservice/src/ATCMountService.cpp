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
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ATCMountServiceListener.hpp"
#include "ATCMountServiceParser.hpp"
#include "ATCMountServiceImpl.hpp"

#include <pthread.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include "Utils.hpp"

#define SYSTEM "/dev/system"

int main(int argc, char** argv) {
	int fd;
	initSwitchController();
#if 0
	if (hasLastAPP() || homeReady()) {
		beginProcess();
		atc_syslogd ("has last app or home app has alreay done, so begin to process external storage devices!\r\n");
	} else {
		atc_syslogd ("does not have last app, so do not begin to process external storage devices!\r\n");
	}
#endif
	beginProcess();

	ATCMountServiceListener *mountServiceListener = ATCMountServiceListener::Instance();
	if (mountServiceListener == NULL) {
		atc_sysloge("get ATCMountServiceListener instance fail!\r\n");
	}
	mountServiceListener->start();

	ATCMountServiceParser *mountServiceParser = ATCMountServiceParser::Instance();
	if (mountServiceParser == NULL) {
		atc_sysloge("get ATCMountServiceParser instance fail!\r\n");
	}
	mountServiceParser->start();
	//mountServiceParser->setWatcher(myService);

	// Do coldboot here so it won't block booting,
	// also the cold boot is needed in case we have flash drive
	// connected before Vold launched
	coldboot("/sys/block");

	while (true) {
		//atc_sysloge("%s Waiting for calls... (Abort with CTRL+C)!\r\n");
		sleep(1);
	}

	destroySwitchController();
	return 0;
}
