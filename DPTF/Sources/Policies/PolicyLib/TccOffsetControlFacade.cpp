/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "TccOffsetControlFacade.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

TccOffsetControlFacade::TccOffsetControlFacade(
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

TccOffsetControlFacade::~TccOffsetControlFacade(void)
{
}

Bool TccOffsetControlFacade::supportsTccOffsetControl(void)
{
	return m_domainProperties.implementsTccOffsetControlInterface();
}

Temperature TccOffsetControlFacade::getTccOffsetTemperature(void)
{
	if (m_tccOffset.isInvalid())
	{
		m_tccOffset.set(m_policyServices.domainTccOffsetControl->getTccOffsetTemperature(m_participantIndex, m_domainIndex));
	}
	return m_tccOffset.get();
}

void TccOffsetControlFacade::setTccOffsetTemperature(const Temperature& tccOffset)
{
	m_policyServices.domainTccOffsetControl->setTccOffsetTemperature(m_participantIndex, m_domainIndex, tccOffset);
	m_tccOffset.set(tccOffset);
}

Temperature TccOffsetControlFacade::getMaxTccOffsetTemperature(void)
{
	return m_policyServices.domainTccOffsetControl->getMaxTccOffsetTemperature(m_participantIndex, m_domainIndex);
}

Temperature TccOffsetControlFacade::getMinTccOffsetTemperature(void)
{
	return m_policyServices.domainTccOffsetControl->getMinTccOffsetTemperature(m_participantIndex, m_domainIndex);
}
