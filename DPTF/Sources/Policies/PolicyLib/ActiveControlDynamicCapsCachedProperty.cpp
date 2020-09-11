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

#include "ActiveControlDynamicCapsCachedProperty.h"
using namespace std;

ActiveControlDynamicCapsCachedProperty::ActiveControlDynamicCapsCachedProperty(
	UIntN participantIndex,
	UIntN domainIndex,
	const DomainProperties& domainProperties,
	const PolicyServicesInterfaceContainer& policyServices)
	: CachedProperty()
	, DomainProperty(participantIndex, domainIndex, domainProperties, policyServices)
	, m_capabilities()
{
}

ActiveControlDynamicCapsCachedProperty::~ActiveControlDynamicCapsCachedProperty(void)
{
}

void ActiveControlDynamicCapsCachedProperty::refreshData(void)
{
	DptfRequest request(DptfRequestType::ActiveControlGetDynamicCaps, getParticipantIndex(), getDomainIndex());
	auto result = getPolicyServices().serviceRequest->submitRequest(request);
	result.throwIfFailure();
	try
	{
		m_capabilities = ActiveControlDynamicCaps::createFromFcdc(result.getData());
	}
	catch (const std::exception&)
	{
		// assume full fan capabilities if failure to retrieve from the system
		m_capabilities = ActiveControlDynamicCaps(Percentage(0.0), Percentage(1.0));
	}
}

Bool ActiveControlDynamicCapsCachedProperty::supportsProperty(void)
{
	return getDomainProperties().implementsActiveControlInterface();
}

const ActiveControlDynamicCaps& ActiveControlDynamicCapsCachedProperty::getCapabilities()
{
	if (isCacheValid() == false)
	{
		refresh();
	}
	return m_capabilities;
}
