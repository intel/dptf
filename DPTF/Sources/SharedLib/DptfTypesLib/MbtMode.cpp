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

#include "MbtMode.h"

namespace MbtMode
{
	std::string toString(MbtMode::Type mbtMode)
	{
		switch (mbtMode)
		{
		case DcEnduranceGaming:
			return "Endurance Gaming";
		case DcBetterBattery:
			return "Better Battery";
		case DcBetterPerformance:
			return "Better Performance";
		case AcBalanced:
			return "Balanced";
		case AcQuiet:
			return "Quiet";
		case AcCollaboration:
			return "Collaboration";	
		default:
			throw dptf_exception("MbtMode::Type is invalid");
		}
	}

	MbtMode::Type fromString(std::string mbtMode)
	{
		if (mbtMode == toString(MbtMode::DcEnduranceGaming))
		{
			return MbtMode::DcEnduranceGaming;
		}
		else if (mbtMode == toString(MbtMode::DcBetterBattery))
		{
			return MbtMode::DcBetterBattery;
		}
		else if (mbtMode == toString(MbtMode::DcBetterPerformance))
		{
			return MbtMode::DcBetterPerformance;
		}
		else if (mbtMode == toString(MbtMode::AcBalanced))
		{
			return MbtMode::AcBalanced;
		}
		else if (mbtMode == toString(MbtMode::AcQuiet))
		{
			return MbtMode::AcQuiet;
		}
		else if (mbtMode == toString(MbtMode::AcCollaboration))
		{
			return MbtMode::AcCollaboration;
		}
		return MbtMode::Invalid;
	}

	std::string toPpmKeyString(MbtMode::Type mbtMode)
	{
		switch (mbtMode)
		{
		case DcEnduranceGaming:
			return "EnduranceGaming";
		case DcBetterBattery:
			return "BetterBatt";
		case DcBetterPerformance:
			return "BetterPerf";
		case AcBalanced:
			return "Balanced";
		case AcQuiet:
			return "Quiet";
		case AcCollaboration:
			return "Collaboration";
		default:
			throw dptf_exception("MbtMode::Type is invalid");
		}
	}
}
