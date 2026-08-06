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

#ifndef CAPPMANAGER_H
#define CAPPMANAGER_H

#include "ctaskmanager.h"
#include "appobj.h"
#include "cmanager.h"
#include "singleton.h"
#include "cobjfactory.h"

class CAppManager : public CTaskManager, public universal_utils::Singleton<CAppManager>
{
public:
    CAppManager(CObjFactory *pFactory);
    ~CAppManager();

    bool init();
    bool startTaskWatcher ();
    bool restartTaskWatcher();

    static const char *EGLFS;
    static const char *WAYLAND;

private:
    CAppManager(const CAppManager &rhs);
    const CAppManager& operator = (const CAppManager &rhs);

    bool doTask (const CCmdTask &cmdTask);
    bool appActionProc (const CCmdTask &cmdTask);

    CManager *m_manager;
    CObjFactory *m_factory;
//    MCUProxy *m_MCUProxy;
};

#endif // CAPPMANAGER_H
