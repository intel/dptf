/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "OnOffToggle.h"

namespace OnOffToggle
{
	std::string toString(OnOffToggle::Type onOffType)
	{
		switch (onOffType)
		{
		case Off:
			return "Off";
		case On:
			return "On";
		default:
			throw dptf_exception("OnOffToggle::Type is invalid");
		}
	}

	OnOffToggle::Type toType(UIntN value)
	{
		if (value == 0) 
		{
			return OnOffToggle::Off;
		}
		else
		{
			return OnOffToggle::On;
		}
	}
}
