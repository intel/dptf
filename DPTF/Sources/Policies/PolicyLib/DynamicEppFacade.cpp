/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "DynamicEppFacade.h"
using namespace std;

DynamicEppFacade::DynamicEppFacade(
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

DynamicEppFacade::~DynamicEppFacade()
{
}

UInt32 DynamicEppFacade::getCurrentEppSensitivityHint()
{
	// This function is called only after checking if soc workload or dynamic epp is supported
	DptfRequest request(DptfRequestType::DynamicEppGetEppSensitivityHint, m_participantIndex, m_domainIndex);
	auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();

	return result.getDataAsUInt32();
}

Bool DynamicEppFacade::supportsDynamicEpp()
{
	if (supportsDynamicEppInterface())
	{
		Bool isDynamicEppSupported = false;
		DptfRequest request(
			DptfRequestType::DynamicEppGetDynamicEppSupport, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			isDynamicEppSupported = result.getDataAsBool();
		}

		return isDynamicEppSupported;
	}

	return false;
}

void DynamicEppFacade::setDynamicEppSupport(UInt32 dynamicEppSupport)
{
	if (supportsDynamicEppInterface())
	{
		DptfRequest request(DptfRequestType::DynamicEppSetDynamicEppSupport, m_participantIndex, m_domainIndex);
		request.setDataFromUInt32(dynamicEppSupport);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		result.throwIfFailure();
	}
}

Bool DynamicEppFacade::supportsDynamicEppInterface()
{
	return m_domainProperties.implementsDynamicEppInterface();
}