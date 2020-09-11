/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "WIDptfParticipantActivityLoggingEnabled.h"
#include "PolicyManagerInterface.h"
#include "ParticipantManagerInterface.h"
#include "EsifServicesInterface.h"

WIDptfParticipantActivityLoggingEnabled::WIDptfParticipantActivityLoggingEnabled(
	DptfManagerInterface* dptfManager,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt32 capabilityBitMask)
	: DomainWorkItem(
		  dptfManager,
		  FrameworkEvent::Type::DptfParticipantActivityLoggingEnabled,
		  participantIndex,
		  domainIndex)
	, m_capabilityBitMask(capabilityBitMask)
{
}

WIDptfParticipantActivityLoggingEnabled::~WIDptfParticipantActivityLoggingEnabled(void)
{
}

void WIDptfParticipantActivityLoggingEnabled::onExecute(void)
{
	writeDomainWorkItemStartingInfoMessage();

	try
	{
		getParticipantPtr()->activityLoggingEnabled(getDomainIndex(), m_capabilityBitMask);
	}
	catch (participant_index_invalid& ex)
	{
		writeDomainWorkItemWarningMessage(ex, "ParticipantManager::getParticipantPtr");
	}
	catch (std::exception& ex)
	{
		writeDomainWorkItemWarningMessage(ex, "Participant::activityLoggingEnabled");
	}
}
