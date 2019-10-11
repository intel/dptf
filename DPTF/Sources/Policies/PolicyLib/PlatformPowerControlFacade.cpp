/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "PlatformPowerControlFacade.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

PlatformPowerControlFacade::PlatformPowerControlFacade(
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

PlatformPowerControlFacade::~PlatformPowerControlFacade(void)
{
}

Bool PlatformPowerControlFacade::setUsbcPortPowerLimit(const UInt32 portNumber, const Power& powerLimit)
{
	throwIfControlNotSupported();
	DptfRequest request(DptfRequestType::PlaftormPowerControlSetPortPowerLimit, m_participantIndex, m_domainIndex);
	std::pair<UInt32, Power> portAndLimit;
	portAndLimit.first = portNumber;
	portAndLimit.second = powerLimit;
	request.setData(createDptfBuffer(portAndLimit));
	auto result = m_policyServices.serviceRequest->submitRequest(request);
	if (result.isSuccessful())
	{
		m_lastSetPowerLimit[portNumber] = powerLimit;
	}

	return result.isSuccessful();
}

Bool PlatformPowerControlFacade::supportsPlatformPowerControls() const
{
	return m_domainProperties.implementsPlatformPowerControlInterface();
}

void PlatformPowerControlFacade::throwIfControlNotSupported() const
{
	if (supportsPlatformPowerControls() == false)
	{
		throw dptf_exception(
			"Cannot perform platform power control action because \
							 platform power controls are not supported by the domain.");
	}
}

class DptfBuffer PlatformPowerControlFacade::createDptfBuffer(std::pair<UInt32, Power> portAndLimit) const
{
	DptfBuffer buffer;
	buffer.append((UInt8*)&portAndLimit, sizeof(portAndLimit));
	return buffer;
}