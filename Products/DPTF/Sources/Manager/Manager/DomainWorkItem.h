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

#pragma once

#include "ParticipantWorkItem.h"

class DomainWorkItem : public ParticipantWorkItem
{
public:

    DomainWorkItem(DptfManager* dptfManager, FrameworkEvent::Type frameworkEventType, UIntN participantIndex,
        UIntN domainIndex);
    virtual ~DomainWorkItem(void);

    // the following are implemented in the DomainWorkItem class and *can* be overridden
    virtual Bool matches(const WorkItemMatchCriteria& matchCriteria) const override;
    virtual std::string toXml(void) const override;

    // the following is not virtual
    UIntN getDomainIndex(void) const;

private:

    // hide the copy constructor and assignment operator.
    DomainWorkItem(const DomainWorkItem& rhs);
    DomainWorkItem& operator=(const DomainWorkItem& rhs);

    const UIntN m_domainIndex;
};

#define WriteDomainWorkItemStartingInfoMessage() \
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Starting execution of work item."); \
    message.setFrameworkEvent(getFrameworkEventType()); \
    message.setParticipantAndDomainIndex(getParticipantIndex(), getDomainIndex()); \
    getEsifServices()->writeMessageInfo(message);

#define WriteDomainWorkItemErrorMessage_Message(errorMessage) \
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, errorMessage); \
    message.setFrameworkEvent(getFrameworkEventType()); \
    message.setParticipantAndDomainIndex(getParticipantIndex(), getDomainIndex()); \
    getEsifServices()->writeMessageError(message);

#define WriteDomainWorkItemErrorMessage_Function(functionName) \
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item"); \
    message.setFrameworkEvent(getFrameworkEventType()); \
    message.setParticipantAndDomainIndex(getParticipantIndex(), getDomainIndex()); \
    message.setExceptionCaught(functionName, ex.what()); \
    getEsifServices()->writeMessageError(message);

#define WriteDomainWorkItemErrorMessage_Function_Policy(functionName, policyIndex) \
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item"); \
    message.setFrameworkEvent(getFrameworkEventType()); \
    message.setParticipantAndDomainIndex(getParticipantIndex(), getDomainIndex()); \
    message.setExceptionCaught(functionName, ex.what()); \
    message.setPolicyIndex(policyIndex); \
    getEsifServices()->writeMessageError(message);