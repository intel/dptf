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

#include "DomainPlatformPowerStatus_001.h"
#include "BinaryParse.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;

static const UInt16 PsrcSequenceStartBit = 7;
static const UInt16 PsrcSequenceStopBit = 4;
static const UInt16 PsrcValueStartBit = 3;
static const UInt16 PsrcValueStopBit = 0;

static const UInt32 AssertProchot = 0x80000000;

DomainPlatformPowerStatus_001::DomainPlatformPowerStatus_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPlatformPowerStatusBase(participantIndex, domainIndex, participantServicesInterface)
	, m_platformPowerSource()
{
	initializeDataStructures();
}

DomainPlatformPowerStatus_001::~DomainPlatformPowerStatus_001(void)
{
}

Power DomainPlatformPowerStatus_001::getPlatformRestOfPower(UIntN participantIndex, UIntN domainIndex)
{
	m_platformRestOfPower = getParticipantServices()->primitiveExecuteGetAsPower(
		esif_primitive_type::GET_PLATFORM_REST_OF_POWER, domainIndex);
	return m_platformRestOfPower;
}

Power DomainPlatformPowerStatus_001::getAdapterPowerRating(UIntN participantIndex, UIntN domainIndex)
{
	m_adapterRating = getParticipantServices()->primitiveExecuteGetAsPower(
		esif_primitive_type::GET_ADAPTER_POWER_RATING, domainIndex);
	return m_adapterRating;
}

PlatformPowerSource::Type DomainPlatformPowerStatus_001::getPlatformPowerSource(
	UIntN participantIndex,
	UIntN domainIndex)
{
	auto psrcValue = getParticipantServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_PLATFORM_POWER_SOURCE, domainIndex);
	m_psrcSequence.set(
		static_cast<UInt32>(BinaryParse::extractBits(PsrcSequenceStartBit, PsrcSequenceStopBit, psrcValue)));
	m_platformPowerSource = PlatformPowerSource::Type(
		static_cast<UInt32>(BinaryParse::extractBits(PsrcValueStartBit, PsrcValueStopBit, psrcValue)));
	return m_platformPowerSource;
}

UInt32 DomainPlatformPowerStatus_001::getACNominalVoltage(UIntN participantIndex, UIntN domainIndex)
{
	m_acNominalVoltage =
		getParticipantServices()->primitiveExecuteGetAsUInt32(esif_primitive_type::GET_AVOL, domainIndex);
	return m_acNominalVoltage;
}

UInt32 DomainPlatformPowerStatus_001::getACOperationalCurrent(UIntN participantIndex, UIntN domainIndex)
{
	m_acOperationalCurrent =
		getParticipantServices()->primitiveExecuteGetAsUInt32(esif_primitive_type::GET_ACUR, domainIndex);
	return m_acOperationalCurrent;
}

Percentage DomainPlatformPowerStatus_001::getAC1msPercentageOverload(UIntN participantIndex, UIntN domainIndex)
{
	m_ac1msPercentageOverload =
		getParticipantServices()->primitiveExecuteGetAsPercentage(esif_primitive_type::GET_AP01, domainIndex);
	return m_ac1msPercentageOverload;
}

Percentage DomainPlatformPowerStatus_001::getAC2msPercentageOverload(UIntN participantIndex, UIntN domainIndex)
{
	m_ac2msPercentageOverload =
		getParticipantServices()->primitiveExecuteGetAsPercentage(esif_primitive_type::GET_AP02, domainIndex);
	return m_ac2msPercentageOverload;
}

Percentage DomainPlatformPowerStatus_001::getAC10msPercentageOverload(UIntN participantIndex, UIntN domainIndex)
{
	m_ac10msPercentageOverload =
		getParticipantServices()->primitiveExecuteGetAsPercentage(esif_primitive_type::GET_AP10, domainIndex);
	return m_ac10msPercentageOverload;
}

void DomainPlatformPowerStatus_001::notifyForProchotDeassertion(UIntN participantIndex, UIntN domainIndex)
{
	UInt32 prochotAssertionRequest = 0;
	try
	{
		if (m_psrcSequence.isValid())
		{
			prochotAssertionRequest |= m_psrcSequence.get();
		}

		prochotAssertionRequest |= AssertProchot;
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Setting PBOK: " + friendlyValue(prochotAssertionRequest); });

		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_PBOK, prochotAssertionRequest, domainIndex);
	}
	catch (const std::exception& ex)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG_EX({
			std::stringstream message;
			message << "Failed to set PBOK: " << ex.what();
			return message.str();
		});
	}
}

void DomainPlatformPowerStatus_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	// ESIF handles status reporting to participant log
}

void DomainPlatformPowerStatus_001::onClearCachedData(void)
{
	initializeDataStructures();
}

std::shared_ptr<XmlNode> DomainPlatformPowerStatus_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("platform_power_status");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	auto psrcStatus = XmlNode::createWrapperElement("platform_power_status_object");
	psrcStatus->addChild(XmlNode::createDataElement("name", "Platform Power Source (PSRC)"));
	try
	{
		psrcStatus->addChild(XmlNode::createDataElement(
			"value", PlatformPowerSource::ToString(getPlatformPowerSource(getParticipantIndex(), domainIndex))));
	}
	catch (...)
	{
		psrcStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(psrcStatus);

	auto artgStatus = XmlNode::createWrapperElement("platform_power_status_object");
	artgStatus->addChild(XmlNode::createDataElement("name", "Adapter Power Rating (ARTG)"));
	try
	{
		artgStatus->addChild(XmlNode::createDataElement(
			"value", getAdapterPowerRating(getParticipantIndex(), domainIndex).toString() + "mW"));
	}
	catch (...)
	{
		artgStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(artgStatus);

	auto propStatus = XmlNode::createWrapperElement("platform_power_status_object");
	propStatus->addChild(XmlNode::createDataElement("name", "Rest Of Platform Power (PROP)"));
	try
	{
		propStatus->addChild(XmlNode::createDataElement(
			"value", getPlatformRestOfPower(getParticipantIndex(), domainIndex).toString() + "mW"));
	}
	catch (...)
	{
		propStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(propStatus);

	auto avolStatus = XmlNode::createWrapperElement("platform_power_status_object");
	avolStatus->addChild(XmlNode::createDataElement("name", "AC Nominal Voltage (AVOL)"));
	try
	{
		avolStatus->addChild(XmlNode::createDataElement(
			"value", friendlyValue(getACNominalVoltage(getParticipantIndex(), domainIndex)) + "mV"));
	}
	catch (...)
	{
		avolStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(avolStatus);

	auto acurStatus = XmlNode::createWrapperElement("platform_power_status_object");
	acurStatus->addChild(XmlNode::createDataElement("name", "AC Operational Current (ACUR)"));
	try
	{
		acurStatus->addChild(XmlNode::createDataElement(
			"value", friendlyValue(getACOperationalCurrent(getParticipantIndex(), domainIndex)) + "mA"));
	}
	catch (...)
	{
		acurStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(acurStatus);

	auto ap01Status = XmlNode::createWrapperElement("platform_power_status_object");
	ap01Status->addChild(XmlNode::createDataElement("name", "AC 1ms Percentage Overload (AP01)"));
	try
	{
		ap01Status->addChild(XmlNode::createDataElement(
			"value", getAC1msPercentageOverload(getParticipantIndex(), domainIndex).toStringWithPrecision(0) + "%"));
	}
	catch (...)
	{
		ap01Status->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(ap01Status);

	auto ap02Status = XmlNode::createWrapperElement("platform_power_status_object");
	ap02Status->addChild(XmlNode::createDataElement("name", "AC 2ms Percentage Overload (AP02)"));
	try
	{
		ap02Status->addChild(XmlNode::createDataElement(
			"value", getAC2msPercentageOverload(getParticipantIndex(), domainIndex).toStringWithPrecision(0) + "%"));
	}
	catch (...)
	{
		ap02Status->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(ap02Status);

	auto ap10Status = XmlNode::createWrapperElement("platform_power_status_object");
	ap10Status->addChild(XmlNode::createDataElement("name", "AC 10ms Percentage Overload (AP10)"));
	try
	{
		ap10Status->addChild(XmlNode::createDataElement(
			"value", getAC10msPercentageOverload(getParticipantIndex(), domainIndex).toStringWithPrecision(0) + "%"));
	}
	catch (...)
	{
		ap10Status->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(ap10Status);

	return root;
}

std::string DomainPlatformPowerStatus_001::getName(void)
{
	return "Platform Power Status";
}

void DomainPlatformPowerStatus_001::initializeDataStructures(void)
{
	m_platformRestOfPower = Power::createInvalid();
	m_adapterRating = Power::createInvalid();
	m_acNominalVoltage = Constants::Invalid;
	m_acOperationalCurrent = Constants::Invalid;
	m_ac1msPercentageOverload = Percentage::createInvalid();
	m_ac2msPercentageOverload = Percentage::createInvalid();
	m_ac10msPercentageOverload = Percentage::createInvalid();
}
