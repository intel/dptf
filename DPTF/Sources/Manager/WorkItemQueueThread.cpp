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

#include "WorkItemQueueThread.h"
#include "ParticipantManagerInterface.h"

WorkItemQueueThread::WorkItemQueueThread(
	DptfManagerInterface* dptfManager,
	ImmediateWorkItemQueue* immediateQueue,
	DeferredWorkItemQueue* deferredQueue,
	EsifSemaphore* workItemQueueSemaphore,
	WorkItemStatistics* workItemStatistics)
	: m_dptfManager(dptfManager)
	, m_participantManager(nullptr)
	, m_destroyThread(false)
	, m_immediateQueue(immediateQueue)
	, m_deferredQueue(deferredQueue)
	, m_workItemQueueSemaphore(workItemQueueSemaphore)
	, m_workItemQueueThreadHandle(nullptr)
	, m_workItemQueueThreadId(nullptr)
	, m_workItemQueueThreadExitSemaphore(nullptr)
	, m_workItemStatistics(workItemStatistics)
{
	m_participantManager = m_dptfManager->getParticipantManager();
	m_workItemQueueThreadExitSemaphore = new EsifSemaphore();
	m_workItemQueueThreadHandle = new EsifThread(ThreadStart, this);
}

WorkItemQueueThread::~WorkItemQueueThread(void)
{
	m_destroyThread = true;

	m_workItemQueueSemaphore->signal();
	m_workItemQueueThreadExitSemaphore->wait();

	DELETE_MEMORY_TC(m_workItemQueueThreadHandle);
	DELETE_MEMORY_TC(m_workItemQueueThreadExitSemaphore);
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
		m_workItemQueueSemaphore->wait();
	}

	DELETE_MEMORY_TC(m_workItemQueueThreadId);

	m_workItemQueueThreadExitSemaphore->signal();
}

void WorkItemQueueThread::processImmediateQueue(void)
{
	auto immediateWorkItem = m_immediateQueue->dequeue();
	while (immediateWorkItem.get() != nullptr)
	{
		// FrameworkEvent::Type eventType = immediateWorkItem->getFrameworkEventType();

#ifdef INCLUDE_WORK_ITEM_STATISTICS
		try
		{
			immediateWorkItem->setWorkItemExecutionStartTime();
		}
		catch (...)
		{
		}
#endif

		try
		{
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

#ifdef INCLUDE_WORK_ITEM_STATISTICS
		try
		{
			m_workItemStatistics->incrementImmediateTotals(immediateWorkItem);
		}
		catch (...)
		{
		}
#endif
		immediateWorkItem = m_immediateQueue->dequeue();
	}
}

void* ThreadStart(void* contextPtr)
{
	WorkItemQueueThread* workItemQueueThread = static_cast<WorkItemQueueThread*>(contextPtr);
	workItemQueueThread->executeThread();
	return nullptr;
}
