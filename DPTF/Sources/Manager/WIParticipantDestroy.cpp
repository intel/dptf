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

#include "WIParticipantDestroy.h"
#include "DptfManager.h"
#include "WorkItemQueueManagerInterface.h"
#include "PolicyManagerInterface.h"
#include "ParticipantManagerInterface.h"
#include "WorkItemMatchCriteria.h"
#include "EsifServicesInterface.h"

WIParticipantDestroy::WIParticipantDestroy(DptfManagerInterface* dptfManager, UIntN participantIndex)
	: ParticipantWorkItem(dptfManager, FrameworkEvent::ParticipantDestroy, participantIndex)
{
}

WIParticipantDestroy::~WIParticipantDestroy(void)
{
}

void WIParticipantDestroy::onExecute(void)
{
	writeParticipantWorkItemStartingInfoMessage();

	// Call unbind participant for each policy before we actually destroy the participant.

	auto policyManager = getPolicyManager();
	auto policyIndexes = policyManager->getPolicyIndexes();

	for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
	{
		try
		{
			auto policy = policyManager->getPolicyPtr(*i);
			policy->unbindParticipant(getParticipantIndex());
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeParticipantWorkItemErrorMessagePolicy(ex, "Policy::unbindParticipant", *i);
		}
	}

	// Remove any work items from the queues that were submitted for this participant

	try
	{
		WorkItemMatchCriteria workItemMatchCriteria;
		workItemMatchCriteria.addParticipantIndexToMatchList(getParticipantIndex());
		getDptfManager()->getWorkItemQueueManager()->removeIfMatches(workItemMatchCriteria);
	}
	catch (std::exception& ex)
	{
		writeParticipantWorkItemErrorMessage(ex, "WorkItemQueueManager::removeIfMatches");
	}

	// Now let the participant manager destroy the participant

	try
	{
		getParticipantManager()->destroyParticipant(getParticipantIndex());
	}
	catch (std::exception& ex)
	{
		writeParticipantWorkItemErrorMessage(ex, "ParticipantManager::destroyParticipant");
	}
}
