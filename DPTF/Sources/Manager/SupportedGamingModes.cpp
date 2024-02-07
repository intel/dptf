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

#include "SupportedGamingModes.h"
#include <string>

using namespace std;
using namespace DttGamingMode;

SupportedGamingModes::SupportedGamingModes(const set<Type>& supportedModes)
	: m_supportedModes(supportedModes)
{

}

size_t SupportedGamingModes::count() const
{
	return m_supportedModes.size();
}

Bool SupportedGamingModes::contains(Type gameMode) const
{
	return m_supportedModes.count(gameMode);
}

string SupportedGamingModes::toString() const
{
	stringstream stream;
	for (const auto mode : m_supportedModes)
	{
		stream << DttGamingMode::toString(mode) << ", "s;
	}
	auto result = stream.str();

	if (result.size() >= 2)
	{
		result.pop_back();
		result.pop_back();
		return result;
	}
	return "Empty";
}

set<FrameworkEvent::Type> SupportedGamingModes::getRequiredEvents() const
{
	set<FrameworkEvent::Type> eventList;
	if (contains(MaxPerformance))
	{
		eventList.insert(FrameworkEvent::PolicyApplicationOptimizationChanged);
		eventList.insert(FrameworkEvent::PolicyOperatingSystemPowerSourceChanged);
	}

	if (contains(EnduranceGaming))
	{
		eventList.insert(FrameworkEvent::DptfAppBroadcastUnprivileged);
		eventList.insert(FrameworkEvent::PolicyOperatingSystemPowerSourceChanged);
	}
	return eventList;
}