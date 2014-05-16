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

#include "Dptf.h"
#include "WorkItem.h"
#include "WorkItemQueueThread.h"
#include "ImmediateWorkItemQueue.h"
#include "DeferredWorkItemQueue.h"
#include "ImmediateWorkItem.h"
#include "DeferredWorkItem.h"
#include "EsifMutex.h"

class DptfManager;

class WorkItemQueueManager
{
public:

    WorkItemQueueManager(DptfManager* dptfManager);
    ~WorkItemQueueManager(void);

    void enqueueImmediateWorkItemAndReturn(WorkItem* workItem);
    void enqueueImmediateWorkItemAndReturn(WorkItem* workItem, UIntN priority);
    void enqueueImmediateWorkItemAndWait(WorkItem* workItem);
    void enqueueImmediateWorkItemAndWait(WorkItem* workItem, UIntN priority);
    void enqueueDeferredWorkItem(WorkItem* workItem, UInt64 numMilliSecUntilExecution);

    UIntN removeIfMatches(const WorkItemMatchCriteria& matchCriteria);
    Bool isWorkItemThread(void);

    void disableAndEmptyAllQueues(void);

    XmlNode* getStatusAsXml(void);

private:

    WorkItemQueueManager(const WorkItemQueueManager& rhs);
    WorkItemQueueManager& operator=(const WorkItemQueueManager& rhs);

    DptfManager* m_dptfManager;
    Bool m_enqueueingEnabled;                                       // if TRUE, new work items will be added to the queues
    mutable EsifMutex m_mutex;

    WorkItemStatistics* m_workItemStatistics;
    ImmediateWorkItemQueue* m_immediateQueue;
    DeferredWorkItemQueue* m_deferredQueue;
    WorkItemQueueThread* m_workItemQueueThread;

    // - The following semaphore is signaled when:
    //    * an item is placed in the immediate or deferred queue
    //    * the system is shutting down and the thread needs to exit
    // - The work item queue thread will block waiting on this semaphore.
    // - It is created by the WorkItemQueueManager and passed in to the queues and thread as a parameter.
    EsifSemaphore* m_workItemQueueSemaphore;

    void deleteAllObjects(void);
    Bool canEnqueueImmediateWorkItem(WorkItem* workItem) const;
};