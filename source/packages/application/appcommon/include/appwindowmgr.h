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
 
#include <QWindow>


class AppWindowMgr : public QObject {
public:
    static AppWindowMgr *getInstance(void) {
        return &g_windowmgr;
    }

    QWindow *pop(void);
    int push(QWindow *window);
    int bringToTop(QWindow *window);
    int remove(QWindow *window);
    QWindow *getActiveWindow(void);

    int freeWindowResource(QWindow *window);

private:
#define MAX_APPWINDOW_COUNT  10

    AppWindowMgr(void);
    virtual ~AppWindowMgr(void);

    int getWindowIndex(QWindow *window);

    QWindow *m_winstacks[MAX_APPWINDOW_COUNT];
    int      m_wincount;

    static AppWindowMgr g_windowmgr;
};
