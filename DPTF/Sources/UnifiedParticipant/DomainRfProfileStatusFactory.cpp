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

#include "DomainRfProfileStatusFactory.h"
#include "DomainRfProfileStatus_000.h"
#include "DomainRfProfileStatus_001.h"
#include "DomainRfProfileStatus_002.h"
#include "DomainRfProfileStatus_003.h"

ControlBase* DomainRfProfileStatusFactory::make(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN version,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
{
	switch (version)
	{
	case 0: // capability not supported
		return new DomainRfProfileStatus_000(participantIndex, domainIndex, participantServicesInterface);
		break;
	case 1: // fivr
		return new DomainRfProfileStatus_001(participantIndex, domainIndex, participantServicesInterface);
		break;
	case 2: // wireless
		return new DomainRfProfileStatus_002(participantIndex, domainIndex, participantServicesInterface);
		break;
	case 3: // memory
		return new DomainRfProfileStatus_003(participantIndex, domainIndex, participantServicesInterface);
		break;
	default:
		std::stringstream message;
		message << "Received request for DomainRfProfileStatus version that isn't defined: " << version;
		throw dptf_exception(message.str());
		break;
	}
}
