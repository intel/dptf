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

#include "DomainWorkItem.h"
#include "Participant.h"
#include "EsifServicesInterface.h"

DomainWorkItem::DomainWorkItem(DptfManagerInterface* dptfManager, FrameworkEvent::Type frameworkEventType,
    UIntN participantIndex, UIntN domainIndex) :
    ParticipantWorkItem(dptfManager, frameworkEventType, participantIndex),
    m_domainIndex(domainIndex)
{
}

DomainWorkItem::~DomainWorkItem(void)
{
}

Bool DomainWorkItem::matches(const WorkItemMatchCriteria& matchCriteria) const
{
    return matchCriteria.testAgainstMatchList(getFrameworkEventType(), getUniqueId(),
        getParticipantIndex(), getDomainIndex());
}

std::string DomainWorkItem::toXml(void) const
{
    throw implement_me();
}

UIntN DomainWorkItem::getDomainIndex(void) const
{
    return m_domainIndex;
}

void DomainWorkItem::writeDomainWorkItemErrorMessage(const std::exception & ex, const std::string & functionName) const
{
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item");
    message.setFrameworkEvent(getFrameworkEventType());
    message.setParticipantAndDomainIndex(getParticipantIndex(), getDomainIndex());
    message.setExceptionCaught(functionName, ex.what());
    getEsifServices()->writeMessageError(message);
}

void DomainWorkItem::writeDomainWorkItemErrorMessagePolicy(const std::exception& ex, const std::string& functionName, UIntN policyIndex) const
{
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item");
    message.setFrameworkEvent(getFrameworkEventType());
    message.setParticipantAndDomainIndex(getParticipantIndex(), getDomainIndex());
    message.setExceptionCaught(functionName, ex.what());
    message.setPolicyIndex(policyIndex);
    getEsifServices()->writeMessageError(message);
}

void DomainWorkItem::writeDomainWorkItemErrorMessage(const std::string& errorMessage) const
{
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, errorMessage);
    message.setFrameworkEvent(getFrameworkEventType());
    message.setParticipantAndDomainIndex(getParticipantIndex(), getDomainIndex());
    getEsifServices()->writeMessageError(message);
}

void DomainWorkItem::writeDomainWorkItemStartingInfoMessage() const
{
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Starting execution of work item.");
    message.setFrameworkEvent(getFrameworkEventType());
    message.setParticipantAndDomainIndex(getParticipantIndex(), getDomainIndex());
    getEsifServices()->writeMessageInfo(message);
}
