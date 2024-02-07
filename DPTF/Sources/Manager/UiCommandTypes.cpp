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
#include "UiCommandTypes.h"
#include <string>
#include "Constants.h"

std::string UiCommandGroupType::ToString(UiCommandGroupType::Type type)
{
	switch (type)
	{
	case Policies:
		return "Policies";
	case Participants:
		return "Participants";
	case Framework:
		return "Framework";
	case Arbitrator:
		return "Arbitrator";
	case System:
		return "System";
	default:
		return Constants::InvalidString;
	}
}

std::string UiCommandManagerModuleType::ToString(UiCommandManagerModuleType::Type type)
{
	switch (type)
	{
	case Events:
		return "Event Status";
	case Participants:
		return "Participant Status";
	case Policies:
		return "Policy Status";
	case Statistics:
		return "Work Item Statistics";
	default:
		return Constants::InvalidString;
	}
}

std::string UiCommandSystemModuleType::ToString(UiCommandSystemModuleType::Type type)
{
	switch (type)
	{
	case SystemConfiguration:
		// it is important that this name matches the name in the "policy" cpp file
		return "System Configuration";
	default:
		return Constants::InvalidString;
	}
}