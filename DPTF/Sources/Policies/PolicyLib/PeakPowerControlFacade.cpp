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

#include "PeakPowerControlFacade.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

PeakPowerControlFacade::PeakPowerControlFacade(
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

PeakPowerControlFacade::~PeakPowerControlFacade(void)
{
}

Bool PeakPowerControlFacade::supportsPeakPowerControl(void)
{
	return m_domainProperties.implementsPeakPowerControlInterface();
}

Power PeakPowerControlFacade::getACPeakPower(void)
{
	if (m_acPeakPower.isInvalid())
	{
		m_acPeakPower.set(m_policyServices.domainPeakPowerControl->getACPeakPower(m_participantIndex, m_domainIndex));
	}
	return m_acPeakPower.get();
}

Power PeakPowerControlFacade::getDCPeakPower(void)
{
	if (m_dcPeakPower.isInvalid())
	{
		m_dcPeakPower.set(m_policyServices.domainPeakPowerControl->getDCPeakPower(m_participantIndex, m_domainIndex));
	}
	return m_dcPeakPower.get();
}

void PeakPowerControlFacade::setACPeakPower(const Power& acPeakPower)
{
	m_policyServices.domainPeakPowerControl->setACPeakPower(m_participantIndex, m_domainIndex, acPeakPower);
	m_acPeakPower.set(acPeakPower);
}

void PeakPowerControlFacade::setDCPeakPower(const Power& dcPeakPower)
{
	m_policyServices.domainPeakPowerControl->setDCPeakPower(m_participantIndex, m_domainIndex, dcPeakPower);
	m_dcPeakPower.set(dcPeakPower);
}
