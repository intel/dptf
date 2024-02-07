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

#include "CallbackScheduler.h"
#include "PolicyLogger.h"

using namespace std;

CallbackScheduler::CallbackScheduler(
	const PolicyServicesInterfaceContainer& policyServices,
	std::shared_ptr<ThermalRelationshipTable> trt,
	std::shared_ptr<TimeInterface> time)
	: m_sourceAvailability(policyServices, time)
	, m_trt(trt)
	, m_policyServices(policyServices)
	, m_requestSchedule(std::map<TargetSourceRelationship, TimeSpan>())
{
	m_targetScheduler.reset(new PolicyCallbackScheduler(policyServices, time));
	m_minSampleTime = policyServices.platformConfigurationData->getMinimumAllowableSamplePeriod();
}

CallbackScheduler::~CallbackScheduler()
{
}

Bool CallbackScheduler::isFreeForRequests(UIntN target, UIntN source, const TimeSpan& time) const
{
	auto relationship = m_requestSchedule.find(TargetSourceRelationship(target, source));
	if (relationship == m_requestSchedule.end())
	{
		return true;
	}
	else
	{
		auto timeBusyUntil = relationship->second;
		return (time >= timeBusyUntil);
	}
}

void CallbackScheduler::markBusyForRequests(UIntN target, UIntN source, const TimeSpan& time)
{
	auto sampleTime = m_trt->getSampleTimeForRelationship(target, source);
	auto timeBusyUntil = time + sampleTime;
	m_requestSchedule[TargetSourceRelationship(target, source)] = timeBusyUntil;
}

void CallbackScheduler::ensureCallbackByNextSamplePeriod(UIntN target, UIntN source, const TimeSpan& time)
{
	TimeSpan sampleTime = m_trt->getSampleTimeForRelationship(target, source);
	if (sampleTime.isInvalid() || (m_minSampleTime.isValid() && sampleTime.isValid() && (sampleTime < m_minSampleTime)))
	{
		sampleTime = m_minSampleTime;
		POLICY_LOG_MESSAGE_DEBUG({
			return "Sample time requested is below min for target #" + std::to_string(target)
				   + ". Setting sample time to" + m_minSampleTime.toStringSeconds() + "s.";
		});
	}

	if (m_targetScheduler->hasCallbackWithinTimeRange(target, time, time + sampleTime) == false)
	{
		m_targetScheduler->cancelCallback(target);
		m_targetScheduler->suspend(target, time, sampleTime);
	}
}

void CallbackScheduler::ensureCallbackByShortestSamplePeriod(UIntN target, const TimeSpan& time)
{
	TimeSpan sampleTime = m_trt->getShortestSamplePeriodForTarget(target);
	if (sampleTime.isInvalid() || (m_minSampleTime.isValid() && sampleTime.isValid() && (sampleTime < m_minSampleTime)))
	{
		sampleTime = m_minSampleTime;
		POLICY_LOG_MESSAGE_DEBUG({
			return "Sample time requested is below min for target #" + std::to_string(target)
				   + ". Setting sample time to" + m_minSampleTime.toStringSeconds() + "s.";
		});
	}

	if (sampleTime.isValid() && m_targetScheduler->hasCallbackWithinTimeRange(target, time, time + sampleTime) == false)
	{
		m_targetScheduler->cancelCallback(target);
		m_targetScheduler->suspend(target, time, sampleTime);
	}
}

Bool CallbackScheduler::isFreeForCommits(UIntN source, const TimeSpan& time) const
{
	return !m_sourceAvailability.isBusy(source, time);
}

void CallbackScheduler::removeParticipantFromSchedule(UIntN participant)
{
	m_sourceAvailability.remove(participant);
	m_targetScheduler->cancelCallback(participant);
}

void CallbackScheduler::markSourceAsBusy(UIntN source, const TargetMonitor& targetMonitor, const TimeSpan& time)
{
	TimeSpan minimumSamplePeriod =
		m_trt->getMinimumActiveSamplePeriodForSource(source, targetMonitor.getMonitoredTargets());
	m_sourceAvailability.setSourceAsBusy(source, time + minimumSamplePeriod);
}

void CallbackScheduler::acknowledgeCallback(UIntN target)
{
	m_targetScheduler->acknowledgeCallback(target);
}

void CallbackScheduler::cancelAllCallbackRequests()
{
	m_targetScheduler->cancelAllCallbackRequests();
}

void CallbackScheduler::setTrt(std::shared_ptr<ThermalRelationshipTable> trt)
{
	m_trt = trt;
}

void CallbackScheduler::setTimeObject(std::shared_ptr<TimeInterface> time)
{
	m_targetScheduler->setTimeObject(time);
	m_sourceAvailability.setTime(time);
}

std::shared_ptr<XmlNode> CallbackScheduler::getXml() const
{
	auto status = XmlNode::createWrapperElement("callback_scheduler");
	status->addChild(m_sourceAvailability.getXml());
	status->addChild(m_targetScheduler->getStatus());
	return status;
}

const PolicyServicesInterfaceContainer& CallbackScheduler::getPolicyServices() const
{
	return m_policyServices;
}
