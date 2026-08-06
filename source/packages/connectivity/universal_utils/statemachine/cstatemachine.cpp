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

#include "cstatemachine.h"

namespace universal_utils {

static const bool DBG = false;

CStateMachine::CStateMachine(const std::string name)
    : m_smHandler(NULL)
{
    mWaitQuit.lock();
    mWaitQuitId = mWaitQuit.newCondition();
    mWaitQuit.unlock();
    initStateMachine(name);
}

CStateMachine::~CStateMachine()
{
    if (m_smHandler) {
        m_smHandler->stopProcess();

        delete m_smHandler;
        m_smHandler = NULL;
    }
    if (-1 != mWaitQuitId) {
        mWaitQuit.lock();
        mWaitQuit.releaseCondition(mWaitQuitId);
        mWaitQuitId = -1;
        mWaitQuit.unlock();
    }
}

int CStateMachine::initStateMachine(const std::string name)
{
    if (DBG) UTILS_LOGD("CStateMachine", "initStateMachine");

    //m_smHandler can be !NULL, for the first time to start, and no stop before
    //this case no need to do the init again.
    //witch was done by CStateMachine::CStateMachine() when first create
    if (NULL != m_smHandler) {
        UTILS_LOGW("CStateMachine", "m_smHandler !NULL, return");
        return 0;
    }

    m_name = name;
    m_smHandler = new SMHander(this);
    m_smHandler->startProcess();

    return 0;
}

int CStateMachine::deinitStateMachine()
{
    if (DBG) UTILS_LOGD("CStateMachine", "DeinitStateMachine");

    if (m_smHandler) {
        m_smHandler->stopProcess();

        delete m_smHandler;
        m_smHandler = NULL;
    }

    return 0;
}

void CStateMachine::start()
{
    if (DBG) UTILS_LOGD("CStateMachine", "start");

    // m_smHandler can be NULL if the state machine has quit.
    if (m_smHandler == NULL) return;

    /** Send the complete construction message */
    m_smHandler->completeConstruction();
}


//protected

/**
 * Add a new state to the state machine
 * @param state the state to add
 * @param parent the parent of state
 */
void CStateMachine::addState(CState *state, CState *parent)
{
    m_smHandler->addState(state, parent);
}

/**
 * @return current message
 */
CMessage CStateMachine::getCurrentMessage()
{
    return m_smHandler->getCurrentMessage();
}

/**
 * @return current state
 */
CState *CStateMachine::getCurrentState()
{
    return m_smHandler->getCurrentState();
}

/**
 * Add a new state to the state machine, parent will be NULL
 * @param state to add
 */
void CStateMachine::addState(CState *state)
{
    m_smHandler->addState(state, NULL);
}

/**
 * Set the initial state. This must be invoked before
 * and messages are sent to the state machine.
 *
 * @param initialState is the state which will receive the first message.
 */
void CStateMachine::setInitialState(CState *initialState)
{
    m_smHandler->setInitialState(initialState);
}

/**
 * transition to destination state. Upon returning
 * from processMessage the current state's exit will
 * be executed and upon the next message arriving
 * destState.enter will be invoked.
 *
 * this function can also be called inside the enter function of the
 * previous transition target, but the behavior is undefined when it is
 * called mid-way through a previous transition (for example, calling this
 * in the enter() routine of a intermediate node when the current transition
 * target is one of the nodes descendants).
 *
 * @param destState will be the state that receives the next message.
 */
void CStateMachine::transitionTo(CState *destState)
{
    m_smHandler->transitionTo(destState);
}

/**
 * transition to halt state. Upon returning
 * from processMessage we will exit all current
 * states, execute the onHalting() method and then
 * for all subsequent messages haltedProcessMessage
 * will be called.
 */
void CStateMachine::transitionToHaltingState()
{
    m_smHandler->transitionTo(m_smHandler->m_HaltingState);
}

/**
 * Defer this message until next state transition.
 * Upon transitioning all deferred messages will be
 * placed on the queue and reprocessed in the original
 * order. (i.e. The next state the oldest messages will
 * be processed first)
 *
 * @param msg is deferred until the next transition.
 */
void CStateMachine::deferMessage(const CMessage &msg)
{
    m_smHandler->deferMessage(msg);
}

/**
 * Called when message wasn't handled
 *
 * @param msg that couldn't be handled.
 */
void CStateMachine::unhandledMessage(const CMessage &msg)
{
}

/**
 * Called for any message that is received after
 * transitionToHalting is called.
 */
void CStateMachine::haltedProcessMessage(const CMessage &msg)
{
}

/**
 * This will be called once after handling a message that called
 * transitionToHalting. All subsequent messages will invoke
 * {@link StateMachine#haltedProcessMessage(CMessage)}
 */
void CStateMachine::onHalting()
{
}

/**
 * This will be called once after a quit message that was NOT handled by
 * the derived StateMachine. The StateMachine will stop and any subsequent messages will be
 * ignored. In addition, if this StateMachine created the thread, the thread will
 * be stopped after this method returns.
 */
void CStateMachine::onQuitting()
{
    if (DBG) UTILS_LOGD("CStateMachine", "onQuitting");
}

/**
 * @return the name
 */
std::string CStateMachine::getName() const
{
    return m_name;
}


/**
 * @return Handler
 */
CMessageHandler *CStateMachine::getHandler()
{
    return m_smHandler;
}


/**
 * Enqueue a message to this state machine.
 */
void CStateMachine::sendMessage(int what)
{
    // m_smHandler can be NULL if the state machine has quit.
    if (m_smHandler == NULL) return;

    CMessage message;
    message.what = what;

    m_smHandler->sendMessage(message);
}


/**
 * Enqueue a message to this state machine.
 */
void CStateMachine::sendMessage(const CMessage &msg)
{
    // m_smHandler can be NULL if the state machine has quit.
    if (m_smHandler == NULL) return;

    m_smHandler->sendMessage(msg);
}

/**
 * Enqueue a message to this state machine after a delay.
 */
void CStateMachine::sendMessageDelayed(int what, long delayMillis)
{
    // m_smHandler can be NULL if the state machine has quit.
    if (m_smHandler == NULL) return;

    CMessage message;
    message.what = what;

    m_smHandler->sendMessageDelayed(message, delayMillis);
}

/**
 * Enqueue a message to this state machine after a delay.
 */
void CStateMachine::sendMessageDelayed(int what, const CRawString &obj, long delayMillis)
{
    // m_smHandler can be NULL if the state machine has quit.
    if (m_smHandler == NULL) return;

    CMessage message;
    message.what = what;
    message.setArgRaw(obj);

    m_smHandler->sendMessageDelayed(message, delayMillis);
}

/**
 * Enqueue a message to this state machine after a delay.
 */
void CStateMachine::sendMessageDelayed(const CMessage &msg, long delayMillis)
{
    // m_smHandler can be NULL if the state machine has quit.
    if (m_smHandler == NULL) return;

    m_smHandler->sendMessageDelayed(msg, delayMillis);
}

void CStateMachine::sendMessageDelayed(const CMessage &msg, const CRawString &obj, long delayMillis)
{
    if (m_smHandler == NULL) return;

    CMessage delayMsg = msg;
    delayMsg.setArgRaw(obj);

    m_smHandler->sendMessageDelayed(delayMsg, delayMillis);

}

/**
 * Enqueue a message to the front of the queue for this state machine.
 * Protected, may only be called by instances of StateMachine.
 */
void CStateMachine::sendMessageAtFrontOfQueue(int what, const CRawString &obj)
{
    CMessage message;
    message.what = what;
    message.setArgRaw(obj);

    m_smHandler->sendMessageAtFrontOfQueue(message);
}

/**
 * Enqueue a message to the front of the queue for this state machine.
 * Protected, may only be called by instances of StateMachine.
 */
void CStateMachine::sendMessageAtFrontOfQueue(int what)
{
    CMessage message;
    message.what = what;

    m_smHandler->sendMessageAtFrontOfQueue(message);
}

/**
 * Enqueue a message to the front of the queue for this state machine.
 * Protected, may only be called by instances of StateMachine.
 */
void CStateMachine::sendMessageAtFrontOfQueue(const CMessage &msg)
{
    m_smHandler->sendMessageAtFrontOfQueue(msg);
}

/**
 * Removes a message from the message queue.
 * Protected, may only be called by instances of StateMachine.
 */
void CStateMachine::removeMessages(int what)
{
    m_smHandler->removeDeferMessage(what);
    m_smHandler->removeMessages(what);
}

/**
 * Removes a message from the deferred message queue.
 * Protected, may only be called by instances of StateMachine.
 */
void CStateMachine::removeDeferredMessages(int what)
{
    m_smHandler->removeDeferMessage(what);
}

void CStateMachine::flushMessages()
{
    m_smHandler->flushMessage();
    m_smHandler->flushDeferMessage();
}

/**
 * Quit the state machine after all currently queued up messages are processed.
 */
void CStateMachine::quit()
{
    // m_smHandler can be NULL if the state machine is already stopped.
    if (m_smHandler == NULL) return;

    m_smHandler->quit();
}

/**
 * Quit the state machine immediately all currently queued messages will be discarded.
 */
void CStateMachine::quitNow()
{
    if (DBG) UTILS_LOGD("CStateMachine", "quitNow");

    // m_smHandler can be NULL if the state machine is already stopped.
    if (m_smHandler == NULL) return;

    m_smHandler->quitNow();
}

void CStateMachine::waitForQuit()
{
    if (DBG) UTILS_LOGD("CStateMachine", "waitForQuit");

    if (-1 != mWaitQuitId) {
        mWaitQuit.lock();
        while (!m_smHandler->m_HasQuit){
            mWaitQuit.await(mWaitQuitId);
        }
        mWaitQuit.unlock();
    }
}

bool CStateMachine::hasMessages(int what)
{
    bool ret = false;
    if (m_smHandler == NULL) {
        return ret;
    }

    ret = m_smHandler->hasMessages(what);
    if (ret == false) {
        ret = m_smHandler->hasDeferMessage(what);
    }

    return ret;
}

bool CStateMachine::hasDeferredMessages(int what)
{
    bool ret = false;
    if (m_smHandler == NULL) {
        return ret;
    }

    ret = m_smHandler->hasDeferMessage(what);

    return ret;
}


//------------------------------------------------------------------------------------
const static char *TAGSMHander = "SMHander";

SMHander::SMHander(CStateMachine *sm)
    : m_HaltingState(NULL)
    , m_QuittingState(NULL)
    , m_StateMachine(sm)
    , m_HasQuit(false)
    , m_IsConstructionCompleted(false)
    , m_initState(NULL)
    , m_desState(NULL)
    , m_TempStateStackCount(0)
    , m_StateStackTopIndex(-1)
    , mTempStateStack(NULL)
    , mStateStack(NULL)
{
    if (NULL == m_HaltingState) {
        m_HaltingState = new HaltingState(this);
        if (DBG) {
            UTILS_LOGD(TAGSMHander, "m_HaltingState %p", m_HaltingState);
        }
        addState(m_HaltingState, NULL);
    }

    if (NULL == m_QuittingState) {
        m_QuittingState = new QuittingState(this);
        if (DBG) {
            UTILS_LOGD(TAGSMHander, "m_QuittingState %p", m_QuittingState);
        }
        addState(m_QuittingState, NULL);
    }
}

SMHander::~SMHander()
{
    clear();
}

 SMHander::HaltingState::HaltingState(SMHander *handler)
     : CState(handler->m_StateMachine)
     , m_SMHander(handler)
{

}

bool SMHander::HaltingState::processMessage(const CMessage &message)
{
    m_SMHander->m_StateMachine->haltedProcessMessage(message);

    return true;
}

SMHander::QuittingState::QuittingState(SMHander *handler)
    : CState(handler->m_StateMachine)
    , m_SMHander(handler)

{
   if (DBG) UTILS_LOGD(TAGSMHander, "QuittingState %p %p", this, m_SMHander);
}


bool SMHander::QuittingState::processMessage(const CMessage &message)
{
    return false;
}

/** @see StateMachine#setInitialState(State) */
void SMHander::setInitialState(CState *initialState)
{
    if (DBG) {
        UTILS_LOGD(TAGSMHander, "set init state %p", initialState);
    }

    m_initState = initialState;
}

void SMHander::transitionTo(CState *destState)
{
    if (DBG) {
        UTILS_LOGD(TAGSMHander, "transfer state %p", destState);
    }

    m_desState = destState;
}

void SMHander::deferMessage(const CMessage &msg)
{
    if (DBG) {
        UTILS_LOGD(TAGSMHander, "defer message %d", msg.what);
    }

    m_deferredMessages.push_back(msg);
}

void SMHander::removeDeferMessage(int what)
{
    if (DBG) {
        UTILS_LOGD(TAGSMHander, "remove defer message %d", what);
    }

    std::vector<CMessage>::iterator it = m_deferredMessages.begin();

    while (it != m_deferredMessages.end()) {
        if (it->what == what) {
            it = m_deferredMessages.erase(it);
        } else {
            ++it;
        }
    }
}

void SMHander::flushDeferMessage()
{
    if (DBG) {
        UTILS_LOGD(TAGSMHander, "remove all defer message");
    }

    m_deferredMessages.clear();
}

void SMHander::completeConstruction()
{
    /**
     * Determine the maximum depth of the state hierarchy
     * so we can allocate the state stacks.
     */
    int maxDepth = 0;

    std::map<CState*, CStateInfo*>::iterator itMap = m_StateInfo.begin();

    for (; itMap != m_StateInfo.end(); ++itMap) {
        int depth = 0;
        for (CStateInfo *i = itMap->second; i != NULL; depth++) {
            i = i->parentStateInfo;
        }
        if (maxDepth < depth) {
            maxDepth = depth;
        }
    }

    mStateStack = new CStateInfo*[maxDepth];
    mTempStateStack = new CStateInfo*[maxDepth];
    setupInitialStateStack();

    /** Sending SM_INIT_CMD message to invoke enter methods asynchronously */
    CMessage message;
    message.what = CStateMachine::SM_INIT_COMMAND;

    sendMessageAtFrontOfQueue(message);

}

void SMHander::setupInitialStateStack()
{
    if (DBG) {
        UTILS_LOGD(TAGSMHander, "setup initial state stack");
    }

    std::map<CState*, CStateInfo*>::iterator itMap = m_StateInfo.find(m_initState);
    if (m_StateInfo.end() == itMap) {
        return;
    }

    CStateInfo *curStateInfo = itMap->second;

    for (m_TempStateStackCount = 0; curStateInfo != NULL; m_TempStateStackCount++) {
        mTempStateStack[m_TempStateStackCount] = curStateInfo;
        curStateInfo = curStateInfo->parentStateInfo;
    }

    // Empty the StateStack
    m_StateStackTopIndex = -1;

    moveTempStateStackToStateStack();
}


int SMHander::handleMessage(const CMessage &message)
{
    if (!m_HasQuit) {
        m_message = message;

        if (m_IsConstructionCompleted) {
            processMsg(message);
        } else if (!m_IsConstructionCompleted &&
                   message.what == CStateMachine::SM_INIT_COMMAND){
            m_IsConstructionCompleted = true;
            invokeEnterMethods(0);
        }
        if (DBG) UTILS_LOGD(TAGSMHander, "performTransitions");

        performTransitions();
    }

    return 0;
}

void SMHander::processMsg(const CMessage &msg)
{
    CStateInfo *curStateInfo = mStateStack[m_StateStackTopIndex];

    if (isQuit(msg)) {
        if (DBG) UTILS_LOGD(TAGSMHander, "transitionTo(m_QuittingState)");
        transitionTo(m_QuittingState);
    } else {
        while (!curStateInfo->state->processMessage(msg)) {
            /**
             * Not processed
             */
            curStateInfo = curStateInfo->parentStateInfo;
            if (curStateInfo == NULL) {
                /**
                 * No parents left so it's not handled
                 */
                m_StateMachine->unhandledMessage(msg);
                break;
            }
        }
    }

}

void SMHander::performTransitions()
{
    CState *destState = NULL;
    while (m_desState != NULL) {
        if (DBG) {
            UTILS_LOGD(TAGSMHander, "perform transfer");
        }

        destState = m_desState;
        m_desState = NULL;

        CStateInfo *commonStateInfo = setupTempStateStackWithStatesToEnter(destState);
        invokeExitMethods(commonStateInfo);
        int stateStackEnteringIndex = moveTempStateStackToStateStack();
        invokeEnterMethods(stateStackEnteringIndex);

        moveDeferredMessageAtFrontOfQueue();
    }

     if (destState != NULL) {
        if (destState == m_QuittingState) {
            /**
             * Call onQuitting to let subclasses cleanup.
             */
            if (DBG) UTILS_LOGD(TAGSMHander, "destState is m_QuittingState, onQuitting");
            m_StateMachine->onQuitting();
            cleanupAfterQuitting();
        } else if (destState == m_HaltingState) {
            /**
             * Call onHalting() if we've transitioned to the halting
             * state. All subsequent messages will be processed in
             * in the halting state which invokes haltedProcessMessage(msg);
             */
            m_StateMachine->onHalting();
        }
    }
}

void SMHander::cleanupAfterQuitting()
{
    if (DBG) {
        UTILS_LOGD(TAGSMHander, "cleanupAfterQuitting");
    }

    clear();

    if (-1 != m_StateMachine->mWaitQuitId) {
        m_StateMachine->mWaitQuit.lock();
        m_HasQuit = true;
        m_StateMachine->mWaitQuit.signal(m_StateMachine->mWaitQuitId);
        m_StateMachine->mWaitQuit.unlock();
    }
}

void SMHander::clear()
{
    if (DBG) {
        UTILS_LOGD(TAGSMHander, "clear");
    }

    m_deferredMessages.clear();

    std::map<CState*, CStateInfo*>::iterator it = m_StateInfo.begin();
    for (; it != m_StateInfo.end(); ++it) {
       if (it->second) {
           delete it->second;
       }
    }
    m_StateInfo.clear();

    if (mStateStack) {
       delete[] mStateStack;
       mStateStack = NULL;
    }

    if (mTempStateStack) {
       delete[] mTempStateStack;
       mTempStateStack = NULL;
    }

    if (m_QuittingState) {
       delete m_QuittingState;
       m_QuittingState = NULL;
    }

    if (m_HaltingState) {
       delete m_HaltingState;
       m_HaltingState = NULL;
    }
}

CStateInfo *SMHander::setupTempStateStackWithStatesToEnter(CState *destState)
{
    m_TempStateStackCount = 0;

    if (DBG) {
        UTILS_LOGD(TAGSMHander, "setup temp stack with states to enter %p", destState);
    }

    std::map<CState*, CStateInfo*>::iterator itMap = m_StateInfo.find(destState);
    if (m_StateInfo.end() == itMap) {
        return NULL;
    }

    CStateInfo *curStateInfo = itMap->second;
    do {
        mTempStateStack[m_TempStateStackCount++] = curStateInfo;
        curStateInfo = curStateInfo->parentStateInfo;
    } while ((curStateInfo != NULL) && !curStateInfo->active);

    return curStateInfo;
}

void SMHander::invokeExitMethods(CStateInfo *commonStateInfo)
{
    if (DBG) {
        UTILS_LOGD(TAGSMHander, "invoke exit methods, state stack top index = %d",
        m_StateStackTopIndex);
    }

    while ((m_StateStackTopIndex >= 0) &&
            (mStateStack[m_StateStackTopIndex] != commonStateInfo)) {
        CState *curState = mStateStack[m_StateStackTopIndex]->state;

        curState->exit();
        mStateStack[m_StateStackTopIndex]->active = false;
        m_StateStackTopIndex -= 1;
    }
}

void SMHander::invokeEnterMethods(int stateStackEnteringIndex)
{
    if (DBG) {
        UTILS_LOGD(TAGSMHander, "invoke enter methods, state stack enter index = %d, stack top index = %d",
                stateStackEnteringIndex,
                m_StateStackTopIndex);
    }


    for (int i = stateStackEnteringIndex; i <= m_StateStackTopIndex; i++) {
        mStateStack[i]->state->enter();
        mStateStack[i]->active = true;
    }
}

int SMHander::moveTempStateStackToStateStack()
{
    int startingIndex = m_StateStackTopIndex + 1;
    int i = m_TempStateStackCount - 1;
    int j = startingIndex;
    while (i >= 0) {
        mStateStack[j] = mTempStateStack[i];
        j += 1;
        i -= 1;
    }

    m_StateStackTopIndex = j - 1;

    if (DBG) {
        UTILS_LOGD(TAGSMHander, "state stack start index = %d, top index = %d",
                startingIndex, m_TempStateStackCount);
    }

    return startingIndex;
}


void SMHander::moveDeferredMessageAtFrontOfQueue()
{
    /**
     * The oldest messages on the deferred list must be at
     * the front of the queue so start at the back, which
     * as the most resent message and end with the oldest
     * messages at the front of the queue.
     */
    for (int i = m_deferredMessages.size() - 1; i >= 0; i-- ) {
        CMessage curMsg = m_deferredMessages.at(i);

        sendMessageAtFrontOfQueue(curMsg);
    }
    m_deferredMessages.clear();
}

void SMHander::quit()
{
    CMessage message;
    message.what = CStateMachine::SM_QUIT_COMMAND;

    sendMessage(message);
}

void SMHander::quitNow()
{
    CMessage message;
    if (DBG) UTILS_LOGD("SMHander", "quitNow");
    message.what = CStateMachine::SM_QUIT_COMMAND;

    sendMessageAtFrontOfQueue(message);
}

bool SMHander::isQuit(const CMessage &msg)
{
    if (DBG) UTILS_LOGD("isQuit", "msg.what = %d", msg.what);

    return (msg.what == CStateMachine::SM_QUIT_COMMAND)/* && (msg.obj == m_smHandlerObj)*/;
}

CStateInfo *SMHander::addState(CState *state, CState *parent)
{
    CStateInfo *parentStateInfo = NULL;
    std::map<CState*, CStateInfo*>::iterator itMap;

    if (parent != NULL) {
        itMap = m_StateInfo.find(parent);
        if (m_StateInfo.end() == itMap) {
            // Recursively add our parent as it's not been added yet.
            parentStateInfo = addState(parent, NULL);
        } else {
            parentStateInfo = itMap->second;
        }
    }

    CStateInfo *stateInfo = NULL;
    itMap = m_StateInfo.find(state);
    if (m_StateInfo.end() == itMap) {
        stateInfo = new CStateInfo();
        m_StateInfo.insert(std::pair<CState*, CStateInfo*>(state, stateInfo));
    }

    // Validate that we aren't adding the same state in two different hierarchies.
    if ((stateInfo->parentStateInfo != NULL) &&
            (stateInfo->parentStateInfo != parentStateInfo)) {
        UTILS_LOGE(TAGSMHander, "state already added: %p, parent: %p",
                stateInfo->parentStateInfo, parentStateInfo);
    }
    stateInfo->state = state;
    stateInfo->parentStateInfo = parentStateInfo;
    stateInfo->active = false;

    return stateInfo;
}

CMessage SMHander::getCurrentMessage() {
    return m_message;
}


CState *SMHander::getCurrentState()
{
    return mStateStack[m_StateStackTopIndex]->state;
}

bool SMHander::hasDeferMessage(int what)
{
    bool ret = false;

    std::vector<CMessage>::iterator it = m_deferredMessages.begin();
    while (it != m_deferredMessages.end()) {
        if (it->what == what) {
            ret = true;
            break;
        } else {
            ++it;
        }
    }

    return ret;
}

}
