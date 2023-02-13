/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "WIDomainCreate.h"
#include "Participant.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"

WIDomainCreate::WIDomainCreate(
	DptfManagerInterface* dptfManager,
	UIntN participantIndex,
	UIntN domainIndex,
	const AppDomainDataPtr domainDataPtr,
	Bool domainEnabled,
	Bool* domainCreated)
	: DomainWorkItem(dptfManager, FrameworkEvent::DomainCreate, participantIndex, domainIndex)
	, m_domainDataPtr(domainDataPtr)
	, m_domainEnabled(domainEnabled)
	, m_domainCreated(domainCreated)
{
}

WIDomainCreate::~WIDomainCreate(void)
{
}

void WIDomainCreate::onExecute(void)
{
	writeDomainWorkItemStartingInfoMessage();

	Bool domainCreated = false;

	try
	{
		getParticipantPtr()->createDomain(getDomainIndex(), m_domainDataPtr, m_domainEnabled);
		domainCreated = true;
	}
	catch (participant_index_invalid& ex)
	{
		writeDomainWorkItemWarningMessage(ex, "ParticipantManager::getParticipantPtr");
	}
	catch (std::exception& ex)
	{
		writeDomainWorkItemErrorMessage(ex, "Participant::createDomain");
	}

	*m_domainCreated = domainCreated;

	if (domainCreated == true)
	{
		//
		// Iterate through the list of policies and let them know about the new domain
		//

		auto policyManager = getPolicyManager();
		auto policyIndexes = policyManager->getPolicyIndexes();

		for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
		{
			try
			{
				auto policy = policyManager->getPolicyPtr(*i);
				policy->bindDomain(getParticipantIndex(), getDomainIndex());
			}
			catch (const policy_index_invalid&)
			{
				// do nothing.  No item in the policy list at this index.
			}
			catch (const dptf_exception& ex)
			{
				writeDomainWorkItemWarningMessage(ex, "Policy::bindDomain");
			}
			catch (const std::exception& ex)
			{
				writeDomainWorkItemErrorMessagePolicy(ex, "Policy::bindDomain", *i);
			}
		}
	}
}
