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
		case Balanced:
			return "Balanced";
		case AcQuiet:
			return "Quiet";
		case Collaboration:
			return "Collaboration";
		case IpAlignment:
			return "VC IP Alignment";
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
		else if (mbtMode == toString(MbtMode::Balanced) || mbtMode == toString(MbtMode::DcBetterPerformance))
		{
			return MbtMode::Balanced;
		}
		else if (mbtMode == toString(MbtMode::AcQuiet))
		{
			return MbtMode::AcQuiet;
		}
		else if (mbtMode == toString(MbtMode::Collaboration))
		{
			return MbtMode::Collaboration;
		}
		else if (mbtMode == toString(MbtMode::IpAlignment))
		{
			return MbtMode::IpAlignment;
		}
		return MbtMode::Invalid;
	}
}
