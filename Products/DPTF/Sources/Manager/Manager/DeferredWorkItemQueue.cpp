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

#include "DeferredWorkItemQueue.h"
#include "EsifMutexHelper.h"
#include "WorkItemMatches.h"
#include "XmlNode.h"

DeferredWorkItemQueue::DeferredWorkItemQueue(EsifSemaphore* workItemQueueSemaphore) :
    m_workItemQueueSemaphore(workItemQueueSemaphore), m_timer(TimerCallback, this), m_maxCount(0)
{
}

DeferredWorkItemQueue::~DeferredWorkItemQueue(void)
{
    // makeEmpty is a public function and it will lock the mutex
    makeEmtpy();
}

void DeferredWorkItemQueue::enqueue(DeferredWorkItem* newWorkItem)
{
    // FIMXE:  during round 2, need to add statistics logging

    // Insert into the queue sorted based on time stamp.  The timer must be set to expire
    // when the first item in the queue is ready to process.

    EsifMutexHelper esifMutexHelper(&m_mutex);
    esifMutexHelper.lock();

    insertSortedByDeferredProcessingTime(newWorkItem);
    updateMaxCount();
    setTimer();

    esifMutexHelper.unlock();
}

DeferredWorkItem* DeferredWorkItemQueue::dequeue(void)
{
    // Returns the first item in the queue if it the work item time is >= the current time.

    EsifMutexHelper esifMutexHelper(&m_mutex);
    esifMutexHelper.lock();

    DeferredWorkItem* firstReadyWorkItem = getFirstReadyWorkItemFromQueue();
    setTimer();

    esifMutexHelper.unlock();

    return firstReadyWorkItem;
}

void DeferredWorkItemQueue::makeEmtpy(void)
{
    EsifMutexHelper esifMutexHelper(&m_mutex);
    esifMutexHelper.lock();

    m_timer.cancelTimer();

    while (m_queue.empty() == false)
    {
        DeferredWorkItem* currentWorkItem = m_queue.front();
        delete currentWorkItem;
        m_queue.pop_front();
    }

    esifMutexHelper.unlock();
}

UInt64 DeferredWorkItemQueue::getCount(void) const
{
    UInt64 count;
    EsifMutexHelper esifMutexHelper(&m_mutex);

    esifMutexHelper.lock();
    count = m_queue.size();
    esifMutexHelper.unlock();

    return count;
}

UInt64 DeferredWorkItemQueue::getMaxCount(void) const
{
    UInt64 maxCount;
    EsifMutexHelper esifMutexHelper(&m_mutex);

    esifMutexHelper.lock();
    maxCount = m_maxCount;
    esifMutexHelper.unlock();

    return maxCount;
}

UIntN DeferredWorkItemQueue::removeIfMatches(const WorkItemMatchCriteria& matchCriteria)
{
    EsifMutexHelper esifMutexHelper(&m_mutex);
    esifMutexHelper.lock();

    UInt64 originalCount = m_queue.size();
    m_queue.remove_if(WorkItemMatches(matchCriteria));
    UInt64 finalCount = m_queue.size();
    setTimer();

    esifMutexHelper.unlock();

    return static_cast<UIntN>(originalCount - finalCount);
}

XmlNode* DeferredWorkItemQueue::getXml(void) const
{
    EsifMutexHelper esifMutexHelper(&m_mutex);
    esifMutexHelper.lock();

    XmlNode* deferredQueueStastics = XmlNode::createWrapperElement("deferred_queue_statistics");
    deferredQueueStastics->addChild(XmlNode::createDataElement("current_count", std::to_string(m_queue.size())));
    deferredQueueStastics->addChild(XmlNode::createDataElement("max_count", std::to_string(m_maxCount)));

    esifMutexHelper.unlock();

    return deferredQueueStastics;
}

//
// The following *private* methods do not need to lock the mutex.  The caller is responsible for
// locking and unlocking.
//

void DeferredWorkItemQueue::setTimer(void)
{
    // Set the timer to expire when the first item in the queue is ready to process

    if (m_queue.empty() == true)
    {
        // the timer shouldn't be running.  cancel it if needed.
        if (m_timer.isExpirationTimeValid() == true)
        {
            m_timer.cancelTimer();
        }
    }
    else
    {
        DeferredWorkItem* firstWorkItem = m_queue.front();
        EsifTime firstWorkItemTime = firstWorkItem->getDeferredProcessingTime();

        if (m_timer.isExpirationTimeValid() == true)
        {
            // Make sure the timer is set correctly.  If not, cancel and reset it.
            EsifTime expirationTime = m_timer.getExpirationTime();
            if (firstWorkItemTime != expirationTime)
            {
                m_timer.startTimer(firstWorkItemTime);
            }
        }
        else
        {
            // Need to start the timer
            m_timer.startTimer(firstWorkItemTime);
        }
    }
}

DeferredWorkItem* DeferredWorkItemQueue::getFirstReadyWorkItemFromQueue(void)
{
    DeferredWorkItem* firstReadyWorkItem = nullptr;

    if (m_queue.empty() == false)
    {
        EsifTime currentTime;
        firstReadyWorkItem = m_queue.front();

        if (firstReadyWorkItem->getDeferredProcessingTime() <= currentTime)
        {
            m_queue.pop_front();
        }
        else
        {
            firstReadyWorkItem = nullptr;
        }
    }

    return firstReadyWorkItem;
}

void DeferredWorkItemQueue::insertSortedByDeferredProcessingTime(DeferredWorkItem* newWorkItem)
{
    auto it = m_queue.begin();
    for (; it != m_queue.end(); it++)
    {
        DeferredWorkItem* currentWorkItem = *it;
        if (currentWorkItem->getDeferredProcessingTime() > newWorkItem->getDeferredProcessingTime())
        {
            m_queue.insert(it, newWorkItem);
            break;
        }
    }

    if (it == m_queue.end())
    {
        m_queue.push_back(newWorkItem);
    }
}

void DeferredWorkItemQueue::updateMaxCount()
{
    if (m_queue.size() > m_maxCount)
    {
        m_maxCount = m_queue.size();
    }
}

//
// The following two functions get called when the timer expires.  The semaphore is signaled so the
// work item thread will check the queue for work items that are ready to process.
// The timer starts again when the work item thread dequeues the first work item.

void DeferredWorkItemQueue::timerCallback(void) const
{
    // The WorkItemQueueManager is not locked while this executes.  So, do nothing but signal the semaphore.
    m_workItemQueueSemaphore->signal();
}

void TimerCallback(const void* context_ptr)
{
    const DeferredWorkItemQueue* deferredWorkItemQueue = static_cast<const DeferredWorkItemQueue*>(context_ptr);
    deferredWorkItemQueue->timerCallback();
}