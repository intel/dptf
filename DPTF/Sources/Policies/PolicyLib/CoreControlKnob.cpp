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

#include "CoreControlKnob.h"
#include <cmath>
#include "PolicyLogger.h"

using namespace std;

CoreControlKnob::CoreControlKnob(
	const PolicyServicesInterfaceContainer& policyServices,
	UIntN participantIndex,
	UIntN domainIndex,
	shared_ptr<CoreControlFacadeInterface> coreControl,
	std::shared_ptr<PerformanceControlKnob> performanceControlKnob)
	: ControlKnobBase(policyServices, participantIndex, domainIndex)
	, m_coreControl(coreControl)
	, m_performanceControlKnob(performanceControlKnob)
	, m_requests(std::map<UIntN, UIntN>())
{
}

CoreControlKnob::~CoreControlKnob(void)
{
}

void CoreControlKnob::limit(UIntN target)
{
	if (canLimit(target))
	{
		try
		{
			// TODO: pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				std::stringstream messageBefore;
				messageBefore << "Calculating request to limit active cores."
							  << " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return messageBefore.str();
			});

			Percentage stepSize = m_coreControl->getPreferences().getStepSize();
			UIntN totalCores = m_coreControl->getStaticCapabilities().getTotalLogicalProcessors();
			UIntN stepAmount = calculateStepAmount(stepSize, totalCores);
			UIntN currentActiveCores = getTargetRequest(target);
			UIntN minActiveCores = m_coreControl->getDynamicCapabilities().getMinActiveCores();
			UIntN nextActiveCores = std::max((int)currentActiveCores - (int)stepAmount, (int)minActiveCores);
			m_requests[target] = nextActiveCores;

			// TODO: pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				stringstream messageAfter;
				messageAfter << "Requesting to limit active cores to" << nextActiveCores << "."
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

void CoreControlKnob::unlimit(UIntN target)
{
	if (canUnlimit(target))
	{
		try
		{
			// TODO: pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				stringstream messageBefore;
				messageBefore << "Calculating request to unlimit active cores."
							  << " ParticipantIndex = " << getParticipantIndex() << ". Domain = " << getDomainIndex();
				return messageBefore.str();
			});

			Percentage stepSize = m_coreControl->getPreferences().getStepSize();
			UIntN totalCores = m_coreControl->getStaticCapabilities().getTotalLogicalProcessors();
			UIntN stepAmount = calculateStepAmount(stepSize, totalCores);
			UIntN currentActiveCores = getTargetRequest(target);
			UIntN maxActiveCores = m_coreControl->getDynamicCapabilities().getMaxActiveCores();
			UIntN nextActiveCores = std::min((int)currentActiveCores + (int)stepAmount, (int)maxActiveCores);
			m_requests[target] = nextActiveCores;

			// TODO: pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				stringstream messageAfter;
				messageAfter << "Requesting to unlimit active cores to" << nextActiveCores << "."
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

Bool CoreControlKnob::canLimit(UIntN target)
{
	try
	{
		if ((m_coreControl->supportsCoreControls() == false)
			|| (m_coreControl->getPreferences().isLpoEnabled() == false))
		{
			return false;
		}

		UIntN startPerformanceIndex = m_coreControl->getPreferences().getStartPState();
		UIntN currentPerformanceIndex;
		if (m_performanceControlKnob->supportsPerformanceControls())
		{
			currentPerformanceIndex = m_performanceControlKnob->getTargetRequest(target);
		}
		else
		{
			currentPerformanceIndex = startPerformanceIndex;
		}

		UIntN numActiveCoreRequest = getTargetRequest(target);
		UIntN minActiveCores = m_coreControl->getDynamicCapabilities().getMinActiveCores();
		return ((currentPerformanceIndex >= startPerformanceIndex) && (numActiveCoreRequest > minActiveCores));
	}
	catch (...)
	{
		return false;
	}
}

Bool CoreControlKnob::canUnlimit(UIntN target)
{
	try
	{
		if ((m_coreControl->supportsCoreControls() == false)
			|| (m_coreControl->getPreferences().isLpoEnabled() == false))
		{
			return false;
		}

		UIntN numActiveCoreRequest = getTargetRequest(target);
		UIntN maxActiveCores = m_coreControl->getDynamicCapabilities().getMaxActiveCores();
		return (numActiveCoreRequest < maxActiveCores);
	}
	catch (...)
	{
		return false;
	}
}

UIntN CoreControlKnob::calculateStepAmount(Percentage stepSize, UIntN totalAvailableCores)
{
	double result = stepSize * (double)totalAvailableCores;
	return (UIntN)std::ceil(result);
}

Bool CoreControlKnob::commitSetting()
{
	try
	{
		if ((m_coreControl->supportsCoreControls() == true) && (m_coreControl->getPreferences().isLpoEnabled() == true))
		{
			UIntN nextActiveCores = snapToCapabilitiesBounds(findLowestActiveCoresRequest());
			if (m_coreControl->getStatus().getNumActiveLogicalProcessors() != nextActiveCores)
			{
				// TODO: pass in participant index and domain
				POLICY_LOG_MESSAGE_DEBUG({
					stringstream messageBefore;
					messageBefore << "Attempting to change active core limit to " << nextActiveCores << "."
								  << " ParticipantIndex = " << getParticipantIndex()
								  << ". Domain = " << getDomainIndex();
					return messageBefore.str();
				});

				m_coreControl->setActiveCoreControl(CoreControlStatus(nextActiveCores));

				// TODO: pass in participant index and domain
				POLICY_LOG_MESSAGE_DEBUG({
					stringstream messageAfter;
					messageAfter << "Changed active core limit to " << nextActiveCores << "."
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

void CoreControlKnob::adjustRequestsToCapabilities()
{
	for (auto request = m_requests.begin(); request != m_requests.end(); request++)
	{
		request->second = snapToCapabilitiesBounds(request->second);
	}
}

UIntN CoreControlKnob::findLowestActiveCoresRequest() const
{
	if (m_requests.size() == 0)
	{
		return m_coreControl->getStatus().getNumActiveLogicalProcessors();
	}
	else
	{
		UIntN lowestActiveCores(Constants::Invalid);
		for (auto request = m_requests.begin(); request != m_requests.end(); request++)
		{
			if (request->second < lowestActiveCores)
			{
				lowestActiveCores = request->second;
			}
		}
		return lowestActiveCores;
	}
}

UIntN CoreControlKnob::getTargetRequest(UIntN target)
{
	auto request = m_requests.find(target);
	if (request == m_requests.end())
	{
		return m_coreControl->getDynamicCapabilities().getMaxActiveCores();
	}
	else
	{
		return request->second;
	}
}

UIntN CoreControlKnob::snapToCapabilitiesBounds(UIntN numActiveCores)
{
	UIntN maxActiveCores(m_coreControl->getDynamicCapabilities().getMaxActiveCores());
	UIntN minActiveCores(m_coreControl->getDynamicCapabilities().getMinActiveCores());
	if (numActiveCores < minActiveCores)
	{
		numActiveCores = minActiveCores;
	}
	if (numActiveCores > maxActiveCores)
	{
		numActiveCores = maxActiveCores;
	}
	return numActiveCores;
}

void CoreControlKnob::clearRequestForTarget(UIntN target)
{
	auto targetRequest = m_requests.find(target);
	if (targetRequest != m_requests.end())
	{
		m_requests.erase(targetRequest);
	}
}

void CoreControlKnob::clearAllRequests()
{
	m_requests.clear();
}

std::shared_ptr<XmlNode> CoreControlKnob::getXml()
{
	auto status = XmlNode::createWrapperElement("core_control_knob_status");
	if (m_coreControl->supportsCoreControls())
	{
		status->addChild(m_coreControl->getStaticCapabilities().getXml());
		status->addChild(m_coreControl->getStatus().getXml());
	}
	return status;
}
