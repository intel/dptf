/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "ParticipantManagerInterface.h"
#include "UniqueIdGenerator.h"
#include "EsifServices.h"

WorkItem::WorkItem(DptfManagerInterface* dptfManager, FrameworkEvent::Type frameworkEventType) :
    m_dptfManager(dptfManager),
    m_policyManager(dptfManager->getPolicyManager()),
    m_participantManager(dptfManager->getParticipantManager()),
    m_esifServices(dptfManager->getEsifServices()),
    m_uniqueId(UniqueIdGenerator::instance()->getNextId()),
    m_frameworkEventType(frameworkEventType),
    m_workItemCreationTime(EsifTime().getTimeStamp()),
    m_workItemExecutionStartTime(TimeSpan::createFromMilliseconds(0)),
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

DptfManagerInterface* WorkItem::getDptfManager(void) const
{
    return m_dptfManager;
}

PolicyManager* WorkItem::getPolicyManager(void) const
{
    return m_policyManager;
}

ParticipantManagerInterface* WorkItem::getParticipantManager(void) const
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

const TimeSpan& WorkItem::getWorkItemCreationTime(void) const
{
    return m_workItemCreationTime;
}

void WorkItem::setWorkItemExecutionStartTime(void)
{
    m_workItemExecutionStartTime = EsifTime().getTimeStamp();
}

const TimeSpan& WorkItem::getWorkItemExecutionStartTime(void) const
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

void WorkItem::writeWorkItemStartingInfoMessage() const
{
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Starting execution of work item.");
    message.setFrameworkEvent(getFrameworkEventType());
    getEsifServices()->writeMessageInfo(message);
}

void WorkItem::writeWorkItemWarningMessage(const std::exception& ex, const std::string& functionName) const
{
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item");
    message.setFrameworkEvent(getFrameworkEventType());
    message.setExceptionCaught(functionName, ex.what());
    getEsifServices()->writeMessageWarning(message);
}

void WorkItem::writeWorkItemErrorMessage(const std::exception& ex, const std::string& functionName) const
{
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item");
    message.setFrameworkEvent(getFrameworkEventType());
    message.setExceptionCaught(functionName, ex.what());
    getEsifServices()->writeMessageError(message);
}

void WorkItem::writeWorkItemErrorMessage(const std::exception& ex, const std::string& functionName, const std::string& messageKey, const std::string& messageValue) const
{
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item");
    message.setFrameworkEvent(getFrameworkEventType());
    message.setExceptionCaught(functionName, ex.what());
    message.addMessage(messageKey, messageValue);
    getEsifServices()->writeMessageError(message);
}

void WorkItem::writeWorkItemErrorMessagePolicy(const std::exception& ex, const std::string& functionName, UIntN policyIndex) const
{
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item");
    message.setFrameworkEvent(getFrameworkEventType());
    message.setExceptionCaught(functionName, ex.what());
    message.setPolicyIndex(policyIndex);
    getEsifServices()->writeMessageError(message);
}

void WorkItem::writeWorkItemErrorMessageParticipant(const std::exception& ex, const std::string& functionName, UIntN participantIndex) const
{
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item");
    message.setFrameworkEvent(getFrameworkEventType());
    message.setExceptionCaught(functionName, ex.what());
    message.setParticipantIndex(participantIndex);
    getEsifServices()->writeMessageError(message);
}
