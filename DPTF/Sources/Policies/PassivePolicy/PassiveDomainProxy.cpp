/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "PassiveDomainProxy.h"
#include "StatusFormat.h"
#include "PolicyLogger.h"

using namespace std;

PassiveDomainProxy::PassiveDomainProxy(
	UIntN domainIndex,
	ParticipantProxyInterface* participant,
	const PolicyServicesInterfaceContainer& policyServices)
	: DomainProxy(domainIndex, participant, policyServices)
{
	auto participantIndex = participant->getIndex();
	m_perfControlRequests = std::make_shared<std::map<UIntN, UIntN>>();
	m_pstateControlKnob = std::make_shared<PerformanceControlKnob>(PerformanceControlKnob(
		policyServices,
		participantIndex,
		domainIndex,
		m_performanceControl,
		m_perfControlRequests,
		PerformanceControlType::PerformanceState));
	m_tstateControlKnob = std::make_shared<PerformanceControlKnob>(PerformanceControlKnob(
		policyServices,
		participantIndex,
		domainIndex,
		m_performanceControl,
		m_perfControlRequests,
		PerformanceControlType::ThrottleState));
	m_powerControlKnob =
		std::make_shared<PowerControlKnob>(policyServices, m_powerControl, participantIndex, domainIndex);
	m_displayControlKnob =
		std::make_shared<DisplayControlKnob>(policyServices, m_displayControl, participantIndex, domainIndex);
	m_coreControlKnob = std::make_shared<CoreControlKnob>(
		policyServices, participantIndex, domainIndex, m_coreControl, m_pstateControlKnob);
}

PassiveDomainProxy::PassiveDomainProxy()
	: DomainProxy()
	, m_perfControlRequests(std::make_shared<std::map<UIntN, UIntN>>())
{
}

PassiveDomainProxy::~PassiveDomainProxy()
{
}

void PassiveDomainProxy::requestLimit(UIntN target)
{
	// attempt to limit each control in turn.  if any is not supported or has a problem, it will return "true" and the
	// execution will continue to the next limit attempt.  these lines of code take advantage of the 'short circuit'
	// rule in C++ in that the next limit will not be called if the previous one returns false.
	requestLimitPowerAndShouldContinue(target) && requestLimitPstatesWithCoresAndShouldContinue(target)
		&& requestLimitCoresAndShouldContinue(target) && requestLimitTstatesAndContinue(target)
		&& requestLimitDisplayAndContinue(target);
}

void PassiveDomainProxy::requestUnlimit(UIntN target)
{
	// attempt to unlimit each control in turn.  if any is not supported or has a problem, it will return "true" and the
	// execution will continue to the next unlimit attempt.  these lines of code take advantage of the 'short circuit'
	// rule in C++ in that the next unlimit will not be called if the previous one returns false.
	requestUnlimitDisplayAndContinue(target) && requestUnlimitTstatesAndContinue(target)
		&& requestUnlimitCoresWithPstatesAndShouldContinue(target) && requestUnlimitPstatesAndShouldContinue(target)
		&& requestUnlimitPowerAndShouldContinue(target);
}

Bool PassiveDomainProxy::requestLimitPowerAndShouldContinue(UIntN target)
{
	try
	{
		if (m_powerControlKnob->canLimit(target))
		{
			m_powerControlKnob->limit(target);
			return false;
		}
	}
	catch (...)
	{
		// we want to continue on if this control fails for whatever reason
	}
	return true;
}

Bool PassiveDomainProxy::requestLimitPstatesWithCoresAndShouldContinue(UIntN target)
{
	try
	{
		if (m_pstateControlKnob->canLimit(target))
		{
			m_pstateControlKnob->limit(target);

			try
			{
				if (m_coreControlKnob->canLimit(target))
				{
					m_coreControlKnob->limit(target);
				}
			}
			catch (...)
			{
				// don't throw out of function if limiting cores fails
			}

			return false;
		}
	}
	catch (...)
	{
		// we want to continue on if this control fails for whatever reason
	}
	return true;
}

Bool PassiveDomainProxy::requestLimitCoresAndShouldContinue(UIntN target)
{
	try
	{
		if (m_coreControlKnob->canLimit(target))
		{
			m_coreControlKnob->limit(target);
			return false;
		}
	}
	catch (...)
	{
		// we want to continue on if this control fails for whatever reason
	}
	return true;
}

Bool PassiveDomainProxy::requestLimitTstatesAndContinue(UIntN target)
{
	try
	{
		if (m_tstateControlKnob->canLimit(target))
		{
			m_tstateControlKnob->limit(target);
			return false;
		}
	}
	catch (...)
	{
		// we want to continue on if this control fails for whatever reason
	}
	return true;
}

Bool PassiveDomainProxy::requestLimitDisplayAndContinue(UIntN target)
{
	try
	{
		if (m_displayControlKnob->canLimit(target))
		{
			m_displayControlKnob->limit(target);
			return false;
		}
	}
	catch (...)
	{
		// we want to continue on if this control fails for whatever reason
	}
	return true;
}

Bool PassiveDomainProxy::requestUnlimitDisplayAndContinue(UIntN target)
{
	try
	{
		if (m_displayControlKnob->canUnlimit(target))
		{
			m_displayControlKnob->unlimit(target);
			return false;
		}
	}
	catch (...)
	{
		// we want to continue on if this control fails for whatever reason
	}
	return true;
}

Bool PassiveDomainProxy::requestUnlimitTstatesAndContinue(UIntN target)
{
	try
	{
		if (m_tstateControlKnob->canUnlimit(target))
		{
			m_tstateControlKnob->unlimit(target);
			return false;
		}
	}
	catch (...)
	{
		// we want to continue on if this control fails for whatever reason
	}
	return true;
}

Bool PassiveDomainProxy::requestUnlimitCoresWithPstatesAndShouldContinue(UIntN target)
{
	try
	{
		if (m_coreControlKnob->canUnlimit(target))
		{
			m_coreControlKnob->unlimit(target);

			try
			{
				if (m_pstateControlKnob->canUnlimit(target))
				{
					m_pstateControlKnob->unlimit(target);
				}
			}
			catch (...)
			{
				// we want to catch errors here
			}
			return false;
		}
	}
	catch (...)
	{
		// we want to continue on if this control fails for whatever reason
	}
	return true;
}

Bool PassiveDomainProxy::requestUnlimitPstatesAndShouldContinue(UIntN target)
{
	try
	{
		if (m_pstateControlKnob->canUnlimit(target))
		{
			m_pstateControlKnob->unlimit(target);
			return false;
		}
	}
	catch (...)
	{
		// we want to continue on if this control fails for whatever reason
	}
	return true;
}

Bool PassiveDomainProxy::requestUnlimitPowerAndShouldContinue(UIntN target)
{
	try
	{
		if (m_powerControlKnob->canUnlimit(target))
		{
			m_powerControlKnob->unlimit(target);
			return false;
		}
	}
	catch (...)
	{
		// we want to continue on if this control fails for whatever reason
	}
	return true;
}

Bool PassiveDomainProxy::commitLimits()
{
	Bool committedPower = m_powerControlKnob->commitSetting();
	Bool committedPstate = m_pstateControlKnob->commitSetting();
	Bool committedCore = m_coreControlKnob->commitSetting();
	Bool committedTstate = m_tstateControlKnob->commitSetting();
	Bool committedDisplay = m_displayControlKnob->commitSetting();
	return committedPower || committedPstate || committedCore || committedTstate || committedDisplay;
}

void PassiveDomainProxy::setArbitratedPowerLimit()
{
	m_powerControlKnob->commitSetting();
}

const PolicyServicesInterfaceContainer& PassiveDomainProxy::getPolicyServices() const
{
	return m_policyServices;
}

void PassiveDomainProxy::setArbitratedPerformanceLimit()
{
	// TODO: fix so this code is in the performance control knob, and likely there should be a single performance
	// control knob that handles both P-State and T-States.

	// get highest perf state request
	UIntN arbitratedRequest = 0;
	for (auto request = m_perfControlRequests->begin(); request != m_perfControlRequests->end(); request++)
	{
		if (request->second > arbitratedRequest)
		{
			arbitratedRequest = request->second;
		}
	}

	// adjust for capabilities
	UIntN upperLimit = m_performanceControl->getDynamicCapabilities().getCurrentUpperLimitIndex();
	UIntN lowerLimit = m_performanceControl->getDynamicCapabilities().getCurrentLowerLimitIndex();
	arbitratedRequest = std::max(arbitratedRequest, upperLimit);
	arbitratedRequest = std::min(arbitratedRequest, lowerLimit);

	// set the value
	if (arbitratedRequest != m_performanceControl->getStatus().getCurrentControlSetIndex())
	{
		POLICY_LOG_MESSAGE_DEBUG({
			// TODO: might want to pass in participant index instead
			stringstream messageBefore;
			messageBefore << "Attempting to change performance limit to " << arbitratedRequest << "."
						  << ". ParticipantIndex = " << getParticipantIndex() << ". DomainIndex=" << getDomainIndex();
			return messageBefore.str();
		});

		m_performanceControl->setControl(arbitratedRequest);

		POLICY_LOG_MESSAGE_DEBUG({
			// TODO: might want to pass in participant index instead
			stringstream messageAfter;
			messageAfter << "Changed performance limit to " << arbitratedRequest << "."
						 << ". ParticipantIndex = " << getParticipantIndex() << ". DomainIndex=" << getDomainIndex();
			return messageAfter.str();
		});
	}
}

void PassiveDomainProxy::setArbitratedCoreLimit()
{
	m_coreControlKnob->commitSetting();
}

void PassiveDomainProxy::adjustPowerRequests()
{
	m_powerControlKnob->adjustRequestsToCapabilities();
}

void PassiveDomainProxy::adjustPerformanceRequests()
{
	// TODO: fix so this code is in the performance control knob, and likely there should be a single performance
	// control knob that handles both P-State and T-States.

	// adjust for capabilities
	UIntN upperLimit = m_performanceControl->getDynamicCapabilities().getCurrentUpperLimitIndex();
	UIntN lowerLimit = m_performanceControl->getDynamicCapabilities().getCurrentLowerLimitIndex();
	for (auto request = m_perfControlRequests->begin(); request != m_perfControlRequests->end(); request++)
	{
		request->second = std::max(request->second, upperLimit);
		request->second = std::min(request->second, lowerLimit);
	}
}

void PassiveDomainProxy::adjustCoreRequests()
{
	m_coreControlKnob->adjustRequestsToCapabilities();
}

Bool PassiveDomainProxy::canLimit(UIntN target)
{
	return m_powerControlKnob->canLimit(target) || m_pstateControlKnob->canLimit(target)
		   || m_coreControlKnob->canLimit(target) || m_tstateControlKnob->canLimit(target)
		   || m_displayControlKnob->canLimit(target);
}

Bool PassiveDomainProxy::canUnlimit(UIntN target)
{
	return m_displayControlKnob->canUnlimit(target) || m_tstateControlKnob->canUnlimit(target)
		   || m_coreControlKnob->canUnlimit(target) || m_pstateControlKnob->canUnlimit(target)
		   || m_powerControlKnob->canUnlimit(target);
}

void PassiveDomainProxy::setTstateUtilizationThreshold(UtilizationStatus tstateUtilizationThreshold)
{
	m_tstateControlKnob->setTstateUtilizationThreshold(tstateUtilizationThreshold);
}

void PassiveDomainProxy::clearAllRequestsForTarget(UIntN target)
{
	m_powerControlKnob->clearRequestForTarget(target);
	m_pstateControlKnob->clearRequestForTarget(target);
	m_coreControlKnob->clearRequestForTarget(target);
	m_tstateControlKnob->clearRequestForTarget(target);
	m_displayControlKnob->clearRequestForTarget(target);
}

void PassiveDomainProxy::clearAllPerformanceControlRequests()
{
	m_perfControlRequests->clear();
}

void PassiveDomainProxy::clearAllPowerControlRequests()
{
	m_powerControlKnob->clearAllRequests();
}

void PassiveDomainProxy::clearAllCoreControlRequests()
{
	m_coreControlKnob->clearAllRequests();
}

void PassiveDomainProxy::clearAllDisplayControlRequests()
{
	m_displayControlKnob->clearAllRequests();
}

void PassiveDomainProxy::clearAllControlKnobRequests()
{
	clearAllPerformanceControlRequests();
	clearAllPowerControlRequests();
	clearAllCoreControlRequests();
	clearAllDisplayControlRequests();
}

std::shared_ptr<XmlNode> PassiveDomainProxy::getXmlForPassiveControlKnobs()
{
	auto domainStatus = XmlNode::createWrapperElement("domain_passive_control_knobs");
	domainStatus->addChild(m_domainProperties.getXml());
	domainStatus->addChild(m_participantProperties.getXml());
	domainStatus->addChild(m_powerControlKnob->getXml());
	domainStatus->addChild(m_pstateControlKnob->getXml());
	domainStatus->addChild(m_coreControlKnob->getXml());
	domainStatus->addChild(m_tstateControlKnob->getXml());
	domainStatus->addChild(m_displayControlKnob->getXml());
	return domainStatus;
}
