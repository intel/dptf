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

#include "Arbitrator.h"

Arbitrator::Arbitrator()
{
	m_coreControlArbitrator = new CoreControlArbitrator();
	m_displayControlArbitrator = new DisplayControlArbitrator();
	m_performanceControlArbitrator = new PerformanceControlArbitrator();
	m_powerControlArbitrator = new PowerControlArbitrator();
	m_powerControlCapabilitiesArbitrator = new PowerControlCapabilitiesArbitrator();
	m_displayControlCapabilitiesArbitrator = new DisplayControlCapabilitiesArbitrator();
	m_performanceControlCapabilitiesArbitrator = new PerformanceControlCapabilitiesArbitrator();
	m_systemPowerControlArbitrator = new SystemPowerControlArbitrator();
	m_peakPowerControlArbitrator = new PeakPowerControlArbitrator();
}

Arbitrator::~Arbitrator(void)
{
	DELETE_MEMORY_TC(m_coreControlArbitrator);
	DELETE_MEMORY_TC(m_displayControlArbitrator);
	DELETE_MEMORY_TC(m_performanceControlArbitrator);
	DELETE_MEMORY_TC(m_powerControlArbitrator);
	DELETE_MEMORY_TC(m_powerControlCapabilitiesArbitrator);
	DELETE_MEMORY_TC(m_displayControlCapabilitiesArbitrator);
	DELETE_MEMORY_TC(m_performanceControlCapabilitiesArbitrator);
	DELETE_MEMORY_TC(m_systemPowerControlArbitrator);
	DELETE_MEMORY_TC(m_peakPowerControlArbitrator);
}

void Arbitrator::clearPolicyCachedData(UIntN policyIndex)
{
	// call each arbitrator class to remove the specified policy
	m_coreControlArbitrator->clearPolicyCachedData(policyIndex);
	m_displayControlArbitrator->clearPolicyCachedData(policyIndex);
	m_performanceControlArbitrator->clearPolicyCachedData(policyIndex);
	m_powerControlArbitrator->removeRequestsForPolicy(policyIndex);
	m_powerControlCapabilitiesArbitrator->removeRequestsForPolicy(policyIndex);
	m_displayControlCapabilitiesArbitrator->removeRequestsForPolicy(policyIndex);
	m_performanceControlCapabilitiesArbitrator->removeRequestsForPolicy(policyIndex);
	m_systemPowerControlArbitrator->removeRequestsForPolicy(policyIndex);
	m_peakPowerControlArbitrator->clearPolicyCachedData(policyIndex);
}

std::shared_ptr<XmlNode> Arbitrator::getArbitrationXmlForPolicy(UIntN policyIndex, ControlFactoryType::Type type) const
{
	// call each arbitrator class to get the xml for the specified policy
	auto domainRoot = XmlNode::createWrapperElement("arbitrators_status");
	switch (type)
	{
	case ControlFactoryType::Active:
		// using the new interface now
		break;
	case ControlFactoryType::Core:
		domainRoot->addChild(m_coreControlArbitrator->getArbitrationXmlForPolicy(policyIndex));
		break;
	case ControlFactoryType::Display:
		domainRoot->addChild(m_displayControlArbitrator->getArbitrationXmlForPolicy(policyIndex));
		domainRoot->addChild(m_displayControlCapabilitiesArbitrator->getArbitrationXmlForPolicy(policyIndex));
		break;
	case ControlFactoryType::PeakPowerControl:
		domainRoot->addChild(m_peakPowerControlArbitrator->getArbitrationXmlForPolicy(policyIndex));
		break;
	case ControlFactoryType::Performance:
		domainRoot->addChild(m_performanceControlArbitrator->getArbitrationXmlForPolicy(policyIndex));
		domainRoot->addChild(m_performanceControlCapabilitiesArbitrator->getArbitrationXmlForPolicy(policyIndex));
		break;
	case ControlFactoryType::PowerControl:
		domainRoot->addChild(m_powerControlArbitrator->getArbitrationXmlForPolicy(policyIndex));
		domainRoot->addChild(m_powerControlCapabilitiesArbitrator->getArbitrationXmlForPolicy(policyIndex));
		break;
	case ControlFactoryType::SystemPower:
		domainRoot->addChild(m_systemPowerControlArbitrator->getArbitrationXmlForPolicy(policyIndex));
		break;
	case ControlFactoryType::ProcessorControl:
	case ControlFactoryType::Temperature:
		// using the new interface
		break;
	default:
		// does not have an arbitrator
		break;
	}

	return domainRoot;
}

CoreControlArbitrator* Arbitrator::getCoreControlArbitrator(void) const
{
	return m_coreControlArbitrator;
}

DisplayControlArbitrator* Arbitrator::getDisplayControlArbitrator(void) const
{
	return m_displayControlArbitrator;
}

PerformanceControlArbitrator* Arbitrator::getPerformanceControlArbitrator(void) const
{
	return m_performanceControlArbitrator;
}

PowerControlArbitrator* Arbitrator::getPowerControlArbitrator(void) const
{
	return m_powerControlArbitrator;
}

PowerControlCapabilitiesArbitrator* Arbitrator::getPowerControlCapabilitiesArbitrator(void) const
{
	return m_powerControlCapabilitiesArbitrator;
}

DisplayControlCapabilitiesArbitrator* Arbitrator::getDisplayControlCapabilitiesArbitrator(void) const
{
	return m_displayControlCapabilitiesArbitrator;
}

PerformanceControlCapabilitiesArbitrator* Arbitrator::getPerformanceControlCapabilitiesArbitrator(void) const
{
	return m_performanceControlCapabilitiesArbitrator;
}

SystemPowerControlArbitrator* Arbitrator::getSystemPowerControlArbitrator(void) const
{
	return m_systemPowerControlArbitrator;
}

PeakPowerControlArbitrator* Arbitrator::getPeakPowerControlArbitrator(void) const
{
	return m_peakPowerControlArbitrator;
}
