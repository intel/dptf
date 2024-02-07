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

#include "PlatformPowerSource.h"

namespace PlatformPowerSource
{
	std::string ToString(PlatformPowerSource::Type type)
	{
		switch (type)
		{
		case PlatformPowerSource::AC:
			return "AC";
		case PlatformPowerSource::DC:
			return "DC";
		case PlatformPowerSource::USB:
			return "USB";
		case PlatformPowerSource::WC:
			return "Wireless";
		default:
			throw dptf_exception("PlatformPowerSource::Type is invalid.");
		}
	}
}
