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

#include "MbtHint.h"

namespace MbtHint
{
	std::string toString(MbtHint::Type mbtHint)
	{
		switch (mbtHint)
		{
		case Idle:
			return "Idle";
		case SemiActive:
			return "Semi-Active";
		case Bursty:
			return "Bursty";
		case Sustained:
			return "Sustained";
		case BatteryLife:
			return "Battery Life";
		default:
			return Constants::InvalidString;
		}
	}
}
