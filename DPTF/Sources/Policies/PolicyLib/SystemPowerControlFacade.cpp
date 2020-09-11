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

#include "SystemPowerControlFacade.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

SystemPowerControlFacade::SystemPowerControlFacade(
	UIntN participantIndex,
	UIntN domainIndex,
	const DomainProperties& domainProperties,
	const PolicyServicesInterfaceContainer& policyServices)
	: m_policyServices(policyServices)
	, m_domainProperties(domainProperties)
	, m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
{
}

SystemPowerControlFacade::~SystemPowerControlFacade(void)
{
}

Bool SystemPowerControlFacade::isPl1PowerLimitEnabled(void)
{
	if (m_pl1PowerLimitEnabled.isInvalid())
	{
		m_pl1PowerLimitEnabled.set(m_policyServices.domainSystemPowerControl->isSystemPowerLimitEnabled(
			m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL1));
	}
	return m_pl1PowerLimitEnabled.get();
}

Bool SystemPowerControlFacade::isPl2PowerLimitEnabled(void)
{
	if (m_pl2PowerLimitEnabled.isInvalid())
	{
		m_pl2PowerLimitEnabled.set(m_policyServices.domainSystemPowerControl->isSystemPowerLimitEnabled(
			m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL2));
	}
	return m_pl2PowerLimitEnabled.get();
}

Bool SystemPowerControlFacade::isPl3PowerLimitEnabled(void)
{
	if (m_pl3PowerLimitEnabled.isInvalid())
	{
		m_pl3PowerLimitEnabled.set(m_policyServices.domainSystemPowerControl->isSystemPowerLimitEnabled(
			m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL3));
	}
	return m_pl3PowerLimitEnabled.get();
}

Power SystemPowerControlFacade::getPl1PowerLimit(void)
{
	if (m_pl1PowerLimit.isInvalid())
	{
		m_pl1PowerLimit.set(m_policyServices.domainSystemPowerControl->getSystemPowerLimit(
			m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL1));
	}
	return m_pl1PowerLimit.get();
}

Power SystemPowerControlFacade::getPl2PowerLimit(void)
{
	if (m_pl2PowerLimit.isInvalid())
	{
		m_pl2PowerLimit.set(m_policyServices.domainSystemPowerControl->getSystemPowerLimit(
			m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL2));
	}
	return m_pl2PowerLimit.get();
}

Power SystemPowerControlFacade::getPl3PowerLimit(void)
{
	if (m_pl3PowerLimit.isInvalid())
	{
		m_pl3PowerLimit.set(m_policyServices.domainSystemPowerControl->getSystemPowerLimit(
			m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL3));
	}
	return m_pl3PowerLimit.get();
}

TimeSpan SystemPowerControlFacade::getPl1TimeWindow(void)
{
	if (m_pl1TimeWindow.isInvalid())
	{
		m_pl1TimeWindow.set(m_policyServices.domainSystemPowerControl->getSystemPowerLimitTimeWindow(
			m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL1));
	}
	return m_pl1TimeWindow.get();
}

TimeSpan SystemPowerControlFacade::getPl3TimeWindow(void)
{
	if (m_pl3TimeWindow.isInvalid())
	{
		m_pl3TimeWindow.set(m_policyServices.domainSystemPowerControl->getSystemPowerLimitTimeWindow(
			m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL3));
	}
	return m_pl3TimeWindow.get();
}

Percentage SystemPowerControlFacade::getPl3DutyCycle(void)
{
	if (m_pl3DutyCycle.isInvalid())
	{
		m_pl3DutyCycle.set(m_policyServices.domainSystemPowerControl->getSystemPowerLimitDutyCycle(
			m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL3));
	}
	return m_pl3DutyCycle.get();
}

void SystemPowerControlFacade::setPl1PowerLimit(const Power& powerLimit)
{
	m_policyServices.domainSystemPowerControl->setSystemPowerLimit(
		m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL1, powerLimit);
	m_pl1PowerLimit.set(powerLimit);
}

void SystemPowerControlFacade::setPl2PowerLimit(const Power& powerLimit)
{
	m_policyServices.domainSystemPowerControl->setSystemPowerLimit(
		m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL2, powerLimit);
	m_pl2PowerLimit.set(powerLimit);
}

void SystemPowerControlFacade::setPl3PowerLimit(const Power& powerLimit)
{
	m_policyServices.domainSystemPowerControl->setSystemPowerLimit(
		m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL3, powerLimit);
	m_pl3PowerLimit.set(powerLimit);
}

void SystemPowerControlFacade::setPl1TimeWindow(const TimeSpan& timeWindow)
{
	m_policyServices.domainSystemPowerControl->setSystemPowerLimitTimeWindow(
		m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL1, timeWindow);
	m_pl1TimeWindow.set(timeWindow);
}

void SystemPowerControlFacade::setPl3TimeWindow(const TimeSpan& timeWindow)
{
	m_policyServices.domainSystemPowerControl->setSystemPowerLimitTimeWindow(
		m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL3, timeWindow);
	m_pl1TimeWindow.set(timeWindow);
}

void SystemPowerControlFacade::setPl3DutyCycle(const Percentage& dutyCycle)
{
	m_policyServices.domainSystemPowerControl->setSystemPowerLimitDutyCycle(
		m_participantIndex, m_domainIndex, PsysPowerLimitType::PSysPL3, dutyCycle);
	m_pl3DutyCycle.set(dutyCycle);
}

std::shared_ptr<XmlNode> SystemPowerControlFacade::getXml() const
{
	auto control = XmlNode::createWrapperElement("system_power_control");
	control->addChild(createPl1XmlData());
	control->addChild(createPl2XmlData());
	control->addChild(createPl3XmlData());
	return control;
}

std::shared_ptr<XmlNode> SystemPowerControlFacade::createPl1XmlData() const
{
	auto pl1 = XmlNode::createWrapperElement("power_limit_1");
	if (m_pl1PowerLimitEnabled.isValid())
	{
		pl1->addChild(XmlNode::createDataElement("enabled", friendlyValue(m_pl1PowerLimitEnabled.get())));
	}
	else
	{
		pl1->addChild(XmlNode::createDataElement("enabled", "Invalid"));
	}

	if (m_pl1PowerLimit.isValid())
	{
		pl1->addChild(XmlNode::createDataElement("power_limit", m_pl1PowerLimit.get().toString()));
	}
	else
	{
		pl1->addChild(XmlNode::createDataElement("power_limit", "Invalid"));
	}

	if (m_pl1TimeWindow.isValid())
	{
		pl1->addChild(XmlNode::createDataElement("time_window", m_pl1TimeWindow.get().toStringMicroseconds()));
	}
	else
	{
		pl1->addChild(XmlNode::createDataElement("time_window", "Invalid"));
	}
	pl1->addChild(XmlNode::createDataElement("duty_cycle", Constants::NotAvailableString));
	return pl1;
}

std::shared_ptr<XmlNode> SystemPowerControlFacade::createPl2XmlData() const
{
	auto pl2 = XmlNode::createWrapperElement("power_limit_2");
	if (m_pl2PowerLimitEnabled.isValid())
	{
		pl2->addChild(XmlNode::createDataElement("enabled", friendlyValue(m_pl2PowerLimitEnabled.get())));
	}
	else
	{
		pl2->addChild(XmlNode::createDataElement("enabled", "Invalid"));
	}

	if (m_pl2PowerLimit.isValid())
	{
		pl2->addChild(XmlNode::createDataElement("power_limit", m_pl2PowerLimit.get().toString()));
	}
	else
	{
		pl2->addChild(XmlNode::createDataElement("power_limit", "Invalid"));
	}

	pl2->addChild(XmlNode::createDataElement("time_window", Constants::NotAvailableString));
	pl2->addChild(XmlNode::createDataElement("duty_cycle", Constants::NotAvailableString));
	return pl2;
}

std::shared_ptr<XmlNode> SystemPowerControlFacade::createPl3XmlData() const
{
	auto pl3 = XmlNode::createWrapperElement("power_limit_3");
	if (m_pl3PowerLimitEnabled.isValid())
	{
		pl3->addChild(XmlNode::createDataElement("enabled", friendlyValue(m_pl3PowerLimitEnabled.get())));
	}
	else
	{
		pl3->addChild(XmlNode::createDataElement("enabled", "Invalid"));
	}

	if (m_pl3PowerLimit.isValid())
	{
		pl3->addChild(XmlNode::createDataElement("power_limit", m_pl3PowerLimit.get().toString()));
	}
	else
	{
		pl3->addChild(XmlNode::createDataElement("power_limit", "Invalid"));
	}

	if (m_pl3TimeWindow.isValid())
	{
		pl3->addChild(XmlNode::createDataElement("time_window", m_pl3TimeWindow.get().toStringMicroseconds()));
	}
	else
	{
		pl3->addChild(XmlNode::createDataElement("time_window", "Invalid"));
	}

	if (m_pl3DutyCycle.isValid())
	{
		pl3->addChild(XmlNode::createDataElement("duty_cycle", m_pl3DutyCycle.get().toString()));
	}
	else
	{
		pl3->addChild(XmlNode::createDataElement("duty_cycle", "Invalid"));
	}
	return pl3;
}
