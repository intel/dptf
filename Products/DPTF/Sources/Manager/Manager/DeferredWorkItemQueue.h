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

#include "WorkItemQueueInterface.h"
#include "DeferredWorkItem.h"
#include "EsifMutex.h"
#include "EsifSemaphore.h"
#include "EsifTime.h"
#include "EsifTimer.h"

static void TimerCallback(const void* context_ptr);

class DeferredWorkItemQueue : public WorkItemQueueInterface
{
public:

    DeferredWorkItemQueue(EsifSemaphore* workItemQueueSemaphore);
    virtual ~DeferredWorkItemQueue(void);

    void enqueue(DeferredWorkItem* newWorkItem);
    DeferredWorkItem* dequeue(void);

    // implement WorkItemQueueInterface
    virtual void makeEmtpy(void) override final;
    virtual UInt64 getCount(void) const override final;
    virtual UInt64 getMaxCount(void) const override final;
    virtual UIntN removeIfMatches(const WorkItemMatchCriteria& matchCriteria) override final;
    virtual XmlNode* getXml(void) const override final;

private:

    // hide the copy constructor and assignment operator.
    DeferredWorkItemQueue(const DeferredWorkItemQueue& rhs);
    DeferredWorkItemQueue& operator=(const DeferredWorkItemQueue& rhs);

    std::list<DeferredWorkItem*> m_queue;
    UInt64 m_maxCount;                                              // stores the maximum number of items in the queue at any one time
    mutable EsifMutex m_mutex;
    EsifSemaphore* m_workItemQueueSemaphore;
    EsifTimer m_timer;

    void setTimer(void);
    DeferredWorkItem* getFirstReadyWorkItemFromQueue(void);
    void insertSortedByDeferredProcessingTime(DeferredWorkItem* newWorkItem);
    void updateMaxCount(void);

    // The timer will call a 'C' function which will need to forward the call to our private
    // timerCallback method.  We set it up as a friend so it has access.
    void timerCallback(void) const;
    friend void TimerCallback(const void* context_ptr);
};

void TimerCallback(const void* context_ptr);