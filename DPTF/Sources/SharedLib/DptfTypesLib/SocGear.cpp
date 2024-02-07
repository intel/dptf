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

#include "SocGear.h"

namespace SocGear
{
	std::string toString(SocGear::Type socGear)
	{
		switch (socGear)
		{
		case Performance:
			return "Performance";
		case Balanced:
			return "Balanced";
		case PowerSaver:
			return "PowerSaver";
		default:
			return Constants::InvalidString;
		}
	}

	SocGear::Type fromString(std::string socGear)
	{
		if (socGear == toString(SocGear::Performance))
		{
			return SocGear::Performance;
		}
		else if (socGear == toString(SocGear::Balanced))
		{
			return SocGear::Balanced;
		}
		else if (socGear == toString(SocGear::PowerSaver))
		{
			return SocGear::PowerSaver;
		}		
		return SocGear::Invalid;
	}
}
