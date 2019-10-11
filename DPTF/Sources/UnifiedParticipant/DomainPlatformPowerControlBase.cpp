/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "DomainPlatformPowerControlBase.h"
#include <DptfBufferStream.h>
using namespace std::placeholders;

DomainPlatformPowerControlBase::DomainPlatformPowerControlBase(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	bindRequestHandlers();
}

DomainPlatformPowerControlBase::~DomainPlatformPowerControlBase()
{
}

void DomainPlatformPowerControlBase::bindRequestHandlers()
{
	bindRequestHandler(DptfRequestType::PlaftormPowerControlSetPortPowerLimit, [=](const PolicyRequest& policyRequest) {
		return this->handleSetPortPowerLimit(policyRequest);
	});
	bindRequestHandler(DptfRequestType::ClearPolicyRequestsForAllControls, [=](const PolicyRequest& policyRequest) {
		return this->handleRemovePolicyRequests(policyRequest);
	});
}

DptfRequestResult DomainPlatformPowerControlBase::handleSetPortPowerLimit(const PolicyRequest& policyRequest)
{
	auto policyIndex = policyRequest.getPolicyIndex();
	auto& request = policyRequest.getRequest();

	std::pair<UInt32, Power> portAndLimit = createFromDptfBuffer(request.getData());
	UInt32 portNumber = portAndLimit.first;
	Power powerLimit = portAndLimit.second;
	auto newPowerLimit = m_arbitrator.arbitrate(policyIndex, portNumber, powerLimit);
	try
	{
		setPortPowerLimit(portNumber, newPowerLimit);
	}
	catch (dptf_exception& ex)
	{
		std::stringstream message;
		message << "Set power limit for port number " << portNumber << " for policy FAILED: " << ex.getDescription();
		return DptfRequestResult(false, message.str(), request);
	}

	m_arbitrator.commitPolicyRequest(policyIndex, portNumber, powerLimit);
	std::stringstream message;
	message << "Set power limit for port number " << portNumber << " for policy.";

	sendActivityLoggingDataIfEnabled(getParticipantIndex(), getDomainIndex());

	return DptfRequestResult(true, message.str(), request);
}

DptfRequestResult DomainPlatformPowerControlBase::handleRemovePolicyRequests(const PolicyRequest& policyRequest)
{
	// the only thing the policy can set is the power limit in this control
	auto& request = policyRequest.getRequest();
	auto policyIndex = policyRequest.getPolicyIndex();

	auto portNumbers = m_arbitrator.removeRequestAndGetAffectedPortNumbers(policyIndex);
	if (m_arbitrator.arbitratedValueChanged())
	{
		for (auto portNumber = portNumbers.begin(); portNumber != portNumbers.end(); ++portNumber)
		{
			Power arbitratedPowerLimit = m_arbitrator.getArbitratedValue(*portNumber);
			setPortPowerLimit(*portNumber, arbitratedPowerLimit);
		}
	}

	return DptfRequestResult(true, "Removed policy set power limit requests from Platform Power Control.", request);
}

std::pair<UInt32, Power> DomainPlatformPowerControlBase::createFromDptfBuffer(const DptfBuffer& buffer)
{
	if (buffer.size() != (sizeof(UInt32) + sizeof(Power)))
	{
		throw dptf_exception("Buffer given to Platform Power Control class has invalid length.");
	}

	DptfBuffer bufferCopy = buffer;
	DptfBufferStream stream(bufferCopy);

	std::pair<UInt32, Power> newRequest;
	newRequest.first = stream.readNextUint32();
	newRequest.second = stream.readNextPower();
	return newRequest;
}
