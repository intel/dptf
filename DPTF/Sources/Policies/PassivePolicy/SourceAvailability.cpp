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

#include "SourceAvailability.h"
#include "StatusFormat.h"

using namespace std;
using namespace StatusFormat;

SourceAvailability::SourceAvailability(
	const PolicyServicesInterfaceContainer& policyServices,
	std::shared_ptr<TimeInterface> time)
	: m_time(time)
	, m_policyServices(policyServices)
	, m_schedule(std::map<UIntN, TimeSpan>())
{
}

SourceAvailability::~SourceAvailability()
{
}

void SourceAvailability::setSourceAsBusy(UIntN source, const TimeSpan& time)
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

std::shared_ptr<XmlNode> SourceAvailability::getXml() const
{
	auto currentTime = m_time->getCurrentTime();
	auto status = XmlNode::createWrapperElement("source_availability");
	for (auto source = m_schedule.begin(); source != m_schedule.end(); source++)
	{
		auto activeSource = XmlNode::createWrapperElement("activity");
		activeSource->addChild(XmlNode::createDataElement("source", friendlyValue(source->first)));
		auto timeTilAvailable = source->second - currentTime;
		std::string availability;
		if (timeTilAvailable.asMillisecondsInt() <= 0)
		{
			availability = "Now";
		}
		else
		{
			availability = timeTilAvailable.toStringSeconds();
		}
		activeSource->addChild(XmlNode::createDataElement("time_until_available", availability));
		status->addChild(activeSource);
	}
	return status;
}

void SourceAvailability::setTime(std::shared_ptr<TimeInterface> time)
{
	m_time = time;
}

Bool SourceAvailability::isBusy(UIntN source, const TimeSpan& time) const
{
	auto sourceSchedule = m_schedule.find(source);
	if (sourceSchedule == m_schedule.end())
	{
		return false;
	}
	else
	{
		if (time < sourceSchedule->second)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
