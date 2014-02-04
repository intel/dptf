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

#include "DeferredWorkItem.h"
#include "WorkItem.h"

DeferredWorkItem::DeferredWorkItem(WorkItemInterface* workItem, UInt64 numMilliSecUntilExecution) :
    m_workItem(workItem)
{
    m_deferredProcessingTime = EsifTime() + numMilliSecUntilExecution;
}

DeferredWorkItem::~DeferredWorkItem(void)
{
    DELETE_MEMORY_TC(m_workItem);
}

UInt64 DeferredWorkItem::getUniqueId(void) const
{
    return m_workItem->getUniqueId();
}

FrameworkEvent::Type DeferredWorkItem::getFrameworkEventType(void) const
{
    return m_workItem->getFrameworkEventType();
}

EsifTime DeferredWorkItem::getWorkItemCreationTime(void) const
{
    return m_workItem->getWorkItemCreationTime();
}

void DeferredWorkItem::setWorkItemExecutionStartTime(void)
{
    m_workItem->setWorkItemExecutionStartTime();
}

EsifTime DeferredWorkItem::getWorkItemExecutionStartTime(void) const
{
    return m_workItem->getWorkItemExecutionStartTime();
}

void DeferredWorkItem::signalAtCompletion(EsifSemaphore* semaphore)
{
    return m_workItem->signalAtCompletion(semaphore);
}

Bool DeferredWorkItem::matches(const WorkItemMatchCriteria& matchCriteria) const
{
    return m_workItem->matches(matchCriteria);
}

std::string DeferredWorkItem::toXml(void) const
{
    return m_workItem->toXml();
}

void DeferredWorkItem::execute(void)
{
    m_workItem->execute();
}

WorkItemInterface* DeferredWorkItem::getWorkItem(void) const
{
    return m_workItem;
}

EsifTime DeferredWorkItem::getDeferredProcessingTime(void) const
{
    return m_deferredProcessingTime;
}