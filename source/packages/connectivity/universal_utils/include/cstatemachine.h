/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#ifndef CSTATEMACHINE_H
#define CSTATEMACHINE_H

#include <stddef.h>
#include <list>
#include <string>
#include <map>
#include <vector>

#include "cconditionlock.h"
#include "cmessagehandler.h"
#include "cstate.h"
#include "cmessage.h"

namespace universal_utils {

class SMHander;

class CStateInfo {
public:
    /** The state */
    CState *state;

    /** The parent of this state, null if there is no parent */
    CStateInfo *parentStateInfo;

    /** True when the state has been entered and on the stack */
    bool active;

    /**
     * Convert StateInfo to string
     */
};

class CStateMachine
{
public:
    CStateMachine(const std::string name);
    virtual ~CStateMachine();

    static const int SM_QUIT_COMMAND = -1;
    static const int SM_INIT_COMMAND = -2;
    static const int HANDLED = true;
    static const int NOT_HANDLED = false;

    int initStateMachine(const std::string name);
    int deinitStateMachine();
    void start();
    void addState(CState *state, CState *parent);
    CMessage getCurrentMessage();
    CState *getCurrentState();
    void addState(CState *state);
    void setInitialState(CState *initialState);
    void transitionTo(CState *destState);
    void transitionToHaltingState();
    void deferMessage(const CMessage &msg);
    void unhandledMessage(const CMessage &msg);
    void haltedProcessMessage(const CMessage &msg);
    virtual void onHalting();
    virtual void onQuitting();
    std::string getName() const;
    CMessageHandler *getHandler();
    void sendMessage(int what);
    void sendMessage(const CMessage &msg);
    void sendMessageDelayed(int what, long delayMillis);
    void sendMessageDelayed(int what, const CRawString &obj, long delayMillis);
    void sendMessageDelayed(const CMessage &msg, long delayMillis);
    void sendMessageDelayed(const CMessage &msg, const CRawString &obj, long delayMillis);
    void sendMessageAtFrontOfQueue(int what, const CRawString &obj);
    void sendMessageAtFrontOfQueue(int what);
    void sendMessageAtFrontOfQueue(const CMessage &msg);
    void removeMessages(int what);
    void removeDeferredMessages(int what);
    void flushMessages();
    void quit();
    void quitNow();
    void waitForQuit();
    bool hasMessages(int what);
    bool hasDeferredMessages(int what);
    CConditionLock mWaitQuit;
    int mWaitQuitId;

private:
    SMHander *m_smHandler;
    std::string m_name;

};

class SMHander : public CMessageHandler
{
public:
    SMHander(CStateMachine* sm);
    ~SMHander();

    void setInitialState(CState *initialState);
    void transitionTo(CState *destState);
    void deferMessage(const CMessage &msg);
    void removeDeferMessage(int what);
    void flushDeferMessage();
    void completeConstruction();
    void setupInitialStateStack();
    int handleMessage(const CMessage &message);
    bool isQuit(const CMessage &msg);
    CStateInfo *addState(CState *state, CState *parent);
    CMessage getCurrentMessage();
    CState *getCurrentState();
    void quit();
    void quitNow();
    bool hasDeferMessage(int what);
    bool m_HasQuit;

    class HaltingState : public CState
    {
    public:
        HaltingState(SMHander *handler);
        bool processMessage(const CMessage &message);
    private:
        SMHander *m_SMHander;
    };


    class QuittingState : public CState
    {
    public:
        QuittingState(SMHander *handler);
        bool processMessage(const CMessage &message);
    private:
        SMHander *m_SMHander;
    };

    HaltingState *m_HaltingState;
    QuittingState *m_QuittingState;
protected:

private:
    CStateMachine *m_StateMachine;

    bool m_IsConstructionCompleted;
    CState *m_initState;
    CState *m_desState;
    int m_TempStateStackCount;
    int m_StateStackTopIndex;

    std::vector<CMessage> m_deferredMessages;
    std::map<CState*, CStateInfo*> m_StateInfo;

    CMessage m_message;
    CStateInfo **mTempStateStack;
    CStateInfo **mStateStack;

    int processMessage(const CMessage &message);
    void processMsg(const CMessage &msg);
    void performTransitions();
    void cleanupAfterQuitting();
    void clear();
    CStateInfo *setupTempStateStackWithStatesToEnter(CState *destState);
    void invokeExitMethods(CStateInfo *commonStateInfo);
    void invokeEnterMethods(int stateStackEnteringIndex);
    int moveTempStateStackToStateStack();
    void moveDeferredMessageAtFrontOfQueue();
};

}

#endif // CSTATEMACHINE_H
