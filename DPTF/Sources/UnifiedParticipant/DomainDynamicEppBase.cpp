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

#include "DomainDynamicEppBase.h"

DomainDynamicEppBase::DomainDynamicEppBase(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_isDynamicEppSupported(false)
{
	bindRequestHandlers();
}

DomainDynamicEppBase::~DomainDynamicEppBase()
{
}

void DomainDynamicEppBase::bindRequestHandlers()
{
	bindRequestHandler(DptfRequestType::ClearCachedData, [=](const PolicyRequest& policyRequest) {
		return this->handleClearCachedResults(policyRequest);
		});
	bindRequestHandler(
		DptfRequestType::DynamicEppGetEppSensitivityHint,
		[=](const PolicyRequest& policyRequest) { return this->handleGetEppSensitivityHint(policyRequest); });
	bindRequestHandler(
		DptfRequestType::DynamicEppGetDynamicEppSupport, [=](const PolicyRequest& policyRequest) {
			return this->handleGetDynamicEppSupport(policyRequest);
		});
}

DptfRequestResult DomainDynamicEppBase::handleClearCachedResults(const PolicyRequest& policyRequest)
{
	clearAllCachedResults();
	auto& request = policyRequest.getRequest();
	DptfRequestResult result(true, "Successfully cleared all cached results.", request);

	return result;
}

DptfRequestResult DomainDynamicEppBase::handleGetEppSensitivityHint(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		if (requestResultIsCached(request))
		{
			return getCachedResult(request);
		}
		else
		{
			const UInt32 eppHint = getEppSensitivityHint();
			DptfRequestResult result(true, "Successfully retrieved current EPP Sensitivity Hint.", request);
			result.setDataFromUInt32(eppHint);
			updateCachedResult(result);

			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(false, "Failed to retrieve current EPP Sensitivity Hint: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to retrieve current EPP Sensitivity Hint.", request);
	}
}

DptfRequestResult DomainDynamicEppBase::handleGetDynamicEppSupport(const PolicyRequest& policyRequest) const
{
	auto& request = policyRequest.getRequest();
	DptfRequestResult result(true, "Successfully retrieved Dynamic EPP support.", request);
	result.setDataFromBool(m_isDynamicEppSupported);

	return result;
}

void DomainDynamicEppBase::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	// Do nothing
}

void DomainDynamicEppBase::onClearCachedData()
{
	// Do nothing
}