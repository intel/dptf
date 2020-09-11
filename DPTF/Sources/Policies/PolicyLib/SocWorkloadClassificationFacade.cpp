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

#include "SocWorkloadClassificationFacade.h"
using namespace std;

SocWorkloadClassificationFacade::SocWorkloadClassificationFacade(
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

SocWorkloadClassificationFacade::~SocWorkloadClassificationFacade()
{
}

UInt32 SocWorkloadClassificationFacade::getCurrentSocWorkload(void)
{
	if (supportsSocWorkloadClassification())
	{
		DptfRequest request(
			DptfRequestType::SocWorkloadClassificationGetSocWorkload, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		result.throwIfFailure();

		return result.getDataAsUInt32();
	}
	else
	{
		throw dptf_exception("Domain does not support Soc workload classification.");
	}
}

Bool SocWorkloadClassificationFacade::supportsSocWorkloadClassification()
{
	if (supportsSocWorkloadClassificationInterface())
	{
		Bool isSocWorkloadSupported = false;
		DptfRequest request(
			DptfRequestType::SocWorkloadClassificationGetSocWorkloadSupport, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			isSocWorkloadSupported = result.getDataAsBool();
		}

		return isSocWorkloadSupported;
	}

	return false;
}

Bool SocWorkloadClassificationFacade::supportsSocWorkloadClassificationInterface()
{
	return m_domainProperties.implementsSocWorkloadClassificationInterface();
}