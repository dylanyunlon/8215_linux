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

#include "appwindowmgr.h"

#include <QDebug>

AppWindowMgr AppWindowMgr::g_windowmgr;


AppWindowMgr::AppWindowMgr(void) {
    //qDebug("[appwindowmgr] -> AppWindowMgr::AppWindowMgr enter\n");
    m_wincount = 0;
}


AppWindowMgr::~AppWindowMgr(void) {
    //qDebug("[appwindowmgr] -> AppWindowMgr::~AppWindowMgr enter\n");
}


int AppWindowMgr::getWindowIndex(QWindow *window) {
    int window_idx = -1;
    for (int i = 0; i < m_wincount; i++) {
        if (m_winstacks[i] == window) {
            window_idx = i;
            //qDebug("[AppWindowMgr] window %p is found and index is %d\n", window, window_idx);
            break;
        }
    }
    return (window_idx);
}


QWindow *AppWindowMgr::pop(void) {
    //qDebug("[AppWindowMgr] AppWindowMgr::pop enter 111111111111\n");
    return (NULL);
}


int AppWindowMgr::push(QWindow *window) {
    qDebug("[AppWindowMgr] AppWindowMgr::push enter 111111111111 window = %p\n", window);
    if (!window) {
        //qDebug("[AppWindowMgr] AppWindowMgr::push leave 111111111111 window = %p\n", window);
        return (-1);
    }
    int window_idx = getWindowIndex(window);
    if (window_idx >= 0) {
    } else {
        m_winstacks[m_wincount] = window;
        m_wincount++;
    }
    return (-1);
}


int AppWindowMgr::bringToTop(QWindow *window) {
    //qDebug("[AppWindowMgr] AppWindowMgr::bringToTop enter 111111111111 window = %p\n", window);
    return (-1);
}


int AppWindowMgr::remove(QWindow *window) {
    //qDebug("[AppWindowMgr] AppWindowMgr::remove enter 111111111111 window = %p\n", window);
    if (!window) {
        //qDebug("[AppWindowMgr] AppWindowMgr::remove leave 111111111111 window = %p\n", window);
        return (-1);
    }
    int window_idx = getWindowIndex(window);
    if (window_idx < 0) {
        //qDebug("[AppWindowMgr] AppWindowMgr::remove leave 222222222222 window = %p\n", window);
        return (-1);
    }
    for (int i = window_idx; i < m_wincount - 1; i++) {
        m_winstacks[i] = m_winstacks[i+1];
    }
    m_wincount--;
    if (m_winstacks[m_wincount-1]) {
        //qDebug("[AppWindowMgr] -> window = %p, restore window = %p\n", window, m_winstacks[m_wincount-1]);
        window->hide();
        window->destroy();
        m_winstacks[m_wincount-1]->create();
        m_winstacks[m_wincount-1]->show();
    }

    return (0);
}


QWindow *AppWindowMgr::getActiveWindow(void) {
    //qDebug("[AppWindowMgr] AppWindowMgr::getActiveWindow enter 111111111111\n");
    if (m_wincount <= 0) {
        //qDebug("[AppWindowMgr] AppWindowMgr::getActiveWindow leave 111111111111\n");
        return (NULL);
    }
    else {
        qDebug("[AppWindowMgr] AppWindowMgr::getActiveWindow leave 2222222222222\n");
        return (m_winstacks[m_wincount - 1]);
    }
}


int AppWindowMgr::freeWindowResource(QWindow *window) {
    //qDebug("[AppWindowMgr] AppWindowMgr::freeWindowResource enter 111111111111\n");
    if (!window) {
       return (-1);
    }
    window->hide();
    window->destroy();
    
    return (0);
}
