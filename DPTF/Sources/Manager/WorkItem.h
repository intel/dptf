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
#include "WorkItemInterface.h"
#include "EsifSemaphore.h"
#include "ManagerMessage.h"
#include "EsifTime.h"
#include "DptfManagerInterface.h"

class PolicyManagerInterface;
class ParticipantManagerInterface;
class EsifServicesInterface;

class WorkItem : public WorkItemInterface
{
public:
	WorkItem(DptfManagerInterface* dptfManager, FrameworkEvent::Type frameworkEventType);

	// in the destructor we signal the semaphore if provided
	virtual ~WorkItem(void);

	virtual void execute(void) override;
	virtual void signal(void) override;

	DptfManagerInterface* getDptfManager(void) const;
	PolicyManagerInterface* getPolicyManager(void) const;
	ParticipantManagerInterface* getParticipantManager(void) const;
	EsifServicesInterface* getEsifServices(void) const;

	// the following are implemented in the WorkItem class and *cannot* be overridden
	virtual UInt64 getUniqueId(void) const override final;
	virtual FrameworkEvent::Type getFrameworkEventType(void) const override final;
	virtual const TimeSpan& getWorkItemCreationTime(void) const override final;
	virtual void setWorkItemExecutionStartTime(void) override final;
	virtual const TimeSpan& getWorkItemExecutionStartTime(void) const override final;
	virtual void signalAtCompletion(EsifSemaphore* semaphore) override final;

	// the following are implemented in the WorkItem class and *can* be overridden
	virtual Bool matches(const WorkItemMatchCriteria& matchCriteria) const override;
	virtual std::string toXml(void) const override;

protected:
	void writeWorkItemStartingInfoMessage() const;
	void writeWorkItemWarningMessage(const std::exception& ex, const std::string& functionName) const;
	void writeWorkItemWarningMessage(
		const std::exception& ex,
		const std::string& functionName,
		const std::string& messageKey,
		const std::string& messageValue) const;
	void writeWorkItemErrorMessage(const std::exception& ex, const std::string& functionName) const;
	void writeWorkItemErrorMessage(
		const std::exception& ex,
		const std::string& functionName,
		const std::string& messageKey,
		const std::string& messageValue) const;
	void writeWorkItemErrorMessagePolicy(const std::exception& ex, const std::string& functionName, UIntN policyIndex)
		const;
	void writeWorkItemWarningMessagePolicy(const std::exception& ex, const std::string& functionName, UIntN policyIndex)
		const;
	void writeWorkItemErrorMessageParticipant(
		const std::exception& ex,
		const std::string& functionName,
		UIntN participantIndex) const;

	virtual void onExecute(void) = 0;

private:
	// hide the copy constructor and assignment operator.
	WorkItem(const WorkItem& rhs);
	WorkItem& operator=(const WorkItem& rhs);

	DptfManagerInterface* m_dptfManager;
	PolicyManagerInterface* m_policyManager;
	ParticipantManagerInterface* m_participantManager;
	EsifServicesInterface* m_esifServices;

	const UInt64 m_uniqueId;
	const FrameworkEvent::Type m_frameworkEventType;
	TimeSpan m_workItemCreationTime;
	TimeSpan m_workItemExecutionStartTime;

	// The creator of the work item may need to return synchronously to the caller.  In this case a semaphore
	// is passed in to the work item and when the destructor is executed it will signal the semaphore.
	EsifSemaphore* m_completionSemaphore;
};
