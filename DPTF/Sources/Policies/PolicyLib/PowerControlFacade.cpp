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

#include "PowerControlFacade.h"
#include "PolicyLogger.h"
#include "StatusFormat.h"
using namespace StatusFormat;
using namespace std;

PowerControlFacade::PowerControlFacade(
	UIntN participantIndex,
	UIntN domainIndex,
	const DomainProperties& domainProperties,
	const PolicyServicesInterfaceContainer& policyServices)
	: m_policyServices(policyServices)
	, m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
	, m_domainProperties(domainProperties)
	, m_powerStatusProperty(participantIndex, domainIndex, domainProperties, policyServices)
	, m_powerControlCapabilitiesProperty(participantIndex, domainIndex, domainProperties, policyServices)
	, m_controlsHaveBeenInitialized(false)
	, m_lastSetPowerLimit(std::map<PowerControlType::Type, Power>())
	, m_lastSetTimeWindow(std::map<PowerControlType::Type, TimeSpan>())
	, m_lastSetDutyCycle(std::map<PowerControlType::Type, Percentage>())
{
}

PowerControlFacade::~PowerControlFacade()
{
}

void PowerControlFacade::initializeControlsIfNeeded()
{
	if (supportsPowerControls())
	{
		if (m_controlsHaveBeenInitialized == false)
		{
			setControlsToMax();
			m_controlsHaveBeenInitialized = true;
		}
	}
}

Bool PowerControlFacade::supportsPowerControls(void) const
{
	return m_domainProperties.implementsPowerControlInterface();
}

Bool PowerControlFacade::supportsPowerStatus() const
{
	return m_domainProperties.implementsPowerStatusInterface();
}

Bool PowerControlFacade::isPowerShareControl() const
{
	return supportsPowerControls()
		   && m_policyServices.domainPowerControl->isPowerShareControl(m_participantIndex, m_domainIndex);
}

Power PowerControlFacade::getCurrentPower()
{
	return m_powerStatusProperty.getStatus().getCurrentPower();
}

Power PowerControlFacade::getAveragePower()
{
	throwIfControlNotSupported();
	const auto& capsSet = getCapabilities();
	if (capsSet.hasCapability(PowerControlType::PL1) && supportsPowerStatus())
	{
		const auto& caps = capsSet.getCapability(PowerControlType::PL1);
		return m_powerStatusProperty.getAveragePower(caps);
	}

	if (!capsSet.hasCapability(PowerControlType::PL1) && supportsPowerStatus())
	{
		return getCurrentPower();
	}

	if (capsSet.hasCapability(PowerControlType::PL1) && !supportsPowerStatus())
	{
		const auto& caps = capsSet.getCapability(PowerControlType::PL1);
		return caps.getMaxPowerLimit();
	}

	throw dptf_exception("Cannot perform power status action because power status in not supported on the domain.");
}

Bool PowerControlFacade::isSocPowerFloorEnabled()
{
	return m_policyServices.domainPowerControl->isSocPowerFloorEnabled(m_participantIndex, m_domainIndex);
}

Bool PowerControlFacade::isSocPowerFloorSupported()
{
	return m_policyServices.domainPowerControl->isSocPowerFloorSupported(m_participantIndex, m_domainIndex);
}

const PowerControlDynamicCapsSet& PowerControlFacade::getCapabilities()
{
	return m_powerControlCapabilitiesProperty.getDynamicCapsSet();
}

void PowerControlFacade::refreshCapabilities()
{
	m_powerControlCapabilitiesProperty.refresh();
}

void PowerControlFacade::setCapability(const PowerControlDynamicCaps& capability)
{
	auto currentCapabilitiesSet = getCapabilities();
	auto controlTypes = currentCapabilitiesSet.getControlTypes();
	PowerControlDynamicCapsSet newCapabilitiesSet;
	for (auto controlType = controlTypes.begin(); controlType != controlTypes.end(); ++controlType)
	{
		PowerControlDynamicCaps capabilitiesToSet = PowerControlDynamicCaps(
			*controlType,
			Power::createInvalid(),
			Power::createInvalid(),
			Power::createInvalid(),
			TimeSpan::createInvalid(),
			TimeSpan::createInvalid(),
			Percentage::createInvalid(),
			Percentage::createInvalid());
		newCapabilitiesSet.setCapability(capabilitiesToSet);
	}

	newCapabilitiesSet.setCapability(capability);
	m_policyServices.domainPowerControl->setPowerControlDynamicCapsSet(
		m_participantIndex, m_domainIndex, newCapabilitiesSet);
	m_powerControlCapabilitiesProperty.invalidate();
}

void PowerControlFacade::setControlsToMax()
{
	throwIfControlNotSupported();
	// Set only PL1 as Policy controls only PL1
	setPL1PowerLimitControlToMax();
}

void PowerControlFacade::setPL1PowerLimitControlToMax()
{
	const auto& capsSet = getCapabilities();

	if (capsSet.hasCapability(PowerControlType::PL1))
	{
		const auto& caps = capsSet.getCapability(PowerControlType::PL1);
		setPowerLimitPL1(caps.getMaxPowerLimit());
	}
}

void PowerControlFacade::setPL2PowerLimitControlToMax()
{
	const auto& capsSet = getCapabilities();

	if (capsSet.hasCapability(PowerControlType::PL2))
	{
		const auto& caps = capsSet.getCapability(PowerControlType::PL2);
		setPowerLimitPL2(caps.getMaxPowerLimit());
	}
}

void PowerControlFacade::setPowerLimitPL1(const Power& powerLimit)
{
	throwIfControlNotSupported();

	Int64 pl1Tau = 0;
	try
	{
		pl1Tau = getPowerLimitTimeWindowPL1().asMicroseconds();
	}
	catch (dptf_exception& ex)
	{
		POLICY_LOG_MESSAGE_WARNING_EX({
			return "Failed to get PL1 time window for participant " + StatusFormat::friendlyValue(m_participantIndex)
				   + ": " + string(ex.what());
		});
	}
	catch(...)
	{
		POLICY_LOG_MESSAGE_WARNING({
			return "Failed to get PL1 time window for participant " + StatusFormat::friendlyValue(m_participantIndex);});
	}

	try
	{
		// pl1Tau is 0 when PL1 control is disabled or PL1 Time Window value is 0
		if (pl1Tau == 0)
		{
			const auto& capsSet = getCapabilities();
			if (capsSet.hasCapability(PowerControlType::PL1))
			{
				const auto& caps = capsSet.getCapability(PowerControlType::PL1);
				m_policyServices.domainPowerControl->setPowerLimitTimeWindowWithoutUpdatingEnabled(
					m_participantIndex, m_domainIndex, PowerControlType::PL1, caps.getMaxTimeWindow());
				m_lastSetTimeWindow[PowerControlType::PL1] = caps.getMaxTimeWindow();
			}
		}
	}
	catch (dptf_exception& ex)
	{
		POLICY_LOG_MESSAGE_WARNING_EX({
			return "Failed to set PL1 time window without updating enabled for participant "
				   + StatusFormat::friendlyValue(m_participantIndex) + ": " + string(ex.what());
		});
	}
	catch(...)
	{
		POLICY_LOG_MESSAGE_WARNING({
			return "Failed to set PL1 time window without updating enabled for participant " + StatusFormat::friendlyValue(m_participantIndex);});
	}

	m_policyServices.domainPowerControl->setPowerLimit(
		m_participantIndex, m_domainIndex, PowerControlType::PL1, powerLimit);
	m_lastSetPowerLimit[PowerControlType::PL1] = powerLimit;
}

void PowerControlFacade::setSocPowerFloorState(Bool socPowerFloorState)
{
	throwIfControlNotSupported();
	m_policyServices.domainPowerControl->setSocPowerFloorState(m_participantIndex, m_domainIndex, socPowerFloorState);
}

void PowerControlFacade::setPowerLimitPL2(const Power& powerLimit)
{
	throwIfControlNotSupported();
	m_policyServices.domainPowerControl->setPowerLimit(
		m_participantIndex, m_domainIndex, PowerControlType::PL2, powerLimit);
	m_lastSetPowerLimit[PowerControlType::PL2] = powerLimit;
}

void PowerControlFacade::setPowerLimitPL3(const Power& powerLimit)
{
	throwIfControlNotSupported();
	m_policyServices.domainPowerControl->setPowerLimit(
		m_participantIndex, m_domainIndex, PowerControlType::PL3, powerLimit);
	m_lastSetPowerLimit[PowerControlType::PL3] = powerLimit;
}

void PowerControlFacade::setPowerLimitPL4(const Power& powerLimit)
{
	throwIfControlNotSupported();
	m_policyServices.domainPowerControl->setPowerLimit(
		m_participantIndex, m_domainIndex, PowerControlType::PL4, powerLimit);
	m_lastSetPowerLimit[PowerControlType::PL4] = powerLimit;
}

void PowerControlFacade::setPowerLimitTimeWindowPL1(const TimeSpan& timeWindow)
{
	throwIfControlNotSupported();

	Int64 pl1 = 0;
	try
	{
		pl1 = getPowerLimitPL1().toInt32();
	}
	catch (dptf_exception& ex)
	{
		POLICY_LOG_MESSAGE_WARNING_EX({
			return "Failed to get PL1 for participant " + StatusFormat::friendlyValue(m_participantIndex) + ": "
				   + string(ex.what());
		});
	}

	try
	{
		// pl1 is 0 when PL1 control is disabled or PL1 value is 0
		if (pl1 == 0)
		{
			const auto& capsSet = getCapabilities();
			if (capsSet.hasCapability(PowerControlType::PL1))
			{
				const auto& caps = capsSet.getCapability(PowerControlType::PL1);
				m_policyServices.domainPowerControl->setPowerLimitWithoutUpdatingEnabled(
					m_participantIndex, m_domainIndex, PowerControlType::PL1, caps.getMaxPowerLimit());
				m_lastSetPowerLimit[PowerControlType::PL1] = caps.getMaxPowerLimit();
			}
		}
	}
	catch (dptf_exception& ex)
	{
		POLICY_LOG_MESSAGE_WARNING_EX({
			return "Failed to set PL1 without updating enabled for participant "
				   + StatusFormat::friendlyValue(m_participantIndex) + ": " + string(ex.what());
		});
	}

	m_policyServices.domainPowerControl->setPowerLimitTimeWindow(
		m_participantIndex, m_domainIndex, PowerControlType::PL1, timeWindow);
	m_lastSetTimeWindow[PowerControlType::PL1] = timeWindow;
}

void PowerControlFacade::setPowerLimitTimeWindowPL3(const TimeSpan& timeWindow)
{
	throwIfControlNotSupported();
	m_policyServices.domainPowerControl->setPowerLimitTimeWindow(
		m_participantIndex, m_domainIndex, PowerControlType::PL3, timeWindow);
	m_lastSetTimeWindow[PowerControlType::PL3] = timeWindow;
}

void PowerControlFacade::setPowerLimitDutyCyclePL3(const Percentage& dutyCycle)
{
	throwIfControlNotSupported();
	m_policyServices.domainPowerControl->setPowerLimitDutyCycle(
		m_participantIndex, m_domainIndex, PowerControlType::PL3, dutyCycle);
	m_lastSetDutyCycle[PowerControlType::PL3] = dutyCycle;
}

void PowerControlFacade::setValuesWithinCapabilities()
{
	setPowerLimitsWithinCapabilities();
	setTimeWindowsWithinCapabilities();
}

void PowerControlFacade::lockCapabilities()
{
	throwIfControlNotSupported();
	m_policyServices.domainPowerControl->setPowerCapsLock(m_participantIndex, m_domainIndex, true);
}

void PowerControlFacade::unlockCapabilities()
{
	throwIfControlNotSupported();
	m_policyServices.domainPowerControl->setPowerCapsLock(m_participantIndex, m_domainIndex, false);
}

void PowerControlFacade::removePowerLimitPolicyRequest(PowerControlType::Type controlType)
{
	m_policyServices.domainPowerControl->removePowerLimitPolicyRequest(m_participantIndex, m_domainIndex, controlType);
}

void PowerControlFacade::setPowerSharePolicyPower(const Power& powerSharePolicyPower)
{
	m_policyServices.domainPowerControl->setPowerSharePolicyPower(
		m_participantIndex, m_domainIndex, powerSharePolicyPower);
}

void PowerControlFacade::clearPowerLimit()
{
	m_policyServices.domainPowerControl->clearPowerLimit(m_participantIndex, m_domainIndex);
}

void PowerControlFacade::setPowerLimitsWithinCapabilities()
{
	for (auto powerLimit = m_lastSetPowerLimit.begin(); powerLimit != m_lastSetPowerLimit.end(); powerLimit++)
	{
		auto capsSet = m_powerControlCapabilitiesProperty.getDynamicCapsSet();
		auto limit = capsSet.snapToCapability(powerLimit->first, powerLimit->second);
		if (limit != powerLimit->second)
		{
			switch (powerLimit->first)
			{
			case PowerControlType::PL1:
				setPowerLimitPL1(limit);
				break;
			case PowerControlType::PL2:
				setPowerLimitPL2(limit);
				break;
			case PowerControlType::PL3:
				setPowerLimitPL3(limit);
				break;
			case PowerControlType::PL4:
				setPowerLimitPL4(limit);
				break;
			default:
				break;
			}
		}
	}
}

void PowerControlFacade::setTimeWindowsWithinCapabilities()
{
	for (auto timeWindow = m_lastSetTimeWindow.begin(); timeWindow != m_lastSetTimeWindow.end(); timeWindow++)
	{
		auto capsSet = m_powerControlCapabilitiesProperty.getDynamicCapsSet();
		auto limit = capsSet.snapToCapability(timeWindow->first, timeWindow->second);
		if (limit != timeWindow->second)
		{
			switch (timeWindow->first)
			{
			case PowerControlType::PL1:
				setPowerLimitTimeWindowPL1(limit);
				break;
			case PowerControlType::PL3:
				setPowerLimitTimeWindowPL3(limit);
				break;
			default:
				break;
			}
		}
	}
}

Bool PowerControlFacade::isPl1PowerLimitEnabled(void)
{
	throwIfControlNotSupported();
	return m_policyServices.domainPowerControl->isPowerLimitEnabled(
		m_participantIndex, m_domainIndex, PowerControlType::PL1);
}

Bool PowerControlFacade::isPl2PowerLimitEnabled(void)
{
	throwIfControlNotSupported();
	return m_policyServices.domainPowerControl->isPowerLimitEnabled(
		m_participantIndex, m_domainIndex, PowerControlType::PL2);
}

Bool PowerControlFacade::isPl3PowerLimitEnabled(void)
{
	throwIfControlNotSupported();
	return m_policyServices.domainPowerControl->isPowerLimitEnabled(
		m_participantIndex, m_domainIndex, PowerControlType::PL3);
}

Bool PowerControlFacade::isPl4PowerLimitEnabled(void)
{
	throwIfControlNotSupported();
	return m_policyServices.domainPowerControl->isPowerLimitEnabled(
		m_participantIndex, m_domainIndex, PowerControlType::PL1);
}

Power PowerControlFacade::getPowerLimitPL1()
{
	throwIfControlNotSupported();
	auto powerLimit = m_lastSetPowerLimit.find(PowerControlType::PL1);
	if (powerLimit != m_lastSetPowerLimit.end())
	{
		return powerLimit->second;
	}
	else
	{
		return m_policyServices.domainPowerControl->getPowerLimit(
			m_participantIndex, m_domainIndex, PowerControlType::PL1);
	}
}

Power PowerControlFacade::getPowerLimitPL2()
{
	throwIfControlNotSupported();
	auto powerLimit = m_lastSetPowerLimit.find(PowerControlType::PL2);
	if (powerLimit != m_lastSetPowerLimit.end())
	{
		return powerLimit->second;
	}
	else
	{
		return m_policyServices.domainPowerControl->getPowerLimit(
			m_participantIndex, m_domainIndex, PowerControlType::PL2);
	}
}

Power PowerControlFacade::getPowerLimitPL3()
{
	throwIfControlNotSupported();
	auto powerLimit = m_lastSetPowerLimit.find(PowerControlType::PL3);
	if (powerLimit != m_lastSetPowerLimit.end())
	{
		return powerLimit->second;
	}
	else
	{
		return m_policyServices.domainPowerControl->getPowerLimit(
			m_participantIndex, m_domainIndex, PowerControlType::PL3);
	}
}

Power PowerControlFacade::getPowerLimitPL4()
{
	throwIfControlNotSupported();
	auto powerLimit = m_lastSetPowerLimit.find(PowerControlType::PL4);
	if (powerLimit != m_lastSetPowerLimit.end())
	{
		return powerLimit->second;
	}
	else
	{
		return m_policyServices.domainPowerControl->getPowerLimit(
			m_participantIndex, m_domainIndex, PowerControlType::PL4);
	}
}

TimeSpan PowerControlFacade::getPowerLimitTimeWindowPL1()
{
	throwIfControlNotSupported();
	auto timeWindow = m_lastSetTimeWindow.find(PowerControlType::PL1);
	if (timeWindow != m_lastSetTimeWindow.end())
	{
		return timeWindow->second;
	}
	else
	{
		return m_policyServices.domainPowerControl->getPowerLimitTimeWindow(
			m_participantIndex, m_domainIndex, PowerControlType::PL1);
	}
}

TimeSpan PowerControlFacade::getPowerLimitTimeWindowPL3()
{
	throwIfControlNotSupported();
	auto timeWindow = m_lastSetTimeWindow.find(PowerControlType::PL3);
	if (timeWindow != m_lastSetTimeWindow.end())
	{
		return timeWindow->second;
	}
	else
	{
		return m_policyServices.domainPowerControl->getPowerLimitTimeWindow(
			m_participantIndex, m_domainIndex, PowerControlType::PL3);
	}
}

Percentage PowerControlFacade::getPowerLimitDutyCyclePL3()
{
	throwIfControlNotSupported();
	auto dutyCycle = m_lastSetDutyCycle.find(PowerControlType::PL3);
	if (dutyCycle != m_lastSetDutyCycle.end())
	{
		return dutyCycle->second;
	}
	else
	{
		return m_policyServices.domainPowerControl->getPowerLimitDutyCycle(
			m_participantIndex, m_domainIndex, PowerControlType::PL3);
	}
}

void PowerControlFacade::throwIfControlNotSupported() const
{
	if (supportsPowerControls() == false)
	{
		throw dptf_exception(
			"Cannot perform power control action because power controls \
							 are not supported on the domain.");
	}
}

Power PowerControlFacade::getLivePowerLimitPL1()
{
	throwIfControlNotSupported();
	return m_policyServices.domainPowerControl->getPowerLimit(m_participantIndex, m_domainIndex, PowerControlType::PL1);
}

Power PowerControlFacade::getLivePowerLimitPL2()
{
	throwIfControlNotSupported();
	return m_policyServices.domainPowerControl->getPowerLimit(m_participantIndex, m_domainIndex, PowerControlType::PL2);
}

Power PowerControlFacade::getLivePowerLimitPL3()
{
	throwIfControlNotSupported();
	return m_policyServices.domainPowerControl->getPowerLimit(m_participantIndex, m_domainIndex, PowerControlType::PL3);
}

Power PowerControlFacade::getLivePowerLimitPL4()
{
	throwIfControlNotSupported();
	return m_policyServices.domainPowerControl->getPowerLimit(m_participantIndex, m_domainIndex, PowerControlType::PL4);
}

TimeSpan PowerControlFacade::getLivePowerLimitTimeWindowPL1()
{
	throwIfControlNotSupported();
	return m_policyServices.domainPowerControl->getPowerLimitTimeWindow(
		m_participantIndex, m_domainIndex, PowerControlType::PL1);
}

TimeSpan PowerControlFacade::getLivePowerLimitTimeWindowPL3()
{
	throwIfControlNotSupported();
	return m_policyServices.domainPowerControl->getPowerLimitTimeWindow(
		m_participantIndex, m_domainIndex, PowerControlType::PL3);
}

Percentage PowerControlFacade::getLivePowerLimitDutyCyclePL3()
{
	throwIfControlNotSupported();
	return m_policyServices.domainPowerControl->getPowerLimitDutyCycle(
		m_participantIndex, m_domainIndex, PowerControlType::PL3);
}

void PowerControlFacade::updatePowerLimitTimeWindowPL1(const TimeSpan& timeWindow)
{
	throwIfControlNotSupported();
	m_lastSetTimeWindow[PowerControlType::PL1] = timeWindow;
}

const PolicyServicesInterfaceContainer& PowerControlFacade::getPolicyServices() const
{
	return m_policyServices;
}
