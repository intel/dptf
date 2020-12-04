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

#include "DomainActiveControlBase.h"
#include "DptfBufferStream.h"
#include "ActiveControlDynamicCaps.h"
using namespace std::placeholders;

DomainActiveControlBase::DomainActiveControlBase(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	bindRequestHandlers();
}

DomainActiveControlBase::~DomainActiveControlBase()
{
}

std::shared_ptr<XmlNode> DomainActiveControlBase::getArbitratorXml(UIntN policyIndex) const
{
	std::shared_ptr<XmlNode> node = XmlNode::createRoot();
	node->addChild(m_arbitratorFanSpeed.getStatusForPolicy(policyIndex));
	node->addChild(m_arbitratorDynamicCaps.getStatusForPolicy(policyIndex));
	return node;
}

void DomainActiveControlBase::bindRequestHandlers()
{
	bindRequestHandler(DptfRequestType::ActiveControlGetStaticCaps, [=](const PolicyRequest& policyRequest) {
		return this->handleGetStaticCaps(policyRequest);
	});
	bindRequestHandler(DptfRequestType::ActiveControlGetDynamicCaps, [=](const PolicyRequest& policyRequest) {
		return this->handleGetDynamicCaps(policyRequest);
	});
	bindRequestHandler(DptfRequestType::ActiveControlGetStatus, [=](const PolicyRequest& policyRequest) {
		return this->handleGetStatus(policyRequest);
	});
	bindRequestHandler(DptfRequestType::ActiveControlGetControlSet, [=](const PolicyRequest& policyRequest) {
		return this->handleGetControlSet(policyRequest);
	});
	bindRequestHandler(DptfRequestType::ActiveControlSetFanSpeed, [=](const PolicyRequest& policyRequest) {
		return this->handleSetFanSpeed(policyRequest);
	});
	bindRequestHandler(DptfRequestType::ActiveControlSetFanDirection, [=](const PolicyRequest& policyRequest) {
		return this->handleSetFanDirection(policyRequest);
	});
	bindRequestHandler(DptfRequestType::ActiveControlSetDynamicCaps, [=](const PolicyRequest& policyRequest) {
		return this->handleSetDynamicCaps(policyRequest);
	});
	bindRequestHandler(DptfRequestType::ActiveControlSetFanCapsLock, [=](const PolicyRequest& policyRequest) {
		return this->handleSetFanCapsLock(policyRequest);
	});
	bindRequestHandler(DptfRequestType::ClearPolicyRequestsForAllControls, [=](const PolicyRequest& policyRequest) {
		return this->handleRemovePolicyRequests(policyRequest);
	});
}

DptfRequestResult DomainActiveControlBase::handleGetStaticCaps(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto participantIndex = request.getParticipantIndex();
	auto domainIndex = request.getDomainIndex();

	if (requestResultIsCached(request))
	{
		return getCachedResult(request);
	}
	else
	{
		auto caps = getActiveControlStaticCaps(participantIndex, domainIndex);
		DptfRequestResult result(true, "Successfully retrieved active control static capabilities.", request);
		result.setData(caps);
		updateCachedResult(result);
		return result;
	}
}

DptfRequestResult DomainActiveControlBase::handleGetDynamicCaps(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto participantIndex = request.getParticipantIndex();
	auto domainIndex = request.getDomainIndex();

	if (requestResultIsCached(request))
	{
		return getCachedResult(request);
	}
	else
	{
		auto caps = getActiveControlDynamicCaps(participantIndex, domainIndex);
		DptfRequestResult result(true, "Successfully retrieved active control dynamic capabilities.", request);
		result.setData(caps);
		updateCachedResult(result);
		return result;
	}
}

DptfRequestResult DomainActiveControlBase::handleGetStatus(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto participantIndex = request.getParticipantIndex();
	auto domainIndex = request.getDomainIndex();

	if (requestResultIsCached(request))
	{
		return getCachedResult(request);
	}
	else
	{
		auto status = getActiveControlStatus(participantIndex, domainIndex);
		DptfRequestResult result(true, "Successfully retrieved active control status.", request);
		result.setData(status);
		updateCachedResult(result);
		return result;
	}
}

DptfRequestResult DomainActiveControlBase::handleGetControlSet(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto participantIndex = request.getParticipantIndex();
	auto domainIndex = request.getDomainIndex();

	if (requestResultIsCached(request))
	{
		return getCachedResult(request);
	}
	else
	{
		auto controlSet = getActiveControlSet(participantIndex, domainIndex);
		DptfRequestResult result(true, "Successfully retrieved active control set.", request);
		result.setData(controlSet);
		updateCachedResult(result);
		return result;
	}
}

DptfRequestResult DomainActiveControlBase::handleSetFanSpeed(const PolicyRequest& policyRequest)
{
	auto policyIndex = policyRequest.getPolicyIndex();
	auto& request = policyRequest.getRequest();
	auto participantIndex = request.getParticipantIndex();
	auto domainIndex = request.getDomainIndex();
	auto fanSpeedRequest = Percentage::createFromDptfBuffer(request.getData());

	auto currentArbitratedFanSpeed = m_arbitratorFanSpeed.getArbitratedValue();
	auto calculatedFanSpeedIfUpdated = m_arbitratorFanSpeed.calculateNewArbitratedValue(policyIndex, fanSpeedRequest);
	if (calculatedFanSpeedIfUpdated != currentArbitratedFanSpeed)
	{
		setActiveControl(participantIndex, domainIndex, calculatedFanSpeedIfUpdated);
		clearCachedResult(DptfRequestType::ActiveControlGetStatus, participantIndex, domainIndex);
		sendActivityLoggingDataIfEnabled(participantIndex, domainIndex);
	}
	m_arbitratorFanSpeed.commitRequest(policyIndex, fanSpeedRequest);
	return DptfRequestResult(true, "Set fan speed for policy.", request);
}

DptfRequestResult DomainActiveControlBase::handleSetFanDirection(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto fanDirectionRequest = request.getDataAsUInt32();

	try
	{
		setActiveControlFanDirection(fanDirectionRequest);
	}
	catch (dptf_exception& ex)
	{
		std::stringstream message;
		message << "Set fan direction for policy FAILED: " << ex.getDescription();
		return DptfRequestResult(false, message.str(), request);
	}

	std::stringstream message;
	message << "Set fan direction for policy.";
	return DptfRequestResult(true, message.str(), request);
}

DptfRequestResult DomainActiveControlBase::handleSetDynamicCaps(const PolicyRequest& policyRequest)
{
	auto policyIndex = policyRequest.getPolicyIndex();
	auto& request = policyRequest.getRequest();
	auto participantIndex = request.getParticipantIndex();
	auto domainIndex = request.getDomainIndex();
	auto capsRequest = ActiveControlDynamicCaps::createFromFcdc(request.getData());

	auto currentCaps = m_arbitratorDynamicCaps.getArbitratedCapabilities();
	auto calculatedCapsIfUpdated = m_arbitratorDynamicCaps.calculateNewArbitratedCapabilities(
		policyIndex, capsRequest.getMinFanSpeed(), capsRequest.getMaxFanSpeed());
	if (calculatedCapsIfUpdated != currentCaps)
	{
		setActiveControlDynamicCaps(participantIndex, domainIndex, calculatedCapsIfUpdated);
		clearCachedResult(DptfRequestType::ActiveControlGetDynamicCaps, participantIndex, domainIndex);
		sendActivityLoggingDataIfEnabled(participantIndex, domainIndex);
	}
	m_arbitratorDynamicCaps.commitDynamicCapsRequest(policyIndex, capsRequest);
	return DptfRequestResult(true, "Set fan dynamic capabilities for policy.", request);
}

DptfRequestResult DomainActiveControlBase::handleSetFanCapsLock(const PolicyRequest& policyRequest)
{
	auto policyIndex = policyRequest.getPolicyIndex();
	auto& request = policyRequest.getRequest();
	auto participantIndex = request.getParticipantIndex();
	auto domainIndex = request.getDomainIndex();
	DptfBuffer data = request.getData();
	DptfBufferStream stream(data);
	auto lock = stream.readNextBool();

	auto currentArbitratedLock = m_arbitratorDynamicCaps.getArbitratedLock();
	auto calculatedLockIfUpdated = m_arbitratorDynamicCaps.calculateNewArbitratedLock(policyIndex, lock);
	if (calculatedLockIfUpdated != currentArbitratedLock)
	{
		setFanCapsLock(participantIndex, domainIndex, calculatedLockIfUpdated);
	}
	m_arbitratorDynamicCaps.commitLockRequest(policyIndex, lock);
	return DptfRequestResult(true, "Set fan capabilities lock for policy.", request);
}

DptfRequestResult DomainActiveControlBase::handleRemovePolicyRequests(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto policyIndex = policyRequest.getPolicyIndex();
	auto participantIndex = request.getParticipantIndex();
	auto domainIndex = request.getDomainIndex();

	// remove fan speed request
	auto previousFanSpeed = m_arbitratorFanSpeed.getArbitratedValue();
	m_arbitratorFanSpeed.removeRequest(policyIndex);
	auto newFanSpeed = m_arbitratorFanSpeed.getArbitratedValue();
	if (previousFanSpeed != newFanSpeed)
	{
		setActiveControl(participantIndex, domainIndex, newFanSpeed);
		clearCachedResult(DptfRequestType::ActiveControlGetStatus, participantIndex, domainIndex);
		sendActivityLoggingDataIfEnabled(participantIndex, domainIndex);
	}

	// remove dynamic caps request
	auto previousCaps = m_arbitratorDynamicCaps.getArbitratedCapabilities();
	m_arbitratorDynamicCaps.removeCapabilitiesRequest(policyIndex);
	auto newCaps = m_arbitratorDynamicCaps.getArbitratedCapabilities();
	if (previousCaps != newCaps)
	{
		setActiveControlDynamicCaps(participantIndex, domainIndex, newCaps);
		clearCachedResult(DptfRequestType::ActiveControlGetDynamicCaps, participantIndex, domainIndex);
		sendActivityLoggingDataIfEnabled(participantIndex, domainIndex);
	}

	// remove dynamic caps lock request
	auto previousCapsLock = m_arbitratorDynamicCaps.getArbitratedLock();
	m_arbitratorDynamicCaps.removeLockRequest(policyIndex);
	auto newCapsLock = m_arbitratorDynamicCaps.getArbitratedLock();
	if (previousCapsLock != newCapsLock)
	{
		setFanCapsLock(participantIndex, domainIndex, newCapsLock);
	}
	return DptfRequestResult(true, "Removed policy requests from Active Control.", request);
}
