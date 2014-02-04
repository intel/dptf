/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

UInt64 tensOfSecondsToMilliseconds(UInt64 tensOfSeconds)
{
    return tensOfSeconds * 100;
}

CallbackScheduler::CallbackScheduler(
    const PolicyServicesInterfaceContainer& policyServices,
    const ThermalRelationshipTable& trt,
    std::shared_ptr<TimeInterface> time)
    : m_policyServices(policyServices), m_time(time), m_trt(trt),
    m_sourceAvailability(policyServices, time),
    m_targetScheduler(policyServices, time)
{
}

CallbackScheduler::~CallbackScheduler()
{
}

void CallbackScheduler::setTrt(const ThermalRelationshipTable& trt)
{
    m_trt = trt;
}

void CallbackScheduler::setTimeObject(std::shared_ptr<TimeInterface> time)
{
    m_time = time;
    m_sourceAvailability.setTime(time);
    m_targetScheduler.setTimeObject(time);
}

void CallbackScheduler::removeParticipantFromSchedule(UIntN participant)
{
    m_sourceAvailability.remove(participant);
    m_targetScheduler.cancelCallback(participant);
}

Bool CallbackScheduler::isSourceBusyNow(UIntN sourceIndex)
{
    return m_sourceAvailability.isBusyNow(sourceIndex);
}

void CallbackScheduler::scheduleCallbackAsSoonAsPossible(UIntN target, UIntN source)
{
    UInt64 timeNow = m_time->getCurrentTimeInMilliseconds();
    UInt64 timeDelta = m_sourceAvailability.getNextAvailableTime(source, timeNow);
    if (m_targetScheduler.hasCallbackScheduledAtOrBeforeTime(target, timeDelta) == false)
    {
        m_targetScheduler.cancelCallback(target);
        m_targetScheduler.scheduleCallback(target, timeDelta);
        m_sourceAvailability.setSourceAsBusy(source, timeNow + timeDelta);
    }

    m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
        "Scheduled a callback in " + std::to_string(timeDelta) + " ms" +
        " for target " + std::to_string(target) + " source " + std::to_string(source) + ".", target));
}

void CallbackScheduler::scheduleCallbackAfterShortestSamplePeriod(UIntN target)
{
    UInt64 timeDelta = tensOfSecondsToMilliseconds(m_trt.getShortestSamplePeriodForTarget(target));
    if (m_targetScheduler.hasCallbackScheduledAtOrBeforeTime(target, timeDelta) == false)
    {
        m_targetScheduler.cancelCallback(target);
        m_targetScheduler.scheduleCallback(target, timeDelta);
    }

    m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
        "Scheduled a callback in " + std::to_string(timeDelta) + " ms" +
        " for target " + std::to_string(target) + ".", target));
}

void CallbackScheduler::scheduleCallbackAfterNextSamplingPeriod(UIntN target, UIntN source)
{
    UInt64 timeNow = m_time->getCurrentTimeInMilliseconds();
    const ThermalRelationshipTableEntry& entry = m_trt.getEntryForTargetAndSource(target, source);
    UInt64 timeDelta = tensOfSecondsToMilliseconds(entry.thermalSamplingPeriod());
    if (m_targetScheduler.hasCallbackScheduledAtOrBeforeTime(target, timeDelta) == false)
    {
        m_targetScheduler.cancelCallback(target);
        m_targetScheduler.scheduleCallback(target, timeDelta);
        UInt64 minimumSamplePeriod = tensOfSecondsToMilliseconds(m_trt.getMinimumSamplePeriodForSource(source));
        m_sourceAvailability.setSourceAsBusy(source, timeNow + minimumSamplePeriod);
    }

    m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
        "Scheduled a callback in " + std::to_string(timeDelta) + " ms" +
        " for target " + std::to_string(target) + " source " + std::to_string(source) + ".", target));
}

void CallbackScheduler::acknowledgeCallback(UIntN target)
{
    m_targetScheduler.removeCallback(target);
}

XmlNode* CallbackScheduler::getXml() const
{
    XmlNode* status = XmlNode::createWrapperElement("callback_scheduler");
    status->addChild(m_sourceAvailability.getXml());
    status->addChild(m_targetScheduler.getXml());
    return status;
}