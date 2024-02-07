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

#include "DomainTemperature_002.h"

DomainTemperature_002::DomainTemperature_002(
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
	setTemperatureToDefaultValue();
}

DomainTemperature_002::~DomainTemperature_002(void)
{
}

TemperatureStatus DomainTemperature_002::getTemperatureStatus()
{
	try
	{
		Temperature temperature = getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(
			esif_primitive_type::GET_TEMPERATURE, getDomainIndex());

		if (!temperature.isValid())
		{
			PARTICIPANT_LOG_MESSAGE_WARNING({ return "Last set temperature for virtual sensor is invalid."; });

			return TemperatureStatus(Temperature::minValidTemperature);
		}

		return TemperatureStatus(temperature);
	}
	catch (primitive_destination_unavailable&)
	{
		return TemperatureStatus(Temperature::minValidTemperature);
	}
	catch (dptf_exception& ex)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING_EX({ return ex.getDescription(); });

		return TemperatureStatus(Temperature::minValidTemperature);
	}
}

Temperature DomainTemperature_002::getPowerShareTemperatureThreshold()
{
	throw dptf_exception("Get Power Share Temperature Threshold is not supported by " + getName() + ".");
}

DptfBuffer DomainTemperature_002::getCalibrationTable()
{
	auto calibrationTableBuffer = DptfBuffer();

	try
	{
		calibrationTableBuffer = getParticipantServices()->primitiveExecuteGet(
			esif_primitive_type::GET_VIRTUAL_SENSOR_CALIB_TABLE, ESIF_DATA_BINARY, getDomainIndex());
	}
	catch (dptf_exception& ex)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG_EX({ return ex.getDescription(); });
	}

	return calibrationTableBuffer;
}

DptfBuffer DomainTemperature_002::getPollingTable()
{
	auto pollingTableBuffer = DptfBuffer();

	try
	{
		pollingTableBuffer = getParticipantServices()->primitiveExecuteGet(
			esif_primitive_type::GET_VIRTUAL_SENSOR_POLLING_TABLE, ESIF_DATA_BINARY, getDomainIndex());
	}
	catch (dptf_exception& ex)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG_EX({ return ex.getDescription(); });
	}

	return pollingTableBuffer;
}

Bool DomainTemperature_002::isVirtualTemperature()
{
	return true;
}

void DomainTemperature_002::setVirtualTemperature(const Temperature& temperature)
{
	getParticipantServices()->primitiveExecuteSetAsTemperatureTenthK(
		esif_primitive_type::SET_VIRTUAL_TEMPERATURE, temperature, getDomainIndex());
}

std::string DomainTemperature_002::getName(void)
{
	return "Virtual Temperature Status and Control";
}

std::shared_ptr<XmlNode> DomainTemperature_002::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("temperature_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "002"));
	root->addChild(getTemperatureStatus().getXml());

	if (m_areTemperatureThresholdsSupported)
	{
		root->addChild(getTemperatureThresholds().getXml());
	}

	return root;
}

void DomainTemperature_002::setTemperatureToDefaultValue()
{
	try
	{
		setVirtualTemperature(Temperature::minValidTemperature);
	}
	catch (dptf_exception& ex)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG_EX({ return ex.getDescription(); });
	}
}
