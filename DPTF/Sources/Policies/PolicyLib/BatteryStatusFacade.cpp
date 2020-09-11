/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "BatteryStatusFacade.h"
#include "StatusFormat.h"
#include "PolicyLogger.h"

using namespace std;
using namespace StatusFormat;

BatteryStatusFacade::BatteryStatusFacade(
	UIntN participantIndex,
	UIntN domainIndex,
	const DomainProperties& domainProperties,
	const PolicyServicesInterfaceContainer& policyServices)
	: m_policyServices(policyServices)
	, m_domainProperties(domainProperties)
	, m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
	, m_maxBatteryPower(Power::createInvalid())
	, m_chargerType()
	, m_batterySteadyState(Power::createInvalid())
	, m_batteryHighFrequencyImpedance(Constants::Invalid)
	, m_batteryNoLoadVoltage(Constants::Invalid)
	, m_batteryMaxPeakCurrent(Constants::Invalid)
	, m_batteryPercentage(Constants::Invalid)
{
}

BatteryStatusFacade::~BatteryStatusFacade(void)
{
}

Power BatteryStatusFacade::getMaxBatteryPower(void)
{
	if (m_domainProperties.implementsBatteryStatusInterface())
	{
		DptfRequest request(DptfRequestType::BatteryStatusGetMaxBatteryPower, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			m_maxBatteryPower = Power::createFromDptfBuffer(result.getData());
			return m_maxBatteryPower;
		}

		POLICY_LOG_MESSAGE_WARNING(
			{ return "Failed to get PMAX for participant " + StatusFormat::friendlyValue(m_participantIndex) + " ."; });

		result.throwIfFailure();
	}

	throw dptf_exception("No support for battery status interface");
}

const PolicyServicesInterfaceContainer& BatteryStatusFacade::getPolicyServices() const
{
	return m_policyServices;
}

DptfBuffer BatteryStatusFacade::getBatteryStatus(void)
{
	if (m_domainProperties.implementsBatteryStatusInterface())
	{
		DptfRequest request(DptfRequestType::BatteryStatusGetBatteryStatus, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			return result.getData();
		}

		POLICY_LOG_MESSAGE_WARNING({
			return "Failed to get battery status for participant " + StatusFormat::friendlyValue(m_participantIndex)
				   + " .";
		});

		result.throwIfFailure();
	}

	throw dptf_exception("No support for battery status interface");
}

DptfBuffer BatteryStatusFacade::getBatteryInformation(void)
{
	if (m_domainProperties.implementsBatteryStatusInterface())
	{
		DptfRequest request(DptfRequestType::BatteryStatusGetBatteryInformation, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			return result.getData();
		}

		POLICY_LOG_MESSAGE_WARNING({
			return "Failed to get battery information for participant "
				   + StatusFormat::friendlyValue(m_participantIndex) + " .";
		});

		result.throwIfFailure();
	}

	throw dptf_exception("No support for battery status interface");
}

ChargerType::Type BatteryStatusFacade::getChargerType(void)
{
	if (m_domainProperties.implementsBatteryStatusInterface())
	{
		DptfRequest request(DptfRequestType::BatteryStatusGetChargerType, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			m_chargerType = ChargerType::createFromDptfBuffer(result.getData());
			return m_chargerType;
		}

		POLICY_LOG_MESSAGE_WARNING(
			{ return "Failed to get CTYP for participant " + StatusFormat::friendlyValue(m_participantIndex) + " ."; });

		result.throwIfFailure();
	}

	throw dptf_exception("No support for battery status interface");
}

Power BatteryStatusFacade::getPlatformBatterySteadyState(void)
{
	if (m_domainProperties.implementsBatteryStatusInterface())
	{
		DptfRequest request(DptfRequestType::BatteryStatusGetBatterySteadyState, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			m_batterySteadyState = Power::createFromDptfBuffer(result.getData());
			return m_batterySteadyState;
		}

		POLICY_LOG_MESSAGE_WARNING(
			{ return "Failed to get PBSS for participant " + StatusFormat::friendlyValue(m_participantIndex) + " ."; });

		result.throwIfFailure();
	}

	throw dptf_exception("No support for battery status interface");
}

UInt32 BatteryStatusFacade::getBatteryHighFrequencyImpedance(void)
{
	if (m_domainProperties.implementsBatteryStatusInterface())
	{
		DptfRequest request(
			DptfRequestType::BatteryStatusGetBatteryHighFrequencyImpedance, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			m_batteryHighFrequencyImpedance = result.getDataAsUInt32();
			return m_batteryHighFrequencyImpedance;
		}

		POLICY_LOG_MESSAGE_WARNING(
			{ return "Failed to get RBHF for participant " + StatusFormat::friendlyValue(m_participantIndex) + " ."; });

		result.throwIfFailure();
	}

	throw dptf_exception("No support for battery status interface");
}

UInt32 BatteryStatusFacade::getBatteryNoLoadVoltage(void)
{
	if (m_domainProperties.implementsBatteryStatusInterface())
	{
		DptfRequest request(DptfRequestType::BatteryStatusGetBatteryNoLoadVoltage, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			m_batteryNoLoadVoltage = result.getDataAsUInt32();
			return m_batteryNoLoadVoltage;
		}

		POLICY_LOG_MESSAGE_WARNING(
			{ return "Failed to get VBNL for participant " + StatusFormat::friendlyValue(m_participantIndex) + " ."; });

		result.throwIfFailure();
	}

	throw dptf_exception("No support for battery status interface");
}

UInt32 BatteryStatusFacade::getBatteryMaxPeakCurrent(void)
{
	if (m_domainProperties.implementsBatteryStatusInterface())
	{
		DptfRequest request(DptfRequestType::BatteryStatusGetBatteryMaxPeakCurrent, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			m_batteryMaxPeakCurrent = result.getDataAsUInt32();
			return m_batteryMaxPeakCurrent;
		}

		POLICY_LOG_MESSAGE_WARNING(
			{ return "Failed to get CMPP for participant " + StatusFormat::friendlyValue(m_participantIndex) + " ."; });

		result.throwIfFailure();
	}

	throw dptf_exception("No support for battery status interface");
}

Percentage BatteryStatusFacade::getBatteryPercentage(void)
{
	if (m_domainProperties.implementsBatteryStatusInterface())
	{
		DptfRequest request(DptfRequestType::BatteryStatusGetBatteryPercentage, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			m_batteryPercentage = result.getDataAsUInt32();
			return Percentage::fromWholeNumber(m_batteryPercentage);
		}

		POLICY_LOG_MESSAGE_WARNING({
			return "Failed to get battery percentage for participant " + StatusFormat::friendlyValue(m_participantIndex)
				   + " .";
		});

		result.throwIfFailure();
	}

	throw dptf_exception("No support for battery status interface");
}

void BatteryStatusFacade::setBatteryPercentage(Percentage batteryPercentage)
{
	DptfRequest request(DptfRequestType::BatteryStatusSetBatteryPercentage, batteryPercentage.toDptfBuffer(), m_participantIndex, m_domainIndex);
	auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
}

std::shared_ptr<XmlNode> BatteryStatusFacade::getXml() const
{
	auto control = XmlNode::createWrapperElement("battery_status");
	control->addChild(XmlNode::createDataElement("max_battery_power", m_maxBatteryPower.toString()));
	std::string chargerTypeString = Constants::InvalidString;
	try
	{
		chargerTypeString = ChargerType::ToString(m_chargerType);
	}
	catch (dptf_exception&)
	{
		// do nothing, CTYP not supported
	}
	control->addChild(XmlNode::createDataElement("charger_type", chargerTypeString));
	control->addChild(XmlNode::createDataElement("battery_steady_state", m_batterySteadyState.toString()));
	control->addChild(
		XmlNode::createDataElement("battery_high_freq_impedance", friendlyValue(m_batteryHighFrequencyImpedance)));
	control->addChild(XmlNode::createDataElement("battery_no_load_voltage", friendlyValue(m_batteryNoLoadVoltage)));
	control->addChild(XmlNode::createDataElement("battery_max_peak_current", friendlyValue(m_batteryMaxPeakCurrent)));
	return control;
}
