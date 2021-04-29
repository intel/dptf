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

#pragma once

#include "Dptf.h"
#include "WorkItemQueueInterface.h"
#include "DeferredWorkItem.h"
#include "EsifMutex.h"
#include "EsifSemaphore.h"
#include "EsifTime.h"
#include "EsifTimer.h"
#include "ImmediateWorkItemQueue.h"

class DeferredWorkItemQueue : public WorkItemQueueInterface
{
public:
	DeferredWorkItemQueue(EsifSemaphore* workItemQueueSemaphore, ImmediateWorkItemQueue* immediateWorkItemQueue);
	virtual ~DeferredWorkItemQueue(void);

	void enqueue(std::shared_ptr<DeferredWorkItem> newWorkItem);
	std::shared_ptr<DeferredWorkItem> dequeue(void);

	// implement WorkItemQueueInterface
	virtual void makeEmtpy(void) override final;
	virtual UInt64 getCount(void) const override final;
	virtual UInt64 getMaxCount(void) const override final;
	virtual UIntN removeIfMatches(const WorkItemMatchCriteria& matchCriteria) override final;
	virtual std::shared_ptr<XmlNode> getXml(void) const override final;

private:
	// hide the copy constructor and assignment operator.
	DeferredWorkItemQueue(const DeferredWorkItemQueue& rhs);
	DeferredWorkItemQueue& operator=(const DeferredWorkItemQueue& rhs);

	std::list<std::shared_ptr<DeferredWorkItem>> m_queue;
	UInt64 m_maxCount; // stores the maximum number of items in the queue at any one time
	mutable EsifMutex m_mutex;
	EsifSemaphore* m_workItemQueueSemaphore;
	ImmediateWorkItemQueue* m_immediateQueue;
	EsifTimer m_timer;

	void setTimer(void);
	std::shared_ptr<DeferredWorkItem> getFirstReadyWorkItemFromQueue(void);
	void insertSortedByDeferredProcessingTime(std::shared_ptr<DeferredWorkItem> newWorkItem);
	void updateMaxCount(void);

	// The timer will call a 'C' function which will need to forward the call to our private
	// timerCallback method.  We set it up as a friend so it has access.
	void timerCallback(void);
	friend void TimerCallback(void* context_ptr);
};

void TimerCallback(void* context_ptr);
