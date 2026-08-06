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
 
#include "utils.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define  MONKEY_INFOFILE   "/var/run/monkey.info"

bool isMonkeySingleAppTesting(void)
{
    FILE *fp = fopen(MONKEY_INFOFILE, "r");
    if (!fp) {
        return (false);
    }
    char buf[100];

    memset(buf, 0, 100);
    fgets(buf,  100, fp);
    fclose(fp);
    if (!strncmp(buf, "SingleApp=1", 11)) {
        return (true);
    } else if (!strncmp(buf, "SingleApp=0", 11)) {
        return (false);
    } else {
    }

    return (false);
}
