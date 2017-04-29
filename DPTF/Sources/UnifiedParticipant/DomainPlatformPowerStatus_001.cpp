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

#include "DomainPlatformPowerStatus_001.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;

DomainPlatformPowerStatus_001::DomainPlatformPowerStatus_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPlatformPowerStatusBase(participantIndex, domainIndex, participantServicesInterface)
	, m_platformPowerSource()
	, m_chargerType()
{
	initializeDataStructures();
}

DomainPlatformPowerStatus_001::~DomainPlatformPowerStatus_001(void)
{
}

Power DomainPlatformPowerStatus_001::getMaxBatteryPower(UIntN participantIndex, UIntN domainIndex)
{
	m_maxBatteryPower = getParticipantServices()->primitiveExecuteGetAsPower(
		esif_primitive_type::GET_PLATFORM_MAX_BATTERY_POWER, domainIndex);
	return m_maxBatteryPower;
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

DptfBuffer DomainPlatformPowerStatus_001::getBatteryStatus(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_BATTERY_STATUS, ESIF_DATA_BINARY, domainIndex);
}

DptfBuffer DomainPlatformPowerStatus_001::getBatteryInformation(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_BATTERY_INFORMATION, ESIF_DATA_BINARY, domainIndex);
}

PlatformPowerSource::Type DomainPlatformPowerStatus_001::getPlatformPowerSource(
	UIntN participantIndex,
	UIntN domainIndex)
{
	auto powerSource = getParticipantServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_PLATFORM_POWER_SOURCE, domainIndex);
	m_platformPowerSource = PlatformPowerSource::Type(powerSource);
	return m_platformPowerSource;
}

ChargerType::Type DomainPlatformPowerStatus_001::getChargerType(UIntN participantIndex, UIntN domainIndex)
{
	auto chargerType =
		getParticipantServices()->primitiveExecuteGetAsUInt32(esif_primitive_type::GET_CHARGER_TYPE, domainIndex);
	m_chargerType = ChargerType::Type(chargerType);
	return m_chargerType;
}

Power DomainPlatformPowerStatus_001::getPlatformBatterySteadyState(UIntN participantIndex, UIntN domainIndex)
{
	m_batterySteadyState = getParticipantServices()->primitiveExecuteGetAsPower(
		esif_primitive_type::GET_PLATFORM_BATTERY_STEADY_STATE, domainIndex);
	return m_batterySteadyState;
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

void DomainPlatformPowerStatus_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS;
			capability.size = sizeof(capability);
			capability.data.platformPowerStatus.maxBatteryPower = 0;
			capability.data.platformPowerStatus.steadyStateBatteryPower = 0;
			capability.data.platformPowerStatus.platformRestOfPower = 0;
			capability.data.platformPowerStatus.adapterPowerRating = 0;
			capability.data.platformPowerStatus.chargerType = 0;
			capability.data.platformPowerStatus.platformPowerSource = 0;
			capability.data.platformPowerStatus.acNominalVoltage = 0;
			capability.data.platformPowerStatus.acOperationalCurrent = 0;
			capability.data.platformPowerStatus.ac1msOverload = 0;
			capability.data.platformPowerStatus.ac2msOverload = 0;
			capability.data.platformPowerStatus.ac10msOverload = 0;

			try
			{
				capability.data.platformPowerStatus.maxBatteryPower =
					(UInt32)getMaxBatteryPower(participantIndex, domainIndex);
			}
			catch (const std::exception& ex)
			{
				std::stringstream message;
				message << "Failed to get PMAX: " << ex.what();
				getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
			}
			try
			{
				capability.data.platformPowerStatus.steadyStateBatteryPower =
					(UInt32)getPlatformBatterySteadyState(participantIndex, domainIndex);
			}
			catch (const std::exception& ex)
			{
				std::stringstream message;
				message << "Failed to get PBSS: " << ex.what();
				getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
			}

			try
			{
				capability.data.platformPowerStatus.platformRestOfPower =
					(UInt32)getPlatformRestOfPower(participantIndex, domainIndex);
			}
			catch (const std::exception& ex)
			{
				std::stringstream message;
				message << "Failed to get PROP: " << ex.what();
				getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
			}

			try
			{
				capability.data.platformPowerStatus.adapterPowerRating =
					(UInt32)getAdapterPowerRating(participantIndex, domainIndex);
			}
			catch (const std::exception& ex)
			{
				std::stringstream message;
				message << "Failed to get ARTG: " << ex.what();
				getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
			}

			try
			{
				capability.data.platformPowerStatus.chargerType = (UInt32)getChargerType(participantIndex, domainIndex);
			}
			catch (const std::exception& ex)
			{
				std::stringstream message;
				message << "Failed to get CTYP: " << ex.what();
				getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
			}

			try
			{
				capability.data.platformPowerStatus.platformPowerSource =
					(UInt32)getPlatformPowerSource(participantIndex, domainIndex);
			}
			catch (const std::exception& ex)
			{
				std::stringstream message;
				message << "Failed to get PSRC: " << ex.what();
				getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
			}

			try
			{
				capability.data.platformPowerStatus.acNominalVoltage =
					getACNominalVoltage(participantIndex, domainIndex);
			}
			catch (const std::exception& ex)
			{
				std::stringstream message;
				message << "Failed to get AVOL: " << ex.what();
				getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
			}

			try
			{
				capability.data.platformPowerStatus.acOperationalCurrent =
					getACOperationalCurrent(participantIndex, domainIndex);
			}
			catch (const std::exception& ex)
			{
				std::stringstream message;
				message << "Failed to get ACUR: " << ex.what();
				getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
			}

			try
			{
				capability.data.platformPowerStatus.ac1msOverload =
					getAC1msPercentageOverload(participantIndex, domainIndex).toWholeNumber();
			}
			catch (const std::exception& ex)
			{
				std::stringstream message;
				message << "Failed to get AP01: " << ex.what();
				getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
			}

			try
			{
				capability.data.platformPowerStatus.ac2msOverload =
					getAC2msPercentageOverload(participantIndex, domainIndex).toWholeNumber();
			}
			catch (const std::exception& ex)
			{
				std::stringstream message;
				message << "Failed to get AP02: " << ex.what();
				getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
			}

			try
			{
				capability.data.platformPowerStatus.ac10msOverload =
					getAC10msPercentageOverload(participantIndex, domainIndex).toWholeNumber();
			}
			catch (const std::exception& ex)
			{
				std::stringstream message;
				message << "Failed to get AP10: " << ex.what();
				getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
			}

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));

			std::stringstream message;
			message << "Published activity for participant " << getParticipantIndex() << ", "
					<< "domain " << getName() << " "
					<< "("
					<< "Platform Power Status"
					<< ")";
			getParticipantServices()->writeMessageInfo(ParticipantMessage(FLF, message.str()));
		}
	}
	catch (...)
	{
		// skip if there are any issue in sending log data
	}
}

void DomainPlatformPowerStatus_001::clearCachedData(void)
{
	initializeDataStructures();
}

std::shared_ptr<XmlNode> DomainPlatformPowerStatus_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("platform_power_status");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	auto pmaxStatus = XmlNode::createWrapperElement("platform_power_status_object");
	pmaxStatus->addChild(XmlNode::createDataElement("name", "Max Battery Power (PMAX)"));
	try
	{
		pmaxStatus->addChild(XmlNode::createDataElement(
			"value", getMaxBatteryPower(getParticipantIndex(), domainIndex).toString() + "mW"));
	}
	catch (...)
	{
		pmaxStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(pmaxStatus);

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

	auto ctypStatus = XmlNode::createWrapperElement("platform_power_status_object");
	ctypStatus->addChild(XmlNode::createDataElement("name", "Charger Type (CTYP)"));
	try
	{
		ctypStatus->addChild(XmlNode::createDataElement(
			"value", ChargerType::ToString(getChargerType(getParticipantIndex(), domainIndex))));
	}
	catch (...)
	{
		ctypStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(ctypStatus);

	auto propStatus = XmlNode::createWrapperElement("platform_power_status_object");
	propStatus->addChild(XmlNode::createDataElement("name", "Platform Rest Of Power (PROP)"));
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

	auto pbssStatus = XmlNode::createWrapperElement("platform_power_status_object");
	pbssStatus->addChild(XmlNode::createDataElement("name", "Platform Battery Steady State (PBSS)"));
	try
	{
		pbssStatus->addChild(XmlNode::createDataElement(
			"value", getPlatformBatterySteadyState(getParticipantIndex(), domainIndex).toString() + "mW"));
	}
	catch (...)
	{
		pbssStatus->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(pbssStatus);

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
	m_maxBatteryPower = Power::createInvalid();
	m_platformRestOfPower = Power::createInvalid();
	m_adapterRating = Power::createInvalid();
	m_batterySteadyState = Power::createInvalid();
	m_acNominalVoltage = Constants::Invalid;
	m_acOperationalCurrent = Constants::Invalid;
	m_ac1msPercentageOverload = Percentage::createInvalid();
	m_ac2msPercentageOverload = Percentage::createInvalid();
	m_ac10msPercentageOverload = Percentage::createInvalid();
}
