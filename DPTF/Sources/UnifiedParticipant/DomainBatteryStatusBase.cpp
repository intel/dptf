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

#include "DomainBatteryStatusBase.h"

DomainBatteryStatusBase::DomainBatteryStatusBase(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	bindRequestHandlers();
}

DomainBatteryStatusBase::~DomainBatteryStatusBase()
{
}

void DomainBatteryStatusBase::onClearCachedData(void)
{
	clearAllCachedResults();
}

void DomainBatteryStatusBase::bindRequestHandlers()
{
	bindRequestHandler(DptfRequestType::BatteryStatusGetMaxBatteryPower, [=](const PolicyRequest& policyRequest) {
		return this->handleGetMaxBatteryPower(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BatteryStatusGetBatteryStatus, [=](const PolicyRequest& policyRequest) {
		return this->handleGetBatteryStatus(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BatteryStatusGetBatteryInformation, [=](const PolicyRequest& policyRequest) {
		return this->handleGetBatteryInformation(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BatteryStatusGetChargerType, [=](const PolicyRequest& policyRequest) {
		return this->handleGetChargerType(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BatteryStatusGetBatterySteadyState, [=](const PolicyRequest& policyRequest) {
		return this->handleGetBatterySteadyState(policyRequest);
	});
	bindRequestHandler(DptfRequestType::ClearCachedData, [=](const PolicyRequest& policyRequest) {
		return this->handleClearCachedData(policyRequest);
	});
	bindRequestHandler(
		DptfRequestType::BatteryStatusGetBatteryHighFrequencyImpedance, [=](const PolicyRequest& policyRequest) {
		return this->handleGetBatteryHighFrequencyImpedance(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BatteryStatusGetBatteryNoLoadVoltage, [=](const PolicyRequest& policyRequest) {
		return this->handleGetBatteryNoLoadVoltage(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BatteryStatusGetBatteryMaxPeakCurrent, [=](const PolicyRequest& policyRequest) {
		return this->handleGetBatteryMaxPeakCurrent(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BatteryStatusGetBatteryPercentage, [=](const PolicyRequest& policyRequest) {
		return this->handleGetBatteryPercentage(policyRequest);
	});
	bindRequestHandler(DptfRequestType::BatteryStatusSetBatteryPercentage, [=](const PolicyRequest& policyRequest) {
		return this->handleSetBatteryPercentage(policyRequest);
	});
}

DptfRequestResult DomainBatteryStatusBase::handleGetMaxBatteryPower(const PolicyRequest& policyRequest)
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
			auto pmax = getMaxBatteryPower();
			DptfRequestResult result(true, "Successfully retrieved Max Battery Power.", request);
			result.setData(pmax.toDptfBuffer());
			updateCachedResult(result);
			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to retrieve Max Battery Power: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBatteryStatusBase::handleGetBatteryStatus(const PolicyRequest& policyRequest)
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
			auto bst = getBatteryStatus();
			DptfRequestResult result(true, "Successfully retrieved Battery Status.", request);
			result.setData(bst);
			updateCachedResult(result);
			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to retrieve Battery Status: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBatteryStatusBase::handleGetBatteryInformation(const PolicyRequest& policyRequest)
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
			auto bix = getBatteryInformation();
			DptfRequestResult result(true, "Successfully retrieved Battery Information.", request);
			result.setData(bix);
			updateCachedResult(result);
			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(
			false, "Failed to retrieve Battery Information: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBatteryStatusBase::handleGetChargerType(const PolicyRequest& policyRequest)
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
			auto ctyp = getChargerType();
			DptfRequestResult result(true, "Successfully retrieved Charger Type.", request);
			result.setData(ChargerType::toDptfBuffer(ctyp));
			updateCachedResult(result);
			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(false, "Failed to retrieve Charger Type: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBatteryStatusBase::handleGetBatterySteadyState(const PolicyRequest& policyRequest)
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
			auto pbss = getPlatformBatterySteadyState();
			DptfRequestResult result(true, "Successfully retrieved Battery Steady State.", request);
			result.setData(pbss.toDptfBuffer());
			updateCachedResult(result);
			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(
			false, "Failed to retrieve Battery Steady State: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBatteryStatusBase::handleClearCachedData(const PolicyRequest& policyRequest)
{
	clearAllCachedResults();
	auto& request = policyRequest.getRequest();
	DptfRequestResult result(true, "Successfully cleared all cached requests.", request);
	return result;
}

DptfRequestResult DomainBatteryStatusBase::handleGetBatteryHighFrequencyImpedance(const PolicyRequest& policyRequest)
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
			auto rbhf = getBatteryHighFrequencyImpedance();
			DptfRequestResult result(true, "Successfully retrieved Battery High Frequency Impedance.", request);
			result.setDataFromUInt32(rbhf);
			updateCachedResult(result);
			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(
			false, "Failed to retrieve Battery High Frequency Impedance: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBatteryStatusBase::handleGetBatteryNoLoadVoltage(const PolicyRequest& policyRequest)
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
			auto vbnl = getBatteryNoLoadVoltage();
			DptfRequestResult result(true, "Successfully retrieved Battery No Load Voltage.", request);
			result.setDataFromUInt32(vbnl);
			updateCachedResult(result);
			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(
			false, "Failed to retrieve Battery No Load Voltage: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBatteryStatusBase::handleGetBatteryMaxPeakCurrent(const PolicyRequest& policyRequest)
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
			auto cmpp = getBatteryMaxPeakCurrent();
			DptfRequestResult result(true, "Successfully retrieved Battery Max Peak Current.", request);
			result.setDataFromUInt32(cmpp);
			updateCachedResult(result);
			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(
			false, "Failed to retrieve Battery Max Peak Current: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBatteryStatusBase::handleGetBatteryPercentage(const PolicyRequest& policyRequest)
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
			auto batteryPercentage = getBatteryPercentage();
			DptfRequestResult result(true, "Successfully retrieved Battery Percentage.", request);
			result.setDataFromUInt32(batteryPercentage.toWholeNumber());
			updateCachedResult(result);
			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		DptfRequestResult failureResult(
			false, "Failed to retrieve Battery Percentage: " + ex.getDescription(), request);
		return failureResult;
	}
}

DptfRequestResult DomainBatteryStatusBase::handleSetBatteryPercentage(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		auto batteryPercentage = Percentage::createFromDptfBuffer(request.getData());
		setBatteryPercentage(batteryPercentage);

		return DptfRequestResult(true, "Successfully set battery percentage.", request);
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(false, "Failed to set battery percentage: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to set battery percentage.", request);
	}
}
