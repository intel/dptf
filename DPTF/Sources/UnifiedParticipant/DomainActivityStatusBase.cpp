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

#include "DomainActivityStatusBase.h"

DomainActivityStatusBase::DomainActivityStatusBase(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ControlBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainActivityStatusBase::~DomainActivityStatusBase()
{
}

UInt32 DomainActivityStatusBase::getSocDgpuPerformanceHintPoints(UIntN participantIndex, UIntN domainIndex)
{
	UInt32 numberOfSocDgpuPerformanceHintPoints = Constants::Invalid;

	try
	{
		numberOfSocDgpuPerformanceHintPoints = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_SOC_DGPU_WEIGHTS, domainIndex);
	}
	catch (dptf_exception& ex)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			std::stringstream message;
			message << "Failed to get SOC DGPU Performance Hint. for participant index = "
						   + std::to_string(participantIndex) + "and domain Index = " + std::to_string(domainIndex)
						   + ", exception: " + std::string(ex.what());
			return message.str();
		});
	}

	return numberOfSocDgpuPerformanceHintPoints;
}
