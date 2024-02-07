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

#include "DomainProcessorControlBase.h"

DomainProcessorControlBase::DomainProcessorControlBase(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	bindRequestHandlers();
}

DomainProcessorControlBase::~DomainProcessorControlBase()
{
}

void DomainProcessorControlBase::bindRequestHandlers()
{
	bindRequestHandler(DptfRequestType::ClearCachedData, [=](const PolicyRequest& policyRequest) {
		return this->handleClearCachedData(policyRequest);
	});
	bindRequestHandler(
		DptfRequestType::ProcessorControlSetUnderVoltageThreshold,
		[=](const PolicyRequest& policyRequest) { return this->handleSetUnderVoltageThreshold(policyRequest); });
	bindRequestHandler(DptfRequestType::ClearPolicyRequestsForAllControls, [=](const PolicyRequest& policyRequest) {
		return this->handleRemovePolicyRequests(policyRequest);
	});
	bindRequestHandler(
		DptfRequestType::ProcessorControlSetTccOffsetTemperature,
		[=](const PolicyRequest& policyRequest) { return this->handleSetTccOffsetTemperature(policyRequest); });
	bindRequestHandler(
		DptfRequestType::ProcessorControlGetMaxTccOffsetTemperature,
		[=](const PolicyRequest& policyRequest) { return this->handleGetMaxTccOffsetTemperature(policyRequest); });
	bindRequestHandler(
		DptfRequestType::ProcessorControlGetMinTccOffsetTemperature,
		[=](const PolicyRequest& policyRequest) { return this->handleGetMinTccOffsetTemperature(policyRequest); });
	bindRequestHandler(
		DptfRequestType::ProcessorControlSetPerfPreferenceMax,
		[=](const PolicyRequest& policyRequest) { return this->handleSetPerfPreferenceMax(policyRequest); });
	bindRequestHandler(
		DptfRequestType::ProcessorControlSetPerfPreferenceMin,
		[=](const PolicyRequest& policyRequest) { return this->handleSetPerfPreferenceMin(policyRequest); });
	bindRequestHandler(
		DptfRequestType::ProcessorControlGetPcieThrottleRequestState,
		[=](const PolicyRequest& policyRequest) { return this->handleGetPcieThrottleRequestState(policyRequest); });
	bindRequestHandler(
		DptfRequestType::ProcessorControlGetSocGear,
		[=](const PolicyRequest& policyRequest) { return this->handleGetSocGear(policyRequest); });
	bindRequestHandler(
		DptfRequestType::ProcessorControlSetSocGear,
		[=](const PolicyRequest& policyRequest) { return this->handleSetSocGear(policyRequest); });
	bindRequestHandler(
		DptfRequestType::ProcessorControlGetSystemUsageMode,
		[=](const PolicyRequest& policyRequest) { return this->handleGetSocSystemUsageMode(policyRequest); });
	bindRequestHandler(
		DptfRequestType::ProcessorControlSetSystemUsageMode,
		[=](const PolicyRequest& policyRequest) { return this->handleSetSocSystemUsageMode(policyRequest); });
}

DptfRequestResult DomainProcessorControlBase::handleClearCachedData(const PolicyRequest& policyRequest)
{
	clearAllCachedResults();
	auto& request = policyRequest.getRequest();
	DptfRequestResult result(true, "Successfully cleared all cached requests.", request);
	return result;
}

DptfRequestResult DomainProcessorControlBase::handleSetUnderVoltageThreshold(const PolicyRequest& policyRequest)
{
	auto policyIndex = policyRequest.getPolicyIndex();
	auto& request = policyRequest.getRequest();

	UInt32 voltageThreshold = request.getDataAsUInt32();
	UInt32 currentThreshold = m_arbitrator.getArbitratedVoltageThresholdValue();
	auto newThreshold = m_arbitrator.arbitrateVoltageThresholdRequest(policyIndex, voltageThreshold);
	if (newThreshold != currentThreshold)
	{
		try
		{
			setUnderVoltageThreshold(newThreshold);
		}
		catch (dptf_exception& ex)
		{
			std::stringstream message;
			message << "Set under voltage threshold (UVTH) for policy FAILED: " << ex.getDescription();
			return DptfRequestResult(false, message.str(), request);
		}
	}

	m_arbitrator.commitPolicyVoltageThresholdRequest(policyIndex, voltageThreshold);
	std::stringstream message;
	message << "Set under voltage threshold (UVTH) for policy.";

	sendActivityLoggingDataIfEnabled(getParticipantIndex(), getDomainIndex());

	return DptfRequestResult(true, message.str(), request);
}

DptfRequestResult DomainProcessorControlBase::handleRemovePolicyRequests(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto policyIndex = policyRequest.getPolicyIndex();

	auto resultForRemovingVoltageRequest = removePolicySetUnderVoltageThresholdRequest(policyIndex, request);
	auto resultForRemovingOffsetRequest = removePolicySetTccOffsetTemperatureRequest(policyIndex, request);

	return DptfRequestResult(
		resultForRemovingVoltageRequest.isSuccessful() && resultForRemovingOffsetRequest.isSuccessful(),
		resultForRemovingVoltageRequest.getMessage() + "\n" + resultForRemovingOffsetRequest.getMessage(),
		request);
}

DptfRequestResult DomainProcessorControlBase::removePolicySetUnderVoltageThresholdRequest(
	UIntN policyIndex,
	const DptfRequest& request)
{
	m_arbitrator.removeVoltageThresholdRequest(policyIndex);
	if (m_arbitrator.arbitratedVoltageThresholdValueChanged())
	{
		setUnderVoltageThreshold(m_arbitrator.getArbitratedVoltageThresholdValue());
	}

	return DptfRequestResult(true, "Removed policy set UVTH request from Processor Control.", request);
}

DptfRequestResult DomainProcessorControlBase::handleSetTccOffsetTemperature(const PolicyRequest& policyRequest)
{
	auto policyIndex = policyRequest.getPolicyIndex();
	auto& request = policyRequest.getRequest();

	Temperature tccOffset = Temperature::createFromDptfBuffer(request.getData());
	Temperature currentTccOffset = m_arbitrator.getArbitratedTccOffsetValue();
	auto newTccOffset = m_arbitrator.arbitrateTccOffsetRequest(policyIndex, tccOffset);
	if (!currentTccOffset.isValid() || newTccOffset != currentTccOffset)
	{
		try
		{
			setTccOffsetTemperature(newTccOffset);
		}
		catch (dptf_exception& ex)
		{
			std::stringstream message;
			message << "Set TCC offset temperature for policy FAILED: " << ex.getDescription();
			return DptfRequestResult(false, message.str(), request);
		}
	}

	m_arbitrator.commitPolicyTccOffsetRequest(policyIndex, tccOffset);
	sendActivityLoggingDataIfEnabled(getParticipantIndex(), getDomainIndex());
	std::stringstream message;
	message << "Set TCC offset temperature for policy.";
	return DptfRequestResult(true, message.str(), request);
}

DptfRequestResult DomainProcessorControlBase::handleSetPerfPreferenceMax(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	try
	{
		Percentage cpuMaxRatio = Percentage::createFromDptfBuffer(request.getData());
		setPerfPreferenceMax(cpuMaxRatio);
	}
	catch (dptf_exception& ex)
	{
		std::stringstream message;
		message << "Set CPU MAX frequency for policy FAILED: " << ex.getDescription();
		return DptfRequestResult(false, message.str(), request);
	}
	
	std::stringstream message;
	message << "Set CPU MAX frequency for policy.";
	return DptfRequestResult(true, message.str(), request);
}

DptfRequestResult DomainProcessorControlBase::handleSetPerfPreferenceMin(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	try
	{
		Percentage cpuMinRatio = Percentage::createFromDptfBuffer(request.getData());
		setPerfPreferenceMin(cpuMinRatio);
	}
	catch (dptf_exception& ex)
	{
		std::stringstream message;
		message << "Set CPU MIN frequency for policy FAILED: " << ex.getDescription();
		return DptfRequestResult(false, message.str(), request);
	}

	std::stringstream message;
	message << "Set CPU MIN frequency for policy.";
	return DptfRequestResult(true, message.str(), request);
}

DptfRequestResult DomainProcessorControlBase::handleGetPcieThrottleRequestState(const PolicyRequest& policyRequest)
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
			auto pcieThrottleRequestState = getPcieThrottleRequestState();
			DptfRequestResult result(true, "Successfully retrieved PCIe Throttle Request State.", request);
			result.setDataFromUInt32(pcieThrottleRequestState);
			updateCachedResult(result);
			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(
			false, "Failed to retrieve PCIe Throttle Request State: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainProcessorControlBase::removePolicySetTccOffsetTemperatureRequest(
	UIntN policyIndex,
	const DptfRequest& request)
{
	m_arbitrator.removeTccOffsetRequest(policyIndex);
	if (m_arbitrator.arbitratedTccOffsetValueChanged())
	{
		setTccOffsetTemperature(m_arbitrator.getArbitratedTccOffsetValue());
	}

	return DptfRequestResult(true, "Removed policy set TCC offset request from Processor Control.", request);
}

DptfRequestResult DomainProcessorControlBase::handleGetMaxTccOffsetTemperature(const PolicyRequest& policyRequest)
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
			auto maxTccOffset = getMaxTccOffsetTemperature();
			DptfRequestResult result(true, "Successfully retrieved Max TCC offset temperature.", request);
			result.setData(maxTccOffset.toDptfBuffer());
			updateCachedResult(result);
			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(
			false, "Failed to retrieve Max TCC offset temperature: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainProcessorControlBase::handleGetMinTccOffsetTemperature(const PolicyRequest& policyRequest)
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
			auto minTccOffset = getMinTccOffsetTemperature();
			DptfRequestResult result(true, "Successfully retrieved Min TCC offset temperature.", request);
			result.setData(minTccOffset.toDptfBuffer());
			updateCachedResult(result);
			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(
			false, "Failed to retrieve Min TCC offset temperature: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainProcessorControlBase::handleGetSocGear(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		auto gear = getSocGear();
		DptfRequestResult result(true, "Successfully retrieved Soc gear", request);
		result.setDataFromUInt32(gear);
		return result;
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(
			false, "Failed to get Soc gear: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainProcessorControlBase::handleSetSocGear(const PolicyRequest& policyRequest)
{

	auto& request = policyRequest.getRequest();
	try
	{
		SocGear::Type gear = static_cast<SocGear::Type>(request.getDataAsUInt32());
		setSocGear(gear);
	}
	catch (dptf_exception& ex)
	{
		std::stringstream message;
		message << "Unable to set Soc gear: " << ex.getDescription();
		return DptfRequestResult(false, message.str(), request);
	}

	std::stringstream message;
	message << "Successfully set Soc gear!";
	return DptfRequestResult(true, message.str(), request);		
}

DptfRequestResult DomainProcessorControlBase::handleGetSocSystemUsageMode(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		auto mode = getSocSystemUsageMode();
		DptfRequestResult result(true, "Successfully retrieved Soc system usage mode", request);
		result.setDataFromUInt32(mode);
		return result;
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to get Soc system usage mode: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainProcessorControlBase::handleSetSocSystemUsageMode(const PolicyRequest& policyRequest)
{

	auto& request = policyRequest.getRequest();
	try
	{
		SystemUsageMode::Type mode = static_cast<SystemUsageMode::Type>(request.getDataAsUInt32());
		setSocSystemUsageMode(mode);
	}
	catch (dptf_exception& ex)
	{
		std::stringstream message;
		message << "Unable to set Soc system usage mode: " << ex.getDescription();
		return DptfRequestResult(false, message.str(), request);
	}

	std::stringstream message;
	message << "Successfully set Soc system usage mode!";
	return DptfRequestResult(true, message.str(), request);
}
