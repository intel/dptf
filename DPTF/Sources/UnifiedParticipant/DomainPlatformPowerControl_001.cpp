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

#include "DomainPlatformPowerControl_001.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;
using namespace std;

DomainPlatformPowerControl_001::DomainPlatformPowerControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPlatformPowerControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_initialState(this) // the platform power control state needs the control to capture and restore
{
	clearCachedData();
	capture();
}

DomainPlatformPowerControl_001::~DomainPlatformPowerControl_001(void)
{
	restore();
}

Bool DomainPlatformPowerControl_001::isPlatformPowerLimitEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType)
{
	return isEnabled(limitType);
}

Power DomainPlatformPowerControl_001::getPlatformPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType)
{
	throwIfLimitNotEnabled(limitType);
	throwIfTypeInvalidForPowerLimit(limitType);
	return getParticipantServices()->primitiveExecuteGetAsPower(
		esif_primitive_type::GET_PLATFORM_POWER_LIMIT, domainIndex, (UInt8)limitType);
}

void DomainPlatformPowerControl_001::setPlatformPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType,
	const Power& powerLimit)
{
	throwIfLimitNotEnabled(limitType);
	throwIfTypeInvalidForPowerLimit(limitType);
	getParticipantServices()->primitiveExecuteSetAsPower(
		esif_primitive_type::SET_PLATFORM_POWER_LIMIT, powerLimit, domainIndex, (UInt8)limitType);
}

TimeSpan DomainPlatformPowerControl_001::getPlatformPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType)
{
	throwIfLimitNotEnabled(limitType);
	throwIfTypeInvalidForTimeWindow(limitType);
	return getParticipantServices()->primitiveExecuteGetAsTimeInMilliseconds(
		esif_primitive_type::GET_PLATFORM_POWER_LIMIT_TIME_WINDOW, domainIndex, (UInt8)limitType);
}

void DomainPlatformPowerControl_001::setPlatformPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType,
	const TimeSpan& timeWindow)
{
	throwIfLimitNotEnabled(limitType);
	throwIfTypeInvalidForTimeWindow(limitType);
	getParticipantServices()->primitiveExecuteSetAsTimeInMilliseconds(
		esif_primitive_type::SET_PLATFORM_POWER_LIMIT_TIME_WINDOW, timeWindow, domainIndex, (UInt8)limitType);
}

Percentage DomainPlatformPowerControl_001::getPlatformPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType)
{
	throwIfLimitNotEnabled(limitType);
	throwIfTypeInvalidForDutyCycle(limitType);
	return getParticipantServices()->primitiveExecuteGetAsPercentage(
		esif_primitive_type::GET_PLATFORM_POWER_LIMIT_DUTY_CYCLE, domainIndex, (UInt8)limitType);
}

void DomainPlatformPowerControl_001::setPlatformPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PlatformPowerLimitType::Type limitType,
	const Percentage& dutyCycle)
{
	throwIfLimitNotEnabled(limitType);
	throwIfTypeInvalidForDutyCycle(limitType);
	getParticipantServices()->primitiveExecuteSetAsPercentage(
		esif_primitive_type::SET_PLATFORM_POWER_LIMIT_DUTY_CYCLE, dutyCycle, domainIndex, (UInt8)limitType);
}

void DomainPlatformPowerControl_001::clearCachedData(void)
{
}

std::shared_ptr<XmlNode> DomainPlatformPowerControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("platform_power_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	auto set = XmlNode::createWrapperElement("platform_power_limit_set");
	set->addChild(createStatusNode(PlatformPowerLimitType::PSysPL1));
	set->addChild(createStatusNode(PlatformPowerLimitType::PSysPL2));
	set->addChild(createStatusNode(PlatformPowerLimitType::PSysPL3));
	root->addChild(set);
	return root;
}

void DomainPlatformPowerControl_001::capture(void)
{
	m_initialState.capture();
}

void DomainPlatformPowerControl_001::restore(void)
{
	m_initialState.restore();
}

std::shared_ptr<XmlNode> DomainPlatformPowerControl_001::createStatusNode(PlatformPowerLimitType::Type limitType)
{
	auto pl = XmlNode::createWrapperElement("platform_power_limit");
	pl->addChild(XmlNode::createDataElement("type", PlatformPowerLimitType::ToString(limitType)));
	pl->addChild(XmlNode::createDataElement("enabled", createStatusStringForEnabled(limitType)));
	pl->addChild(XmlNode::createDataElement("limit_value", createStatusStringForLimitValue(limitType)));
	pl->addChild(XmlNode::createDataElement("time_window", createStatusStringForTimeWindow(limitType)));
	pl->addChild(XmlNode::createDataElement("duty_cycle", createStatusStringForDutyCycle(limitType)));
	return pl;
}

std::string DomainPlatformPowerControl_001::createStatusStringForEnabled(PlatformPowerLimitType::Type limitType)
{
	switch (limitType)
	{
	case PlatformPowerLimitType::PSysPL1:
		return friendlyValue(m_pl1Enabled);
	case PlatformPowerLimitType::PSysPL2:
		return friendlyValue(m_pl2Enabled);
	case PlatformPowerLimitType::PSysPL3:
		return friendlyValue(m_pl3Enabled);
	default:
		return "ERROR";
	}
}

std::string DomainPlatformPowerControl_001::createStatusStringForLimitValue(PlatformPowerLimitType::Type limitType)
{
	try
	{
		if (isEnabled(limitType))
		{
			Power powerLimit = getPlatformPowerLimit(getParticipantIndex(), getDomainIndex(), limitType);
			return powerLimit.toString();
		}
		else
		{
			return "DISABLED";
		}
	}
	catch (...)
	{
		return "ERROR";
	}
}

std::string DomainPlatformPowerControl_001::createStatusStringForTimeWindow(PlatformPowerLimitType::Type limitType)
{
	try
	{
		if (isEnabled(limitType)
			&& ((limitType == PlatformPowerLimitType::PSysPL1) || (limitType == PlatformPowerLimitType::PSysPL3)))
		{
			TimeSpan timeWindow = getPlatformPowerLimitTimeWindow(getParticipantIndex(), getDomainIndex(), limitType);
			return timeWindow.toStringMilliseconds();
		}
		else
		{
			return "DISABLED";
		}
	}
	catch (...)
	{
		return "ERROR";
	}
}

std::string DomainPlatformPowerControl_001::createStatusStringForDutyCycle(PlatformPowerLimitType::Type limitType)
{
	try
	{
		if (isEnabled(limitType) && (limitType == PlatformPowerLimitType::PSysPL3))
		{
			Percentage dutyCycle = getPlatformPowerLimitDutyCycle(getParticipantIndex(), getDomainIndex(), limitType);
			return dutyCycle.toString();
		}
		else
		{
			return "DISABLED";
		}
	}
	catch (...)
	{
		return "ERROR";
	}
}

std::string DomainPlatformPowerControl_001::getName(void)
{
	return "Platform Power Control (Psys)";
}

void DomainPlatformPowerControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_PSYS_CONTROL;
			capability.size = sizeof(capability);

			for (UInt32 powerType = PlatformPowerLimitType::PSysPL1; powerType < PlatformPowerLimitType::MAX;
				 powerType++)
			{
				capability.data.psysControl.powerDataSet[powerType].powerLimitType =
					(PlatformPowerLimitType::Type)powerType + 1;

				try
				{
					capability.data.psysControl.powerDataSet[powerType].powerLimit = (UInt32)getPlatformPowerLimit(
						participantIndex, domainIndex, (PlatformPowerLimitType::Type)powerType);
				}
				catch (const std::exception& ex)
				{
					std::stringstream message;
					message << "Failed to get "
							<< PlatformPowerLimitType::ToString((PlatformPowerLimitType::Type)powerType)
							<< " power limit: " << ex.what();
					getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
				}

				if (powerType == PlatformPowerLimitType::PSysPL1)
				{
					try
					{
						capability.data.psysControl.powerDataSet[powerType].PowerLimitTimeWindow =
							(UInt32)getPlatformPowerLimitTimeWindow(
								participantIndex, domainIndex, (PlatformPowerLimitType::Type)powerType)
								.asMillisecondsUInt(); // 1 & 3
					}
					catch (const std::exception& ex)
					{
						std::stringstream message;
						message << "Failed to get "
								<< PlatformPowerLimitType::ToString((PlatformPowerLimitType::Type)powerType)
								<< " time window: " << ex.what();
						getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
					}
				}

				if (powerType == PlatformPowerLimitType::PSysPL3)
				{
					try
					{
						capability.data.psysControl.powerDataSet[powerType].PowerLimitTimeWindow =
							(UInt32)getPlatformPowerLimitTimeWindow(
								participantIndex, domainIndex, (PlatformPowerLimitType::Type)powerType)
								.asMillisecondsUInt(); // 1 & 3
					}
					catch (const std::exception& ex)
					{
						std::stringstream message;
						message << "Failed to get "
								<< PlatformPowerLimitType::ToString((PlatformPowerLimitType::Type)powerType)
								<< " time window: " << ex.what();
						getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
					}

					try
					{
						capability.data.psysControl.powerDataSet[powerType].PowerLimitDutyCycle =
							(UInt32)getPlatformPowerLimitDutyCycle(
								participantIndex, domainIndex, (PlatformPowerLimitType::Type)powerType)
								.toWholeNumber(); // 3
					}
					catch (const std::exception& ex)
					{
						std::stringstream message;
						message << "Failed to get "
								<< PlatformPowerLimitType::ToString((PlatformPowerLimitType::Type)powerType)
								<< " duty cycle: " << ex.what();
						getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
					}
				}
			}

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));
		}
	}
	catch (std::exception& ex)
	{
		std::stringstream message2;
		message2 << "Error capturing activity logging data for PSYS: " << ex.what();
		getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message2.str()));
	}
}

void DomainPlatformPowerControl_001::throwIfLimitNotEnabled(PlatformPowerLimitType::Type limitType)
{
	if (isEnabled(limitType) == false)
	{
		string message = "Platform " + PlatformPowerLimitType::ToString(limitType) + " is disabled.";
		throw dptf_exception(message);
	}
}

void DomainPlatformPowerControl_001::throwIfTypeInvalidForPowerLimit(PlatformPowerLimitType::Type limitType)
{
	switch (limitType)
	{
	case PlatformPowerLimitType::PSysPL1:
	case PlatformPowerLimitType::PSysPL2:
	case PlatformPowerLimitType::PSysPL3:
		return;
	default:
		throw dptf_exception("Invalid power limit type selected for Platform Power Limit.");
	}
}

void DomainPlatformPowerControl_001::throwIfTypeInvalidForTimeWindow(PlatformPowerLimitType::Type limitType)
{
	switch (limitType)
	{
	case PlatformPowerLimitType::PSysPL1:
	case PlatformPowerLimitType::PSysPL3:
		return;
	case PlatformPowerLimitType::PSysPL2:
		throw dptf_exception(
			"Platform power limit time window not supported for " + PlatformPowerLimitType::ToString(limitType) + ".");
	default:
		throw dptf_exception("Invalid power limit type selected for Platform Power Time Window.");
	}
}

void DomainPlatformPowerControl_001::throwIfTypeInvalidForDutyCycle(PlatformPowerLimitType::Type limitType)
{
	switch (limitType)
	{
	case PlatformPowerLimitType::PSysPL1:
	case PlatformPowerLimitType::PSysPL2:
		throw dptf_exception(
			"Platform power limit duty cycle not supported for " + PlatformPowerLimitType::ToString(limitType) + ".");
	case PlatformPowerLimitType::PSysPL3:
		return;
	default:
		throw dptf_exception("Invalid power limit type selected for Platform Power Duty Cycle.");
	}
}
