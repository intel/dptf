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

#include "PerformanceControlStatusCachedProperty.h"
using namespace std;

PerformanceControlStatusCachedProperty::PerformanceControlStatusCachedProperty(
	UIntN participantIndex,
	UIntN domainIndex,
	const DomainProperties& domainProperties,
	const PolicyServicesInterfaceContainer& policyServices)
	: CachedProperty()
	, DomainProperty(participantIndex, domainIndex, domainProperties, policyServices)
	, m_performanceControlStatus(Constants::Invalid)
{
}

PerformanceControlStatusCachedProperty::~PerformanceControlStatusCachedProperty()
{
}

Bool PerformanceControlStatusCachedProperty::implementsPerformanceControlInterface(void)
{
	return getDomainProperties().implementsPerformanceControlInterface();
}

const PerformanceControlStatus& PerformanceControlStatusCachedProperty::getStatus(void)
{
	if (implementsPerformanceControlInterface())
	{
		if (isCacheValid() == false)
		{
			refresh();
		}
		return m_performanceControlStatus;
	}
	else
	{
		throw dptf_exception("Domain does not support the performance control interface.");
	}
}

Bool PerformanceControlStatusCachedProperty::supportsProperty(void)
{
	if (isCacheValid() == false)
	{
		refresh();
	}
	return implementsPerformanceControlInterface();
}

void PerformanceControlStatusCachedProperty::refreshData(void)
{
	m_performanceControlStatus = getPolicyServices().domainPerformanceControl->getPerformanceControlStatus(
		getParticipantIndex(), getDomainIndex());
}
