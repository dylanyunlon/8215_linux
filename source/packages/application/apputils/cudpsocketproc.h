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
 
#ifndef CUDPSOCKETPROC_H
#define CUDPSOCKETPROC_H

#include "cautoudpsocket.h"

template <class T>
class CUDPSocketProc : public CAutoUDPSocket
{
public:
    CUDPSocketProc()
    {
        m_object = NULL;
        m_procFunc = NULL;
    }
    virtual ~CUDPSocketProc(){}
    bool setListener(T *object, bool (T::*procFunc)(CUDPSocket *socket))
    {
        m_object = object;
        m_procFunc = procFunc;
        return true;
    }

private:
    int onReceive() //running on thread;
    {
        if (m_object && m_procFunc) {
            (m_object->*m_procFunc)(this);
        }

        return 0;
    }

    bool (T::*m_procFunc)(CUDPSocket *socket);
    T *m_object;
};

#endif // CUDPSOCKETPROC_H
