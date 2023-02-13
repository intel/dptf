/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "DomainProcessorControlFactory.h"
#include "DomainProcessorControl_000.h"
#include "DomainProcessorControl_001.h"

ControlBase* DomainProcessorControlFactory::make(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN version,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
{
	switch (version)
	{
	case 0:
		return new DomainProcessorControl_000(participantIndex, domainIndex, participantServicesInterface);
		break;
	case 1:
		return new DomainProcessorControl_001(participantIndex, domainIndex, participantServicesInterface);
		break;
	default:
		std::stringstream message;
		message << "Received request for DomainProcessorControl version that isn't defined: " << version;
		throw dptf_exception(message.str());
		break;
	}
}
