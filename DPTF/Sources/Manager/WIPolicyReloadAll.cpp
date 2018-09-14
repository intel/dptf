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

#include "WIPolicyReloadAll.h"
#include "ParticipantManagerInterface.h"
#include "Participant.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"

WIPolicyReloadAll::WIPolicyReloadAll(DptfManagerInterface* dptfManager)
	: WorkItem(dptfManager, FrameworkEvent::PolicyCreate)
{
}

WIPolicyReloadAll::~WIPolicyReloadAll(void)
{
}

void WIPolicyReloadAll::execute(void)
{
	writeWorkItemStartingInfoMessage();

	// Iterate through the list of policies and unbind all participant and domains from them

	DptfManagerInterface* dptfManager = getDptfManager();
	auto policyManager = getPolicyManager();
	auto participantIndexList = getParticipantManager()->getParticipantIndexes();

	// Unbind every participant and domain from every policy
	for (auto participantIndex = participantIndexList.begin(); participantIndex != participantIndexList.end();
		++participantIndex)
	{
		dptfManager->unbindDomainsFromPolicies(*participantIndex);
		dptfManager->unbindParticipantFromPolicies(*participantIndex);
	}

	// No try-catch here because destroyAllPolicies' calling tree catches all exceptions.
	policyManager->destroyAllPolicies();

	try
	{
		policyManager->getSupportedPolicyList()->update();
		policyManager->createAllPolicies(dptfManager->getDptfPolicyDirectoryPath());
	}
	catch (dptf_exception& ex)
	{
		writeWorkItemErrorMessage(ex, "policyManager::reloadAllPolicies");
	}

	// Bind every participant and domain to every policy
	for (auto participantIndex = participantIndexList.begin(); participantIndex != participantIndexList.end();
		++participantIndex)
	{
		dptfManager->bindParticipantToPolicies(*participantIndex);
		dptfManager->bindDomainsToPolicies(*participantIndex);
	}
}
