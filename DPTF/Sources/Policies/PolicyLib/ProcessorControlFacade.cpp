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

#include "ProcessorControlFacade.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

ProcessorControlFacade::ProcessorControlFacade(
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

ProcessorControlFacade::~ProcessorControlFacade(void)
{
}

Bool ProcessorControlFacade::supportsProcessorControl(void) const
{
	return m_domainProperties.implementsProcessorControlInterface();
}

Bool ProcessorControlFacade::setTccOffsetTemperature(const Temperature& tccOffset)
{
	throwIfControlNotSupported();
	DptfRequest request(DptfRequestType::ProcessorControlSetTccOffsetTemperature, m_participantIndex, m_domainIndex);
	request.setData(tccOffset.toDptfBuffer());
	auto result = m_policyServices.serviceRequest->submitRequest(request);

	return result.isSuccessful();
}

Bool ProcessorControlFacade::setPerfPreferenceMax(const Percentage& cpuMaxRatio)
{
	throwIfControlNotSupported();
	DptfRequest request(DptfRequestType::ProcessorControlSetPerfPreferenceMax , m_participantIndex, m_domainIndex);
	request.setData(cpuMaxRatio.toDptfBuffer());
	auto result = m_policyServices.serviceRequest->submitRequest(request);

	return result.isSuccessful();
}

Bool ProcessorControlFacade::setPerfPreferenceMin(const Percentage& cpuMinRatio)
{
	throwIfControlNotSupported();
	DptfRequest request(DptfRequestType::ProcessorControlSetPerfPreferenceMin, m_participantIndex, m_domainIndex);
	request.setData(cpuMinRatio.toDptfBuffer());
	auto result = m_policyServices.serviceRequest->submitRequest(request);

	return result.isSuccessful();
}

UInt32 ProcessorControlFacade::getPcieThrottleRequestState()
{
	if (supportsProcessorControl())
	{
		DptfRequest request(
			DptfRequestType::ProcessorControlGetPcieThrottleRequestState, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			return result.getDataAsUInt32();
		}
	}

	return (UInt32)0;
}

Temperature ProcessorControlFacade::getMaxTccOffsetTemperature(void)
{
	if (supportsProcessorControl())
	{
		DptfRequest request(DptfRequestType::ProcessorControlGetMaxTccOffsetTemperature, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			return Temperature::createFromDptfBuffer(result.getData());
		}
	}
	return Temperature::createInvalid();
}

Temperature ProcessorControlFacade::getMinTccOffsetTemperature(void)
{
	if (supportsProcessorControl())
	{
		DptfRequest request(DptfRequestType::ProcessorControlGetMinTccOffsetTemperature, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			return Temperature::createFromDptfBuffer(result.getData());
		}
	}
	return Temperature::createInvalid();
}

Bool ProcessorControlFacade::setUnderVoltageThreshold(const UInt32 voltageThreshold)
{
	throwIfControlNotSupported();
	DptfRequest request(DptfRequestType::ProcessorControlSetUnderVoltageThreshold, m_participantIndex, m_domainIndex);
	request.setDataFromUInt32(voltageThreshold);
	auto result = m_policyServices.serviceRequest->submitRequest(request);
	// do not cache, there is no "get" function
	return result.isSuccessful();
}

SocGear::Type ProcessorControlFacade::getSocGear() const
{
	if (supportsProcessorControl())
	{
		DptfRequest request(
			DptfRequestType::ProcessorControlGetSocGear, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			return static_cast<SocGear::Type>(result.getDataAsUInt32());
		}
	}

	return SocGear::Invalid;
}

Bool ProcessorControlFacade::setSocGear(SocGear::Type gear)
{
	if (supportsProcessorControl())
	{
		DptfRequest request(
			DptfRequestType::ProcessorControlSetSocGear, m_participantIndex, m_domainIndex);
		request.setDataFromUInt32(gear);
		auto result = m_policyServices.serviceRequest->submitRequest(request);

		return result.isSuccessful();		
	}

	return false;
}

SystemUsageMode::Type ProcessorControlFacade::getSocSystemUsageMode()
{
	if (supportsProcessorControl())
	{
		DptfRequest request(DptfRequestType::ProcessorControlGetSystemUsageMode, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		if (result.isSuccessful())
		{
			return static_cast<SystemUsageMode::Type>(result.getDataAsUInt32());
		}
	}

	return SystemUsageMode::Invalid;
}

Bool ProcessorControlFacade::setSocSystemUsageMode(SystemUsageMode::Type mode)
{
	if (supportsProcessorControl())
	{
		DptfRequest request(DptfRequestType::ProcessorControlSetSystemUsageMode, m_participantIndex, m_domainIndex);
		request.setDataFromUInt32(mode);
		auto result = m_policyServices.serviceRequest->submitRequest(request);

		return result.isSuccessful();
	}

	return false;
}

void ProcessorControlFacade::throwIfControlNotSupported() const
{
	if (supportsProcessorControl() == false)
	{
		throw dptf_exception(
			"Cannot perform processor control action because processor controls are not supported by the domain.");
	}
}
