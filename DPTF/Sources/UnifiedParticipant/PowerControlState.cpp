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

#include "PowerControlState.h"
#include "StatusFormat.h"

PowerControlState::PowerControlState(DomainPowerControlBase* control)
	: m_control(control)
{
}

PowerControlState::~PowerControlState()
{
}

void PowerControlState::capture()
{
	captureEnables();
	captureLimit(PowerControlType::PL1, m_pl1Limit);
	captureLimit(PowerControlType::PL2, m_pl2Limit);
	captureLimit(PowerControlType::PL3, m_pl3Limit);
	captureLimit(PowerControlType::PL4, m_pl4Limit);
	captureTimeWindow(PowerControlType::PL1, m_pl1TimeWindow);
	captureTimeWindow(PowerControlType::PL3, m_pl3TimeWindow);
	captureDutyCycle(PowerControlType::PL3, m_pl3DutyCycle);
}

void PowerControlState::restore()
{
	restoreLimit(PowerControlType::PL1, m_pl1Limit);
	restoreLimit(PowerControlType::PL2, m_pl2Limit);
	restoreLimit(PowerControlType::PL3, m_pl3Limit);
	restoreLimit(PowerControlType::PL4, m_pl4Limit);
	restoreTimeWindow(PowerControlType::PL1, m_pl1TimeWindow);
	restoreTimeWindow(PowerControlType::PL3, m_pl3TimeWindow);
	restoreDutyCycle(PowerControlType::PL3, m_pl3DutyCycle);
	restoreEnables();
}

std::shared_ptr<XmlNode> PowerControlState::toXml() const
{
	auto set = XmlNode::createWrapperElement("power_control_state_set");

	auto entryPl1 = XmlNode::createWrapperElement("power_control_state_entry");
	entryPl1->addChild(XmlNode::createDataElement("control", PowerControlType::ToString(PowerControlType::PL1)));
	entryPl1->addChild(XmlNode::createDataElement("enabled", StatusFormat::friendlyValue(m_pl1Enabled.get())));
	entryPl1->addChild(XmlNode::createDataElement("limit", m_pl1Limit.get().toString()));
	entryPl1->addChild(XmlNode::createDataElement("time_window", m_pl1TimeWindow.get().toStringMilliseconds()));
	entryPl1->addChild(XmlNode::createDataElement("duty_cycle", "DISABLED"));
	set->addChild(entryPl1);

	auto entryPl2 = XmlNode::createWrapperElement("power_control_state_entry");
	entryPl1->addChild(XmlNode::createDataElement("control", PowerControlType::ToString(PowerControlType::PL2)));
	entryPl1->addChild(XmlNode::createDataElement("enabled", StatusFormat::friendlyValue(m_pl2Enabled.get())));
	entryPl1->addChild(XmlNode::createDataElement("limit", m_pl2Limit.get().toString()));
	entryPl1->addChild(XmlNode::createDataElement("time_window", "DISABLED"));
	entryPl1->addChild(XmlNode::createDataElement("duty_cycle", "DISABLED"));
	set->addChild(entryPl2);

	auto entryPl3 = XmlNode::createWrapperElement("power_control_state_entry");
	entryPl1->addChild(XmlNode::createDataElement("control", PowerControlType::ToString(PowerControlType::PL3)));
	entryPl1->addChild(XmlNode::createDataElement("enabled", StatusFormat::friendlyValue(m_pl3Enabled.get())));
	entryPl1->addChild(XmlNode::createDataElement("limit", m_pl3Limit.get().toString()));
	entryPl1->addChild(XmlNode::createDataElement("time_window", m_pl3TimeWindow.get().toStringMilliseconds()));
	entryPl1->addChild(XmlNode::createDataElement("duty_cycle", m_pl3DutyCycle.get().toString()));
	set->addChild(entryPl3);

	auto entryPl4 = XmlNode::createWrapperElement("power_control_state_entry");
	entryPl1->addChild(XmlNode::createDataElement("control", PowerControlType::ToString(PowerControlType::PL4)));
	entryPl1->addChild(XmlNode::createDataElement("enabled", StatusFormat::friendlyValue(m_pl4Enabled.get())));
	entryPl1->addChild(XmlNode::createDataElement("limit", m_pl4Limit.get().toString()));
	entryPl1->addChild(XmlNode::createDataElement("time_window", "DISABLED"));
	entryPl1->addChild(XmlNode::createDataElement("duty_cycle", "DISABLED"));
	set->addChild(entryPl4);

	auto root = XmlNode::createWrapperElement("power_control_state");
	root->addChild(set);

	return root;
}

void PowerControlState::restoreEnables()
{
	if (m_pl1Enabled.isValid())
	{
		m_control->setEnabled(PowerControlType::PL1, m_pl1Enabled.get());
	}

	if (m_pl2Enabled.isValid())
	{
		m_control->setEnabled(PowerControlType::PL2, m_pl2Enabled.get());
	}

	if (m_pl3Enabled.isValid())
	{
		m_control->setEnabled(PowerControlType::PL3, m_pl3Enabled.get());
	}

	// there is no PL4 enable functionality
}

void PowerControlState::captureEnables()
{
	m_control->updateEnabled(PowerControlType::PL1);
	m_pl1Enabled.set(m_control->isEnabled(PowerControlType::PL1));
	m_control->updateEnabled(PowerControlType::PL2);
	m_pl2Enabled.set(m_control->isEnabled(PowerControlType::PL2));
	m_control->updateEnabled(PowerControlType::PL3);
	m_pl3Enabled.set(m_control->isEnabled(PowerControlType::PL3));
	m_control->updateEnabled(PowerControlType::PL4);
	m_pl4Enabled.set(m_control->isEnabled(PowerControlType::PL4));
}

void PowerControlState::captureLimit(PowerControlType::Type controlType, CachedValue<Power>& limit)
{
	try
	{
		limit.set(m_control->getPowerLimit(m_control->getParticipantIndex(), m_control->getDomainIndex(), controlType));
	}
	catch (...)
	{
		limit.invalidate();
	}
}

void PowerControlState::captureTimeWindow(PowerControlType::Type controlType, CachedValue<TimeSpan>& timeWindow)
{
	try
	{
		timeWindow.set(m_control->getPowerLimitTimeWindow(
			m_control->getParticipantIndex(), m_control->getDomainIndex(), controlType));
	}
	catch (...)
	{
		timeWindow.invalidate();
	}
}

void PowerControlState::captureDutyCycle(PowerControlType::Type controlType, CachedValue<Percentage>& dutyCycle)
{
	try
	{
		dutyCycle.set(m_control->getPowerLimitDutyCycle(
			m_control->getParticipantIndex(), m_control->getDomainIndex(), controlType));
	}
	catch (...)
	{
		dutyCycle.invalidate();
	}
}

void PowerControlState::restoreLimit(PowerControlType::Type controlType, const CachedValue<Power>& limit)
{
	if (limit.isValid())
	{
		try
		{
			m_control->setPowerLimitIgnoringCaps(
				m_control->getParticipantIndex(), m_control->getDomainIndex(), controlType, limit.get());
		}
		catch (...)
		{
		}
	}
}

void PowerControlState::restoreTimeWindow(PowerControlType::Type controlType, const CachedValue<TimeSpan>& timeWindow)
{
	if (timeWindow.isValid())
	{
		try
		{
			m_control->setPowerLimitTimeWindowIgnoringCaps(
				m_control->getParticipantIndex(), m_control->getDomainIndex(), controlType, timeWindow.get());
		}
		catch (...)
		{
		}
	}
}

void PowerControlState::restoreDutyCycle(PowerControlType::Type controlType, const CachedValue<Percentage>& dutyCycle)
{
	if (dutyCycle.isValid())
	{
		try
		{
			m_control->setPowerLimitDutyCycle(
				m_control->getParticipantIndex(), m_control->getDomainIndex(), controlType, dutyCycle.get());
		}
		catch (...)
		{
		}
	}
}
