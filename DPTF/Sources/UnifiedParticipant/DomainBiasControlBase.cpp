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

#include "DomainBiasControlBase.h"

DomainBiasControlBase::DomainBiasControlBase(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_isBiasControlSupported(false)
{
	bindRequestHandlers();
}

void DomainBiasControlBase::bindRequestHandlers()
{
	bindRequestHandler(DptfRequestType::ClearCachedData, [=](const PolicyRequest& policyRequest) {
		return this->handleClearCachedResults(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlSetCpuOpboostEnableAC, [=](const PolicyRequest& policyRequest) {
		return this->handleSetCpuOpboostEnableAC(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlSetCpuOpboostEnableDC, [=](const PolicyRequest& policyRequest) {
		return this->handleSetCpuOpboostEnableDC(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlSetGpuOpboostEnableAC, [=](const PolicyRequest& policyRequest) {
		return this->handleSetGpuOpboostEnableAC(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlSetGpuOpboostEnableDC, [=](const PolicyRequest& policyRequest) {
		return this->handleSetGpuOpboostEnableDC(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlSetSplitRatio, [=](const PolicyRequest& policyRequest) {
		return this->handleSetSplitRatio(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlSetSplitRatioMax, [=](const PolicyRequest& policyRequest) {
		return this->handleSetSplitRatioMax(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlGetCpuOpboostEnableAC, [=](const PolicyRequest& policyRequest) {
		return this->handleGetCpuOpboostEnableAC(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlGetCpuOpboostEnableDC, [=](const PolicyRequest& policyRequest) {
		return this->handleGetCpuOpboostEnableDC(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlGetGpuOpboostEnableAC, [=](const PolicyRequest& policyRequest) {
		return this->handleGetGpuOpboostEnableAC(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlGetGpuOpboostEnableDC, [=](const PolicyRequest& policyRequest) {
		return this->handleGetGpuOpboostEnableDC(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlGetSplitRatio, [=](const PolicyRequest& policyRequest) {
		return this->handleGetSplitRatio(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlGetSplitRatioActive, [=](const PolicyRequest& policyRequest) {
		return this->handleGetSplitRatioActive(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlGetSplitRatioMax, [=](const PolicyRequest& policyRequest) {
		return this->handleGetSplitRatioMax(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlGetReservedTgp, [=](const PolicyRequest& policyRequest) {
		return this->handleGetReservedTgp(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BiasControlGetOppBoostMode, [=](const PolicyRequest& policyRequest) {
		return this->handleGetOppBoostMode(policyRequest);
	});
}

DptfRequestResult DomainBiasControlBase::handleClearCachedResults(const PolicyRequest& policyRequest)
{
	clearAllCachedResults();
	const auto& request = policyRequest.getRequest();
	DptfRequestResult result(true, "Successfully cleared all cached results.", request);

	return result;
}

DptfRequestResult DomainBiasControlBase::handleSetCpuOpboostEnableAC(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto cpuOpboostEnable = static_cast<Bool>(request.getDataAsUInt32());
		setCpuOpboostEnableAC(cpuOpboostEnable);
	}
	catch (dptf_exception& ex)
	{
		return { false, "Failed to set CPU Opboost Enable AC: " + ex.getDescription(), request };
	}
	return { true, "Successfully set CPU Opboost Enable AC", request };
}

DptfRequestResult DomainBiasControlBase::handleSetCpuOpboostEnableDC(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto cpuOpboostEnable = static_cast<Bool>(request.getDataAsUInt32());
		setCpuOpboostEnableDC(cpuOpboostEnable);
	}
	catch (dptf_exception& ex)
	{
		return { false, "Failed to set CPU Opboost Enable DC: " + ex.getDescription(), request };
	}
	return { true, "Successfully set CPU Opboost Enable DC", request };
}

DptfRequestResult DomainBiasControlBase::handleSetGpuOpboostEnableAC(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto gpuOpboostEnable = static_cast<Bool>(request.getDataAsUInt32());
		setGpuOpboostEnableAC(gpuOpboostEnable);
	}
	catch (dptf_exception& ex)
	{
		return { false, "Failed to set GPU Opboost Enable AC: " + ex.getDescription(), request };
	}
	return { true, "Successfully set GPU Opboost Enable AC", request };
}

DptfRequestResult DomainBiasControlBase::handleSetGpuOpboostEnableDC(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto gpuOpboostEnable = static_cast<Bool>(request.getDataAsUInt32());
		setGpuOpboostEnableDC(gpuOpboostEnable);
	}
	catch (dptf_exception& ex)
	{
		return { false, "Failed to set GPU Opboost Enable DC: " + ex.getDescription(), request };
	}
	return { true, "Successfully set GPU Opboost Enable DC", request };
}

DptfRequestResult DomainBiasControlBase::handleSetSplitRatio(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto splitRatio = Percentage::createFromDptfBuffer(request.getData());
		setSplitRatio(splitRatio);
	}
	catch (dptf_exception& ex)
	{
		return { false, "Failed to set Split Ratio: " + ex.getDescription(), request };
	}
	return { true, "Successfully set Split Ratio", request };
}

DptfRequestResult DomainBiasControlBase::handleSetSplitRatioMax(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto splitRatioMax = Percentage::createFromDptfBuffer(request.getData());
		setSplitRatioMax(splitRatioMax);
	}
	catch (dptf_exception& ex)
	{
		return { false, "Failed to set Split Ratio Max: " + ex.getDescription(), request };
	}
	return { true, "Successfully set Split Ratio Max", request };
}

DptfRequestResult DomainBiasControlBase::handleGetCpuOpboostEnableAC(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto cpuOpboostEnable = getCpuOpboostEnableAC();
		DptfRequestResult result(true, "Successfully retrieved CPU Opboost Enable AC", request);
		result.setDataFromBool(cpuOpboostEnable);
		return result;
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to get CPU Opboost Enable AC: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBiasControlBase::handleGetCpuOpboostEnableDC(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto cpuOpboostEnable = getCpuOpboostEnableDC();
		DptfRequestResult result(true, "Successfully retrieved CPU Opboost Enable DC", request);
		result.setDataFromBool(cpuOpboostEnable);
		return result;
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to get CPU Opboost Enable DC: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBiasControlBase::handleGetGpuOpboostEnableAC(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto gpuOpboostEnable = getGpuOpboostEnableAC();
		DptfRequestResult result(true, "Successfully retrieved GPU Opboost Enable AC", request);
		result.setDataFromBool(gpuOpboostEnable);
		return result;
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to get GPU Opboost Enable AC: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBiasControlBase::handleGetGpuOpboostEnableDC(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto gpuOpboostEnable = getGpuOpboostEnableDC();
		DptfRequestResult result(true, "Successfully retrieved GPU Opboost Enable DC", request);
		result.setDataFromBool(gpuOpboostEnable);
		return result;
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to get GPU Opboost Enable DC: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBiasControlBase::handleGetSplitRatio(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto splitRatio = getSplitRatio();
		DptfRequestResult result(true, "Successfully retrieved Split Ratio", request);
		result.setData(splitRatio.toDptfBuffer());
		return result;
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to get Split Ratio: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBiasControlBase::handleGetSplitRatioActive(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto splitRatioActive = getSplitRatioActive();
		DptfRequestResult result(true, "Successfully retrieved Split Ratio Active", request);
		result.setData(splitRatioActive.toDptfBuffer());
		return result;
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to get Split Ratio Active: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBiasControlBase::handleGetSplitRatioMax(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto splitRatioMax = getSplitRatioMax();
		DptfRequestResult result(true, "Successfully retrieved Split Ratio Max", request);
		result.setData(splitRatioMax.toDptfBuffer());
		return result;
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to get Split Ratio Max: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBiasControlBase::handleGetReservedTgp(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto reservedTgp = getReservedTgp();
		DptfRequestResult result(true, "Successfully retrieved Reserved TGP", request);
		result.setData(reservedTgp.toDptfBuffer());
		return result;
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to get Reserved TGP: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBiasControlBase::handleGetOppBoostMode(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	try
	{
		const auto mode = getOppBoostMode();
		DptfRequestResult result(true, "Successfully retrieved Opportunistic Boost Mode", request);
		result.setDataFromUInt32(toUInt32(mode));
		return result;
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to get Opportunistic Boost Mode: " + ex.getDescription(), request);
		return failureResult;
	}
}

void DomainBiasControlBase::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	// Do nothing
}

void DomainBiasControlBase::onClearCachedData()
{
	// Do nothing
}
