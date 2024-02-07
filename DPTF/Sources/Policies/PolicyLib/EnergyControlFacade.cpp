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

#include "EnergyControlFacade.h"
using namespace std;

EnergyControlFacade::EnergyControlFacade(
	UIntN participantIndex,
	UIntN domainIndex,
	const DomainProperties& domainProperties,
	const PolicyServicesInterfaceContainer& policyServices)
	: m_policyServices(policyServices)
	, m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
	, m_domainProperties(domainProperties)
{
}

Bool EnergyControlFacade::supportsEnergyControl() const
{
	return m_domainProperties.implementsEnergyControlInterface();
}

UInt32 EnergyControlFacade::getRaplEnergyCounter() const
{
	throwIfControlUnsupported();
	return m_policyServices.domainEnergyControl->getRaplEnergyCounter(m_participantIndex, m_domainIndex);
}

EnergyCounterInfo EnergyControlFacade::getRaplEnergyCounterInfo()
{
	throwIfControlUnsupported();
	return m_policyServices.domainEnergyControl->getRaplEnergyCounterInfo(m_participantIndex, m_domainIndex);
}

double EnergyControlFacade::getRaplEnergyUnit()
{
	throwIfControlUnsupported();
	return m_policyServices.domainEnergyControl->getRaplEnergyUnit(m_participantIndex, m_domainIndex);
}

UInt32 EnergyControlFacade::getRaplEnergyCounterWidth()
{
	throwIfControlUnsupported();
	return m_policyServices.domainEnergyControl->getRaplEnergyCounterWidth(m_participantIndex, m_domainIndex);
}

Power EnergyControlFacade::getInstantaneousPower()
{
	throwIfControlUnsupported();
	return m_policyServices.domainEnergyControl->getInstantaneousPower(m_participantIndex, m_domainIndex);
}

UInt32 EnergyControlFacade::getEnergyThreshold()
{
	throwIfControlUnsupported();
	return m_policyServices.domainEnergyControl->getEnergyThreshold(m_participantIndex, m_domainIndex);
}

void EnergyControlFacade::setEnergyThreshold(UInt32 energyThreshold)
{
	throwIfControlUnsupported();
	m_policyServices.domainEnergyControl->setEnergyThreshold(m_participantIndex, m_domainIndex, energyThreshold);
}

void EnergyControlFacade::setEnergyThresholdInterruptDisable()
{
	throwIfControlUnsupported();
	m_policyServices.domainEnergyControl->setEnergyThresholdInterruptDisable(m_participantIndex, m_domainIndex);
}

void EnergyControlFacade::throwIfControlUnsupported() const
{
	if (!supportsEnergyControl())
	{
		throw dptf_exception(
			"Cannot perform EnergyControl action because the domain does not support the control.");
	}
}
