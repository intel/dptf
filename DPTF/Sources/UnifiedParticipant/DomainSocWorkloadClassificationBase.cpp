/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "DomainSocWorkloadClassificationBase.h"

DomainSocWorkloadClassificationBase::DomainSocWorkloadClassificationBase(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	bindRequestHandlers();
}

DomainSocWorkloadClassificationBase::~DomainSocWorkloadClassificationBase()
{
}

void DomainSocWorkloadClassificationBase::bindRequestHandlers()
{
	bindRequestHandler(DptfRequestType::ClearCachedData, [=](const PolicyRequest& policyRequest) {
		return this->handleClearCachedResults(policyRequest);
	});
	bindRequestHandler(
		DptfRequestType::SocWorkloadClassificationGetSocWorkload,
		[=](const PolicyRequest& policyRequest) { return this->handleGetSocWorkloadClassification(policyRequest); });
	bindRequestHandler(
		DptfRequestType::SocWorkloadClassificationGetExtendedWorkloadPrediction,
		[=](const PolicyRequest& policyRequest) { return this->handleGetExtendedWorkloadPrediction(policyRequest); });
}

DptfRequestResult DomainSocWorkloadClassificationBase::handleClearCachedResults(const PolicyRequest& policyRequest)
{
	clearAllCachedResults();
	auto& request = policyRequest.getRequest();
	DptfRequestResult result(true, "Successfully cleared all cached results.", request);

	return result;
}

DptfRequestResult DomainSocWorkloadClassificationBase::handleGetSocWorkloadClassification(
	const PolicyRequest& policyRequest)
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
			auto currentSocWorkload = getSocWorkloadClassification();
			DptfRequestResult result(true, "Successfully retrieved current Soc workload.", request);
			result.setDataFromUInt32(currentSocWorkload);
			updateCachedResult(result);

			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(false, "Failed to retrieve current Soc workload: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to retrieve current Soc workload.", request);
	}
}

// We do not use cache here as the cache is used by SOC Workload classification DptfRequestResult
DptfRequestResult DomainSocWorkloadClassificationBase::handleGetExtendedWorkloadPrediction(
	const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		auto currentExtendedWorkloadPrediction = getExtendedWorkloadPrediction();
		DptfRequestResult result(true, "Successfully retrieved current Extended Workload Prediction.", request);
		result.setDataFromUInt32(currentExtendedWorkloadPrediction);

		return result;
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(false, "Failed to retrieve current Extended Workload Prediction: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to retrieve current Extended Workload Prediction.", request);
	}
}

void DomainSocWorkloadClassificationBase::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	// Do nothing
}

void DomainSocWorkloadClassificationBase::onClearCachedData(void)
{
	// Do nothing
}