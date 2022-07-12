/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "DomainActivityStatusFactory.h"
#include "DomainActivityStatus_000.h"
#include "DomainActivityStatus_001.h"
#include "DomainActivityStatus_002.h"

ControlBase* DomainActivityStatusFactory::make(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN version,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
{
	switch (version)
	{
	case 0:
		return new DomainActivityStatus_000(participantIndex, domainIndex, participantServicesInterface);
	case 1:
		return new DomainActivityStatus_001(participantIndex, domainIndex, participantServicesInterface);
	case 2:
		return new DomainActivityStatus_002(participantIndex, domainIndex, participantServicesInterface);
	default:
		std::stringstream message;
		message << "Received request for DomainActivityStatus version that isn't defined: " << version;
		throw dptf_exception(message.str());
	}
}
