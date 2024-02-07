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

#include "WIDptfResume.h"
#include "PolicyManagerInterface.h"
#include "ParticipantManagerInterface.h"
#include "EsifServicesInterface.h"

WIDptfResume::WIDptfResume(DptfManagerInterface* dptfManager)
	: WorkItem(dptfManager, FrameworkEvent::DptfResume)
{
}

WIDptfResume::~WIDptfResume(void)
{
}

void WIDptfResume::onExecute(void)
{
	// On a Windows machine DptfSuspend/DptfResume is called during the D0 Entry/Exit callback functions.  This is
	// synchronous and the new state doesn't take effect until this work item completes.

	writeWorkItemStartingInfoMessage();

	// notify all participants

	auto participantManager = getParticipantManager();
	auto participantIndexList = participantManager->getParticipantIndexes();

	for (auto i = participantIndexList.begin(); i != participantIndexList.end(); ++i)
	{
		try
		{
			Participant* participant = participantManager->getParticipantPtr(*i);
			participant->resume();
		}
		catch (participant_index_invalid&)
		{
			// do nothing.  No item in the participant list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessageParticipant(ex, "Participant::resume", *i);
		}
	}

	// notify all policies

	auto policyManager = getPolicyManager();
	auto policyIndexes = policyManager->getPolicyIndexes();

	for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
	{
		try
		{
			auto policy = policyManager->getPolicyPtr(*i);
			policy->executeResume();
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, "Policy::executeResume", *i);
		}
	}
}
