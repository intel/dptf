/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "WorkItemQueueManager.h"
#include "EsifServicesInterface.h"
#include "EsifMutexHelper.h"
#include "EsifThreadId.h"
#include "XmlNode.h"
#include "ManagerLogger.h"
#include <memory>

WorkItemQueueManager::WorkItemQueueManager(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
	, m_enqueueingEnabled(true)
	, m_workItemStatistics(nullptr)
	, m_immediateQueue(nullptr)
	, m_deferredQueue(nullptr)
	, m_workItemQueueThread(nullptr)
	, m_workItemQueueSemaphore(nullptr)
{
	try
	{
		m_workItemStatistics = new WorkItemStatistics();
		m_workItemQueueSemaphore = new EsifSemaphore();
		m_immediateQueue = new ImmediateWorkItemQueue(m_workItemQueueSemaphore);
		m_deferredQueue = new DeferredWorkItemQueue(m_workItemQueueSemaphore, m_immediateQueue);
		m_workItemQueueThread = new WorkItemQueueThread(
			m_dptfManager, m_immediateQueue, m_deferredQueue, m_workItemQueueSemaphore, m_workItemStatistics);
	}
	catch (...)
	{
		deleteAllObjects();
		m_enqueueingEnabled = false;
		throw;
	}
}

WorkItemQueueManager::~WorkItemQueueManager(void)
{
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	deleteAllObjects();

	esifMutexHelper.unlock();
}

void WorkItemQueueManager::deleteAllObjects(void)
{
	// Do not acquire mutex for this function.
	DELETE_MEMORY_TC(m_workItemQueueThread);
	DELETE_MEMORY_TC(m_deferredQueue);
	DELETE_MEMORY_TC(m_immediateQueue);
	DELETE_MEMORY_TC(m_workItemQueueSemaphore);
	DELETE_MEMORY_TC(m_workItemStatistics);
}

void WorkItemQueueManager::disableAndEmptyAllQueues(void)
{
	// This has to be atomic while holding the lock.  So, both items (disable and empty) are within the same function.

	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	m_enqueueingEnabled = false;
	m_immediateQueue->makeEmtpy();
	m_deferredQueue->makeEmtpy();

	esifMutexHelper.unlock();
}

void WorkItemQueueManager::enqueueImmediateWorkItemAndReturn(std::shared_ptr<WorkItemInterface> workItem)
{
	const FrameworkEventData event = (*FrameworkEventInfo::instance())[workItem->getFrameworkEventType()];
	enqueueImmediateWorkItemAndReturn(workItem, event.priority);
}

void WorkItemQueueManager::enqueueImmediateWorkItemAndReturn(
	std::shared_ptr<WorkItemInterface> workItem,
	UIntN priority)
{
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	if (canEnqueueImmediateWorkItem(workItem))
	{
		auto immediateWorkItem = std::make_shared<ImmediateWorkItem>(workItem, priority);
		m_immediateQueue->enqueue(immediateWorkItem);
	}
	else
	{
		throw dptf_exception("Failed to enqueue work item.  Enqueueing has been disabled.");
	}

	esifMutexHelper.unlock();
}

void WorkItemQueueManager::enqueueImmediateWorkItemAndWait(std::shared_ptr<WorkItemInterface> workItem)
{
	const FrameworkEventData event = (*FrameworkEventInfo::instance())[workItem->getFrameworkEventType()];
	enqueueImmediateWorkItemAndWait(workItem, event.priority);
}

void WorkItemQueueManager::enqueueImmediateWorkItemAndWait(std::shared_ptr<WorkItemInterface> workItem, UIntN priority)
{
	if (isWorkItemThread() == true)
	{
		// This is in place to prevent a deadlock.  Keep in mind that we run a single thread to process work items.
		// There are conditions where a work item is running (on a work item thread) and it submits another work item
		// and waits for the return.  In that case we have an automatic deadlock without this special processing
		// in place.  When this happens we just treat it like a function call and execute the work item directly
		// and return.  Without this in place the work item would just sit in the queue and never execute since
		// the thread is being held by the currently running work item.
		workItem->execute();
	}
	else
	{
		EsifSemaphore semaphore;

		EsifMutexHelper esifMutexHelper(&m_mutex);
		esifMutexHelper.lock();

		if (canEnqueueImmediateWorkItem(workItem))
		{
			auto immediateWorkItem = std::make_shared<ImmediateWorkItem>(workItem, priority);
			immediateWorkItem->signalAtCompletion(&semaphore);
			m_immediateQueue->enqueue(immediateWorkItem);
		}
		else
		{
			throw dptf_exception("Failed to enqueue work item.  Enqueueing has been disabled.");
		}

		esifMutexHelper.unlock();

		semaphore.wait();
	}
}

void WorkItemQueueManager::enqueueDeferredWorkItem(
	std::shared_ptr<WorkItemInterface> workItem,
	const TimeSpan& timeUntilExecution)
{
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	if (m_enqueueingEnabled == true)
	{
		auto deferredWorkItem = std::make_shared<DeferredWorkItem>(workItem, timeUntilExecution);
		m_deferredQueue->enqueue(deferredWorkItem);
	}
	else
	{
		throw dptf_exception("Failed to enqueue work item.  Enqueueing has been disabled.");
	}

	esifMutexHelper.unlock();
}

UIntN WorkItemQueueManager::removeIfMatches(const WorkItemMatchCriteria& matchCriteria)
{
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	UIntN numRemovedImmediate = m_immediateQueue->removeIfMatches(matchCriteria);
	UIntN numRemovedDeferred = m_deferredQueue->removeIfMatches(matchCriteria);

	esifMutexHelper.unlock();

	UIntN numRemoved = numRemovedImmediate + numRemovedDeferred;

	if (numRemoved > 0)
	{
		MANAGER_LOG_MESSAGE_DEBUG({
			ManagerMessage message = ManagerMessage(
				m_dptfManager, _file, _line, _function, "One or more work items have been removed from the queues.");
			message.addMessage("Immediate Queue removed", numRemovedImmediate);
			message.addMessage("Deferred Queue removed", numRemovedDeferred);
			return message;
		});
	}
	return numRemoved;
}

Bool WorkItemQueueManager::isWorkItemThread()
{
	Bool isWorkItemThread = false;

	try
	{
		EsifThreadId currentThreadId;
		EsifThreadId workItemQueueThreadId = m_workItemQueueThread->getWorkItemQueueThreadId();
		isWorkItemThread = (currentThreadId == workItemQueueThreadId);
	}
	catch (...)
	{
	}

	return isWorkItemThread;
}

std::shared_ptr<XmlNode> WorkItemQueueManager::getStatusAsXml(void)
{
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	auto root = XmlNode::createRoot();
	root->addChild(XmlNode::createComment("format_id=C5-61-4D-E9-30-80-4D-B5-98-1A-D1-D1-67-DD-4C-D7"));

	auto workItemQueueManagerStatus = XmlNode::createWrapperElement("work_item_queue_manager_status");
	root->addChild(workItemQueueManagerStatus);

	workItemQueueManagerStatus->addChild(m_immediateQueue->getXml());
	workItemQueueManagerStatus->addChild(m_deferredQueue->getXml());
	workItemQueueManagerStatus->addChild(m_workItemStatistics->getXml());

	esifMutexHelper.unlock();

	return root;
}

std::shared_ptr<XmlNode> WorkItemQueueManager::getDiagnosticsAsXml(void)
{
	auto root = XmlNode::createRoot();
	return root;
}

Bool WorkItemQueueManager::canEnqueueImmediateWorkItem(std::shared_ptr<WorkItemInterface> workItem) const
{
	return (
		(m_enqueueingEnabled == true) || (workItem->getFrameworkEventType() == FrameworkEvent::PolicyDestroy)
		|| (workItem->getFrameworkEventType() == FrameworkEvent::ParticipantDestroy));
}

EsifServicesInterface* WorkItemQueueManager::getEsifServices() const
{
	return m_dptfManager->getEsifServices();
}
