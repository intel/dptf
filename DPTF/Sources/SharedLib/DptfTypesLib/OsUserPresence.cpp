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

#include "OsUserPresence.h"

namespace OsUserPresence
{
	std::string toString(OsUserPresence::Type osUserPresence)
	{
		switch (osUserPresence)
		{
		case Present:
			return "Present";
		case NotPresent:
			return "NotPresent";
		case Inactive:
			return "Inactive";
		default:
			throw dptf_exception("OsUserPresence::Type is invalid");
		}
	}

	OsUserPresence::Type toType(UIntN value)
	{
		if (value == 0)
		{
			return OsUserPresence::Present;
		}
		else if (value == 1)
		{
			return OsUserPresence::NotPresent;
		}
		else
		{
			return OsUserPresence::Inactive;
		}
	}
}
