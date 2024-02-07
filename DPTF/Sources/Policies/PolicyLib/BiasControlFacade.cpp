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

#include "BiasControlFacade.h"
using namespace std;

BiasControlFacade::BiasControlFacade(
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

Bool BiasControlFacade::supportsBiasControl() const
{
	return m_domainProperties.implementsBiasControlInterface();
}

void BiasControlFacade::setCpuOpboostEnableAC(Bool enabled)
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlSetCpuOpboostEnableAC, m_participantIndex, m_domainIndex);
	request.setDataFromUInt32(static_cast<UInt32>(enabled));
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
}

void BiasControlFacade::setCpuOpboostEnableDC(Bool enabled)
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlSetCpuOpboostEnableDC, m_participantIndex, m_domainIndex);
	request.setDataFromUInt32(static_cast<UInt32>(enabled));
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
}

void BiasControlFacade::setGpuOpboostEnableAC(Bool enabled)
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlSetGpuOpboostEnableAC, m_participantIndex, m_domainIndex);
	request.setDataFromUInt32(static_cast<UInt32>(enabled));
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
}

void BiasControlFacade::setGpuOpboostEnableDC(Bool enabled)
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlSetGpuOpboostEnableDC, m_participantIndex, m_domainIndex);
	request.setDataFromUInt32(static_cast<UInt32>(enabled));
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
}

void BiasControlFacade::setSplitRatio(const Percentage& splitRatio)
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlSetSplitRatio, m_participantIndex, m_domainIndex);
	request.setData(splitRatio.toDptfBuffer());
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
}

void BiasControlFacade::setSplitRatioMax(const Percentage& splitRatio)
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlSetSplitRatioMax, m_participantIndex, m_domainIndex);
	request.setData(splitRatio.toDptfBuffer());
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
}

Bool BiasControlFacade::getCpuOpboostEnableAC()
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlGetCpuOpboostEnableAC, m_participantIndex, m_domainIndex);
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
	return result.getDataAsBool();
}

Bool BiasControlFacade::getCpuOpboostEnableDC()
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlGetCpuOpboostEnableDC, m_participantIndex, m_domainIndex);
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
	return result.getDataAsBool();
}

Bool BiasControlFacade::getGpuOpboostEnableAC()
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlGetGpuOpboostEnableAC, m_participantIndex, m_domainIndex);
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
	return result.getDataAsBool();
}

Bool BiasControlFacade::getGpuOpboostEnableDC()
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlGetGpuOpboostEnableDC, m_participantIndex, m_domainIndex);
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
	return result.getDataAsBool();
}

Percentage BiasControlFacade::getSplitRatio()
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlGetSplitRatio, m_participantIndex, m_domainIndex);
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
	return Percentage::createFromDptfBuffer(result.getData());
}

Percentage BiasControlFacade::getSplitRatioActive()
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlGetSplitRatioActive, m_participantIndex, m_domainIndex);
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
	return Percentage::createFromDptfBuffer(result.getData());
}

Percentage BiasControlFacade::getSplitRatioMax()
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlGetSplitRatioMax, m_participantIndex, m_domainIndex);
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
	return Percentage::createFromDptfBuffer(result.getData());
}

Power BiasControlFacade::getReservedTgp()
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlGetReservedTgp, m_participantIndex, m_domainIndex);
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
	return Power::createFromDptfBuffer(result.getData());
}

OpportunisticBoostMode::Type BiasControlFacade::getOppBoostMode()
{
	throwIfControlUnsupported();
	DptfRequest request(DptfRequestType::BiasControlGetOppBoostMode, m_participantIndex, m_domainIndex);
	const auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
	return OpportunisticBoostMode::fromUInt32(result.getDataAsUInt32());
}

void BiasControlFacade::throwIfControlUnsupported() const
{
	if (!supportsBiasControl())
	{
		throw dptf_exception(
			"Cannot perform BiasControl action because the domain does not support the control.");
	}
}