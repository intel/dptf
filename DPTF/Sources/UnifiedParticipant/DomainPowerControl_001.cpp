/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "DomainPowerControl_001.h"
#include "XmlNode.h"
#include "StatusFormat.h"
#include <cmath>
#include "ParticipantLogger.h"
using namespace StatusFormat;

DomainPowerControl_001::DomainPowerControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPowerControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_initialState(this)
	, // the power control state needs the control to capture and restore
	m_capabilitiesLocked(false)
{
	onClearCachedData();
	capture();
	m_socPowerFloorSupported.invalidate();
}

DomainPowerControl_001::~DomainPowerControl_001(void)
{
	restore();
}

PowerControlDynamicCapsSet DomainPowerControl_001::getPowerControlDynamicCapsSet(
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (m_powerControlDynamicCaps.isInvalid())
	{
		m_powerControlDynamicCaps.set(getDynamicCapabilities());
	}
	return m_powerControlDynamicCaps.get();
}

void DomainPowerControl_001::setPowerControlDynamicCapsSet(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlDynamicCapsSet capsSet)
{
	DptfBuffer buffer = capsSet.toPpccBinary();
	getParticipantServices()->primitiveExecuteSet(
		esif_primitive_type::SET_RAPL_POWER_CONTROL_CAPABILITIES,
		ESIF_DATA_BINARY,
		buffer.get(),
		buffer.size(),
		buffer.size(),
		domainIndex,
		Constants::Esif::NoPersistInstance);
	m_powerControlDynamicCaps.set(getDynamicCapabilities());
}

void DomainPowerControl_001::setPowerCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	m_capabilitiesLocked = lock;
}

TimeSpan DomainPowerControl_001::getPowerSharePowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsTimeInMilliseconds(
		esif_primitive_type::GET_POWERSHARE_POWER_LIMIT_TIME_WINDOW, domainIndex, Constants::Esif::NoInstance);
}

Bool DomainPowerControl_001::isPowerShareControl(UIntN participantIndex, UIntN domainIndex)
{
	return false;
}

PowerControlDynamicCapsSet DomainPowerControl_001::getDynamicCapabilities()
{
	DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_RAPL_POWER_CONTROL_CAPABILITIES, ESIF_DATA_BINARY, getDomainIndex());

	auto pl4PowerLimit = Power::createInvalid();
	try
	{
		pl4PowerLimit = getPowerLimit(getParticipantIndex(), getDomainIndex(), PowerControlType::PL4);
	}
	catch (...)
	{
	}

	auto dynamicCapsSetFromControl = PowerControlDynamicCapsSet::createFromPpcc(buffer, pl4PowerLimit);
	throwIfDynamicCapabilitiesAreEmpty(dynamicCapsSetFromControl);
	return dynamicCapsSetFromControl;
}

Bool DomainPowerControl_001::isPowerLimitEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	return isEnabled(controlType);
}

Power DomainPowerControl_001::getPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throwIfLimitNotEnabled(controlType);
	throwIfTypeInvalidForPowerLimit(controlType);
	return getParticipantServices()->primitiveExecuteGetAsPower(
		esif_primitive_type::GET_RAPL_POWER_LIMIT, domainIndex, (UInt8)controlType);
}

Power DomainPowerControl_001::getPowerLimitWithoutCache(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throw dptf_exception("Get Power Limit Without Cache is not supported by " + getName() + ".");
}

Bool DomainPowerControl_001::isSocPowerFloorEnabled(UIntN participantIndex, UIntN domainIndex)
{
	auto isSocPowerFloorEnabled = false;

	if (isSocPowerFloorSupported(participantIndex, domainIndex))
	{
		try
		{
			UInt32 socPowerFloor = getParticipantServices()->primitiveExecuteGetAsUInt32(
				esif_primitive_type::GET_RAPL_FLOOR_STATE, domainIndex);
			isSocPowerFloorEnabled = socPowerFloor > 0 ? true : false;
		}
		catch (...)
		{
			PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to get Rapl Floor State"; });
		}
	}

	return isSocPowerFloorEnabled;
}

Bool DomainPowerControl_001::isSocPowerFloorSupported(UIntN participantIndex, UIntN domainIndex)
{
	if (m_socPowerFloorSupported.isInvalid())
	{
		try
		{
			getParticipantServices()->primitiveExecuteGetAsUInt32(
				esif_primitive_type::GET_RAPL_FLOOR_STATE, domainIndex);
			m_socPowerFloorSupported.set(true);
		}
		catch (...)
		{
			PARTICIPANT_LOG_MESSAGE_DEBUG(
				{ return "Failed to get Rapl Floor State. Hence, SoC Power Floor is not supported"; });
			m_socPowerFloorSupported.set(false);
		}
	}

	return m_socPowerFloorSupported.get();
}

void DomainPowerControl_001::setPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	throwIfTypeInvalidForPowerLimit(controlType);
	throwIfPowerLimitIsOutsideCapabilityRange(controlType, powerLimit);
	getParticipantServices()->primitiveExecuteSetAsPower(
		esif_primitive_type::SET_RAPL_POWER_LIMIT, powerLimit, domainIndex, (UInt8)controlType);
	PARTICIPANT_LOG_MESSAGE_DEBUG({
		std::stringstream message;
		message << "Successfully set RAPL Power Limit of control type " << PowerControlType::ToString(controlType)
				<< " to " << std::to_string(powerLimit);
		return message.str();
	});
	setAndUpdateEnabled(controlType);
	throwIfLimitNotEnabled(controlType);
}

void DomainPowerControl_001::setPowerLimitWithoutUpdatingEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	throwIfTypeInvalidForPowerLimit(controlType);
	throwIfPowerLimitIsOutsideCapabilityRange(controlType, powerLimit);
	getParticipantServices()->primitiveExecuteSetAsPower(
		esif_primitive_type::SET_RAPL_POWER_LIMIT, powerLimit, domainIndex, (UInt8)controlType);
	PARTICIPANT_LOG_MESSAGE_DEBUG({
		std::stringstream message;
		message << "Successfully set RAPL Power Limit of control type " << PowerControlType::ToString(controlType)
				<< " to " << std::to_string(powerLimit);
		return message.str();
	});
}

void DomainPowerControl_001::setPowerLimitIgnoringCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	throwIfTypeInvalidForPowerLimit(controlType);
	getParticipantServices()->primitiveExecuteSetAsPower(
		esif_primitive_type::SET_RAPL_POWER_LIMIT, powerLimit, domainIndex, (UInt8)controlType);
	setAndUpdateEnabled(controlType);
	throwIfLimitNotEnabled(controlType);
}

TimeSpan DomainPowerControl_001::getPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throwIfLimitNotEnabled(controlType);
	throwIfTypeInvalidForTimeWindow(controlType);
	return getParticipantServices()->primitiveExecuteGetAsTimeInMilliseconds(
		esif_primitive_type::GET_RAPL_POWER_LIMIT_TIME_WINDOW, domainIndex, (UInt8)controlType);
}

void DomainPowerControl_001::setPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throwIfTypeInvalidForTimeWindow(controlType);
	throwIfTimeWindowIsOutsideCapabilityRange(controlType, timeWindow);
	getParticipantServices()->primitiveExecuteSetAsTimeInMilliseconds(
		esif_primitive_type::SET_RAPL_POWER_LIMIT_TIME_WINDOW, timeWindow, domainIndex, (UInt8)controlType);
	setAndUpdateEnabled(controlType);
	throwIfLimitNotEnabled(controlType);
	getParticipantServices()->createEventPowerLimitTimeWindowChanged();
}

void DomainPowerControl_001::setPowerLimitTimeWindowIgnoringCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throwIfTypeInvalidForTimeWindow(controlType);
	getParticipantServices()->primitiveExecuteSetAsTimeInMilliseconds(
		esif_primitive_type::SET_RAPL_POWER_LIMIT_TIME_WINDOW, timeWindow, domainIndex, (UInt8)controlType);
	setAndUpdateEnabled(controlType);
	throwIfLimitNotEnabled(controlType);
}

void DomainPowerControl_001::setPowerLimitTimeWindowWithoutUpdatingEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throwIfTypeInvalidForTimeWindow(controlType);
	throwIfTimeWindowIsOutsideCapabilityRange(controlType, timeWindow);
	getParticipantServices()->primitiveExecuteSetAsTimeInMilliseconds(
		esif_primitive_type::SET_RAPL_POWER_LIMIT_TIME_WINDOW, timeWindow, domainIndex, (UInt8)controlType);
}

Percentage DomainPowerControl_001::getPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throwIfLimitNotEnabled(controlType);
	throwIfTypeInvalidForDutyCycle(controlType);
	return getParticipantServices()->primitiveExecuteGetAsPercentage(
		esif_primitive_type::GET_RAPL_POWER_LIMIT_DUTY_CYCLE, domainIndex, (UInt8)controlType);
}

void DomainPowerControl_001::setPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Percentage& dutyCycle)
{
	throwIfTypeInvalidForDutyCycle(controlType);
	throwIfDutyCycleIsOutsideCapabilityRange(controlType, dutyCycle);
	getParticipantServices()->primitiveExecuteSetAsPercentage(
		esif_primitive_type::SET_RAPL_POWER_LIMIT_DUTY_CYCLE, dutyCycle, domainIndex, (UInt8)controlType);
	setAndUpdateEnabled(controlType);
	throwIfLimitNotEnabled(controlType);
}

void DomainPowerControl_001::setSocPowerFloorState(UIntN participantIndex, UIntN domainIndex, Bool socPowerFloorState)
{
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_RAPL_FLOOR_STATE, socPowerFloorState ? (UInt32)1 : (UInt32)0, domainIndex);
}

void DomainPowerControl_001::clearPowerLimit(UIntN participantIndex, UIntN domainIndex)
{
	const UInt32 unusedValue = 0;
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_RAPL_POWER_LIMIT_CLEAR, unusedValue, domainIndex);
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_RAPL_POWER_MIN_CLEAR, unusedValue, domainIndex);
}

void DomainPowerControl_001::setAndUpdateEnabled(PowerControlType::Type controlType)
{
	if (!isEnabled(controlType))
	{
		setEnabled(controlType, true);
		switch (controlType)
		{
		case PowerControlType::PL1:
			updateEnabled(controlType);
			break;
		case PowerControlType::PL2:
			updateEnabled(controlType);
			break;
		case PowerControlType::PL3:
			updateEnabled(controlType);
			break;
		case PowerControlType::PL4:
			updateEnabled(controlType);
			break;
		case PowerControlType::max:
			// do nothing
			break;
		default:
			// do nothing
			break;
		}
	}
}

void DomainPowerControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_POWER_CONTROL;
			capability.size = sizeof(capability);

			for (UInt32 powerType = PowerControlType::PL1; powerType < PowerControlType::max; powerType++)
			{
				capability.data.powerControl.powerDataSet[powerType].isEnabled =
					(UInt32)isEnabled((PowerControlType::Type)powerType);
				if (capability.data.powerControl.powerDataSet[powerType].isEnabled == (UInt32) true)
				{
					capability.data.powerControl.powerDataSet[powerType].powerType =
						(PowerControlType::Type)powerType + 1;
					capability.data.powerControl.powerDataSet[powerType].powerLimit =
						(UInt32)getPowerLimit(participantIndex, domainIndex, (PowerControlType::Type)powerType);
					try
					{
						PowerControlDynamicCaps powerControlCaps =
							getPowerControlDynamicCapsSet(participantIndex, domainIndex)
								.getCapability((PowerControlType::Type)powerType);
						capability.data.powerControl.powerDataSet[powerType].lowerLimit =
							powerControlCaps.getMinPowerLimit();
						capability.data.powerControl.powerDataSet[powerType].upperLimit =
							powerControlCaps.getMaxPowerLimit();
						capability.data.powerControl.powerDataSet[powerType].stepsize =
							powerControlCaps.getPowerStepSize();
						capability.data.powerControl.powerDataSet[powerType].minTimeWindow =
							(UInt32)powerControlCaps.getMinTimeWindow().asMillisecondsUInt();
						capability.data.powerControl.powerDataSet[powerType].maxTimeWindow =
							(UInt32)powerControlCaps.getMaxTimeWindow().asMillisecondsUInt();
						capability.data.powerControl.powerDataSet[powerType].minDutyCycle =
							powerControlCaps.getMinDutyCycle().toWholeNumber();
						capability.data.powerControl.powerDataSet[powerType].maxDutyCycle =
							powerControlCaps.getMaxDutyCycle().toWholeNumber();
					}
					catch (dptf_exception& ex)
					{
						PARTICIPANT_LOG_MESSAGE_DEBUG_EX({ return ex.getDescription(); });

						capability.data.powerControl.powerDataSet[powerType].lowerLimit = 0;
						capability.data.powerControl.powerDataSet[powerType].upperLimit = 0;
						capability.data.powerControl.powerDataSet[powerType].stepsize = 0;
						capability.data.powerControl.powerDataSet[powerType].minTimeWindow = 0;
						capability.data.powerControl.powerDataSet[powerType].maxTimeWindow = 0;
						capability.data.powerControl.powerDataSet[powerType].minDutyCycle = 0;
						capability.data.powerControl.powerDataSet[powerType].maxDutyCycle = 0;
					}
				}
			}
			capability.data.powerControl.socPowerFloorData.isSupported =
				isSocPowerFloorSupported(participantIndex, domainIndex) ? (UInt32)1 : (UInt32)0;
			capability.data.powerControl.socPowerFloorData.socPowerFloorState =
				isSocPowerFloorEnabled(participantIndex, domainIndex) ? (UInt32)1 : (UInt32)0;

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));

			PARTICIPANT_LOG_MESSAGE_INFO({
				std::stringstream message;
				message << "Published activity for participant " << getParticipantIndex() << ", "
						<< "domain " << getName() << " "
						<< "("
						<< "Power Control"
						<< ")";
				return message.str();
			});
		}
	}
	catch (...)
	{
		// skip if there are any issue in sending log data
	}
}

void DomainPowerControl_001::onClearCachedData(void)
{
	m_powerControlDynamicCaps.invalidate();

	if (m_capabilitiesLocked == false)
	{
		try
		{
			DptfBuffer capabilitiesBuffer = createResetPrimitiveTupleBinary(
				esif_primitive_type::SET_RAPL_POWER_CONTROL_CAPABILITIES, Constants::Esif::NoPersistInstance);
			getParticipantServices()->primitiveExecuteSet(
				esif_primitive_type::SET_CONFIG_RESET,
				ESIF_DATA_BINARY,
				capabilitiesBuffer.get(),
				capabilitiesBuffer.size(),
				capabilitiesBuffer.size(),
				0,
				Constants::Esif::NoInstance);
		}
		catch (...)
		{
			// best effort
			PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to restore the initial power control capabilities. "; });
		}
	}
}

std::shared_ptr<XmlNode> DomainPowerControl_001::getXml(UIntN domainIndex)
{
	auto participantIndex = getParticipantIndex();
	auto root = XmlNode::createWrapperElement("power_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));
	root->addChild(getPowerControlDynamicCapsSet(participantIndex, domainIndex).getXml());

	auto set = XmlNode::createWrapperElement("power_limit_set");
	set->addChild(createStatusNode(PowerControlType::PL1));
	set->addChild(createStatusNode(PowerControlType::PL2));
	set->addChild(createStatusNode(PowerControlType::PL3));
	set->addChild(createStatusNode(PowerControlType::PL4));
	root->addChild(set);

	auto socPowerFloorStatus = XmlNode::createWrapperElement("soc_power_floor_status");
	socPowerFloorStatus->addChild(XmlNode::createDataElement(
		"is_soc_power_floor_supported", friendlyValue(isSocPowerFloorSupported(participantIndex, domainIndex))));
	socPowerFloorStatus->addChild(XmlNode::createDataElement(
		"soc_power_floor_state", friendlyValue(isSocPowerFloorEnabled(participantIndex, domainIndex))));
	root->addChild(socPowerFloorStatus);

	return root;
}

void DomainPowerControl_001::capture(void)
{
	m_initialState.capture();
}

void DomainPowerControl_001::restore(void)
{
	m_initialState.restore();
}

std::shared_ptr<XmlNode> DomainPowerControl_001::createStatusNode(PowerControlType::Type controlType)
{
	auto pl = XmlNode::createWrapperElement("power_limit");
	pl->addChild(XmlNode::createDataElement("type", PowerControlType::ToString(controlType)));
	pl->addChild(XmlNode::createDataElement("enabled", createStatusStringForEnabled(controlType)));
	pl->addChild(XmlNode::createDataElement("limit_value", createStatusStringForLimitValue(controlType)));
	pl->addChild(XmlNode::createDataElement("time_window", createStatusStringForTimeWindow(controlType)));
	pl->addChild(XmlNode::createDataElement("duty_cycle", createStatusStringForDutyCycle(controlType)));
	return pl;
}

std::string DomainPowerControl_001::createStatusStringForEnabled(PowerControlType::Type controlType)
{
	switch (controlType)
	{
	case PowerControlType::PL1:
		return friendlyValue(m_pl1Enabled);
	case PowerControlType::PL2:
		return friendlyValue(m_pl2Enabled);
	case PowerControlType::PL3:
		return friendlyValue(m_pl3Enabled);
	case PowerControlType::PL4:
		return friendlyValue(m_pl4Enabled);
	default:
		return "ERROR";
	}
}

std::string DomainPowerControl_001::createStatusStringForLimitValue(PowerControlType::Type controlType)
{
	try
	{
		if (isEnabled(controlType))
		{
			Power powerLimit = getPowerLimit(getParticipantIndex(), getDomainIndex(), controlType);
			return powerLimit.toString();
		}
		else
		{
			return "DISABLED";
		}
	}
	catch (primitive_not_found_in_dsp&)
	{
		return "NOT SUPPORTED";
	}
	catch (...)
	{
		return "ERROR";
	}
}

std::string DomainPowerControl_001::createStatusStringForTimeWindow(PowerControlType::Type controlType)
{
	try
	{
		if (isEnabled(controlType)
			&& ((controlType == PowerControlType::PL1) || (controlType == PowerControlType::PL3)))
		{
			TimeSpan timeWindow = getPowerLimitTimeWindow(getParticipantIndex(), getDomainIndex(), controlType);
			return timeWindow.toStringMilliseconds();
		}
		else
		{
			return "DISABLED";
		}
	}
	catch (primitive_not_found_in_dsp&)
	{
		return "NOT SUPPORTED";
	}
	catch (...)
	{
		return "ERROR";
	}
}

std::string DomainPowerControl_001::createStatusStringForDutyCycle(PowerControlType::Type controlType)
{
	try
	{
		if (isEnabled(controlType) && (controlType == PowerControlType::PL3))
		{
			Percentage dutyCycle = getPowerLimitDutyCycle(getParticipantIndex(), getDomainIndex(), controlType);
			return dutyCycle.toStringWithPrecision(0);
		}
		else
		{
			return "DISABLED";
		}
	}
	catch (primitive_not_found_in_dsp&)
	{
		return "NOT SUPPORTED";
	}
	catch (...)
	{
		return "ERROR";
	}
}

std::string DomainPowerControl_001::getName(void)
{
	return "Power Control";
}

void DomainPowerControl_001::throwIfLimitNotEnabled(PowerControlType::Type controlType)
{
	if (isEnabled(controlType) == false)
	{
		std::string message = PowerControlType::ToString(controlType) + " is disabled.";
		throw dptf_exception(message);
	}
}

void DomainPowerControl_001::throwIfTypeInvalidForPowerLimit(PowerControlType::Type controlType)
{
	switch (controlType)
	{
	case PowerControlType::PL1:
	case PowerControlType::PL2:
	case PowerControlType::PL3:
	case PowerControlType::PL4:
		return;
	default:
		throw dptf_exception("Invalid power limit type selected for Power Limit.");
	}
}

void DomainPowerControl_001::throwIfTypeInvalidForTimeWindow(PowerControlType::Type controlType)
{
	switch (controlType)
	{
	case PowerControlType::PL1:
	case PowerControlType::PL3:
		return;
	case PowerControlType::PL2:
	case PowerControlType::PL4:
		throw dptf_exception(
			"Power limit time window not supported for " + PowerControlType::ToString(controlType) + ".");
	default:
		throw dptf_exception("Invalid power limit type selected for Power Time Window.");
	}
}

void DomainPowerControl_001::throwIfTypeInvalidForDutyCycle(PowerControlType::Type controlType)
{
	switch (controlType)
	{
	case PowerControlType::PL3:
		return;
	case PowerControlType::PL1:
	case PowerControlType::PL2:
	case PowerControlType::PL4:
		throw dptf_exception(
			"Power limit duty cycle not supported for " + PowerControlType::ToString(controlType) + ".");
	default:
		throw dptf_exception("Invalid power limit type selected for Power Duty Cycle.");
	}
}

void DomainPowerControl_001::throwIfDynamicCapabilitiesAreEmpty(const PowerControlDynamicCapsSet& capabilities)
{
	if (capabilities.isEmpty())
	{
		throw dptf_exception("Dynamic caps set is empty.  Impossible if we support power controls.");
	}
}

void DomainPowerControl_001::throwIfPowerLimitIsOutsideCapabilityRange(
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	auto capabilities = getPowerControlDynamicCapsSet(getParticipantIndex(), getDomainIndex());
	if (capabilities.hasCapability(controlType))
	{
		if (capabilities.getCapability(controlType).arePowerLimitCapsValid() == false)
		{
			throw dptf_exception("Power limit capabilities are out of order. Cannot set power limit.");
		}
		if (powerLimit > capabilities.getCapability(controlType).getMaxPowerLimit())
		{
			throw dptf_exception("Power limit is higher than maximum capability.");
		}
		if (powerLimit < capabilities.getCapability(controlType).getMinPowerLimit())
		{
			throw dptf_exception("Power limit is lower than minimum capability.");
		}
	}
}

void DomainPowerControl_001::throwIfTimeWindowIsOutsideCapabilityRange(
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	auto capabilities = getPowerControlDynamicCapsSet(getParticipantIndex(), getDomainIndex());
	if (capabilities.hasCapability(controlType))
	{
		if (capabilities.getCapability(controlType).areTimeWindowCapsValid() == false)
		{
			throw dptf_exception("Time Window capabilities are out of order. Cannot set time window.");
		}
		if (timeWindow > capabilities.getCapability(controlType).getMaxTimeWindow())
		{
			throw dptf_exception("Time Window is higher than maximum capability.");
		}
		if (timeWindow < capabilities.getCapability(controlType).getMinTimeWindow())
		{
			throw dptf_exception("Time Window is lower than minimum capability.");
		}
	}
}

void DomainPowerControl_001::throwIfDutyCycleIsOutsideCapabilityRange(
	PowerControlType::Type controlType,
	const Percentage& dutyCycle)
{
	auto capabilities = getPowerControlDynamicCapsSet(getParticipantIndex(), getDomainIndex());
	if (capabilities.hasCapability(controlType)
		&& capabilities.getCapability(controlType).areDutyCycleCapsValid() == false)
	{
		throw dptf_exception("Duty Cycle capabilities are out of order. Cannot set duty cycle.");
	}
	if (dutyCycle > Percentage(1.0))
	{
		throw dptf_exception("Duty Cycle is higher than maximum capability.");
	}
}

double DomainPowerControl_001::getPidKpTerm(UIntN participantIndex, UIntN domainIndex)
{
	throw dptf_exception("Pid Kp Term is not supported by " + getName() + ".");
}

double DomainPowerControl_001::getPidKiTerm(UIntN participantIndex, UIntN domainIndex)
{
	throw dptf_exception("Pid Ki Term is not supported by " + getName() + ".");
}

TimeSpan DomainPowerControl_001::getAlpha(UIntN participantIndex, UIntN domainIndex)
{
	throw dptf_exception("Alpha is not supported by " + getName() + ".");
}

TimeSpan DomainPowerControl_001::getFastPollTime(UIntN participantIndex, UIntN domainIndex)
{
	throw dptf_exception("Fast Poll Time is not supported by " + getName() + ".");
}

TimeSpan DomainPowerControl_001::getSlowPollTime(UIntN participantIndex, UIntN domainIndex)
{
	throw dptf_exception("Slow Poll Time is not supported by " + getName() + ".");
}

TimeSpan DomainPowerControl_001::getWeightedSlowPollAvgConstant(UIntN participantIndex, UIntN domainIndex)
{
	throw dptf_exception("Weighted Slow Poll Averaging Constant is not supported by " + getName() + ".");
}

Power DomainPowerControl_001::getSlowPollPowerThreshold(UIntN participantIndex, UIntN domainIndex)
{
	throw dptf_exception("Get Slow Poll Power Threshold is not supported by " + getName() + ".");
}

void DomainPowerControl_001::removePowerLimitPolicyRequest(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	// Do nothing.  Not an error.
}

void DomainPowerControl_001::setPowerSharePolicyPower(
	UIntN participantIndex,
	UIntN domainIndex,
	const Power& powerSharePolicyPower)
{
	try
	{
		getParticipantServices()->primitiveExecuteSetAsPower(
			esif_primitive_type::SET_POWER_SHARE_POLICY_POWER,
			powerSharePolicyPower,
			domainIndex,
			Constants::Esif::NoPersistInstance);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			std::stringstream message;
			message << "Failed to set Power Share Policy Power for participant index = "
						   + std::to_string(participantIndex) + "and domain Index = " + std::to_string(domainIndex);
			return message.str();
		});
	}
}