/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "PowerControlKnob.h"
#include <algorithm>
#include "PolicyLogger.h"

using namespace std;

PowerControlKnob::PowerControlKnob(
	const PolicyServicesInterfaceContainer& policyServices,
	std::shared_ptr<PowerControlFacadeInterface> powerControl,
	UIntN participantIndex,
	UIntN domainIndex)
	: ControlKnobBase(policyServices, participantIndex, domainIndex)
	, m_powerControl(powerControl)
	, m_requests(std::map<UIntN, Power>())
{
}

PowerControlKnob::~PowerControlKnob(void)
{
}

void PowerControlKnob::limit(UIntN target)
{
	if (canLimit(target))
	{
		try
		{
			// TODO: want to pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				std::stringstream message;
				message << "Calculating request to limit power controls."
						<< " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return message.str();
			});

			const auto& pl1Capabilities = m_powerControl->getCapabilities().getCapability(PowerControlType::PL1);
			Power minimumPowerLimit = pl1Capabilities.getMinPowerLimit();
			Power stepSize = pl1Capabilities.getPowerStepSize();

			Power nextPowerLimit(pl1Capabilities.getMaxPowerLimit());
			Power targetRequest = getTargetRequest(target);
			if (targetRequest >= pl1Capabilities.getMaxPowerLimit())
			{
				// limit one step from current power
				Power currentPower = m_powerControl->getAveragePower();

				// TODO: want to pass in participant index and domain
				POLICY_LOG_MESSAGE_DEBUG({
					std::stringstream message;
					message << "Current power is " << currentPower.toString() << "."
							<< " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
					return message.str();
				});

				nextPowerLimit = calculateNextLowerPowerLimit(currentPower, minimumPowerLimit, stepSize, targetRequest);
			}
			else
			{
				// limit one step size down
				nextPowerLimit = std::max((int)targetRequest - (int)stepSize, (int)minimumPowerLimit);
			}
			m_requests[target] = nextPowerLimit;

			// TODO: want to pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				std::stringstream message;
				message << "Requesting to limit power to " << nextPowerLimit.toString() << "."
						<< " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return message.str();
			});
		}
		catch (std::exception& ex)
		{
			// TODO: want to pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG_EX({
				std::stringstream message;
				message << ex.what() << "."
						<< " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return message.str();
			});

			throw ex;
		}
	}
}

void PowerControlKnob::unlimit(UIntN target)
{
	if (canUnlimit(target))
	{
		try
		{
			// TODO: want to pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				std::stringstream message;
				message << "Calculating request to unlimit power controls."
						<< " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return message.str();
			});

			const auto& pl1Capabilities = m_powerControl->getCapabilities().getCapability(PowerControlType::PL1);

			Power lastRequest = getTargetRequest(target);
			Power nextPowerAfterStep = lastRequest + pl1Capabilities.getPowerStepSize();
			Power nextPowerLimit(std::min(nextPowerAfterStep, pl1Capabilities.getMaxPowerLimit()));
			m_requests[target] = nextPowerLimit;

			// TODO: want to pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				std::stringstream message;
				message << "Requesting to unlimit power to " << nextPowerLimit.toString() << "."
						<< " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return message.str();
			});
		}
		catch (std::exception& ex)
		{
			// TODO: want to pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG_EX({
				std::stringstream message;
				message << ex.what() << "."
						<< " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return message.str();
			});

			throw ex;
		}
	}
}

Bool PowerControlKnob::canLimit(UIntN target)
{
	try
	{
		if (m_powerControl->supportsPowerControls())
		{
			const PowerControlDynamicCaps& pl1Capabilities =
				m_powerControl->getCapabilities().getCapability(PowerControlType::PL1);
			Power currentStatus = getTargetRequest(target);
			return (currentStatus > pl1Capabilities.getMinPowerLimit());
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

Bool PowerControlKnob::canUnlimit(UIntN target)
{
	try
	{
		if (m_powerControl->supportsPowerControls())
		{
			const PowerControlDynamicCaps& pl1Capabilities =
				m_powerControl->getCapabilities().getCapability(PowerControlType::PL1);
			Power currentStatus = getTargetRequest(target);
			return (currentStatus < pl1Capabilities.getMaxPowerLimit());
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

Power PowerControlKnob::calculateNextLowerPowerLimit(
	Power currentPower,
	Power minimumPowerLimit,
	Power stepSize,
	Power currentPowerLimit)
{
	Power nextPowerLimit(Power::createInvalid());
	if (currentPower > minimumPowerLimit)
	{
		if (stepSize > currentPower)
		{
			nextPowerLimit = minimumPowerLimit;
		}
		else
		{
			Power leastPowerValue = std::min(currentPowerLimit - stepSize, currentPower - stepSize);
			nextPowerLimit = std::max(minimumPowerLimit, leastPowerValue);
		}
	}
	else
	{
		nextPowerLimit = minimumPowerLimit;
	}
	return nextPowerLimit;
}

Bool PowerControlKnob::commitSetting()
{
	try
	{
		if (m_powerControl->supportsPowerControls())
		{
			// find lowest power limit in request
			Power lowestPowerLimit = snapToCapabilitiesBounds(findLowestPowerLimitRequest());

			// set new power status
			Power currentPowerLimit = m_powerControl->getPowerLimitPL1();
			if (currentPowerLimit != lowestPowerLimit)
			{
				// TODO: want to pass in participant index and domain
				POLICY_LOG_MESSAGE_DEBUG({
					stringstream messageBefore;
					messageBefore << "Attempting to change power limit to " << lowestPowerLimit.toString() << "."
								  << " ParticipantIndex = " << getParticipantIndex()
								  << ". Domain = " << getDomainIndex();
					return messageBefore.str();
				});

				m_powerControl->setPowerLimitPL1(lowestPowerLimit);

				// TODO: want to pass in participant index and domain
				POLICY_LOG_MESSAGE_DEBUG({
					stringstream messageAfter;
					messageAfter << "Changed power limit to " << lowestPowerLimit.toString() << "."
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
		// TODO: want to pass in participant index and domain
		POLICY_LOG_MESSAGE_DEBUG_EX({
			stringstream message;
			message << ex.what() << "."
					<< " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
			return message.str();
		});

		throw ex;
	}
}

void PowerControlKnob::adjustRequestsToCapabilities()
{
	for (auto request = m_requests.begin(); request != m_requests.end(); request++)
	{
		request->second = snapToCapabilitiesBounds(request->second);
	}
}

Power PowerControlKnob::findLowestPowerLimitRequest()
{
	if (m_requests.size() == 0)
	{
		const auto& pl1Capabilities = m_powerControl->getCapabilities().getCapability(PowerControlType::PL1);
		return pl1Capabilities.getMaxPowerLimit();
	}
	else
	{
		Power lowestPower;
		for (auto request = m_requests.begin(); request != m_requests.end(); request++)
		{
			if (lowestPower.isValid() == false)
			{
				lowestPower = request->second;
			}
			else
			{
				if (request->second < lowestPower)
				{
					lowestPower = request->second;
				}
			}
		}
		return lowestPower;
	}
}

Power PowerControlKnob::getTargetRequest(UIntN target)
{
	const auto& pl1Capabilities = m_powerControl->getCapabilities().getCapability(PowerControlType::PL1);
	auto targetRequest = m_requests.find(target);
	if (targetRequest == m_requests.end())
	{
		return pl1Capabilities.getMaxPowerLimit();
	}
	else
	{
		return targetRequest->second;
	}
}

UIntN PowerControlKnob::snapToCapabilitiesBounds(Power powerLimit)
{
	const auto& pl1Capabilities = m_powerControl->getCapabilities().getCapability(PowerControlType::PL1);
	Power maxPowerLimit = pl1Capabilities.getMaxPowerLimit();
	Power minPowerLimit = pl1Capabilities.getMinPowerLimit();
	if (powerLimit > maxPowerLimit)
	{
		powerLimit = maxPowerLimit;
	}
	if (powerLimit < minPowerLimit)
	{
		powerLimit = minPowerLimit;
	}
	return powerLimit;
}

void PowerControlKnob::clearRequestForTarget(UIntN target)
{
	auto targetRequest = m_requests.find(target);
	if (targetRequest != m_requests.end())
	{
		m_requests.erase(targetRequest);
	}
}

void PowerControlKnob::clearAllRequests()
{
	m_requests.clear();
}

std::shared_ptr<XmlNode> PowerControlKnob::getXml() const
{
	auto knobStatus = XmlNode::createWrapperElement("power_control_status");
	if (m_powerControl->supportsPowerControls())
	{
		auto pl1Capabilities = m_powerControl->getCapabilities().getCapability(PowerControlType::PL1);
		knobStatus->addChild(pl1Capabilities.getXml());
		Power currentPowerLimit = m_powerControl->getPowerLimitPL1();
		auto powerControl = XmlNode::createDataElement("power_limit", currentPowerLimit.toString());
		knobStatus->addChild(powerControl);
	}
	return knobStatus;
}
