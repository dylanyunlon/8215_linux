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
 
#ifndef CMANAGER_H
#define CMANAGER_H

#include "appobj.h"
#include "ccmdtask.h"

class CManager
{
public:
    CManager();
    virtual ~CManager();

    virtual bool start() = 0;
    virtual bool onDupRun(unsigned char appID, unsigned int data);
    virtual bool onRun(unsigned char appID, unsigned int data) = 0;
    virtual bool onExit(unsigned char appID, unsigned int data) = 0;
    virtual bool onRemove(unsigned char appID, unsigned int data) = 0;
    virtual bool onShowFront(unsigned char appID, unsigned int data) = 0;
    virtual bool onHideFront(unsigned char appID, unsigned int data) = 0;
    virtual bool onShowRear(unsigned char appID, unsigned int data) = 0;
    virtual bool onHideRear(unsigned char appID, unsigned int data) = 0;
    virtual bool onVideoReq(unsigned char appID, unsigned int data) = 0;
    virtual bool onVideoRel(unsigned char appID, unsigned int data) = 0;
    virtual bool onAudioReq(unsigned char appID, unsigned int data) = 0;
    virtual bool onAudioRel(unsigned char appID, unsigned int data) = 0;
    virtual bool onAVReq(unsigned char appID, unsigned int data) = 0;
    virtual bool onAVRel(unsigned char appID, unsigned int data) = 0;
    virtual bool onGoHome(unsigned char appID, unsigned int data) = 0;
    virtual bool notifyAllApp(const CCmdTask &cmdTask) = 0;
    virtual bool processAppJump(const CCmdTask &cmdTask) = 0;
    virtual bool processAutoTestCmd(const CCmdTask &cmdTask);
    virtual bool onMiscRequest(const CCmdTask &cmdTask);
    virtual bool onKeyEvent(const CCmdTask &cmdTask) = 0;
};

#endif // CMANAGER_H
