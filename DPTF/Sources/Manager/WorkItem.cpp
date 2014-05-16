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

#include "WorkItem.h"
#include "DptfManager.h"
#include "PolicyManager.h"
#include "ParticipantManager.h"
#include "UniqueIdGenerator.h"

WorkItem::WorkItem(DptfManager* dptfManager, FrameworkEvent::Type frameworkEventType) :
    m_dptfManager(dptfManager),
    m_policyManager(dptfManager->getPolicyManager()),
    m_participantManager(dptfManager->getParticipantManager()),
    m_esifServices(dptfManager->getEsifServices()),
    m_uniqueId(UniqueIdGenerator::instance()->getNextId()),
    m_frameworkEventType(frameworkEventType),
    m_workItemCreationTime(EsifTime()),
    m_workItemExecutionStartTime(EsifTime(0)),
    m_completionSemaphore(nullptr)
{
}

WorkItem::~WorkItem(void)
{
    // if needed, signal the completion event
    if (m_completionSemaphore != nullptr)
    {
        m_completionSemaphore->signal();
    }
}

DptfManager* WorkItem::getDptfManager(void) const
{
    return m_dptfManager;
}

PolicyManager* WorkItem::getPolicyManager(void) const
{
    return m_policyManager;
}

ParticipantManager* WorkItem::getParticipantManager(void) const
{
    return m_participantManager;
}

EsifServices* WorkItem::getEsifServices(void) const
{
    return m_esifServices;
}

UInt64 WorkItem::getUniqueId(void) const
{
    return m_uniqueId;
}

FrameworkEvent::Type WorkItem::getFrameworkEventType(void) const
{
    return m_frameworkEventType;
}

EsifTime WorkItem::getWorkItemCreationTime(void) const
{
    return m_workItemCreationTime;
}

void WorkItem::setWorkItemExecutionStartTime(void)
{
    m_workItemExecutionStartTime = EsifTime();
}

EsifTime WorkItem::getWorkItemExecutionStartTime(void) const
{
    return m_workItemExecutionStartTime;
}

void WorkItem::signalAtCompletion(EsifSemaphore* semaphore)
{
    m_completionSemaphore = semaphore;
}

Bool WorkItem::matches(const WorkItemMatchCriteria& matchCriteria) const
{
    return matchCriteria.testAgainstMatchList(getFrameworkEventType(), getUniqueId());
}

std::string WorkItem::toXml(void) const
{
    throw implement_me();
}