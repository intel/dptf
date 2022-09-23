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

#include "ForegroundRatioChoice.h"

namespace ForegroundRatioChoice
{
	ForegroundRatioChoice::Type fromString(std::string foregroundChoice)
	{
		if (foregroundChoice == toString(ForegroundRatioChoice::SingleCore))
		{
			return ForegroundRatioChoice::SingleCore;
		}
		else if (foregroundChoice == toString(ForegroundRatioChoice::TotalPhysicalCores))
		{
			return ForegroundRatioChoice::TotalPhysicalCores;
		}
		else if (foregroundChoice == toString(ForegroundRatioChoice::TotalLogicalProcessors))
		{
			return ForegroundRatioChoice::TotalLogicalProcessors;
		}
		else if (foregroundChoice == toString(ForegroundRatioChoice::TotalSmtCores))
		{
			return ForegroundRatioChoice::TotalSmtCores;
		}
		else
		{
			return ForegroundRatioChoice::Invalid;
		}
	}

	std::string toString(ForegroundRatioChoice::Type foregroundChoice)
	{
		switch (foregroundChoice)
		{
		case SingleCore:
			return "SingleCore";
		case TotalPhysicalCores:
			return "TotalPhysicalCores";
		case TotalLogicalProcessors:
			return "TotalLogicalProcessors";
		case TotalSmtCores:
			return "TotalSmtCores";
		default:
			return "Invalid";
		}
	}
}
