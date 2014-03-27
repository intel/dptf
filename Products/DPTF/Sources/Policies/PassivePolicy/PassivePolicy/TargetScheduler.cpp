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

#include "TargetScheduler.h"
#include <iomanip>
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;


TargetScheduler::TargetScheduler(
    const PolicyServicesInterfaceContainer& policyServices, std::shared_ptr<TimeInterface> time)
    : m_policyServices(policyServices), m_time(time)
{
}

TargetScheduler::~TargetScheduler()
{
}

Bool TargetScheduler::hasCallbackWithinTimeRange(UIntN target, UInt64 beginTime, UInt64 endTime) const
{
    auto row = m_schedule.find(target);
    if (row == m_schedule.end())
    {
        return false;
    }

    UInt64 scheduledTime = row->second.getTimeStamp() + row->second.getTimeDelta();
    if ((beginTime <= scheduledTime) && (scheduledTime <= endTime))
    {
        return true;
    }
    else
    {
        return false;
    }
}

Bool TargetScheduler::hasCallbackScheduledAtOrBeforeTime(UIntN target, UInt64 time) const
{
    auto row = m_schedule.find(target);
    if (row != m_schedule.end())
    {
        return row->second.getTimeStamp() <= time;
    }
    else
    {
        return false;
    }
}

void TargetScheduler::cancelCallback(UIntN target)
{
    auto row = m_schedule.find(target);
    if (row != m_schedule.end())
    {
        m_policyServices.policyInitiatedCallback->removePolicyInitiatedCallback(row->second.getCallbackHandle());
        removeCallback(target);
    }
}

void TargetScheduler::removeCallback(UIntN target)
{
    m_schedule.erase(target);
}

void TargetScheduler::scheduleCallback(UIntN target, UInt64 time, UInt64 timeDelta)
{
    UInt64 callbackHandle =
        m_policyServices.policyInitiatedCallback->createPolicyInitiatedDeferredCallback(0, target, nullptr, timeDelta);
    m_schedule[target] = TargetCallback(timeDelta, time, callbackHandle);
}

void TargetScheduler::setTimeObject(std::shared_ptr<TimeInterface> time)
{
    m_time = time;
}

XmlNode* TargetScheduler::getXml() const
{
    float currentTime = (float)m_time->getCurrentTimeInMilliseconds();

    XmlNode* targetScheduler = XmlNode::createWrapperElement("target_scheduler");
    for (auto target = m_schedule.begin(); target != m_schedule.end(); target++)
    {
        XmlNode* targetCallback = XmlNode::createWrapperElement("target_callback");
        targetCallback->addChild(XmlNode::createDataElement("target", friendlyValue(target->first)));
        float expireTime = (float)target->second.getTimeStamp() + (float)target->second.getTimeDelta();
        float expireTimeDiffFromNow = (expireTime - currentTime) / (float)1000;
        targetCallback->addChild(
            XmlNode::createDataElement("time_until_expires", friendlyValue(expireTimeDiffFromNow)));
        targetScheduler->addChild(targetCallback);
    }
    return targetScheduler;
}