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

#include "WorkItemQueueThread.h"
#include "EsifThread.h"
#include "DptfManager.h"
#include "ParticipantManager.h"

WorkItemQueueThread::WorkItemQueueThread(DptfManager* dptfManager, ImmediateWorkItemQueue* immediateQueue,
    DeferredWorkItemQueue* deferredQueue, EsifSemaphore* workItemQueueSemaphore,
    WorkItemStatistics* workItemStatistics) :
    m_dptfManager(dptfManager), m_destroyThread(false),
    m_immediateQueue(immediateQueue), m_deferredQueue(deferredQueue),
    m_workItemQueueSemaphore(workItemQueueSemaphore), m_workItemQueueThreadId(nullptr),
    m_workItemStatistics(workItemStatistics)
{
    m_participantManager = m_dptfManager->getParticipantManager();
    EsifThread(ThreadStart, this);
}

WorkItemQueueThread::~WorkItemQueueThread(void)
{
    m_destroyThread = true;
    m_workItemQueueSemaphore->signal();
    DELETE_MEMORY_TC(m_workItemQueueThreadId);
}

EsifThreadId WorkItemQueueThread::getWorkItemQueueThreadId(void) const
{
    if (m_workItemQueueThreadId == nullptr)
    {
        throw dptf_exception("Work item queue thread not initialized");
    }

    return *m_workItemQueueThreadId;
}

std::string WorkItemQueueThread::toXml(void) const
{
    throw implement_me();
}

void WorkItemQueueThread::executeThread(void)
{
    // This is running on a separate thread and processes all work items until
    // the destructor is executed

    m_workItemQueueThreadId = new EsifThreadId();

    while (m_destroyThread == false)
    {
        processImmediateQueue();
        processDeferredQueue();

        m_workItemQueueSemaphore->wait();
    }
}

void WorkItemQueueThread::processImmediateQueue(void)
{
    ImmediateWorkItem* immediateWorkItem = m_immediateQueue->dequeue();

    while (immediateWorkItem != nullptr)
    {
        //FrameworkEvent::Type eventType = immediateWorkItem->getFrameworkEventType();

        try
        {
            immediateWorkItem->setWorkItemExecutionStartTime();
            immediateWorkItem->execute();
        }
        catch (...)
        {
        }

        try
        {
            m_participantManager->clearAllParticipantCachedData();
        }
        catch (...)
        {
        }

        m_workItemStatistics->incrementImmediateTotals(immediateWorkItem);
        delete immediateWorkItem;

        immediateWorkItem = m_immediateQueue->dequeue();
    }
}

void WorkItemQueueThread::processDeferredQueue(void)
{
    DeferredWorkItem* deferredWorkItem = m_deferredQueue->dequeue();

    while (deferredWorkItem != nullptr)
    {
        //FrameworkEvent::Type eventType = deferredWorkItem->getFrameworkEventType();

        try
        {
            deferredWorkItem->setWorkItemExecutionStartTime();
            deferredWorkItem->execute();
        }
        catch (...)
        {
        }

        try
        {
            m_participantManager->clearAllParticipantCachedData();
        }
        catch (...)
        {
        }

        m_workItemStatistics->incrementDeferredTotals(deferredWorkItem);
        delete deferredWorkItem;

        deferredWorkItem = m_deferredQueue->dequeue();
    }
}

void* ThreadStart(void* contextPtr)
{
    WorkItemQueueThread* workItemQueueThread = static_cast<WorkItemQueueThread*>(contextPtr);
    workItemQueueThread->executeThread();
    return nullptr;
}