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

#include "DomainTemperature_002.h"
#include "XmlNode.h"

DomainTemperature_002::DomainTemperature_002(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainTemperatureBase(participantIndex, domainIndex, participantServicesInterface)
	, m_lastSetTemperature(Temperature::createInvalid())
{
}

DomainTemperature_002::~DomainTemperature_002(void)
{
	clearCachedData();
}

TemperatureStatus DomainTemperature_002::getTemperatureStatus(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		Temperature temperature = getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(
			esif_primitive_type::GET_TEMPERATURE, domainIndex);

		if (!temperature.isValid())
		{
			getParticipantServices()->writeMessageWarning(
				ParticipantMessage(FLF, "Last set temperature for virtual sensor is invalid."));
			return TemperatureStatus(Temperature::minValidTemperature);
		}

		return TemperatureStatus(temperature);
	}
	catch (primitive_destination_unavailable)
	{
		return TemperatureStatus(Temperature::minValidTemperature);
	}
	catch (dptf_exception& ex)
	{
		getParticipantServices()->writeMessageWarning(ParticipantMessage(FLF, ex.what()));
		return TemperatureStatus(Temperature::minValidTemperature);
	}
}

DptfBuffer DomainTemperature_002::getCalibrationTable(UIntN participantIndex, UIntN domainIndex)
{
	if (m_calibrationTableBuffer.size() == 0)
	{
		createCalibrationTableBuffer(domainIndex);
	}

	return m_calibrationTableBuffer;
}

DptfBuffer DomainTemperature_002::getPollingTable(UIntN participantIndex, UIntN domainIndex)
{
	if (m_pollingTableBuffer.size() == 0)
	{
		createPollingTableBuffer(domainIndex);
	}

	return m_pollingTableBuffer;
}

Bool DomainTemperature_002::isVirtualTemperature(UIntN participantIndex, UIntN domainIndex)
{
	return true;
}

void DomainTemperature_002::setVirtualTemperature(
	UIntN participantIndex,
	UIntN domainIndex,
	const Temperature& temperature)
{
	getParticipantServices()->primitiveExecuteSetAsTemperatureTenthK(
		esif_primitive_type::SET_VIRTUAL_TEMPERATURE, temperature, domainIndex);
	m_lastSetTemperature = temperature;
}

void DomainTemperature_002::clearCachedData(void)
{
	m_calibrationTableBuffer.allocate(0);
	m_pollingTableBuffer.allocate(0);
}

std::string DomainTemperature_002::getName(void)
{
	return "Virtual Temperature Control";
}

std::shared_ptr<XmlNode> DomainTemperature_002::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("temperature_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "002"));
	root->addChild(getTemperatureStatus(getParticipantIndex(), domainIndex).getXml());
	root->addChild(getTemperatureThresholds(getParticipantIndex(), domainIndex).getXml());

	return root;
}

void DomainTemperature_002::createCalibrationTableBuffer(UIntN domainIndex)
{
	m_calibrationTableBuffer = getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_VIRTUAL_SENSOR_CALIB_TABLE, ESIF_DATA_BINARY, domainIndex);
}

void DomainTemperature_002::createPollingTableBuffer(UIntN domainIndex)
{
	m_pollingTableBuffer = getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_VIRTUAL_SENSOR_POLLING_TABLE, ESIF_DATA_BINARY, domainIndex);
}
