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

#include "WIDomainAC10msPercentageOverloadChanged.h"
#include "Participant.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"

WIDomainAC10msPercentageOverloadChanged::WIDomainAC10msPercentageOverloadChanged(
	DptfManagerInterface* dptfManager,
	UIntN participantIndex,
	UIntN domainIndex)
	: DomainWorkItem(
		dptfManager,
		FrameworkEvent::Type::DomainAC10msPercentageOverloadChanged,
		participantIndex,
		domainIndex)
{
}

WIDomainAC10msPercentageOverloadChanged::~WIDomainAC10msPercentageOverloadChanged(void)
{
}

void WIDomainAC10msPercentageOverloadChanged::onExecute(void)
{
	writeDomainWorkItemStartingInfoMessage();

	try
	{
		getParticipantPtr()->domainAC10msPercentageOverloadChanged();
	}
	catch (participant_index_invalid& ex)
	{
		writeDomainWorkItemWarningMessage(ex, "ParticipantManager::getParticipantPtr");
	}
	catch (std::exception& ex)
	{
		writeDomainWorkItemErrorMessage(ex, "Participant::domainAC10msPercentageOverloadChanged");
	}

	auto policyManager = getPolicyManager();
	auto policyIndexes = policyManager->getPolicyIndexes();

	for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
	{
		try
		{
			auto policy = policyManager->getPolicyPtr(*i);
			policy->executeDomainAC10msPercentageOverloadChanged(getParticipantIndex());
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeDomainWorkItemErrorMessagePolicy(ex, "Policy::executeDomainAC10msPercentageOverloadChanged", *i);
		}
	}
}
