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

#include "DomainPerformanceControlBase.h"

DomainPerformanceControlBase::DomainPerformanceControlBase(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ControlBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainPerformanceControlBase::~DomainPerformanceControlBase()
{
}

void DomainPerformanceControlBase::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			UInt32 performanceControlIndex = getCurrentPerformanceControlIndex(participantIndex, domainIndex);

			if (performanceControlIndex == Constants::Invalid)
			{
				performanceControlIndex = 0;
			}
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_PERF_CONTROL;
			capability.size = sizeof(capability);
			capability.data.performanceControl.pStateLimit = performanceControlIndex;
			capability.data.performanceControl.lowerLimit =
				getPerformanceControlDynamicCaps(participantIndex, domainIndex).getCurrentLowerLimitIndex();
			capability.data.performanceControl.upperLimit =
				getPerformanceControlDynamicCaps(participantIndex, domainIndex).getCurrentUpperLimitIndex();

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));

			std::stringstream message;
			message << "Published activity for participant " << getParticipantIndex() << ", "
					<< "domain " << getName() << " "
					<< "("
					<< "Performance Control"
					<< ")";
			getParticipantServices()->writeMessageInfo(ParticipantMessage(FLF, message.str()));
		}
	}
	catch (...)
	{
		// skip if there are any issue in sending log data
	}
}
