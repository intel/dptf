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

#include "ArbitratorProcessorControl.h"
#include "StatusFormat.h"

ArbitratorProcessorControl::ArbitratorProcessorControl()
	: m_arbitratedVoltageValueChangedSinceLastSet(false)
	, m_arbitratedOffsetValueChangedSinceLastSet(false)
	, m_arbitratedVoltage(Constants::Invalid)
	, m_arbitratedTccOffset(Temperature::createInvalid())
{
}

ArbitratorProcessorControl::~ArbitratorProcessorControl(void)
{
}

UInt32 ArbitratorProcessorControl::arbitrateVoltageThresholdRequest(UIntN policyIndex, const UInt32 voltageThreshold)
{
	auto tempPolicyRequests = m_requestedVoltages;
	tempPolicyRequests[policyIndex] = voltageThreshold;
	auto arbitratedValue = getLowestVoltageRequest(tempPolicyRequests);
	return arbitratedValue;
}

void ArbitratorProcessorControl::commitPolicyVoltageThresholdRequest(UIntN policyIndex, const UInt32 voltageThreshold)
{
	m_requestedVoltages[policyIndex] = voltageThreshold;
	auto oldArbitratedValue = m_arbitratedVoltage;
	m_arbitratedVoltage = getLowestVoltageRequest(m_requestedVoltages);
	m_arbitratedVoltageValueChangedSinceLastSet = (oldArbitratedValue != m_arbitratedVoltage);
}

void ArbitratorProcessorControl::removeVoltageThresholdRequest(UIntN policyIndex)
{
	auto policyRequest = m_requestedVoltages.find(policyIndex);
	if (policyRequest != m_requestedVoltages.end())
	{
		auto oldArbitratedValue = m_arbitratedVoltage;
		m_requestedVoltages[policyIndex] = Constants::Invalid;
		m_arbitratedVoltage = getLowestVoltageRequest(m_requestedVoltages);
		m_arbitratedVoltageValueChangedSinceLastSet = (oldArbitratedValue != m_arbitratedVoltage);
	}
	else {
		m_arbitratedVoltageValueChangedSinceLastSet = false;
	}
}

Bool ArbitratorProcessorControl::arbitratedVoltageThresholdValueChanged() const
{
	return m_arbitratedVoltageValueChangedSinceLastSet;
}

UInt32 ArbitratorProcessorControl::getArbitratedVoltageThresholdValue() const
{
	return m_arbitratedVoltage;
}

Temperature ArbitratorProcessorControl::arbitrateTccOffsetRequest(UIntN policyIndex, const Temperature& tccOffset)
{
	auto tempPolicyRequests = m_requestedOffsets;
	tempPolicyRequests[policyIndex] = tccOffset;
	auto arbitratedValue = getLowestTccOffsetRequest(tempPolicyRequests);
	return arbitratedValue;
}

void ArbitratorProcessorControl::commitPolicyTccOffsetRequest(UIntN policyIndex, const Temperature& tccOffset)
{
	m_requestedOffsets[policyIndex] = tccOffset;
	auto oldArbitratedValue = m_arbitratedTccOffset;
	m_arbitratedTccOffset = getLowestTccOffsetRequest(m_requestedOffsets);
	m_arbitratedOffsetValueChangedSinceLastSet = (oldArbitratedValue != m_arbitratedTccOffset);
}

void ArbitratorProcessorControl::removeTccOffsetRequest(UIntN policyIndex)
{
	auto policyRequest = m_requestedOffsets.find(policyIndex);
	if (policyRequest != m_requestedOffsets.end())
	{
		auto oldArbitratedValue = m_arbitratedTccOffset;
		m_requestedOffsets[policyIndex] = Temperature::createInvalid();
		m_arbitratedTccOffset = getLowestTccOffsetRequest(m_requestedOffsets);
		m_arbitratedOffsetValueChangedSinceLastSet = (m_arbitratedTccOffset.isValid() && oldArbitratedValue != m_arbitratedTccOffset);
	}
	else {
		m_arbitratedOffsetValueChangedSinceLastSet = false;
	}
}

Bool ArbitratorProcessorControl::arbitratedTccOffsetValueChanged() const
{
	return m_arbitratedOffsetValueChangedSinceLastSet;
}

Temperature ArbitratorProcessorControl::getArbitratedTccOffsetValue() const
{
	return m_arbitratedTccOffset;
}

std::shared_ptr<XmlNode> ArbitratorProcessorControl::getStatusForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("processor_control_arbitrator_status");
	auto underVoltageThreshold = Constants::Invalid;
	auto policyVoltageRequest = m_requestedVoltages.find(policyIndex);
	if (policyVoltageRequest != m_requestedVoltages.end())
	{
		underVoltageThreshold = policyVoltageRequest->second;
	}
	requestRoot->addChild(XmlNode::createDataElement("under_voltage_threshold", StatusFormat::friendlyValue(underVoltageThreshold)));

	auto tccOffset = Temperature::createInvalid();
	auto policyOffsetRequest = m_requestedOffsets.find(policyIndex);
	if (policyOffsetRequest != m_requestedOffsets.end())
	{
		tccOffset = policyOffsetRequest->second;
	}
	requestRoot->addChild(XmlNode::createDataElement("tcc_offset", tccOffset.toString()));

	return requestRoot;
}

UInt32 ArbitratorProcessorControl::getLowestVoltageRequest(std::map<UIntN, UInt32> requestedVoltages)
{
	UInt32 lowestRequest = Constants::Invalid;
	for (auto policyRequest = requestedVoltages.begin(); policyRequest != requestedVoltages.end(); ++policyRequest)
	{
		UInt32 thisRequest = policyRequest->second;
		if ((thisRequest != Constants::Invalid) &&
			((lowestRequest == Constants::Invalid) || (thisRequest < lowestRequest)))
		{
			lowestRequest = thisRequest;
		}
	}
	return lowestRequest;
}

Temperature ArbitratorProcessorControl::getLowestTccOffsetRequest(std::map<UIntN, Temperature> requestedOffsets)
{
	Temperature lowestRequest = Temperature::createInvalid();
	for (auto policyRequest = requestedOffsets.begin(); policyRequest != requestedOffsets.end(); ++policyRequest)
	{
		auto thisRequest = policyRequest->second;
		if ((thisRequest.isValid()) &&
			((!lowestRequest.isValid()) || (thisRequest < lowestRequest)))
		{
			lowestRequest = thisRequest;
		}
	}
	return lowestRequest;
}
