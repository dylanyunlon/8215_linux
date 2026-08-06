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

#include <sys/sysinfo.h>
#include "applog.h"
#include "memorywatcher.h"
#include "globalbus.h"

static const char path[] = "/app/appmanager";
static const int porj_id = 1;

static const char TAG[] = "MemoryWatcher";

MemoryWatcherClient::MemoryWatcherClient()
    : ProcessRecordClient(path, porj_id)
{
}


MemoryWatcherService::MemoryWatcherService(const APPConfig &conf)
    : ProcessRecordService(path, porj_id)
    , m_conf(conf)
{
}
/*
bool MemoryWatcherService::checkCondition()
{
    int res = -1;
    bool ret = false;
    struct sysinfo sys;

    res = sysinfo(&sys);
    if (-1 == res) {
        LOGE(TAG, "sysinfo fail: %s\n", strerror(errno));
        res = false;
    } else {
        bool cond1 = false, cond2 = false, cond3 = false;
        unsigned long memSize = sys.freeram / 1024 * sys.mem_unit;
        unsigned long totalMem = sys.totalram / 1024 * sys.mem_unit;

        if (m_conf.m_minFreeRamKB > 300) {
            cond1 = memSize < m_conf.m_minFreeRamKB;
            if (cond1)
                LOGI(TAG, "current freeram %dK, min freeram %dK, ", memSize, m_conf.m_minFreeRamKB);
        }

        if (m_conf.m_maxAPPCount > 0) {
            cond2 = getProcessCount() > m_conf.m_maxAPPCount;
            if (cond2)
                LOGI(TAG, "current APP count %d, max APP count is %d\n", getProcessCount(), m_conf.m_maxAPPCount);
        }

        if (m_conf.m_minFreeRamPCT < 100) {
            cond3 = (memSize * 100 / totalMem) < m_conf.m_minFreeRamPCT;
            if (cond3)
                LOGI(TAG, "only %d%% freeram left, totalram is %dKB\n", (memSize * 100 / totalMem), totalMem);
        }

        ret = (cond1 || cond2 || cond3);
    }

    return ret;
}
*/

bool MemoryWatcherService::checkCondition()
{
    int res = -1;
    bool ret = false;
    char path[] = "/proc/meminfo";

    FILE *fp = fopen(path, "r");
    if (NULL == fp) {
        LOGE(TAG, "fopen %s fail: %s\n", path, strerror(errno));
    } else {
        char *buffer = NULL;
        size_t len = 0;
        char avail[] = "MemAvailable";
        unsigned long memSize = 0;
        char total[] = "MemTotal";
        unsigned long totalMem = 0;

        do {
            res = getline(&buffer, &len, fp);
            if (-1 == res) {
                LOGE(TAG, "getline fail: %s\n", strerror(errno));
                break;
            }

            if (NULL != strstr(buffer, total)) {
                sscanf(buffer, "%*s%ld", &totalMem);
                continue;
            }

            if (NULL != strstr(buffer, avail)) {
                sscanf(buffer, "%*s%ld", &memSize);
                continue;
            }
        } while (-1 != res && (0 == totalMem || 0 == memSize));

        free(buffer);
        fclose(fp);

        if (0 == totalMem) {
            LOGE(TAG, "totalMem is 0\n");
            return false;
        }

        if (0 == memSize) {
            LOGE(TAG, "memSize is 0\n");
            return false;
        }

        bool cond1 = false, cond2 = false, cond3 = false;

        if (m_conf.m_minFreeRamKB > 300) {
            cond1 = memSize < m_conf.m_minFreeRamKB;
            if (cond1)
                LOGI(TAG, "current freeram %dK, min freeram %dK, ", memSize, m_conf.m_minFreeRamKB);
        }

        if (m_conf.m_maxAPPCount > 0) {
            cond2 = getProcessCount() > m_conf.m_maxAPPCount;
            if (cond2)
                LOGI(TAG, "current APP count %d, max APP count is %d\n", getProcessCount(), m_conf.m_maxAPPCount);
        }

        if (m_conf.m_minFreeRamPCT < 100) {
            cond3 = (memSize * 100 / totalMem) < m_conf.m_minFreeRamPCT;
            if (cond3)
                LOGI(TAG, "only %d%% freeram left, totalram is %dKB\n", (memSize * 100 / totalMem), totalMem);
        }

        ret = (cond1 || cond2 || cond3);
    }

    return ret;
}

bool MemoryWatcherService::processExit(int ID)
{
    using namespace GlobalBus;
    return applyFor(ACTION_EXIT, ID, 0);
}

