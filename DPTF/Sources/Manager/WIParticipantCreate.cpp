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

#include "WIParticipantCreate.h"
#include "ParticipantManagerInterface.h"
#include "Participant.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"

WIParticipantCreate::WIParticipantCreate(
	DptfManagerInterface* dptfManager,
	UIntN participantIndex,
	const AppParticipantDataPtr participantDataPtr,
	Bool participantEnabled,
	Bool* participantCreated)
	: ParticipantWorkItem(dptfManager, FrameworkEvent::ParticipantCreate, participantIndex)
	, m_participantDataPtr(participantDataPtr)
	, m_participantEnabled(participantEnabled)
	, m_participantCreated(participantCreated)
{
}

WIParticipantCreate::~WIParticipantCreate(void)
{
}

void WIParticipantCreate::onExecute(void)
{
	writeParticipantWorkItemStartingInfoMessage();

	Bool participantCreated = false;

	try
	{
		getParticipantManager()->createParticipant(getParticipantIndex(), m_participantDataPtr, m_participantEnabled);
		participantCreated = true;
	}
	catch (std::exception& ex)
	{
		writeParticipantWorkItemErrorMessage(ex, "ParticipantManager::createParticipant");
	}

	*m_participantCreated = participantCreated;

	if (participantCreated == true)
	{
		//
		// Iterate through the list of policies and let them know about the new participant
		//

		auto policyManager = getPolicyManager();
		auto policyIndexes = policyManager->getPolicyIndexes();

		for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
		{
			try
			{
				auto policy = policyManager->getPolicyPtr(*i);
				policy->bindParticipant(getParticipantIndex());
			}
			catch (policy_index_invalid&)
			{
				// do nothing.  No item in the policy list at this index.
			}
			catch (unsupported_result_temp_type& ex)
			{
				writeParticipantWorkItemWarningMessage(ex, "Policy::bindParticipant");
			}
			catch (std::exception& ex)
			{
				writeParticipantWorkItemErrorMessagePolicy(ex, "Policy::bindParticipant", *i);
			}
		}
	}
}
