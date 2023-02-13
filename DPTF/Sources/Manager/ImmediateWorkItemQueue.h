/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
#include "WorkItemQueueInterface.h"
#include "ImmediateWorkItem.h"
#include "EsifMutex.h"
#include "EsifSemaphore.h"

class ImmediateWorkItemQueue : public WorkItemQueueInterface
{
public:
	ImmediateWorkItemQueue(EsifSemaphore* workItemQueueSemaphore);
	virtual ~ImmediateWorkItemQueue(void);

	void enqueue(std::shared_ptr<ImmediateWorkItem> newWorkItem);
	std::shared_ptr<ImmediateWorkItem> dequeue(void);

	// implement WorkItemQueueInterface
	virtual void makeEmtpy(void) override final;
	virtual UInt64 getCount(void) const override final;
	virtual UInt64 getMaxCount(void) const override final;
	virtual UIntN removeIfMatches(const WorkItemMatchCriteria& matchCriteria) override final;
	virtual std::shared_ptr<XmlNode> getXml(void) const override final;

private:
	// hide the copy constructor and assignment operator.
	ImmediateWorkItemQueue(const ImmediateWorkItemQueue& rhs);
	ImmediateWorkItemQueue& operator=(const ImmediateWorkItemQueue& rhs);

	std::list<std::shared_ptr<ImmediateWorkItem>> m_queue;
	UInt64 m_maxCount; // stores the maximum number of items in the queue at any one time
	mutable EsifMutex m_mutex;
	EsifSemaphore* m_workItemQueueSemaphore;

	void throwIfDuplicateThermalThresholdCrossedEvent(std::shared_ptr<ImmediateWorkItem> newWorkItem);
	void insertSortedByPriority(std::shared_ptr<ImmediateWorkItem> newWorkItem);
	void updateMaxCount(void);
};
