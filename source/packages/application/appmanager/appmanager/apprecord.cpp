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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <algorithm>
#include "apprecord.h"
#include "appobj.h"
#include "applog.h"

static const char TAG[] = "AppRecord";

static const int maxCount = 2;

const char savefile[] = "/data/startapp.txt";

AppRecord::AppRecord()
{
    //m_maxCount = maxCount;
/*
    m_fd = open(savefile, O_RDWR | O_CREAT | O_CLOEXEC, S_IRUSR | S_IWUSR);
    if (-1 == m_fd) {
        LOGE(TAG, "open fail: %s\n", strerror(errno));
        throw m_fd;
    }
*/
    m_fd = open(savefile, O_RDWR | O_CLOEXEC);
    if (-1 == m_fd) {
        if (errno == ENOENT) {
            LOGI(TAG, "%s not exist, creating new file\n", savefile);
            m_fd = open(savefile, O_RDWR | O_CREAT | O_CLOEXEC, S_IRUSR | S_IWUSR);
            if (-1 == m_fd) {
                LOGE(TAG, "open fail: %s\n", strerror(errno));
                throw m_fd;
            }
        } else
            LOGE(TAG, "open fail: %s\n", strerror(errno));
    }
}

AppRecord::~AppRecord()
{
    close(m_fd);
}

int AppRecord::saveRecord()
{
    int ret = -1;
    size_t nwritten;
    struct stat buf;

    if (ftruncate(m_fd, 0) == -1) {
        LOGE(TAG,"ftruncate fail:%s\n",strerror(errno));
    }
    if (lseek(m_fd, 0, SEEK_SET) == -1) {
        LOGE(TAG,"lseek fail:%s\n",strerror(errno));
    }
    for (int i = 0; i < 2; i++) {
        if (!m_appList[i].empty()) {
            nwritten = write(m_fd, &(*m_appList[i].begin()), sizeof(int));
            if (nwritten == -1) {
                if (errno == EINTR) { // If it is interrupted by the signal.
                    i--;
                    continue;
                } else {
                    LOGE(TAG,"write fail:%s\n",strerror(errno));
                    return ret;
                }
            } else {
                LOGD(TAG,"nwritten=%d\n",nwritten);
            }
        }
    }
    if (syncfs(m_fd) == -1) {
        LOGE(TAG,"syncfs fail:%s\n",strerror(errno));
    }

    ret = stat(savefile, &buf);
    if (-1 == ret) {
        LOGD(TAG, "[TMP]stat fail %s\n", strerror(errno));
    } else {
        LOGD(TAG, "[TMP]%s: %dB, mode %o\n", savefile, buf.st_size, buf.st_mode);
    }

    ret = 0;


    return ret;
}

int AppRecord::saveApp(int appID)
{
    int ret = -1;

    LOGD(TAG, "[TMP]%s: appID %d\n", __func__, appID);

    if (CAPPBaseObj::APPID_BT == appID) {
        return 0;
    }

    if (CAPPBaseObj::APPID_NAVI == appID) {
        m_appList[1].push_back(appID);
    } else {
        m_appList[0].clear();

        m_appList[0].push_back(appID);
    }

    ret = saveRecord();

    return ret;
}

int AppRecord::removeApp(int appID)
{
    int ret = -1;
    int i = 0;
    bool has = false;

    LOGD(TAG, "[TMP]%s: appID %d\n", __func__, appID);

    for (i = 0; i < 2; i++) {
        has = has || m_appList[i].has(appID);
        m_appList[i].remove(appID);
    }

    if (!has) {
        LOGW(TAG, "can't find appID %d\n", appID);
    }

    ret = saveRecord();

    return ret;
}

int AppRecord::getApp(int &appID)
{
    int ret = -1;

    ret = read(m_fd, &appID, sizeof(appID));
    if (-1 == ret) {
        LOGE(TAG, "read fail: %s\n", strerror(errno));
    } /* else if (0 == ret)
        saveRecord(); */

    return ret;
}

