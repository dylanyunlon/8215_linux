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

#ifndef CAUTOUDPSOCKET_H
#define CAUTOUDPSOCKET_H

#include "cudpsocket.h"
#include "threadproc.h"

class CAutoUDPSocket : public CUDPSocket
{
public:
    CAutoUDPSocket();
    virtual ~CAutoUDPSocket();
    int startService();
    int stopService();

protected:
    virtual int onReceive () = 0; //running on thread;

private:
    CAutoUDPSocket(const CAutoUDPSocket &rhs);
    const CAutoUDPSocket& operator = (const CAutoUDPSocket &rhs);
    bool recveiveProc(); //running on thread;

    universal_utils::CThreadProc<CAutoUDPSocket> *m_threadProc;
};

#endif // CAUTOUDPSOCKET_H
