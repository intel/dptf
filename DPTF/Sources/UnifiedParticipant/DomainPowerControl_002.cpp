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

#include "DomainPowerControl_002.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;

DomainPowerControl_002::DomainPowerControl_002(
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
}

DomainPowerControl_002::~DomainPowerControl_002(void)
{
	restore();
}

PowerControlDynamicCapsSet DomainPowerControl_002::getPowerControlDynamicCapsSet(
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (m_powerControlDynamicCaps.isInvalid())
	{
		m_powerControlDynamicCaps.set(getDynamicCapabilities());
	}
	return m_powerControlDynamicCaps.get();
}

void DomainPowerControl_002::setPowerControlDynamicCapsSet(
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

void DomainPowerControl_002::setPowerCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	m_capabilitiesLocked = lock;
}

TimeSpan DomainPowerControl_002::getPowerSharePowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

Bool DomainPowerControl_002::isPowerShareControl(UIntN participantIndex, UIntN domainIndex)
{
	return true;
}

PowerControlDynamicCapsSet DomainPowerControl_002::getDynamicCapabilities()
{
	DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
		esif_primitive_type::GET_RAPL_POWER_CONTROL_CAPABILITIES, ESIF_DATA_BINARY, getDomainIndex());
	auto dynamicCapsSetFromControl = PowerControlDynamicCapsSet::createFromPpcc(buffer, Power::createInvalid());
	throwIfDynamicCapabilitiesAreWrong(dynamicCapsSetFromControl);
	return dynamicCapsSetFromControl;
}

Bool DomainPowerControl_002::isPowerLimitEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	return isEnabled(controlType);
}

Power DomainPowerControl_002::getPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throwIfLimitNotEnabled(controlType);
	throwIfTypeInvalidForPowerLimit(controlType);
	CachedValue<Power> plLimit;
	
	switch (controlType)
	{
	case PowerControlType::PL1:
		plLimit = getRaplPowerLimit(participantIndex, domainIndex, controlType, m_pl1Limit);
		break;
	case PowerControlType::PL2:
		plLimit = getRaplPowerLimit(participantIndex, domainIndex, controlType, m_pl2Limit);
		break;
	case PowerControlType::PL4:
		plLimit = getRaplPowerLimit(participantIndex, domainIndex, controlType, m_pl4Limit);
		break;
	default:
		throw dptf_exception("Invalid Power Control Type.");
	}

	return plLimit.get();
}

CachedValue<Power> DomainPowerControl_002::getRaplPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	CachedValue<Power> plLimit)
{
	if (plLimit.isInvalid())
	{
		try
		{
			plLimit.set(getParticipantServices()->primitiveExecuteGetAsPower(
				esif_primitive_type::GET_RAPL_POWER_LIMIT, domainIndex, (UInt8)controlType));
		}
		catch (...)
		{
			plLimit.set(getPowerControlDynamicCapsSet(participantIndex, domainIndex)
							   .getCapability(controlType)
							   .getMaxPowerLimit());
		}
	}
	return plLimit;
}

Power DomainPowerControl_002::getPowerLimitWithoutCache(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	auto pl1PowerLimit = Power::createInvalid();
	try
	{
		pl1PowerLimit = getParticipantServices()->primitiveExecuteGetAsPower(
			esif_primitive_type::GET_RAPL_POWER_LIMIT, domainIndex, (UInt8)controlType);
	}
	catch (...)
	{
		pl1PowerLimit = getPowerControlDynamicCapsSet(participantIndex, domainIndex)
							.getCapability(PowerControlType::PL1)
							.getMaxPowerLimit();
	}

	return pl1PowerLimit;
}

Bool DomainPowerControl_002::isSocPowerFloorEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

Bool DomainPowerControl_002::isSocPowerFloorSupported(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPowerControl_002::setPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	setAndUpdateEnabled(controlType);
	throwIfLimitNotEnabled(controlType);
	throwIfTypeInvalidForPowerLimit(controlType);
	throwIfPowerLimitIsOutsideCapabilityRange(controlType, powerLimit);

	switch (controlType)
	{
	case PowerControlType::PL1:
		m_pl1Limit.set(powerLimit);
		break;
	case PowerControlType::PL2:
		m_pl2Limit.set(powerLimit);
		break;
	case PowerControlType::PL4:
		m_pl4Limit.set(powerLimit);
		break;
	default:
		throw dptf_exception("Invalid Power Control Type.");
	}

	getParticipantServices()->primitiveExecuteSetAsPower(
		esif_primitive_type::SET_RAPL_POWER_LIMIT,
		powerLimit,
		domainIndex,
		Constants::Esif::NoPersistInstanceOffset + (UInt8)controlType);
	getParticipantServices()->createEventPowerLimitChanged();
}

double DomainPowerControl_002::getPidKpTerm(UIntN participantIndex, UIntN domainIndex)
{
	auto pidKpTerm =
		getParticipantServices()->primitiveExecuteGetAsUInt32(esif_primitive_type::GET_PID_KP_TERM, domainIndex);
	return ((double)(pidKpTerm / 10.0));
}

double DomainPowerControl_002::getPidKiTerm(UIntN participantIndex, UIntN domainIndex)
{
	auto pidKiTerm =
		getParticipantServices()->primitiveExecuteGetAsUInt32(esif_primitive_type::GET_PID_KI_TERM, domainIndex);
	return ((double)(pidKiTerm / 10.0));
}

TimeSpan DomainPowerControl_002::getAlpha(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsTimeInMilliseconds(
		esif_primitive_type::GET_PID_TIME_AVERAGE_CONSTANT, domainIndex, Constants::Esif::NoInstance);
}

TimeSpan DomainPowerControl_002::getFastPollTime(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsTimeInMilliseconds(
		esif_primitive_type::GET_FAST_POLL_TIME, domainIndex, Constants::Esif::NoInstance);
}

TimeSpan DomainPowerControl_002::getSlowPollTime(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsTimeInMilliseconds(
		esif_primitive_type::GET_SLOW_POLL_TIME, domainIndex, Constants::Esif::NoInstance);
}

TimeSpan DomainPowerControl_002::getWeightedSlowPollAvgConstant(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsTimeInMilliseconds(
		esif_primitive_type::GET_WEIGHTED_SLOWPOLL_CONSTANT, domainIndex, Constants::Esif::NoInstance);
}

Power DomainPowerControl_002::getSlowPollPowerThreshold(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsPower(
		esif_primitive_type::GET_SLOWPOLL_POWER_THRESHOLD, domainIndex, Constants::Esif::NoInstance);
}

void DomainPowerControl_002::setPowerLimitIgnoringCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	setAndUpdateEnabled(controlType);
	throwIfLimitNotEnabled(controlType);
	throwIfTypeInvalidForPowerLimit(controlType);

	switch (controlType)
	{
	case PowerControlType::PL1:
		m_pl1Limit.set(powerLimit);
		break;
	case PowerControlType::PL2:
		m_pl2Limit.set(powerLimit);
		break;
	case PowerControlType::PL4:
		m_pl4Limit.set(powerLimit);
		break;
	default:
		throw dptf_exception("Invalid Power Control Type.");
	}

	getParticipantServices()->primitiveExecuteSetAsPower(
		esif_primitive_type::SET_RAPL_POWER_LIMIT,
		powerLimit,
		domainIndex,
		Constants::Esif::NoPersistInstanceOffset + (UInt8)controlType);
}

TimeSpan DomainPowerControl_002::getPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throw dptf_exception("Power Limit Time Window is not supported by " + getName() + ".");
}

void DomainPowerControl_002::setPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throw dptf_exception("Power Limit Time Window is not supported by " + getName() + ".");
}

void DomainPowerControl_002::setPowerLimitTimeWindowIgnoringCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throw dptf_exception("Power Limit Time Window is not supported by " + getName() + ".");
}

Percentage DomainPowerControl_002::getPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throw dptf_exception("Power Limit Duty Cycle is not supported by " + getName() + ".");
}

void DomainPowerControl_002::setPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Percentage& dutyCycle)
{
	throw dptf_exception("Power Limit Duty Cycle is not supported by " + getName() + ".");
}

void DomainPowerControl_002::setSocPowerFloorState(UIntN participantIndex, UIntN domainIndex, Bool socPowerFloorState)
{
	throw dptf_exception("SoC Power Floor is not supported by " + getName() + ".");
}

void DomainPowerControl_002::setAndUpdateEnabled(PowerControlType::Type controlType)
{
	if (!isEnabled(controlType))
	{
		switch (controlType)
		{
		case PowerControlType::PL1:
			setEnabled(controlType, true);
			updateEnabled(controlType);
			break;
		case PowerControlType::PL2:
		case PowerControlType::PL3:
		case PowerControlType::PL4:
		case PowerControlType::max:
			// do nothing
			break;
		default:
			// do nothing
			break;
		}
	}
}

void DomainPowerControl_002::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_POWER_CONTROL;
			capability.size = sizeof(capability);
			UInt32 powerType = PowerControlType::PL1;

			capability.data.powerControl.powerDataSet[powerType].isEnabled =
				(UInt32)isEnabled((PowerControlType::Type)powerType);
			if (capability.data.powerControl.powerDataSet[powerType].isEnabled == (UInt32) true)
			{
				capability.data.powerControl.powerDataSet[powerType].powerType = (PowerControlType::Type)powerType + 1;
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
					capability.data.powerControl.powerDataSet[powerType].stepsize = powerControlCaps.getPowerStepSize();
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

void DomainPowerControl_002::onClearCachedData(void)
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
			PARTICIPANT_LOG_MESSAGE_DEBUG(
				{ return "Failed to restore the initial power share control capabilities. "; });
		}
	}
}

std::shared_ptr<XmlNode> DomainPowerControl_002::getXml(UIntN domainIndex)
{
	auto participantIndex = getParticipantIndex();

	auto root = XmlNode::createWrapperElement("power_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "002"));
	root->addChild(getPowerControlDynamicCapsSet(participantIndex, domainIndex).getXml());

	auto set = XmlNode::createWrapperElement("power_limit_set");
	set->addChild(createStatusNode(PowerControlType::PL1));
	set->addChild(createStatusNode(PowerControlType::PL2));
	set->addChild(createStatusNode(PowerControlType::PL3));
	set->addChild(createStatusNode(PowerControlType::PL4));
	root->addChild(set);

	auto socPowerFloorStatus = XmlNode::createWrapperElement("soc_power_floor_status");
	socPowerFloorStatus->addChild(XmlNode::createDataElement(
		"is_soc_power_floor_supported", friendlyValue(false)));
	socPowerFloorStatus->addChild(XmlNode::createDataElement(
		"soc_power_floor_state", friendlyValue(false)));
	root->addChild(socPowerFloorStatus);

	return root;
}

void DomainPowerControl_002::capture(void)
{
	m_initialState.capture();
}

void DomainPowerControl_002::restore(void)
{
	try
	{
		DptfBuffer powerLimitBuffer = createResetPrimitiveTupleBinary(
			esif_primitive_type::SET_RAPL_POWER_LIMIT,
			Constants::Esif::NoPersistInstanceOffset + (UInt8)PowerControlType::PL1);
		getParticipantServices()->primitiveExecuteSet(
			esif_primitive_type::SET_CONFIG_RESET,
			ESIF_DATA_BINARY,
			powerLimitBuffer.get(),
			powerLimitBuffer.size(),
			powerLimitBuffer.size(),
			0,
			Constants::Esif::NoInstance);
	}
	catch (...)
	{
		// best effort
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to restore the power share limit to initial state. "; });
	}
}

std::shared_ptr<XmlNode> DomainPowerControl_002::createStatusNode(PowerControlType::Type controlType)
{
	auto pl = XmlNode::createWrapperElement("power_limit");
	pl->addChild(XmlNode::createDataElement("type", PowerControlType::ToString(controlType)));
	pl->addChild(XmlNode::createDataElement("enabled", createStatusStringForEnabled(controlType)));
	pl->addChild(XmlNode::createDataElement("limit_value", createStatusStringForLimitValue(controlType)));
	pl->addChild(XmlNode::createDataElement("time_window", "NOT SUPPORTED"));
	pl->addChild(XmlNode::createDataElement("duty_cycle", "NOT SUPPORTED"));
	return pl;
}

std::string DomainPowerControl_002::createStatusStringForEnabled(PowerControlType::Type controlType)
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

std::string DomainPowerControl_002::createStatusStringForLimitValue(PowerControlType::Type controlType)
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

std::string DomainPowerControl_002::getName(void)
{
	return "Power Share Control";
}

void DomainPowerControl_002::throwIfLimitNotEnabled(PowerControlType::Type controlType)
{
	if (isEnabled(controlType) == false)
	{
		std::string message = PowerControlType::ToString(controlType) + " is disabled.";
		throw dptf_exception(message);
	}
}

void DomainPowerControl_002::throwIfTypeInvalidForPowerLimit(PowerControlType::Type controlType)
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

void DomainPowerControl_002::throwIfDynamicCapabilitiesAreWrong(const PowerControlDynamicCapsSet& capabilities)
{
	if (capabilities.isEmpty())
	{
		throw dptf_exception("Dynamic caps set is empty.  Impossible if we support power controls.");
	}

	auto controlTypes = capabilities.getControlTypes();
	for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); controlType++)
	{
		auto capability = capabilities.getCapability(*controlType);
		std::string controlTypeString = PowerControlType::ToString(capability.getPowerControlType());
		
		auto maxPowerLimit = capability.getMaxPowerLimit();
		auto minPowerLimit = capability.getMinPowerLimit();
		if (maxPowerLimit.isValid() && minPowerLimit.isValid() && maxPowerLimit < minPowerLimit)
		{
			std::string errorMessage = controlTypeString + " has bad power limit capabilities: max < min.";
			throw dptf_exception(errorMessage);
		}

		auto maxTimeWindow = capability.getMaxTimeWindow();
		auto minTimeWindow = capability.getMinTimeWindow();
		if (maxTimeWindow.isValid() && minTimeWindow.isValid() && maxTimeWindow < minTimeWindow)
		{
			std::string errorMessage = controlTypeString + " has bad time window capabilities: max < min.";
			throw dptf_exception(errorMessage);
		}

		auto maxDutyCycle = capability.getMaxDutyCycle();
		auto minDutyCycle = capability.getMinDutyCycle();
		if (maxDutyCycle.isValid() && minDutyCycle.isValid() && maxDutyCycle < minDutyCycle)
		{
			std::string errorMessage = controlTypeString + " has bad duty cycle capabilities: max < min.";
			throw dptf_exception(errorMessage);
		}
	}
}

void DomainPowerControl_002::throwIfPowerLimitIsOutsideCapabilityRange(
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	auto capabilities = getPowerControlDynamicCapsSet(getParticipantIndex(), getDomainIndex());
	if (capabilities.hasCapability(controlType))
	{
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

void DomainPowerControl_002::removePowerLimitPolicyRequest(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	// Do nothing.  Not an error.
}

void DomainPowerControl_002::setPowerSharePolicyPower(
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