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
#include "WorkItem.h"

class Participant;

class ParticipantWorkItem : public WorkItem
{
public:
	ParticipantWorkItem(
		DptfManagerInterface* dptfManager,
		FrameworkEvent::Type frameworkEventType,
		UIntN participantIndex);
	virtual ~ParticipantWorkItem(void);

	// the following are implemented in the ParticipantWorkItem class and *can* be overridden
	virtual Bool matches(const WorkItemMatchCriteria& matchCriteria) const override;
	virtual std::string toXml(void) const override;

	// the following is not virtual
	UIntN getParticipantIndex(void) const;
	Participant* getParticipantPtr(void) const;

protected:
	void writeParticipantWorkItemErrorMessage(const std::exception& ex, const std::string& functionName) const;
	void writeParticipantWorkItemWarningMessage(const std::exception& ex, const std::string& functionName) const;
	void writeParticipantWorkItemErrorMessagePolicy(
		const std::exception& ex,
		const std::string& functionName,
		UIntN policyIndex);
	void writeParticipantWorkItemWarningMessagePolicy(
		const std::exception& ex,
		const std::string& functionName,
		UIntN policyIndex);
	void writeParticipantWorkItemStartingInfoMessage() const;

private:
	// hide the copy constructor and assignment operator.
	ParticipantWorkItem(const ParticipantWorkItem& rhs);
	ParticipantWorkItem& operator=(const ParticipantWorkItem& rhs);

	const UIntN m_participantIndex;
};
