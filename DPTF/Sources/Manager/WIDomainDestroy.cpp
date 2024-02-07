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

#include "WIDomainDestroy.h"
#include "Participant.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"

WIDomainDestroy::WIDomainDestroy(DptfManagerInterface* dptfManager, UIntN participantIndex, UIntN domainIndex)
	: DomainWorkItem(dptfManager, FrameworkEvent::DomainDestroy, participantIndex, domainIndex)
{
}

WIDomainDestroy::~WIDomainDestroy(void)
{
}

void WIDomainDestroy::onExecute(void)
{
	writeDomainWorkItemStartingInfoMessage();

	if (getParticipantPtr()->isDomainValid(getDomainIndex()) == false)
	{
		writeDomainWorkItemErrorMessage("Received request to remove a domain that is invalid.");
	}
	else
	{
		auto policyManager = getPolicyManager();
		auto policyIndexes = policyManager->getPolicyIndexes();

		// Let each policy know that the domain is going away

		for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
		{
			try
			{
				auto policy = policyManager->getPolicyPtr(*i);
				policy->unbindDomain(getParticipantIndex(), getDomainIndex());
			}
			catch (policy_index_invalid&)
			{
				// do nothing.  No item in the policy list at this index.
			}
			catch (std::exception& ex)
			{
				writeDomainWorkItemErrorMessagePolicy(ex, "Policy::unbindDomain", *i);
			}
		}

		// Let the participant know to remove the domain

		try
		{
			getParticipantPtr()->destroyDomain(getDomainIndex());
		}
		catch (std::exception& ex)
		{
			writeDomainWorkItemErrorMessage(ex, "Participant::destroyDomain");
		}
	}
}
