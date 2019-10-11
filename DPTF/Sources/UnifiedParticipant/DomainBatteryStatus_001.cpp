/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "DomainBatteryStatus_001.h"
#include "BinaryParse.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;

DomainBatteryStatus_001::DomainBatteryStatus_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainBatteryStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainBatteryStatus_001::~DomainBatteryStatus_001(void)
{
}

Power DomainBatteryStatus_001::getMaxBatteryPower()
{
	return getParticipantServices()->primitiveExecuteGetAsPower(
		esif_primitive_type::GET_PLATFORM_MAX_BATTERY_POWER, getDomainIndex());
}

DptfBuffer DomainBatteryStatus_001::getBatteryStatus()
{
	return getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_BATTERY_STATUS, ESIF_DATA_BINARY, getDomainIndex());
}

DptfBuffer DomainBatteryStatus_001::getBatteryInformation()
{
	return getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_BATTERY_INFORMATION, ESIF_DATA_BINARY, getDomainIndex());
}

ChargerType::Type DomainBatteryStatus_001::getChargerType()
{
	return ChargerType::Type(getParticipantServices()->primitiveExecuteGetAsUInt32(esif_primitive_type::GET_CHARGER_TYPE, getDomainIndex()));
}

Power DomainBatteryStatus_001::getPlatformBatterySteadyState()
{
	return getParticipantServices()->primitiveExecuteGetAsPower(
		esif_primitive_type::GET_PLATFORM_BATTERY_STEADY_STATE, getDomainIndex());
}

UInt32 DomainBatteryStatus_001::getBatteryHighFrequencyImpedance()
{
	return getParticipantServices()->primitiveExecuteGetAsUInt32(esif_primitive_type::GET_BATTERY_HIGH_FREQUENCY_IMPEDANCE, getDomainIndex());
}

UInt32 DomainBatteryStatus_001::getBatteryNoLoadVoltage()
{
	return getParticipantServices()->primitiveExecuteGetAsUInt32(esif_primitive_type::GET_BATTERY_NO_LOAD_VOLTAGE, getDomainIndex());
}

UInt32 DomainBatteryStatus_001::getBatteryMaxPeakCurrent()
{
	return getParticipantServices()->primitiveExecuteGetAsUInt32(esif_primitive_type::GET_BATTERY_MAX_PEAK_CURRENT, getDomainIndex());
}

void DomainBatteryStatus_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	// ESIF handles status reporting to participant log
}

std::shared_ptr<XmlNode> DomainBatteryStatus_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("battery_status");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	auto pmaxStatus = XmlNode::createWrapperElement("battery_status_object");
	pmaxStatus->addChild(XmlNode::createDataElement("name", "Battery Max Peak Power (PMAX)"));
	try
	{
		pmaxStatus->addChild(XmlNode::createDataElement(
			"value", getMaxBatteryPower().toString() + "mW"));
	}
	catch (...)
	{
		pmaxStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(pmaxStatus);

	auto ctypStatus = XmlNode::createWrapperElement("battery_status_object");
	ctypStatus->addChild(XmlNode::createDataElement("name", "Charger Type (CTYP)"));
	try
	{
		ctypStatus->addChild(XmlNode::createDataElement(
			"value", ChargerType::ToString(getChargerType())));
	}
	catch (...)
	{
		ctypStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(ctypStatus);

	auto pbssStatus = XmlNode::createWrapperElement("battery_status_object");
	pbssStatus->addChild(XmlNode::createDataElement("name", "Battery Sustained Peak Power (PBSS)"));
	try
	{
		pbssStatus->addChild(XmlNode::createDataElement(
			"value", getPlatformBatterySteadyState().toString() + "mW"));
	}
	catch (...)
	{
		pbssStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(pbssStatus);

	auto rbhfStatus = XmlNode::createWrapperElement("battery_status_object");
	rbhfStatus->addChild(XmlNode::createDataElement("name", "Battery High Frequency Impedance (RBHF)"));
	try
	{
		rbhfStatus->addChild(XmlNode::createDataElement(
			"value", friendlyValue(getBatteryHighFrequencyImpedance()) + "mOhm"));
	}
	catch (...)
	{
		rbhfStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(rbhfStatus);

	auto vbnlStatus = XmlNode::createWrapperElement("battery_status_object");
	vbnlStatus->addChild(XmlNode::createDataElement("name", "Battery No Load Voltage (VBNL)"));
	try
	{
		vbnlStatus->addChild(XmlNode::createDataElement(
			"value", friendlyValue(getBatteryNoLoadVoltage()) + "mV"));
	}
	catch (...)
	{
		vbnlStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(vbnlStatus);

	auto cmppStatus = XmlNode::createWrapperElement("battery_status_object");
	cmppStatus->addChild(XmlNode::createDataElement("name", "Battery Max Peak Current (CMPP)"));
	try
	{
		cmppStatus->addChild(XmlNode::createDataElement(
			"value", friendlyValue(getBatteryMaxPeakCurrent()) + "mA"));
	}
	catch (...)
	{
		cmppStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(cmppStatus);

	return root;
}

std::string DomainBatteryStatus_001::getName(void)
{
	return "Battery Status";
}