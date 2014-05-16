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

#include "ImmediateWorkItem.h"
#include "WorkItem.h"

ImmediateWorkItem::ImmediateWorkItem(WorkItemInterface* workItem, UIntN priority) :
    m_workItem(workItem), m_priority(priority)
{
}

ImmediateWorkItem::~ImmediateWorkItem(void)
{
    DELETE_MEMORY_TC(m_workItem);
}

UInt64 ImmediateWorkItem::getUniqueId(void) const
{
    return m_workItem->getUniqueId();
}

FrameworkEvent::Type ImmediateWorkItem::getFrameworkEventType(void) const
{
    return m_workItem->getFrameworkEventType();
}

EsifTime ImmediateWorkItem::getWorkItemCreationTime(void) const
{
    return m_workItem->getWorkItemCreationTime();
}

void ImmediateWorkItem::setWorkItemExecutionStartTime(void)
{
    m_workItem->setWorkItemExecutionStartTime();
}

EsifTime ImmediateWorkItem::getWorkItemExecutionStartTime(void) const
{
    return m_workItem->getWorkItemExecutionStartTime();
}

void ImmediateWorkItem::signalAtCompletion(EsifSemaphore* semaphore)
{
    return m_workItem->signalAtCompletion(semaphore);
}

Bool ImmediateWorkItem::matches(const WorkItemMatchCriteria& matchCriteria) const
{
    return m_workItem->matches(matchCriteria);
}

std::string ImmediateWorkItem::toXml(void) const
{
    return m_workItem->toXml();
}

void ImmediateWorkItem::execute(void)
{
    m_workItem->execute();
}

WorkItemInterface* ImmediateWorkItem::getWorkItem(void) const
{
    return m_workItem;
}

UIntN ImmediateWorkItem::getPriority(void) const
{
    return m_priority;
}