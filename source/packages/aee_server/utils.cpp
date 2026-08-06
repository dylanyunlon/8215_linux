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

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <libgen.h>
#include <errno.h>
//#include <iostream>
//#include <cstdarg>
#include <unistd.h>
//#include <fcntl.h>
//#include <stdarg.h>
#include <syslog.h>

#include <sys/stat.h>
#include <sys/statfs.h>

#include "utils.h"
#include "aee_internal.h"

int create_dir(char *whole_path)
{
    char temp[255],dname[255];

    AEE_LOGI("create dir %s.\n",whole_path);
    strncpy(temp,whole_path,sizeof(temp)-1);
    strncpy(dname, dirname(temp),sizeof(dname)-1);

    if (access(dname,F_OK) != 0) {
        create_dir(dname);
    }
    
    if (TEMP_FAILURE_RETRY(mkdir(whole_path, 0775)) == -1) {
        AEE_LOGE("mkdir %s fail(%s).\n",whole_path, strerror(errno));
        if (errno == EEXIST)
            return 1;
        else
            return 0;
    }

    return 1;
}

bool cmd_execute(char * cmd) {
    if (cmd) {
        int status = system(cmd);
        if ((status != -1) && WIFEXITED(status) && (0 == WEXITSTATUS(status))) {
            return true;
        }
        AEE_LOGE("command '%s'execute failed: %d-%d-%d.\n", cmd, status, WIFEXITED(status), WEXITSTATUS(status));
    }
    return false;
}
