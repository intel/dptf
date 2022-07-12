/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "EnableDisableToggle.h"
#include "StringConverter.h"
namespace EnableDisableToggle
{
	std::string toString(EnableDisableToggle::State state)
	{
		switch (state)
		{
		case EnableDisableToggle::Disable:
			return "Disable";
		case EnableDisableToggle::Enable:
			return "Enable";
		default:
			return Constants::InvalidString;
		}
	}

	EnableDisableToggle::State toState(std::string state)
	{
		if (StringConverter::toUInt32(state) == 0)
		{
			return EnableDisableToggle::Disable;
		}
		else if (StringConverter::toUInt32(state) == 1)
		{
			return EnableDisableToggle::Enable;
		}
		else
		{
			throw dptf_exception("state is invalid");
		}
	}
}
