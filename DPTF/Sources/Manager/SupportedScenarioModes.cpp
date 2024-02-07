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

#include "SupportedScenarioModes.h"
#include <string>

using namespace std;
using namespace ScenarioMode;

SupportedScenarioModes::SupportedScenarioModes(const set<Type>& supportedModes)
	: m_supportedModes(supportedModes)
{

}

size_t SupportedScenarioModes::count() const
{
	return m_supportedModes.size();
}

Bool SupportedScenarioModes::contains(Type scenarioMode) const
{
	return m_supportedModes.count(scenarioMode);
}

string SupportedScenarioModes::toString() const
{
	stringstream stream;
	for (const auto scenarioMode : m_supportedModes)
	{
		stream << ScenarioMode::toString(scenarioMode) << ", "s;
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

set<FrameworkEvent::Type> SupportedScenarioModes::getRequiredEvents() const
{
	set<FrameworkEvent::Type> eventList;
	if (contains(Collaboration))
	{
		eventList.insert(FrameworkEvent::PolicyCollaborationChanged);
	}

	if (contains(EwpCoolAndQuiet) || contains(EwpPerformance))
	{
		eventList.insert(FrameworkEvent::DomainExtendedWorkloadPredictionChanged);
	}
	return eventList;
}
