/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "ImmediateWorkItemQueue.h"
#include "DeferredWorkItemQueue.h"
#include "EsifThread.h"
#include "EsifThreadId.h"
#include "WorkItemStatistics.h"

class ParticipantManagerInterface;

class WorkItemQueueThread
{
public:
	WorkItemQueueThread(
		DptfManagerInterface* dptfManager,
		ImmediateWorkItemQueue* immediateQueue,
		DeferredWorkItemQueue* deferredQueue,
		EsifSemaphore* workItemQueueSemaphore,
		WorkItemStatistics* workItemStatistics);
	~WorkItemQueueThread(void);

	EsifThreadId getWorkItemQueueThreadId(void) const;
	std::string toXml(void) const;

private:
	// hide the copy constructor and assignment operator.
	WorkItemQueueThread(const WorkItemQueueThread& rhs);
	WorkItemQueueThread& operator=(const WorkItemQueueThread& rhs);

	DptfManagerInterface* m_dptfManager;
	ParticipantManagerInterface* m_participantManager;
	Bool m_destroyThread;
	ImmediateWorkItemQueue* m_immediateQueue;
	DeferredWorkItemQueue* m_deferredQueue;
	EsifSemaphore* m_workItemQueueSemaphore;
	EsifThread* m_workItemQueueThreadHandle;
	EsifThreadId* m_workItemQueueThreadId;
	EsifSemaphore* m_workItemQueueThreadExitSemaphore;
	WorkItemStatistics* m_workItemStatistics;

	friend void* ThreadStart(void* contextPtr);
	void executeThread(void);
	void processImmediateQueue(void);
};

void* ThreadStart(void* contextPtr);
