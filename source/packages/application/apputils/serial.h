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

#pragma once
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include "cautothread.h"
#include "apptype.h"
#include <string.h>

#define DEFAULT_SERIAL_BUFF_LEN 1024

class CSerialObject
{
public:
    virtual ~CSerialObject() {};
    virtual bool OnDataRecv(unsigned char *pData, unsigned int dataLen) = 0;
};

typedef bool (CSerialObject::*PSERIAL_DATA_CALLBACK_FUNC)(unsigned char *pData,
                                                            unsigned int dataLen);

class CSerial : public universal_utils::CAutoThread
{
public:
    CSerial(CSerialObject *pObject,
            PSERIAL_DATA_CALLBACK_FUNC dataFunc,
            unsigned int fufLen = DEFAULT_SERIAL_BUFF_LEN);
    virtual ~CSerial(void);

    bool initSerial(CSerialObject *pObject,
                    PSERIAL_DATA_CALLBACK_FUNC pDataFunc);

    bool openSerial(const char *devPort,
                    unsigned int baudRate,
                    unsigned char byteSize = 8,
                    unsigned char parity = 'N',
                    unsigned char stopBits = 1);
    bool closeSerial();
    bool writeData(const void *pBuf, unsigned int bufLen);
    bool isOpen() const;

protected:
    unsigned long threadRun();

private:
    PSERIAL_DATA_CALLBACK_FUNC m_pDataFunc;
    unsigned int m_bufLen;

    int m_comDev_fd;
    unsigned char *m_pBuf;
    CSerialObject *m_pObject;
};
