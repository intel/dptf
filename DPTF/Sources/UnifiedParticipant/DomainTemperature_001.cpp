/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "DomainTemperature_001.h"

DomainTemperature_001::DomainTemperature_001(
	UIntN participantIndex,
	UIntN domainIndex,
	Bool areTemperatureThresholdsSupported,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainTemperatureBase(
		participantIndex,
		domainIndex,
		areTemperatureThresholdsSupported,
		participantServicesInterface)
{
}

DomainTemperature_001::~DomainTemperature_001()
{
}

TemperatureStatus DomainTemperature_001::getTemperatureStatus()
{
	try
	{
		Temperature temperature = getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(
			esif_primitive_type::GET_TEMPERATURE, getDomainIndex());

		return TemperatureStatus(temperature);
	}
	catch (primitive_destination_unavailable&)
	{
		return TemperatureStatus(Temperature::minValidTemperature);
	}
	catch (dptf_exception& ex)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING_EX({ return ex.getDescription(); });

		// TODO: Let the policies handle the exceptions themselves and don't return a value.
		return TemperatureStatus(Temperature::minValidTemperature);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING({ return "Something went wrong when getting temperature"; });

		return TemperatureStatus(Temperature::minValidTemperature);
	}
}

Temperature DomainTemperature_001::getPowerShareTemperatureThreshold()
{
	auto powerShareTemperatureThreshold = Temperature::createInvalid();

	try
	{
		powerShareTemperatureThreshold = getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(
			esif_primitive_type::GET_POWER_SHARE_TEMPERATURE_THRESHOLD, getDomainIndex());
	}
	catch (primitive_not_found_in_dsp&)
	{
		PARTICIPANT_LOG_MESSAGE_INFO(
			{ return "Participant does not support the get power share temperature threshold primitive"; });
	}

	return powerShareTemperatureThreshold;
}

DptfBuffer DomainTemperature_001::getCalibrationTable()
{
	throw not_implemented();
}

DptfBuffer DomainTemperature_001::getPollingTable()
{
	throw not_implemented();
}

Bool DomainTemperature_001::isVirtualTemperature()
{
	return false;
}

void DomainTemperature_001::setVirtualTemperature(const Temperature& temperature)
{
	throw not_implemented();
}

std::shared_ptr<XmlNode> DomainTemperature_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("temperature_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));
	root->addChild(getTemperatureStatus().getXml());

	if (m_areTemperatureThresholdsSupported)
	{
		root->addChild(getTemperatureThresholds().getXml());
	}

	return root;
}

std::string DomainTemperature_001::getName(void)
{
	return "Temperature Status";
}
