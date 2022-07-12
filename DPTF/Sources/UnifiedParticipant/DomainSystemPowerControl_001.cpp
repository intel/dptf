/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "DomainSystemPowerControl_001.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;
using namespace std;

DomainSystemPowerControl_001::DomainSystemPowerControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainSystemPowerControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_initialState(this) // the System power control state needs the control to capture and restore
{
	onClearCachedData();
	capture();
}

DomainSystemPowerControl_001::~DomainSystemPowerControl_001(void)
{
	restore();
}

Bool DomainSystemPowerControl_001::isSystemPowerLimitEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	return isEnabled(limitType);
}

Power DomainSystemPowerControl_001::getSystemPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throwIfLimitNotEnabled(limitType);
	throwIfTypeInvalidForPowerLimit(limitType);
	return getParticipantServices()->primitiveExecuteGetAsPower(
		esif_primitive_type::GET_PLATFORM_POWER_LIMIT, domainIndex, (UInt8)limitType);
}

void DomainSystemPowerControl_001::setSystemPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType,
	const Power& powerLimit)
{
	throwIfLimitNotEnabled(limitType);
	throwIfTypeInvalidForPowerLimit(limitType);
	getParticipantServices()->primitiveExecuteSetAsPower(
		esif_primitive_type::SET_PLATFORM_POWER_LIMIT, powerLimit, domainIndex, (UInt8)limitType);
}

TimeSpan DomainSystemPowerControl_001::getSystemPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throwIfLimitNotEnabled(limitType);
	throwIfTypeInvalidForTimeWindow(limitType);
	return getParticipantServices()->primitiveExecuteGetAsTimeInMilliseconds(
		esif_primitive_type::GET_PLATFORM_POWER_LIMIT_TIME_WINDOW, domainIndex, (UInt8)limitType);
}

void DomainSystemPowerControl_001::setSystemPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType,
	const TimeSpan& timeWindow)
{
	throwIfLimitNotEnabled(limitType);
	throwIfTypeInvalidForTimeWindow(limitType);
	getParticipantServices()->primitiveExecuteSetAsTimeInMilliseconds(
		esif_primitive_type::SET_PLATFORM_POWER_LIMIT_TIME_WINDOW, timeWindow, domainIndex, (UInt8)limitType);
}

Percentage DomainSystemPowerControl_001::getSystemPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throwIfLimitNotEnabled(limitType);
	throwIfTypeInvalidForDutyCycle(limitType);
	return getParticipantServices()->primitiveExecuteGetAsPercentage(
		esif_primitive_type::GET_PLATFORM_POWER_LIMIT_DUTY_CYCLE, domainIndex, (UInt8)limitType);
}

void DomainSystemPowerControl_001::setSystemPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType,
	const Percentage& dutyCycle)
{
	throwIfLimitNotEnabled(limitType);
	throwIfTypeInvalidForDutyCycle(limitType);
	getParticipantServices()->primitiveExecuteSetAsPercentage(
		esif_primitive_type::SET_PLATFORM_POWER_LIMIT_DUTY_CYCLE, dutyCycle, domainIndex, (UInt8)limitType);
}

void DomainSystemPowerControl_001::onClearCachedData(void)
{
}

std::shared_ptr<XmlNode> DomainSystemPowerControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("system_power_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	auto set = XmlNode::createWrapperElement("system_power_limit_set");
	set->addChild(createStatusNode(PsysPowerLimitType::PSysPL1));
	set->addChild(createStatusNode(PsysPowerLimitType::PSysPL2));
	set->addChild(createStatusNode(PsysPowerLimitType::PSysPL3));
	root->addChild(set);
	return root;
}

void DomainSystemPowerControl_001::capture(void)
{
	m_initialState.capture();
}

void DomainSystemPowerControl_001::restore(void)
{
	m_initialState.restore();
}

std::shared_ptr<XmlNode> DomainSystemPowerControl_001::createStatusNode(PsysPowerLimitType::Type limitType)
{
	auto pl = XmlNode::createWrapperElement("system_power_limit");
	pl->addChild(XmlNode::createDataElement("type", PsysPowerLimitType::ToString(limitType)));
	pl->addChild(XmlNode::createDataElement("enabled", createStatusStringForEnabled(limitType)));
	pl->addChild(XmlNode::createDataElement("limit_value", createStatusStringForLimitValue(limitType)));
	pl->addChild(XmlNode::createDataElement("time_window", createStatusStringForTimeWindow(limitType)));
	pl->addChild(XmlNode::createDataElement("duty_cycle", createStatusStringForDutyCycle(limitType)));
	return pl;
}

std::string DomainSystemPowerControl_001::createStatusStringForEnabled(PsysPowerLimitType::Type limitType)
{
	switch (limitType)
	{
	case PsysPowerLimitType::PSysPL1:
		return friendlyValue(m_pl1Enabled);
	case PsysPowerLimitType::PSysPL2:
		return friendlyValue(m_pl2Enabled);
	case PsysPowerLimitType::PSysPL3:
		return friendlyValue(m_pl3Enabled);
	default:
		return "ERROR";
	}
}

std::string DomainSystemPowerControl_001::createStatusStringForLimitValue(PsysPowerLimitType::Type limitType)
{
	try
	{
		if (isEnabled(limitType))
		{
			Power powerLimit = getSystemPowerLimit(getParticipantIndex(), getDomainIndex(), limitType);
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

std::string DomainSystemPowerControl_001::createStatusStringForTimeWindow(PsysPowerLimitType::Type limitType)
{
	try
	{
		if (isEnabled(limitType)
			&& ((limitType == PsysPowerLimitType::PSysPL1) || (limitType == PsysPowerLimitType::PSysPL3)))
		{
			TimeSpan timeWindow = getSystemPowerLimitTimeWindow(getParticipantIndex(), getDomainIndex(), limitType);
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

std::string DomainSystemPowerControl_001::createStatusStringForDutyCycle(PsysPowerLimitType::Type limitType)
{
	try
	{
		if (isEnabled(limitType) && (limitType == PsysPowerLimitType::PSysPL3))
		{
			Percentage dutyCycle = getSystemPowerLimitDutyCycle(getParticipantIndex(), getDomainIndex(), limitType);
			return dutyCycle.toStringWithPrecision(0);
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

std::string DomainSystemPowerControl_001::getName(void)
{
	return "System Power Control (Psys)";
}

void DomainSystemPowerControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_PSYS_CONTROL;
			capability.size = sizeof(capability);

			for (UInt32 powerType = PsysPowerLimitType::PSysPL1; powerType < PsysPowerLimitType::MAX; powerType++)
			{
				capability.data.psysControl.powerDataSet[powerType].powerLimitType =
					(PsysPowerLimitType::Type)powerType + 1;

				capability.data.psysControl.powerDataSet[powerType].powerLimit = Constants::Invalid;
				capability.data.psysControl.powerDataSet[powerType].PowerLimitTimeWindow = Constants::Invalid;
				capability.data.psysControl.powerDataSet[powerType].PowerLimitDutyCycle = Constants::Invalid;

				try
				{
					capability.data.psysControl.powerDataSet[powerType].powerLimit =
						(UInt32)getSystemPowerLimit(participantIndex, domainIndex, (PsysPowerLimitType::Type)powerType);
				}
				catch (const std::exception& ex)
				{
					PARTICIPANT_LOG_MESSAGE_DEBUG_EX({
						std::stringstream message;
						message << "Failed to get " << PsysPowerLimitType::ToString((PsysPowerLimitType::Type)powerType)
								<< " power limit: " << ex.what();
						return message.str();
					});
				}

				if (powerType == PsysPowerLimitType::PSysPL1)
				{
					try
					{
						capability.data.psysControl.powerDataSet[powerType].PowerLimitTimeWindow =
							(UInt32)getSystemPowerLimitTimeWindow(
								participantIndex, domainIndex, (PsysPowerLimitType::Type)powerType)
								.asMillisecondsUInt(); // 1 & 3
					}
					catch (const std::exception& ex)
					{
						PARTICIPANT_LOG_MESSAGE_DEBUG_EX({
							std::stringstream message;
							message << "Failed to get "
									<< PsysPowerLimitType::ToString((PsysPowerLimitType::Type)powerType)
									<< " time window: " << ex.what();
							return message.str();
						});
					}
				}

				if (powerType == PsysPowerLimitType::PSysPL3)
				{
					try
					{
						capability.data.psysControl.powerDataSet[powerType].PowerLimitTimeWindow =
							(UInt32)getSystemPowerLimitTimeWindow(
								participantIndex, domainIndex, (PsysPowerLimitType::Type)powerType)
								.asMillisecondsUInt(); // 1 & 3
					}
					catch (const std::exception& ex)
					{
						PARTICIPANT_LOG_MESSAGE_DEBUG_EX({
							std::stringstream message;
							message << "Failed to get "
									<< PsysPowerLimitType::ToString((PsysPowerLimitType::Type)powerType)
									<< " time window: " << ex.what();
							return message.str();
						});
					}

					try
					{
						capability.data.psysControl.powerDataSet[powerType].PowerLimitDutyCycle =
							(UInt32)getSystemPowerLimitDutyCycle(
								participantIndex, domainIndex, (PsysPowerLimitType::Type)powerType)
								.toWholeNumber(); // 3
					}
					catch (const std::exception& ex)
					{
						PARTICIPANT_LOG_MESSAGE_DEBUG_EX({
							std::stringstream message;
							message << "Failed to get "
									<< PsysPowerLimitType::ToString((PsysPowerLimitType::Type)powerType)
									<< " duty cycle: " << ex.what();
							return message.str();
						});
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
		PARTICIPANT_LOG_MESSAGE_DEBUG_EX({
			std::stringstream message2;
			message2 << "Error capturing activity logging data for PSYS: " << ex.what();
			return message2.str();
		});
	}
}

void DomainSystemPowerControl_001::throwIfLimitNotEnabled(PsysPowerLimitType::Type limitType)
{
	if (isEnabled(limitType) == false)
	{
		string message = "System " + PsysPowerLimitType::ToString(limitType) + " is disabled.";
		throw dptf_exception(message);
	}
}

void DomainSystemPowerControl_001::throwIfTypeInvalidForPowerLimit(PsysPowerLimitType::Type limitType)
{
	switch (limitType)
	{
	case PsysPowerLimitType::PSysPL1:
	case PsysPowerLimitType::PSysPL2:
	case PsysPowerLimitType::PSysPL3:
		return;
	default:
		throw dptf_exception("Invalid power limit type selected for System Power Limit.");
	}
}

void DomainSystemPowerControl_001::throwIfTypeInvalidForTimeWindow(PsysPowerLimitType::Type limitType)
{
	switch (limitType)
	{
	case PsysPowerLimitType::PSysPL1:
	case PsysPowerLimitType::PSysPL3:
		return;
	case PsysPowerLimitType::PSysPL2:
		throw dptf_exception(
			"System power limit time window not supported for " + PsysPowerLimitType::ToString(limitType) + ".");
	default:
		throw dptf_exception("Invalid power limit type selected for System Power Time Window.");
	}
}

void DomainSystemPowerControl_001::throwIfTypeInvalidForDutyCycle(PsysPowerLimitType::Type limitType)
{
	switch (limitType)
	{
	case PsysPowerLimitType::PSysPL1:
	case PsysPowerLimitType::PSysPL2:
		throw dptf_exception(
			"System power limit duty cycle not supported for " + PsysPowerLimitType::ToString(limitType) + ".");
	case PsysPowerLimitType::PSysPL3:
		return;
	default:
		throw dptf_exception("Invalid power limit type selected for System Power Duty Cycle.");
	}
}
