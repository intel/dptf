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

#pragma once

#include "esif_uf_ccb_thread.h"
#include "esif_ccb_timer.h"
#include "EsifTime.h"

// Function prototype for timer callback:
// void (*esif_ccb_timer_cb)(void *context);

class EsifTimer
{
public:

    EsifTimer(esif_ccb_timer_cb callbackFunction, void* contextPtr = nullptr);
    ~EsifTimer(void);

    void startTimer(EsifTime expirationTime);
    void cancelTimer(void);

    Bool isExpirationTimeValid(void) const;
    EsifTime getExpirationTime(void) const;

private:

    // hide the copy constructor and assignment operator.
    EsifTimer(const EsifTimer& rhs);
    EsifTimer& operator=(const EsifTimer& rhs);

    esif_ccb_timer_cb m_callbackFunction;
    void* m_contextPtr;

    Bool m_timerInitialized;
    esif_ccb_timer_t m_timer;
    EsifTime m_expirationTime;

    void esifTimerInit(void);
    void esifTimerKill(void);
    void esifTimerSet(EsifTime expirationTime);

    UInt64 calculateMilliSecondsUntilTimerExpires(EsifTime expirationTime);
};