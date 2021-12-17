/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "SystemMode.h"

namespace SystemMode
{
	std::string toString(SystemMode::Type systemMode)
	{
		switch (systemMode)
		{
		case Performance:
			return "Performance";
		case Balanced:
			return "Balanced";
		case Quiet:
			return "Quiet";
		case EnduranceGaming:
			return "EnduranceGaming";
		case Invalid:
			return "Invalid";
		default:
			throw dptf_exception("SystemMode::Type is unknown");
		}
	}

	SystemMode::Type toType(UIntN value)
	{
		if (value == 0)
		{
			return SystemMode::Performance;
		}
		else if (value == 1)
		{
			return SystemMode::Balanced;
		}
		else if (value == 2)
		{
			return SystemMode::Quiet;
		}
		else if (value == 3)
		{
			return SystemMode::EnduranceGaming;
		}
		else if (value == 4)
		{
			return SystemMode::Invalid;
		}
		throw dptf_exception("System Mode value is unknown");
	}
}
