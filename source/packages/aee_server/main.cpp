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

#include <iostream>
#include <cstdarg>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdarg.h>
#include <syslog.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/statfs.h>

#include "aee_internal.h"

int fd = -1;  // default log output.
int ts_fd = -1; // default tombstone fd.

bool enable_dal = true; // true as default

int main(int argc, char *argv[])
{
    if (argc == 2) {
        if (!strcasecmp(argv[1], "-h") ||
            !strcasecmp(argv[1], "-help")) {
            AEE_LOGE("USAGE : aee [options...]\n");
            AEE_LOGE("1. -h/help(case ignore):  Show this help.\n");
            AEE_LOGE("2. have 2 parameters, NE: dump information of process..\n");
            AEE_LOGE("   such as 'aee $(signo) $(pid)'\n");
            AEE_LOGE("3. without parameter, KE: dump information of KE.\n");
            AEE_LOGE("4. Other function to be implement...\n");
        }
    } else if (argc > 2) {
        // NE:
        if (!strncmp((const char*)argv[1], "NE", 2) && argc >= 4) {
            AEE_LOGE("NE: signal is %d(%s).\n", atoi(argv[2]), argv[2]);
            AEE_LOGE("NE: Crashed PID is %d(%s).\n", atoi(argv[3]), argv[3]);
            dump_ne(atoi(argv[2]), atoi(argv[3]));
        } else if (!strcasecmp(argv[1], "-c") && !strcasecmp(argv[2], "dal")) {
            // Clear DAL Screen
            aee_dal_clean();
        } else if (!strcasecmp(argv[1], "-s")) {
            if (!strcasecmp(argv[2], "on")) {
                // Switch on DAL
                enable_dal = true;
            } else if (!strcasecmp(argv[2], "off")) {
                // Switch off DAL
                enable_dal = false;
            }
        } else {
            // TBD: cli
        }
    } else {
        // KE
        dump_ke(NULL);
    }

    return 0;
}

