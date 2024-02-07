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

#include "ActiveControlStaticCapsCachedProperty.h"
#include "DptfRequest.h"
using namespace std;

ActiveControlStaticCapsCachedProperty::ActiveControlStaticCapsCachedProperty(
	UIntN participantIndex,
	UIntN domainIndex,
	const DomainProperties& domainProperties,
	const PolicyServicesInterfaceContainer& policyServices)
	: CachedProperty()
	, DomainProperty(participantIndex, domainIndex, domainProperties, policyServices)
	, m_capabilities(false, false, 0)
{
}

ActiveControlStaticCapsCachedProperty::~ActiveControlStaticCapsCachedProperty(void)
{
}

void ActiveControlStaticCapsCachedProperty::refreshData(void)
{
	DptfRequest request(DptfRequestType::ActiveControlGetStaticCaps, getParticipantIndex(), getDomainIndex());
	auto result = getPolicyServices().serviceRequest->submitRequest(request);
	result.throwIfFailure();
	m_capabilities = ActiveControlStaticCaps::createFromFif(result.getData());
}

Bool ActiveControlStaticCapsCachedProperty::supportsProperty(void)
{
	return getDomainProperties().implementsActiveControlInterface();
}

const ActiveControlStaticCaps& ActiveControlStaticCapsCachedProperty::getCapabilities()
{
	if (isCacheValid() == false)
	{
		refresh();
	}
	return m_capabilities;
}
