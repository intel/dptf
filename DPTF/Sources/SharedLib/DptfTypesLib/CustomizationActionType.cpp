/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "CustomizationActionType.h"

namespace CustomizationAction
{
	std::string toString(CustomizationAction::Type actionType)
	{
		switch (actionType)
		{
		case PpmReset:
			return "PpmReset";
		case PpmSet:
			return "PpmSet";
		default:
			return "Invalid";
		}
	}

	CustomizationAction::Type fromString(std::string actionType)
	{
		if (actionType == toString(CustomizationAction::PpmReset))
		{
			return CustomizationAction::PpmReset;
		}
		else if (actionType == toString(CustomizationAction::PpmSet))
		{
			return CustomizationAction::PpmSet;
		}
		else
		{
			return CustomizationAction::Invalid;
		}
	}
}
