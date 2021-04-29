/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "ImmediateWorkItemQueue.h"
#include "EsifMutexHelper.h"
#include "ParticipantWorkItem.h"
#include "XmlNode.h"
using namespace std;

ImmediateWorkItemQueue::ImmediateWorkItemQueue(EsifSemaphore* workItemQueueSemaphore)
	: m_queue()
	, m_maxCount(0)
	, m_workItemQueueSemaphore(workItemQueueSemaphore)
{
}

ImmediateWorkItemQueue::~ImmediateWorkItemQueue(void)
{
	// makeEmpty is a public function and it will lock the mutex
	makeEmtpy();
}

void ImmediateWorkItemQueue::enqueue(std::shared_ptr<ImmediateWorkItem> newWorkItem)
{
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	// In the case of an interrupt storm we need to make sure we don't enqueue a temperature threshold crossed
	// event if the same event is already in the queue.
	throwIfDuplicateThermalThresholdCrossedEvent(newWorkItem);

	insertSortedByPriority(newWorkItem);
	updateMaxCount();

	esifMutexHelper.unlock();
}

std::shared_ptr<ImmediateWorkItem> ImmediateWorkItemQueue::dequeue(void)
{
	shared_ptr<ImmediateWorkItem> firstItemInQueue;

	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	if (m_queue.empty() == false)
	{
		firstItemInQueue = m_queue.front();
		m_queue.pop_front();
	}

	esifMutexHelper.unlock();

	return firstItemInQueue;
}

void ImmediateWorkItemQueue::makeEmtpy(void)
{
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	while (m_queue.empty() == false)
	{
		m_queue.pop_front();
	}

	esifMutexHelper.unlock();
}

UInt64 ImmediateWorkItemQueue::getCount(void) const
{
	UInt64 count;
	EsifMutexHelper esifMutexHelper(&m_mutex);

	esifMutexHelper.lock();
	count = m_queue.size();
	esifMutexHelper.unlock();

	return count;
}

UInt64 ImmediateWorkItemQueue::getMaxCount(void) const
{
	UInt64 maxCount;
	EsifMutexHelper esifMutexHelper(&m_mutex);

	esifMutexHelper.lock();
	maxCount = m_maxCount;
	esifMutexHelper.unlock();

	return maxCount;
}

UIntN ImmediateWorkItemQueue::removeIfMatches(const WorkItemMatchCriteria& matchCriteria)
{
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	UIntN numRemoved = 0;

	auto it = m_queue.begin();
	while (it != m_queue.end())
	{
		if ((*it)->matches(matchCriteria) == true)
		{
			(*it)->getWorkItem()->signal();
			it = m_queue.erase(it);
			numRemoved++;
		}
		else
		{
			it++;
		}
	}

	esifMutexHelper.unlock();

	return numRemoved;
}

std::shared_ptr<XmlNode> ImmediateWorkItemQueue::getXml(void) const
{
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	auto immediateQueueStastics = XmlNode::createWrapperElement("immediate_queue_statistics");
	immediateQueueStastics->addChild(XmlNode::createDataElement("current_count", std::to_string(m_queue.size())));
	immediateQueueStastics->addChild(XmlNode::createDataElement("max_count", std::to_string(m_maxCount)));

	esifMutexHelper.unlock();

	return immediateQueueStastics;
}

//
// The following *private* methods do not need to lock the mutex.  The caller is responsible for
// locking and unlocking.
//

void ImmediateWorkItemQueue::throwIfDuplicateThermalThresholdCrossedEvent(
	std::shared_ptr<ImmediateWorkItem> newWorkItem)
{
	// FIXME: need to update once domain support has been added.  In that case we should
	// check the domain field as well.

	if (newWorkItem->getFrameworkEventType() == FrameworkEvent::DomainTemperatureThresholdCrossed)
	{
		auto workItem = newWorkItem->getWorkItem();
		auto participantWorkItem = static_pointer_cast<ParticipantWorkItem>(workItem);

		WorkItemMatchCriteria matchCriteria;
		matchCriteria.addFrameworkEventTypeToMatchList(participantWorkItem->getFrameworkEventType());
		matchCriteria.addParticipantIndexToMatchList(participantWorkItem->getParticipantIndex());

		for (auto it = m_queue.begin(); it != m_queue.end(); it++)
		{
			auto currentWorkItem = *it;
			Bool foundDuplicate = currentWorkItem->matches(matchCriteria);
			if (foundDuplicate == true)
			{
				throw duplicate_work_item(
					"Attempted to insert duplicate thermal threshold crossed event into immediate queue.");
			}
		}
	}
}

void ImmediateWorkItemQueue::insertSortedByPriority(std::shared_ptr<ImmediateWorkItem> newWorkItem)
{
	// To improve performance we check for the common cases first and skip the queue
	// iteration when we can.
	if (m_queue.empty() == true || newWorkItem->getPriority() == 0)
	{
		m_queue.push_back(newWorkItem);
	}
	else
	{
		auto it = m_queue.begin();

		for (; it != m_queue.end(); it++)
		{
			auto currentWorkItem = *it;
			if (newWorkItem->getPriority() > currentWorkItem->getPriority())
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

	m_workItemQueueSemaphore->signal();
}

void ImmediateWorkItemQueue::updateMaxCount()
{
	if (m_queue.size() > m_maxCount)
	{
		m_maxCount = m_queue.size();
	}
}
