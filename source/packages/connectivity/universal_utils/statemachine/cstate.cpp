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

#include "cstate.h"
#include "cstatemachine.h"

namespace universal_utils {

CState::CState(CStateMachine *stateMachine) :
    m_stateMachine(stateMachine)
{


}

CState::~CState()
{

}


void CState::enter()
{

}

void CState::exit()
{

}

bool CState::processMessage(const CMessage &message)
{
    return false;
}

void CState::transitionTo(CState *destState)
{
    if (m_stateMachine) {
        m_stateMachine->transitionTo(destState);
    }
}

void CState::deferMessage(const CMessage &msg)
{
    if (m_stateMachine) {
        m_stateMachine->deferMessage(msg);
    }
}


void CState::sendMessage(int what)
{
    if (m_stateMachine) {
        m_stateMachine->sendMessage(what);
    }
}

void CState::sendMessage(const CMessage &msg)
{
    if (m_stateMachine) {
        m_stateMachine->sendMessage(msg);
    }
}

void CState::sendMessageDelayed(int what, long delayMillis)
{
    if (m_stateMachine) {
        m_stateMachine->sendMessageDelayed(what, delayMillis);
    }
}

void CState::sendMessageDelayed(int what, const CRawString &obj, long delayMillis)
{
    if (m_stateMachine) {
        m_stateMachine->sendMessageDelayed(what, obj, delayMillis);
    }
}

void CState::sendMessageDelayed(const CMessage &msg, long delayMillis)
{
    if (m_stateMachine) {
        m_stateMachine->sendMessageDelayed(msg, delayMillis);
    }
}

void CState::sendMessageDelayed(const CMessage &msg, const CRawString &obj, long delayMillis)
{
    if (m_stateMachine) {
        m_stateMachine->sendMessageDelayed(msg, obj, delayMillis);
    }
}

void CState::sendMessageAtFrontOfQueue(int what, const CRawString &obj)
{
    if (m_stateMachine) {
        m_stateMachine->sendMessageAtFrontOfQueue(what, obj);
    }
}

void CState::sendMessageAtFrontOfQueue(int what)
{
    if (m_stateMachine) {
        m_stateMachine->sendMessageAtFrontOfQueue(what);
    }
}

void CState::sendMessageAtFrontOfQueue(const CMessage &msg)
{
    if (m_stateMachine) {
        m_stateMachine->sendMessageAtFrontOfQueue(msg);
    }
}

void CState::removeMessages(int what)
{
    if (m_stateMachine) {
        m_stateMachine->removeMessages(what);
    }
}

void CState::removeDeferredMessages(int what)
{
    if (m_stateMachine) {
        m_stateMachine->removeDeferredMessages(what);
    }
}

void CState::flushMessages()
{
    if (m_stateMachine) {
        m_stateMachine->flushMessages();
    }
}

bool CState::hasMessages(int what)
{
    bool ret = false;

    if (m_stateMachine) {
        ret = m_stateMachine->hasMessages(what);
    }

    return ret;
}

bool CState::hasDeferredMessages(int what)
{
    bool ret = false;

    if (m_stateMachine) {
        ret = m_stateMachine->hasDeferredMessages(what);
    }

        return ret;
}

}
