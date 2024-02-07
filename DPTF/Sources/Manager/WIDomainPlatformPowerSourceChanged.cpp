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

#include "WIDomainPlatformPowerSourceChanged.h"
#include "Participant.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"

WIDomainPlatformPowerSourceChanged::WIDomainPlatformPowerSourceChanged(
	DptfManagerInterface* dptfManager,
	UIntN participantIndex,
	UIntN domainIndex)
	: DomainWorkItem(dptfManager, FrameworkEvent::Type::DomainPlatformPowerSourceChanged, participantIndex, domainIndex)
{
}

WIDomainPlatformPowerSourceChanged::~WIDomainPlatformPowerSourceChanged(void)
{
}

void WIDomainPlatformPowerSourceChanged::onExecute(void)
{
	writeDomainWorkItemStartingInfoMessage();

	try
	{
		getParticipantPtr()->domainPlatformPowerSourceChanged();
		getParticipantPtr()->domainAdapterPowerRatingChanged();
		getParticipantPtr()->domainACNominalVoltageChanged();
		getParticipantPtr()->domainACOperationalCurrentChanged();
		getParticipantPtr()->domainAC1msPercentageOverloadChanged();
		getParticipantPtr()->domainAC2msPercentageOverloadChanged();
		getParticipantPtr()->domainAC10msPercentageOverloadChanged();
	}
	catch (participant_index_invalid& ex)
	{
		writeDomainWorkItemWarningMessage(ex, "ParticipantManager::getParticipantPtr");
	}
	catch (std::exception& ex)
	{
		writeDomainWorkItemErrorMessage(ex, "Participant::domainPlatformPowerSourceChanged");
	}

	auto policyManager = getPolicyManager();
	auto policyIndexes = policyManager->getPolicyIndexes();

	for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
	{
		try
		{
			auto policy = policyManager->getPolicyPtr(*i);

			// FIXME:
			// As requested by DPTF architecture, the event for power source changed
			// should also result in DPTF re-reading up to 3 other data items.  Because
			// we do not have separate events for these on the platform, we are
			// representing these data change events separately in DPTF only.
			// Alternative is to represent all 4 pieces of information as a single
			// data structure with a single event mapped to it, or BIOS/EC should send
			// a different event code for each item if it changes.
			policy->executeDomainPlatformPowerSourceChanged(getParticipantIndex());
			policy->executeDomainAdapterPowerRatingChanged(getParticipantIndex());
			policy->executeDomainACNominalVoltageChanged(getParticipantIndex());
			policy->executeDomainACOperationalCurrentChanged(getParticipantIndex());
			policy->executeDomainAC1msPercentageOverloadChanged(getParticipantIndex());
			policy->executeDomainAC2msPercentageOverloadChanged(getParticipantIndex());
			policy->executeDomainAC10msPercentageOverloadChanged(getParticipantIndex());
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeDomainWorkItemErrorMessagePolicy(ex, "Policy::executeDomainPlatformPowerSourceChanged", *i);
		}
	}
}
