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
 
#ifndef __CMMPLASTMEMDATA__H
#define __CMMPLASTMEMDATA__H

#include "clastmem.h"
#include "applog.h"

class AtcDeviceManager;

class CMmpLastMemData : public CLastMemData
{
public:
    CMmpLastMemData(const std::string& fileFullName);
    CMmpLastMemData(const unsigned int& nvStoreId);
    ~CMmpLastMemData();
    unsigned int readData(char* buf, const unsigned int& bufSize);
    bool writeData(const char* buf, const unsigned int& bufSize, int writeWay);

private:
    std::string m_fileFullName;
    unsigned int m_nvStoreId;
    AtcDeviceManager *m_devManager;
};


#endif // CMMPLASTMENDATA

