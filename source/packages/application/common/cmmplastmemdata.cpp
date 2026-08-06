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
 
#include "cmmplastmemdata.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<QDebug>
#include "applog.h"
//#include "storage_atc.h"
#include "nvstore/nvstore.h"


static const char* const TAG = "CMmpLastMemData";
#define MMP_NV_STORE_SIZE (1024)

CMmpLastMemData::CMmpLastMemData(const std::string& fileFullName)
    : CLastMemData()
    , m_fileFullName(fileFullName)
    , m_nvStoreId(0)
    , m_devManager(NULL)
{
    m_devManager = AtcDeviceManager::getInstance();
    if (NULL == m_devManager) {
        LOGD(TAG, "new AtcDeviceManager fail!!\n");
    }
}

CMmpLastMemData::CMmpLastMemData(const unsigned int& nvStoreId)
    : CLastMemData()
    , m_fileFullName("")
    , m_nvStoreId(nvStoreId)
    , m_devManager(NULL)
{
    m_devManager = AtcDeviceManager::getInstance();
    if (NULL == m_devManager) {
        LOGD(TAG, "new AtcDeviceManager fail!!\n");
    }
}


CMmpLastMemData::~CMmpLastMemData()
{
    if (NULL != m_devManager) {
        //delete m_devManager;
        m_devManager = NULL;
    }
}

unsigned int CMmpLastMemData::readData(char* buf, const unsigned int& bufSize)
{
    unsigned int readSize = 0;

    do {
        if ((buf == NULL) || (bufSize == 0)) {
            break;
        }
#if EABLE_MMP_NV_STORE
        if (NULL == m_devManager) {
            LOGD(TAG, "m_devManager is null, not read.\n");
            break;
        }
        if(0 == m_devManager->nvStoreRead(m_nvStoreId, buf, MMP_NV_STORE_SIZE)){
            readSize = MMP_NV_STORE_SIZE;
        }
        else {
            LOGE(TAG, "m_devManager->nvStoreRead fail!!!\n");
        }
#else
        int fd = 0;
        fd= open(m_fileFullName.c_str(), O_RDONLY);
        if (fd == -1) {
            break;
        }

        readSize = read(fd, buf, bufSize);
        close(fd);
#endif
    } while (0);

    return readSize;
}

bool CMmpLastMemData::writeData(const char* buf, const unsigned int& bufSize, int writeWay)
{
    bool ret = false;

    do {
        //LOGI(TAG, "CMmpLastMemData writeData enter, bufSize:[%d]\n", bufSize);
        if ((buf == NULL) || (bufSize == 0)) {
            break;
        }
#if EABLE_MMP_NV_STORE
        if (NULL == m_devManager) {
            LOGE(TAG, "m_devManager is null, not write.\n");
            break;
        }
        if (bufSize > MMP_NV_STORE_SIZE) {
            LOGE(TAG, "size is :[%d], more than %d. could not store..\n", bufSize, MMP_NV_STORE_SIZE);
            break;
        }
        void *data = (void*)buf;
        if( writeWay == WRITE_CACHE) {
            if(0 != m_devManager->nvStoreWrite(m_nvStoreId, data, bufSize, NVSTORE_CACHE)) {
                LOGE(TAG, "Write nvStore NVSTORE_CACHE fail..\n");
                break;
            }
            m_devManager->nvStoreFlush(m_nvStoreId);
        }
        else {
            if(0 != m_devManager->nvStoreWrite(m_nvStoreId, data, bufSize, NVSTORE_WRITETHROUGH)) {
                LOGE(TAG, "Write nvStore NVSTORE_WRITETHROUGH fail..\n");
                break;
            }
        }

#else
        ssize_t writeSize = 0;
        int fd = 0;
        fd= open(m_fileFullName.c_str(), O_RDWR | O_CREAT |O_TRUNC);
        if (fd == -1) {
            LOGE(TAG, "CMmpLastMemData writeData error, open error.\n");
            break;
        }

        writeSize = write(fd, buf, bufSize);
        if (writeSize != (ssize_t)bufSize) {
            close(fd);
            remove(m_fileFullName.c_str());
            LOGE(TAG, "CMmpLastMemData writeData error, writeSize:[%d].\n", bufSize);
            break;
        }
        close(fd);
#endif
        ret = true;
        break;
    } while (0);

    if(!ret) {
        LOGD(TAG, "CMmpLastMemData writeData leave,bufSize:[%d] ret: [%d]\n", bufSize, ret);
    }

    //LOGI(TAG, "CMmpLastMemData writeData leave, ret: [%d]\n", ret);

    return ret;
}
