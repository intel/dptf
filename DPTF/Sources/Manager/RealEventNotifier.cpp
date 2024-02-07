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

#include "RealEventNotifier.h"
using namespace std;

void RealEventNotifier::notify(FrameworkEvent::Type event, const DptfBuffer& eventPayload)
{
	for (const auto& observer : m_registrations[event])
	{
		observer->update(event, eventPayload);
	}
}

void RealEventNotifier::registerObserver(
	const shared_ptr<EventObserverInterface>& observer,
	set<FrameworkEvent::Type> events)
{
	for (const auto& event : events)
	{
		m_registrations[event].insert(observer);
	}
}

void RealEventNotifier::unregisterObserverFromAllEvents(const shared_ptr<EventObserverInterface>& observer)
{
	for (auto& registration : m_registrations)
	{
		registration.second.erase(observer);
	}
}

void RealEventNotifier::unregisterObserver(
	const shared_ptr<EventObserverInterface>& observer,
	set<FrameworkEvent::Type> events)
{
	for (const auto& event : events)
	{
		if (m_registrations.find(event) != m_registrations.end())
		{
			m_registrations.at(event).erase(observer);
		}
	}
}
