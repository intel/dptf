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

#include "DomainTemperatureFactory.h"
#include "DomainTemperature_000.h"
#include "DomainTemperature_001.h"
#include "DomainTemperature_002.h"

ControlBase* DomainTemperatureFactory::make(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN version,
	UIntN associatedVersion,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
{
	// Check for threshold support
	Bool areTemperatureThresholdsSupported = false;
	if (associatedVersion > 0) {
		areTemperatureThresholdsSupported = true;
	}

	switch (version)
	{
	case 0:
		return new DomainTemperature_000(participantIndex, domainIndex, areTemperatureThresholdsSupported, participantServicesInterface);
		break;
	case 1:
		return new DomainTemperature_001(participantIndex, domainIndex, areTemperatureThresholdsSupported, participantServicesInterface);
		break;
	case 2:
		return new DomainTemperature_002(participantIndex, domainIndex, areTemperatureThresholdsSupported, participantServicesInterface);
		break;
	default:
		std::stringstream message;
		message << "Received request for DomainTemperature version that isn't defined: " << version;
		throw dptf_exception(message.str());
		break;
	}
}

ControlBase* DomainTemperatureFactory::make(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN version,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
{
	throw not_implemented();
}

