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

#include "WIDomainExtendedWorkloadPredictionChanged.h"
#include "PolicyManagerInterface.h"
#include "Participant.h"
#include "EventPayloadExtendedWorkloadPredictionType.h"

using namespace std;

WIDomainExtendedWorkloadPredictionChanged::WIDomainExtendedWorkloadPredictionChanged(
	DptfManagerInterface* dptfManager,
	UIntN participantIndex,
	UIntN domainIndex,
	ExtendedWorkloadPrediction::Type extendedWorkloadPrediction)
	: DomainWorkItem(
		dptfManager, 
		FrameworkEvent::DomainExtendedWorkloadPredictionChanged, 
		participantIndex, 
		domainIndex)
	, m_extendedWorkloadPrediction(extendedWorkloadPrediction)
{
}

void WIDomainExtendedWorkloadPredictionChanged::onExecute()
{
	writeDomainWorkItemStartingInfoMessage();

	getDptfManager()->getEventNotifier()->notify(
		getFrameworkEventType(),
		EventPayloadExtendedWorkloadPredictionType{m_extendedWorkloadPrediction}
	); 

	try
	{
		getParticipantPtr()->domainExtendedWorkloadPredictionChanged(m_extendedWorkloadPrediction);
	}
	catch (participant_index_invalid& ex)
	{
		writeDomainWorkItemWarningMessage(ex, "ParticipantManager::getParticipantPtr"s);
	}
	catch (std::exception& ex)
	{
		writeDomainWorkItemErrorMessage(ex, "Participant::domainExtendedWorkloadPredictionChanged"s);
	}

	const auto policyManager = getPolicyManager();
	const auto policyIndexes = policyManager->getPolicyIndexes();
	for (const auto policyId : policyIndexes)
	{
		try
		{
			const auto policy = policyManager->getPolicyPtr(policyId);
			policy->executeDomainExtendedWorkloadPredictionChanged(
				getParticipantIndex(), 
				getDomainIndex(), 
				m_extendedWorkloadPrediction);
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeDomainWorkItemErrorMessagePolicy(
				ex, "Policy::executeDomainExtendedWorkloadPredictionChanged"s, policyId);
		}
	}
}
