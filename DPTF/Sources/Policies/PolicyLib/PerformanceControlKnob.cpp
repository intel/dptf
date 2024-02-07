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

#include "PerformanceControlKnob.h"
#include "PolicyLogger.h"

using namespace std;

PerformanceControlKnob::PerformanceControlKnob(
	const PolicyServicesInterfaceContainer& policyServices,
	UIntN participantIndex,
	UIntN domainIndex,
	shared_ptr<PerformanceControlFacadeInterface> performanceControl,
	std::shared_ptr<std::map<UIntN, UIntN>> perfControlRequests,
	PerformanceControlType::Type controlType)
	: ControlKnobBase(policyServices, participantIndex, domainIndex)
	, m_performanceControl(performanceControl)
	, m_controlType(controlType)
	, m_requests(perfControlRequests)
	, m_tstateUtilizationThreshold(0.0)
{
}

PerformanceControlKnob::~PerformanceControlKnob(void)
{
}

void PerformanceControlKnob::limit(UIntN target)
{
	if (canLimit(target))
	{
		try
		{
			// TODO: pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				stringstream messageBefore;
				messageBefore << "Calculating request to limit " << controlTypeToString(m_controlType) << " controls."
							  << " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return messageBefore.str();
			});

			const PerformanceControlDynamicCaps& dynamicCapabilities = m_performanceControl->getDynamicCapabilities();
			UIntN lowerLimitIndex = dynamicCapabilities.getCurrentLowerLimitIndex();
			UIntN upperLimitIndex = dynamicCapabilities.getCurrentUpperLimitIndex();
			UIntN currentIndex = std::max(getTargetRequest(target), upperLimitIndex);
			UIntN nextIndex = std::min(currentIndex + 1, lowerLimitIndex);
			(*m_requests)[target] = nextIndex;

			// TODO: pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				stringstream messageAfter;
				messageAfter << "Requesting to limit " << controlTypeToString(m_controlType) << " controls to"
							 << nextIndex << "."
							 << " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return messageAfter.str();
			});
		}
		catch (std::exception& ex)
		{
			// TODO: pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG_EX({
				stringstream message;
				message << ex.what() << "."
						<< " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return message.str();
			});
			throw ex;
		}
	}
}

void PerformanceControlKnob::unlimit(UIntN target)
{
	if (canUnlimit(target))
	{
		try
		{
			// TODO: pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				stringstream messageBefore;
				messageBefore << "Calculating request to unlimit " << controlTypeToString(m_controlType) << " controls."
							  << " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return messageBefore.str();
			});

			const PerformanceControlDynamicCaps& dynamicCapabilities = m_performanceControl->getDynamicCapabilities();
			UIntN lowerLimitIndex = dynamicCapabilities.getCurrentLowerLimitIndex();
			UIntN upperLimitIndex = dynamicCapabilities.getCurrentUpperLimitIndex();
			UIntN currentIndex = std::min(getTargetRequest(target), lowerLimitIndex);
			UIntN nextIndex = std::max(currentIndex - 1, upperLimitIndex);
			(*m_requests)[target] = nextIndex;

			// TODO: pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				stringstream messageAfter;
				messageAfter << "Requesting to unlimit " << controlTypeToString(m_controlType) << " controls to"
							 << nextIndex << "."
							 << " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return messageAfter.str();
			});
		}
		catch (std::exception& ex)
		{
			// TODO: pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG_EX({
				stringstream message;
				message << ex.what() << "."
						<< " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return message.str();
			});
			throw ex;
		}
	}
}

Bool PerformanceControlKnob::canLimit(UIntN target)
{
	try
	{
		if (m_performanceControl->supportsPerformanceControls())
		{
			const PerformanceControlDynamicCaps& dynamicCapabilities = m_performanceControl->getDynamicCapabilities();
			UIntN lowerLimitIndex = dynamicCapabilities.getCurrentLowerLimitIndex();
			UIntN upperLimitIndex = dynamicCapabilities.getCurrentUpperLimitIndex();
			UIntN currentIndex = std::max(getTargetRequest(target), upperLimitIndex);
			UIntN nextIndex = currentIndex + 1;
			if (currentIndex >= lowerLimitIndex)
			{
				return false;
			}

			PerformanceControlType::Type currControlType =
				m_performanceControl->getControls()[currentIndex].getPerformanceControlType();
			PerformanceControlType::Type nextControlType =
				m_performanceControl->getControls()[nextIndex].getPerformanceControlType();

			// pstate to pstate
			if ((currControlType == PerformanceControlType::PerformanceState)
				&& (nextControlType == PerformanceControlType::PerformanceState))
			{
				if (m_controlType == PerformanceControlType::PerformanceState)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			// pstate to tstate
			else if (
				(currControlType == PerformanceControlType::PerformanceState)
				&& (nextControlType == PerformanceControlType::ThrottleState))
			{
				if (m_controlType == PerformanceControlType::PerformanceState)
				{
					return false;
				}
				else
				{
					return checkUtilizationIsLessThanThreshold();
				}
			}
			// t-state to t-state
			else if (
				(currControlType == PerformanceControlType::ThrottleState)
				&& (nextControlType == PerformanceControlType::ThrottleState))
			{
				if (m_controlType == PerformanceControlType::ThrottleState)
				{
					if (nextIndex > currentIndex)
					{
						return checkUtilizationIsLessThanThreshold();
					}
					else
					{
						return true;
					}
				}
				else
				{
					return false;
				}
			}
			// tstate to pstate
			else if (
				(currControlType == PerformanceControlType::ThrottleState)
				&& (nextControlType == PerformanceControlType::PerformanceState))
			{
				if (m_controlType == PerformanceControlType::ThrottleState)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		return false;
	}
}

Bool PerformanceControlKnob::canUnlimit(UIntN target)
{
	try
	{
		if (m_performanceControl->supportsPerformanceControls())
		{
			const PerformanceControlDynamicCaps& dynamicCapabilities = m_performanceControl->getDynamicCapabilities();
			UIntN lowerLimitIndex = dynamicCapabilities.getCurrentLowerLimitIndex();
			UIntN upperLimitIndex = dynamicCapabilities.getCurrentUpperLimitIndex();
			UIntN currentIndex = std::min(getTargetRequest(target), lowerLimitIndex);
			if (currentIndex <= upperLimitIndex)
			{
				return false;
			}

			PerformanceControlType::Type nextControlType =
				m_performanceControl->getControls()[currentIndex - 1].getPerformanceControlType();
			return (nextControlType == m_controlType);
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		return false;
	}
}

string PerformanceControlKnob::controlTypeToString(PerformanceControlType::Type controlType)
{
	switch (controlType)
	{
	case PerformanceControlType::PerformanceState:
		return "P-State";
	case PerformanceControlType::ThrottleState:
		return "T-State";
	default:
		return "Unknown-Type";
	}
}

Bool PerformanceControlKnob::commitSetting()
{
	try
	{
		if (m_performanceControl->supportsPerformanceControls())
		{
			UIntN currentIndex = m_performanceControl->getStatus().getCurrentControlSetIndex();
			UIntN nextIndex = snapToCapabilitiesBounds(findHighestPerformanceIndexRequest());
			if (currentIndex != nextIndex)
			{
				// TODO: pass in participant index and domain
				POLICY_LOG_MESSAGE_DEBUG({
					stringstream messageBefore;
					messageBefore << "Attempting to change " << controlTypeToString(m_controlType) << " limit to "
								  << nextIndex << "."
								  << " ParticipantIndex = " << getParticipantIndex()
								  << ". Domain = " << getDomainIndex();
					return messageBefore.str();
				});

				m_performanceControl->setControl(nextIndex);

				// TODO: pass in participant index and domain
				POLICY_LOG_MESSAGE_DEBUG({
					stringstream messageAfter;
					messageAfter << "Changed " << controlTypeToString(m_controlType) << " limit to " << nextIndex << "."
								 << " ParticipantIndex = " << getParticipantIndex()
								 << ". Domain = " << getDomainIndex();
					return messageAfter.str();
				});

				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	catch (std::exception& ex)
	{
		// TODO: pass in participant index and domain
		POLICY_LOG_MESSAGE_DEBUG_EX({
			stringstream message;
			message << ex.what() << "."
					<< " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
			return message.str();
		});

		throw ex;
	}
}

void PerformanceControlKnob::adjustRequestsToCapabilities()
{
	for (auto request = m_requests->begin(); request != m_requests->end(); request++)
	{
		request->second = snapToCapabilitiesBounds(request->second);
	}
}

void PerformanceControlKnob::setTstateUtilizationThreshold(UtilizationStatus tstateUtilizationThreshold)
{
	m_tstateUtilizationThreshold = tstateUtilizationThreshold;
}

Bool PerformanceControlKnob::supportsPerformanceControls() const
{
	return m_performanceControl->supportsPerformanceControls();
}

UIntN PerformanceControlKnob::findHighestPerformanceIndexRequest() const
{
	if (m_requests->size() == 0)
	{
		return m_performanceControl->getDynamicCapabilities().getCurrentUpperLimitIndex();
	}
	else
	{
		UIntN highestIndex(0);
		for (auto request = m_requests->begin(); request != m_requests->end(); request++)
		{
			if (request->second > highestIndex)
			{
				highestIndex = request->second;
			}
		}
		return highestIndex;
	}
}

Bool PerformanceControlKnob::checkUtilizationIsLessThanThreshold() const
{
	UtilizationStatus currentUtilization(1.0);
	try
	{
		currentUtilization =
			getPolicyServices().domainUtilization->getUtilizationStatus(getParticipantIndex(), getDomainIndex());
	}
	catch (...)
	{
		currentUtilization = UtilizationStatus(1.0);
	}

	if (currentUtilization.getCurrentUtilization() < m_tstateUtilizationThreshold.getCurrentUtilization())
	{
		// TODO: pass in participant index and domain
		POLICY_LOG_MESSAGE_DEBUG({
			stringstream message;
			message << "Cannot set T-state because utilization ("
					<< currentUtilization.getCurrentUtilization().toString() << ") is less than the threshold ("
					<< m_tstateUtilizationThreshold.getCurrentUtilization().toString() << ")."
					<< " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
			return message.str();
		});

		return false;
	}
	else
	{
		return true;
	}
}

UIntN PerformanceControlKnob::getTargetRequest(UIntN target) const
{
	auto targetRequest = m_requests->find(target);
	if (targetRequest != m_requests->end())
	{
		return targetRequest->second;
	}
	else
	{
		const PerformanceControlDynamicCaps& dynamicCapabilities = m_performanceControl->getDynamicCapabilities();
		return dynamicCapabilities.getCurrentUpperLimitIndex();
	}
}

UIntN PerformanceControlKnob::snapToCapabilitiesBounds(UIntN controlIndex)
{
	const PerformanceControlDynamicCaps& dynamicCapabilities = m_performanceControl->getDynamicCapabilities();
	UIntN lowerLimitIndex = dynamicCapabilities.getCurrentLowerLimitIndex();
	UIntN upperLimitIndex = dynamicCapabilities.getCurrentUpperLimitIndex();
	if (controlIndex > lowerLimitIndex)
	{
		controlIndex = lowerLimitIndex;
	}
	if (controlIndex < upperLimitIndex)
	{
		controlIndex = upperLimitIndex;
	}
	return controlIndex;
}

void PerformanceControlKnob::clearRequestForTarget(UIntN target)
{
	auto targetRequest = m_requests->find(target);
	if (targetRequest != m_requests->end())
	{
		m_requests->erase(targetRequest);
	}
}

void PerformanceControlKnob::clearAllRequests()
{
	m_requests->clear();
}

std::shared_ptr<XmlNode> PerformanceControlKnob::getXml()
{
	auto status = XmlNode::createWrapperElement("performance_control_status");
	if (m_performanceControl->supportsPerformanceControls())
	{
		status->addChild(XmlNode::createDataElement("type", controlTypeToString(m_controlType)));
		auto dynamicCapabilities = m_performanceControl->getDynamicCapabilities();
		status->addChild(dynamicCapabilities.getXml());
		status->addChild(m_performanceControl->getStatus().getXml());
	}
	return status;
}
