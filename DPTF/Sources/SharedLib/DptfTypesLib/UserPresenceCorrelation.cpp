/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "UserPresenceCorrelation.h"

namespace UserPresenceCorrelation
{
	std::string toString(UserPresenceCorrelation::Type userPresenceCorrelation)
	{
		switch (userPresenceCorrelation)
		{
		case Negative:
			return "Negative";
		case Positive:
			return "Positive";
		default:
			throw dptf_exception("UserPresenceCorrelation::Type is invalid");
		}
	}

	UserPresenceCorrelation::Type toType(UIntN value)
	{
		if (value == 0)
		{
			return UserPresenceCorrelation::Negative;
		}
		else if (value == 1)
		{
			return UserPresenceCorrelation::Positive;
		}
		throw dptf_exception("Value is invalid");
	}
}
