/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "PolicyCallbackScheduler.h"
#include "StatusFormat.h"

using namespace std;
using namespace StatusFormat;

PolicyCallbackScheduler::PolicyCallbackScheduler(const PolicyServicesInterfaceContainer& policyServicesContainer,
    std::shared_ptr<TimeInterface> timeInterface)
    : m_policyServices(policyServicesContainer), m_time(timeInterface)
{
}

PolicyCallbackScheduler::~PolicyCallbackScheduler()
{
}

void PolicyCallbackScheduler::suspend(UIntN participantIndex, const TimeSpan& time)
{
    auto currentTime = getCurrentTime();
    cancelCallback(participantIndex);
    scheduleCallback(participantIndex, currentTime, time);
}

void PolicyCallbackScheduler::suspend(UIntN participantIndex, UInt64 fromTime, const TimeSpan& suspendTime)
{
    cancelCallback(participantIndex);
    scheduleCallback(participantIndex, fromTime, suspendTime);
}

void PolicyCallbackScheduler::cancelCallback(UIntN participantIndex)
{
    auto row = m_schedule.find(participantIndex);
    if (row != m_schedule.end())
    {
        m_policyServices.policyInitiatedCallback->removePolicyInitiatedCallback(row->second.getCallbackHandle());
        acknowledgeCallback(participantIndex);
    }
}

Bool PolicyCallbackScheduler::hasCallbackWithinTimeRange(UIntN target, UInt64 beginTime, UInt64 endTime) const
{
    auto row = m_schedule.find(target);
    if (row == m_schedule.end())
    {
        return false;
    }

    UInt64 scheduledTime = row->second.getTimeStamp() + row->second.getTimeDelta().asMillisecondsInt();
    if ((beginTime <= scheduledTime) && (scheduledTime <= endTime))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void PolicyCallbackScheduler::acknowledgeCallback(UIntN participantIndex)
{
    m_schedule.erase(participantIndex);
}

void PolicyCallbackScheduler::setTimeObject(std::shared_ptr<TimeInterface> time)
{
    m_time = time;
}

std::shared_ptr<XmlNode> PolicyCallbackScheduler::getStatus()
{
    auto status = XmlNode::createWrapperElement("policy_callback_scheduler");
    for (auto participant = m_schedule.begin(); participant != m_schedule.end(); participant++)
    {
        status->addChild(getStatusForParticipant(participant->first));
    }

    return status;
}

std::shared_ptr<XmlNode> PolicyCallbackScheduler::getStatusForParticipant(UIntN participantIndex)
{
    auto status = XmlNode::createWrapperElement("participant_callback");

    try
    {
        status->addChild(
            XmlNode::createDataElement("participant_index", friendlyValue(participantIndex)));

        auto scheduledCallback = m_schedule.find(participantIndex);
        if (scheduledCallback != m_schedule.end())
        {
            auto expireTime = scheduledCallback->second.getTimeStamp() +
                scheduledCallback->second.getTimeDelta().asMillisecondsInt();
            auto currentTime = getCurrentTime();
            if (expireTime >= currentTime)
            {
                auto expireTimeDiffFromNow = (double)(expireTime - currentTime) / 1000.0;
                status->addChild(
                    XmlNode::createDataElement("time_until_expires", friendlyValue(expireTimeDiffFromNow)));
            }
            else
            {
                status->addChild(XmlNode::createDataElement("time_until_expires", "X"));
            }
        }
        else
        {
            status->addChild(XmlNode::createDataElement("time_until_expires", "X"));
        }
    }
    catch (dptf_exception)
    {
        status->addChild(XmlNode::createDataElement("time_until_expires", "X"));
    }

    return status;
}

UInt64 PolicyCallbackScheduler::getCurrentTime()
{
    return m_time->getCurrentTimeInMilliseconds();
}

void PolicyCallbackScheduler::scheduleCallback(UIntN participantIndex, UInt64 currentTime, const TimeSpan& pollTime)
{
     UInt64 callbackHandle = m_policyServices.policyInitiatedCallback->createPolicyInitiatedDeferredCallback
        (0, participantIndex, nullptr, pollTime);
     m_schedule[participantIndex] = ParticipantCallback(pollTime, currentTime, callbackHandle);

    m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
        "Scheduled a callback in " + pollTime.toStringMilliseconds() + " ms" +
        " for participant " + StlOverride::to_string(participantIndex) + ".",participantIndex));
}
