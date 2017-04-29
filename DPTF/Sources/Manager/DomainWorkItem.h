/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
#include "ParticipantWorkItem.h"

class DomainWorkItem : public ParticipantWorkItem
{
public:
	DomainWorkItem(
		DptfManagerInterface* dptfManager,
		FrameworkEvent::Type frameworkEventType,
		UIntN participantIndex,
		UIntN domainIndex);
	virtual ~DomainWorkItem(void);

	// the following are implemented in the DomainWorkItem class and *can* be overridden
	virtual Bool matches(const WorkItemMatchCriteria& matchCriteria) const override;
	virtual std::string toXml(void) const override;

	// the following is not virtual
	UIntN getDomainIndex(void) const;

protected:
	void writeDomainWorkItemErrorMessage(const std::exception& ex, const std::string& functionName) const;
	void writeDomainWorkItemErrorMessagePolicy(
		const std::exception& ex,
		const std::string& functionName,
		UIntN policyIndex) const;
	void writeDomainWorkItemErrorMessage(const std::string& errorMessage) const;
	void writeDomainWorkItemStartingInfoMessage() const;

private:
	// hide the copy constructor and assignment operator.
	DomainWorkItem(const DomainWorkItem& rhs);
	DomainWorkItem& operator=(const DomainWorkItem& rhs);

	const UIntN m_domainIndex;
};
