/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

using namespace std;

CallbackScheduler::CallbackScheduler(
    const PolicyServicesInterfaceContainer& policyServices,
    const ThermalRelationshipTable& trt,
    TargetMonitor* targetMonitor, 
    std::shared_ptr<TimeInterface> time)
    : m_sourceAvailability(policyServices, time), m_trt(trt), m_targetMonitor(targetMonitor)
{
    m_targetScheduler.reset(new PolicyCallbackScheduler(policyServices, time));
}

CallbackScheduler::~CallbackScheduler()
{
}

Bool CallbackScheduler::isFreeForRequests(UIntN target, UIntN source, UInt64 time) const
{
    auto relationship = m_requestSchedule.find(TargetSourceRelationship(target, source));
    if (relationship == m_requestSchedule.end())
    {
        return true;
    }
    else
    {
        UInt64 timeBusyUntil = relationship->second;
        return (time >= timeBusyUntil);
    }
}

void CallbackScheduler::markBusyForRequests(UIntN target, UIntN source, UInt64 time)
{
    TimeSpan sampleTime = m_trt.getSampleTimeForRelationship(target, source);
    UInt64 timeBusyUntil = time + sampleTime.asMillisecondsInt();
    m_requestSchedule[TargetSourceRelationship(target, source)] = timeBusyUntil;
}

void CallbackScheduler::ensureCallbackByNextSamplePeriod(UIntN target, UIntN source, UInt64 time)
{
    TimeSpan sampleTime = m_trt.getSampleTimeForRelationship(target, source);
    if (m_targetScheduler->hasCallbackWithinTimeRange(target, time, time + sampleTime.asMillisecondsInt()) == false)
    {
        m_targetScheduler->cancelCallback(target);
        m_targetScheduler->suspend(target, time, sampleTime);
    }
}

void CallbackScheduler::ensureCallbackByShortestSamplePeriod(UIntN target, UInt64 time)
{
    TimeSpan sampleTime = m_trt.getShortestSamplePeriodForTarget(target);
    if (m_targetScheduler->hasCallbackWithinTimeRange(target, time, time + sampleTime.asMillisecondsInt()) == false)
    {
        m_targetScheduler->cancelCallback(target);
        m_targetScheduler->suspend(target, time, sampleTime);
    }
}

Bool CallbackScheduler::isFreeForCommits(UIntN source, UInt64 time) const
{
    return !m_sourceAvailability.isBusy(source, time);
}

void CallbackScheduler::removeParticipantFromSchedule(UIntN participant)
{
    m_sourceAvailability.remove(participant);
    m_targetScheduler->cancelCallback(participant);
}

void CallbackScheduler::markSourceAsBusy(UIntN source, const TargetMonitor& targetMonitor, UInt64 time)
{
    TimeSpan minimumSamplePeriod = m_trt.getMinimumActiveSamplePeriodForSource(
        source, targetMonitor.getMonitoredTargets());
    m_sourceAvailability.setSourceAsBusy(source, time + minimumSamplePeriod.asMillisecondsInt());
}

void CallbackScheduler::acknowledgeCallback(UIntN target)
{
    m_targetScheduler->acknowledgeCallback(target);
}

void CallbackScheduler::setTrt(const ThermalRelationshipTable& trt)
{
    m_trt = trt;
}

void CallbackScheduler::setTimeObject(std::shared_ptr<TimeInterface> time)
{
    m_targetScheduler->setTimeObject(time);
    m_sourceAvailability.setTime(time);
}

XmlNode* CallbackScheduler::getXml() const
{
    XmlNode* status = XmlNode::createWrapperElement("callback_scheduler");
    status->addChild(m_sourceAvailability.getXml());
    status->addChild(m_targetScheduler->getStatus());
    return status;
}