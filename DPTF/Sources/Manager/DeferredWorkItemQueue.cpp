/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "XmlNode.h"

DeferredWorkItemQueue::DeferredWorkItemQueue(
	EsifSemaphore* workItemQueueSemaphore,
	ImmediateWorkItemQueue* immediateWorkItemQueue)
	: m_queue()
	, m_maxCount(0)
	, m_workItemQueueSemaphore(workItemQueueSemaphore)
	, m_immediateQueue(immediateWorkItemQueue)
	, m_timer(TimerCallback, this)
{
}

DeferredWorkItemQueue::~DeferredWorkItemQueue(void)
{
	// makeEmpty is a public function and it will lock the mutex
	makeEmtpy();
}

void DeferredWorkItemQueue::enqueue(std::shared_ptr<DeferredWorkItem> newWorkItem)
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

std::shared_ptr<DeferredWorkItem> DeferredWorkItemQueue::dequeue(void)
{
	// Returns the first item in the queue if it the work item time is >= the current time.
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();
	auto firstReadyWorkItem = getFirstReadyWorkItemFromQueue();
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
		auto currentWorkItem = m_queue.front();
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

	UIntN numRemoved = 0;

	auto it = m_queue.begin();
	while (it != m_queue.end())
	{
		if ((*it)->matches(matchCriteria) == true)
		{
			(*it)->signal();
			it = m_queue.erase(it);
			numRemoved++;
		}
		else
		{
			it++;
		}
	}

	setTimer();

	esifMutexHelper.unlock();

	return numRemoved;
}

std::shared_ptr<XmlNode> DeferredWorkItemQueue::getXml(void) const
{
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	auto deferredQueueStastics = XmlNode::createWrapperElement("deferred_queue_statistics");
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

	if (m_queue.empty() == false)
	{
		auto firstWorkItem = m_queue.front();
		auto firstWorkItemTime = firstWorkItem->getDeferredProcessingTime();

		m_timer.startTimer(firstWorkItemTime);
	}
}

std::shared_ptr<DeferredWorkItem> DeferredWorkItemQueue::getFirstReadyWorkItemFromQueue(void)
{
	std::shared_ptr<DeferredWorkItem> firstReadyWorkItem;
	if (m_queue.empty() == false)
	{
		auto currentTime = EsifTime().getTimeStamp();
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

void DeferredWorkItemQueue::insertSortedByDeferredProcessingTime(std::shared_ptr<DeferredWorkItem> newWorkItem)
{
	auto it = m_queue.begin();
	for (; it != m_queue.end(); it++)
	{
		auto currentWorkItem = *it;
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

void DeferredWorkItemQueue::timerCallback(void)
{
	// The WorkItemQueueManager is not locked while this executes.
	// Move the ready items to the immediate queue and signal the semaphore.

	auto readyWorkItem = dequeue();
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	while (readyWorkItem != nullptr)
	{
		try
		{
			auto immediateWorkItem = std::make_shared<ImmediateWorkItem>(readyWorkItem, 0);
			m_immediateQueue->enqueue(immediateWorkItem);
		}
		catch (...)
		{
		}
		readyWorkItem = dequeue();
	}

	esifMutexHelper.unlock();
	m_workItemQueueSemaphore->signal();
}

void TimerCallback(void* context_ptr)
{
	DeferredWorkItemQueue* deferredWorkItemQueue = static_cast<DeferredWorkItemQueue*>(context_ptr);
	deferredWorkItemQueue->timerCallback();
}
