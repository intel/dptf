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

#include "DomainPerformanceControlFactory.h"
#include "DomainPerformanceControl_000.h"
#include "DomainPerformanceControl_001.h"
#include "DomainPerformanceControl_002.h"
#include "DomainPerformanceControl_003.h"
#include "DomainPerformanceControl_004.h"

ControlBase* DomainPerformanceControlFactory::make(
	UIntN participant,
	UIntN domain,
	UIntN version,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
{
	switch (version)
	{
	case 0:
		return new DomainPerformanceControl_000(participant, domain, participantServicesInterface);
		break;
	case 1: // Generic Participant
		return new DomainPerformanceControl_001(participant, domain, participantServicesInterface);
		break;
	case 2: // Processor participant (CPU domain)
		return new DomainPerformanceControl_002(participant, domain, participantServicesInterface);
		break;
	case 3: // Processor participant (GFX domain, Interface V1)
		return new DomainPerformanceControl_003(participant, domain, participantServicesInterface);
		break;
	case 4: // OSTF supported participant (2D Camera)
		return new DomainPerformanceControl_004(participant, domain, participantServicesInterface);
		break;
	default:
		std::stringstream message;
		message << "Received request for DomainPerformanceControl version that isn't defined: " << version;
		throw dptf_exception(message.str());
		break;
	}
}
