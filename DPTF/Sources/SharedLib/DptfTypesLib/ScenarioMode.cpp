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

#include "ScenarioMode.h"
#include "DptfExceptions.h"

using namespace std;

namespace ScenarioMode
{
	std::string toString(const Type scenarioMode)
	{
		switch (scenarioMode)
		{
		case OverClock:
			return "OverClock"s;
		case Gaming:
			return "Gaming"s;
		case Collaboration:
			return "Collaboration"s;
		case EwpPerformance:
			return "EwpPerformance"s;
		case EwpCoolAndQuiet:
			return "EwpCoolAndQuiet"s;
		case Invalid:
			return "Invalid"s;
		default:
			throw dptf_exception("Scenario Mode Type is unknown"s);
		}
	}

	Type toType(UIntN value)
	{
		if (value > Invalid)
		{
			throw dptf_exception("Scenario Mode value is unknown"s);
		}
		return static_cast<Type>(value);
	}

	Type fromString(const std::string& scenarioMode)
	{
		if (scenarioMode == toString(OverClock))
		{
			return OverClock;
		}

		if (scenarioMode == toString(Gaming))
		{
			return Gaming;
		}

		if (scenarioMode == toString(Collaboration))
		{
			return Collaboration;
		}

		if (scenarioMode == toString(EwpPerformance))
		{
			return EwpPerformance;
		}

		if (scenarioMode == toString(EwpCoolAndQuiet))
		{
			return EwpCoolAndQuiet;
		}

		return Invalid;
	}
}
