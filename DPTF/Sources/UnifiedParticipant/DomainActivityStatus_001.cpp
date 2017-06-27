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

#include "DomainActivityStatus_001.h"
#include "XmlNode.h"
#include "esif_ccb.h"

DomainActivityStatus_001::DomainActivityStatus_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainActivityStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainActivityStatus_001::~DomainActivityStatus_001()
{
}

UInt32 DomainActivityStatus_001::getEnergyThreshold(UIntN participantIndex, UIntN domainIndex)
{
	auto energyThreshold = 0;

	try
	{
		energyThreshold = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_PARTICIPANT_ENERGY_THRESHOLD, domainIndex);
	}
	catch (primitive_not_found_in_dsp)
	{
		getParticipantServices()->writeMessageInfo(
			ParticipantMessage(FLF, "Participant does not support the get energy threshold primitive"));
	}
	catch (...)
	{
		getParticipantServices()->writeMessageInfo(ParticipantMessage(FLF, "Failed to get energy threshold interrupt"));
	}

	return energyThreshold;
}

void DomainActivityStatus_001::setEnergyThreshold(UIntN participantIndex, UIntN domainIndex, UInt32 energyThreshold)
{
	try
	{
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_ENERGY_THRESHOLD_COUNT, energyThreshold, domainIndex);
	}
	catch (primitive_not_found_in_dsp)
	{
		getParticipantServices()->writeMessageInfo(
			ParticipantMessage(FLF, "Participant does not support the set energy threshold primitive"));
	}
	catch (...)
	{
		getParticipantServices()->writeMessageInfo(ParticipantMessage(FLF, "Failed to set energy threshold"));
	}
}

Temperature DomainActivityStatus_001::getPowerShareTemperatureThreshold(UIntN participantIndex, UIntN domainIndex)
{
	auto powerShareTemperatureThreshold = Temperature::createInvalid();

	try
	{
		powerShareTemperatureThreshold = getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(
			esif_primitive_type::GET_POWER_SHARE_TEMPERATURE_THRESHOLD, domainIndex);
	}
	catch (primitive_not_found_in_dsp)
	{
		getParticipantServices()->writeMessageInfo(ParticipantMessage(
			FLF, "Participant does not support the get power share temperature threshold primitive"));
	}

	return powerShareTemperatureThreshold;
}

Percentage DomainActivityStatus_001::getUtilizationThreshold(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsPercentage(
		esif_primitive_type::GET_PARTICIPANT_UTILIZATION_THRESHOLD, domainIndex);
}

Percentage DomainActivityStatus_001::getResidencyUtilization(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsPercentage(
		esif_primitive_type::GET_PARTICIPANT_RESIDENCY_UTILIZATION, domainIndex);
}

void DomainActivityStatus_001::setEnergyThresholdInterruptDisable(
	UIntN participantIndex,
	UIntN domainIndex)
{
	try
	{
		getParticipantServices()->primitiveExecuteSet(
			esif_primitive_type::SET_ENERGY_THRESHOLD_INT_DISABLE, esif_data_type::ESIF_DATA_VOID, nullptr, 0, 0, domainIndex, Constants::Esif::NoInstance);
	}
	catch (primitive_not_found_in_dsp)
	{
		getParticipantServices()->writeMessageInfo(ParticipantMessage(
			FLF, "Participant does not support the set energy threshold interrupt enable primitive"));
	}
	catch (...)
	{
		getParticipantServices()->writeMessageInfo(
			ParticipantMessage(FLF, "Failed to set energy threshold interrupt flag"));
	}
}

void DomainActivityStatus_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	// do nothing
}

void DomainActivityStatus_001::clearCachedData(void)
{
	// do nothing
}

std::shared_ptr<XmlNode> DomainActivityStatus_001::getXml(UIntN domainIndex)
{
	std::shared_ptr<XmlNode> root = XmlNode::createWrapperElement("activity_status");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	return root;
}

std::string DomainActivityStatus_001::getName(void)
{
	return "Activity Status";
}
