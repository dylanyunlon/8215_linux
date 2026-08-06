/*
* Copyright (c) 2016 AutoChips Inc.
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

#ifndef __AEE_UTILS_H__
#define __AEE_UTILS_H__

//#define ATC_OS_LINUX
#define ATC_OS_HYVR

#ifndef MAX_PATH
#define MAX_PATH      256
#endif

#ifdef ATC_OS_LINUX
#define INT_SDCARD_PATH     "/media/udisk2"
#define INT_COREDUMP_PATH   "/media/udisk2"
#else // hypervisor
#ifdef CONFIG_ATC_NAND
#define INT_SDCARD_PATH     "/media/udisk2"
#define INT_COREDUMP_PATH   "/media/udisk2"
#else
#define INT_SDCARD_PATH     "/data"
#define INT_COREDUMP_PATH   "/data"
#endif
#endif

#define AEE_KE_PATH         "atclog/aee/ke"
#define AEE_NE_PATH         "atclog/aee/ne"

int create_dir(char *whole_path);
bool cmd_execute(char * cmd);


#endif // #ifndef __AEE_UTILS_H__
