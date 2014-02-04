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

#include "Dptf.h"
#include "WorkItemInterface.h"
#include "EsifSemaphore.h"
#include "ManagerMessage.h"
#include "EsifTime.h"

class DptfManager;
class PolicyManager;
class ParticipantManager;
class EsifServices;

class WorkItem : public WorkItemInterface
{
public:

    WorkItem(DptfManager* dptfManager, FrameworkEvent::Type frameworkEventType);

    // in the destructor we signal the semaphore if provided
    virtual ~WorkItem(void);

    DptfManager* getDptfManager(void) const;
    PolicyManager* getPolicyManager(void) const;
    ParticipantManager* getParticipantManager(void) const;
    EsifServices* getEsifServices(void) const;

    // the following are implemented in the WorkItem class and *cannot* be overridden
    virtual UInt64 getUniqueId(void) const override final;
    virtual FrameworkEvent::Type getFrameworkEventType(void) const override final;
    virtual EsifTime getWorkItemCreationTime(void) const override final;
    virtual void setWorkItemExecutionStartTime(void) override final;
    virtual EsifTime getWorkItemExecutionStartTime(void) const override final;
    virtual void signalAtCompletion(EsifSemaphore* semaphore) override final;

    // the following are implemented in the WorkItem class and *can* be overridden
    virtual Bool matches(const WorkItemMatchCriteria& matchCriteria) const override;
    virtual std::string toXml(void) const override;

private:

    // hide the copy constructor and assignment operator.
    WorkItem(const WorkItem& rhs);
    WorkItem& operator=(const WorkItem& rhs);

    DptfManager* m_dptfManager;
    PolicyManager* m_policyManager;
    ParticipantManager* m_participantManager;
    EsifServices* m_esifServices;

    const UInt64 m_uniqueId;
    const FrameworkEvent::Type m_frameworkEventType;
    EsifTime m_workItemCreationTime;
    EsifTime m_workItemExecutionStartTime;

    // The creator of the work item may need to return synchronously to the caller.  In this case a semaphore
    // is passed in to the work item and when the destructor is executed it will signal the semaphore.
    EsifSemaphore* m_completionSemaphore;
};

#define WriteWorkItemStartingDebugMessage() \
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Starting execution of work item."); \
    message.setFrameworkEvent(getFrameworkEventType()); \
    getEsifServices()->writeMessageDebug(message);

#define WriteWorkItemErrorMessage_Function(functionName) \
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item"); \
    message.setFrameworkEvent(getFrameworkEventType()); \
    message.setExceptionCaught(functionName, ex.what()); \
    getEsifServices()->writeMessageError(message);

#define WriteWorkItemErrorMessage_Function_Policy(functionName, policyIndex) \
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item"); \
    message.setFrameworkEvent(getFrameworkEventType()); \
    message.setExceptionCaught(functionName, ex.what()); \
    message.setPolicyIndex(policyIndex); \
    getEsifServices()->writeMessageError(message);

#define WriteWorkItemErrorMessage_Function_Participant(functionName, participantIndex) \
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item"); \
    message.setFrameworkEvent(getFrameworkEventType()); \
    message.setExceptionCaught(functionName, ex.what()); \
    message.setParticipantIndex(participantIndex); \
    getEsifServices()->writeMessageError(message);

#define WriteWorkItemErrorMessage_Function_MessageKey_MessageValue(functionName, messageKey, messageValue) \
    ManagerMessage message = ManagerMessage(getDptfManager(), FLF, "Unhandled exception caught during execution of work item"); \
    message.setFrameworkEvent(getFrameworkEventType()); \
    message.setExceptionCaught(functionName, ex.what()); \
    message.addMessage(messageKey, messageValue); \
    getEsifServices()->writeMessageError(message);