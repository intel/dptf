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

#include <map>
#include "ControlKnobType.h"
#include "DptfExceptions.h"

using namespace std;

const map<ControlKnobType::Type, string> typeNames =
{
	{ControlKnobType::PowerControlPL1, "PowerControlPL1"s},
	{ControlKnobType::PowerControlPl2, "PowerControlPL2"s},
	{ControlKnobType::PowerControlPl3, "PowerControlPL3"s},
	{ControlKnobType::PowerControlPl4, "PowerControlPL4"s},
	{ControlKnobType::PerformanceControlPerfFrequency, "PerformanceControlPerfFrequency"s},
	{ControlKnobType::PerformanceControlThrottleFrequency, "PerformanceControlThrottleFrequency"s},
	{ControlKnobType::DbptControlIccMax, "DbptControlIccMax"s},
	{ControlKnobType::CoreControlLpo, "CoreControlLpo"s},
	{ControlKnobType::DisplayControlBrightness, "DisplayControlBrightness"s},
	{ControlKnobType::PercentageFanControl, "PercentageFanControl"s},
	{ControlKnobType::TauControlPl1, "TauControlPl1"s},
	{ControlKnobType::TauControlPl2, "TauControlPl2"s},
	{ControlKnobType::TauControlPl3, "TauControlPl3"s},
	{ControlKnobType::DataThroughput, "DataThroughput"s},
	{ControlKnobType::PerformanceControlPerfIndex, "PerformanceControlPerfIndex"s},
	{ControlKnobType::PSysPowerControlPl1, "PSysPowerControlPl1"s},
	{ControlKnobType::PSysPowerControlPl2, "PSysPowerControlPl2"s},
	{ControlKnobType::PSysPowerControlPl3, "PSysPowerControlPl3"s},
	{ControlKnobType::RPMFanControl, "RPMFanControl"s},
};

namespace ControlKnobType
{
	string toString(ControlKnobType::Type type)
	{
		const auto entry = typeNames.find(type);
		if (entry != typeNames.end())
		{
			return entry->second;
		}
		throw dptf_exception("ControlKnobType::Type is invalid.");
	}
	ControlKnobType::Type fromString(const std::string& typeName)
	{
		for (const auto& entry : typeNames)
		{
				if (entry.second == typeName)
			{
				return entry.first;
			}
		}
		throw dptf_exception("ControlKnobType name is invalid.");
	}
}
