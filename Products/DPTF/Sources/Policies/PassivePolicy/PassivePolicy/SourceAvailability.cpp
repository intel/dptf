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

#include "SourceAvailability.h"
#include "StatusFormat.h"
#include <iomanip>

using namespace std;
using namespace StatusFormat;

SourceAvailability::SourceAvailability(
    const PolicyServicesInterfaceContainer& policyServices,
    std::shared_ptr<TimeInterface> time)
    : m_policyServices(policyServices), m_time(time)
{
}

SourceAvailability::~SourceAvailability()
{
}

UInt64 SourceAvailability::getNextAvailableTime(UIntN source, UInt64 startTime)
{
    if (startTime < m_schedule[source])
    {
        return m_schedule[source] - startTime;
    }
    else
    {
        return 0;
    }
}

void SourceAvailability::setSourceAsBusy(UIntN source, UInt64 time)
{
    m_schedule[source] = time;
}

void SourceAvailability::remove(UIntN source)
{
    auto item = m_schedule.find(source);
    if (item != m_schedule.end())
    {
        m_schedule.erase(item);
    }
}

Bool SourceAvailability::isBusyNow(UIntN source)
{
    auto item = m_schedule.find(source);
    if (item != m_schedule.end())
    {
        return (m_time->getCurrentTimeInMilliseconds() < item->second);
    }
    else
    {
        return false;
    }
}

XmlNode* SourceAvailability::getXml() const
{
    float currentTime = (float)m_time->getCurrentTimeInMilliseconds();

    XmlNode* status = XmlNode::createWrapperElement("source_availability");
    for (auto source = m_schedule.begin(); source != m_schedule.end(); source++)
    {
        XmlNode* activeSource = XmlNode::createWrapperElement("activity");
        activeSource->addChild(XmlNode::createDataElement("source", friendlyValue(source->first)));
        float timeTilAvailable = ((float)source->second - currentTime) / (float)1000;
        activeSource->addChild(XmlNode::createDataElement("time_until_available", friendlyValue(timeTilAvailable)));
        status->addChild(activeSource);
    }
    return status;
}

void SourceAvailability::setTime(std::shared_ptr<TimeInterface> time)
{
    m_time = time;
}