/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "PowerControlDynamicCaps.h"
#include "StatusFormat.h"
#include "XmlNode.h"
#include "EsifDataBinaryPpccPackage.h"

using namespace StatusFormat;

PowerControlDynamicCaps::PowerControlDynamicCaps(
	PowerControlType::Type powerControlType,
	const Power& minPowerLimit,
	const Power& maxPowerLimit,
	const Power& powerStepSize,
	const TimeSpan& minTimeWindow,
	const TimeSpan& maxTimeWindow,
	const Percentage& minDutyCycle,
	const Percentage& maxDutyCycle)
	: m_valid(true)
	, m_powerControlType(powerControlType)
	, m_minPowerLimit(minPowerLimit)
	, m_maxPowerLimit(maxPowerLimit)
	, m_powerStepSize(powerStepSize)
	, m_minTimeWindow(minTimeWindow)
	, m_maxTimeWindow(maxTimeWindow)
	, m_minDutyCycle(minDutyCycle)
	, m_maxDutyCycle(maxDutyCycle)
{
}

PowerControlDynamicCaps::PowerControlDynamicCaps()
	: m_valid(false)
	, m_powerControlType()
{
}

PowerControlType::Type PowerControlDynamicCaps::getPowerControlType() const
{
	throwIfNotValid();
	return m_powerControlType;
}

Bool PowerControlDynamicCaps::arePowerLimitCapsValid() const
{
	try
	{
		return m_minPowerLimit <= m_maxPowerLimit;
	}
	catch (...)
	{
		return false;
	}
}

Power PowerControlDynamicCaps::getMinPowerLimit() const
{
	return m_minPowerLimit;
}

Power PowerControlDynamicCaps::getMaxPowerLimit() const
{
	return m_maxPowerLimit;
}

Power PowerControlDynamicCaps::getPowerStepSize() const
{
	return m_powerStepSize;
}

void PowerControlDynamicCaps::setMinPowerLimit(const Power& minPower)
{
	m_minPowerLimit = minPower;
}

void PowerControlDynamicCaps::setMaxPowerLimit(const Power& maxPower)
{
	m_maxPowerLimit = maxPower;
}

void PowerControlDynamicCaps::setPowerStepSize(const Power& stepSize)
{
	m_powerStepSize = stepSize;
}

void PowerControlDynamicCaps::setMinTimeWindow(const TimeSpan& minTimeWindow)
{
	m_minTimeWindow = minTimeWindow;
}

void PowerControlDynamicCaps::setMaxTimeWindow(const TimeSpan& maxTimeWindow)
{
	m_maxTimeWindow = maxTimeWindow;
}

Bool PowerControlDynamicCaps::areTimeWindowCapsValid() const
{
	try
	{
		return m_minTimeWindow <= m_maxTimeWindow;
	}
	catch (...)
	{
		return false;
	}
}

TimeSpan PowerControlDynamicCaps::getMinTimeWindow() const
{
	return m_minTimeWindow;
}

TimeSpan PowerControlDynamicCaps::getMaxTimeWindow() const
{
	return m_maxTimeWindow;
}

Bool PowerControlDynamicCaps::areDutyCycleCapsValid() const
{
	try
	{
		return m_minDutyCycle <= m_maxDutyCycle;
	}
	catch (...)
	{
		return false;
	}
}

Percentage PowerControlDynamicCaps::getMinDutyCycle() const
{
	return m_minDutyCycle;
}

Percentage PowerControlDynamicCaps::getMaxDutyCycle() const
{
	return m_maxDutyCycle;
}

Bool PowerControlDynamicCaps::operator==(const PowerControlDynamicCaps& rhs) const
{
	return (
		m_valid == rhs.m_valid && m_powerControlType == rhs.m_powerControlType && m_minPowerLimit == rhs.m_minPowerLimit
		&& m_maxPowerLimit == rhs.m_maxPowerLimit && m_powerStepSize == rhs.m_powerStepSize
		&& m_minTimeWindow == rhs.m_minTimeWindow && m_maxTimeWindow == rhs.m_maxTimeWindow
		&& m_minDutyCycle == rhs.m_minDutyCycle && m_maxDutyCycle == rhs.m_maxDutyCycle);
}

Bool PowerControlDynamicCaps::operator!=(const PowerControlDynamicCaps& rhs) const
{
	return !(*this == rhs);
}

std::shared_ptr<XmlNode> PowerControlDynamicCaps::getXml() const
{
	throwIfNotValid();
	auto root = XmlNode::createWrapperElement("power_control_dynamic_caps");
	root->addChild(XmlNode::createDataElement("control_type", PowerControlType::ToString(m_powerControlType)));
	root->addChild(XmlNode::createDataElement("power_limit_caps_valid", friendlyValue(arePowerLimitCapsValid())));
	root->addChild(XmlNode::createDataElement("max_power_limit", m_maxPowerLimit.toString()));
	root->addChild(XmlNode::createDataElement("min_power_limit", m_minPowerLimit.toString()));
	root->addChild(XmlNode::createDataElement("power_step_size_valid", friendlyValue(m_powerStepSize.isValid())));
	root->addChild(XmlNode::createDataElement("power_step_size", m_powerStepSize.toString()));
	root->addChild(XmlNode::createDataElement("time_window_caps_valid", friendlyValue(areTimeWindowCapsValid())));
	root->addChild(XmlNode::createDataElement("max_time_window", m_maxTimeWindow.toStringMilliseconds()));
	root->addChild(XmlNode::createDataElement("min_time_window", m_minTimeWindow.toStringMilliseconds()));
	return root;
}

void PowerControlDynamicCaps::throwIfNotValid() const
{
	if (m_valid == false)
	{
		throw dptf_exception("Power control dynamic capabilities is not valid.");
	}
}

PowerControlDynamicCaps PowerControlDynamicCaps::getDefaultPpccRowValues(PowerControlType::Type PowerControlType) const
{
	PowerControlDynamicCaps temp(
		static_cast<PowerControlType::Type>(PowerControlType),
		Power::createInvalid(),
		Power::createInvalid(),
		Power::createInvalid(),
		TimeSpan::createInvalid(),
		TimeSpan::createInvalid(),
		Percentage(0.0),
		Percentage(0.0));
	return temp;
}

PowerControlDynamicCaps PowerControlDynamicCaps::getDefaultPpccPl4RowValues(const Power& pl4PowerLimit) const
{
	PowerControlDynamicCaps temp(
		static_cast<PowerControlType::Type>(PowerControlType::Type::PL4),
		static_cast<UIntN>(pl4PowerLimit),
		static_cast<UIntN>(pl4PowerLimit),
		static_cast<UIntN>(500),
		TimeSpan::createFromMilliseconds(static_cast<UIntN>(0)),
		TimeSpan::createFromMilliseconds(static_cast<UIntN>(0)),
		Percentage(0.0),
		Percentage(0.0));
	return temp;
}

PowerControlDynamicCaps PowerControlDynamicCaps::getPpccPlRowValues(const EsifDataBinaryPpccPackage* currentRow) const
{
	auto powerMin = Power::createInvalid();
	auto powerMax = Power::createInvalid();
	auto powerStep = Power::createInvalid();
	auto timeMin = TimeSpan::createInvalid();
	auto timeMax = TimeSpan::createInvalid();

	if (static_cast<UIntN>(currentRow->powerLimitMinimum.integer.value) != Constants::Invalid)
	{
		powerMin = static_cast<UIntN>(currentRow->powerLimitMinimum.integer.value);
	}

	if (static_cast<UIntN>(currentRow->powerLimitMaximum.integer.value) != Constants::Invalid)
	{
		powerMax = static_cast<UIntN>(currentRow->powerLimitMaximum.integer.value);
	}

	if (static_cast<UIntN>(currentRow->stepSize.integer.value) != Constants::Invalid)
	{
		powerStep = static_cast<UIntN>(currentRow->stepSize.integer.value);
	}

	if (static_cast<UIntN>(currentRow->timeWindowMinimum.integer.value) != Constants::Invalid)
	{
		timeMin = TimeSpan::createFromMilliseconds(static_cast<UIntN>(currentRow->timeWindowMinimum.integer.value));
	}

	if (static_cast<UIntN>(currentRow->timeWindowMaximum.integer.value) != Constants::Invalid)
	{
		timeMax = TimeSpan::createFromMilliseconds(static_cast<UIntN>(currentRow->timeWindowMaximum.integer.value));
	}

	PowerControlDynamicCaps temp(
		static_cast<PowerControlType::Type>(currentRow->powerLimitIndex.integer.value),
		powerMin,
		powerMax,
		powerStep,
		timeMin,
		timeMax,
		Percentage(0.0),
		Percentage(0.0));
	return temp;
}