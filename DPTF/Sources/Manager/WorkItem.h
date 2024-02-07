/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "DptfManagerInterface.h"

class PolicyManagerInterface;
class ParticipantManagerInterface;
class EsifServicesInterface;

class WorkItem : public WorkItemInterface
{
public:
	WorkItem(DptfManagerInterface* dptfManager, FrameworkEvent::Type frameworkEventType);
	WorkItem(const WorkItem& other) = delete;
	WorkItem(WorkItem&& other) noexcept = delete;
	WorkItem& operator=(const WorkItem& other) = delete;
	WorkItem& operator=(WorkItem&& other) noexcept = delete;
	// in the destructor we signal the semaphore if provided
	~WorkItem() override = default;

	void execute() override;
	void signal() override;

	DptfManagerInterface* getDptfManager() const;
	PolicyManagerInterface* getPolicyManager() const;
	ParticipantManagerInterface* getParticipantManager() const;
	EsifServicesInterface* getEsifServices() const;

	// the following are implemented in the WorkItem class and *cannot* be overridden
	UInt64 getUniqueId() const final;
	FrameworkEvent::Type getFrameworkEventType() const final;
	const TimeSpan& getWorkItemCreationTime() const final;
	void setWorkItemExecutionStartTime() final;
	const TimeSpan& getWorkItemExecutionStartTime() const final;
	void signalAtCompletion(EsifSemaphore* semaphore) final;

	// the following are implemented in the WorkItem class and *can* be overridden
	Bool matches(const WorkItemMatchCriteria& matchCriteria) const override;
	std::string toXml() const override;

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

	virtual void onExecute() = 0;

private:
	DptfManagerInterface* m_dptfManager;
	PolicyManagerInterface* m_policyManager;
	ParticipantManagerInterface* m_participantManager;
	EsifServicesInterface* m_esifServices;

	UInt64 m_uniqueId;
	FrameworkEvent::Type m_frameworkEventType;
	TimeSpan m_workItemCreationTime;
	TimeSpan m_workItemExecutionStartTime;

	// The creator of the work item may need to return synchronously to the caller.  In this case a semaphore
	// is passed in to the work item and when the destructor is executed it will signal the semaphore.
	EsifSemaphore* m_completionSemaphore;
};
