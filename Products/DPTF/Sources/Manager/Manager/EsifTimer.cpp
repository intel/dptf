/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#include "EsifTimer.h"

EsifTimer::EsifTimer(esif_ccb_timer_cb callbackFunction, void* contextPtr) :
    m_callbackFunction(callbackFunction), m_contextPtr(contextPtr), m_timerInitialized(false),
    m_expirationTime(EsifTime(0))
{
    esif_ccb_memset(&m_timer, sizeof(esif_ccb_timer_t), 0);
}

EsifTimer::~EsifTimer(void)
{
    esifTimerKill();
}

void EsifTimer::startTimer(EsifTime expirationTime)
{
    esifTimerSet(expirationTime);
}

void EsifTimer::cancelTimer(void)
{
    esifTimerKill();
}

Bool EsifTimer::isExpirationTimeValid(void) const
{
    return (m_expirationTime.getTimeStampInMilliSec() != 0);
}

EsifTime EsifTimer::getExpirationTime(void) const
{
    return m_expirationTime;
}

void EsifTimer::esifTimerInit()
{
    if (m_timerInitialized == false)
    {
        eEsifError rc = esif_ccb_timer_init(&m_timer, m_callbackFunction, m_contextPtr);
        if (rc != ESIF_OK)
        {
            esif_ccb_memset(&m_timer, sizeof(esif_ccb_timer_t), 0);
            throw dptf_exception("Failed to initialize timer.");
        }

        m_timerInitialized = true;
    }
}

void EsifTimer::esifTimerKill()
{
    if (m_timerInitialized == true)
    {
        // Do not check the return code or throw an exception.  ESIF is responsible for
        // killing the timer and we can't do anything if this fails.
        esif_ccb_timer_kill(&m_timer);

        esif_ccb_memset(&m_timer, sizeof(esif_ccb_timer_t), 0);
        m_timerInitialized = false;
        m_expirationTime = EsifTime(0);
    }
}

void EsifTimer::esifTimerSet(EsifTime expirationTime)
{
    esifTimerKill();
    esifTimerInit();

    eEsifError rc = esif_ccb_timer_set_msec(&m_timer, calculateMilliSecondsUntilTimerExpires(expirationTime));
    if (rc != ESIF_OK)
    {
        throw dptf_exception("Failed to start timer.");
    }

    m_expirationTime = expirationTime;
}

UInt64 EsifTimer::calculateMilliSecondsUntilTimerExpires(EsifTime expirationTime)
{
    EsifTime currentTime;
    UInt64 numMilliSeconds = (expirationTime > currentTime) ? (expirationTime - currentTime) : 1;
    return numMilliSeconds;
}